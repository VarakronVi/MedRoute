#ifndef DOCTOR_H
#define DOCTOR_H

#include <string>
#include <vector>
using namespace std;

struct Doctor {
    string id;
    string name;
    int level;
    string department;
    int current_cases;
    int max_threshold;
};

struct DoctorBSTNode {
    Doctor* doc;
    DoctorBSTNode* left;
    DoctorBSTNode* right;
};

void loadDoctorsFromCSV(const string& filename);
void clearAllDoctors();

Doctor* findDoctorByID(const string& id);
Doctor* findAvailableDoctor(int requiredLevel, const string& dept);
vector<Doctor*> findDoctorsByLevel(int level);
vector<Doctor*> findDoctorsByDepartment(const string& dept);

bool addDoctor(const Doctor& d);
bool deleteDoctor(const string& id);

bool canTakeCase(const string& doctorID);
void incrementCase(const string& doctorID);
void decrementCase(const string& doctorID);
int getRemainingCapacity(const string& doctorID);

void displayAllDoctors();
void displayDoctorByLevel(int level);
void displayDoctorByDepartment(const string& dept);
void displayDoctorDetail(const string& id);

int getDoctorCount();
int getThresholdForLevel(int level);

#endif