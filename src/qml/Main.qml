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

    width: 800
    height: 500

    property string selectedInputLanguage: TranslationManager.inputLanguage
    property string selectedOutputLanguage: TranslationManager.outputLanguage
    property var translationSegments: []

    function countWords(text) {
        return text.trim() === "" ? 0 : text.trim().split(/\s+/).length
    }

    Component.onCompleted: {
        selectedInputLanguage = TranslationManager.inputLanguage
        selectedOutputLanguage = TranslationManager.outputLanguage
        inputTextArea.forceActiveFocus()
    }

    onSelectedInputLanguageChanged: TranslationManager.inputLanguage = selectedInputLanguage
    onSelectedOutputLanguageChanged: {
        TranslationManager.outputLanguage = selectedOutputLanguage
        // Only translate if we have text and the output language changed
        if (inputTextArea.text.trim() !== "") {
            translateText()
        }
    }

    function translateText() {
        if (inputTextArea.text.trim() === "") {
            return;
        }
        
        TranslationManager.translateDetailed(
            inputTextArea.text,
            root.selectedInputLanguage,
            root.selectedOutputLanguage
        );
    }

    Connections {
        target: TranslationManager
        function onTranslationFinished(result) {
            if (result.translation) {
                translatedTextLabel.text = result.translation;
                root.translationSegments = result.segments;
            }
        }
    }

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
                onTriggered: translateText()
            },
            Kirigami.Action {
                text: i18n("Switch languages")
                icon.name: "exchange-positions"
                enabled: selectedInputLanguage !== i18n("Auto detect")
                onTriggered: {
                    // Don't switch if input is "Auto detect"
                    if (selectedInputLanguage === i18n("Auto detect")) {
                        return;
                    }
                    
                    // Store current values
                    let tempInput = selectedInputLanguage;
                    let tempOutput = selectedOutputLanguage;
                    let tempInputText = inputTextArea.text;
                    let tempOutputText = translatedTextLabel.text;
                    
                    // Switch languages
                    selectedInputLanguage = tempOutput;
                    selectedOutputLanguage = tempInput;
                    
                    // Switch texts
                    inputTextArea.text = tempOutputText;
                    translatedTextLabel.text = tempInputText;
                    root.translationSegments = [];
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
            },
            Kirigami.Action {
                text: i18n("Translation engine")
                icon.name: "applications-internet"

                Kirigami.Action {
                    text: i18n("Auto")
                    checkable: true
                    checked: TranslationManager.translationEngine === "auto"
                    Controls.ActionGroup.group: engineActionGroup
                    onTriggered: TranslationManager.translationEngine = "auto"
                }
                Kirigami.Action {
                    text: i18n("Google")
                    checkable: true
                    checked: TranslationManager.translationEngine === "google"
                    Controls.ActionGroup.group: engineActionGroup
                    onTriggered: TranslationManager.translationEngine = "google"
                }
                Kirigami.Action {
                    text: i18n("Bing")
                    checkable: true
                    checked: TranslationManager.translationEngine === "bing"
                    Controls.ActionGroup.group: engineActionGroup
                    onTriggered: TranslationManager.translationEngine = "bing"
                }
                Kirigami.Action {
                    text: i18n("Yandex")
                    checkable: true
                    checked: TranslationManager.translationEngine === "yandex"
                    Controls.ActionGroup.group: engineActionGroup
                    onTriggered: TranslationManager.translationEngine = "yandex"
                }
            }
        ]
    }

    Controls.ActionGroup {
        id: engineActionGroup
    }

    // Language dialog component
    Kirigami.Dialog {
        id: languageDialog
        title: i18n("Select Languages")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 20
        
        onOpened: {
            // Set initial values when dialog opens
            for (let i = 0; i < inputLanguageComboBox.model.length; i++) {
                if (inputLanguageComboBox.model[i] === root.selectedInputLanguage) {
                    inputLanguageComboBox.currentIndex = i;
                    break;
                }
            }
            for (let i = 0; i < outputLanguageComboBox.model.length; i++) {
                if (outputLanguageComboBox.model[i] === root.selectedOutputLanguage) {
                    outputLanguageComboBox.currentIndex = i;
                    break;
                }
            }
        }
        
        onAccepted: {
            root.selectedInputLanguage = inputLanguageComboBox.currentText
            root.selectedOutputLanguage = outputLanguageComboBox.currentText
        }

        onRejected: {
            // Reset to the last accepted values
            inputLanguageComboBox.currentIndex = inputLanguageComboBox.model.indexOf(root.selectedInputLanguage)
            outputLanguageComboBox.currentIndex = outputLanguageComboBox.model.indexOf(root.selectedOutputLanguage)
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
                popup.height: Math.min(popup.contentItem.implicitHeight, root.height / 2)
            }
            
            // Output language selection
            Controls.ComboBox {
                id: outputLanguageComboBox
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Kirigami.FormData.label: i18n("Output Language:")
                model: TranslationManager.availableLanguages.filter(function(lang) { return lang !== i18n("Auto detect") })
                popup.height: Math.min(popup.contentItem.implicitHeight, root.height / 2)
            }
        }
    }

    // Alternatives dialog component
    Kirigami.Dialog {
        id: alternativesDialog
        title: i18n("Alternatives")
        standardButtons: Kirigami.Dialog.Close
        preferredWidth: Kirigami.Units.gridUnit * 25
        padding: Kirigami.Units.largeSpacing

        ListView {
            id: alternativesList
            implicitHeight: Math.min(contentHeight, Kirigami.Units.gridUnit * 15)
            clip: true
            model: root.translationSegments
            spacing: Kirigami.Units.smallSpacing

            delegate: Column {
                width: alternativesList.width

                Controls.Label {
                    width: parent.width
                    text: modelData.segment
                    wrapMode: Text.WordWrap
                    font.bold: true
                }
                Controls.Label {
                    width: parent.width
                    leftPadding: Kirigami.Units.largeSpacing
                    text: modelData.alternatives.join(", ")
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                }
            }
        }
    }

    // Set the first page that will be loaded when the app opens
    // This can also be set to an id of a Kirigami.Page
    pageStack.initialPage: Kirigami.Page {
        // Add actions to the page header
        actions: [
            Kirigami.Action {
                text: i18n("Switch languages")
                icon.name: "exchange-positions"
            enabled: selectedInputLanguage !== i18n("Auto detect")
            onTriggered: {
                // Don't switch if input is "Auto detect"
                if (selectedInputLanguage === i18n("Auto detect")) {
                    return;
                }
                
                // Store current values
                let tempInput = selectedInputLanguage;
                let tempOutput = selectedOutputLanguage;
                let tempInputText = inputTextArea.text;
                let tempOutputText = translatedTextLabel.text;
                
                // Switch languages
                selectedInputLanguage = tempOutput;
                selectedOutputLanguage = tempInput;
                
                // Switch texts
                inputTextArea.text = tempOutputText;
                translatedTextLabel.text = tempInputText;
                root.translationSegments = [];
            }
        },
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
                onTriggered: translateText()
            }
        ]

        // Replace the single label with a Column layout
        Column {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing

            // Output area - displays translated text (not editable)
            Kirigami.AbstractCard {
                id: translatedCard
                width: parent.width
                height: parent.height / 2 - Kirigami.Units.largeSpacing
                
                contentItem: Controls.Label {
                    id: translatedTextLabel
                    width: parent.width
                    height: parent.height
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

            Controls.ToolButton {
                anchors {
                    top: translatedCard.top
                    right: translatedCard.right
                    margins: Kirigami.Units.smallSpacing
                }
                icon.name: "edit-copy"
                text: i18n("Copy")
                display: Controls.ToolButton.IconOnly
                visible: translatedTextLabel.text !== ""
                onClicked: {
                    TranslationManager.copyToClipboard(translatedTextLabel.text)
                }
            }

            Controls.BusyIndicator {
                anchors.centerIn: translatedCard
                implicitWidth: Kirigami.Units.gridUnit * 3
                implicitHeight: Kirigami.Units.gridUnit * 3
                visible: TranslationManager.busy
                running: visible
            }

            Controls.ToolButton {
                anchors {
                    left: translatedCard.left
                    bottom: translatedCard.bottom
                    margins: Kirigami.Units.smallSpacing
                }
                icon.name: "view-list-details"
                text: i18n("Alternatives")
                display: Controls.ToolButton.IconOnly
                visible: root.translationSegments.length > 0
                onClicked: alternativesDialog.open()
            }

            Controls.Label {
                anchors {
                    bottom: translatedCard.bottom
                    right: translatedCard.right
                    margins: Kirigami.Units.largeSpacing
                }
                visible: translatedTextLabel.text !== ""
                text: translatedTextLabel.text.length + " chars · " + root.countWords(translatedTextLabel.text) + " words"
                opacity: 0.6
                font: Kirigami.Theme.smallFont
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
                    wrapMode: TextEdit.Wrap
                    background: null
                    opacity: text === "" ? 0.6 : 1.0

                    Keys.onReturnPressed: {
                        if (event.modifiers === Qt.ShiftModifier) {
                            // Insert a new line when Shift+Enter is pressed
                            inputTextArea.insert(inputTextArea.cursorPosition, "\n")
                            event.accepted = true
                        } else if (event.modifiers === Qt.NoModifier) {
                            // Translate when Enter is pressed without modifiers
                            translateText()
                            event.accepted = true
                        }
                    }

                    // Show placeholder text when empty
                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: parent.width - (Kirigami.Units.largeSpacing * 4)
                        visible: inputTextArea.text === ""
                        text: i18n("Enter text to translate")
                    }
                }
            }

            Controls.ToolButton {
                anchors {
                    top: inputCard.top
                    right: inputCard.right
                    margins: Kirigami.Units.smallSpacing
                }
                icon.name: "edit-clear"
                text: i18n("Clear")
                display: Controls.ToolButton.IconOnly
                visible: inputTextArea.text !== ""
                onClicked: {
                    inputTextArea.text = ""
                }
            }

            Controls.Label {
                anchors {
                    bottom: inputCard.bottom
                    right: inputCard.right
                    margins: Kirigami.Units.largeSpacing
                }
                visible: inputTextArea.text !== ""
                text: inputTextArea.text.length + " chars · " + root.countWords(inputTextArea.text) + " words"
                opacity: 0.6
                font: Kirigami.Theme.smallFont
            }
        }

        // Header displaying selected languages
        title: root.selectedInputLanguage + " -> " + root.selectedOutputLanguage
    }
}
