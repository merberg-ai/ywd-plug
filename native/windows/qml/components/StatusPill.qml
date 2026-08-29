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
    color: "#050502"
    border.width: 1
    border.color: root.error ? "#ff4d4d" : root.good ? "#55ff77" : root.busy ? "#ffb000" : "#5c3d00"

    Row {
        anchors.centerIn: parent
        spacing: 7

        Text {
            text: root.error ? "!" : root.good ? "+" : root.busy ? "*" : "-"
            color: root.error ? "#ff4d4d" : root.good ? "#55ff77" : root.busy ? "#ffb000" : "#8a5d00"
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
            color: root.error ? "#ff6b6b" : root.good ? "#7cff94" : root.busy ? "#ffd166" : "#a87818"
            font.family: "Consolas"
            font.pixelSize: 10
            font.bold: true
        }
    }
}
