cmake_minimum_required(VERSION 3.16)

project(SysMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VY Add include directories so target can find the QML_ELEMENTS
set(QML_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Graphs)


if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "macOS-specific QML and sources.")
    # macOS-specific files
    list(APPEND QML_SOURCES_COMMON macos/kernelproxy.h macos/kernelproxy.cpp)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows-specific QML and sources.")
    # Windows-specific files
    list(APPEND QML_SOURCES_COMMON winos/kernelproxy.h winos/kernelproxy.cpp)

# elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     message(STATUS "Linux-specific QML and sources.")
    # Linux-specific files
    # list(APPEND QML_SOURCES_COMMON linux_dbus_interface.cpp)

else()
    message(WARNING "No specific QML files defined for ${CMAKE_SYSTEM_NAME}.")
endif()

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSysMonitor  WIN32 MACOSX_BUNDLE
    main.cpp
)
qt_add_qml_module(appSysMonitor
    URI SysMonitor
    VERSION 1.0
    QML_FILES
        qt/qml/Main.qml
        SOURCES shared/TempLib.h
        # SOURCES test/testmac.h test/testmac.cpp
        SOURCES shared/stru.h
        SOURCES shared/procprovider.h shared/procprovider.cpp
        # SOURCES macos/lib.h
        SOURCES shared/memprovider.h shared/memprovider.cpp
        SOURCES shared/logger.h shared/logger.cpp
        QML_FILES qt/qml/ProcViewDelegate.qml
        QML_FILES qt/qml/ProcViewHighlight.qml
        QML_FILES qt/qml/HeaderElem.qml
        QML_FILES qt/qml/MemView.qml
        QML_FILES qt/qml/lib.js
        RESOURCES TODO.txt
        RESOURCES README.md
        RESOURCES .gitignore
        # SOURCES winos/kernelproxy.h winos/kernelproxy.cpp
        SOURCES ${QML_SOURCES_COMMON}       # Includes the common and OS-specific C++ sources
)

# qt_add_qml_module(appSysMonitor
#   URI com.sysmonitor
#   VERSION 1.0
# )

# VY Add includepaths
target_include_directories(appSysMonitor PRIVATE
   ${QML_INCLUDES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSysMonitor PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSysMonitor
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSysMonitor PRIVATE Qt6::Quick)
target_link_libraries(appSysMonitor PRIVATE Qt6::Core)
target_link_libraries(appSysMonitor PRIVATE Qt6::Graphs)


include(GNUInstallDirs)
install(TARGETS appSysMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
