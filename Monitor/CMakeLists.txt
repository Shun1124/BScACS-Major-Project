cmake_minimum_required(VERSION 3.16) # Specifies the minimum version of CMake required to build the project

project(Monitor VERSION 0.1 LANGUAGES CXX)

add_definitions(-DAWS_MSVC_CLZ_U32_DEFINED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

# Define AWS SDK path
set(CMAKE_PREFIX_PATH "C:/SDKs/aws-sdk-cpp-build/aws-sdk-cpp-install")

# Specify AWS SDK components
find_package(AWSSDK REQUIRED COMPONENTS sns sesv2)

set(HEADER_FILES # Defines a list of header files in the project
    registryKey.h
    monitoring.h
    jsonUtils.h
    rollback.h
    registryKeyModel.h
    alert.h
    settings.h
)

set(SOURCE_FILES # Defines a list of source files in the project
    main.cpp
    registryKey.cpp
    monitoring.cpp
    jsonUtils.cpp
    rollback.cpp
    registryKeyModel.cpp
    alert.cpp
    settings.cpp
)

set(JSON_FILES # Defines a list of JSON files used in the project
    monitoredKeys.json
    awsconfig.json
)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})
source_group("JSON Files" FILES ${JSON_FILES})

# Define the executable target first
qt_add_executable(appMonitor
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${JSON_FILES}
)

# Link AWS SDK libraries
target_link_libraries(appMonitor PRIVATE ${AWSSDK_LINK_LIBRARIES} Qt6::Quick aws-cpp-sdk-sns aws-cpp-sdk-sesv2)

target_include_directories(appMonitor PRIVATE ${AWSSDK_INCLUDE_DIR} "C:/SDKs/aws-sdk-cpp-install/include")

qt_add_qml_module(appMonitor # Adds a QML module to the appMonitor executable
    URI Monitor
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(appMonitor PROPERTIES # Sets properties for the appMonitor executable
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs) # Includes module to set up standard installation directories

install(TARGETS appMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
