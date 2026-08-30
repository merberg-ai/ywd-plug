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
                Text { text: "> SCAN LIST DATABASE"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                Text { text: appController.codeplugReady ? appController.scanListCount + " LISTS // PRIORITY + MEMBERSHIP // READ ONLY" : "NO DECODED SCAN LIST IMAGE LOADED"; color: appController.codeplugReady ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Text { text: "SYS://RADIO/SCANLISTS"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
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
                        Text { Layout.preferredWidth: 46; text: "#"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 190; text: "SCAN LIST"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 76; text: "MEMBERS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 82; text: "HANG"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 120; text: "PRI 1"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 120; text: "PRI 2"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 130; text: "TX"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.fillWidth: true; text: "CHANNELS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }
                }

                ListView {
                    id: scanList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.scanLists
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        width: scanList.width
                        height: 42
                        color: index % 2 === 0 ? "#070809" : "#090b0d"
                        border.color: "#171a1d"
                        border.width: 1
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 0
                            Text { Layout.preferredWidth: 46; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 190; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 76; text: modelData.channelCount; color: modelData.channelCount > 0 ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 82; text: modelData.hangTime; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 120; text: modelData.priority1; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 120; text: modelData.priority2; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 130; text: modelData.designatedTx; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.fillWidth: true; text: modelData.channelsText.length > 0 ? modelData.channelsText : "--"; color: modelData.channelCount > 0 ? root.silver : root.silverDim; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                        }
                    }
                }
            }
        }

        Text { Layout.fillWidth: true; text: "READ-ONLY SCAN LIST VIEW // PRIORITY AND DESIGNATED-TX VALUES MIRROR THE RADIO IMAGE"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
    }
}
