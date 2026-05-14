// ============================================================
// FILE: patient.cpp
// PROJECT: MedRoute — Hospital Queue Management System
// MODULE: Patient & Symptom Module (Omsin's part)
// ============================================================
// This file implements all 10 functions declared in patient.h
// Data Structures used:
//   - unordered_map (Hash Table) : symptom → department  O(1)
//   - BST (manual)               : symptom → difficulty  O(log n)
//   - Linked List (manual)       : patient history       O(1) append
//   - Stack (STL stack<string>)  : station tracking      O(1) push/pop
// ============================================================

#include "patient.h"
#include <iostream>
#include <ctime>        // for automatic timestamp
#include <iomanip>      // for time formatting
using namespace std;


// ============================================================
// GLOBAL DATA STORES
// These live for the entire runtime of the program.
// Other modules should NOT access these directly —
// use the functions below instead.
// ============================================================

// Hash Table: stores all registered patients, key = patient ID
// e.g. patientTable["P001"] = pointer to Patient object
unordered_map<string, Patient*> patientTable;

// Counter for auto-generating patient IDs (P001, P002, ...)
int patientCounter = 1;

// Hash Table: symptom string → department string
// Populated once in initSymptomMaps(), used by mapSymptomToDepartment()
unordered_map<string, string> symptomDeptMap;

// BST root: symptom string → difficulty int (1-4)
// Populated once in initSymptomMaps(), used by mapSymptomToDifficulty()
BSTNode* difficultyBSTRoot = nullptr;


// ============================================================
// SECTION A: BST HELPER FUNCTIONS (internal use only)
// Not declared in patient.h — only used inside this file
// ============================================================

// Insert a new symptom+difficulty pair into the BST
// BST is ordered alphabetically by symptom string
BSTNode* bstInsert(BSTNode* root, string symptom, int difficulty) {
    if (root == nullptr) {
        // Create a new node when we reach an empty spot
        BSTNode* newNode = new BSTNode();
        newNode->symptom    = symptom;
        newNode->difficulty = difficulty;
        newNode->left       = nullptr;
        newNode->right      = nullptr;
        return newNode;
    }
    // Go left if symptom comes before current node alphabetically
    if (symptom < root->symptom) {
        root->left = bstInsert(root->left, symptom, difficulty);
    }
    // Go right if symptom comes after current node alphabetically
    else if (symptom > root->symptom) {
        root->right = bstInsert(root->right, symptom, difficulty);
    }
    // If equal, symptom already exists — do nothing (no duplicates)
    return root;
}

// Search BST for a symptom, return its difficulty (0 = not found)
// Time complexity: O(log n) average
int bstSearch(BSTNode* root, string symptom) {
    if (root == nullptr) return 0; // not found → return 0
    if (symptom == root->symptom)  return root->difficulty;
    if (symptom < root->symptom)   return bstSearch(root->left,  symptom);
    return                                bstSearch(root->right, symptom);
}


// ============================================================
// SECTION B: SYMPTOM MAP INITIALIZATION
// Called once at the start — loads all known symptoms into:
//   1. symptomDeptMap (Hash Table)
//   2. difficultyBSTRoot (BST)
// ============================================================
void initSymptomMaps() {
    // ---- Hash Table: symptom → department ----
    // O(1) lookup per symptom
    symptomDeptMap["general fever"]        = "Internal Medicine";
    symptomDeptMap["severe stomach pain"]  = "Surgery";
    symptomDeptMap["child with fever"]     = "Pediatrics";
    symptomDeptMap["difficulty breathing"] = "Internal Medicine";
    symptomDeptMap["chest pain"]           = "Cardiology";
    symptomDeptMap["unconscious"]          = "Emergency";
    symptomDeptMap["cardiac arrest"]       = "Emergency";
    symptomDeptMap["broken bone"]          = "Orthopedics";
    symptomDeptMap["severe headache"]      = "Neurology";
    symptomDeptMap["eye pain"]             = "Ophthalmology";

    // ---- BST: symptom → difficulty (1=easy, 4=critical) ----
    // Inserted in random order; BST self-organizes alphabetically
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "general fever",        1);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "severe stomach pain",  1);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "child with fever",     2);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "difficulty breathing", 2);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "chest pain",           3);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "unconscious",          3);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "cardiac arrest",       4);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "broken bone",          2);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "severe headache",      2);
    difficultyBSTRoot = bstInsert(difficultyBSTRoot, "eye pain",             1);
}


// ============================================================
// SECTION C: TIMESTAMP HELPER (internal use only)
// Returns current time as "HH:MM" string
// ============================================================
string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    // Format hours and minutes with leading zeros
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", ltm);
    return string(buffer);
}


// ============================================================
// FUNCTION 1: registerPatient()
// Creates a new Patient, assigns ID, maps symptom, starts history.
// Called by: nurse UI (Nulee's module)
// Returns: pointer to the new Patient
// ============================================================
Patient* registerPatient(string name, string symptom, int urgency) {

    // Make sure symptom maps are loaded
    // (safe to call multiple times — only fills if empty)
    if (symptomDeptMap.empty()) {
        initSymptomMaps();
    }

    // --- Create new Patient on the heap ---
    Patient* p = new Patient();

    // Auto-generate ID: P001, P002, ...
    // setw(3) and setfill('0') ensures leading zeros e.g. P001 not P1
    string idNum = to_string(patientCounter++);
    while (idNum.size() < 3) idNum = "0" + idNum; // pad to 3 digits
    p->id = "P" + idNum;

    p->name    = name;
    p->symptom = symptom;
    p->urgency = urgency;

    // Map symptom → department (Hash Table O(1))
    p->department = mapSymptomToDepartment(symptom);

    // Map symptom → difficulty (BST O(log n))
    p->difficulty = mapSymptomToDifficulty(symptom);

   // Create first history node directly
    HistoryNode* firstNode = new HistoryNode();
    firstNode->timestamp   = getCurrentTime();
    firstNode->event       = "Registered | symptom: " + symptom;
    firstNode->next        = nullptr;
    p->history             = firstNode;

// Add to table AFTER history is set
patientTable[p->id] = p;


    cout << "[INFO] Patient registered: " << p->id
         << " | " << p->name
         << " | dept: " << p->department
         << " | difficulty: " << p->difficulty << endl;

    return p;
}


// ============================================================
// FUNCTION 2: mapSymptomToDepartment()
// Uses Hash Table (unordered_map) — O(1) lookup
// Returns "unknown" if symptom not in table (graceful degradation)
// ============================================================
string mapSymptomToDepartment(string symptom) {
    if (symptomDeptMap.empty()) initSymptomMaps();

    // unordered_map.count() returns 1 if key exists, 0 if not
    if (symptomDeptMap.count(symptom)) {
        return symptomDeptMap[symptom];
    }
    // Symptom not found → return "unknown", do NOT crash
    return "unknown";
}


// ============================================================
// FUNCTION 3: mapSymptomToDifficulty()
// Uses BST — O(log n) search
// Returns 0 if symptom not found (0 = unknown difficulty)
// ============================================================
int mapSymptomToDifficulty(string symptom) {
    if (difficultyBSTRoot == nullptr) initSymptomMaps();
    return bstSearch(difficultyBSTRoot, symptom);
}


// ============================================================
// FUNCTION 4: addHistoryNode()
// Appends a new event to the END of patient's Linked List.
// Time complexity: O(n) to traverse to tail
//   (could be O(1) with tail pointer — acceptable for this scope)
// ============================================================
void addHistoryNode(string patientID, string event) {

    // Find the patient first
    Patient* p = findPatientByID(patientID);

    // If patient doesn't exist yet (first call during registration),
    // we handle this by checking patientTable directly
    if (p == nullptr) {
        // During registerPatient(), patient is not in table yet.
        // We need to find it via the table after insertion — skip for now.
        // This case is handled by registerPatient() calling addHistoryNode
        // before inserting into patientTable, so we create history manually.
        return;
    }

    // Create new history node
    HistoryNode* newNode = new HistoryNode();
    newNode->timestamp = getCurrentTime();
    newNode->event     = event;
    newNode->next      = nullptr;

    // If history list is empty, new node becomes the head
    if (p->history == nullptr) {
        p->history = newNode;
        return;
    }

    // Otherwise traverse to the tail and append
    HistoryNode* current = p->history;
    while (current->next != nullptr) {
        current = current->next;
    }
    current->next = newNode;
}


// ============================================================
// FUNCTION 5: pushStation()
// Push a station ID onto patient's stack.
// Called when patient enters a new station.
// Time complexity: O(1)
// ============================================================
void pushStation(string patientID, string stationID) {
    Patient* p = findPatientByID(patientID);
    if (p == nullptr) {
        cout << "[ERROR] pushStation: patient " << patientID << " not found." << endl;
        return;
    }
    p->stationStack.push(stationID);
    cout << "[INFO] " << patientID << " entered station: " << stationID << endl;
}


// ============================================================
// FUNCTION 6: popStation()
// Pop and return the most recent station from patient's stack.
// Called when patient leaves a station or needs to backtrack.
// Time complexity: O(1)
// Returns "empty" if stack is empty (no crash)
// ============================================================
string popStation(string patientID) {
    Patient* p = findPatientByID(patientID);
    if (p == nullptr) {
        cout << "[ERROR] popStation: patient " << patientID << " not found." << endl;
        return "error";
    }
    if (p->stationStack.empty()) {
        cout << "[WARN] popStation: " << patientID << " has no stations in stack." << endl;
        return "empty";
    }
    string topStation = p->stationStack.top(); // read top
    p->stationStack.pop();                      // remove top
    cout << "[INFO] " << patientID << " left station: " << topStation << endl;
    return topStation;
}


// ============================================================
// FUNCTION 7: findPatientByID()
// Searches patientTable (Hash Table) by ID — O(1)
// Returns nullptr if not found (no crash)
// ============================================================
Patient* findPatientByID(string id) {
    if (patientTable.count(id)) {
        return patientTable[id];
    }
    return nullptr; // not found
}


// ============================================================
// FUNCTION 8: findPatientBySymptom()
// Linear scan of all patients — O(n)
// Returns first patient whose symptom matches
// Returns nullptr if not found
// ============================================================
Patient* findPatientBySymptom(string symptom) {
    for (auto& entry : patientTable) {
        if (entry.second->symptom == symptom) {
            return entry.second;
        }
    }
    return nullptr; // not found
}


// ============================================================
// FUNCTION 9: deletePatient()
// Removes patient from Hash Table and frees all memory.
// Returns true if deleted, false if not found (no crash)
// Memory management: frees Linked List nodes one by one
// ============================================================
bool deletePatient(string id) {
    Patient* p = findPatientByID(id);
    if (p == nullptr) {
        cout << "[WARN] deletePatient: patient " << id << " not found." << endl;
        return false; // graceful — do not crash
    }

    // Free all Linked List (history) nodes
    HistoryNode* current = p->history;
    while (current != nullptr) {
        HistoryNode* next = current->next;
        delete current;
        current = next;
    }

    // Remove from Hash Table
    patientTable.erase(id);

    // Free the Patient object itself
    delete p;

    cout << "[INFO] Patient " << id << " deleted successfully." << endl;
    return true;
}


// ============================================================
// FUNCTION 10: displayPatientHistory()
// Traverses patient's Linked List and prints each event.
// Time complexity: O(n) where n = number of history events
// ============================================================
void displayPatientHistory(string patientID) {
    Patient* p = findPatientByID(patientID);
    if (p == nullptr) {
        cout << "[ERROR] displayPatientHistory: patient "
             << patientID << " not found." << endl;
        return;
    }

    cout << "========================================" << endl;
    cout << "History for: " << p->name
         << " (" << p->id << ")" << endl;
    cout << "Symptom   : " << p->symptom << endl;
    cout << "Department: " << p->department << endl;
    cout << "Difficulty: " << p->difficulty
         << " | Urgency: " << p->urgency << endl;
    cout << "----------------------------------------" << endl;

    // Traverse Linked List from head to tail
    HistoryNode* current = p->history;
    int step = 1;
    while (current != nullptr) {
        cout << " [" << current->timestamp << "] "
             << step << ". " << current->event << endl;
        current = current->next;
        step++;
    }

    cout << "========================================" << endl;
}

// ============================================================
// END OF FILE: patient.cpp
// ============================================================