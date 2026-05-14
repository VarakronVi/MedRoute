# 🏥 MedRoute: Hospital Queue & Routing Management System

**MedRoute** is an advanced smart hospital queueing, load-balancing, and patient routing system built in C++. Developed as a collaborative academic project for the **Data Structures & Algorithms** course, it demonstrates practical implementation of foundational computer science data structures to solve real-world hospital operational bottlenecks.

---

## 📁 Professional Repository Structure

To adhere to industry software engineering best practices, this repository is organized into standard directory structures:

```text
MedRoute-Project/
├── src/                      # ซอร์สโค้ดหลักทั้งหมด (Core Source Code)
│   ├── main.cpp              # Application entry point & menu loops
│   ├── doctor.h / .cpp       # Module 1: Doctor database & Min-Heap load balancing
│   ├── station.h / .cpp      # Module 2: Station routing & FIFO queues
│   ├── patient.h / .cpp      # Module 3: Patient registration, BST mapping & history linked lists
│   └── routing.h / .cpp      # Module 4: Graph BFS cascading failure & Emergency priority queues
├── data/                     # โฟลเดอร์จัดเก็บฐานข้อมูล (Data Layer)
│   └── doctors.csv           # Initial master list of hospital doctors
├── docs/                     # เอกสารประกอบโปรเจกต์ (Documentation & Guides)
│   ├── MedRoute_DataStructure_Guide.md  # โพยสรุป Data Structure สำหรับตอบคำถามอาจารย์
│   └── MedRoute_Slide_Outline.md        # โครงสร้างเนื้อหา Presentation Slides (10 นาที)
└── README.md                 # คู่มือแนะนำโปรเจกต์ (Project Overview)
```

---

## 🚀 How to Compile & Run

Open your terminal, navigate to the root directory of this repository, and execute the following commands:

### 1. Compile the Project
Using `g++` with C++17 standards to output an executable named `MedRoute`:
```bash
g++ -std=c++17 src/*.cpp -o MedRoute
```

### 2. Run the Application
Execute the compiled binary from the root directory so it correctly locates `data/doctors.csv`:
```bash
./MedRoute
```

---

## 🛠️ Data Structures Implemented

- **Hash Tables (`unordered_map`)**: $O(1)$ constant time lookup for doctors, stations, and symptom-to-department routing.
- **Binary Search Trees (BST)**: $O(\log n)$ ordered storage for professional hierarchies and symptom difficulty rankings.
- **Min-Heap (`priority_queue`)**: Efficiently identifies the doctor with the lowest active workload to achieve load balancing.
- **Max-Heap Priority Queue**: Ensures incoming emergency override cases preempt standard queues.
- **Singly Linked List**: Dynamically appends patient event histories without arbitrary storage caps.
- **Stacks (`stack`)**: $O(1)$ LIFO operation used to track visited stations and preserve pre-emergency baseline configurations during critical interventions.
- **Graph BFS**: Discovers neighboring backup departments through iterative graph traversal to gracefully resolve departmental full-capacity blockades.

---
*Developed with ❤️ for Advanced C++ Data Structures Engineering.*
