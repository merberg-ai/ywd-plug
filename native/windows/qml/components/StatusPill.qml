import QtQuick

Rectangle {
    id: root

    property string text: "READY"
    property bool good: false
    property bool busy: false

    implicitWidth: label.implicitWidth + 34
    implicitHeight: 28
    radius: 14
    color: good ? "#10271f" : busy ? "#16222b" : "#101820"
    border.width: 1
    border.color: good ? "#6bf4a5" : busy ? "#62e9ff" : "#2a6678"

    Row {
        anchors.centerIn: parent
        spacing: 8

        Rectangle {
            width: 7
            height: 7
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: root.good ? "#6bf4a5" : root.busy ? "#62e9ff" : "#7ea3ad"

            SequentialAnimation on opacity {
                running: root.busy
                loops: Animation.Infinite
                NumberAnimation { to: 0.25; duration: 420 }
                NumberAnimation { to: 1.0; duration: 420 }
            }
        }

        Text {
            id: label
            text: root.text
            color: root.good ? "#aaf7c8" : root.busy ? "#bdf7ff" : "#9cb9c1"
            font.family: "Consolas"
            font.pixelSize: 11
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
