#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <inttypes.h>
#include <tlhelp32.h>
#include <minwinbase.h>
#include <iomanip>
// #include <cstdlib>
// #include <iomanip>
#include <psapi.h>
#include <tchar.h>

#include <QString>

#include "../shared/stru.h"

// using std::vector;

#ifndef WIN_TICK_COEF   // actialy nano
#define WIN_TICK_COEF 10000000ULL
#endif
#ifndef WIN_EPOC_DIFF
#define WIN_EPOC_DIFF 11644473600ULL
#endif

namespace WinLib {

struct TreadCount {
    int total{0};
    int active{0};
};

std::string sysLogPath()
// { return "~Library/Logs"; }
{ return "./Logs"; }    // default for testing

// deprecated
int getCrntEUID(){
    return -1;
    // return geteuid();
}

// blank function
int lastErrNo(){
    return 0;
}

int canTerminate(int pid) {
    HANDLE hpr = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (hpr == NULL) return -1;
    CloseHandle(hpr);
    return 0;
}


int termProc(int pid){
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

/*
 *  auto closeGui_v2 = [](DWORD lpid) {
     bool success = false;
     HWND toClose_hwnd = NULL; // mainWindow for process pid
     std::cout << "closeGui" << std::endl;
     EnumWindows([](HWND crnt_hwnd, LPARAM lParam) -> BOOL {
         // auto& data = *reinterpret_cast<HWND*>(lParam);
         DWORD wPID;
         GetWindowThreadProcessId(crnt_hwnd, &wPID);
         std::cout << "closeGui wPID=" << wPID << " lParam=" << *(reinterpret_cast<DWORD*>(lParam)) << std::endl;

         if (wPID == *(reinterpret_cast<DWORD*>(lParam)) ) {
             if (IsWindowVisible(crnt_hwnd) && GetWindowTextLength(crnt_hwnd) > 0 && GetParent(crnt_hwnd) == NULL){
                 PostMessage(crnt_hwnd, WM_CLOSE, 0, 0) != 0;
                 return false;
             }
         }
         return TRUE;
     }, reinterpret_cast<LPARAM>(&lpid));
     return success;
 }; */


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

VProcInfoList getProcList(){
    // std::cout << "WinLib getProc" << std::endl;
    VProcInfoList res;

    HANDLE hProcessSnap;
    HANDLE hProcess;
    // Set the structure and size
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof( PROCESSENTRY32 );
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // bool ok{true};

    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    // ok &= (hProcessSnap != INVALID_HANDLE_VALUE);
    if( hProcessSnap == INVALID_HANDLE_VALUE )
    {
        // std::cerr << "CreateToolhelp32Snapshot (of processes)" << std::endl;
        return std::move(res);
    }

    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    // ok &= (hThreadSnap != INVALID_HANDLE_VALUE);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        // std::cerr << "Error: Unable to create thread snapshot. Code: " << GetLastError() << "\n";
        CloseHandle( hProcessSnap );          // clean the snapshot object
        return std::move(res);
    }


    // Retrieve information about the first process,
    // and exit if unsuccessful
    if( !Process32First( hProcessSnap, &pe32 ) )
    {
        // printError( TEXT("Process32First") ); // show cause of failure
        // std::cout << "Process32First" << std::endl;
        CloseHandle( hProcessSnap );          // clean the snapshot object
        CloseHandle( hThreadSnap );          // clean the snapshot object
        return std::move(res);
    }

    auto lthread = [&hThreadSnap, &te32](auto lpid/*, auto& th*/){
        TreadCount th;
        std::cout << "auto lthread lpid=" << lpid << std::endl;
        if (!Thread32First(hThreadSnap, &te32)) {
            // std::cerr << "Error: Unable to get first thread. Code: " << GetLastError() << "\n";
            return th;
        }
        do {
            std::cout << "auto lthread Thread32Next lpid=" << lpid << std::endl;
            if (te32.th32OwnerProcessID == lpid) {
                // Try opening the thread to check if it's active
                ++th.total;
                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
                if (hThread) {
                    // If we can open it, it's active
                    ++th.active;
                    std::cout << "Thread ID: " << te32.th32ThreadID << " (Active)\n";
                    CloseHandle(hThread);
                } else {
                    std::cout << "Thread ID: " << te32.th32ThreadID << " (Not accessible / possibly terminated)\n";
                }
            }
        } while (Thread32Next(hThreadSnap, &te32));
        // return;
        return th;
    };

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    PROCESS_MEMORY_COUNTERS mem;
    FILETIME creationTime, exitTime, kernelTime, userTime;
    // SYSTEMTIME tmUTC, stLocal;
    // time_t epocTime;
    do
    {
        vk_proc_info vpri;
        // std::vector<char> buffer(MAX_PATH +1);

        // Retrieve the priority class.
        dwPriorityClass = 0;
        hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
        if( hProcess == NULL ){
            // printError( TEXT("OpenProcess") );
        // std::cout << "OpenProcess" << std::endl;
        } else {
            dwPriorityClass = GetPriorityClass( hProcess );
            if( !dwPriorityClass ){
                // printError( TEXT("GetPriorityClass") );
                // std::cout << "GetPriorityClass" << std::endl;
                CloseHandle( hProcess );
                continue;
            }
        }
        if (GetProcessMemoryInfo(hProcess, &mem, sizeof(mem))) {
            // std::cout << "Process ID: " << processID << std::endl;
            // std::cout << "Working Set Size: " << mem.WorkingSetSize / 1024 << " KB" << std::endl;
            // std::cout << "Pagefile Usage: " << pmc.PagefileUsage / 1024 << " KB" << std::endl;
        }   // else std::cout << "Working Set Size: ERROR" << std::endl;

        ULARGE_INTEGER uliTime;
        uliTime.LowPart  = 0;
        uliTime.HighPart = 0;
        if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            // std::cerr << "GetProcessTimes failed. Error code: " << GetLastError() << "\n";
            // return 1;
        } else {
            uliTime.LowPart  = creationTime.dwLowDateTime;
            uliTime.HighPart = creationTime.dwHighDateTime;
        }
        // TreadCount th;
        // th = lthread(pe32.th32ProcessID);
        vpri.pid = pe32.th32ProcessID;
        vpri.ppid = pe32.th32ParentProcessID;
        // vpri.comm = std::wstring(buffer.data()); //pe32.szExeFile;
        vpri.qcomm = QString(pe32.szExeFile);
        vpri.mem = mem.WorkingSetSize / 1024;
        vpri.vm = 0;
        vpri.th_all = pe32.cntThreads;
        vpri.th_active = 0;
        vpri.tm = (uliTime.QuadPart / WIN_TICK_COEF) - WIN_EPOC_DIFF;
        vpri.uid = 0;
        res << vpri;
        // std::cout << "ID="<< pe32.th32ProcessID
        //           // << " comm="<< _tprintf( TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile
        //            << " Thr="<< pe32.cntThreads
        //           << " PIr="<< pe32.th32ParentProcessID
        //           << " Priority="<< pe32.pcPriClassBase
        //           << std::endl;

    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle(hThreadSnap);
    CloseHandle( hProcessSnap );
    return std::move(res);
}

QString getProcPath(int pid) {
    auto res = QString("");
    HANDLE hp = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

    if (hp) {
        TCHAR filePath[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageName(hp, 0, filePath, &size)) {
            // std::wcout << L"Process Path: " << filePath << std::endl;
        } else {
            // std::cerr << "Failed to retrieve process path." << std::endl;
        }
        res = QString(filePath);
        CloseHandle(hp);
    } else {
        // std::cerr << "Failed to open process." << std::endl;
    }
    return res;
}

QString getProcPath_v2(int pid) {
    auto res = QString("");
    HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (hp) {
        TCHAR filePath[MAX_PATH];
        if (GetModuleFileNameEx(hp, NULL, filePath, MAX_PATH)) {
            // std::wcout << L"Process Path: " << filePath << std::endl;
        } else {
            // std::cerr << "Failed to retrieve process path." << std::endl;
        }
        res = QString(filePath);
        CloseHandle(hp);
    } else {
        // std::cerr << "Failed to open process." << std::endl;
    }
    return res;
}

// res =0 for error, but it's not fair enought
uint64_t getRAMSize() {
    uint64_t res{0};  // same as unsigned long long
    // ULONGLONG totalKB = 0;

    // Retrieve the amount of physically installed RAM in kilobytes
    if (GetPhysicallyInstalledSystemMemory(&res)) {
        // std::cout << "Total Installed RAM: " << res << " GB" << std::endl;
    } else {
        // std::cerr << "Error: Unable to retrieve RAM size. Error code: "
        //           << GetLastError() << std::endl;
        return 1;
    }

    return res * 1024;
}

// res =0 for error, but it's not fair enought
uint64_t getRAMUsage() {
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




}


#endif // LIB_H
