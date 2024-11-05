cmake_minimum_required(VERSION 3.16)

project(Monitor VERSION 0.1 LANGUAGES CXX)

add_definitions(-DAWS_MSVC_CLZ_U32_DEFINED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)

# Define AWS SDK path
set(CMAKE_PREFIX_PATH "C:/SDKs/aws-sdk-cpp-build/aws-sdk-cpp-install")

# Specify AWS SDK components
find_package(AWSSDK REQUIRED COMPONENTS sns sesv2)

set(HEADER_FILES
    registryKey.h
    monitoring.h
    jsonUtils.h
    rollback.h
    registryKeyModel.h
    alert.h
    settings.h
)

set(SOURCE_FILES
    main.cpp
    registryKey.cpp
    monitoring.cpp
    jsonUtils.cpp
    rollback.cpp
    registryKeyModel.cpp
    alert.cpp
    settings.cpp
)

set(JSON_FILES
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

# Link AWS SDK libraries to your target after defining it
target_link_libraries(appMonitor PRIVATE ${AWSSDK_LINK_LIBRARIES} Qt6::Quick aws-cpp-sdk-sns aws-cpp-sdk-sesv2)
target_include_directories(appMonitor PRIVATE ${AWSSDK_INCLUDE_DIR} "C:/SDKs/aws-sdk-cpp-install/include")

qt_add_qml_module(appMonitor
    URI Monitor
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(appMonitor PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS appMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
