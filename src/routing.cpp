#include "routing.h"
#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

// ============================================================================
// INTERNAL DATA STRUCTURES FOR MODULE 4
// ============================================================================

// 1. Department adjacency graph for BFS cascading failure routing
static unordered_map<string, vector<string>> routingGraph;

// 2. Custom comparator for Emergency Override Priority Queue (Button E)
struct OverrideComparator {
    bool operator()(Patient* a, Patient* b) {
        // Higher urgency or difficulty gets top priority
        if (a->urgency != b->urgency) {
            return a->urgency < b->urgency; // max-heap by urgency
        }
        return a->difficulty < b->difficulty; // if urgency equals, check difficulty
    }
};

static priority_queue<Patient*, vector<Patient*>, OverrideComparator> overrideQueue;

// 3. Stack to save state of paused patients during critical pause (Button C)
static stack<Patient*> criticalStopStack;

// 4. Stack to save system state during Mass Casualty Incident mode (Button M)
static bool isMCIModeActive = false;
static stack<bool> mciStateStack;

// ============================================================================
// IMPLEMENTATION
// ============================================================================

string standardizeDepartmentName(string rawDept) {
    string s = rawDept;
    if (s == "Internal Medicine" || s == "internal") return "internal";
    if (s == "Surgery" || s == "surgery") return "surgery";
    if (s == "Pediatrics" || s == "pediatrics") return "pediatrics";
    if (s == "Emergency" || s == "emergency") return "emergency";
    if (s == "Cardiology") return "internal";   // Map Cardiology to internal medicine
    if (s == "Orthopedics") return "surgery";   // Map Orthopedics to surgery
    if (s == "Neurology") return "internal";    // Map Neurology to internal medicine
    if (s == "Ophthalmology") return "surgery"; // Map Ophthalmology to surgery
    if (s == "obstetrics") return "obstetrics";
    return "internal";
}

void initRoutingGraph() {
    if (!routingGraph.empty()) return;
    // Define adjacency list relationships for BFS Cascading Failure search
    routingGraph["emergency"]  = {"surgery", "internal"};
    routingGraph["surgery"]    = {"emergency", "internal"};
    routingGraph["internal"]   = {"emergency", "pediatrics"};
    routingGraph["pediatrics"] = {"internal", "emergency"};
    routingGraph["obstetrics"] = {"surgery", "internal"};
}

Doctor* assignDoctorToCase(Patient* p) {
    if (p == nullptr) return nullptr;
    initRoutingGraph();

    string targetDept = standardizeDepartmentName(p->department);
    int requiredLevel = p->difficulty;
    
    if (isMCIModeActive) {
        cout << "[MCI NOTICE] Routing active under Mass Casualty protocol." << endl;
    }

    // Attempt to find the available doctor with minimal case load in the primary department
    Doctor* assignedDoc = findAvailableDoctor(requiredLevel, targetDept);

    if (assignedDoc != nullptr) {
        incrementCase(assignedDoc->id);
        addHistoryNode(p->id, "Assigned to " + assignedDoc->name + 
            " (A" + to_string(assignedDoc->level) + ", " + assignedDoc->department + ")");
        return assignedDoc;
    }

    // Trigger Cascading Failure search via BFS traversal to adjacent backup departments
    cout << "[WARN] Department '" << targetDept << "' full. Triggering BFS Cascading Failure search..." << endl;

    queue<string> q;
    unordered_map<string, bool> visited;

    q.push(targetDept);
    visited[targetDept] = true;

    while (!q.empty()) {
        string currDept = q.front();
        q.pop();

        if (currDept != targetDept) {
            // Check available doctors in backup department (accepting level 1+ to handle spillover cases)
            Doctor* d = findAvailableDoctor(1, currDept);
            if (d != nullptr) {
                cout << "[SUCCESS] Cascading BFS found backup doctor in department: " << currDept << endl;
                incrementCase(d->id);
                addHistoryNode(p->id, "Cascading assigned to backup " + d->name + 
                    " (A" + to_string(d->level) + ", " + d->department + ")");
                return d;
            }
        }

        for (const string& neighbor : routingGraph[currDept]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    cout << "[ERROR] System Overload! No available doctors in primary or backup departments." << endl;
    addHistoryNode(p->id, "Waiting for doctor (System Overloaded)");
    return nullptr;
}

string findBackupDepartment(string fullDept) {
    initRoutingGraph();
    string startDept = standardizeDepartmentName(fullDept);

    queue<string> q;
    unordered_map<string, bool> visited;

    q.push(startDept);
    visited[startDept] = true;

    while (!q.empty()) {
        string curr = q.front();
        q.pop();

        if (curr != startDept) {
            Doctor* d = findAvailableDoctor(1, curr);
            if (d != nullptr) {
                return curr;
            }
        }

        for (const string& neighbor : routingGraph[curr]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }
    return "none";
}

void localOverride(Patient* urgentPatient) {
    if (urgentPatient == nullptr) return;
    overrideQueue.push(urgentPatient);
    cout << "[EMERGENCY] Patient " << urgentPatient->name << " (" << urgentPatient->id 
         << ") added to Override Priority Queue (Urgency: " << urgentPatient->urgency << ")." << endl;
    addHistoryNode(urgentPatient->id, "Triggered Emergency Override (Button E)");
}

void processNextOverride() {
    if (overrideQueue.empty()) {
        cout << "[INFO] No pending override cases in priority queue." << endl;
        return;
    }
    Patient* p = overrideQueue.top();
    overrideQueue.pop();

    cout << "\n>>> Processing Override Patient: " << p->name << " (" << p->id << ") <<<" << endl;
    Doctor* doc = assignDoctorToCase(p);
    if (doc != nullptr) {
        cout << "[OK] Override case successfully assigned to " << doc->name << endl;
    } else {
        cout << "[CRITICAL] Cannot assign override case! Immediate manual backup needed." << endl;
    }
}

void criticalStop(Patient* criticalPatient) {
    if (criticalPatient == nullptr) return;
    criticalStopStack.push(criticalPatient);
    cout << "[CRITICAL STOP] Paused current case: " << criticalPatient->name 
         << " (" << criticalPatient->id << "). Saved state to Stack." << endl;
    addHistoryNode(criticalPatient->id, "Paused due to Critical Stop (Button C)");
}

void resumeCriticalStop() {
    if (criticalStopStack.empty()) {
        cout << "[INFO] No paused patients in Critical Stop stack." << endl;
        return;
    }
    Patient* p = criticalStopStack.top();
    criticalStopStack.pop();
    cout << "\n>>> Resuming Paused Patient from Stack: " << p->name << " (" << p->id << ") <<<" << endl;
    addHistoryNode(p->id, "Resumed processing from Critical Stop stack");
    
    Doctor* doc = assignDoctorToCase(p);
    if (doc != nullptr) {
        cout << "[OK] Resumed case successfully assigned to " << doc->name << endl;
    }
}

void enterMCIMode() {
    mciStateStack.push(isMCIModeActive);
    isMCIModeActive = true;
    cout << "\n=================================================================" << endl;
    cout << " [MCI MODE ACTIVATED] Mass Casualty Incident protocol initiated! " << endl;
    cout << " Previous state successfully preserved onto MCI Stack.           " << endl;
    cout << "=================================================================\n" << endl;
}

void exitMCIMode() {
    if (mciStateStack.empty()) {
        cout << "[INFO] MCI Mode stack is empty." << endl;
        return;
    }
    isMCIModeActive = mciStateStack.top();
    mciStateStack.pop();
    cout << "\n=================================================================" << endl;
    cout << " [MCI MODE DEACTIVATED] Restored previous system routing state.  " << endl;
    cout << "=================================================================\n" << endl;
}

void displayDashboard() {
    cout << "\n=================================================================" << endl;
    cout << "               MEDROUTE LIVE HOSPITAL DASHBOARD                  " << endl;
    cout << "=================================================================" << endl;
    cout << " System Status         : " << (isMCIModeActive ? "[ WARNING: MCI MODE ACTIVE ]" : "[ NORMAL OPERATION ]") << endl;
    cout << " Pending Overrides (E) : " << overrideQueue.size() << " case(s)" << endl;
    cout << " Paused Cases (C)      : " << criticalStopStack.size() << " case(s)" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    displayAllStations();
    cout << " Total Doctors Loaded  : " << getDoctorCount() << endl;
    cout << "=================================================================\n" << endl;
}

void mainMenu() {
    initRoutingGraph();
    string choice;
    while (true) {
        cout << "\n=================================================================" << endl;
        cout << "            MEDROUTE MAIN MENU (Routing & Emergency UI)          " << endl;
        cout << "=================================================================" << endl;
        cout << " 1. Register Patient & Route Case" << endl;
        cout << " 2. Display Live System Dashboard" << endl;
        cout << " 3. Display All Loaded Doctors" << endl;
        cout << " 4. [Button E] Emergency Override: Insert urgent case to Priority Queue" << endl;
        cout << " 5. Process Next Top Override Case" << endl;
        cout << " 6. [Button C] Critical Stop: Pause current case to Stack" << endl;
        cout << " 7. Resume Paused Case from Stack" << endl;
        cout << " 8. [Button M] Activate MCI Mode (Mass Casualty Incident)" << endl;
        cout << " 9. Deactivate MCI Mode" << endl;
        cout << " 10. Display Patient Event History" << endl;
        cout << " 0. Exit Program" << endl;
        cout << "=================================================================" << endl;
        cout << "Select option (0-10): ";
        cin >> choice;

        if (choice == "0") {
            cout << "\nThank you for using MedRoute System. Goodbye!\n" << endl;
            break;
        } else if (choice == "1") {
            string name, symptom;
            int urgency;
            cout << "\n--- Register New Patient ---" << endl;
            cout << "Patient Name: ";
            cin >> ws;
            getline(cin, name);
            cout << "Initial Symptom (e.g. chest pain, general fever, broken bone): ";
            getline(cin, symptom);
            cout << "Urgency Level (1=Normal, 2=Urgent, 3=Critical): ";
            cin >> urgency;

            Patient* p = registerPatient(name, symptom, urgency);
            if (p != nullptr) {
                // ===== T08: ถ้าอาการไม่รู้จัก ให้พยาบาลเลือกแผนกเอง =====
                if (p->department == "unknown") {
                    cout << "\n[WARN] Symptom '" << symptom << "' not in database!" << endl;
                    cout << "Please choose department manually:" << endl;
                    cout << "  1. Internal Medicine" << endl;
                    cout << "  2. Surgery" << endl;
                    cout << "  3. Pediatrics" << endl;
                    cout << "  4. Emergency" << endl;
                    cout << "  5. Obstetrics" << endl;
                    cout << "Choice (1-5): ";
                    int deptChoice;
                    cin >> deptChoice;

                    switch (deptChoice) {
                        case 1: p->department = "Internal Medicine"; break;
                        case 2: p->department = "Surgery"; break;
                        case 3: p->department = "Pediatrics"; break;
                        case 4: p->department = "Emergency"; break;
                        case 5: p->department = "obstetrics"; break;
                        default: p->department = "Internal Medicine"; break;
                    }

                    if (p->difficulty == 0) p->difficulty = urgency;
                    cout << "[OK] Manually set department to: " << p->department << endl;
                }

                Station* targetStation = findFreeStation();
                if (targetStation == nullptr) {
                    targetStation = findEarliestFreeStation();
                }

                addPatientToStation(targetStation->id, p->id);
                pushStation(p->id, targetStation->id);

                cout << "\nSearching for optimal doctor via Graph BFS Routing..." << endl;
                Doctor* assigned = assignDoctorToCase(p);
                if (assigned != nullptr) {
                    cout << "[SUCCESS] Assigned patient " << p->name << " to " << assigned->name
                         << " (A" << assigned->level << ") in department " << assigned->department
                         << " | Cases: " << assigned->current_cases << "/" << assigned->max_threshold
                         << endl;
                }
            }
        } else if (choice == "2") {
            displayDashboard();
        } else if (choice == "3") {
            displayAllDoctors();
        } else if (choice == "4") {
            string pid;
            cout << "\n--- Button E: Emergency Override ---" << endl;
            cout << "Enter Patient ID to override (e.g. P001): ";
            cin >> pid;
            Patient* p = findPatientByID(pid);
            if (p != nullptr) {
                localOverride(p);
            } else {
                cout << "[ERROR] Patient ID not found: " << pid << endl;
            }
        } else if (choice == "5") {
            processNextOverride();
        } else if (choice == "6") {
            string pid;
            cout << "\n--- Button C: Critical Stop ---" << endl;
            cout << "Enter Patient ID to pause onto Stack: ";
            cin >> pid;
            Patient* p = findPatientByID(pid);
            if (p != nullptr) {
                criticalStop(p);
            } else {
                cout << "[ERROR] Patient ID not found: " << pid << endl;
            }
        } else if (choice == "7") {
            resumeCriticalStop();
        } else if (choice == "8") {
            enterMCIMode();
        } else if (choice == "9") {
            exitMCIMode();
        } else if (choice == "10") {
            string pid;
            cout << "\n--- Search Patient History ---" << endl;
            cout << "Enter Patient ID (e.g. P001): ";
            cin >> pid;
            displayPatientHistory(pid);
        } else {
            cout << "[ERROR] Invalid option. Please choose between 0-10." << endl;
        }
    }
}