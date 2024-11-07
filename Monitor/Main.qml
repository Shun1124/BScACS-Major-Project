import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15

// Main Window
Window { // Creates the main application window
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("Windows Registry Monitor")

    // Declare properties
    property string monitoringStatus: "Waiting..."
    property var logMessages: []
    property var criticalChanges: []

    // Function to add a message to the log
    function addLog(message) {
        logMessages.push(message)
        logArea.text = logMessages.join("\n")
    }

    // Timer declaration for critical changes
    Timer {
        id: removalTimer
        interval: 30000
        repeat: false
        running: false
        onTriggered: { // Action when the timer triggers
            if (criticalChanges.length > 0) {
                criticalChanges = [];
                criticalChanges.push({ "changeText": "No changes detected yet." });
                criticalChangesListView.model = criticalChanges;
            }
        }
    }

    // Function to add a critical change to the list and start the timer
    function addCriticalChange(message) {
        if (typeof message !== 'string') {
            console.error("addCriticalChange expects a string message. Got:", message);
            return; // Exits function if validation fails
        }

        // Remove placeholder message if present
        for (let i = 0; i < criticalChanges.length; i++) {
            if (criticalChanges[i].changeText === "No changes detected yet.") {
                criticalChanges.splice(i, 1);
                break;
            }
        }

        let formattedMessage = "[CRITICAL] Rollback performed for key: " + message;
        criticalChanges.push({ "changeText": message, "cancelled": false });
        criticalChangesListView.model = criticalChanges; // Updates the ListView with new changes

        removalTimer.start(); // Starts the timer for resetting critical changes
    }

    // Connect C++ signal to the QML function
    Connections {
        target: Monitoring
        function onCriticalChangeDetected(message) {
            addCriticalChange(message);
        }
    }

    // Container for scrolling content
    ScrollView { // Main scrollable container for the application content
        anchors.fill: parent
        anchors.centerIn: parent
        contentWidth: parent.width
        padding: 10
        spacing: 0

        // Main column to hold UI components
        Column { // Column layout for stacking UI components vertically
            width: parent.width * 0.9
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            // Title text
            Text { // Text item for displaying the title
                text: "Windows Registry Monitor"
                font.pointSize: 24
                color: "green"
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Row for monitoring status and buttons
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                Text { // Text item for displaying monitoring status
                    id: statusText
                    text: "Status: " + monitoringStatus
                    font.pointSize: 16
                    color: "green"
                }

                Button { // Button to start monitoring
                    text: "Start Monitoring"
                    onClicked: { // Action when button is clicked
                        Monitoring.startMonitoring()
                        monitoringStatus = "Monitoring..."
                        addLog("[INFO] Monitoring started.")
                    }
                }

                Button { // Button to stop monitoring
                    text: "Stop Monitoring"
                    onClicked: { // Action when button is clicked
                        Monitoring.stopMonitoring()
                        monitoringStatus = "Waiting..."
                        addLog("[INFO] Monitoring stopped.")
                    }
                }
            }

            Row {
                id: mainRow
                width: parent.width * 0.9
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                Column { // Column for logs and critical changes
                    id: firstColumn
                    width: (mainRow.width - mainRow.spacing) / 2
                    spacing: 10

                    Rectangle { // Container for log messages
                        width: firstColumn.width
                        height: 150
                        color: "#F0F0F0"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1

                        Text { // Label for logs section
                            text: "Logs:"
                            font.pointSize: 12
                            color: "black"
                            topPadding: 10
                            leftPadding: 10
                        }

                        ScrollView { // Scrollable container for the log area
                            anchors.fill: parent
                            anchors.margins: 10
                            anchors.topMargin: 30

                            TextArea { // Text area to display log messages
                                id: logArea
                                width: parent.width
                                height: 100
                                readOnly: true
                                placeholderText: "[INFO] System monitoring initialized."
                            }
                        }
                    }

                    Rectangle { // Container for displaying critical changes
                        width: firstColumn.width
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1
                        height: 150

                        Column { // Column for critical changes
                            anchors.fill: parent
                            anchors.margins: 10

                            Text { // Label for critical changes section
                                text: "Critical Changes:"
                                font.pointSize: 12
                                color: "black"
                            }

                            ScrollView { // Scrollable container for critical changes
                                width: parent.width
                                height: parent.height

                                ListView {
                                    id: criticalChangesListView
                                    width: parent.width
                                    model: criticalChanges.length > 0 ? criticalChanges : [{"changeText": "No changes detected yet."}]

                                    delegate: Rectangle { // Delegate defines how each item in the ListView will be displayed
                                        width: parent.width
                                        height: 40
                                        color: "#F7F7F7"

                                        Row { // Row layout for displaying critical change text and button
                                            width: parent.width
                                            spacing: 10
                                            anchors.verticalCenter: parent.verticalCenter

                                            Text { // Displays the text for the critical change
                                                text: modelData.changeText.split("\\").pop() || ""
                                                font.pointSize: 10
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                                width: parent.width * 0.6
                                            }

                                            Button { // Button for acknowledging the critical change
                                                text: "Acknowledge"
                                                visible: modelData.changeText !== "No changes detected yet."
                                                width: parent.width * 0.3
                                                onClicked: { // Action when button is clicked
                                                    let keyName = modelData.changeText.split("key: ")[1]; // Extracts the key name
                                                    if (keyName) {
                                                        Monitoring.allowChange(keyName.trim());
                                                    }

                                                    addLog("[INFO] Acknowledged change for: " + keyName); // Logs acknowledgment of change
                                                    for (let i = 0; i < criticalChanges.length; i++) { // Loops through criticalChanges to remove acknowledged item
                                                        if (criticalChanges[i].changeText === modelData.changeText) {
                                                            criticalChanges.splice(i, 1);
                                                            break; 
                                                        }
                                                    }

                                                    if (criticalChanges.length === 0) { // If no critical changes left, add default message
                                                        criticalChanges.push({ "changeText": "No changes detected yet." });
                                                    }

                                                    criticalChangesListView.model = criticalChanges; 
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar { // Vertical scrollbar for ListView
                                        policy: ScrollBar.AsNeeded
                                    }
                                }
                            }
                        }
                    }
                }

                Column { // Second column containing monitored registry keys and settings
                    id: secondColumn
                    width: (mainRow.width - mainRow.spacing) / 2
                    spacing: 10

                    Rectangle { // Container for monitored registry keys
                        width: secondColumn.width
                        height: 150
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1

                        Column { // Column layout for registry keys list
                            anchors.fill: parent
                            anchors.margins: 10

                            Text { // Label for the registry keys section
                                text: "Monitored Registry Keys:"
                                font.pointSize: 12
                                color: "black"
                                bottomPadding: 10
                            }

                            ScrollView { // Scrollable container for registry keys list
                                id: registryScrollView
                                width: parent.width
                                height: parent.height - 40
                                anchors.horizontalCenter: parent.horizontalCenter
                                clip: true

                                ListView { // ListView to display registry keys
                                    id: keyListView
                                    width: parent.width
                                    model: Monitoring ? Monitoring.registryKeys : []

                                    delegate: Rectangle { // Delegate defining the display of each registry key item
                                        width: keyListView.width
                                        height: 30
                                        color: index % 2 === 0 ? "#F7F7F7" : "#FFFFFF"

                                        Row { // Row layout for each registry key item
                                            width: parent.width
                                            spacing: 10
                                            padding: 5

                                            CheckBox { // CheckBox to mark registry keys as critical or non-critical
                                                font.pointSize: 12
                                                text: model.displayText
                                                checked: model.isCritical
                                                onCheckedChanged: { // Action when checkbox state changes
                                                    Monitoring.setKeyCriticalStatus(model.name, checked);
                                                    addLog("[INFO] " + model.name + (checked ? " marked as critical." : " marked as uncritical."));
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar { // Vertical scrollbar for the registry keys ListView
                                        policy: ScrollBar.AsNeeded
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { // Container for alert settings
                        width: secondColumn.width
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1
                        height: 300

                        Column { // Column layout for user information and alert settings
                            id: contentItem
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 20
                            topPadding: 10

                            Text { // Label for the user information section
                                font.pointSize: 12
                                text: "User Information"
                            }

                            TextField { // Text field for entering user email
                                id: emailField
                                placeholderText: "Enter your email"
                                width: parent.width
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                                bottomPadding: 5
                            }

                            TextField { // Text field for entering user phone number
                                id: phoneField
                                placeholderText: "Enter your phone number"
                                width: parent.width
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                            }

                            Text { // Label for the alert settings section
                                font.pointSize: 12
                                text: "Alert Settings"

                            TextField { // Text field for entering threshold for non-critical alerts
                                id: nonCriticalAlertField
                                placeholderText: "Enter the number of times before sending alert for non-critical changes."
                                width: parent.width
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                            }

                            Button { // Button to save settings
                                text: "Save Settings"
                                width: 150
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: { // Action when button is clicked
                                    Settings.setEmail(emailField.text);
                                    Settings.setPhoneNumber(phoneField.text);
                                    Settings.setNonCriticalAlertThreshold(nonCriticalAlertField.text);

                                    addLog("[INFO] Settings saved: Email - " + emailField.text + ", Phone - " + phoneField.text
                                           + ", Threshold - " + nonCriticalAlertField.text); // Logs settings save confirmation
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
