import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15

// Main Window
Window {
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
        interval: 30000  // 1 minutes
        repeat: false
        running: false
        onTriggered: {
            if (criticalChanges.length > 0) {
                criticalChanges = [];
                criticalChanges.push({ "changeText": "No changes detected yet." });
                criticalChangesListView.model = criticalChanges;
            }
        }
    }

    // Function to add a critical change to the list and start the timer
    function addCriticalChange(message) {
        //console.log("Adding critical change:", message);
        if (typeof message !== 'string') {
            console.error("addCriticalChange expects a string message. Got:", message);
            return;
        }

        // Remove the placeholder message if it exists
        for (let i = 0; i < criticalChanges.length; i++) {
            if (criticalChanges[i].changeText === "No changes detected yet.") {
                criticalChanges.splice(i, 1);
                break;
            }
        }

        // Format and update message
        let formattedMEssage = "[CRITICAL] Rollback performed for key: " + message.split("\\").pop();
        criticalChanges.push({ "changeText": message, "cancelled": false });
        console.log("Current criticalChanges array:", JSON.stringify(criticalChanges));
        criticalChangesListView.model = criticalChanges;

        // Start the timer
        removalTimer.start();
    }

    // Connect C++ signal to the QML function
    Connections {
        target: Monitoring  // Replace with the correct ID or object name if necessary
        function onCriticalChangeDetected(message) {
            console.log("onCriticalChangeDetected triggered with message: ", message);
            addCriticalChange(message);
        }
    }

    // Container for scrolling content
    ScrollView {
        anchors.fill: parent
        anchors.centerIn: parent
        contentWidth: parent.width
        padding: 10
        spacing: 0

        // Main column to holder UI components
        Column {
            width: parent.width * 0.9
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            // Title text
            Text {
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

                // Status display text
                Text {
                    id: statusText
                    text: "Status: " + monitoringStatus
                    font.pointSize: 16
                    color: "green"
                }

                // Button to start monitoring
                Button {
                    text: "Start Monitoring"
                    onClicked: {
                        Monitoring.startMonitoring()
                        monitoringStatus = "Monitoring..."
                        addLog("[INFO] Monitoring started.")
                    }
                }

                // Button to stop monitoring
                Button {
                    text: "Stop Monitoring"
                    onClicked: {
                        Monitoring.stopMonitoring()
                        monitoringStatus = "Waiting..."
                        addLog("[INFO] Monitoring stopped.")
                    }
                }
            }

            // Row to hold columns for logs and critical changes
            Row {
                id: mainRow
                width: parent.width * 0.9
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                // First column for logs and critical changes
                Column {
                    id: firstColumn
                    width: (mainRow.width - mainRow.spacing) / 2
                    spacing: 10


                    // Containers for the logs
                    Rectangle {
                        width: firstColumn.width
                        height: 150
                        color: "#F0F0F0"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1

                        // Label for the logs
                        Text {
                            text: "Logs:"
                            font.pointSize: 12
                            color: "black"
                            topPadding: 10
                            leftPadding: 10
                        }

                        // Column for the log text
                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            anchors.topMargin: 30

                            ScrollView {
                                anchors.fill: parent
                                anchors.margins: 10

                                // Text area for displaying log messages
                                TextArea {
                                    id: logArea
                                    width: parent.width
                                    height: 100
                                    readOnly: true
                                    placeholderText: "[INFO] System monitoring initialized."
                                }
                            }
                        }
                    }

                    // Container for the critical changes
                    Rectangle {
                        width: firstColumn.width
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1
                        height: 150

                        // Column for the critical changes
                        Column {
                            anchors.fill: parent
                            anchors.margins: 10

                            // Display critical changes
                            Text {
                                text: "Critical Changes:"
                                font.pointSize: 12
                                color: "black"
                            }

                            //
                            ScrollView {
                                width: parent.width
                                height: parent.height
                                anchors.horizontalCenter: parent.horizontalCenter

                                ListView {
                                    id: criticalChangesListView
                                    width: parent.width
                                    model: criticalChanges.length > 0 ? criticalChanges : [{"changeText": "No changes detected yet."}]

                                    delegate: Rectangle {
                                        width: parent.width
                                        height: 40
                                        color: "#F7F7F7"

                                        Row {
                                            width: parent.width
                                            spacing: 10
                                            anchors.verticalCenter: parent.verticalCenter

                                            Text {
                                                text: modelData.changeText.split("\\").pop() || ""  // Display the critical change text
                                                font.pointSize: 10
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                                width: parent.width * 0.6
                                            }

                                            // Only show the button if there is an actual change detected
                                            Button {
                                                text: "Acknowledge"
                                                visible: modelData.changeText !== "No changes detected yet."
                                                width: parent.width * 0.3
                                                onClicked: {
                                                    let keyName = modelData.changeText.split("key: ")[1];  // Extract the key name
                                                    if (keyName) {
                                                        Monitoring.allowChange(keyName.trim());  // Call with the trimmed key name
                                                    }

                                                    // Log and remove acknowledged change
                                                    addLog("[INFO] Acknowledged change for: " + keyName);
                                                    for (let i = 0; i < criticalChanges.length; i++) {
                                                        if (criticalChanges[i].changeText === modelData.changeText) {
                                                            criticalChanges.splice(i, 1);
                                                            break;
                                                        }
                                                    }

                                                    if (criticalChanges.length === 0) {
                                                        criticalChanges.push({ "changeText": "No changes detected yet." });
                                                    }

                                                    criticalChangesListView.model = criticalChanges;
                                                }
                                            }

                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar {
                                        policy: ScrollBar.AsNeeded
                                    }
                                }
                            }
                        }
                    }
                }

                Column {
                    id: secondColumn
                    width: (mainRow.width - mainRow.spacing) /2
                    spacing: 10

                    Rectangle {
                        width: secondColumn.width
                        height: 150
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10

                            Text {
                                text: "Monitored Registry Keys:"
                                font.pointSize: 12
                                color: "black"
                                bottomPadding: 10
                            }

                            ScrollView {
                                id: registryScrollView
                                width: parent.width
                                height: parent.height - 40
                                anchors.horizontalCenter: parent.horizontalCenter
                                clip: true

                                ListView {
                                    id: keyListView
                                    width: parent.width
                                    model: Monitoring.registryKeys

                                    delegate: Rectangle {
                                        width: keyListView.width
                                        height: 30
                                        color: index % 2 === 0 ? "#F7F7F7" : "#FFFFFF"

                                        Row {
                                            width: parent.width
                                            spacing: 10
                                            padding: 5

                                            CheckBox {
                                                font.pointSize: 12
                                                text: model.displayText
                                                checked: model.isCritical
                                                onCheckedChanged: {
                                                    Monitoring.setKeyCriticalStatus(model.name, checked)
                                                    addLog("[INFO] " + model.name + (checked ? " marked as critical." : " marked as uncritical."))
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar {
                                        policy: ScrollBar.AsNeeded
                                    }
                                }
                            }
                        }
                    }

                    // Alert Settings Section
                    Rectangle {
                        width: secondColumn.width
                        color: "#F8F8F8"
                        radius: 5
                        border.color: "#D3D3D3"
                        border.width: 1
                        height: 300
                        Column {
                            id: contentItem
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 20
                            topPadding: 10

                            Text {
                                font.pointSize: 12
                                text: "User Information"
                            }

                            TextField {
                                id: emailField
                                placeholderText: "Enter your email"
                                width: parent.width * 1
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                                bottomPadding: 5
                            }

                            TextField {
                                id: phoneField
                                placeholderText: "Enter your phone number"
                                width: parent.width * 1
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                            }

                            Text {
                                font.pointSize: 12
                                text: "Alert Settings"
                            }

                            TextField {
                                id: nonCriticalAlertField
                                placeholderText: "Enter the number of times before sending alert for non-critical changes."
                                width: parent.width * 1
                                height: 30
                                topPadding: 7
                                leftPadding: 10
                            }

                            Button {
                                text: "Save Settings"
                                width: 150
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: {
                                    // Call the methods in Settings instead of setting properties directly
                                    Settings.setEmail(emailField.text);
                                    Settings.setPhoneNumber(phoneField.text);
                                    Settings.setNonCriticalAlertThreshold(nonCriticalAlertField.text);

                                    addLog("[INFO] Settings saved: Email - " + emailField.text + ", Phone - " + phoneField.text
                                           + ", Threshold - " + nonCriticalAlertField.text);
                                    console.log("Settings saved: Email - " + emailField.text + ", Phone - " + phoneField.text
                                                + ", Threshold - " + nonCriticalAlertField.text);
                                }
                            }

                        }
                    }
                }
            }
        }
    }
}


