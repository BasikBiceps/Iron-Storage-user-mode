import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Window 2.12

import QtQuick.Controls.Universal 2.3

Window {
    id: root

    flags: Qt.Dialog
    minimumWidth: 200
    minimumHeight: 65
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight

    Universal.accent: Universal.Steel

    signal entered(string password)
    signal canceled

    Text {
        id: label
        text: qsTr("Enter password:")

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 10
    }

    TextField {
        id: input

        anchors.top: label.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        anchors.topMargin: 10
        height: 25

        echoMode: TextInput.Password
        maximumLength: 32
        horizontalAlignment: TextInput.Right
        verticalAlignment: TextInput.Right
        focus: true

        onEditingFinished: {
            if (length > 0) {
                root.close()
            }
        }
    }

    onClosing: {
        var tmp = input.text
        input.text = ""
        if (root.tmp === "") {
            root.canceled()
        }
        else {
            root.entered(tmp)
        }
    }
}
