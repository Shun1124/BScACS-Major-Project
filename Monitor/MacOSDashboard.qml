import QtQuick 2.15 // Imports the QtQuick module for basic QML components
import QtQuick.Controls 2.15 // Imports the QtQuick.Controls module for UI controls like buttons and sliders
import QtQml 2.15 // Imports QtQml for interacting with QML elements in JavaScript

// Main Window
Window { // Creates the main application window
    id: window // Unique identifier for the window
    width: 800 // Sets the window width
    height: 600 // Sets the window height
    visible: false // Makes the window visible
    title: qsTr("System Configuration Monitor") // Sets the window title

    // Declare properties
    property string monitoringStatus: "Waiting..." // Initializes the monitoring status property with "Waiting..."
    property var logMessages: [] // Array to store log messages for the application
    property var criticalChanges: [] // Array to store critical changes notifications

    // Function to add a message to the log
    function addLog(message) { // Function that adds a new log entry and updates the display
        logMessages.push(message) // Appends the message to the logMessages array
        logArea.text = logMessages.join("\n") // Joins log messages with line breaks and updates the logArea
    }

    // Timer declaration for critical changes
    Timer { // Timer to clear critical changes after a set interval
        id: removalTimer // Unique identifier for the timer
        interval: 30000 // Timer interval set to 30 seconds
        repeat: false // Ensures the timer triggers only once
        running: false // Starts with the timer off
        onTriggered: { // Action when the timer triggers
            if (criticalChanges.length > 0) { // Checks if there are critical changes present
                criticalChanges = []; // Clears the critical changes array
                criticalChanges.push({ "changeText": "No changes detected yet." }); // Adds default message when cleared
                criticalChangesListView.model = criticalChanges; // Updates the ListView model with the cleared list
            }
        }
    }

    // Function to add a critical change to the list and start the timer
    function addCriticalChange(message) { // Function to add a critical change notification and start the timer
        if (typeof message !== 'string') { // Validates that the message is a string
            console.error("addCriticalChange expects a string message. Got:", message); // Logs an error if not a string
            return; // Exits function if validation fails
        }

        // Remove placeholder message if present
        for (let i = 0; i < criticalChanges.length; i++) { // Loops through criticalChanges to find placeholders
            if (criticalChanges[i].changeText === "No changes detected yet.") { // Checks for the placeholder message
                criticalChanges.splice(i, 1); // Removes the placeholder if found
                break; // Exits loop after removing
            }
        }

        let formattedMessage = "[CRITICAL] Rollback performed for file: " + message; // Formats the critical change message
        criticalChanges.push({ "changeText": message, "cancelled": false }); // Adds new critical change to array
        criticalChangesListView.model = criticalChanges; // Updates the ListView with new changes

        removalTimer.start(); // Starts the timer for resetting critical changes
    }

    // Connect C++ signal to the QML function
    Connections { // Manages connections between C++ signals and QML functions
        target: Monitoring // Links to the Monitoring object defined in C++
        function onCriticalChangeDetected(message) { // Function triggered when C++ emits the critical change signal
            addCriticalChange(message); // Calls addCriticalChange to handle the new message
        }
    }

    // Container for scrolling content
    ScrollView { // Main scrollable container for the application content
        anchors.fill: parent // Fills the parent window
        anchors.centerIn: parent // Centers the content in the parent window
        contentWidth: parent.width // Matches the content width to the parent window's width
        padding: 10 // Adds padding around the content
        spacing: 0 // Sets spacing between child items to zero

        // Main column to hold UI components
        Column { // Column layout for stacking UI components vertically
            width: parent.width * 0.9 // Sets width to 90% of the parent width
            spacing: 20 // Adds space between items in the column
            anchors.horizontalCenter: parent.horizontalCenter // Horizontally centers the column in the parent

            // Title text
            Text { // Text item for displaying the title
                text: "MacOS Configuration Monitor" // Sets the text to be displayed
                font.pointSize: 24 // Sets the font size to 24 points
                color: "green" // Sets the text color to green
                horizontalAlignment: Text.AlignHCenter // Horizontally centers the text within its bounds
                anchors.horizontalCenter: parent.horizontalCenter // Aligns the text to the horizontal center of parent
            }

            // Row for monitoring status and buttons
            Row { // Row layout to place status text and control buttons side-by-side
                anchors.horizontalCenter: parent.horizontalCenter // Centers the row horizontally in the parent
                spacing: 10 // Adds space between child items in the row

                Text { // Text item for displaying monitoring status
                    id: statusText // Unique identifier for the status text
                    text: "Status: " + monitoringStatus // Displays the current monitoring status
                    font.pointSize: 16 // Sets font size for the status text
                    color: "green" // Sets color for the status text
                }

                Button { // Button to start monitoring
                    text: "Start Monitoring" // Sets button label
                    onClicked: { // Action when button is clicked
                        Monitoring.startMonitoring() // Calls startMonitoring method in Monitoring object
                        monitoringStatus = "Monitoring..." // Updates status text to "Monitoring..."
                        addLog("[INFO] Monitoring started.") // Adds a log entry for monitoring start
                    }
                }

                Button { // Button to stop monitoring
                    text: "Stop Monitoring" // Sets button label
                    onClicked: { // Action when button is clicked
                        Monitoring.stopMonitoring() // Calls stopMonitoring method in Monitoring object
                        monitoringStatus = "Waiting..." // Updates status text to "Waiting..."
                        addLog("[INFO] Monitoring stopped.") // Adds a log entry for monitoring stop
                    }
                }
            }

            Row { // Row layout for holding columns for logs and critical changes
                id: mainRow // Unique identifier for the row
                width: parent.width * 0.9 // Sets width to 90% of the parent width
                spacing: 10 // Adds spacing between columns in the row
                anchors.horizontalCenter: parent.horizontalCenter // Centers the row horizontally

                Column { // Column for logs and critical changes
                    id: firstColumn // Unique identifier for the first column
                    width: (mainRow.width - mainRow.spacing) / 2 // Sets column width to half of the mainRow width minus spacing
                    spacing: 10 // Adds spacing between items in the column

                    Rectangle { // Container for log messages
                        width: firstColumn.width // Sets the rectangle width to match the first column
                        height: 150 // Sets height for the log container
                        color: "#F0F0F0" // Background color for the container
                        radius: 5 // Sets border radius for rounded corners
                        border.color: "#D3D3D3" // Sets border color
                        border.width: 1 // Sets border width

                        Text { // Label for logs section
                            text: "Logs:" // Sets label text
                            font.pointSize: 12 // Font size for the label
                            color: "black" // Sets color for the label text
                            topPadding: 10 // Padding at the top
                            leftPadding: 10 // Padding at the left
                        }

                        ScrollView { // Scrollable container for the log area
                            anchors.fill: parent // Fills the parent container
                            anchors.margins: 10 // Adds margins around the ScrollView
                            anchors.topMargin: 30

                            TextArea { // Text area to display log messages
                                id: logArea // Unique identifier for the log area
                                width: parent.width // Sets width to match parent
                                height: 100 // Sets height for the log area
                                readOnly: true // Makes the text area read-only
                                placeholderText: "[INFO] System monitoring initialized." // Placeholder text
                            }
                        }
                    }

                    Rectangle { // Container for displaying critical changes
                        width: firstColumn.width // Matches the first column width
                        color: "#F8F8F8" // Sets background color
                        radius: 5 // Rounded corners
                        border.color: "#D3D3D3" // Border color
                        border.width: 1 // Border width
                        height: 150 // Container height

                        Column { // Column for critical changes
                            anchors.fill: parent // Fills the parent container
                            anchors.margins: 10 // Adds margins around the content

                            Text { // Label for critical changes section
                                text: "Critical Changes:" // Sets the label text
                                font.pointSize: 12 // Sets font size
                                color: "black" // Text color
                            }

                            ScrollView { // Scrollable container for critical changes
                                width: parent.width // Matches width to parent
                                height: parent.height // Matches height to parent

                                ListView { // List view to display critical changes
                                    id: criticalChangesListView // Unique identifier for the ListView
                                    width: parent.width // Matches width to parent
                                    model: criticalChanges.length > 0 ? criticalChanges : [{"changeText": "No changes detected yet."}] // Sets model to criticalChanges or a default message if empty

                                    delegate: Rectangle { // Delegate defines how each item in the ListView will be displayed
                                        width: parent.width // Sets the width to match the ListView's width
                                        height: 40 // Sets height for each item in the ListView
                                        color: "#F7F7F7" // Background color for each item

                                        Row { // Row layout for displaying critical change text and button
                                            width: parent.width // Matches width to parent
                                            spacing: 10 // Spacing between child items
                                            anchors.verticalCenter: parent.verticalCenter // Vertically centers the Row within the parent

                                            Text { // Displays the text for the critical change
                                                text: modelData.changeText.split("\\").pop() || "" // Shows the last part of the plist path
                                                font.pointSize: 10 // Sets font size
                                                verticalAlignment: Text.AlignVCenter // Vertically centers the text
                                                elide: Text.ElideRight // Trims text with ellipsis if it’s too long
                                                width: parent.width * 0.6 // Sets the width to 60% of the parent width
                                            }

                                            Button { // Button for acknowledging the critical change
                                                text: "Acknowledge" // Button label
                                                visible: modelData.changeText !== "No changes detected yet." // Hides the button if it's the default message
                                                width: parent.width * 0.3 // Sets the width to 30% of the parent width
                                                onClicked: { // Action when button is clicked
                                                    let fileName = modelData.changeText.split("file: ")[1]; // Extracts the file name
                                                    if (fileName) {
                                                        Monitoring.allowChange(fileName.trim()); // Calls Monitoring to allow change for this file
                                                    }

                                                    addLog("[INFO] Acknowledged change for: " + fileName); // Logs acknowledgment of change
                                                    for (let i = 0; i < criticalChanges.length; i++) { // Loops through criticalChanges to remove acknowledged item
                                                        if (criticalChanges[i].changeText === modelData.changeText) {
                                                            criticalChanges.splice(i, 1); // Removes the acknowledged item
                                                            break; // Exits loop after removing
                                                        }
                                                    }

                                                    if (criticalChanges.length === 0) { // If no critical changes left, add default message
                                                        criticalChanges.push({ "changeText": "No changes detected yet." });
                                                    }

                                                    criticalChangesListView.model = criticalChanges; // Updates the ListView model
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar { // Vertical scrollbar for ListView
                                        policy: ScrollBar.AsNeeded // Shows scrollbar only when necessary
                                    }
                                }
                            }
                        }
                    }
                }

                Column { // Second column containing monitored plist files and settings
                    id: secondColumn // Unique identifier for the second column
                    width: (mainRow.width - mainRow.spacing) / 2 // Sets width to half of mainRow width minus spacing
                    spacing: 10 // Spacing between items in the column

                    Rectangle { // Container for monitored plist files
                        width: secondColumn.width // Matches the width of the second column
                        height: 150 // Sets height for the plist files container
                        color: "#F8F8F8" // Background color for the container
                        radius: 5 // Rounded corners
                        border.color: "#D3D3D3" // Border color
                        border.width: 1 // Border width

                        Column { // Column layout for plist files list
                            anchors.fill: parent // Fills the parent container
                            anchors.margins: 10 // Adds margins around the content

                            Text { // Label for the plist files section
                                text: "Monitored Configuration Files:" // Label text
                                font.pointSize: 12 // Font size
                                color: "black" // Text color
                                bottomPadding: 10 // Padding at the bottom
                            }

                            ScrollView { // Scrollable container for plist files list
                                id: plistScrollView // Unique identifier for the ScrollView
                                width: parent.width // Matches width to parent
                                height: parent.height - 40 // Height matches parent minus 40 pixels
                                anchors.horizontalCenter: parent.horizontalCenter // Horizontally centers within parent
                                clip: true // Clips contents to avoid overflow

                                ListView { // ListView to display plist files
                                    id: plistListView // Unique identifier for the ListView
                                    width: parent.width // Matches width to parent
                                    model: Monitoring ? Monitoring.plistFiles : [] // Sets model to plist files in Monitoring object

                                    delegate: Rectangle { // Delegate defining the display of each plist file item
                                        width: plistListView.width // Matches the ListView width
                                        height: 30 // Sets height for each plist file item
                                        color: index % 2 === 0 ? "#F7F7F7" : "#FFFFFF" // Alternates background color for items

                                        Row { // Row layout for each plist file item
                                            width: parent.width // Matches width to parent
                                            spacing: 10 // Spacing between child items
                                            padding: 5 // Padding around the Row

                                            CheckBox { // CheckBox to mark plist files as critical or non-critical
                                                font.pointSize: 12 // Font size for checkbox text
                                                text: model.displayText // Sets checkbox label to plist file name
                                                checked: model.isCritical // Sets initial checked state based on critical status
                                                onCheckedChanged: { // Action when checkbox state changes
                                                    Monitoring.setFileCriticalStatus(model.name, checked); // Calls method to update critical status
                                                    addLog("[INFO] " + model.name + (checked ? " marked as critical." : " marked as uncritical.")); // Logs the status change
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar { // Vertical scrollbar for the plist files ListView
                                        policy: ScrollBar.AsNeeded // Shows scrollbar only when necessary
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { // Container for alert settings
                        width: secondColumn.width // Matches width of the second column
                        color: "#F8F8F8" // Background color
                        radius: 5 // Rounded corners
                        border.color: "#D3D3D3" // Border color
                        border.width: 1 // Border width
                        height: 300 // Height of the settings container

                        Column { // Column layout for user information and alert settings
                            id: contentItem // Unique identifier for this content column
                            anchors.fill: parent // Fills the parent container
                            anchors.margins: 10 // Adds margins around the content
                            spacing: 20 // Space between child items
                            topPadding: 10 // Adds padding at the top

                            Text { // Label for the user information section
                                font.pointSize: 12 // Font size
                                text: "User Information" // Label text
                            }

                            TextField { // Text field for entering user email
                                id: emailField // Unique identifier for the email text field
                                placeholderText: "Enter your email" // Placeholder text in the field
                                width: parent.width // Matches width to parent
                                height: 30 // Sets height of the text field
                                topPadding: 7 // Padding at the top
                                leftPadding: 10 // Padding on the left
                                bottomPadding: 5 // Padding at the bottom
                            }

                            TextField { // Text field for entering user phone number
                                id: phoneField // Unique identifier for the phone number field
                                placeholderText: "Enter your phone number" // Placeholder text
                                width: parent.width // Matches width to parent
                                height: 30 // Sets height
                                topPadding: 7 // Padding at the top
                                leftPadding: 10 // Padding on the left
                            }

                            Text { // Label for the alert settings section
                                font.pointSize: 12 // Font size
                                text: "Alert Settings" // Label text
                            }

                            TextField { // Text field for entering threshold for non-critical alerts
                                id: nonCriticalAlertField // Unique identifier for this text field
                                placeholderText: "Enter the number of times before sending alert for non-critical changes." // Placeholder
                                width: parent.width // Matches width to parent
                                height: 30 // Sets height
                                topPadding: 7 // Padding at the top
                                leftPadding: 10 // Padding on the left
                            }

                            Button { // Button to save settings
                                text: "Save Settings" // Button label
                                width: 150 // Button width
                                anchors.horizontalCenter: parent.horizontalCenter // Centers button horizontally
                                onClicked: { // Action when button is clicked
                                    Settings.setEmail(emailField.text); // Sets email in Settings
                                    Settings.setPhoneNumber(phoneField.text); // Sets phone number in Settings
                                    Settings.setNonCriticalAlertThreshold(nonCriticalAlertField.text); // Sets alert threshold

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
