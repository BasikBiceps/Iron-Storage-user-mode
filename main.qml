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

    CreateDiskOptionsDialog {
        id: createDiskOptionsDialog

        availableLetters: facade.availableLetters

        onDone: {
            facade.optionsForCreateDiskEntered(encrypted, letter, volumeSize, volumeSizeUnit)
        }

        onCanceled: {
            facade.optionsForCreateDiskCanceled()
        }
    }

    Dialogs.MessageDialog {
        id: errorBox

        standardButtons: StandardButton.Ok
    }

//    SplashScreen {
//        anchors.fill: parent
//    }

    Connections {
        target: facade

        onPasswordRequired: {
            passwordInputDialog.show()
        }

        onOptionsForCreateDiskRequired: {
            createDiskOptionsDialog.show()
        }

        onError: {
            errorBox.title = title
            errorBox.text = text
            errorBox.open()
        }
    }
}
