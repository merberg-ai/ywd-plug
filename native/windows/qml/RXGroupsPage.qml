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
                Text { text: "> RX GROUP DATABASE"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                Text { text: appController.codeplugReady ? appController.rxGroupCount + " GROUPS // DMR RECEIVE MEMBERSHIP // READ ONLY" : "NO DECODED RX GROUP IMAGE LOADED"; color: appController.codeplugReady ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Text { text: "SYS://RADIO/RXGROUPS"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
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
                        Text { Layout.preferredWidth: 280; text: "RX GROUP"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 100; text: "MEMBERS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.fillWidth: true; text: "DMR / TALK GROUP IDS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }
                }

                ListView {
                    id: groupList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.rxGroups
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        width: groupList.width
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
                            Text { Layout.preferredWidth: 280; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: modelData.memberCount; color: modelData.memberCount > 0 ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.fillWidth: true; text: modelData.membersText.length > 0 ? modelData.membersText : "--"; color: modelData.memberCount > 0 ? root.silver : root.silverDim; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                        }
                    }
                }
            }
        }

        Text { Layout.fillWidth: true; text: "READ-ONLY RX GROUP VIEW // MEMBER IDS ARE THE RAW DMR / TALK GROUP IDENTIFIERS STORED BY THE RADIO"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
    }
}
