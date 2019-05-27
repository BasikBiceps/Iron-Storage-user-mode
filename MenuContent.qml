import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

import Qt.labs.platform 1.0

import backend 1.0

ColumnLayout {
    id: root

    readonly property QmlFacade facade: qmlFacadeContext


    Text {
        text: qsTr("Mounted disks: ") + facade.mountedDisks.length
        color: "white"
        font.family: "Arial"
        font.pixelSize: 14
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true

        color: "transparent"
        border.width: 1
        border.color: "#424242"

        ListView {
            anchors.fill: parent
            anchors.margins: 5
            model: facade.mountedDisks
            interactive: contentHeight > height
            snapMode: ListView.SnapToItem
            clip: true

            delegate: MountedDiskListElement {
                width: parent.width
                letter: modelData.letter
                path: modelData.path
                volumeSize: modelData.volume

                onUnmountTriggered: {
                    facade.unmount(index)
                }
            }
        }
    }

    RowLayout {
        id: row

        readonly property int prefWidth: parent.width / row.children.length

        Layout.fillWidth: true
        spacing: 5

        Button {
            id: createVolumeButton

            Layout.preferredHeight: height
            Layout.preferredWidth: row.prefWidth
            Layout.fillWidth: true
            text: qsTr("Create volume")
            font.capitalization: Font.AllUppercase

            onClicked: {
                facade.updateLetters()
                if (facade.availableLetters.length > 0) {
                    selectFileForCreateDialog.open()
                }
                else {
                    facade.error("Error!", "Cannot create disk. Reason: all disk's letters is busy")
                }
            }
        }

        Button {
            id: mountButton

            Layout.preferredHeight: height
            Layout.preferredWidth: row.prefWidth
            Layout.fillWidth: true
            text: qsTr("Mount")
            font.capitalization: Font.AllUppercase

            onClicked: {
                selectFileForMountDialog.open()
            }
        }

        Button {
            id: unmountAllButton

            Layout.preferredHeight: height
            Layout.preferredWidth: row.prefWidth
            Layout.fillWidth: true
            text: qsTr("Unmount All")
            font.capitalization: Font.AllUppercase
            enabled: facade.mountedDisks.length > 0

            onClicked: {
                facade.unmountAll()
            }
        }
    }

    FileDialog {
        id: selectFileForMountDialog
        nameFilters: ["Image files (*.image)"]

        onAccepted: {
            facade.mount(file.toString().replace("file:///", ""))
        }
    }

    FileDialog {
        id: selectFileForCreateDialog
        nameFilters: ["Image files (*.image)"]

        fileMode: FileDialog.SaveFile

        onAccepted: {
            facade.createDisk(file.toString().replace("file:///", ""))
        }
    }
}
