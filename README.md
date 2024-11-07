# COMP 8800 Major Project - Windows Registry Monitor Prototype

This project monitors Windows registry keys for changes, detects critical and non-critical changes, and sends alerts via email and SMS using AWS services (SES and SNS). It also provides a rollback feature for critical registry changes. The project consists of a C++ backend for monitoring registry changes and an Electron-based frontend for displaying log information and managing user settings.

## Table of Contents
[Features](#features)\
[Prerequisites](#prerequisites)\
[Installation Instructions](#installation-instructions)\
[Configuration Settings](#configuration-settings)\
[Testing the Project](#testing-the-project)\
[Code Structure](#code-structure)

## Features
**Real-Time Monitoring**: Monitors critical registry keys using `RegNotifyChangeKeyValue`. Detected changes are logged and processed immediately.

**Rollback System**:  Critical changes are reverted within 5 seconds using `RegSetValueEx`, ensuring system integrity is maintained.

**Customizable Alerts**: Alerts are sent to system administrators within 5 seconds of detecting critical changes, ensuring timely intervention.

## Prerequisites
### Tools and Technologies Required
#### Windows System
**Operating System**: Windows 10 or later (64-bit).

**Development Environment**: Qt Creator

**AWS Account** (Optional): For sending email and SMS alerts through SES (Simple Email Service) and SNS (Simple Notification Service). 

#### Libraries and Frameworks
**Qt 6.5 or later**: For the graphical user interface and integration.

**AWS SDK for C++**: Used for sending emails and SMS via AWS SES and SNS.

**CMake**: For building the project.

### Installation Instructions
#### Step 1: Install AWS SDK for C++
1. Follow the link to download and install the AWS SDK for C++ from (https://github.com/aws/aws-sdk-cpp).

#### Step 2: Install Qt Creator
- Download Qt 6.5 or later from (https://doc.qt.io/qtcreator/)
- Include `Qt Quick` components for QML

#### Step 3: Clone the Repository
Clone the project repository to your local machine
```
git clone https://github.com/Shun1124/BScACS-Major-Project.git
```

#### Step 4: Set up AWS Services (Optional)
- Specific testing email and password is provided in the prototype report, therefore, this step is optional.
- Signup for an AWS account and set up SES and SNS.
- **SES**: Verify your email address (required by AWS SES to send emails).
- **SNS**: Register and set up SMS service in the AWS console.

#### Step 5: Set Up AWS Credentials
Create a  `awsconfig.json` file in the `Monitor` folder that contains access key information for a restricted IAM user with full access to SNS and SES. See the report for access key information. If you want to use your own AWS account instead, you can modify the existing `awsconfig.json` with your own credentials in the `Backend` directory with your own access information.
##### Inside the awsconfig.json file:
```
{
  "accessKeyId": "YOUR_ACCESS_KEY_ID",
  "secretAccessKey": "YOUR_SECRET_ACCESS_KEY",
  "region": "YOUR_AWS_REGION"
}
```

#### Step 6: Build the Project
- Open the project in Qt Creator
- Configure with CMake and build the project

### Configuration Settings
**AWS Configuration**
- **AWS SES**: Requires a verified email address to send alerts. This can be set up in the AWS Console under the SES service.
- **AWS SNS**: Will use a phone number for SMS notifications. Ensure the number is registered in AWS SNS.
- If you plan to use your own AWS Console account, remember to verify your email and phone number first.

## Running the Project
Follow these steps to run the project:

### Step 1: Start the Application
- Run the application to initialize monitoring. Set your preferences for alerts in teh UI.

### Step 2: Alert Preferences(Optional)
1. Enter your email and phone number in the GUI
2. Specify a threshold for non-critical alerts if needed.
3. Click **Save Settings** to update your preferences.
Once saved, alerts will be sent via SMS and email when critical registry key changes occur.

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
2. When the rollback is triggered, immediately use the **Acknowledge** option in teh UI to bypass the rollback.
3. Observe the registry key's value in the system after the change.

### Test Case 5: Settings Threshold for Non-Critical Alerts
1. Open the application's **Settings** and set a specific non-critical alert threshold (e.g., 3 changes).
2. Modify a non-critical registry key. such as `LimitBlankPasswordUse`, multiple times.
3. Count the number of modificaitons to confirm if an alert is sent only after reaching the specified threshold.

## Code Structures
### Frontend (QML)
- **main.qml**: The main user interface, displaying logs, alerts, and registry key statuses. Allows users to interact with monitoring settings and alert configurations.

### Backend (C++ with Qt)
- **main.cpp**: Initializes the AWS SDK and application settings, loads the QML UI, and exposes core components to the QML interface.

- **registryKey.h/registryKey.cpp**: Represents each registry key being monitored. Tracks key values, state, critical status, and manages registry interactions.

- **monitoring.h/monitoring.cpp**: Contains the core monitoring logic. Sets up timers to check for registry changes and triggers alerts or rollbacks based on the key's status.

- **rollback.h/rollback.cpp**: Implements rollback mechanisms for critical registry keys, ensuring their values revert to the previous state when changes are detected.

- **alert.h/alert.cpp**:Manages alert notifications using AWS SNS for SMS alerts and AWS SES for email alerts. Alerts are sent based on registry key changes and configured thresholds.

- **settings.h/settings.cpp**: Manages user-configurable settings, such as alert thresholds and contact information (email and phone number).

- **registryKeyModel.h/registryKeyModel.cpp**: Defines a custom model to represent registry keys in the UI. Custom roles (e.g., `NameRole`, `IsCriticalRole`, `DisplayTextRole`) expose data points for dynamic display and management in the QML interface.

- **jsonUtils.h//jsonUtils.cpp**: Contains utility functions to read and parse JSON data. Loads registry key information from `monitoredKeys.json` at startup.

- **CMakeLists.txt**: Configures the project's build process. It sets paths for the AWS SDK, defines dependencies for the Qt and AWS SDK components, organizes source files, and registers JSON configuration files. Essential for compiling and linking project components effectively.

### JSON Configuration
- **awsconfig.json**: Stores AWS credentials for SES and SNS integrations.

- **monitoredKeys.json**: Lists registry keys to be monitored, including their paths, values, and critical status indicators.
