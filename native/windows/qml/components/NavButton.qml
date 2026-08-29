import QtQuick
import QtQuick.Controls

Button {
    id: control

    property bool active: false
    property string indexText: "--"

    implicitHeight: 38
    leftPadding: 10
    rightPadding: 10

    contentItem: Text {
        text: (control.active ? "> " : "  ") + control.indexText + "  " + control.text.toUpperCase()
        color: !control.enabled ? "#3f4449" : control.active ? "#f0b43c" : control.hovered ? "#d9dde1" : "#8a9096"
        font.family: "Consolas"
        font.pixelSize: 12
        font.bold: control.active
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 0
        color: control.active ? "#11100c" : control.hovered ? "#0d0f11" : "transparent"
        border.width: control.active || control.hovered ? 1 : 0
        border.color: control.active ? "#d79a2b" : "#3b4045"

        Rectangle {
            visible: control.active
            width: 2
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: "#f0b43c"
        }
    }
}
