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


#if defined(__APPLE__)          //definedQ_OS_MAC)
#include "spec/mackernel.h"
using Krnl = MacKernel;
#elif defined(_WIN64)           //defined(Q_OS_WIN)
// static_assert(false, "Windows is not supported");
#include "spec/winkernel.h"
using Krnl = WinKernel;
#elif defined(__linux__)            //defined(Q_OS_LINUX)
// static_assert(false, "Linux is not supported");
#include "spec/linuxkernel.h"
using Krnl = LinuxKernel;
#else
#error "Platform not supported"
#endif


using std::make_unique;
constexpr uint SM_PROC_REFRESH = 1000; // milisecnd
constexpr uint SM_MEM_REFRESH = 1000; // milisecnd


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // std::cout << "Qt Version: " << QT_VERSION_STR << std::endl;
    auto& proxy = Krnl::getSelf();

    // std::vector<vk_proc_info> procData;
    auto logger = std::make_unique<Logger>( proxy.logPath() );
    // auto logger = std::make_unique<Logger>( KernelProxy::getSelf().logPath() );
    // auto logger = std::make_unique<Logger>( "./Logs" );
    if (!logger->isValid()){
        // qFatal("App LOG dir not exist and can't be created.");
        qCritical() << logger->lastError();
        return -1;
    }

    logger->log(QString("Application started with logging to %1").arg(logger->logPath()), 1);

    // StaticBase<KernelProxy> *pkrnl = &KernelProxy::getSelf();
    // StaticBase<KernelProxy> *pkrnl = &StaticBase::getSelf();


    // std::cout
    //     << "KernelProxy = " << &KernelProxy::getSelf()
    //     << " pkrnl = " << &(*pkrnl)
    //     << " crntEUID= " << pkrnl->test()
    //     << std::endl;

    // ProcProvider procProvider;
    auto procProvider = std::make_unique<ProcProvider>();

    auto memProvider = std::make_unique<MemProvider>();

    // MemProvider memProvider;
    memProvider->setTotalRAM(proxy.sizeRAM());
    // memProvider->setTotalRAM(pkrnl->sizeRAM());

    // std::atomic<bool> et_working_flag{true};
    volatile bool et_working_flag{true};
    // execution thread
    std::thread etProc([&procProvider, &et_working_flag, &proxy]( auto sleep){
        // while (et_working_flag.load(std::memory_order_relaxed)) {
        while (et_working_flag) {
            procProvider->addProcList(std::move(proxy.procList()));
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
        // std::cerr << "etProc thread stopped." << std::endl;
    }, SM_PROC_REFRESH);
    etProc.detach();

    // execution thread
    std::thread etMem([&memProvider, &et_working_flag, &proxy]( auto sleep){
        // memProvider.setTotalRAM(fn_getRAMSize());
        // int n{0};
        // while (et_working_flag.load(std::memory_order_relaxed) && n < 100) {
        //      ++n;
        // while (et_working_flag.load(std::memory_order_relaxed)) {
        while (et_working_flag) {
            memProvider->addData(proxy.usageRAM());
            // memProvider->addData(pkrnl->usageRAM());
            // auto mem = fn_getRAMUsage();
            // std::cerr << "main.cpp data=" << mem/(1024*1024) << "MB" << " =" << mem << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
        // std::cerr << "etMem thread stopped." << std::endl;
    }, SM_MEM_REFRESH);
    etMem.detach();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        { "procProvider", QVariant::fromValue(procProvider.get()) }
        , { "memProvider", QVariant::fromValue(memProvider.get()) }
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
        // et_working_flag.store(false, std::memory_order_relaxed);
        et_working_flag = false;
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
