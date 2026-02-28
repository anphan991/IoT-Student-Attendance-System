🎓 Smart IoT Student Attendance System

>This project implements an automated student attendance system using RFID technology, the ESP32 microcontroller, and a modern Web Dashboard. It provides real-time attendance tracking, automated eligibility calculations for exams, and a streamlined interface for classroom management.
________________________________________
🏗️ Project Structure

>The project is organized into a decoupled architecture to separate hardware logic, data processing, and user interface:

>IoT_Attendance_Project/

>├── hardware/             
>│   ├── sketch.ino        
>│   └── diagram.json      
>│   └── rfid-rc522.chip.c
>
>│   └── rfid-rc522.chip.json
>   
>│   └── wokwi-project.txt
>
>│   └── libraries.txt
>
>├── backend/              
>│   ├── main.py           
>│   ├── Dockerfile        
>│   ├── requirements.txt  
>│   └── .dockerignore
>├── web/                  
>│   └── web.html        
>├── docs/                 
>└── README.md
________________________________________
🛠️ Tech Stack

📡 IoT & Hardware

>•	Microcontroller: ESP32 (Simulated via Wokwi).

>•	Sensor: RFID-RC522 for student identification.

>•	Protocol: RESTful API over HTTP/HTTPS.

☁️ Cloud Data (Hybrid Database)

>•	Firebase: Handles real-time attendance triggers.

>•	Supabase: Manages relational data, student profiles, and class lists.

⚙️ Backend
>•	Framework: Python (FastAPI).

>•	Deployment: Cloud-hosted on Hugging Face Spaces.

💻 Frontend (Web Dashboard)
>•	Design: Modern Glassmorphism with Plus Jakarta Sans typography.

>•	Core Features: * Dark Mode: Dynamic theme switching for better user experience.

>>o	Custom UI: Replaced native alerts with smooth Toast notifications and Custom Modals.

>>o	Logic: Automatic counting of students marked as "Ineligible for Exam" based on absence data.
________________________________________
✨ Key Features
>•	Real-time Processing: Immediate attendance logging upon RFID card tap.

>•	Smart Statistics: Instant overview of total students and attendance warnings directly on the dashboard.

>•	Highly Responsive UI: Smooth animations, glass-like panels, and full mobile compatibility.

>•	Role-based Management: Secure login and class-specific data filtering.

>•	Professional Reporting: One-click export of attendance data to Excel format.
________________________________________
🚀 Getting Started
1. Hardware Simulation (Wokwi)
   
>Access Wokwi and import the hardware/diagram.json file.

>Set WiFi SSID to Wokwi-GUEST.

>Update the BASE_URL in sketch.ino to point to your live API.
   
2. Backend Deployment
   
>Deploy the backend/ folder to Hugging Face Spaces.

>Configure environment variables for Firebase/Supabase in the Settings tab.

>Default Endpoint: https://danganhle0623-iot.hf.space.
   
3. Web Dashboard
>Open web/index.html in any modern web browser.

>Log in with your administrator credentials to start managing classes.
________________________________________
📝 Development Notes
>•	Attendance Warnings: The system automatically identifies students with "Ineligible" status based on API response strings.

>•	Theme Persistence: Dark Mode settings are saved to localStorage to remain active after page reloads.

>•	Cold Start: Note that the Hugging Face backend may require a few seconds to wake up after a period of inactivity.
________________________________________
🔗 Quick Access & Demo
You can explore the live simulation and hosted dashboard through the links below:

>•	📺 Video Demonstration: (URL Update Soon) — A full walkthrough of the hardware scanning process and real-time dashboard updates.

>•	🔌 Wokwi Online Simulation: (URL Update Soon) — Interact with the virtual ESP32 and RFID logic directly in your browser.

>•	🚀 Live Web Dashboard: (URL Update Soon) — The production-ready interface hosted on Hugging Face Spaces.
________________________________________
🤝 In Partnership

>This project is a collaborative effort by the following individuals:

>•	Nguyễn Đức Học (ndhoc) - Project Lead & API Developer

>>Contact:

>•	Lê Đặng Hoàng Anh (HAgudboi) - Backend & System Architect

>>Contact:

>•	Trần Công Khánh (NCK) - Project Coordinator & Flex Developer

>>Contact:

>•	Nguyễn Bá Nam (sepNAM) - Project Manager & Documentation

>>Contact:

>•	Phan Khánh An (ap991) – Firmware  & Frontend

>>Contact:

________________________________________
🚀 Upcoming Updates (Coming Soon)
