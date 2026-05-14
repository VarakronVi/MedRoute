#include "routing.h"
#include <iostream>

using namespace std;

int main() {
    cout << "=================================================================" << endl;
    cout << "          Welcome to MedRoute Hospital Management System         " << endl;
    cout << "=================================================================\n" << endl;

    // 1. Initialize station module (Module 2)
    initStations();

    // 2. Load doctors from CSV file (Module 1)
    loadDoctorsFromCSV("data/doctors.csv");

    // 3. Enter main menu for patient routing and emergency UI (Module 4)
    mainMenu();

    return 0;
}
