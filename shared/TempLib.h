#ifndef TEMPLIB_H
#define TEMPLIB_H

// #include <QByteArray>
#include "./stru.h"

// currently not used

using VProcInfoList = QList<vk_proc_info>;

template <typename Derived>
class StaticBase {
public:
    // static StaticBase & getSelf() { return &impl()->getSelf(); }
    // static Derived& getSelf() { return *static_cast<Derived*>(instance); }

    static Derived& instance() {
        static Derived staticInstance;
        return staticInstance;
    }

    int test() { return impl()->test(); }

    int crntEUID() { return impl()->crntEUID(); }
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
    StaticBase() = default;
    friend Derived;
    StaticBase(const StaticBase&) = delete;
    StaticBase(StaticBase&&) = delete;
    StaticBase& operator=(const StaticBase&) = delete;
    StaticBase& operator=(StaticBase&&) = delete;

private:
    Derived* impl() {  return static_cast<Derived*>(this); }
    const Derived* impl() const { return static_cast<const Derived*>(this); }
};

#endif // TEMPLIB_H
