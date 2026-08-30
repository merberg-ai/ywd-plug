import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property color panel: "#0b0d0f"
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color line: "#30353a"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"
    property color green: "#58d878"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 3
                Text { text: "> ZONE DATABASE"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                Text { text: appController.codeplugReady ? appController.zoneCount + " ZONES // CHANNEL MEMBERSHIP // READ ONLY" : "NO DECODED ZONE IMAGE LOADED"; color: appController.codeplugReady ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Text { text: "SYS://RADIO/ZONES"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#070809"
            border.color: root.lineStrong
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    color: root.panel
                    border.color: root.line
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 0
                        Text { Layout.preferredWidth: 54; text: "#"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 260; text: "ZONE"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 100; text: "MEMBERS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.fillWidth: true; text: "CHANNELS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }
                }

                ListView {
                    id: zoneList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.zones
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        width: zoneList.width
                        height: 42
                        color: index % 2 === 0 ? "#070809" : "#090b0d"
                        border.color: "#171a1d"
                        border.width: 1
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 0
                            Text { Layout.preferredWidth: 54; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 260; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: modelData.channelCount; color: modelData.channelCount > 0 ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.fillWidth: true; text: modelData.channelsText.length > 0 ? modelData.channelsText : "--"; color: modelData.channelCount > 0 ? root.silver : root.silverDim; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                        }
                    }
                }
            }
        }

        Text { Layout.fillWidth: true; text: "READ-ONLY ZONE VIEW // CHANNEL NUMBERS REFER TO THE CHANNEL DATABASE"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
    }
}
