import QtQuick
import QtQuick.Controls

Button {
    id: control

    property bool active: false

    implicitHeight: 42
    leftPadding: 14
    rightPadding: 14

    contentItem: Text {
        text: control.text
        color: control.active ? "#62e9ff" : control.hovered ? "#d8edf2" : "#87a8b1"
        font.family: "Consolas"
        font.pixelSize: 13
        font.bold: control.active
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 8
        color: control.active ? "#102934" : control.hovered ? "#0d2028" : "transparent"
        border.width: control.active ? 1 : 0
        border.color: "#2a6678"

        Rectangle {
            visible: control.active
            width: 3
            radius: 2
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 7
            color: "#62e9ff"
        }
    }
}
