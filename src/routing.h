#ifndef ROUTING_H
#define ROUTING_H

#include "doctor.h"
#include "station.h"
#include "patient.h"
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>

using namespace std;

// ============================================================================
// MODULE 4: Routing & Emergency Module
// ============================================================================

// Maps full department name from Module 3 to short lowercase name in CSV
string standardizeDepartmentName(string rawDept);

// Initializes department adjacency graph for BFS cascading failure search
void initRoutingGraph();

// 1. Graph matching Case <-> Doctor (BFS to find suitable doctor)
Doctor* assignDoctorToCase(Patient* p);

// 2. Cascading failure: BFS to find backup department when primary is full
string findBackupDepartment(string fullDept);

// 3. Priority Queue for emergency override (Button E)
void localOverride(Patient* urgentPatient);
void processNextOverride(); // Process the highest priority override case

// 4. Stack save state for critical pause (Button C)
void criticalStop(Patient* criticalPatient);
void resumeCriticalStop(); // Resume paused case from stack

// 5. Stack save state for MCI mode (Button M)
void enterMCIMode();
void exitMCIMode();

// 6. Main menu (CLI dashboard) & UI
void mainMenu();
void displayDashboard();

#endif
