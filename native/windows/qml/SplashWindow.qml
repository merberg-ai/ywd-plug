import QtQuick
import QtQuick.Window

Window {
    id: splash

    signal finished()

    width: 920
    height: 540
    x: Math.round((Screen.width - width) / 2)
    y: Math.round((Screen.height - height) / 2)
    visible: true
    color: "#050607"
    flags: Qt.SplashScreen | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    transientParent: null

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
        { tag: "OK", tone: "green", text: "BRANDING ASSET PACK MOUNTED" },
        { tag: "OK", tone: "green", text: "REFERENCED CONTACT READER ARMED" },
        { tag: "--", tone: "amber", text: "RADIO WRITE PATH LOCKED" },
        { tag: "OK", tone: "green", text: "WINDOW STATE DATABASE OPEN" },
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
            opacity: 0.26
        }
    }

    Rectangle {
        x: 20
        y: 20
        width: 500
        height: 414
        color: "#060708"
        border.color: splash.line
        border.width: 1
        clip: true

        Image {
            anchors.fill: parent
            anchors.margins: 2
            source: "qrc:/branding/ywd-plug-win-logo1.png"
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            smooth: true
            mipmap: true
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 30
            color: "#C0050607"
            Text {
                anchors.centerIn: parent
                text: "[ YWD-PLUG // NATIVE WINDOWS // PHASE 5 ]"
                color: splash.silver
                font.family: "Consolas"
                font.pixelSize: 10
                font.letterSpacing: 1.0
            }
        }
    }

    Rectangle {
        x: 536
        y: 20
        width: 364
        height: 414
        color: "#080a0b"
        border.color: splash.line
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 9

            Text {
                text: "YWD-PLUG BOOT ROM"
                color: splash.silverBright
                font.family: "Consolas"
                font.pixelSize: 16
                font.bold: true
            }
            Text {
                text: "v0.1-dev // WIN32 // M5"
                color: splash.amber
                font.family: "Consolas"
                font.pixelSize: 10
            }
            Text {
                text: "POST> INITIALIZING RADIO WORKSTATION"
                color: splash.silverDim
                font.family: "Consolas"
                font.pixelSize: 9
            }

            Rectangle { width: parent.width; height: 1; color: splash.line }

            Item {
                width: parent.width
                height: 292

                Column {
                    anchors.fill: parent
                    spacing: 8

                    Repeater {
                        model: Math.min(splash.bootStep, splash.bootLines.length)

                        Row {
                            spacing: 9
                            height: 17

                            Text {
                                text: "[" + splash.bootLines[index].tag + "]"
                                color: splash.toneColor(splash.bootLines[index].tone)
                                font.family: "Consolas"
                                font.pixelSize: 9
                                font.bold: true
                            }
                            Text {
                                width: 290
                                text: splash.bootLines[index].text
                                color: splash.silver
                                elide: Text.ElideRight
                                font.family: "Consolas"
                                font.pixelSize: 9
                            }
                        }
                    }

                    Row {
                        visible: splash.bootStep < splash.bootLines.length
                        spacing: 9
                        height: 17

                        Text { text: "[>>]"; color: splash.amber; font.family: "Consolas"; font.pixelSize: 9; font.bold: true }
                        Text { text: "INITIALIZING..."; color: splash.silverDim; font.family: "Consolas"; font.pixelSize: 9 }
                        Text {
                            text: "_"
                            color: splash.amber
                            font.family: "Consolas"
                            font.pixelSize: 9
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
        }
    }

    Text {
        x: 20
        y: 450
        text: "LOAD> " + Math.round(splash.progress * 100).toString().padStart(3, "0") + "%"
        color: splash.progress >= 1.0 ? splash.green : splash.silver
        font.family: "Consolas"
        font.pixelSize: 10
        font.bold: true
    }

    Row {
        x: 92
        y: 450
        spacing: 3

        Repeater {
            model: 50
            Rectangle {
                width: 13
                height: 13
                border.width: 1
                border.color: index < Math.floor(splash.progress * 50) ? splash.silverDim : splash.line
                color: {
                    if (index >= Math.floor(splash.progress * 50)) return "transparent"
                    if (splash.progress >= 1.0) return splash.green
                    if (index === Math.floor(splash.progress * 50) - 1) return splash.amber
                    return splash.silver
                }
            }
        }
    }

    Rectangle {
        x: 20
        y: 482
        width: 880
        height: 38
        color: "#080a0b"
        border.color: splash.progress >= 1.0 ? "#235d31" : splash.line
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 14
            Text {
                text: splash.progress >= 1.0 ? "[ SYSTEM READY ]" : "[ POST IN PROGRESS ]"
                color: splash.progress >= 1.0 ? splash.green : splash.amber
                font.family: "Consolas"
                font.pixelSize: 10
                font.bold: true
            }
            Text {
                text: "KJ6YWD.NET // DM-32UV // READ-ONLY SAFE MODE"
                color: splash.silverDim
                font.family: "Consolas"
                font.pixelSize: 10
            }
        }
    }

    Timer {
        id: bootTimer
        interval: 185
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
