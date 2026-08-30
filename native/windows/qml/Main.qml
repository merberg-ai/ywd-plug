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
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#747b82"
    property color line: "#30353a"
    property color lineStrong: "#51575d"
    property color amber: "#d79a2b"
    property color green: "#58d878"
    property color red: "#e45c5c"
    property color muted: "#676d73"
    property int activePage: 1

    property bool operationError: appController.status.indexOf("FAILED") >= 0
                                  || appController.status.indexOf("BLOCKED") >= 0
                                  || appController.status.indexOf("NO SERIAL") >= 0
    property bool readingRadio: appController.busy && appController.operation === "read"
    property bool readingBackup: appController.busy && appController.operation === "backup"
    property bool probing: appController.busy && appController.operation === "probe"

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
            Layout.preferredHeight: 154
            color: "#070809"
            border.color: window.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 20
                anchors.topMargin: 9
                anchors.bottomMargin: 9
                spacing: 20

                Rectangle {
                    Layout.preferredWidth: 330
                    Layout.fillHeight: true
                    Layout.topMargin: 2
                    Layout.bottomMargin: 2
                    color: "#050607"
                    border.color: window.line
                    border.width: 1
                    clip: true

                    Image {
                        id: headerBrandImage
                        anchors.fill: parent
                        anchors.margins: 2
                        source: brandingImageUrl
                        fillMode: Image.PreserveAspectCrop
                        horizontalAlignment: Image.AlignHCenter
                        verticalAlignment: Image.AlignVCenter
                        smooth: true
                        mipmap: true
                        cache: false
                        asynchronous: false
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: !brandingImageExists || headerBrandImage.status === Image.Error
                        text: brandingImageExists ? "[ BRANDING IMAGE LOAD ERROR ]" : "[ BRANDING FILE MISSING ]"
                        color: window.red
                        font.family: "Consolas"
                        font.pixelSize: 9
                        font.bold: true
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 22
                        color: "#B0050607"
                        Text {
                            anchors.centerIn: parent
                            text: "[ YWD-PLUG // RADIO PROGRAMMING WORKSTATION ]"
                            color: window.silverDim
                            font.family: "Consolas"
                            font.pixelSize: 8
                            font.letterSpacing: 0.8
                        }
                    }
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

                    Text { text: "YWD-PLUG / NATIVE WINDOWS"; color: window.silverBright; font.family: "Consolas"; font.pixelSize: 16; font.bold: true }
                    Text { text: "DM-32UV PROGRAMMING + CODEPLUG WORKSTATION"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10; font.letterSpacing: 1.0 }
                    Item { Layout.preferredHeight: 4 }
                    Text { text: "HOST    : WIN32 / x64"; color: window.silver; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { text: "TARGET  : " + (appController.radioDetected ? appController.radioModel : "DM-32UV / DP570UV"); color: appController.radioDetected ? window.green : window.silver; font.family: "Consolas"; font.pixelSize: 10 }
                    Text { text: "PORT    : " + (appController.radioDetected ? appController.detectedPort : "UNBOUND"); color: appController.radioDetected ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 10 }
                    Text {
                        text: "CONTACT : " + (appController.contactsReady ? (appController.contactCount + " REFERENCED / " + appController.contactDatabaseCount + " DATABASE") : "NOT LOADED")
                        color: appController.contactsReady ? window.green : window.silverDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text { text: "ACCESS  : READ-ONLY / WRITE LOCKED"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10 }
                }

                StatusPill {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    text: window.readingRadio ? "FAST READ"
                          : window.readingBackup ? "BACKUP"
                          : window.probing ? "PROBING"
                          : window.operationError ? "FAULT"
                          : appController.contactsReady ? "CONTACTS OK"
                          : appController.liveReadReady ? "LIVE READ OK"
                          : appController.codeplugReady ? "CODEPLUG OK"
                          : appController.backupReady ? "BACKUP OK"
                          : appController.radioDetected ? "LINK OK"
                          : "STANDBY"
                    busy: appController.busy
                    good: appController.liveReadReady || appController.codeplugReady || appController.backupReady || appController.radioDetected
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

                    NavButton { Layout.fillWidth: true; indexText: "01"; text: "Connection"; active: window.activePage === 1; onClicked: window.activePage = 1 }
                    NavButton { Layout.fillWidth: true; indexText: "02"; text: "Channels"; enabled: appController.channelsReady; active: window.activePage === 2; onClicked: window.activePage = 2 }
                    NavButton { Layout.fillWidth: true; indexText: "03"; text: "Zones"; enabled: appController.codeplugReady && appController.zoneCount > 0; active: window.activePage === 3; onClicked: window.activePage = 3 }
                    NavButton { Layout.fillWidth: true; indexText: "04"; text: "Scan Lists"; enabled: appController.codeplugReady && appController.scanListCount > 0; active: window.activePage === 4; onClicked: window.activePage = 4 }
                    NavButton { Layout.fillWidth: true; indexText: "05"; text: "Contacts"; enabled: appController.contactsReady; active: window.activePage === 5; onClicked: window.activePage = 5 }
                    NavButton { Layout.fillWidth: true; indexText: "06"; text: "RX Groups"; enabled: appController.codeplugReady && appController.rxGroupCount > 0; active: window.activePage === 6; onClicked: window.activePage = 6 }

                    Text { text: "+--[ CONFIG ]-------------------------------"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 10; Layout.topMargin: 14; Layout.bottomMargin: 5 }

                    NavButton { Layout.fillWidth: true; indexText: "10"; text: "Radio IDs"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "11"; text: "Settings"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "12"; text: "Display"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "13"; text: "Calibration"; enabled: false }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 174
                        color: window.black2
                        border.color: window.line
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5

                            Text { text: "SYSTEM> STATUS"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { text: "BRANCH : dev-win"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "PHASE  : milestone-5"; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "READ   : " + (appController.liveReadReady ? "SELECTIVE" : appController.backupReady ? "CAPTURED" : "ARMED"); color: appController.liveReadReady || appController.backupReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                            Text { text: "CHAN   : " + (appController.channelsReady ? appController.channelCount : "--"); color: appController.channelsReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                            Text { text: "ZONE   : " + (appController.codeplugReady ? appController.zoneCount : "--"); color: appController.codeplugReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "SCAN   : " + (appController.codeplugReady ? appController.scanListCount : "--"); color: appController.codeplugReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "CONT   : " + (appController.contactsReady ? appController.contactCount : "--"); color: appController.contactsReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "RXG    : " + (appController.codeplugReady ? appController.rxGroupCount : "--"); color: appController.codeplugReady ? window.green : window.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "WRITE  : LOCKED"; color: window.green; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                        }
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: window.activePage - 1

                ConnectionPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onRequestPage: function(page) { window.activePage = page }
                }

                ChannelsPage { Layout.fillWidth: true; Layout.fillHeight: true }
                ZonesPage { Layout.fillWidth: true; Layout.fillHeight: true }
                ScanListsPage { Layout.fillWidth: true; Layout.fillHeight: true }
                ContactsPage { Layout.fillWidth: true; Layout.fillHeight: true }
                RXGroupsPage { Layout.fillWidth: true; Layout.fillHeight: true }
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
                    text: appController.contactsReady ? "[CONTACTS]"
                          : appController.liveReadReady ? "[LIVE READ]"
                          : appController.codeplugReady ? "[CODEPLUG]"
                          : appController.backupReady ? "[BACKUP]"
                          : appController.radioDetected ? "[OK]"
                          : window.operationError ? "[ERR]"
                          : appController.busy ? "[RUN]"
                          : "[IDLE]"
                    color: appController.liveReadReady || appController.codeplugReady || appController.backupReady || appController.radioDetected ? window.green : window.operationError ? window.red : appController.busy ? window.amber : window.silverDim
                    font.family: "Consolas"
                    font.pixelSize: 9
                    font.bold: true
                }
                Text { text: appController.status; color: window.silverDim; font.family: "Consolas"; font.pixelSize: 9; elide: Text.ElideRight; Layout.fillWidth: true }
                Text { text: "KJ6YWD.NET // YWD-PLUG // DEV-WIN // M5"; color: window.silver; font.family: "Consolas"; font.pixelSize: 9 }
            }
        }
    }

    Connections {
        target: appController

        function onChannelsChanged() {
            if (appController.liveReadReady && appController.channelsReady && window.activePage === 1)
                window.activePage = 2
        }

        function onCodeplugChanged() {
            if (!appController.codeplugReady && (window.activePage === 3 || window.activePage === 4 || window.activePage === 6))
                window.activePage = 1
        }

        function onContactsChanged() {
            if (!appController.contactsReady && window.activePage === 5)
                window.activePage = appController.channelsReady ? 2 : 1
        }
    }

    SplashWindow {
        id: startupSplash
        onFinished: windowState.showRestored()
    }
}
