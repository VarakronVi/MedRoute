
// ============================================================
// FILE: patient.h
// PROJECT: MedRoute — Hospital Queue Management System
// MODULE: Patient & Symptom Module (Omsin's part)
// ============================================================
// HOW TO USE (for teammates):
//   #include "patient.h"
//   That's it. All structs and functions are available.
// ============================================================

#ifndef PATIENT_H   // Guard: prevents this file from being included twice
#define PATIENT_H

#include <string>
#include <stack>
#include <unordered_map>
#include <iostream>
using namespace std;


// ============================================================
// SECTION 1: BST NODE
// Used by: mapSymptomToDifficulty()
// Why BST: supports O(log n) search + in-order traversal
//          so we can list symptoms sorted by difficulty level
// ============================================================
struct BSTNode {
    string symptom;    // key  — e.g. "chest pain"
    int    difficulty; // value — 1 (easy) to 4 (critical)
    BSTNode* left;     // subtree with alphabetically smaller symptoms
    BSTNode* right;    // subtree with alphabetically larger symptoms
};


// ============================================================
// SECTION 2: HISTORY NODE
// Used by: addHistoryNode(), displayPatientHistory()
// Why Linked List: patient history length is unknown in advance;
//                 linked list grows dynamically, no wasted memory
// ============================================================
struct HistoryNode {
    string timestamp;    // e.g. "09:00"
    string event;        // e.g. "Registered | symptom: chest pain"
    HistoryNode* next;   // pointer to next event (nullptr = end of list)
};


// ============================================================
// SECTION 3: PATIENT STRUCT
// The main entity that flows through the entire system.
// Other modules (Routing by Nulee) will receive Patient* pointers.
// ============================================================
struct Patient {
    string id;           // Auto-generated: P001, P002, ...
    string name;         // Patient's name
    string symptom;      // Raw symptom entered by nurse
    string department;   // Mapped department (from Hash Table)
    int    difficulty;   // Case difficulty 1-4  (from BST)
    int    urgency;      // 1=normal(green), 2=urgent(yellow), 3=critical(red)

    HistoryNode* history;        // Head of Linked List — stores all events
    stack<string> stationStack;  // Stack — tracks stations patient has visited
                                 // LIFO: top = most recent station
};


// ============================================================
// SECTION 4: FUNCTION SIGNATURES
// All 10 functions that patient.cpp implements.
// Teammates call these functions; they don't need to know the internals.
// ============================================================

// ------ 4.1 Registration ------
// Creates a new Patient, generates ID, maps symptom, starts history list.
// Returns a pointer to the newly created Patient.
// urgency: 1=normal, 2=urgent, 3=critical
Patient* registerPatient(string name, string symptom, int urgency);


// ------ 4.2 Symptom Mapping ------
// Uses Hash Table (unordered_map) to find department — O(1)
// Returns "unknown" if symptom not found (no crash)
string mapSymptomToDepartment(string symptom);

// Uses BST to find difficulty level 1-4 — O(log n)
// Returns 0 if symptom not found (no crash)
int mapSymptomToDifficulty(string symptom);


// ------ 4.3 Patient History (Linked List) ------
// Appends a new event node to the tail of patient's history list.
// Automatically records current timestamp.
void addHistoryNode(string patientID, string event);


// ------ 4.4 Station Tracking (Stack) ------
// Push: patient enters a new station — push stationID onto stack
void pushStation(string patientID, string stationID);

// Pop: remove and return the most recent station — O(1)
// Returns "empty" if stack is empty (no crash)
string popStation(string patientID);


// ------ 4.5 Search / Delete / Display ------
// Search by patient ID — O(1) using patient hash table
// Returns nullptr if not found
Patient* findPatientByID(string id);

// Search by symptom — returns first matching patient
// Returns nullptr if not found
Patient* findPatientBySymptom(string symptom);

// Delete patient by ID
// Returns true if deleted, false if patient not found (no crash)
bool deletePatient(string id);

// Print full history of a patient (traverse Linked List)
void displayPatientHistory(string patientID);


#endif // PATIENT_H
// ============================================================
// END OF FILE: patient.h
// ============================================================