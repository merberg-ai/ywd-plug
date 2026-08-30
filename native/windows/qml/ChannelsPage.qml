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
    property int sectionIndex: 0

    function frequencyText(value, disabled) {
        if (disabled)
            return "RX ONLY"
        return Number(value).toFixed(5).replace(/0+$/, "").replace(/\.$/, "")
    }

    function sectionButtonColor(index, hovered) {
        if (root.sectionIndex === index)
            return root.amber
        return hovered ? root.silver : root.panel
    }

    function sectionTextColor(index, hovered) {
        if (root.sectionIndex === index || hovered)
            return root.black
        return root.silver
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 3
                Text {
                    text: "> CODEPLUG DATABASE"
                    color: root.silverBright
                    font.family: "Consolas"
                    font.pixelSize: 21
                    font.bold: true
                }
                Text {
                    text: appController.channelsReady
                          ? appController.channelCount + " CH // "
                            + appController.zoneCount + " ZONES // "
                            + appController.scanListCount + " SCAN // "
                            + appController.rxGroupCount + " RX GROUPS // READ ONLY"
                          : "NO DECODED CODEPLUG IMAGE LOADED"
                    color: appController.channelsReady ? root.green : root.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 10
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "SYS://RADIO/CODEPLUG"
                color: root.silverDim
                font.family: "Consolas"
                font.pixelSize: 10
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                id: channelsTab
                Layout.preferredWidth: 180
                Layout.preferredHeight: 34
                text: "[ CHANNELS " + appController.channelCount + " ]"
                onClicked: root.sectionIndex = 0
                contentItem: Text {
                    text: channelsTab.text
                    color: root.sectionTextColor(0, channelsTab.hovered)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                }
                background: Rectangle {
                    color: root.sectionButtonColor(0, channelsTab.hovered)
                    border.color: root.sectionIndex === 0 ? root.amber : root.lineStrong
                    border.width: 1
                }
            }

            Button {
                id: zonesTab
                Layout.preferredWidth: 160
                Layout.preferredHeight: 34
                text: "[ ZONES " + appController.zoneCount + " ]"
                enabled: appController.codeplugReady
                onClicked: root.sectionIndex = 1
                contentItem: Text {
                    text: zonesTab.text
                    color: !zonesTab.enabled ? root.muted : root.sectionTextColor(1, zonesTab.hovered)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                }
                background: Rectangle {
                    color: zonesTab.enabled ? root.sectionButtonColor(1, zonesTab.hovered) : root.panel
                    border.color: root.sectionIndex === 1 ? root.amber : root.lineStrong
                    border.width: 1
                }
            }

            Button {
                id: scanTab
                Layout.preferredWidth: 190
                Layout.preferredHeight: 34
                text: "[ SCAN LISTS " + appController.scanListCount + " ]"
                enabled: appController.codeplugReady
                onClicked: root.sectionIndex = 2
                contentItem: Text {
                    text: scanTab.text
                    color: !scanTab.enabled ? root.muted : root.sectionTextColor(2, scanTab.hovered)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                }
                background: Rectangle {
                    color: scanTab.enabled ? root.sectionButtonColor(2, scanTab.hovered) : root.panel
                    border.color: root.sectionIndex === 2 ? root.amber : root.lineStrong
                    border.width: 1
                }
            }

            Button {
                id: rxTab
                Layout.preferredWidth: 190
                Layout.preferredHeight: 34
                text: "[ RX GROUPS " + appController.rxGroupCount + " ]"
                enabled: appController.codeplugReady
                onClicked: root.sectionIndex = 3
                contentItem: Text {
                    text: rxTab.text
                    color: !rxTab.enabled ? root.muted : root.sectionTextColor(3, rxTab.hovered)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                }
                background: Rectangle {
                    color: rxTab.enabled ? root.sectionButtonColor(3, rxTab.hovered) : root.panel
                    border.color: root.sectionIndex === 3 ? root.amber : root.lineStrong
                    border.width: 1
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.sectionIndex === 0 ? "CHANNEL RECORDS"
                    : root.sectionIndex === 1 ? "ZONE MEMBERSHIP"
                    : root.sectionIndex === 2 ? "SCAN POLICY"
                    : "DMR RECEIVE MEMBERSHIP"
                color: root.silverDim
                font.family: "Consolas"
                font.pixelSize: 9
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.sectionIndex

            Rectangle {
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

            Rectangle {
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
                            Text { Layout.preferredWidth: 64; text: "#"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 260; text: "ZONE NAME"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: "CHANNELS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.fillWidth: true; text: "CHANNEL MEMBERSHIP"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
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
                            height: 38
                            color: index % 2 === 0 ? "#070809" : "#090b0d"
                            border.color: "#171a1d"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 0
                                Text { Layout.preferredWidth: 64; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 260; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                                Text { Layout.preferredWidth: 100; text: modelData.channelCount; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.fillWidth: true; text: modelData.channelsText; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            }
                        }
                    }
                }
            }

            Rectangle {
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
                            Text { Layout.preferredWidth: 190; text: "SCAN NAME"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 70; text: "CH"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 80; text: "HANG"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: "PRI 1"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: "PRI 2"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: "TX"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.fillWidth: true; text: "CHANNEL MEMBERSHIP"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
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
                            height: 38
                            color: index % 2 === 0 ? "#070809" : "#090b0d"
                            border.color: "#171a1d"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 0
                                Text { Layout.preferredWidth: 54; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 190; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                                Text { Layout.preferredWidth: 70; text: modelData.channelCount; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 80; text: modelData.hangTime; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 100; text: modelData.priority1; color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 100; text: modelData.priority2; color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 100; text: modelData.designatedTx; color: root.silver; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.fillWidth: true; text: modelData.channelsText; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            }
                        }
                    }
                }
            }

            Rectangle {
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
                            Text { Layout.preferredWidth: 64; text: "#"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 260; text: "RX GROUP"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.preferredWidth: 100; text: "MEMBERS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.fillWidth: true; text: "DMR / TALK GROUP IDS"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        }
                    }

                    ListView {
                        id: rxGroupList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: appController.rxGroups
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar { }

                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: rxGroupList.width
                            height: 38
                            color: index % 2 === 0 ? "#070809" : "#090b0d"
                            border.color: "#171a1d"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 0
                                Text { Layout.preferredWidth: 64; text: modelData.number; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.preferredWidth: 260; text: modelData.name; color: root.silverBright; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                                Text { Layout.preferredWidth: 100; text: modelData.memberCount; color: modelData.memberCount > 0 ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { Layout.fillWidth: true; text: modelData.membersText.length > 0 ? modelData.membersText : "--"; color: root.silver; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10 }
                            }
                        }
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "READ-ONLY CODEPLUG VIEW // EDITING AND RADIO WRITES REMAIN LOCKED UNTIL BINARY ROUND-TRIP VERIFICATION"
            color: root.silverDim
            font.family: "Consolas"
            font.pixelSize: 9
        }
    }
}
