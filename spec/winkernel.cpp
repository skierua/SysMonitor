// #include <cstdlib>
// #include <iomanip>
#include <inttypes.h>

#include <iomanip>


#include "winkernel.h"

// WinKernel::WinKernel() {}

int WinKernel::canTerminate(int pid) {
    auto pr = ProcHandle(static_cast<DWORD>(pid), PROCESS_TERMINATE);
    if (!pr.isValid()) return -1;
    return 0;
}

int WinKernel::termProc(int pid) {
    // Gracefully close GUI process
    auto closeGui = [](DWORD lpid) {
        struct EnumData {
            DWORD pid;
            HWND hwnd;
        };
        EnumData data = { lpid, NULL };
        EnumWindows([](HWND crnt_hwnd, LPARAM lParam) -> BOOL {
            EnumData* data = reinterpret_cast<EnumData*>(lParam);
            DWORD crntPID{0};
            GetWindowThreadProcessId(crnt_hwnd, &crntPID);
            // std::cout << "closeGui crntPID=" << crntPID << " lParam=" << *(reinterpret_cast<DWORD*>(lParam)) << std::endl;

            if (crntPID == data->pid && GetWindow(crnt_hwnd, GW_OWNER) == nullptr ) {
                if (IsWindowVisible(crnt_hwnd) && GetWindowTextLength(crnt_hwnd) > 0 && GetParent(crnt_hwnd) == NULL){
                    data->hwnd = crnt_hwnd;
                    return false;
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&data));
        return PostMessage(data.hwnd, WM_CLOSE, 0, 0) != 0;
    };


    // Gracefully close console process
    auto closeConsole = [](DWORD lpid) {
        // Attach to the console of the target process
        if (!AttachConsole(lpid)) {
            return false;
        }
        // Disable Ctrl+C handling for our own process
        SetConsoleCtrlHandler(nullptr, TRUE);

        bool res = GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0) != 0;

        // Detach from console
        FreeConsole();
        // Re-enable Ctrl+C handling
        SetConsoleCtrlHandler(nullptr, FALSE);


        return res;
    };

    bool success = false;

    // Try GUI close first
    if (closeGui(static_cast<DWORD>(pid))) {
        success = true;
    }
    // Try console close if GUI close failed
    else if (closeConsole(static_cast<DWORD>(pid))) {
        success = true;
    }

    if (!success) {
        // std::cerr << "Could not gracefully terminate process " << pid
        //           << ". It may not have a window or console.\n";
        return -1;
    }

    return 0;
}

VProcInfoList WinKernel::procList() {
    // std::cout << "WinLib getProc" << std::endl;
    VProcInfoList res;

    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    auto procSnap = SnapHandle(TH32CS_SNAPPROCESS);
    if( !procSnap.isValid() )
    {
        // std::cerr << "CreateToolhelp32Snapshot (of processes)" << std::endl;
        return std::move(res);
    }

    // Set the structure and size
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    // Retrieve information about the first process,
    if( !Process32First( procSnap.get(), &pe32 ) )
    {
        return std::move(res);
    }


    // get information about each process
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS mem;
    FILETIME creationTime, exitTime, kernelTime, userTime;
    do
    {
        vk_proc_info vpri;

        // Retrieve the priority class.
        dwPriorityClass = 0;
        auto proc = ProcHandle(pe32.th32ProcessID, PROCESS_ALL_ACCESS);
        if( !proc.isValid() ){
            continue;
        } else {    // not actualy used
            dwPriorityClass = GetPriorityClass( proc.get() );
            if( !dwPriorityClass ){
                // std::cout << "GetPriorityClass" << std::endl;
                continue;
            }
        }

        if (GetProcessMemoryInfo(proc.get(), &mem, sizeof(mem))) {
            // std::cout << "Process ID: " << processID << std::endl;
            // std::cout << "Working Set Size: " << mem.WorkingSetSize / 1024 << " KB" << std::endl;
            // std::cout << "Pagefile Usage: " << pmc.PagefileUsage / 1024 << " KB" << std::endl;
        }   // else std::cout << "Working Set Size: ERROR" << std::endl;

        ULARGE_INTEGER uliTime;
        uliTime.LowPart  = 0;
        uliTime.HighPart = 0;
        if (!GetProcessTimes(proc.get(), &creationTime, &exitTime, &kernelTime, &userTime)) {
            // std::cerr << "GetProcessTimes failed. Error code: " << GetLastError() << "\n";
            // return 1;
        } else {
            uliTime.LowPart  = creationTime.dwLowDateTime;
            uliTime.HighPart = creationTime.dwHighDateTime;
        }
        vpri.pid = pe32.th32ProcessID;
        vpri.ppid = pe32.th32ParentProcessID;
        // vpri.comm = std::wstring(buffer.data()); //pe32.szExeFile;
        vpri.qcomm = QString(pe32.szExeFile);
        vpri.mem = mem.WorkingSetSize;  // / 1024;
        vpri.vm = 0;
        vpri.th_all = pe32.cntThreads;
        vpri.th_active = 0;
        vpri.tm = (uliTime.QuadPart / WIN_TICK_COEF) - WIN_EPOC_DIFF;
        vpri.uid = 0;
        res << vpri;

    } while( Process32Next( procSnap.get(), &pe32 ) );

    return std::move(res);
}
/*
VProcInfoList WinKernel::procList() {
    VProcInfoList res;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return std::move(res);

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            vk_proc_info vpri;
            vpri.pid = pe.th32ProcessID;
            vpri.ppid = pe.th32ParentProcessID;
            vpri.qcomm = QString::fromWCharArray(pe.szExeFile);
            vpri.vm = 0;
            vpri.th_all = pe.cntThreads;
            vpri.th_active = 0;

            // Отримуємо додаткову інфо (шлях та RAM)
            auto hProcess = ProcHandle(pe.th32ProcessID, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ);
            // HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
            if (!hProcess.isValid()) {
                continue;
            }
            // Шлях до файлу
            // wchar_t pathBuffer[MAX_PATH];
            // if (GetModuleFileNameExW(hProcess, NULL, pathBuffer, MAX_PATH)) {
            //     vpri.path = QString::fromWCharArray(pathBuffer);
            // }

            // Використання RAM (Working Set Size)
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess.get(), &pmc, sizeof(pmc))) {
                vpri.mem = pmc.WorkingSetSize;
            }

            // Час запуску (спрощено)
            FILETIME createTime, exitTime, kernelTime, userTime;
            // if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
            //     SYSTEMTIME st;
            //     FileTimeToSystemTime(&createTime, &st);
            //     vpri.tm = st;
            //     // info.startTime = QString("%1:%2").arg(st.wHour).arg(st.wMinute);
            // }

            ULARGE_INTEGER uliTime;
            uliTime.LowPart  = 0;
            uliTime.HighPart = 0;
            if (!GetProcessTimes(hProcess.get(), &createTime, &exitTime, &kernelTime, &userTime)) {
                // std::cerr << "GetProcessTimes failed. Error code: " << GetLastError() << "\n";
                // return 1;
            } else {
                uliTime.LowPart  = createTime.dwLowDateTime;
                uliTime.HighPart = createTime.dwHighDateTime;
            }
            vpri.tm = (uliTime.QuadPart / WIN_TICK_COEF) - WIN_EPOC_DIFF;
            vpri.uid = 0;

            // CloseHandle(hProcess);
            // list.push_back(vpri);
            res << vpri;
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return std::move(res);
}
*/
QString WinKernel::procPath(int pid) {
    auto res = QString("");
    auto proc = ProcHandle(static_cast<DWORD>(pid), PROCESS_QUERY_LIMITED_INFORMATION);

    if (proc.isValid()) {
        TCHAR filePath[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageName(proc.get(), 0, filePath, &size) > 0) {
            // std::wcout << L"Process Path: " << filePath << std::endl;
            res = QString(filePath);
        } else {
            // std::cerr << "Failed to retrieve process path." << std::endl;
        }
    } else {
        // std::cerr << "Failed to open process." << std::endl;
    }
    return res;
}

// res =0 for error, but it's not fair enought
/*uint64_t WinKernel::sizeRAM() {
    uint64_t res{0};  // same as unsigned long long

    // Retrieve the amount of physically installed RAM in kilobytes
    if (GetPhysicallyInstalledSystemMemory(&res)) {
        // std::cout << "Total Installed RAM: " << res << " GB" << std::endl;
    } else {
        // std::cerr << "Error: Unable to retrieve RAM size. Error code: "
        //           << GetLastError() << std::endl;
        return 0;
    }

    return res * 1024;
}*/

uint64_t WinKernel::sizeRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys;
    }
    return 0;
}

// res =0 for error, but it's not fair enought
uint64_t WinKernel::usageRAM() {
    uint64_t res{0};  // same as unsigned long long
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);

    if (!GlobalMemoryStatusEx(&memInfo)) {
        return res; // res =0, API call failed
    }

    SIZE_T totalPhys = memInfo.ullTotalPhys; // Total physical memory
    SIZE_T freePhys  = memInfo.ullAvailPhys; // Available physical memory
    res = totalPhys - freePhys;        // Used memory

    // std::cout << "RAMu: " << res << std::endl;
    return res;
}

/*uint64_t WinKernel::usageRAM() {
    PERFORMANCE_INFORMATION pi;
    if (GetPerformanceInfo(&pi, sizeof(pi))) {
        return (uint64_t)(pi.CommitTotal * pi.PageSize);
    }
    return 0;
}*/

