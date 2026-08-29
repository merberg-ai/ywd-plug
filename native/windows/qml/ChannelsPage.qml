import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property color black: "#050607"
    property color panel: "#0b0d0f"
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color line: "#30353a"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"
    property color green: "#58d878"
    property color muted: "#676d73"

    function frequencyText(value, disabled) {
        if (disabled)
            return "RX ONLY"
        return Number(value).toFixed(5).replace(/0+$/, "").replace(/\.$/, "")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 14

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 3
                Text {
                    text: "> CHANNEL DATABASE"
                    color: root.silverBright
                    font.family: "Consolas"
                    font.pixelSize: 21
                    font.bold: true
                }
                Text {
                    text: appController.channelsReady
                          ? appController.channelCount + " RECORDS // NATIVE RAW-IMAGE DECODER // READ ONLY"
                          : "NO DECODED CHANNEL IMAGE LOADED"
                    color: appController.channelsReady ? root.green : root.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 10
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "SYS://RADIO/CHANNELS"
                color: root.silverDim
                font.family: "Consolas"
                font.pixelSize: 10
            }
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
                        Text { Layout.preferredWidth: 220; text: "NAME"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 130; text: "RX MHz"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 130; text: "TX MHz"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 120; text: "MODE"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 86; text: "POWER"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 90; text: "BW"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 80; text: "CC / TS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.fillWidth: true; text: "TX IDX"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }
                }

                ListView {
                    id: channelList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.channels
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar { }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index

                        width: channelList.width
                        height: 34
                        color: index % 2 === 0 ? "#070809" : "#090b0d"
                        border.color: "#171a1d"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 0

                            Text { Layout.preferredWidth: 54; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 220; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 130; text: root.frequencyText(modelData.rxFrequency, false); color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 130; text: root.frequencyText(modelData.txFrequency, modelData.txDisabled); color: modelData.txDisabled ? root.silverDim : root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 120; text: modelData.mode; color: modelData.mode.indexOf("Digital") >= 0 ? root.green : root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 86; text: modelData.power; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 90; text: modelData.bandwidth; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text {
                                Layout.preferredWidth: 80
                                text: modelData.colorCode >= 0 ? ("CC" + modelData.colorCode + " / TS" + modelData.timeSlot) : "--"
                                color: modelData.colorCode >= 0 ? root.green : root.silverDim
                                font.family: "Consolas"
                                font.pixelSize: 10
                            }
                            Text {
                                Layout.fillWidth: true
                                text: modelData.txContactIndex >= 0 ? modelData.txContactIndex : "--"
                                color: root.silverDim
                                font.family: "Consolas"
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "READ-ONLY VIEW // EDITING AND RADIO WRITES REMAIN LOCKED UNTIL BINARY ROUND-TRIP VERIFICATION"
            color: root.silverDim
            font.family: "Consolas"
            font.pixelSize: 9
        }
    }
}
