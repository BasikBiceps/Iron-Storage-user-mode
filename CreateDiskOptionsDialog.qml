import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Window 2.12

import QtQuick.Controls.Universal 2.3

Window {
    id: root

    property var availableLetters: []
    property alias letter: comboBoxDisk.currentText
    property alias encrypted: encryptedCheckBox.checked
    property int volumeSize: parseInt(volumeSizeInputField.text)
    property alias topVolumeSize: volumeSizeValidator.top
    property int volumeSizeUnit: 0
    property bool success: false

    signal done
    signal canceled

    Universal.theme: Universal.Light
    Universal.accent: Universal.Cobalt

    flags: Qt.Dialog
    minimumWidth: 320
    minimumHeight: column.height + 5
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight

    Column {
        id: column

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.leftMargin: 10
        anchors.rightMargin: 10

        CheckBox {
            id: encryptedCheckBox

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: -6

            text: qsTr("Encrypted")
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 35

            TextField {
                id: volumeSizeInputField

                Layout.preferredWidth: 100
                Layout.preferredHeight: 25

                font.pixelSize: 12

                placeholderText: qsTr("Volume size")

                validator: IntValidator {
                    id: volumeSizeValidator
                    bottom: 1
                }
            }

            ButtonGroup {
                id: volumeSizeUnitGroup

                onCheckedButtonChanged: {
                    root.volumeSizeUnit = checkedButton.selfIndex
                }
            }

            Repeater {
                model: ["KB", "MB", "GB"]
                RadioButton {
                    id: radio

                    readonly property int selfIndex: index

                    checked: index === 0
                    text: modelData
                    ButtonGroup.group: volumeSizeUnitGroup
                }
            }
        }

        RowLayout {
            width: parent.width
            height: 35

            ComboBox {
                id: comboBoxDisk

                Layout.preferredHeight: 30
                Layout.preferredWidth: 100

                popup.width: width
                popup.height: Math.min(popup.contentHeight, root.height - 20)

                font.pixelSize: 12
                model: root.availableLetters

                delegate: ItemDelegate {
                    width: comboBoxDisk.width
                    height: comboBoxDisk.height
                    contentItem: Text {
                        text: modelData
                        color: "black"
                        font: comboBoxDisk.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    highlighted: comboBoxDisk.highlightedIndex === index
                }
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text: qsTr("Done")
                enabled: root.volumeSize > 0

                onClicked: {
                    root.success = true
                    root.close()
                }
            }
        }
    }

    onVolumeSizeUnitChanged: {
        volumeSizeUnitGroup.checkedButton = volumeSizeUnitGroup.buttons[root.volumeSizeUnit]
    }

    onClosing: {
        if (success) {
            root.done()
        }
        else {
            root.canceled()
        }
        root.success = false
        root.volumeSize = ""
        root.volumeSizeUnit = 0
        root.encrypted = false
    }
}
