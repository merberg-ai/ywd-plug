import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Text { text: "> CONTACT DATABASE"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
            Item { Layout.fillWidth: true }
            Text { text: "SYS://RADIO/CONTACTS"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#070809"
            border.color: root.lineStrong
            border.width: 1

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                Text { Layout.alignment: Qt.AlignHCenter; text: "[ CONTACT READER NOT YET ENABLED ]"; color: root.amber; font.family: "Consolas"; font.pixelSize: 14; font.bold: true }
                Text { Layout.alignment: Qt.AlignHCenter; text: "CONTACTS USE A SEPARATE V-FRAME MEMORY REGION AND REMAIN RESERVED FOR THE NEXT READ-ONLY PHASE"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
            }
        }
    }
}
