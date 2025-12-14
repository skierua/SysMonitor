#ifndef KERNELPROXY_H
#define KERNELPROXY_H

// #include <iostream>
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

//#include "../shared/stru.h"

// using std::vector;

#ifndef WIN_TICK_COEF   // actialy nano
#define WIN_TICK_COEF 10000000ULL
#endif
#ifndef WIN_EPOC_DIFF
#define WIN_EPOC_DIFF 11644473600ULL
#endif

#include "../shared/TempLib.h"

class ProcHandle {

    HANDLE m_procHandle;
public:
    explicit ProcHandle(DWORD pid, DWORD access_flag = PROCESS_ALL_ACCESS) {
        m_procHandle =OpenProcess(access_flag, FALSE, pid);
    }

    ~ProcHandle() noexcept {
        if( isValid() ) CloseHandle(m_procHandle);
    }

    HANDLE& get() { return m_procHandle; }

    bool isValid() const {
        return m_procHandle != NULL;
    }
};

class SnapHandle {

    HANDLE m_snapHandle;
public:
    explicit SnapHandle(DWORD flag = TH32CS_SNAPPROCESS) {
        m_snapHandle = CreateToolhelp32Snapshot( flag, 0 );
    }

    ~SnapHandle() noexcept {
        if( isValid() ) CloseHandle(m_snapHandle);
    }

    HANDLE& get() { return m_snapHandle; }

    bool isValid() const {
        return m_snapHandle != INVALID_HANDLE_VALUE;
    }
};

class KernelProxy : public StaticBase<KernelProxy>
{

public:
    static KernelProxy & getSelf() {
        static KernelProxy self;
        return self;
    }
    ~KernelProxy() noexcept = default;

    int test() {return 42;}
    int crntEUID() { return -1; }
    int canTerminate(int pid);
    int termProc(int pid);
    VProcInfoList procList();
    QString procPath(int pid);
    uint64_t sizeRAM();
    uint64_t usageRAM();
    const QString& lastError() { return m_lastError; }
    void setLogPath(const QString& path) { m_logPath = path; }
    const QString& logPath() { return m_logPath; }

private:
    static KernelProxy * self;

    QString m_lastError{QString("")};
    QString m_logPath{QString("./Logs")};
    // KernelProxy() = default;
    // KernelProxy(const KernelProxy&)= delete;
    // KernelProxy& operator=(const KernelProxy&)= delete;
    // KernelProxy(KernelProxy&&)= delete;
    // KernelProxy& operator=(KernelProxy&&)= delete;
};

#endif // KERNELPROXY_H
