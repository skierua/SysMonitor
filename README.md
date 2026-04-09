# SysMonitor 🖥️

**SysMonitor** is a cross-platform system monitoring tool developed as a pet project. It provides real-time insights into system processes and resource usage across different operating systems.

## 🖼️ UI Preview
![App Screenshot](https://placeholder.com)
*Visualizing process list and RAM usage charts.*
`
---

## 🚀 Platform Status


| OS | Version/Environment | Status | Notes |
| :--- | :--- | :--- | :--- |
| **macOS** | Tahoe | ✅ | Ready to use. |
| **Windows** | Win10 | ✅ | Ready (MSVC). MinGW 13.1.0 is currently unstable. |
| **Linux** | — | ⚠️ | Done but Not tested yet. |

---
## Architecture Highlights
*   **Zero-overhead Abstraction:** Uses **CRTP** (Curiously Recurring Template Pattern) for static polymorphism, ensuring maximum performance during high-frequency system polling.
---
## ✨ Features

The application provides detailed monitoring of running processes (within permitted access levels):

*   **Process Insights:**
    *   Process ID (PID) and Executable Name.
    *   Full system path to the executable.
    *   Real-time RAM usage.
    *   Open/Active thread count.
    *   Process startup time.
*   **Process Management:** Terminate processes directly from the UI using `Shift + Left Click`.
*   **Visual Analytics:** Real-time chart showing the average RAM usage over the last minute.

---

## 🛠 Tech Stack

### Core Requirements
*   **Framework:** Qt 6.10.0 for Desktop
*   **Build System:** CMake 3.27.7 (Qt)

### Compiler Specifications
*   **macOS:** Clang version 17.0.0 (clang-1700.4.4.1)
*   **Windows:** 
    *   **MSVC 2022 64-bit** (Recommended)
    *   MinGW 13.1.0 64-bit (Experimental/Unstable)
* **ubuntu-latest**
  * /proc2 is used for retrieving data

---

## ⚠️ Known Issues (Windows/MinGW)

*   **MinGW Stability:** The build using MinGW 13.1.0 is currently unstable and may crash during process enumeration.
*   **Permissions:** Some system-level process information might be unavailable depending on user privileges.
*   **MSVC 2022:** It is highly recommended to use the MSVC compiler for a stable experience on Windows.

---

## 🌿 Branching Strategy

*   `stable` — Fully tested, stable releases.
*   `main` — Current Beta version for general use.
*   `devel` — Unstable Alpha branch for active development.

---
## 🏗 Installation & Build
**Bash:**
   ```bash
   git clone https://github.com/skierua/SysMonitor
   cd SysMonitor
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```
Or use Qt Creator to build `CMakeLists.txt`

