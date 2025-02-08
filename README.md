# COMP 8800 and 8900 Major Project - System Configuration Monitor

This project monitors Windows registry keys and macOS configuration files (.plist) for changes, detects critical and non-critical modifications, and sends alerts via email and SMS using AWS services (SES and SNS). It also provides a rollback feature for critical changes. The project consists of a C++ backend for monitoring configuration changes and a Qt-based frontend for displaying logs and managing settings.
## Table of Contents
[Features](#features)\
[Prerequisites](#prerequisites)\
[Installation Instructions](#installation-instructions)\
[Configuration Settings](#configuration-settings)\
[Testing the Project](#testing-the-project)\
[Code Structure](#code-structure)

## Features
**Real-Time Monitoring**: Tracks changes in specified Windows registry keys and macOS .plist files.

**Rollback System**: Automatically reverts critically changes to maintain system integrity.

**Customizable Alerts**: Sends alerts via email and SMS for critical changes and based on user-defined thresholds for non-critical changes.

**Cross-Platform Support**: Monitors Windows registry keys and macOS configuration files for comprehensive coverage.

**Enhanced Security**: Ensures secure data handling with encrypted configurations and log files.

**MySQL Database Support**: Logs monitored changes, user preferences, and alerts securely in a MySQL database.

**Encryption for Sensitive Data**: Encrypts credentials, logs, and settings using OpenSSL for added security.

**Search Functionality**: Users can search historical change data stored in the database via the application's UI.

## Prerequisites
### Tools and Technologies Required
#### Windows System
**Cross-Platform(Windows/macOS)**:
**- Development Environment**: Qt Creator

**- AWS Account** (Optional): For email and SMS alerts through SES and SNS.

**- CMake**: For building the project.

**- AWS SDK for C++**: Used for integrating with AWS services.

**- MySQL Server & Connector**: Required for logging monitored changes in the database.  

**- OpenSSL**: Required for encryption functionality.

#### macOS
**- Operating System**: macOS 11.0 or later.

**- Libraries**: macOS file system APIs for .plist file monitoring.

#### Windows
**- Operating System**: Windows 10 or later (64-bit).

**- Libraries**: Windows Registry APIs for monitoring

### Installation Instructions
#### Step 1: Install AWS SDK for C++
1. Follow the link to download and install the AWS SDK for C++ from (https://github.com/aws/aws-sdk-cpp).
2. Remember to modify the path of the installed SDK inside the `CMakeLists.txt` file.
#### Step 2: Install Qt Creator
- Download Qt 6.5 or later from (https://doc.qt.io/qtcreator/)
- Include `Qt Quick` components for QML

#### Step 3: Install MySQL Server and Connector
1. Install MySQL Server (https://dev.mysql.com/downloads/mysql/).
2. Install MySQL Connector C++ (https://dev.mysql.com/downloads/connector/cpp/).
3. Set up a MySQL database for logging changes and user preferences.

#### Step 4: Clone the Repository
Clone the project repository to your local machine
```
git clone https://github.com/Shun1124/BScACS-Major-Project.git
```

#### Step 5: Set up AWS Services (Optional)
- Specific testing email and password is provided in the prototype report, therefore, this step is optional.
- Signup for an AWS account and set up SES and SNS.
- **SES**: Verify your email address (required by AWS SES to send emails).
- **SNS**: Register and set up SMS service in the AWS console.

#### Step 6: Set Up AWS Credentials
Create a  `awsconfig.json` file in the `Monitor` folder that contains access key information for a restricted IAM user with full access to SNS and SES. See the report for access key information. If you want to use your own AWS account instead, you can modify the existing `awsconfig.json` with your own credentials in the `Backend` directory with your own access information.
##### Inside the awsconfig.json file:
```
{
  "accessKeyId": "YOUR_ACCESS_KEY_ID",
  "secretAccessKey": "YOUR_SECRET_ACCESS_KEY",
  "region": "YOUR_AWS_REGION"
}
```

#### Step 7: Build the Project
- Open the project in Qt Creator
- Configure with CMake and build the project

### Configuration Settings
**JSON Files Configurations**
**- macOS**: Montiros `.plist` files in `/Library/Preferences`
**- Windows**: Monitors registy keys, such as `DoubleclickSpeed` and  `CursorBlinkRate`.
**AWS Configuration**
- **AWS SES**: Requires a verified email address to send alerts. This can be set up in the AWS Console under the SES service.
- **AWS SNS**: Will use a phone number for SMS notifications. Ensure the number is registered in AWS SNS.
- If you plan to use your own AWS Console account, remember to verify your email and phone number first.
- **MySQL Database**: Stores logs, user preferences, and alerts for historical tracking.

## Running the Project
Follow these steps to run the project:

### Step 1: Start the Application
- Run the application and choose your operatings system.

### Step 2: Start Monitoring 
- Click the button to initialize monitoring. Set your preferences for alerts in the UI.

### Step 3: Alert Preferences(Optional)
1. Enter your email and phone number in the GUI
2. Specify a threshold for non-critical alerts if needed.
3. Click **Save Settings** to update your preferences.
Once saved, alerts will be sent via SMS and email when critical registry key changes occur.

### Step 4: Search Historical Data (Optional)
1. Use the search bar in the application to filter and view logged changes.

## Testing the Project
Here are a few test cases to verify the project works as expected:

### Test Case 1: Registry Key Change Detection
1. Modify the wallpaper on the system (this is monitored by default).
2. The console and the log should display the change as non-critical.
3. Change the **DoubleClickSpeed** (a critical registry key).
4. The console should display `[CRITICAL]` and roll back the change automatically.

### Test Case 2: Email and SMS Alerts
1. Modify the **DoubleClickSpeed** key with email and phone settings saved.
2. You should receive an email and SMS alert indicating a critical change.

### Test Case 3: Rollback Functionality
1. Change the **DoubleClickSpeed** key.
2. After the alert, the value should automatically rollback to its original state.
3. Verify the rollback by checking the double-click speed setting on your system.

### Test Case 4: Bypass Rollback
1. Modify a critical registry key (e.g., **DoubleClickSpeed**).
2. When the rollback is triggered, immediately use the **Acknowledge** option in the UI to bypass the rollback.
3. Observe the registry key's value in the system after the change.

### Test Case 5: Settings Threshold for Non-Critical Alerts
1. Open the application's **Settings** and set a specific non-critical alert threshold (e.g., 3 changes).
2. Modify a non-critical registry key. such as `LimitBlankPasswordUse`, multiple times.
3. Count the number of modificaitons to confirm if an alert is sent only after reaching the specified threshold.

### Test Case 6: Cross-Platform Monitoring
1. Modify a .plist file on macOS or a critical registry key on Windows.
2. Confirm that changes are logged and alerts are triggered.

### Test Case 7: Cross-Platform Monitoring Page
1. Launch the program
2. Pick the operating system desired to be monitored.
3. The main monitoring page will be shown according to the chosen operating system.

### Test Case 8: Registry Key Change Detection
1. Modify the wallpaper on the system.
2. Confirm the change is logged in the MySQL database.

### Test Case 9: Database Logging
1. Perform multiple critical and non-critical changes.
2. Query the database to confirm the data is stored correctly.

### Test Case 10: Encryption Security
1. Check that sensitive data (e.g., credentials) is encrypted in logs.

### Test Case 11: Search Functionality
1. Use the application's search feature to query logs for specific changes.

## Code Structures
### Frontend (QML)
- **main.qml**: Defines the main QML user interface for the application, likely setting up navigation and the base layout.

- **WindowsDashboard.qml**: Defines the user interface for the Windows dashboard of the application, tailored to display and manage Windows-specific settings and data.

- **Welcome.qml**: Defines the user interface for the welcome page of the application, likely including navigation to other sections.

- **MacOSDashboard.qml**: Defines the user interface for the macOS dashboard of the application, providing layout and functionality specific to macOS.

### Backend (C++ with Qt)
- **main.cpp**: Contains the entry point for the application, initializing the program and starting the Qt application runtime.

- **registryKey.h/registryKey.cpp**: Implements functions for interacting with Windows registry keys, such as reading, writing, or monitoring changes.

- **monitoringBase.h/WindowsMonitoring.cpp/WindowsMonitoring.h/MacOSMonitoring.h/MacOSMonitoring.cpp**: Specifies the registry keys for Windows and plist files for macOS to be monitored, along with their criticality statuses.

- **WindowsRollback.h/WindowsRollback.cpp/MacOSRollback.cpp/MacOSRollback.h**: Implements the functionality for rolling back changes to registry keys or configurations when necessary, based on critical status or user settings.

- **alert.h/alert.cpp**:Manages alert notifications using AWS SNS for SMS alerts and AWS SES for email alerts. Alerts are sent based on registry key and .plist changes and configured thresholds.

- **settings.h/settings.cpp**: Manages user-configurable settings, such as alert thresholds and contact information (email and phone number).

- **registryKeyModel.h/registryKeyModel.cpp**: Provides the implementation for a model representing registry keys, likely used to manage and display registry data in the application.

- **WindowsJsonUtils.h//WindowsJsonUtils.cpp/MacOSJsonUtils.cpp/MacOSJsonUtils.h**: Implements utility functions for handling JSON data, likely for parsing, reading, and writing configurations or monitoring-related data.

- **CMakeLists.txt**: Configures the build process for the project, specifying dependencies, source files, and setup for Qt and AWS SDK integration

- **plist.cpp/plist.h**: Implements functions for interacting with macOS plist files, such as reading, writing, or parsing them for specific configuration values.

- **plistModel.cpp/plistModel.h**: Provides the implementation for a model representing plist files, likely used to manage and display plist data in the application.

- **Database.h/Database.cpp**: Handles MySQL database connections and queries.

- **EncryptionUtils.h/EncryptionUtils.cpp**: Implements encryption for sensitive data.

### JSON Configuration
- **awsconfig.json**: Stores AWS credentials for SES and SNS integrations.

- **monitoredKeys.json**: Lists registry keys to be monitored, including their paths, values, and critical status indicators.

- **monitoredPlists.json**: Specifies the registry keys for Windows and plist files for macOS to be monitored, along with their criticality statuses
