#include "kernelproxy.h"

// KernelProxy::KernelProxy() {}

int KernelProxy::canTerminate(int pid) {
    HANDLE hpr = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (hpr == NULL) return -1;
    CloseHandle(hpr);
    return 0;
}

int KernelProxy::termProc(int pid) {
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

VProcInfoList KernelProxy::procList() {
    // std::cout << "WinLib getProc" << std::endl;
    VProcInfoList res;

    HANDLE hProcessSnap;
    HANDLE hProcess;
    // Set the structure and size
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof( PROCESSENTRY32 );
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
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
    if( !Process32First( hProcessSnap, &pe32 ) )
    {
        // printError( TEXT("Process32First") ); // show cause of failure
        // std::cout << "Process32First" << std::endl;
        CloseHandle( hProcessSnap );          // clean the snapshot object
        CloseHandle( hThreadSnap );          // clean the snapshot object
        return std::move(res);
    }

    // unused
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

    // get information about each process
    PROCESS_MEMORY_COUNTERS mem;
    FILETIME creationTime, exitTime, kernelTime, userTime;
    do
    {
        vk_proc_info vpri;

        // Retrieve the priority class.
        dwPriorityClass = 0;
        hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
        if( hProcess == NULL ){
            // printError( TEXT("OpenProcess") );
        // std::cout << "OpenProcess" << std::endl;
            continue;
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

    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle(hThreadSnap);
    CloseHandle( hProcessSnap );
    return std::move(res);
}

QString KernelProxy::procPath(int pid) {
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

// res =0 for error, but it's not fair enought
uint64_t KernelProxy::sizeRAM() {
    uint64_t res{0};  // same as unsigned long long
    // ULONGLONG totalKB = 0;

    // Retrieve the amount of physically installed RAM in kilobytes
    if (GetPhysicallyInstalledSystemMemory(&res)) {
        // std::cout << "Total Installed RAM: " << res << " GB" << std::endl;
    } else {
        // std::cerr << "Error: Unable to retrieve RAM size. Error code: "
        //           << GetLastError() << std::endl;
        return 0;
    }

    return res * 1024;
}

// res =0 for error, but it's not fair enought
uint64_t KernelProxy::usageRAM() {
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
