import QtQuick 2.15
import QtQuick.Controls 2.15

QtObject {
    // Dynamically load and show the Welcome page
    Component.onCompleted: {
        // Create and display the Welcome window as the starting page
        let welcomeWindow = welcomePage.createObject();
        if (welcomeWindow) {
            welcomeWindow.show();
        } else {
            console.log("Error: Failed to create Welcome page");
        }
    }

    // Define the Welcome page
    property Component welcomePage: Component {
        Welcome {}
    }

    // Define the Windows Dashboard page
    property Component windowsDashboardPage: Component {
        WindowsDashboard {}
    }

    // Define the macOS Dashboard page
    property Component macosDashboardPage: Component {
        MacOSDashboard {}
    }
}
