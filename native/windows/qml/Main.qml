import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import YWDPlug

ApplicationWindow {
    id: window

    width: 1440
    height: 900
    minimumWidth: 1120
    minimumHeight: 720
    visible: true
    title: "YWD-Plug // Native Windows"
    color: "#05090d"

    property color bg: "#05090d"
    property color panel: "#08151c"
    property color panel2: "#0b1b23"
    property color line: "#1d4654"
    property color lineStrong: "#2a6678"
    property color cyan: "#62e9ff"
    property color magenta: "#ff68d4"
    property color textMain: "#d8edf2"
    property color textMuted: "#7ea3ad"
    property color good: "#6bf4a5"

    background: Rectangle {
        color: window.bg

        Rectangle {
            width: parent.width * 0.9
            height: 360
            anchors.horizontalCenter: parent.horizontalCenter
            y: -220
            radius: width / 2
            color: "#123541"
            opacity: 0.32
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 78
            color: "#071117"
            border.color: window.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                spacing: 16

                Image {
                    source: "qrc:/qt/qml/YWDPlug/resources/ywd-plug.svg"
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 42
                    fillMode: Image.PreserveAspectFit
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "YWD-PLUG"
                        color: window.cyan
                        font.family: "Consolas"
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Text {
                        text: "NATIVE RADIO PROGRAMMING WORKSTATION // WINDOWS"
                        color: window.textMuted
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.letterSpacing: 1.15
                    }
                }

                Item { Layout.fillWidth: true }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 5
                    Text {
                        Layout.alignment: Qt.AlignRight
                        text: appController.radioDetected ? appController.radioModel : "BAOFENG DM-32UV"
                        color: appController.radioDetected ? window.good : window.textMuted
                        font.family: "Consolas"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    StatusPill {
                        Layout.alignment: Qt.AlignRight
                        text: appController.busy ? "PROBING RADIO" : appController.radioDetected ? "RADIO READY" : "NOT CONNECTED"
                        busy: appController.busy
                        good: appController.radioDetected
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 214
                Layout.fillHeight: true
                color: "#061017"
                border.color: window.line
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 4

                    Text {
                        text: "RADIO"
                        color: window.magenta
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.bold: true
                        Layout.leftMargin: 8
                        Layout.topMargin: 6
                        Layout.bottomMargin: 5
                    }

                    NavButton { Layout.fillWidth: true; text: "Connection"; active: true }
                    NavButton { Layout.fillWidth: true; text: "Channels"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Zones"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Scan Lists"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Contacts"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "RX Groups"; enabled: false }

                    Text {
                        text: "RADIO CONFIG"
                        color: window.magenta
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.bold: true
                        Layout.leftMargin: 8
                        Layout.topMargin: 18
                        Layout.bottomMargin: 5
                    }

                    NavButton { Layout.fillWidth: true; text: "Radio IDs"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Settings"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Display"; enabled: false }
                    NavButton { Layout.fillWidth: true; text: "Calibration"; enabled: false }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 78
                        radius: 9
                        color: "#081820"
                        border.color: window.line

                        Column {
                            anchors.centerIn: parent
                            spacing: 4
                            Text { text: "DEV-WIN"; color: window.cyan; font.family: "Consolas"; font.pixelSize: 11; font.bold: true }
                            Text { text: "MILESTONE 1"; color: window.textMuted; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { text: "READ-ONLY PROBE"; color: window.good; font.family: "Consolas"; font.pixelSize: 10 }
                        }
                    }
                }
            }

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: content.implicitHeight + 48
                clip: true

                ColumnLayout {
                    id: content
                    width: parent.width
                    spacing: 18
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 28

                    Item { Layout.preferredHeight: 10 }

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "RADIO CONNECTION"
                            color: window.textMain
                            font.family: "Consolas"
                            font.pixelSize: 22
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "NATIVE Qt 6 / C++20"
                            color: window.cyan
                            font.family: "Consolas"
                            font.pixelSize: 11
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Select the Windows COM port used by the DM-32UV programming cable. The first milestone performs PSEARCH → PASSSTA → SYSINFO only; it does not enter programming mode or write radio memory."
                        wrapMode: Text.WordWrap
                        color: window.textMuted
                        font.family: "Consolas"
                        font.pixelSize: 12
                        lineHeight: 1.25
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 264
                        radius: 12
                        color: "#08151c"
                        border.color: window.lineStrong
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 22
                            spacing: 14

                            Text {
                                text: "SERIAL INTERFACE"
                                color: window.magenta
                                font.family: "Consolas"
                                font.pixelSize: 11
                                font.bold: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ComboBox {
                                    id: portBox
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 44
                                    model: appController.ports
                                    textRole: "label"
                                    valueRole: "name"
                                    enabled: !appController.busy && count > 0

                                    contentItem: Text {
                                        leftPadding: 12
                                        text: portBox.displayText
                                        color: window.textMain
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                        font.family: "Consolas"
                                        font.pixelSize: 12
                                    }
                                    background: Rectangle {
                                        radius: 7
                                        color: "#061017"
                                        border.color: portBox.activeFocus ? window.cyan : window.lineStrong
                                    }
                                }

                                Button {
                                    text: "REFRESH"
                                    enabled: !appController.busy
                                    onClicked: appController.refreshPorts()
                                    contentItem: Text { text: parent.text; color: window.cyan; font.family: "Consolas"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    background: Rectangle { radius: 7; color: parent.hovered ? "#12303b" : "#0b1b23"; border.color: window.lineStrong }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Button {
                                    Layout.preferredWidth: 190
                                    Layout.preferredHeight: 46
                                    text: appController.busy ? "PROBING..." : "PROBE RADIO"
                                    enabled: !appController.busy && portBox.count > 0
                                    onClicked: appController.probePort(portBox.currentValue)
                                    contentItem: Text { text: parent.text; color: "#031013"; font.family: "Consolas"; font.pixelSize: 12; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    background: Rectangle { radius: 8; color: parent.enabled ? window.cyan : "#35515a" }
                                }

                                BusyIndicator {
                                    running: appController.busy
                                    visible: running
                                    palette.dark: window.cyan
                                    Layout.preferredWidth: 28
                                    Layout.preferredHeight: 28
                                }

                                Item { Layout.fillWidth: true }

                                Text {
                                    text: appController.radioDetected ? appController.detectedPort : "115200 // 8N1 // NO FLOW"
                                    color: appController.radioDetected ? window.good : window.textMuted
                                    font.family: "Consolas"
                                    font.pixelSize: 11
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 54
                                radius: 8
                                color: "#050d12"
                                border.color: appController.radioDetected ? "#285c45" : window.line

                                Text {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    text: appController.status
                                    color: appController.radioDetected ? window.good : appController.status.startsWith("PROBE FAILED") ? "#ff6c7d" : window.cyan
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    font.family: "Consolas"
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 180
                        radius: 12
                        color: "#071219"
                        border.color: window.line

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 22
                            spacing: 10

                            Text { text: "NEXT // PROTOCOL PORT"; color: window.magenta; font.family: "Consolas"; font.bold: true; font.pixelSize: 11 }
                            Text {
                                Layout.fillWidth: true
                                text: appController.radioDetected
                                      ? "DM-32UV identification succeeded. Next we port programming-mode entry and block-safe memory reads from the proven browser driver, then build the native channel model/editor on top."
                                      : "Once the native probe identifies the radio reliably on your hardware, the next slice ports PROGRAM mode and read-only memory block access."
                                color: window.textMain
                                wrapMode: Text.WordWrap
                                font.family: "Consolas"
                                font.pixelSize: 12
                                lineHeight: 1.3
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }

                    Item { Layout.preferredHeight: 10 }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "#03080b"
            border.color: window.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                spacing: 16

                Text { text: "●"; color: appController.radioDetected ? window.good : window.textMuted; font.pixelSize: 10 }
                Text { text: appController.status; color: window.textMuted; font.family: "Consolas"; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
                Text { text: "KJ6YWD.NET // YWD-PLUG"; color: window.cyan; font.family: "Consolas"; font.pixelSize: 10 }
            }
        }
    }
}
