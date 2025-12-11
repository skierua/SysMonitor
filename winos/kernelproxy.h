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

class KernelProxy : public StaticBase<KernelProxy>
{

public:
    static KernelProxy & getSelf() {
        static KernelProxy self;
        return self;
    };
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
