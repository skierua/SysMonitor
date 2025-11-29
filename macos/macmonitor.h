#ifndef MACMONITOR_H
#define MACMONITOR_H

// #include <QObject>
// #include <QtQmlIntegration/qqmlintegration.h>
// #include <QCoreApplication>
// #include <QQmlEngine>
// #include <QByteArray>
// #include <QProcess>
#include <iostream>
#include <libproc.h> // Include for macOS specific functions
#include <sys/sysctl.h> // For sysctl() and related definitions
#include <sys/types.h>
#include <system_error>
#include <unistd.h>     // geteuid
// #include <machine/types.h> /* __darwin_time_t */


#include "../shared/TempLib.h"
#include "../shared/stru.h"

// #include <QDebug>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;



// Singleton
class MacMonitor : public StaticBase<MacMonitor> {
public:
    static MacMonitor & getSelf() {
        static MacMonitor self;
        return self;
    };
    ~MacMonitor() noexcept = default;
    int test() const {return 42;}
    int crntEUID() const { return geteuid(); }

private:
    static MacMonitor * self;
    // MacMonitor() = default;
    // MacMonitor(const MacMonitor&)= delete;
    // MacMonitor& operator=(const MacMonitor&)= delete;
    // MacMonitor(MacMonitor&&)= delete;
    // MacMonitor& operator=(MacMonitor&&)= delete;

};

/*
class MacMonitor : public BaseMonitor<MacMonitor>
{

private:

    int top_n{0};

public:
    MacMonitor();
    ~MacMonitor() noexcept = default;
    // void ttt() { qDebug() << "ttt started"; }

    void start();
    vector<vk_proc_info> proc();
};

*/

#endif // MACMONITOR_H
