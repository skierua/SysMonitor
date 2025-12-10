#ifndef STRU_H
#define STRU_H
#include <string>
#include <vector>
// #include <QMetaType>
#include <QList>
#include <QString>

#define VK_MAXCOMLEN       16              /* max command name remembered */

struct vk_proc_info {
    int pid{0};
    int ppid{0};
    // std::string comm;
    QString qcomm;
    unsigned int mem{0};
    unsigned int vm{0};
    int th_all{0};
    int th_active{0};
    long tm{0};
    unsigned int uid{0};        // user ID
    // char chcomm[VK_MAXCOMLEN + 1];
};

// Q_DECLARE_METATYPE(vk_proc_info)
#define VProcInfoList QList<vk_proc_info>
#define VProcInfoVec std::vector<vk_proc_info>

#endif // STRU_H
