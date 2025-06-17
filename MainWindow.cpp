#include "MainWindow.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSettings>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMenuBar>
#include <QMenu>
#include <QByteArray>
#include <QTextEdit>
#include <QDateTime>
#include <QTextCursor>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QClipboard>
#include <QGuiApplication>

//Includes for the codecs
#include "encoders/Base64Codec.h"
#include "encoders/Rot13.h"
#include "encoders/CaesarCipher.h"
#include "encoders/BinaryCodec.h"
#include "encoders/HexCodec.h"
#include "encoders/PigLatin.h"
#include "encoders/Atbash.h"
#include "encoders/MorseCodec.h"
#include "encoders/AESCodec.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();
    connectSignals();
    loadRecentFiles();
    restoreSession();
}

void MainWindow::setupUI() {
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QTextEdit *editor = qobject_cast<QTextEdit*>(tabWidget->widget(index));
        closeTab(editor);
    });

    cursorLabel = new QLabel("Ln 1, Col 1");
    statusBar()->addPermanentWidget(cursorLabel);

    // Main toolbar
    mainToolbar = addToolBar("Main Toolbar");
    mainToolbar->setObjectName("mainToolbar");

    QAction *newAction = new QAction(QIcon::fromTheme("document-new"), "New", this);
    connect(newAction, &QAction::triggered, this, &MainWindow::newTab);
    newAction->setShortcut(QKeySequence::New);

    QAction *openAction = new QAction(QIcon::fromTheme("document-open"), "Open", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    openAction->setShortcut(QKeySequence::Open);

    QAction *saveAction = new QAction(QIcon::fromTheme("document-save"), "Save", this);
    connect(saveAction, &QAction::triggered, [this]() {
        if (QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget())) {
            saveFile(editor);
        }
    });
    saveAction->setShortcut(QKeySequence::Save);

    QAction *saveAllAction = new QAction(QIcon::fromTheme("document-save-all"), "Save All", this);
    connect(saveAllAction, &QAction::triggered, this, &MainWindow::saveAll);
    saveAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    //QAction *closeAction = new QAction(QIcon::fromTheme("window-close"), "Close Document", this);
    //connect(closeAction, &QAction::triggered, this, &MainWindow::closeTab);
    //saveAllAction->setShortcut(QKeySequence(QKeySequence::Close));

    QAction *undoAction = new QAction(QIcon::fromTheme("edit-undo"), "Undo", this);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undoLastChange);
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = new QAction(QIcon::fromTheme("edit-redo"), "Redo", this);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redoLastChange);
    redoAction->setShortcut(QKeySequence::Redo);

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Copy", this);
    connect(copyAction, &QAction::triggered, [this]() {
        QTextEdit *editor = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
        if (editor && tabDataMap.contains(editor)) {
            QGuiApplication::clipboard()->setText(editor->textCursor().selectedText());
        }
    });
    copyAction->setShortcut(QKeySequence::Copy);

    QAction *cutAction = new QAction(QIcon::fromTheme("edit-cut"), "Cut", this);
    connect(cutAction, &QAction::triggered, [this]() {
        QTextEdit *editor = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
        if (editor && tabDataMap.contains(editor)) {
            QTextCursor cursor = editor->textCursor();
            QGuiApplication::clipboard()->setText(cursor.selectedText());
            cursor.removeSelectedText();
            editor->setTextCursor(cursor);
        }
    });
    cutAction->setShortcut(QKeySequence::Cut);

    QAction *pasteAction = new QAction(QIcon::fromTheme("edit-paste"), "Paste", this);
    connect(pasteAction, &QAction::triggered, [this]() {
        QTextEdit *editor = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
        if (editor && tabDataMap.contains(editor)) {
            editor->insertPlainText(QGuiApplication::clipboard()->text());
        }
    });
    pasteAction->setShortcut(QKeySequence::Paste);

    QAction *settingsAction = new QAction(QIcon::fromTheme("preferences-system"), "Settings", this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));

    mainToolbar->addAction(newAction);
    mainToolbar->addAction(openAction);
    mainToolbar->addAction(saveAction);
    mainToolbar->addAction(saveAllAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(copyAction);
    mainToolbar->addAction(cutAction);
    mainToolbar->addAction(pasteAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(undoAction);
    mainToolbar->addAction(redoAction);
    mainToolbar->addSeparator();
    mainToolbar->addAction(settingsAction);

    // Codec toolbar
    codecToolbar = addToolBar("Codec Toolbar");
    codecToolbar->setObjectName("codecToolbar");
    encoderSelector = new QComboBox;
    QPushButton *encodeButton = new QPushButton("Encode");
    QPushButton *decodeButton = new QPushButton("Decode");
    encoderSelector->addItems({"Base64", "Binary", "Hex", "Caesar", "ROT13", "Morse", "Atbash", "Pig Latin", "AES"});
    codecToolbar->addWidget(encoderSelector);
    codecToolbar->addWidget(encodeButton);
    codecToolbar->addWidget(decodeButton);

    connect(encodeButton, &QPushButton::clicked, this, [this]() {
        encodeCurrentText(encoderSelector->currentText());
    });
    connect(decodeButton, &QPushButton::clicked, this, [this]() {
        decodeCurrentText(encoderSelector->currentText());
    });

    // Menus
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);

    QMenu *recentMenu = new QMenu("Open Recent", this);
    for (int i = 0; i < 10; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        recentMenu->addAction(recentFileActions[i]);
    }
    fileMenu->addMenu(recentMenu);

    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAllAction);
    fileMenu->addSeparator();
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);
    quitAction->setShortcut(QKeySequence::Quit);

    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(copyAction);
    editMenu->addAction(cutAction);
    editMenu->addAction(pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(settingsAction);

    QMenu *viewMenu = menuBar()->addMenu("View");

    toggleMainToolbar = new QAction("Show Main Toolbar", this);
    toggleMainToolbar->setCheckable(true);
    toggleMainToolbar->setChecked(true);
    viewMenu->addAction(toggleMainToolbar);
    connect(toggleMainToolbar, &QAction::toggled, mainToolbar, &QToolBar::setVisible);

    toggleCodecToolbar = new QAction("Show Codec Toolbar", this);
    toggleCodecToolbar->setCheckable(true);
    toggleCodecToolbar->setChecked(true);
    viewMenu->addAction(toggleCodecToolbar);
    connect(toggleCodecToolbar, &QAction::toggled, codecToolbar, &QToolBar::setVisible);

    toggleStatusBar = new QAction("Show Status Bar", this);
    toggleStatusBar->setCheckable(true);
    toggleStatusBar->setChecked(true);
    viewMenu->addAction(toggleStatusBar);
    connect(toggleStatusBar, &QAction::toggled, this, [this](bool visible){
        statusBar()->setVisible(visible);
    });

    QMenu *codecMenu = menuBar()->addMenu("Codecs");
    QMenu *encodeMenu = codecMenu->addMenu("Encode");
    QMenu *decodeMenu = codecMenu->addMenu("Decode");

    for (int i = 0; i < encoderSelector->count(); ++i) {
        QString name = encoderSelector->itemText(i);

        QAction *encodeAction = new QAction(name, this);
        connect(encodeAction, &QAction::triggered, this, [this, name]() {
            QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
            if (editor && tabDataMap.contains(editor)) {
                encodeCurrentText(name);
            }
        });
        encodeMenu->addAction(encodeAction);

        QAction *decodeAction = new QAction(name, this);
        connect(decodeAction, &QAction::triggered, this, [this, name]() {
            QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
            if (editor && tabDataMap.contains(editor)) {
                decodeCurrentText(name);
            }
        });
        decodeMenu->addAction(decodeAction);
    }

    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction(QIcon::fromTheme("help-about"), "About", this);
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    aboutAction->setShortcut(QKeySequence::HelpContents);

    applyEditorSettings();
}

void MainWindow::connectSignals() {
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::updateCursorStatus);
}

void MainWindow::updateCursorStatus() {
    QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!editor || !tabDataMap.contains(editor)) return;
    editor->blockSignals(true);
    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    cursorLabel->setText(QString("Ln %1, Col %2").arg(line).arg(col));
    editor->blockSignals(false);
}

void MainWindow::newTab() {
    QTextEdit *editor = new QTextEdit(this);
    tabWidget->addTab(editor, "Untitled");
    tabWidget->setCurrentWidget(editor);

    TabData data;
    data.editor = editor;
    data.isModified = false;
    tabDataMap[editor] = data;

    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateCursorStatus);
    connect(editor, &QTextEdit::textChanged, this, [this, editor]() {
        markModified(editor, true);
    });
}

void MainWindow::markModified(QTextEdit *editor, bool modified) {
    if (!editor || !tabDataMap.contains(editor)) return;
    TabData &data = tabDataMap[editor];

    // Only act if modified state is changing
    if (data.isModified == modified)
        return;

    data.isModified = modified;

    int index = tabWidget->indexOf(editor);
    if (index == -1) return;

    QString title;
    if (!data.filePath.isEmpty()) {
        title = QFileInfo(data.filePath).fileName();
    } else {
        title = "Untitled";
    }

    // Remove '*' if already present
    if (title.endsWith('*'))
        title.chop(1);

    // Add '*' only if modified
    if (modified)
        title += '*';

    // Only set the text if it's different
    if (tabWidget->tabText(index) != title) {
        tabWidget->setTabText(index, title);
    }
}


void MainWindow::encodeCurrentText(const QString &method) {
    QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!editor) return;

    QString text = editor->toPlainText();
    QString result;

    if (method == "Base64") {
        result = Base64Codec::transform(text, false);
    } else if (method == "ROT13") {
        result = Rot13::transform(text);
    } else if (method == "Caesar") {
        bool ok;
        int shift = QInputDialog::getInt(this, "Caesar Shift", "Shift: ", 3, -25, 25, 1, &ok);
        if (ok) result = CaesarCipher::transform(text, shift, false);
        else return;
    } else if (method == "Binary") {
        result = BinaryCodec::transform(text, false);
    } else if (method == "Hex") {
        result = HexCodec::transform(text, false);
    } else if (method == "PigLatin") {
        result = PigLatin::transform(text, false);
    } else if (method == "Atbash") {
        result = Atbash::transform(text);
    } else if (method == "Morse") {
        result = MorseCodec::transform(text, false);
    } else if (method == "AES") {
        bool ok;
        QString key = QInputDialog::getText(this, "AES Key", "Enter encryption key:", QLineEdit::Password, "", &ok);
        if (!ok || key.isEmpty()) return;
        result = AESCodec::encode(text, key);
    } else {
        QMessageBox::warning(this, "Unknown Codec", "The selected decoding method is not supported.");
        return;
    }

    editor->setPlainText(result);
    markModified(editor, true);
}

void MainWindow::decodeCurrentText(const QString &method) {
    QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!editor) return;

    QString text = editor->toPlainText();
    QString result;

    if (method == "Base64") {
        result = Base64Codec::transform(text, true);
    } else if (method == "ROT13") {
        result = Rot13::transform(text);
    } else if (method == "Caesar") {
        bool ok;
        int shift = QInputDialog::getInt(this, "Caesar Shift", "Shift: ", 3, -25, 25, 1, &ok);
        if (ok) result = CaesarCipher::transform(text, shift, true);
        else return;
    } else if (method == "Binary") {
        result = BinaryCodec::transform(text, true);
    } else if (method == "Hex") {
        result = HexCodec::transform(text, true);
    } else if (method == "PigLatin") {
        result = PigLatin::transform(text, true);
    } else if (method == "Atbash") {
        result = Atbash::transform(text);
    } else if (method == "Morse") {
        result = MorseCodec::transform(text, true);
    } else if (method == "AES") {
        bool ok;
        QString key = QInputDialog::getText(this, "AES Key", "Enter decryption key:", QLineEdit::Password, "", &ok);
        if (!ok || key.isEmpty()) return;
        result = AESCodec::decode(text, key);
    } else {
        QMessageBox::warning(this, "Unknown Codec", "The selected decoding method is not supported.");
        return;
    }

    editor->setPlainText(result);
    markModified(editor, true);
}

void MainWindow::openFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open File");
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void MainWindow::loadFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }

    QString content = QTextStream(&file).readAll();
    file.close();

    QTextEdit *editor = new QTextEdit(this);
    editor->blockSignals(true);
    editor->setPlainText(content);
    editor->blockSignals(false);

    int index = tabWidget->addTab(editor, QFileInfo(filePath).fileName());
    tabWidget->setCurrentIndex(index);

    TabData data;
    data.editor = editor;
    data.filePath = filePath;
    data.isModified = false;
    tabDataMap[editor] = data;

    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateCursorStatus);
    connect(editor, &QTextEdit::textChanged, this, [this, editor]() {
        markModified(editor, true);
    });

    // Apply font and tab settings
    editor->blockSignals(true);
    editor->setFont(currentFont);
    QFontMetrics metrics(currentFont);
    editor->setTabStopDistance(currentTabSize * metrics.horizontalAdvance(' '));
    editor->blockSignals(false);

    markModified(editor, false);

    // Update recent files list
    recentFiles.removeAll(filePath);        // Remove any previous entry
    recentFiles.prepend(filePath);          // Add to the top
    while (recentFiles.size() > 10) {       // Limit list size
        recentFiles.removeLast();
    }
    saveRecentFiles();                      // Persist to QSettings
    updateRecentFilesMenu();                // Refresh UI menu
}

void MainWindow::saveFileAs(QTextEdit *editor) {
    if (!editor || !tabDataMap.contains(editor)) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Save File As");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not write to file.");
        return;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();

    tabDataMap[editor].filePath = filePath;
    tabDataMap[editor].isModified = false;

    int index = indexForEditor(editor);
    if (index >= 0) {
        QFileInfo info(filePath);
        tabWidget->setTabText(index, info.fileName());
    }

    markModified(editor, false);
}

void MainWindow::saveFile(QTextEdit *editor) {
    if (!editor || !tabDataMap.contains(editor)) return;
    TabData &data = tabDataMap[editor];
    QString path = data.filePath;

    if (path.isEmpty()) {
        saveFileAs(editor);
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not write to file.");
        return;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();

    data.isModified = false;
    markModified(editor, false);
}

void MainWindow::saveAll() {
    for (auto it = tabDataMap.begin(); it != tabDataMap.end(); ++it) {
        QTextEdit *editor = it.key();
        TabData &data = it.value();

        if (!data.filePath.isEmpty()) {
            saveFile(editor);  // Saves to existing file path
        } else {
            tabWidget->setCurrentWidget(editor);  // Brings the tab to front
            saveFileAs(editor);                   // Prompt for location
        }
    }
}

void MainWindow::openRecentFile() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString filePath = action->data().toString();
        loadFile(filePath);
    }
}

void MainWindow::updateRecentFilesMenu() {
    for (int i = 0; i < 10; ++i) {
        if (i < recentFiles.size()) {
            QString text = QFileInfo(recentFiles[i]).fileName();
            recentFileActions[i]->setText(text);
            recentFileActions[i]->setData(recentFiles[i]);
            recentFileActions[i]->setVisible(true);
        } else {
            recentFileActions[i]->setVisible(false);
        }
    }
}

void MainWindow::saveRecentFiles() {
    QSettings settings("Hellmark Programming Group", "Decode");
    settings.setValue("recentFiles", recentFiles);
}

void MainWindow::loadRecentFiles() {
    QSettings settings("Hellmark Programming Group", "Decode");
    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFilesMenu();
}

// Session saving!
void MainWindow::saveSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    // Saving the current window state
    settings.setValue("window/maximized", isMaximized());
    settings.setValue("window/normalSize", normalGeometry().size());
    settings.setValue("window/normalPos", normalGeometry().topLeft());
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/windowState", saveState());

    if (settings.value("window/maximized", false).toBool()) {
        showMaximized();
    } else {
        resize(settings.value("window/size", QSize(800, 600)).toSize());
        move(settings.value("window/pos", QPoint(200, 200)).toPoint());
    }
    // UI element visibility
    settings.setValue("ui/codecToolbarVisible", codecToolbar->isVisible());
    settings.setValue("ui/mainToolbarVisible", mainToolbar->isVisible());
    settings.setValue("ui/statusBarVisible", statusBar()->isVisible());
    settings.setValue("restorePreviousSession", restorePreviousSession);

    // Tab saving
    settings.beginGroup("session");

    QStringList paths;
    QStringList contents;
    QList<bool> modifiedFlags;

    for (QTextEdit *editor : tabDataMap.keys()) {
        const TabData &data = tabDataMap[editor];
        paths.append(data.filePath);
        contents.append(editor->toPlainText());
        modifiedFlags.append(data.isModified);
    }

    settings.setValue("filePaths", paths);
    settings.setValue("unsavedContents", contents);
    settings.setValue("modifiedFlags", QVariant::fromValue(modifiedFlags));

    settings.endGroup();
    settings.sync();
}

// Session restoration!
void MainWindow::restoreSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    int count = settings.value("count").toInt();

    // Checks if session restoration is wanted, and loads the files if so
    restorePreviousSession = settings.value("restorePreviousSession", true).toBool();
    if (restorePreviousSession) {
        // Loads the info from the config file
        settings.beginGroup("session");
        QStringList paths = settings.value("filePaths").toStringList();
        QStringList contents = settings.value("unsavedContents").toStringList();
        QList<QVariant> modifiedRaw = settings.value("modifiedFlags").toList();
        settings.endGroup();

        // Loops through to load each of the tabs
        for (int i = 0; i < paths.size(); ++i) {
            QString path = i < paths.size() ? paths[i] : "";
            QString content = i < contents.size() ? contents[i] : "";
            bool modified = (i < modifiedRaw.size()) ? modifiedRaw[i].toBool() : false;

            newTab();
            QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
            if (!editor) continue;

            TabData &data = tabDataMap[editor];

            if (!path.isEmpty()) {
                data.filePath = path;
            }

            editor->blockSignals(true);
            editor->setPlainText(content);
            editor->blockSignals(false);

            markModified(editor, modified);

            if (!modified && !path.isEmpty()) {
                // Load from disk if unchanged
                loadFile(path);
            }
        }
    }

    // Saving Window position info, with logic to make sure that things don't get messed up if maximized.
    if (settings.value("window/maximized", false).toBool()) {
        resize(settings.value("window/normalSize", QSize(800, 600)).toSize());
        move(settings.value("window/normalPos", QPoint(200, 200)).toPoint());
        showMaximized();
    } else {
        resize(settings.value("window/normalSize", QSize(800, 600)).toSize());
        move(settings.value("window/normalPos", QPoint(200, 200)).toPoint());
    }

    // Remember if the toolbars and statusbar is set to be visible or not.
    codecToolbar->setVisible(settings.value("ui/codecToolbarVisible", true).toBool());
    mainToolbar->setVisible(settings.value("ui/mainToolbarVisible", true).toBool());
    statusBar()->setVisible(settings.value("ui/statusBarVisible", true).toBool());

    // remembers the position of the toolbars
    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/windowState").toByteArray());

    // make sure that there is at least one tab
    if (tabWidget->count() == 0) {
        newTab();
    }
}

void MainWindow::maybeSaveAndClose(QTextEdit *editor) {
    if (!editor || !tabDataMap.contains(editor)) return;

    if (tabDataMap[editor].isModified) {
        auto response = QMessageBox::question(this, "Unsaved Changes",
                                              "Do you want to save changes before closing?",
                                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (response == QMessageBox::Yes) {
            saveFile(editor);
        } else if (response == QMessageBox::Cancel) {
            return;
        }
    }

    int index = indexForEditor(editor);
    tabDataMap.remove(editor);
    tabWidget->removeTab(index);
    editor->deleteLater();

    if (tabWidget->count() == 0)
        newTab();
}

void MainWindow::closeTab(QTextEdit *editor) {
    // Kick back if no pointer is passed in
    if (!editor) return;
    // Trigger the function for closing and check with user if should save
    maybeSaveAndClose(editor);
    //open a new tab if no other tabs exist
    if (tabWidget->count() == 0) {
        newTab();
    }
}

void MainWindow::undoLastChange() {
    QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!editor || !tabDataMap.contains(editor))
        return;

    auto &data = tabDataMap[editor];
    if (!data.undoStack.isEmpty()) {
        data.redoStack.push(editor->toPlainText());
        QString last = data.undoStack.pop();
        editor->blockSignals(true);
        editor->setPlainText(last);
        editor->blockSignals(false);
        markModified(editor, true);
    }
}

void MainWindow::redoLastChange() {
    QTextEdit *editor = qobject_cast<QTextEdit *>(tabWidget->currentWidget());
    if (!editor || !tabDataMap.contains(editor))
        return;

    auto &data = tabDataMap[editor];
    if (!data.redoStack.isEmpty()) {
        data.undoStack.push(editor->toPlainText());
        QString next = data.redoStack.pop();
        editor->blockSignals(true);
        editor->setPlainText(next);
        editor->blockSignals(false);
        markModified(editor, true);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveSession();
    QSettings settings("Hellmark Programming Group", "Decode");
    QMainWindow::closeEvent(event);
}

void MainWindow::openSettingsDialog() {
    SettingsDialog dialog(this);
    dialog.setFont(currentFont);
    dialog.setTabSize(currentTabSize);
    dialog.setRestoreSession(restorePreviousSession);

    connect(&dialog, &SettingsDialog::clearRecentFiles, this, &MainWindow::clearRecentFiles);
    connect(&dialog, &SettingsDialog::resetUILayout, this, &MainWindow::resetUILayout);

    if (dialog.exec() == QDialog::Accepted) {
        QSettings settings("Hellmark Programming Group", "Decode");
        settings.setValue("editor/font", dialog.getFont());
        settings.setValue("editor/tabSize", dialog.getTabSize());
        //settings.setValue("editor/restoreSession", dialog.shouldRestoreSession());
        currentFont = dialog.getFont();
        currentTabSize = dialog.getTabSize();
        restorePreviousSession = dialog.shouldRestoreSession();
        applyEditorSettings();
    }
}

void MainWindow::applyEditorSettings() {
    QSettings settings("Hellmark Programming Group", "Decode");
    currentFont = settings.value("editor/font", QFont("Monospace", 10)).value<QFont>();
    currentTabSize = settings.value("editor/tabSize", 4).toInt();
    restorePreviousSession = settings.value("editor/restoreSession", true).toBool();

    for (auto &tab : tabDataMap) {
        if (tab.editor) {
            tab.editor->setFont(currentFont);
            QFontMetrics metrics(currentFont);
            tab.editor->setTabStopDistance(currentTabSize * metrics.horizontalAdvance(' '));
        }
    }
}

void MainWindow::clearRecentFiles() {
    QSettings settings("Hellmark Programming Group", "Decode");
    recentFiles.clear();
    updateRecentFilesMenu();
    settings.remove("recentFiles");
}

void MainWindow::resetUILayout() {

    QSettings settings("Hellmark Programming Group", "Decode");
    settings.remove("geometry");
    settings.remove("windowState");
    settings.remove("mainToolbarVisible");
    settings.remove("codecToolbarVisible");
    settings.remove("statusBarVisible");
    settings.remove("maximized");
    settings.remove("editor/font");
    settings.remove("editor/tabSize");
    addToolBar(Qt::TopToolBarArea, mainToolbar);
    addToolBar(Qt::TopToolBarArea, codecToolbar);
    mainToolbar->show();
    codecToolbar->show();
    statusBar()->show();
    restoreState(QByteArray());
    resize(800, 600);

    update();

    currentFont = QFont("Monospace", 10);
    currentTabSize = 4;
    applyEditorSettings();
}

void MainWindow::setTabSize(int size) {
    currentTabSize = size;
    applyEditorSettings();
}

int MainWindow::getTabSize() const {
    return currentTabSize;
}

void MainWindow::setRestoreSession(bool value) {
    restorePreviousSession = value;
}

bool MainWindow::shouldRestoreSession() const {
    return restorePreviousSession;
}

QFont MainWindow::getFont() const {
    return currentFont;
}

void MainWindow::setFont(const QFont &font) {
    currentFont = font;
    applyEditorSettings();
}

void MainWindow::clearSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    settings.beginGroup("session");
    settings.remove("");  // Remove everything
    settings.endGroup();
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "About Decode",
        "Decode v3.0\n"
        "© 2025 Hellmark Programming Group\n\n"
        "A simple cipher utility that helps make things more secure!");
}

int MainWindow::indexForEditor(QTextEdit *editor) const {
    return tabWidget->indexOf(editor);
}
