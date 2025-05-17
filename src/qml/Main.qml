// Includes relevant modules used by the QML
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import io.github.denysmb.klaro 1.0

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
    
    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Change language")
                icon.name: "languages"
                onTriggered: languageDialog.open()
            },
            Kirigami.Action {
                text: i18n("Translate")
                icon.name: "translate"
                onTriggered: {
                    // Translation logic goes here
                }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Use English language names")
                icon.name: "preferences-desktop-locale"
                checkable: true
                checked: TranslationManager.useEnglishNames
                onTriggered: TranslationManager.useEnglishNames = checked
            }
        ]
    }

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
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Kirigami.FormData.label: i18n("Input Language:")
                model: TranslationManager.availableLanguages
                currentIndex: 0  // Set to "auto" by default
                popup.height: Math.min(popup.contentItem.implicitHeight, root.height / 2)
            }
            
            // Output language selection
            Controls.ComboBox {
                id: outputLanguageComboBox
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Kirigami.FormData.label: i18n("Output Language:")
                model: TranslationManager.availableLanguages.filter(function(lang) { return lang !== "auto" })
                currentIndex: 0
                popup.height: Math.min(popup.contentItem.implicitHeight, root.height / 2)
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
