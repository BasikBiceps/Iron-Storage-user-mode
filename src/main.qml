import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Dialogs 1.1 as Dialogs

import Qt.labs.platform 1.0

import QtQuick.Controls.Universal 2.3

import backend 1.0

ApplicationWindow {
    id: mainWindow

    readonly property QmlFacade facade: qmlFacadeContext

    visible: true
    minimumHeight: 480
    minimumWidth: 640
    maximumHeight: minimumHeight
    maximumWidth: minimumWidth
    title: qsTr("Iron Storage")
    opacity: 1

    Universal.theme: Universal.Dark
    Universal.accent: Universal.Cobalt

    MenuContent {
        id: menu

        anchors.fill: parent
        anchors.margins: 10
        visible: !facade.busy
    }

    BusyIndicator {
        anchors.centerIn: parent
        height: 100
        width: height
        running: visible
        visible: facade.busy
    }

    PasswordInputDialog {
        id: passwordInputDialog

        onEntered: {
            facade.passwordEntered(password)
        }

        onCanceled: {
            facade.passwordCanceled()
        }
    }

    DiskOptionsDialog {
        id: diskOptionsDialog

        availableLetters: facade.availableLetters

        onDone: {
            if (diskOptionsDialog.chooseDisk) {
                facade.chooseDiskEntered(letter)
            }
            else {
                facade.optionsForCreateDiskEntered(encrypted, letter, volumeSize, volumeSizeUnit)
            }
        }

        onCanceled: {
            if (diskOptionsDialog.chooseDisk) {
                facade.chooseDiskCanceled()
            }
            else {
                facade.optionsForCreateDiskCanceled()
            }
        }
    }

    Dialogs.MessageDialog {
        id: errorBox

        standardButtons: StandardButton.Ok
    }

    SplashScreen {
        anchors.fill: parent
    }

    Connections {
        target: facade

        onPasswordRequired: {
            passwordInputDialog.show()
        }

        onOptionsForCreateDiskRequired: {
            diskOptionsDialog.show()
        }

        onChooseDiskRequired: {
            diskOptionsDialog.chooseDisk = true
            diskOptionsDialog.show()
        }

        onError: {
            errorBox.title = title
            errorBox.text = text
            errorBox.open()
        }
    }
}
