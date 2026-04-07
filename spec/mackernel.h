#ifndef MACKERNEL_H
#define MACKERNEL_H


#include "../shared/TempLib.h"

using VProcInfoList = QList<vk_proc_info>;

class MacKernel : public StaticBase<MacKernel>
{
    friend class StaticBase<MacKernel>; // Дозволяємо базовому класу доступ до impl()

public:
    static MacKernel & getSelf() {
        static MacKernel self;
        return self;
    }
    ~MacKernel() noexcept = default;

    int test() {return 42;}
    int crntEUID();
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
    static MacKernel * self;

    QString m_lastError{QString("")};
    QString m_logPath{QString("./Logs")};
};

#endif // MACKERNEL_H
