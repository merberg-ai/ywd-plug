import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    signal requestPage(int page)

    property color black: "#050607"
    property color panel: "#0b0d0f"
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color line: "#30353a"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"
    property color amberBright: "#f0b43c"
    property color green: "#58d878"
    property color greenDim: "#235d31"
    property color red: "#e45c5c"
    property color muted: "#676d73"

    property bool operationError: appController.status.indexOf("FAILED") >= 0
                                  || appController.status.indexOf("BLOCKED") >= 0
                                  || appController.status.indexOf("NO SERIAL") >= 0
    property bool readingRadio: appController.busy && appController.operation === "read"
    property bool readingBackup: appController.busy && appController.operation === "backup"
    property bool probing: appController.busy && appController.operation === "probe"

    function buttonBackground(button, accent) {
        return button.enabled && button.hovered ? accent : root.panel
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 52
        clip: true

        ColumnLayout {
            id: contentColumn
            width: parent.width
            spacing: 18
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 26

            Item { Layout.preferredHeight: 8 }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "> RADIO CONNECTION / SELECTIVE READ"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                Item { Layout.fillWidth: true }
                Text { text: "SYS://RADIO/READ"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
            }

            Text {
                Layout.fillWidth: true
                text: "Native Win32 serial transport. READ RADIO performs the selective codeplug read plus channel-referenced digital contacts from the separate contact region; RAW BACKUP remains the exhaustive config safety/diagnostic capture. Radio writes remain locked."
                wrapMode: Text.WordWrap
                color: root.silverDim
                font.family: "Consolas"
                font.pixelSize: 11
                lineHeight: 1.25
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 410
                color: "#070809"
                border.color: root.lineStrong
                border.width: 1

                Text {
                    text: "[ SERIAL INTERFACE / WIN32 / READ-ONLY ]"
                    color: root.silverBright
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                    x: 14
                    y: -7
                    leftPadding: 6
                    rightPadding: 6
                    Rectangle { anchors.fill: parent; anchors.margins: -2; color: root.black; z: -1 }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    anchors.topMargin: 28
                    spacing: 12

                    Text { text: "PORT> SELECT DEVICE"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10 }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        ComboBox {
                            id: portBox
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            model: appController.ports
                            textRole: "label"
                            valueRole: "name"
                            enabled: !appController.busy && count > 0

                            contentItem: Text {
                                leftPadding: 12
                                rightPadding: 30
                                text: "> " + portBox.displayText
                                color: portBox.enabled ? root.silverBright : root.muted
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.family: "Consolas"
                                font.pixelSize: 11
                            }
                            indicator: Text {
                                x: portBox.width - width - 12
                                anchors.verticalCenter: parent.verticalCenter
                                text: "v"
                                color: root.amber
                                font.family: "Consolas"
                                font.pixelSize: 12
                                font.bold: true
                            }
                            background: Rectangle {
                                color: root.black
                                border.color: portBox.activeFocus ? root.amber : root.lineStrong
                                border.width: 1
                            }
                            delegate: ItemDelegate {
                                width: portBox.width
                                height: 34
                                contentItem: Text {
                                    text: "> " + modelData.label
                                    color: highlighted ? root.black : root.silver
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle { color: highlighted ? root.silver : root.panel }
                            }
                            popup: Popup {
                                y: portBox.height
                                width: portBox.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 1
                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: portBox.popup.visible ? portBox.delegateModel : null
                                    currentIndex: portBox.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }
                                background: Rectangle { color: root.panel; border.color: root.lineStrong; border.width: 1 }
                            }
                        }

                        Button {
                            id: refreshButton
                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 42
                            text: "[ REFRESH ]"
                            enabled: !appController.busy
                            onClicked: appController.refreshPorts()
                            contentItem: Text { text: refreshButton.text; color: refreshButton.hovered ? root.black : root.silver; font.family: "Consolas"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: root.buttonBackground(refreshButton, root.silver); border.color: refreshButton.hovered ? root.silverBright : root.lineStrong; border.width: 1 }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Button {
                            id: probeButton
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 44
                            text: root.probing ? "[ PROBING... ]" : "[ PROBE ]"
                            enabled: !appController.busy && portBox.count > 0
                            onClicked: appController.probePort(portBox.currentValue)
                            contentItem: Text { text: probeButton.text; color: !probeButton.enabled ? root.muted : probeButton.hovered ? root.black : root.amberBright; font.family: "Consolas"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: root.buttonBackground(probeButton, root.amber); border.color: probeButton.enabled ? root.amber : root.line; border.width: 1 }
                        }

                        Button {
                            id: readButton
                            Layout.preferredWidth: 180
                            Layout.preferredHeight: 44
                            text: root.readingRadio ? "[ READ " + appController.readProgress + "% ]" : "[ READ RADIO ]"
                            enabled: !appController.busy && portBox.count > 0
                            onClicked: appController.readRadio(portBox.currentValue)
                            contentItem: Text { text: readButton.text; color: !readButton.enabled ? root.muted : readButton.hovered ? root.black : root.green; font.family: "Consolas"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: root.buttonBackground(readButton, root.green); border.color: readButton.enabled ? root.greenDim : root.line; border.width: 1 }
                        }

                        Button {
                            id: backupButton
                            Layout.preferredWidth: 170
                            Layout.preferredHeight: 44
                            text: root.readingBackup ? "[ BACKUP " + appController.readProgress + "% ]" : "[ RAW BACKUP ]"
                            enabled: !appController.busy && appController.radioDetected && portBox.count > 0 && portBox.currentValue === appController.detectedPort
                            onClicked: appController.readRawBackup(portBox.currentValue)
                            contentItem: Text { text: backupButton.text; color: !backupButton.enabled ? root.muted : backupButton.hovered ? root.black : root.silverBright; font.family: "Consolas"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: root.buttonBackground(backupButton, root.silver); border.color: backupButton.enabled ? root.silver : root.line; border.width: 1 }
                        }

                        Button {
                            id: loadButton
                            Layout.preferredWidth: 185
                            Layout.preferredHeight: 44
                            text: "[ LOAD BACKUP ]"
                            enabled: !appController.busy
                            onClicked: {
                                appController.loadLatestBackup()
                                if (appController.channelsReady)
                                    root.requestPage(2)
                            }
                            contentItem: Text { text: loadButton.text; color: !loadButton.enabled ? root.muted : loadButton.hovered ? root.black : root.silverBright; font.family: "Consolas"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: root.buttonBackground(loadButton, root.silver); border.color: loadButton.enabled ? root.lineStrong : root.line; border.width: 1 }
                        }

                        Text {
                            text: root.readingRadio ? "* SELECTIVE READ ACTIVE"
                                  : root.readingBackup ? "* FULL BLOCK READ ACTIVE"
                                  : appController.contactsReady ? "+ CODEPLUG + REFERENCED CONTACTS"
                                  : appController.liveReadReady ? "+ LIVE CODEPLUG DATABASE"
                                  : appController.codeplugReady ? "+ CODEPLUG IMAGE DECODED"
                                  : appController.radioDetected ? "+ PROBE PASSED"
                                  : "- LINK IDLE"
                            color: root.readingRadio || root.readingBackup ? root.amber : (appController.liveReadReady || appController.codeplugReady || appController.radioDetected) ? root.green : root.silverDim
                            font.family: "Consolas"
                            font.pixelSize: 10
                            font.bold: true
                        }

                        Item { Layout.fillWidth: true }
                        Text { text: portBox.count > 0 ? (portBox.currentValue + " / 115200 / 8N1") : "NO COM PORT"; color: root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        visible: root.readingRadio || root.readingBackup || appController.liveReadReady || appController.backupReady

                        Text { text: "READ>"; color: root.readingRadio || root.readingBackup ? root.amber : root.green; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 16
                            color: root.black
                            border.color: root.lineStrong
                            border.width: 1
                            Rectangle {
                                x: 1; y: 1; height: parent.height - 2
                                width: Math.max(0, (parent.width - 2) * appController.readProgress / 100)
                                color: appController.liveReadReady || appController.backupReady ? root.green : root.amber
                            }
                        }
                        Text { text: appController.readProgress.toString().padStart(3, "0") + "%"; color: appController.liveReadReady || appController.backupReady ? root.green : root.silver; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 58
                        color: root.black
                        border.color: root.operationError ? root.red : (appController.liveReadReady || appController.radioDetected ? root.greenDim : root.line)
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10
                            Text { text: root.operationError ? "ERR>" : appController.liveReadReady || appController.radioDetected ? "OK >" : "SYS>"; color: root.operationError ? root.red : appController.liveReadReady || appController.radioDetected ? root.green : root.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { Layout.fillWidth: true; text: appController.status; color: root.operationError ? root.red : appController.liveReadReady || appController.radioDetected ? root.green : root.silver; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: appController.liveReadReady ? 326 : 230
                color: "#070809"
                border.color: root.line
                border.width: 1

                Text {
                    text: "[ SESSION STATUS / PHASE 5 ]"
                    color: root.silverBright
                    font.family: "Consolas"
                    font.pixelSize: 10
                    font.bold: true
                    x: 14
                    y: -7
                    leftPadding: 6
                    rightPadding: 6
                    Rectangle { anchors.fill: parent; anchors.margins: -2; color: root.black; z: -1 }
                }

                Column {
                    anchors.fill: parent
                    anchors.margins: 22
                    anchors.topMargin: 28
                    spacing: 9
                    Text { text: "[OK]  NATIVE WIN32 SERIAL BACKEND ONLINE"; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { text: "[OK]  PSEARCH / PASSSTA / SYSINFO + PROGRAM READ PATH PROVEN"; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { text: "[--]  WRITE-MEMORY API NOT IMPLEMENTED / RADIO WRITES LOCKED"; color: root.amber; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { text: appController.codeplugReady ? "[OK]  CHANNEL / ZONE / SCAN / RX-GROUP DECODERS READY" : "[--]  CODEPLUG DECODERS WAITING FOR IMAGE"; color: appController.codeplugReady ? root.green : root.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { visible: appController.codeplugReady; text: "DATA> " + appController.channelCount + " CH // " + appController.zoneCount + " ZONES // " + appController.scanListCount + " SCAN // " + appController.rxGroupCount + " RXG"; color: root.green; font.family: "Consolas"; font.pixelSize: 10 }
                    Text {
                        visible: appController.liveReadReady
                        text: appController.contactsReady
                              ? "CONT> " + appController.contactCount + " REFERENCED // DATABASE HEADER " + appController.contactDatabaseCount + " // PAGES " + appController.lastReadContactBlocks
                              : appController.contactWarning.length > 0
                                ? "CONT> WARN // " + appController.contactWarning
                                : "CONT> NO REFERENCED CONTACTS"
                        color: appController.contactsReady ? root.green : appController.contactWarning.length > 0 ? root.amber : root.silverDim
                        font.family: "Consolas"
                        font.pixelSize: 9
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Text { visible: appController.liveReadReady; text: "FAST> MAP " + appController.lastReadDiscoveredBlocks + " BLOCKS // CFG " + appController.lastReadDataBlocks + " BLOCKS // CONTACT " + appController.lastReadContactBlocks + " PAGES // " + appController.lastReadBytes + " BYTES // " + (appController.lastReadMs / 1000.0).toFixed(1) + "s"; color: root.green; font.family: "Consolas"; font.pixelSize: 9 }
                    Text { text: appController.contactsReady ? "NEXT> INSPECT CONTACTS OR CHANNEL TX-CONTACT RESOLUTION" : appController.codeplugReady ? "NEXT> USE LEFT NAVIGATION TO INSPECT CODEPLUG SECTIONS" : appController.radioDetected ? "NEXT> READ RADIO OR EXECUTE RAW BACKUP" : "NEXT> SELECT COM PORT AND EXECUTE READ RADIO"; color: root.silverBright; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                }
            }

            Item { Layout.preferredHeight: 10 }
        }
    }
}
