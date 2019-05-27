import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

Column {
    id: root

    property string letter: ""
    property string path: ""
    property string volumeSize: ""

    signal unmountTriggered

    spacing: 2

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 3
        anchors.rightMargin: 5

        spacing: 8

        Text {
            id: letterLabel

            Layout.preferredWidth: 10

            text: root.letter
            color: "white"
            font.pixelSize: 16
            font.bold: true
        }
        Rectangle {
            id: horizontalSeparator_1

            Layout.fillHeight: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5

            width: 1
            color: "#424242"
        }
        Text {
            id: pathLabel
            text: root.path
            Layout.fillWidth: true
            color: "white"
            font.pixelSize: 13
            elide: Text.ElideLeft
        }
        Rectangle {
            id: horizontalSeparator_2

            Layout.fillHeight: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5

            width: 1
            color: "#424242"
        }
        Text {
            id: volumeSizeLabel
            text: root.volumeSize
            Layout.preferredWidth: 70
            color: "white"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
        }
        Rectangle {
            id: horizontalSeparator_3

            Layout.fillHeight: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5

            width: 1
            color: "#424242"
        }
        Button {
            Layout.topMargin: 4
            Layout.bottomMargin: 2
            Layout.preferredHeight: 25
            Layout.preferredWidth: 65

            font.pixelSize: 11

            text: qsTr("Unmount")
            onClicked: root.unmountTriggered()
        }
    }

    Rectangle {
        id: separatorUnderline

        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#424242"
    }
}
