// main.qml (closeTab logic wired in TabView)
import QtQuick 2.0
import QtQuick.Controls

TabView {
    id: tabView

    function getTab(index) {
        return tabView.getTab ? tabView.getTab(index) : tabView.itemAt(index);
    }

    function closeTab(index) {
        let tabItem = getTab(index);
        if (!tabItem) return;

        if (!tabItem.isModified) {
            tabView.removeTab(index);
            return;
        }

        let dialog = tabItem.closeConfirmDialog;
        dialog.visible = true;
        dialog.onAccepted = function(result) {
            if (result === Dialog.Save) {
                let editor = tabItem.editor;
                if (tabItem.filePath) {
                    controller.saveFile(tabItem.filePath, editor.text);
                    tabView.removeTab(index);
                } else {
                    saveDialog.onAccepted = function() {
                        controller.saveFile(saveDialog.fileUrl.toString().substring(7), editor.text);
                        tabView.removeTab(index);
                    }
                    saveDialog.open();
                }
            } else if (result === Dialog.Discard) {
                tabView.removeTab(index);
            }
        }
    }

    TabBar.onClicked: (index, event) => {
        if (event.button === Qt.MiddleButton) {
            closeTab(index);
        }
    }

    TabBar.tabButton: Rectangle {
        implicitWidth: 24
        implicitHeight: 24
        color: "transparent"

        Image {
            anchors.centerIn: parent
            source: "qrc:/icons/close.svg" // Make sure this icon exists or replace it
            width: 12; height: 12
            MouseArea {
                anchors.fill: parent
                onClicked: closeTab(index)
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    MenuBar {
        Menu {
            title: "Encode"
            MenuItem { text: "Base64"; onTriggered: activeEditor.text = controller.encodeBase64(activeEditor.text) }
            MenuItem { text: "ROT13"; onTriggered: activeEditor.text = controller.encodeRot13(activeEditor.text) }
            MenuItem {
                text: "Caesar Cipher..."
                onTriggered: caesarDialogEncode.open()
            }
            MenuItem { text: "Binary"; onTriggered: activeEditor.text = controller.encodeBinary(activeEditor.text) }
            MenuItem { text: "Pig Latin"; onTriggered: activeEditor.text = controller.encodePigLatin(activeEditor.text) }
            MenuItem { text: "Morse"; onTriggered: activeEditor.text = controller.encodeMorse(activeEditor.text) }
            MenuItem { text: "Atbash"; onTriggered: activeEditor.text = controller.encodeAtbash(activeEditor.text) }
        }
        Menu {
            title: "Codecs"
            MenuItem { text: "Base64"; onTriggered: activeEditor.text = controller.decodeBase64(activeEditor.text) }
            MenuItem { text: "ROT13"; onTriggered: activeEditor.text = controller.decodeRot13(activeEditor.text) }
            MenuItem { text: "Caesar Cipher"; onTriggered: caesarDialogDecode.open() }
            MenuItem { text: "Binary"; onTriggered: activeEditor.text = controller.decodeBinary(activeEditor.text) }
            MenuItem { text: "Pig Latin"; onTriggered: activeEditor.text = controller.decodePigLatin(activeEditor.text) }
            MenuItem { text: "Morse"; onTriggered: activeEditor.text = controller.decodeMorse(activeEditor.text) }
            MenuItem { text: "Atbash"; onTriggered: activeEditor.text = controller.decodeAtbash(activeEditor.text) }
        }
    }

    Dialog {
        id: caesarDialogEncode
        modal: true
        title: "Caesar Cipher - Encode"
        standardButtons: Dialog.Ok | Dialog.Cancel
        Column {
            spacing: 10
            Label { text: "Shift amount:" }
            SpinBox { id: caesarShiftEncode; from: -25; to: 25; value: 3 }
        }
        onAccepted: activeEditor.text = controller.encodeCaesar(activeEditor.text, caesarShiftEncode.value)
    }

    Dialog {
        id: caesarDialogDecode
        modal: true
        title: "Caesar Cipher - Decode"
        standardButtons: Dialog.Ok | Dialog.Cancel
        Column {
            spacing: 10
            Label { text: "Shift amount:" }
            SpinBox { id: caesarShiftDecode; from: -25; to: 25; value: 3 }
        }
        onAccepted: activeEditor.text = controller.decodeCaesar(activeEditor.text, caesarShiftDecode.value)
    }

    // ...rest of tabView definition
}
