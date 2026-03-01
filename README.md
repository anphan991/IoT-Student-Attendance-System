# 🎓 Smart IoT Student Attendance System

![Version](https://img.shields.io/badge/version-2.5.0-blue)
![Status](https://img.shields.io/badge/Status-Developing-yellow)
![Tech](https://img.shields.io/badge/Tech-ESP32%20%7C%20FastAPI%20%7C%20Hybrid%20DB-orange)

This project implements an automated student attendance system using **RFID** technology, the **ESP32** microcontroller, and a modern **Web Dashboard**. It provides real-time attendance tracking, automated eligibility calculations for exams, and a streamlined interface for classroom management.

---

## 🏗️ Project Structure

The project is organized into a decoupled architecture to separate hardware logic, data processing, and user interface:

```text
IoT_Attendance_Project/
├── hardware/             
│   ├── sketch.ino        
│   ├── diagram.json      
│   ├── rfid-rc522.chip.c
│   ├── rfid-rc522.chip.json      
│   ├── wokwi-project.txt
│   └── libraries.txt
├── backend/              
│   ├── main.py           
│   ├── Dockerfile        
│   ├── requirements.txt  
│   └── .dockerignore
├── web/                  
│   └── web.html        
├── docs/                 
└── README.md
```

## 🛠️ Tech Stack

### 📡 IoT & Hardware

* **Microcontroller:** ESP32 (Simulated via Wokwi).
* **Sensor:** RFID-RC522 for student identification.
* **Protocol:** RESTful API over HTTP/HTTPS.

### ☁️ Cloud Data (Hybrid Database)

* **Firebase:** Handles real-time attendance triggers.
* **Supabase:** Manages relational data, student profiles, and class lists.

### ⚙️ Backend

* **Framework:** Python (FastAPI).
* **Deployment:** Cloud-hosted on Hugging Face Spaces.

### 💻 Frontend (Web Dashboard)

* **Design:** Modern Glassmorphism with Plus Jakarta Sans typography.
* **Core Features:** * **Dark Mode:** Dynamic theme switching for better user experience.
* **Custom UI:** Replaced native alerts with smooth Toast notifications and Custom Modals.
* **Logic:** Automatic counting of students marked as "Ineligible for Exam" based on absence data.



---

## ✨ Key Features

* **Real-time Processing:** Immediate attendance logging upon RFID card tap.
* **Smart Statistics:** Instant overview of total students and attendance warnings directly on the dashboard.
* **Highly Responsive UI:** Smooth animations, glass-like panels, and full mobile compatibility.
* **Role-based Management:** Secure login and class-specific data filtering.
* **Professional Reporting:** One-click export of attendance data to Excel format.

---

## 🚀 Getting Started

### 1. Hardware Simulation (Wokwi)

1. Access Wokwi and import the `hardware/diagram.json` file.
2. Set **WiFi SSID** to `Wokwi-GUEST`.
3. Update the **BASE_URL** in `sketch.ino` to point to your live API.

### 2. Backend Deployment

1. Deploy the `backend/` folder to **Hugging Face Spaces**.
2. Configure environment variables for Firebase/Supabase in the **Settings** tab.
3. **Default Endpoint:** `https://danganhle0623-iot.hf.space`

### 3. Web Dashboard

1. Open `web/index.html` (or `web/web.html`) in any modern web browser.
2. Log in with your administrator credentials to start managing classes.

---

## 📝 Development Notes

* **Attendance Warnings:** The system automatically identifies students with "Ineligible" status based on API response strings.
* **Theme Persistence:** Dark Mode settings are saved to `localStorage` to remain active after page reloads.
* **Cold Start:** Note that the Hugging Face backend may require a few seconds to wake up after a period of inactivity.
* **Simulation Availability (Wokwi):** To ensure data accuracy and system stability, the student list retrieval on the Wokwi demo is only active during the following school shifts:

* **Morning Shift:** 07:30 AM – 11:00 AM.

* **Afternoon Shift:** 12:45 PM – 05:00 PM.

* **Note:** Outside of these windows, the student list will be unavailable or show as empty.

---

## 🔗 Quick Access & Demo

You can explore the live simulation and hosted dashboard through the links below:

* 📺 **Video Demonstration:** [Watch the Project Demo](https://www.google.com/search?q=%23) — *A full walkthrough of the hardware scanning process and real-time dashboard updates.*
* 🔌 **Wokwi Online Simulation:** [Launch ESP32 Simulation](https://wokwi.com/projects/457191240564457473) — *Interact with the virtual ESP32 and RFID logic directly in your browser | The demo only fetches the student list during active hours (07:30-11:00 and 12:45-17:00).*
* 🚀 **Live Web Dashboard:** [Access IoT Attendance Portal](https://danganhle0623-iot.hf.space) — *The production-ready interface hosted on Hugging Face Spaces.(User name: admin, Password: 123456).*

---

## 🤝 Project Contributors

This project is a collaborative effort by the following individuals:

* **Nguyễn Đức Học (hoc0g)** — Project Lead & API Developer
   
> 🌐 Contact: [GitHub: *ndhoc*](https://github.com/ndhoc) / [Email: *24162039@student.hcmute.edu.vn*](24162039@student.hcmute.edu.vn)


* **Lê Đặng Hoàng Anh (HAgudboi)** — Backend & System Architect
> 🌐 Contact: [GitHub: *ledanghoanganh*](https://github.com/ledanghoanganh) / [Email: *leanhhoang145@gmail.com*](leanhhoang145@gmail.com)


* **Trần Công Khánh (NCK)** — Project Coordinator & Flex Developer
> 🌐 Contact: [GitHub: *TranKhanh20*](https://github.com/TranKhanh206) / [Email: *trancongkhanh2006.tn@gmail.com*](trancongkhanh2006.tn@gmail.com)


* **Nguyễn Bá Nam (sepNAM)** — Project Manager & Documentation
> 🌐 Contact: [GitHub: *nguyenbanam272-cyber*](https://github.com/nguyenbanam272-cyber) / [Email: *nguyenbanam272@gmail.com*](nguyenbanam272@gmail.com)


* **Phan Khánh An (ap991)** — Firmware & Frontend
> 🌐 Contact: [GitHub: *anphan991*](https://github.com/anphan991) / [Email: *an0915129080@gmail.com*](an0915129080@gmail.com)



---

## 🚀 Upcoming Updates (Coming Soon)
* **Update Docs**
* **Real-time WebSockets:** To provide instant dashboard updates without refreshing.
* **Security Hardening:** Implementing advanced encryption for RFID data transmission.
* **Mobile Companion App:** A dedicated application for students to check their own attendance history.

---


Developed with ❤️ 




