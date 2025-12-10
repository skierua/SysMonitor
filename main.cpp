// #include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include <QDebug>

#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For std::chrono::seconds, milliseconds, etc.
// #include <unistd.h> // Required for getpid()

#include "shared/procprovider.h"
#include "shared/memprovider.h"
#include "shared/logger.h"
// #include "shared/TempLib.h"

// #include "test/testmac.h"

#if defined(__APPLE__)
// static_assert(false, "MacOS is not supported");
#include "macos/lib.h"
// namespace SML = MacLib;
#include "macos/kernelproxy.h"
#elif defined(__linux__)
static_assert(false, "Linux is not supported");
#elif defined(_WIN64)
// static_assert(false, "Windows/WIN64 is not supported");
#include "winos/lib.h"
// namespace SML = WinLib;
#else
static_assert(false, "Target OS is not supported");
#endif

using std::make_unique;
#define SM_PROC_REFRESH 1000 // milisecnd
#define SM_MEM_REFRESH 1000 // milisecnd


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // std::cout << "Qt Version: " << QT_VERSION_STR << std::endl;

    // std::vector<vk_proc_info> procData;
    auto logger = std::make_unique<Logger>( KernelProxy::getSelf().logPath() );
    if (!logger->isValid()){
        // qFatal("App LOG dir not exist and can't be created.");
        qCritical() << logger->lastError();
        return -1;
    }

    logger->log(QString("Application started with logging to %1").arg(logger->logPath()), 1);

    // std::cout
    //     << "KernelProxy = " << &KernelProxy::getSelf()
    //     << std::endl;

    // ProcProvider procProvider;
    auto procProvider = std::make_unique<ProcProvider>();

    auto memProvider = std::make_unique<MemProvider>();

    // MemProvider memProvider;
    memProvider->setTotalRAM(KernelProxy::getSelf().sizeRAM());

    std::atomic<bool> et_working_flag{true};
    // execution thread
    std::thread etProc([&procProvider, &et_working_flag]( auto sleep){
        while (et_working_flag.load(std::memory_order_relaxed)) {
            procProvider->addProcList(std::move(KernelProxy::getSelf().procList()));
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
        // std::cout << "etProc thread stopped." << std::endl;
    }, SM_PROC_REFRESH);
    etProc.detach();

    // execution thread
    std::thread etMem([&memProvider, &et_working_flag]( auto sleep){
        // memProvider.setTotalRAM(fn_getRAMSize());
        // int n{0};
        // while (et_working_flag.load(std::memory_order_relaxed) && n < 100) {
        //      ++n;
        while (et_working_flag.load(std::memory_order_relaxed)) {
            memProvider->addData(KernelProxy::getSelf().usageRAM());
            // auto mem = fn_getRAMUsage();
            // std::cout << "main.cpp data=" << mem/(1024*1024) << "MB" << " =" << mem << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }, SM_MEM_REFRESH);
    etMem.detach();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        { "procProvider", QVariant::fromValue(&(*procProvider)) }
        , { "memProvider", QVariant::fromValue(&(*memProvider)) }
    });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);


    // clean up threads before quit
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [&et_working_flag, &etProc, &etMem](){
        // qDebug() << "Application is cleaning up before termination...";
        et_working_flag.store(false, std::memory_order_relaxed);
        // if (etProc.joinable()) {
        //     etProc.join();
        //     std::cout << "etProc thread successfully joined." << std::endl;
        // } else {
        //     std::cout << "etProc thread is not joinable." << std::endl;
        // }
        // qDebug() << "Cleanup complete.";
    });

    QObject::connect(procProvider.get(), &ProcProvider::message, logger.get(),  &Logger::log, Qt::QueuedConnection);

    // terminate in case of FATAL message
    QObject::connect(logger.get(), &Logger::terminate, &app,  &QCoreApplication::quit, Qt::QueuedConnection);

    engine.loadFromModule("SysMonitor", "Main");

    // QQuickView view;
    // view.setResizeMode(QQuickView::SizeRootObjectToView);
    // view.setInitialProperties({{"procProvider", QVariant::fromValue(&procProvider)}});
    // //![0]
    // view.setSource(QUrl("qrc:/SysMonitor/Main.qml"));
    // // view.setSource(QUrl::fromLocalFile("Main.qml"));
    // view.show();

    return app.exec();
}
