import QtQuick

Rectangle {
    id: root

    property string text: "STANDBY"
    property bool good: false
    property bool busy: false
    property bool error: false

    implicitWidth: label.implicitWidth + 30
    implicitHeight: 26
    radius: 0
    color: "#070809"
    border.width: 1
    border.color: root.error ? "#e45c5c" : root.good ? "#58d878" : root.busy ? "#d79a2b" : "#4a4f54"

    Row {
        anchors.centerIn: parent
        spacing: 7

        Text {
            text: root.error ? "!" : root.good ? "+" : root.busy ? "*" : "-"
            color: root.error ? "#e45c5c" : root.good ? "#58d878" : root.busy ? "#d79a2b" : "#83898f"
            font.family: "Consolas"
            font.pixelSize: 11
            font.bold: true

            SequentialAnimation on opacity {
                running: root.busy
                loops: Animation.Infinite
                NumberAnimation { to: 0.25; duration: 360 }
                NumberAnimation { to: 1.0; duration: 360 }
            }
        }

        Text {
            id: label
            text: "[ " + root.text + " ]"
            color: root.error ? "#ff7373" : root.good ? "#7eea95" : root.busy ? "#f0b43c" : "#b8bdc2"
            font.family: "Consolas"
            font.pixelSize: 10
            font.bold: true
        }
    }
}
