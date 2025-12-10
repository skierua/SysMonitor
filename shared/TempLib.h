#ifndef TEMPLIB_H
#define TEMPLIB_H

// #include <QByteArray>
#include "./stru.h"

// currently not used

/*template <class Implementation>
class BaseMonitor {

public:
    int test() { return impl()->test(); }
    int crntEUID() { return impl()->crntEUID(); }

private:
    Implementation* impl() {  return static_cast<Implementation*>(this); }
}; */

template <typename Derived>
class StaticBase {
public:
    int test() { return impl()->test(); }

    int crntEUID() const { return impl()->crntEUID(); }
    int canTerminate(int pid) { return impl()->canTerminate(pid); }
    int termProc(int pid) { return impl()->termProc(pid); }
    VProcInfoList procList() { return impl()->procList(); }
    QString procPath(int pid) { return impl()->procPath(pid); }
    uint64_t sizeRAM() { return impl()->sizeRAM(); }
    uint64_t usageRAM() { return impl()->usageRAM(); }
    const QString& lastError() { return impl()->lastError(); }
    void setLogPath(const QString& path) { return impl()->setLogPath(path); }
    const QString& logPath() { return impl()->logPath(); }

protected:
    // The key to making it "static" and non-instantiable:
    // Only derived classes (which are friends) can access constructors,
    // and we delete them anyway to be safe.
    StaticBase() = default;
    StaticBase(const StaticBase&) = delete;
    StaticBase(StaticBase&&) = delete;
    StaticBase& operator=(const StaticBase&) = delete;
    StaticBase& operator=(StaticBase&&) = delete;

private:
    Derived* impl() {  return static_cast<Derived*>(this); }
};

#endif // TEMPLIB_H
