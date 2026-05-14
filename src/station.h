#ifndef STATION_H
#define STATION_H

#include <string>
#include <queue>
#include <vector>
using namespace std;

struct Station {
    string id;
    string name;
    int duration;
    bool available;
    string current_patient;
    queue<string> waiting;
};

void initStations();
void clearAllStations();

Station* findStationByID(const string& stationID);
Station* findFreeStation();
Station* findEarliestFreeStation();
bool isStationFree(const string& stationID);

void addPatientToStation(const string& stationID, const string& patientID);
void removePatientFromStation(const string& stationID);
bool removePatientFromQueue(const string& stationID, const string& patientID);

int estimateWaitTime(const string& stationID);
int getQueueSize(const string& stationID);
int getTotalPatientsInSystem();

void displayAllStations();
void displayStationQueue(const string& stationID);
void displayStationDetail(const string& stationID);

int getStationCount();
vector<string> getAllStationIDs();

#endif