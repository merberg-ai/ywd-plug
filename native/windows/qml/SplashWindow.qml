import QtQuick
import QtQuick.Window

Window {
    id: splash

    signal finished()

    width: 760
    height: 430
    x: Math.round((Screen.width - width) / 2)
    y: Math.round((Screen.height - height) / 2)
    visible: true
    color: "#050607"
    flags: Qt.SplashScreen | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property color bg: "#050607"
    property color panel: "#0a0c0e"
    property color silver: "#c7ccd1"
    property color silverBright: "#eef0f2"
    property color silverDim: "#737980"
    property color line: "#34383d"
    property color amber: "#d79a2b"
    property color green: "#58d878"
    property color red: "#e45c5c"

    property int bootStep: 0
    property var bootLines: [
        { tag: "OK", tone: "green", text: "YWD CORE INITIALIZED" },
        { tag: "OK", tone: "green", text: "WIN32/x64 RUNTIME ONLINE" },
        { tag: "OK", tone: "green", text: "NATIVE SERIAL BACKEND LOADED" },
        { tag: "OK", tone: "green", text: "DM-32UV PROTOCOL TABLE READY" },
        { tag: "OK", tone: "green", text: "WINDOW STATE DATABASE OPEN" },
        { tag: "--", tone: "amber", text: "RADIO WRITE PATH LOCKED" },
        { tag: "OK", tone: "green", text: "QML TERMINAL WORKSTATION READY" },
        { tag: "OK", tone: "green", text: "HANDOFF TO OPERATOR CONSOLE" }
    ]

    property real progress: bootLines.length > 0 ? Math.min(1.0, bootStep / bootLines.length) : 1.0

    function toneColor(tone) {
        if (tone === "green") return green
        if (tone === "red") return red
        if (tone === "amber") return amber
        return silver
    }

    Rectangle {
        anchors.fill: parent
        color: splash.bg
        border.color: splash.line
        border.width: 1
    }

    Repeater {
        model: Math.ceil(splash.height / 6)
        Rectangle {
            x: 1
            y: index * 6
            width: splash.width - 2
            height: 1
            color: "#14171a"
            opacity: 0.28
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 10

        Text {
            text:
                  "$$      $$ $$      $$ $$$$$$$          $$$$$$$  $$       $$   $$   $$$$$$\n"
                + " $$    $$  $$  $$  $$ $$    $$         $$    $$ $$       $$   $$  $$    $$\n"
                + "  $$  $$   $$ $$$$ $$ $$    $$  $$$$$  $$$$$$$  $$       $$   $$  $$\n"
                + "   $$$$    $$$$  $$$$ $$    $$         $$       $$       $$   $$  $$  $$$\n"
                + "    $$     $$$    $$$ $$    $$         $$       $$       $$   $$  $$   $$\n"
                + "    $$     $$      $$ $$$$$$$          $$       $$$$$$$$  $$$$$$$   $$$$$$"
            color: splash.silverBright
            font.family: "Consolas"
            font.pixelSize: 8
            font.bold: true
            lineHeight: 0.86
            renderType: Text.NativeRendering
        }

        Row {
            spacing: 12
            Text {
                text: "YWD-PLUG BOOT ROM"
                color: splash.amber
                font.family: "Consolas"
                font.pixelSize: 13
                font.bold: true
            }
            Text {
                text: "v0.1-dev // NATIVE WINDOWS"
                color: splash.silverDim
                font.family: "Consolas"
                font.pixelSize: 11
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: splash.line
        }

        Item {
            width: parent.width
            height: 164

            Column {
                anchors.fill: parent
                spacing: 7

                Repeater {
                    model: Math.min(splash.bootStep, splash.bootLines.length)

                    Row {
                        spacing: 10
                        height: 14

                        Text {
                            text: "[" + splash.bootLines[index].tag + "]"
                            color: splash.toneColor(splash.bootLines[index].tone)
                            font.family: "Consolas"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        Text {
                            text: splash.bootLines[index].text
                            color: splash.silver
                            font.family: "Consolas"
                            font.pixelSize: 10
                        }
                    }
                }

                Row {
                    visible: splash.bootStep < splash.bootLines.length
                    spacing: 10
                    height: 14

                    Text {
                        text: "[>>]"
                        color: splash.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                        font.bold: true
                    }
                    Text {
                        text: "INITIALIZING..."
                        color: splash.silverDim
                        font.family: "Consolas"
                        font.pixelSize: 10
                    }
                    Text {
                        text: "_"
                        color: splash.amber
                        font.family: "Consolas"
                        font.pixelSize: 10
                        SequentialAnimation on opacity {
                            running: splash.visible && splash.bootStep < splash.bootLines.length
                            loops: Animation.Infinite
                            NumberAnimation { to: 0; duration: 260 }
                            NumberAnimation { to: 1; duration: 260 }
                        }
                    }
                }
            }
        }

        Text {
            text: "LOAD> " + Math.round(splash.progress * 100).toString().padStart(3, "0") + "%"
            color: splash.silver
            font.family: "Consolas"
            font.pixelSize: 10
            font.bold: true
        }

        Row {
            spacing: 3

            Repeater {
                model: 40
                Rectangle {
                    width: 13
                    height: 12
                    border.width: 1
                    border.color: index < Math.floor(splash.progress * 40) ? splash.silverDim : splash.line
                    color: {
                        if (index >= Math.floor(splash.progress * 40)) return "transparent"
                        if (splash.progress >= 1.0) return splash.green
                        if (index === Math.floor(splash.progress * 40) - 1) return splash.amber
                        return splash.silver
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: 12

            Text {
                text: splash.progress >= 1.0 ? "SYSTEM READY" : "POST IN PROGRESS"
                color: splash.progress >= 1.0 ? splash.green : splash.amber
                font.family: "Consolas"
                font.pixelSize: 10
                font.bold: true
            }
            Text {
                text: "// KJ6YWD.NET // DEV-WIN"
                color: splash.silverDim
                font.family: "Consolas"
                font.pixelSize: 10
            }
        }
    }

    Timer {
        id: bootTimer
        interval: 210
        repeat: true
        running: splash.visible
        triggeredOnStart: true
        onTriggered: {
            if (splash.bootStep < splash.bootLines.length) {
                splash.bootStep += 1
                return
            }
            stop()
            handoffTimer.start()
        }
    }

    Timer {
        id: handoffTimer
        interval: 420
        repeat: false
        onTriggered: {
            splash.finished()
            splash.close()
        }
    }
}
