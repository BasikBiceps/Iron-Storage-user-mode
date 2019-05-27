import QtQuick 2.12
import QtQuick.Controls 2.5

import QtQuick.Controls.Universal 2.3

Rectangle {
    id: root

    color: "black"
    visible: opacity > 0

    Column {
        spacing: 10
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -50

        Text {
            id: ironText

            anchors.horizontalCenter: parent.horizontalCenter
            text: "Iron"
            font.pixelSize: 72
            font.family: "Lucida Console"
            color: "white"
            opacity: 0
        }

        Text {
            id: storageText

            anchors.horizontalCenter: parent.horizontalCenter
            text: "Storage"
            font.pixelSize: 72
            font.family: "Lucida Console"
            color: "white"
            opacity: 0
        }

        Text {
            id: developer1Text

            anchors.horizontalCenter: parent.horizontalCenter
            text: "Baxter"
            font.pixelSize: 16
            font.family: "Lucida Console"
            color: "white"
            opacity: 0
        }
    }

    MouseArea {
        id: clickGuard
        anchors.fill: parent
        enabled: root.visible
    }

    SequentialAnimation {
        id: animation
        running: root.visible

        NumberAnimation {
            target: ironText
            property: "opacity"
            to: 1
            duration: 2000
        }
        NumberAnimation {
            target: storageText
            property: "opacity"
            to: 1
            duration: 2000
        }
        PropertyAction {
            target: root
            property: "visible"
            value: false
        }
    }
}
