#include "doctor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <algorithm>
using namespace std;

static unordered_map<string, Doctor*> doctorHash;
static DoctorBSTNode* bstRoot = nullptr;
static const int thresholdByLevel[] = {0, 2, 3, 4, 5};

int getThresholdForLevel(int level) {
    if (level < 1 || level > 4) return 0;
    return thresholdByLevel[level];
}

static DoctorBSTNode* bstInsert(DoctorBSTNode* root, Doctor* d) {
    if (root == nullptr) {
        DoctorBSTNode* node = new DoctorBSTNode;
        node->doc = d;
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    if (d->level < root->doc->level) {
        root->left = bstInsert(root->left, d);
    } else {
        root->right = bstInsert(root->right, d);
    }
    return root;
}

static void bstInorder(DoctorBSTNode* root) {
    if (root == nullptr) return;
    bstInorder(root->left);
    Doctor* d = root->doc;
    cout << "  " << d->id << "  " << d->name
         << "  A" << d->level
         << "  " << d->department
         << "  " << d->current_cases << "/" << d->max_threshold
         << " cases" << endl;
    bstInorder(root->right);
}

static void bstFindByLevel(DoctorBSTNode* root, int level, vector<Doctor*>& result) {
    if (root == nullptr) return;
    if (root->doc->level == level) {
        result.push_back(root->doc);
    }
    if (level <= root->doc->level) {
        bstFindByLevel(root->left, level, result);
    }
    if (level >= root->doc->level) {
        bstFindByLevel(root->right, level, result);
    }
}

static DoctorBSTNode* bstFindMin(DoctorBSTNode* root) {
    if (root == nullptr) return nullptr;
    while (root->left != nullptr) root = root->left;
    return root;
}

static DoctorBSTNode* bstDelete(DoctorBSTNode* root, const string& id) {
    if (root == nullptr) return nullptr;
    if (root->doc->id == id) {
        if (root->left == nullptr && root->right == nullptr) {
            delete root;
            return nullptr;
        }
        if (root->left == nullptr) {
            DoctorBSTNode* temp = root->right;
            delete root;
            return temp;
        }
        if (root->right == nullptr) {
            DoctorBSTNode* temp = root->left;
            delete root;
            return temp;
        }
        DoctorBSTNode* succ = bstFindMin(root->right);
        root->doc = succ->doc;
        root->right = bstDelete(root->right, succ->doc->id);
        return root;
    }
    root->left = bstDelete(root->left, id);
    root->right = bstDelete(root->right, id);
    return root;
}

static void bstClear(DoctorBSTNode* root) {
    if (root == nullptr) return;
    bstClear(root->left);
    bstClear(root->right);
    delete root;
}

struct DoctorHeapCompare {
    bool operator()(Doctor* a, Doctor* b) {
        if (a->level != b->level) {
            return a->level > b->level;
        }
        return a->current_cases > b->current_cases;
    }
};

void loadDoctorsFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] Cannot open file: " << filename << endl;
        return;
    }

    string line;
    getline(file, line);

    int loaded = 0;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, name, levelStr, dept;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, levelStr, ',');
        getline(ss, dept, ',');

        if (id.empty() || name.empty() || levelStr.empty()) continue;

        Doctor* d = new Doctor;
        d->id = id;
        d->name = name;
        d->level = stoi(levelStr);
        d->department = dept;
        d->current_cases = 0;
        d->max_threshold = getThresholdForLevel(d->level);

        doctorHash[id] = d;
        bstRoot = bstInsert(bstRoot, d);
        loaded++;
    }
    file.close();
    cout << "[OK] Loaded " << loaded << " doctors from " << filename << endl;
}

void clearAllDoctors() {
    bstClear(bstRoot);
    bstRoot = nullptr;
    for (auto& p : doctorHash) delete p.second;
    doctorHash.clear();
}

Doctor* findDoctorByID(const string& id) {
    auto it = doctorHash.find(id);
    return (it != doctorHash.end()) ? it->second : nullptr;
}

Doctor* findAvailableDoctor(int requiredLevel, const string& dept) {
    priority_queue<Doctor*, vector<Doctor*>, DoctorHeapCompare> heap;
    for (auto& p : doctorHash) {
        Doctor* d = p.second;
        if (d->level < requiredLevel) continue;
        if (d->department != dept) continue;
        if (d->current_cases >= d->max_threshold) continue;
        heap.push(d);
    }
    return heap.empty() ? nullptr : heap.top();
}

vector<Doctor*> findDoctorsByLevel(int level) {
    vector<Doctor*> result;
    bstFindByLevel(bstRoot, level, result);
    return result;
}

vector<Doctor*> findDoctorsByDepartment(const string& dept) {
    vector<Doctor*> result;
    for (auto& p : doctorHash) {
        if (p.second->department == dept) {
            result.push_back(p.second);
        }
    }
    return result;
}

bool addDoctor(const Doctor& d) {
    if (doctorHash.find(d.id) != doctorHash.end()) return false;
    Doctor* newDoc = new Doctor(d);
    if (newDoc->max_threshold == 0) {
        newDoc->max_threshold = getThresholdForLevel(newDoc->level);
    }
    doctorHash[newDoc->id] = newDoc;
    bstRoot = bstInsert(bstRoot, newDoc);
    return true;
}

bool deleteDoctor(const string& id) {
    auto it = doctorHash.find(id);
    if (it == doctorHash.end()) return false;
    bstRoot = bstDelete(bstRoot, id);
    Doctor* d = it->second;
    doctorHash.erase(it);
    delete d;
    return true;
}

bool canTakeCase(const string& doctorID) {
    Doctor* d = findDoctorByID(doctorID);
    if (d == nullptr) return false;
    return d->current_cases < d->max_threshold;
}

void incrementCase(const string& doctorID) {
    Doctor* d = findDoctorByID(doctorID);
    if (d != nullptr && d->current_cases < d->max_threshold) {
        d->current_cases++;
    }
}

void decrementCase(const string& doctorID) {
    Doctor* d = findDoctorByID(doctorID);
    if (d != nullptr && d->current_cases > 0) {
        d->current_cases--;
    }
}

int getRemainingCapacity(const string& doctorID) {
    Doctor* d = findDoctorByID(doctorID);
    if (d == nullptr) return 0;
    return d->max_threshold - d->current_cases;
}

void displayAllDoctors() {
    cout << "---------------------------------------------" << endl;
    cout << "  All Doctors (sorted by level via BST)" << endl;
    cout << "---------------------------------------------" << endl;
    if (bstRoot == nullptr) {
        cout << "  (no doctors loaded)" << endl;
    } else {
        bstInorder(bstRoot);
    }
    cout << "---------------------------------------------" << endl;
    cout << "  Total: " << doctorHash.size() << " doctors" << endl;
    cout << "---------------------------------------------" << endl;
}

void displayDoctorByLevel(int level) {
    cout << "---------------------------------------------" << endl;
    cout << "  Doctors at Level A" << level << endl;
    cout << "---------------------------------------------" << endl;
    vector<Doctor*> docs = findDoctorsByLevel(level);
    if (docs.empty()) {
        cout << "  (no doctors at this level)" << endl;
    } else {
        for (Doctor* d : docs) {
            cout << "  " << d->id << "  " << d->name
                 << "  " << d->department
                 << "  " << d->current_cases << "/" << d->max_threshold << endl;
        }
    }
    cout << "---------------------------------------------" << endl;
}

void displayDoctorByDepartment(const string& dept) {
    cout << "---------------------------------------------" << endl;
    cout << "  Doctors in " << dept << " department" << endl;
    cout << "---------------------------------------------" << endl;
    vector<Doctor*> docs = findDoctorsByDepartment(dept);
    if (docs.empty()) {
        cout << "  (no doctors in this department)" << endl;
    } else {
        for (Doctor* d : docs) {
            cout << "  " << d->id << "  " << d->name
                 << "  A" << d->level
                 << "  " << d->current_cases << "/" << d->max_threshold << endl;
        }
    }
    cout << "---------------------------------------------" << endl;
}

void displayDoctorDetail(const string& id) {
    Doctor* d = findDoctorByID(id);
    cout << "---------------------------------------------" << endl;
    if (d == nullptr) {
        cout << "  Doctor with ID " << id << " not found" << endl;
    } else {
        cout << "  Doctor Detail" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "  ID         : " << d->id << endl;
        cout << "  Name       : " << d->name << endl;
        cout << "  Level      : A" << d->level << endl;
        cout << "  Department : " << d->department << endl;
        cout << "  Cases      : " << d->current_cases << "/" << d->max_threshold << endl;
        cout << "  Available  : " << (canTakeCase(id) ? "YES" : "NO (full)") << endl;
    }
    cout << "---------------------------------------------" << endl;
}

int getDoctorCount() {
    return doctorHash.size();
}