import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: welcomeWindow
    width: 800
    height: 600
    title: "Welcome"
    visible: true // Show this window initially

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Choose Your Operating System"
            font.pixelSize: 24
        }

        Button {
            text: "Windows"
            onClicked: {
                console.log("Opening Windows Dashboard");
                let windows = windowsDashboardPage.createObject();
                if (windows) {
                    windows.show(); // Show Windows Dashboard
                    welcomeWindow.close(); // Close Welcome page
                } else {
                    console.log("Error: Failed to create Windows Dashboard");
                }
            }
        }

        Button {
            text: "macOS"
            onClicked: {
                console.log("Opening macOS Dashboard");
                let mac = macosDashboardPage.createObject();
                if (mac) {
                    mac.show(); // Show macOS Dashboard
                    welcomeWindow.close(); // Close Welcome page
                } else {
                    console.log("Error: Failed to create macOS Dashboard");
                }
            }
        }
    }
}
