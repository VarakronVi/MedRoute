#include "station.h"
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

static unordered_map<string, Station*> stationHash;
static vector<string> stationOrder;

void initStations() {
    clearAllStations();

    Station* s01 = new Station;
    s01->id = "S01"; s01->name = "Blood Pressure";
    s01->duration = 2; s01->available = true; s01->current_patient = "";

    Station* s02 = new Station;
    s02->id = "S02"; s02->name = "Weight";
    s02->duration = 1; s02->available = true; s02->current_patient = "";

    Station* s03 = new Station;
    s03->id = "S03"; s03->name = "Height";
    s03->duration = 1; s03->available = true; s03->current_patient = "";

    Station* s04 = new Station;
    s04->id = "S04"; s04->name = "Temperature";
    s04->duration = 1; s04->available = true; s04->current_patient = "";

    Station* s05 = new Station;
    s05->id = "S05"; s05->name = "History Intake";
    s05->duration = 5; s05->available = true; s05->current_patient = "";

    stationHash["S01"] = s01;
    stationHash["S02"] = s02;
    stationHash["S03"] = s03;
    stationHash["S04"] = s04;
    stationHash["S05"] = s05;

    stationOrder = {"S01", "S02", "S03", "S04", "S05"};

    cout << "[OK] Initialized " << stationOrder.size() << " stations" << endl;
}

void clearAllStations() {
    for (auto& p : stationHash) delete p.second;
    stationHash.clear();
    stationOrder.clear();
}

Station* findStationByID(const string& stationID) {
    auto it = stationHash.find(stationID);
    return (it != stationHash.end()) ? it->second : nullptr;
}

Station* findFreeStation() {
    for (const string& id : stationOrder) {
        Station* s = stationHash[id];
        if (s->available) return s;
    }
    return nullptr;
}

Station* findEarliestFreeStation() {
    Station* best = nullptr;
    int bestWaitTime = -1;

    for (const string& id : stationOrder) {
        Station* s = stationHash[id];
        int waitTime;
        if (s->available) {
            waitTime = 0;
        } else {
            waitTime = (s->waiting.size() + 1) * s->duration;
        }
        if (best == nullptr || waitTime < bestWaitTime) {
            best = s;
            bestWaitTime = waitTime;
        }
    }
    return best;
}

bool isStationFree(const string& stationID) {
    Station* s = findStationByID(stationID);
    return s != nullptr && s->available;
}

void addPatientToStation(const string& stationID, const string& patientID) {
    Station* s = findStationByID(stationID);
    if (s == nullptr) {
        cout << "[ERROR] Station " << stationID << " not found" << endl;
        return;
    }
    if (s->available) {
        s->current_patient = patientID;
        s->available = false;
        cout << "[OK] " << patientID << " is now at " << s->name << endl;
    } else {
        s->waiting.push(patientID);
        cout << "[OK] " << patientID << " queued at " << s->name
             << " (position " << s->waiting.size() << ")" << endl;
    }
}

void removePatientFromStation(const string& stationID) {
    Station* s = findStationByID(stationID);
    if (s == nullptr) {
        cout << "[ERROR] Station " << stationID << " not found" << endl;
        return;
    }
    if (s->current_patient.empty()) {
        cout << "[INFO] Station " << s->name << " has no current patient" << endl;
        return;
    }

    string finished = s->current_patient;
    if (!s->waiting.empty()) {
        s->current_patient = s->waiting.front();
        s->waiting.pop();
        cout << "[OK] " << finished << " finished. Next: " << s->current_patient << endl;
    } else {
        s->current_patient = "";
        s->available = true;
        cout << "[OK] " << finished << " finished. " << s->name << " is now free" << endl;
    }
}

bool removePatientFromQueue(const string& stationID, const string& patientID) {
    Station* s = findStationByID(stationID);
    if (s == nullptr) return false;

    queue<string> temp;
    bool found = false;
    while (!s->waiting.empty()) {
        string p = s->waiting.front();
        s->waiting.pop();
        if (p == patientID) {
            found = true;
        } else {
            temp.push(p);
        }
    }
    s->waiting = temp;
    return found;
}

int estimateWaitTime(const string& stationID) {
    Station* s = findStationByID(stationID);
    if (s == nullptr) return -1;
    if (s->available) return 0;
    return (s->waiting.size() + 1) * s->duration;
}

int getQueueSize(const string& stationID) {
    Station* s = findStationByID(stationID);
    if (s == nullptr) return -1;
    return s->waiting.size();
}

int getTotalPatientsInSystem() {
    int total = 0;
    for (auto& p : stationHash) {
        Station* s = p.second;
        if (!s->current_patient.empty()) total++;
        total += s->waiting.size();
    }
    return total;
}

void displayAllStations() {
    cout << "---------------------------------------------" << endl;
    cout << "  Station Dashboard" << endl;
    cout << "---------------------------------------------" << endl;
    for (const string& id : stationOrder) {
        Station* s = stationHash[id];
        cout << "  " << s->id << "  " << s->name;
        int pad = 20 - s->name.length();
        for (int i = 0; i < pad; i++) cout << " ";
        if (s->available) {
            cout << "[ FREE ]";
        } else {
            cout << "[ BUSY: " << s->current_patient << " ]";
        }
        cout << "  Queue: " << s->waiting.size() << endl;
    }
    cout << "---------------------------------------------" << endl;
    cout << "  Total patients in system: " << getTotalPatientsInSystem() << endl;
    cout << "---------------------------------------------" << endl;
}

void displayStationQueue(const string& stationID) {
    Station* s = findStationByID(stationID);
    cout << "---------------------------------------------" << endl;
    if (s == nullptr) {
        cout << "  Station " << stationID << " not found" << endl;
        cout << "---------------------------------------------" << endl;
        return;
    }
    cout << "  Queue at " << s->name << " (" << s->id << ")" << endl;
    cout << "---------------------------------------------" << endl;

    if (!s->current_patient.empty()) {
        cout << "  [Now serving] " << s->current_patient << endl;
    }

    if (s->waiting.empty()) {
        cout << "  (queue is empty)" << endl;
    } else {
        queue<string> temp = s->waiting;
        int pos = 1;
        while (!temp.empty()) {
            cout << "  [" << pos << "] " << temp.front() << endl;
            temp.pop();
            pos++;
        }
    }
    cout << "---------------------------------------------" << endl;
    cout << "  Estimated wait: " << estimateWaitTime(stationID) << " min" << endl;
    cout << "---------------------------------------------" << endl;
}

void displayStationDetail(const string& stationID) {
    Station* s = findStationByID(stationID);
    cout << "---------------------------------------------" << endl;
    if (s == nullptr) {
        cout << "  Station " << stationID << " not found" << endl;
    } else {
        cout << "  Station Detail" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "  ID         : " << s->id << endl;
        cout << "  Name       : " << s->name << endl;
        cout << "  Duration   : " << s->duration << " min per patient" << endl;
        cout << "  Status     : " << (s->available ? "FREE" : "BUSY") << endl;
        if (!s->current_patient.empty()) {
            cout << "  Current    : " << s->current_patient << endl;
        }
        cout << "  Queue size : " << s->waiting.size() << endl;
        cout << "  Wait time  : " << estimateWaitTime(stationID) << " min" << endl;
    }
    cout << "---------------------------------------------" << endl;
}

int getStationCount() {
    return stationHash.size();
}

vector<string> getAllStationIDs() {
    return stationOrder;
}