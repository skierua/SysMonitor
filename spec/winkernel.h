#ifndef WINKERNEL_H
#define WINKERNEL_H

#include <windows.h>
#include <tlhelp32.h>
#include <minwinbase.h>
#include <psapi.h>

#include <tchar.h>

// #include <vector>
#include <QString>

#include "../shared/TempLib.h"

using VProcInfoList = QList<vk_proc_info>;

#ifndef WIN_TICK_COEF   // actualy nano
#define WIN_TICK_COEF 10000000ULL
#endif
#ifndef WIN_EPOC_DIFF
#define WIN_EPOC_DIFF 11644473600ULL
#endif

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

class WinKernel : public StaticBase<WinKernel>
{
    friend class StaticBase<WinKernel>;

public:
    static WinKernel & getSelf() {
        static WinKernel self;
        return self;
    }
    ~WinKernel() noexcept = default;

    int test() {return 42;}
    int crntEUID() { return 0; }
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
    static WinKernel * self;

    QString m_lastError{QString("")};
    QString m_logPath{QString("./Logs")};
};

#endif // WINKERNEL_H
