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
    visible: true
    title: "YWD-Plug // Native Windows"
    color: "#030301"

    property color black: "#030301"
    property color black2: "#070602"
    property color panel: "#0a0802"
    property color amber: "#ffb000"
    property color amberBright: "#ffd166"
    property color amberDim: "#8a5d00"
    property color amberFaint: "#3b2800"
    property color green: "#55ff77"
    property color greenDim: "#1d6b2d"
    property color red: "#ff4d4d"
    property color muted: "#80642d"
    property bool probeError: appController.status.indexOf("PROBE FAILED") === 0

    property string asciiBanner:
          "$$      $$ $$      $$ $$$$$$$          $$$$$$$  $$       $$   $$   $$$$$$\n"
        + " $$    $$  $$  $$  $$ $$    $$         $$    $$ $$       $$   $$  $$    $$\n"
        + "  $$  $$   $$ $$$$ $$ $$    $$  $$$$$  $$$$$$$  $$       $$   $$  $$\n"
        + "   $$$$    $$$$  $$$$ $$    $$         $$       $$       $$   $$  $$  $$$\n"
        + "    $$     $$$    $$$ $$    $$         $$       $$       $$   $$  $$   $$\n"
        + "    $$     $$      $$ $$$$$$$          $$       $$$$$$$$  $$$$$$$   $$$$$$"

    background: Item {
        Rectangle {
            anchors.fill: parent
            color: window.black
        }

        // Very subtle CRT scan lines. Purely decorative; no input handling.
        Repeater {
            model: Math.ceil(window.height / 5)
            Rectangle {
                width: window.width
                height: 1
                y: index * 5
                color: "#211500"
                opacity: 0.14
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // -----------------------------------------------------------------
        // TERMINAL HEADER
        // -----------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 146
            color: "#050402"
            border.color: window.amberDim
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
                    color: window.amber
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
                    color: window.amberFaint
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 5

                    Text {
                        text: "YWD-PLUG / NATIVE WINDOWS"
                        color: window.amberBright
                        font.family: "Consolas"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Text {
                        text: "RADIO PROGRAMMING WORKSTATION"
                        color: window.amberDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.letterSpacing: 1.2
                    }
                    Item { Layout.preferredHeight: 5 }
                    Text {
                        text: "HOST    : WIN32 / x64"
                        color: window.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text {
                        text: "TARGET  : " + (appController.radioDetected ? appController.radioModel : "DM-32UV / DP570UV")
                        color: appController.radioDetected ? window.green : window.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text {
                        text: "PORT    : " + (appController.radioDetected ? appController.detectedPort : "UNBOUND")
                        color: appController.radioDetected ? window.green : window.muted
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text {
                        text: "ACCESS  : READ-ONLY PROBE"
                        color: window.amberDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                }

                StatusPill {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    text: appController.busy ? "PROBING" : probeError ? "FAULT" : appController.radioDetected ? "LINK OK" : "STANDBY"
                    busy: appController.busy
                    good: appController.radioDetected
                    error: window.probeError
                }
            }
        }

        // -----------------------------------------------------------------
        // BODY
        // -----------------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 224
                Layout.fillHeight: true
                color: "#050402"
                border.color: window.amberFaint
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 2

                    Text {
                        text: "+--[ RADIO ]--------------------------------"
                        color: window.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                        Layout.bottomMargin: 5
                    }

                    NavButton { Layout.fillWidth: true; indexText: "01"; text: "Connection"; active: true }
                    NavButton { Layout.fillWidth: true; indexText: "02"; text: "Channels"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "03"; text: "Zones"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "04"; text: "Scan Lists"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "05"; text: "Contacts"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "06"; text: "RX Groups"; enabled: false }

                    Text {
                        text: "+--[ CONFIG ]-------------------------------"
                        color: window.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                        Layout.topMargin: 14
                        Layout.bottomMargin: 5
                    }

                    NavButton { Layout.fillWidth: true; indexText: "10"; text: "Radio IDs"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "11"; text: "Settings"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "12"; text: "Display"; enabled: false }
                    NavButton { Layout.fillWidth: true; indexText: "13"; text: "Calibration"; enabled: false }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 104
                        color: "#070602"
                        border.color: window.amberFaint
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5

                            Text { text: "SYSTEM> STATUS"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10; font.bold: true }
                            Text { text: "BRANCH : dev-win"; color: window.muted; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "PHASE  : milestone-1"; color: window.muted; font.family: "Consolas"; font.pixelSize: 9 }
                            Text { text: "WRITE  : LOCKED"; color: window.green; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                        }
                    }
                }
            }

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

                        Text {
                            text: "> RADIO CONNECTION"
                            color: window.amberBright
                            font.family: "Consolas"
                            font.pixelSize: 21
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "SYS://RADIO/CONNECTION"
                            color: window.amberDim
                            font.family: "Consolas"
                            font.pixelSize: 10
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Initialize the native Win32 serial transport and interrogate the selected radio. This stage issues PSEARCH, PASSSTA and SYSINFO only. PROGRAM mode remains hard-disabled."
                        wrapMode: Text.WordWrap
                        color: "#ba8a2c"
                        font.family: "Consolas"
                        font.pixelSize: 11
                        lineHeight: 1.25
                    }

                    // -----------------------------------------------------
                    // SERIAL PANEL
                    // -----------------------------------------------------
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 286
                        color: "#050402"
                        border.color: window.amberDim
                        border.width: 1

                        Text {
                            text: "[ SERIAL INTERFACE / WIN32 ]"
                            color: window.amberBright
                            font.family: "Consolas"
                            font.pixelSize: 10
                            font.bold: true
                            x: 14
                            y: -7
                            leftPadding: 6
                            rightPadding: 6

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: -2
                                color: window.black
                                z: -1
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 22
                            anchors.topMargin: 28
                            spacing: 12

                            Text {
                                text: "PORT> SELECT DEVICE"
                                color: window.amber
                                font.family: "Consolas"
                                font.pixelSize: 10
                            }

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
                                        color: portBox.enabled ? window.amberBright : window.amberFaint
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
                                        color: "#030301"
                                        border.color: portBox.activeFocus ? window.amberBright : window.amberDim
                                        border.width: 1
                                    }

                                    delegate: ItemDelegate {
                                        width: portBox.width
                                        height: 34
                                        contentItem: Text {
                                            text: "> " + modelData.label
                                            color: highlighted ? "#030301" : window.amber
                                            font.family: "Consolas"
                                            font.pixelSize: 10
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            color: highlighted ? window.amber : "#050402"
                                        }
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

                                        background: Rectangle {
                                            color: "#050402"
                                            border.color: window.amberDim
                                            border.width: 1
                                        }
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
                                        color: refreshButton.hovered ? "#030301" : window.amber
                                        font.family: "Consolas"
                                        font.pixelSize: 10
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: refreshButton.hovered ? window.amber : "#050402"
                                        border.color: window.amberDim
                                        border.width: 1
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Button {
                                    id: probeButton
                                    Layout.preferredWidth: 218
                                    Layout.preferredHeight: 44
                                    text: appController.busy ? "[ PROBING... ]" : "[ EXECUTE PROBE ]"
                                    enabled: !appController.busy && portBox.count > 0
                                    onClicked: appController.probePort(portBox.currentValue)
                                    contentItem: Text {
                                        text: probeButton.text
                                        color: !probeButton.enabled ? "#5c3d00" : probeButton.hovered ? "#030301" : window.amberBright
                                        font.family: "Consolas"
                                        font.pixelSize: 11
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: probeButton.enabled && probeButton.hovered ? window.amber : "#050402"
                                        border.color: probeButton.enabled ? window.amber : window.amberFaint
                                        border.width: 1
                                    }
                                }

                                Text {
                                    text: appController.busy ? "* SERIAL ACTIVITY" : appController.radioDetected ? "+ LINK ESTABLISHED" : "- LINK IDLE"
                                    color: appController.busy ? window.amber : appController.radioDetected ? window.green : window.muted
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

                                Text {
                                    text: portBox.count > 0 ? (portBox.currentValue + " / 115200 / 8N1") : "NO COM PORT"
                                    color: window.amberDim
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 58
                                color: "#020201"
                                border.color: window.probeError ? window.red : appController.radioDetected ? window.greenDim : window.amberFaint
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    spacing: 10

                                    Text {
                                        text: window.probeError ? "ERR>" : appController.radioDetected ? "OK >" : "SYS>"
                                        color: window.probeError ? window.red : appController.radioDetected ? window.green : window.amber
                                        font.family: "Consolas"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: appController.status
                                        color: window.probeError ? window.red : appController.radioDetected ? window.green : window.amber
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

                    // -----------------------------------------------------
                    // SESSION / DIAGNOSTIC PANEL
                    // -----------------------------------------------------
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 218
                        color: "#050402"
                        border.color: window.amberFaint
                        border.width: 1

                        Text {
                            text: "[ SESSION STATUS ]"
                            color: window.amberBright
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
                            Text { text: "[OK]  DM-32UV IDENTIFICATION SEQUENCE LOADED"; color: window.green; font.family: "Consolas"; font.pixelSize: 10 }
                            Text { text: "[--]  PROGRAM MODE DISABLED / RADIO WRITES LOCKED"; color: window.amber; font.family: "Consolas"; font.pixelSize: 10 }
                            Text {
                                text: appController.radioDetected
                                      ? "[OK]  PSEARCH / PASSSTA / SYSINFO ACCEPTED BY " + appController.radioModel
                                      : window.probeError
                                        ? "[!!]  LAST PROBE FAILED -- REVIEW STATUS ABOVE"
                                        : "[--]  AWAITING RADIO PROBE"
                                color: appController.radioDetected ? window.green : window.probeError ? window.red : window.amberDim
                                font.family: "Consolas"
                                font.pixelSize: 10
                            }
                            Text {
                                text: appController.radioDetected
                                      ? "NEXT> PORT PROGRAM-MODE ENTRY + BLOCK-SAFE READ PATH"
                                      : "NEXT> ESTABLISH A VALID DM-32UV LINK"
                                color: window.amberBright
                                font.family: "Consolas"
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }

                    Item { Layout.preferredHeight: 10 }
                }
            }
        }

        // -----------------------------------------------------------------
        // TERMINAL STATUS BAR
        // -----------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: "#020201"
            border.color: window.amberDim
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16

                Text {
                    text: appController.radioDetected ? "[OK]" : window.probeError ? "[ERR]" : appController.busy ? "[RUN]" : "[IDLE]"
                    color: appController.radioDetected ? window.green : window.probeError ? window.red : window.amber
                    font.family: "Consolas"
                    font.pixelSize: 9
                    font.bold: true
                }
                Text {
                    text: appController.status
                    color: window.amberDim
                    font.family: "Consolas"
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text {
                    text: "KJ6YWD.NET // YWD-PLUG // DEV-WIN"
                    color: window.amber
                    font.family: "Consolas"
                    font.pixelSize: 9
                }
            }
        }
    }
}
