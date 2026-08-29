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
        color: !control.enabled ? "#4a3610" : control.active ? "#ffd166" : control.hovered ? "#ffb000" : "#a87818"
        font.family: "Consolas"
        font.pixelSize: 12
        font.bold: control.active
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 0
        color: control.active ? "#171004" : control.hovered ? "#100b02" : "transparent"
        border.width: control.active ? 1 : control.hovered ? 1 : 0
        border.color: control.active ? "#ffb000" : "#5c3d00"

        Rectangle {
            visible: control.active
            width: 2
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: "#ffd166"
        }
    }
}
