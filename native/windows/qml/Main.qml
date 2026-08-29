import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import YWDPlug

ApplicationWindow {
    id: window

    width: 1440
    height: 900
    minimumWidth: 1100
    minimumHeight: 700
    visible: false
    title: "YWD-Plug // Native Windows"
    color: "#050607"

    property color black: "#050607"
    property color black2: "#080a0b"
    property color panel: "#0b0d0f"
    property color panel2: "#0f1113"
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color line: "#30353a"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"
    property color amberBright: "#f0b43c"
    property color amberDim: "#74521c"
    property color green: "#58d878"
    property color greenDim: "#235d31"
    property color red: "#e45c5c"
    property color muted: "#676d73"
    property int activePage: 1

    property bool operationError: appController.status.indexOf("FAILED") >= 0
                                  || appController.status.indexOf("BLOCKED") >= 0
                                  || appController.status.indexOf("NO SERIAL") >= 0
    property bool readingRadio: appController.busy && appController.operation === "read"
    property bool readingBackup: appController.busy && appController.operation === "backup"
    property bool probing: appController.busy && appController.operation === "probe"

    property string asciiBanner:
          "$$      $$ $$      $$ $$$$$$$          $$$$$$$  $$       $$   $$   $$$$$$\n"
        + " $$    $$  $$  $$  $$ $$    $$         $$    $$ $$       $$   $$  $$    $$\n"
        + "  $$  $$   $$ $$$$ $$ $$    $$  $$$$$  $$$$$$$  $$       $$   $$  $$\n"
        + "   $$$$    $$$$  $$$$ $$    $$         $$       $$       $$   $$  $$  $$$\n"
        + "    $$     $$$    $$$ $$    $$         $$       $$       $$   $$  $$   $$\n"
        + "    $$     $$      $$ $$$$$$$          $$       $$$$$$$$  $$$$$$$   $$$$$$"

    background: Item {
        Rectangle { anchors.fill: parent; color: window.black }
        Repeater {
            model: Math.ceil(window.height / 6)
            Rectangle {
                width: window.width
                height: 1
                y: index * 6
                color: "#15181b"
                opacity: 0.24
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 146
            color: "#070809"
            border.color: window.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                spacing: 24

                Text {
                    text: window.asciiBanner
                    color: window.silverBright
                    font.family: "Consolas"
                    font.pixelSize: 9
                    font.bold: true
                    lineHeight: 0.88
                    renderType: Text.NativeRendering
                    Layout.alignment: Qt.AlignVCenter
                }

                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    Layout.topMargin: 8
                    Layout.bottomMargin: 8
                    color: window.line
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 5

                    Text {
                        text: "YWD-PLUG / NATIVE WINDOWS"
                        color: window.silverBright
                        font.family: "Consolas"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Text {
                        text: "RADIO PROGRAMMING WORKSTATION"
                        color: window.silverDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                    }
                    Item { Layout.preferredHeight: 5 }
                    Text { text: "HOST    : WIN32 / x64"; color: window.silver; font.family: "Consolas"; font.pixelSize: 10 }
                    Text {
                        text: "TARGET  : " + (appController.radioDetected ? appController.radioModel : "DM-32UV / DP570UV")
                        color: appController.radioDetected ? window.green : window.silver
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text {
                        text: "PORT    : " + (appController.radioDetected ? appController.detectedPort : "UNBOUND")
                        color: appController.radioDetected ? window.green : window.silverDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text { text: "ACCESS  : READ-ONLY / WRITE LOCKED"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10 }
                }

                StatusPill {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    text: readingRadio ? "FAST READ" : readingBackup ? "BACKUP" : probing ? "PROBING" : operationError ? "FAULT" : appController.liveReadReady ? "LIVE READ OK" : appController.channelsReady ? "CHANNELS OK" : appController.backupReady ? "BACKUP OK" : appController.radioDetected ? "LINK OK" : "STANDBY"
                    busy: appController.busy
                    good: appController.liveReadReady || appController.channelsReady || appController.backupReady || appController.radioDetected
                    error: window.operationError
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 224
                Layout.fillHeight: true
                color: "#070809"
                border.color: window.line
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 2

                    Text { text: "+--[ RADIO ]--------------------------------"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10; Layout.bottomMargin: 5 }

                    NavButton {
                        Layout.fillWidth: true
                        indexText: "01"
                        text: "Connection"
                        active: window.activePage === 1
                        onClicked: window.activePage = 1
                    }
                    NavButton {
                        Layout.fillWidth: true
                        indexText: "02"
                        text: "Channels"
                        enabled: appController.channelsReady
                        active: window.activePage === 2
                        onClicked: window.activePage = 2
                    }
                    NavButton { Layout.fillWidth: true; indexText: "03"; text: "Zones"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "04"; text: "Scan Lists"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "05"; text: "Contacts"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "06"; text: "RX Groups"; enabled: false }

                    Text { text: "+--[ CONFIG ]-------------------------------"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10; Layout.topMargin: 14; Layout.bottomMargin: 5 }

                    NavButton { Layout.fillWidth: true; indexText: "10"; text: "Radio IDs"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "11"; text: "Settings"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "12"; text: "Display"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "13"; text: "Calibration"; enabled: false }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 124
                        color: window.black2
                        border.color: window.line
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5

                            Text { text: "SYSTEM> STATUS"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { text: "BRANCH : dev-win"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "PHASE  : milestone-3"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "READ   : " + (appController.liveReadReady ? "SELECTIVE" : appController.backupReady ? "CAPTURED" : "ARMED"); color: appController.liveReadReady || appController.backupReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                            Text { text: "CHAN   : " + (appController.channelsReady ? appController.channelCount : "--"); color: appController.channelsReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                            Text { text: "WRITE  : LOCKED"; color: window.green; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                        }
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: window.activePage - 1

                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
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
                            spacing: 10
                            Text { text: "> RADIO CONNECTION / SELECTIVE READ"; color: window.silverBright; font.family: "Consolas"; font.pixelSize: 21; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Text { text: "SYS://RADIO/READ"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "Phase 3 adds the normal READ RADIO path. It discovers the 4KB map by reading only each block metadata byte, reads the channel count, then fetches only required channel and TX-contact blocks. RAW BACKUP remains the exhaustive 800 KiB safety/diagnostic capture. Radio writes remain locked."
                            wrapMode: Text.WordWrap
                            color: window.silverDim
                            font.family: "Consolas"
                            font.pixelSize: 11
                            lineHeight: 1.25
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 410
                            color: "#070809"
                            border.color: window.lineStrong
                            border.width: 1

                            Text {
                                text: "[ SERIAL INTERFACE / WIN32 / READ-ONLY ]"
                                color: window.silverBright
                                font.family: "Consolas"
                                font.pixelSize: 10
                                font.bold: true
                                x: 14
                                y: -7
                                leftPadding: 6
                                rightPadding: 6
                                Rectangle { anchors.fill: parent; anchors.margins: -2; color: window.black; z: -1 }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 22
                                anchors.topMargin: 28
                                spacing: 12

                                Text { text: "PORT> SELECT DEVICE"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10 }

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
                                            color: portBox.enabled ? window.silverBright : window.muted
                                            verticalAlignment: Text.AlignVCenter
                                            elide: Text.ElideRight
                                            font.family: "Consolas"
                                            font.pixelSize: 11
                                        }

                                        indicator: Text {
                                            x: portBox.width - width - 12
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: "v"
                                            color: window.amber
                                            font.family: "Consolas"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }

                                        background: Rectangle {
                                            color: window.black
                                            border.color: portBox.activeFocus ? window.amber : window.lineStrong
                                            border.width: 1
                                        }

                                        delegate: ItemDelegate {
                                            width: portBox.width
                                            height: 34
                                            contentItem: Text {
                                                text: "> " + modelData.label
                                                color: highlighted ? window.black : window.silver
                                                font.family: "Consolas"
                                                font.pixelSize: 10
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                            background: Rectangle { color: highlighted ? window.silver : window.panel }
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
                                            background: Rectangle { color: window.panel; border.color: window.lineStrong; border.width: 1 }
                                        }
                                    }

                                    Button {
                                        id: refreshButton
                                        Layout.preferredWidth: 120
                                        Layout.preferredHeight: 42
                                        text: "[ REFRESH ]"
                                        enabled: !appController.busy
                                        onClicked: appController.refreshPorts()
                                        contentItem: Text {
                                            text: refreshButton.text
                                            color: refreshButton.hovered ? window.black : window.silver
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: refreshButton.hovered ? window.silver : window.panel
                                            border.color: refreshButton.hovered ? window.silverBright : window.lineStrong
                                            border.width: 1
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Button {
                                        id: probeButton
                                        Layout.preferredWidth: 150
                                        Layout.preferredHeight: 44
                                        text: probing ? "[ PROBING... ]" : "[ PROBE ]"
                                        enabled: !appController.busy && portBox.count > 0
                                        onClicked: appController.probePort(portBox.currentValue)
                                        contentItem: Text {
                                            text: probeButton.text
                                            color: !probeButton.enabled ? window.muted : probeButton.hovered ? window.black : window.amberBright
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: probeButton.enabled && probeButton.hovered ? window.amber : window.panel
                                            border.color: probeButton.enabled ? window.amber : window.line
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        id: readRadioButton
                                        Layout.preferredWidth: 180
                                        Layout.preferredHeight: 44
                                        text: readingRadio ? "[ READ " + appController.readProgress + "% ]" : "[ READ RADIO ]"
                                        enabled: !appController.busy && portBox.count > 0
                                        onClicked: appController.readRadio(portBox.currentValue)
                                        contentItem: Text {
                                            text: readRadioButton.text
                                            color: !readRadioButton.enabled ? window.muted : readRadioButton.hovered ? window.black : window.green
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: readRadioButton.enabled && readRadioButton.hovered ? window.green : window.panel
                                            border.color: readRadioButton.enabled ? window.greenDim : window.line
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        id: backupButton
                                        Layout.preferredWidth: 170
                                        Layout.preferredHeight: 44
                                        text: readingBackup ? "[ BACKUP " + appController.readProgress + "% ]" : "[ RAW BACKUP ]"
                                        enabled: !appController.busy
                                                 && appController.radioDetected
                                                 && portBox.count > 0
                                                 && portBox.currentValue === appController.detectedPort
                                        onClicked: appController.readRawBackup(portBox.currentValue)
                                        contentItem: Text {
                                            text: backupButton.text
                                            color: !backupButton.enabled ? window.muted : backupButton.hovered ? window.black : window.silverBright
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: backupButton.enabled && backupButton.hovered ? window.silver : window.panel
                                            border.color: backupButton.enabled ? window.silver : window.line
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        id: loadBackupButton
                                        Layout.preferredWidth: 185
                                        Layout.preferredHeight: 44
                                        text: "[ LOAD BACKUP ]"
                                        enabled: !appController.busy
                                        onClicked: {
                                            appController.loadLatestBackup()
                                            if (appController.channelsReady)
                                                window.activePage = 2
                                        }
                                        contentItem: Text {
                                            text: loadBackupButton.text
                                            color: !loadBackupButton.enabled ? window.muted : loadBackupButton.hovered ? window.black : window.silverBright
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: loadBackupButton.enabled && loadBackupButton.hovered ? window.silver : window.panel
                                            border.color: loadBackupButton.enabled ? window.lineStrong : window.line
                                            border.width: 1
                                        }
                                    }

                                    Text {
                                        text: readingRadio ? "* SELECTIVE READ ACTIVE" : readingBackup ? "* FULL BLOCK READ ACTIVE" : appController.liveReadReady ? "+ LIVE CHANNEL DATABASE" : appController.channelsReady ? "+ CHANNEL IMAGE DECODED" : appController.backupReady ? "+ RAW IMAGE CAPTURED" : appController.radioDetected ? "+ PROBE PASSED" : "- LINK IDLE"
                                        color: readingRadio || readingBackup ? window.amber : (appController.liveReadReady || appController.channelsReady || appController.backupReady || appController.radioDetected) ? window.green : window.silverDim
                                        font.family: "Consolas"
                                        font.pixelSize: 10
                                        font.bold: true
                                        SequentialAnimation on opacity {
                                            running: appController.busy
                                            loops: Animation.Infinite
                                            NumberAnimation { to: 0.25; duration: 320 }
                                            NumberAnimation { to: 1.0; duration: 320 }
                                        }
                                    }

                                    Item { Layout.fillWidth: true }
                                    Text { text: portBox.count > 0 ? (portBox.currentValue + " / 115200 / 8N1") : "NO COM PORT"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10
                                    visible: readingRadio || readingBackup || appController.liveReadReady || appController.backupReady

                                    Text { text: "READ>"; color: readingRadio || readingBackup ? window.amber : window.green; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }

                                    Rectangle {
                                        id: progressTrack
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 16
                                        color: window.black
                                        border.color: window.lineStrong
                                        border.width: 1

                                        Rectangle {
                                            x: 1
                                            y: 1
                                            height: parent.height - 2
                                            width: Math.max(0, (parent.width - 2) * appController.readProgress / 100)
                                            color: appController.liveReadReady || appController.backupReady ? window.green : window.amber
                                        }
                                    }

                                    Text {
                                        text: appController.readProgress.toString().padStart(3, "0") + "%"
                                        color: appController.liveReadReady || appController.backupReady ? window.green : window.silver
                                        font.family: "Consolas"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 58
                                    color: window.black
                                    border.color: window.operationError ? window.red : appController.liveReadReady || appController.backupReady || appController.radioDetected ? window.greenDim : window.line
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        spacing: 10

                                        Text {
                                            text: window.operationError ? "ERR>" : appController.liveReadReady || appController.backupReady || appController.radioDetected ? "OK >" : "SYS>"
                                            color: window.operationError ? window.red : appController.liveReadReady || appController.backupReady || appController.radioDetected ? window.green : window.amber
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: appController.status
                                            color: window.operationError ? window.red : appController.liveReadReady || appController.backupReady || appController.radioDetected ? window.green : window.silver
                                            verticalAlignment: Text.AlignVCenter
                                            elide: Text.ElideRight
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                        Text {
                                            text: "_"
                                            color: appController.busy ? window.amberBright : "transparent"
                                            font.family: "Consolas"
                                            font.pixelSize: 12
                                            font.bold: true
                                            SequentialAnimation on opacity {
                                                running: appController.busy
                                                loops: Animation.Infinite
                                                NumberAnimation { to: 0; duration: 420 }
                                                NumberAnimation { to: 1; duration: 420 }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: appController.liveReadReady ? 304 : appController.channelsReady ? 286 : appController.backupReady ? 264 : 236
                            color: "#070809"
                            border.color: window.line
                            border.width: 1

                            Text {
                                text: "[ SESSION STATUS / PHASE 3 ]"
                                color: window.silverBright
                                font.family: "Consolas"
                                font.pixelSize: 10
                                font.bold: true
                                x: 14
                                y: -7
                                leftPadding: 6
                                rightPadding: 6
                                Rectangle { anchors.fill: parent; anchors.margins: -2; color: window.black; z: -1 }
                            }

                            Column {
                                anchors.fill: parent
                                anchors.margins: 22
                                anchors.topMargin: 28
                                spacing: 9

                                Text { text: "[OK]  NATIVE WIN32 SERIAL BACKEND ONLINE"; color: window.green; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { text: "[OK]  PSEARCH / PASSSTA / SYSINFO + PROGRAM READ PATH PROVEN"; color: window.green; font.family: "Consolas"; font.pixelSize: 10 }
                                Text { text: "[--]  WRITE-MEMORY API NOT IMPLEMENTED / RADIO WRITES LOCKED"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10 }
                                Text {
                                    text: appController.liveReadReady
                                          ? "[OK]  SELECTIVE METADATA MAP + REQUIRED-BLOCK READ COMPLETED"
                                          : "[--]  SELECTIVE METADATA MAP + REQUIRED-BLOCK READ READY FOR TEST"
                                    color: appController.liveReadReady ? window.green : window.silverDim
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                }
                                Text {
                                    visible: appController.channelsReady
                                    text: "[OK]  NATIVE CHANNEL DECODER // " + appController.channelCount + " RECORDS READY"
                                    color: window.green
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                }
                                Text {
                                    visible: appController.liveReadReady
                                    text: "FAST> MAP " + appController.lastReadDiscoveredBlocks + " BLOCKS // DATA " + appController.lastReadDataBlocks + " BLOCKS // " + appController.lastReadBytes + " BYTES // " + (appController.lastReadMs / 1000.0).toFixed(1) + "s"
                                    color: window.green
                                    font.family: "Consolas"
                                    font.pixelSize: 9
                                }
                                Text {
                                    visible: appController.backupReady
                                    width: parent.width
                                    text: "BIN>  " + appController.backupPath
                                    color: window.silver
                                    elide: Text.ElideMiddle
                                    font.family: "Consolas"
                                    font.pixelSize: 9
                                }
                                Text {
                                    visible: appController.backupReady && appController.backupManifestPath.length > 0
                                    width: parent.width
                                    text: "JSON> " + appController.backupManifestPath
                                    color: window.silverDim
                                    elide: Text.ElideMiddle
                                    font.family: "Consolas"
                                    font.pixelSize: 9
                                }
                                Text {
                                    visible: appController.backupReady && appController.backupSha256.length > 0
                                    text: "SHA>  " + appController.backupSha256.toUpperCase()
                                    color: window.green
                                    font.family: "Consolas"
                                    font.pixelSize: 9
                                }
                                Text {
                                    text: appController.liveReadReady
                                          ? "NEXT> OPEN CHANNELS // COMPARE FAST READ AGAINST OFFLINE FIXTURE"
                                          : appController.channelsReady
                                            ? "NEXT> OPEN CHANNELS // READ-ONLY NATIVE DATABASE READY"
                                            : appController.radioDetected
                                              ? "NEXT> READ RADIO OR EXECUTE RAW BACKUP"
                                              : "NEXT> SELECT COM PORT AND EXECUTE READ RADIO"
                                    color: window.silverBright
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }

                        Item { Layout.preferredHeight: 10 }
                    }
                }

                ChannelsPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: window.black
            border.color: window.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16

                Text {
                    text: appController.liveReadReady ? "[LIVE READ]" : appController.channelsReady ? "[CHANNELS]" : appController.backupReady ? "[BACKUP]" : appController.radioDetected ? "[OK]" : window.operationError ? "[ERR]" : appController.busy ? "[RUN]" : "[IDLE]"
                    color: appController.liveReadReady || appController.channelsReady || appController.backupReady || appController.radioDetected ? window.green : window.operationError ? window.red : appController.busy ? window.amber : window.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 9
                    font.bold: true
                }
                Text {
                    text: appController.status
                    color: window.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text { text: "KJ6YWD.NET // YWD-PLUG // DEV-WIN // M3"; color: window.silver; font.family: "Consolas"; font.pixelSize: 9 }
            }
        }
    }

    Connections {
        target: appController
        function onChannelsChanged() {
            if (appController.liveReadReady && appController.channelsReady)
                window.activePage = 2
        }
    }

    SplashWindow {
        id: startupSplash
        onFinished: windowState.showRestored()
    }
}
