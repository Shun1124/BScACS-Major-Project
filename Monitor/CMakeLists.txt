cmake_minimum_required(VERSION 3.16) # Specifies the minimum version of CMake required to build the project

project(Monitor VERSION 0.1 LANGUAGES CXX) # Defines the project name "Monitor", version 0.1, and sets the programming language to C++

add_definitions(-DAWS_MSVC_CLZ_U32_DEFINED) # Adds a preprocessor definition required for AWS SDK on MSVC compilers

set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures the C++ standard version is enforced as specified elsewhere

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick) # Finds Qt6 with the required Quick component, ensuring version 6.5 or later

qt_standard_project_setup(REQUIRES 6.5) # Sets up standard Qt configurations with required version 6.5

# Define AWS SDK path
# set(CMAKE_PREFIX_PATH "/Users/chingchun/Documents/SDKs/aws-sdk-cpp-build/aws-sdk-cpp-install") # Specifies the path to the AWS SDK installation

set(CMAKE_PREFIX_PATH
    "/opt/homebrew/opt/qt"
    "/Users/chingchun/Documents/SDKs/aws-sdk-cpp-build/aws-sdk-cpp-install"
)

# Specify AWS SDK components
find_package(AWSSDK REQUIRED COMPONENTS sns sesv2) # Finds the AWS SDK with the required SNS and SESV2 components

set(HEADER_FILES # Defines a list of header files in the project
    registryKey.h
    MacOSMonitoring.h
    WindowsMonitoring.h
    WindowsJsonUtils.h
    WindowsJsonUtils.h
    MacOSRollback.h
    WindowsRollback.h
    registryKeyModel.h
    alert.h
    settings.h
    plistFile.h
    plistFileModel.h
)

set(SOURCE_FILES # Defines a list of source files in the project
    main.cpp
    registryKey.cpp
    MacOSMonitoring.cpp
    WindowsJsonUtils.cpp
    MacOSRollback.cpp
    WindowsRollback.cpp
    MacOSJsonUtils.cpp
    WindowsMonitoring.cpp
    registryKeyModel.cpp
    alert.cpp
    settings.cpp
    plistFile.cpp
    plistFileModel.cpp
)

set(JSON_FILES # Defines a list of JSON files used in the project
    monitoredPlists.json
    awsconfig.json
    monitoredKeys.json
)

source_group("Header Files" FILES ${HEADER_FILES}) # Organizes header files into a group in the IDE
source_group("Source Files" FILES ${SOURCE_FILES}) # Organizes source files into a group in the IDE
source_group("JSON Files" FILES ${JSON_FILES}) # Organizes JSON files into a group in the IDE

# Define the executable target first
qt_add_executable(appMonitor # Creates the main executable target "appMonitor"
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${JSON_FILES}
)

# Link AWS SDK libraries to your target after defining it
target_link_libraries(appMonitor PRIVATE ${AWSSDK_LINK_LIBRARIES} Qt6::Quick aws-cpp-sdk-sns aws-cpp-sdk-sesv2) # Links AWS SDK and Qt libraries to the executable target

target_include_directories(appMonitor PRIVATE ${AWSSDK_INCLUDE_DIR} "C:/SDKs/aws-sdk-cpp-install/include") # Adds include directories for AWS SDK

qt_add_qml_module(appMonitor # Adds a QML module to the appMonitor executable
    URI Monitor # Sets the QML URI to "Monitor"
    VERSION 1.0 # Sets the QML module version to 1.0
    QML_FILES
        Main.qml
        QML_FILES Welcome.qml
        QML_FILES WindowsDashboard.qml
        QML_FILES MacOSDashboard.qml# Adds the main QML file for the user interface
)

set_target_properties(appMonitor PROPERTIES # Sets properties for the appMonitor executable
    MACOSX_BUNDLE TRUE # Marks the executable as a bundle on macOS
    WIN32_EXECUTABLE TRUE # Marks the executable as a Windows application without a console window
)

include(GNUInstallDirs) # Includes module to set up standard installation directories

install(TARGETS appMonitor # Defines installation rules for appMonitor
    BUNDLE DESTINATION . # Installs macOS bundles in the current directory
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # Installs shared libraries to the system library directory
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # Installs executables to the system binary directory
)
