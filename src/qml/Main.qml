// Includes relevant modules used by the QML
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    // Unique identifier to reference this object
    id: root

    width: 600
    height: 500

    property string selectedInputLanguage: i18n("Auto detect")
    property string selectedOutputLanguage: "English"

    // Window title
    // i18nc() makes a string translatable
    // and provides additional context for the translators
    title: i18nc("@title:window", "Klaro")
    
    // Language dialog component
    Kirigami.Dialog {
        id: languageDialog
        title: i18n("Select Languages")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 20
        
        onAccepted: {
            root.selectedInputLanguage = inputLanguageComboBox.currentText
            root.selectedOutputLanguage = outputLanguageComboBox.currentText
            // Here you would update your app's language settings
        }
        
        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Input language selection
            Controls.ComboBox {
                id: inputLanguageComboBox
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Input Language:")
                model: [i18n("Auto detect"), "English", "Spanish", "French", "German", "Chinese", "Japanese"]
            }
            
            // Output language selection
            Controls.ComboBox {
                id: outputLanguageComboBox
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Output Language:")
                model: ["English", "Spanish", "French", "German", "Chinese", "Japanese"]
            }
        }
    }

    // Set the first page that will be loaded when the app opens
    // This can also be set to an id of a Kirigami.Page
    pageStack.initialPage: Kirigami.Page {
        // Add actions to the page header
        actions: [
            Kirigami.Action {
                text: i18n("Change language")
                icon.name: "languages"
                onTriggered: {
                    // Open the language selection dialog
                    languageDialog.open()
                }
            },
            Kirigami.Action {
                text: i18n("Translate")
                icon.name: "translate"
                onTriggered: {
                    // Translation logic goes here
                }
            }
        ]

        // Replace the single label with a Column layout
        Column {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing

            // Output area - displays translated text (not editable)
            // Now using Kirigami.AbstractCard instead of TextArea
            Kirigami.AbstractCard {
                id: translatedCard
                width: parent.width
                height: parent.height / 2 - Kirigami.Units.largeSpacing
                
                contentItem: Controls.Label {
                    id: translatedTextLabel
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.smallSpacing
                    text: ""
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignTop
                    opacity: text === "" ? 0.6 : 1.0
                    
                    // Show placeholder text when empty
                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: translatedTextLabel.text === ""
                        text: i18n("Translated text will appear here")
                    }
                }
            }

            // Input area - for user to enter text to translate
            Kirigami.AbstractCard {
                id: inputCard
                width: parent.width
                height: parent.height / 2 - Kirigami.Units.largeSpacing
                
                contentItem: Controls.TextArea {
                    id: inputTextArea
                    width: parent.width
                    height: parent.height / 2 - Kirigami.Units.largeSpacing
                    placeholderText: i18n("Enter text to translate")
                    wrapMode: TextEdit.Wrap
                    background: null
                    opacity: text === "" ? 0.6 : 1.0

                    // Show placeholder text when empty
                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: inputTextArea.text === ""
                        text: i18n("Enter text to translate")
                    }
                }
            }
        }

        // Header displaying selected languages
        title: root.selectedInputLanguage + " -> " + root.selectedOutputLanguage
    }
}
