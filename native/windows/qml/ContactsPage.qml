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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 14

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 3
                Text { text: "> REFERENCED CONTACTS"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                Text {
                    text: appController.contactsReady
                          ? appController.contactCount + " REFERENCED // " + appController.contactDatabaseCount + " CODEPLUG TALK GROUPS // SELECTIVE LIVE READ"
                          : "CODEPLUG TALK GROUP TABLE NOT LOADED // EXECUTE READ RADIO"
                    color: appController.contactsReady ? root.green : root.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 10
                }
            }

            Item { Layout.fillWidth: true }
            Text { text: "SYS://RADIO/CONTACTS"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: appController.contactWarning.length > 0 ? 40 : 0
            visible: appController.contactWarning.length > 0
            color: root.black
            border.color: root.amber
            border.width: 1

            Text {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                verticalAlignment: Text.AlignVCenter
                text: "WARN> " + appController.contactWarning
                color: root.amber
                elide: Text.ElideRight
                font.family: "Consolas"
                font.pixelSize: 9
                font.bold: true
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
                    visible: appController.contactsReady
                    color: root.panel
                    border.color: root.line
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 0

                        Text { Layout.preferredWidth: 70; text: "IDX"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 300; text: "NAME"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 170; text: "CONTACT / TG"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.preferredWidth: 150; text: "CALL TYPE"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Text { Layout.fillWidth: true; text: "USED BY CHANNELS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }
                }

                ListView {
                    id: contactList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: appController.contactsReady
                    clip: true
                    model: appController.contacts
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { }

                    delegate: Rectangle {
                        required property var modelData
                        required property int index

                        width: contactList.width
                        height: 36
                        color: index % 2 === 0 ? "#070809" : "#090b0d"
                        border.color: "#171a1d"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 0

                            Text { Layout.preferredWidth: 70; text: modelData.index; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 300; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 170; text: modelData.dmrId; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.preferredWidth: 150; text: modelData.callSign.length > 0 ? modelData.callSign : "--"; color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { Layout.fillWidth: true; text: modelData.usedByText.length > 0 ? modelData.usedByText : "--"; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 9 }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !appController.contactsReady
                    spacing: 10

                    Item { Layout.fillHeight: true }
                    Text { Layout.alignment: Qt.AlignHCenter; text: "[ CODEPLUG TALK GROUP TABLE NOT LOADED ]"; color: root.amber; font.family: "Consolas"; font.pixelSize: 14; font.bold: true }
                    Text { Layout.alignment: Qt.AlignHCenter; text: "EXECUTE READ RADIO TO RESOLVE CHANNEL TX-CONTACT INDEXES AGAINST THE RADIO'S TALK GROUP TABLE"; color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                    Item { Layout.fillHeight: true }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "CODEPLUG TALK GROUP VIEW // ONLY CONTACTS REFERENCED BY CHANNEL TX MAPPINGS ARE SHOWN // RADIO WRITES REMAIN LOCKED"
            color: root.silverDim
            font.family: "Consolas"
            font.pixelSize: 9
        }
    }
}
