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
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

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
        int index = tabWidget->currentIndex();
        saveFile(index);
    });
    saveAction->setShortcut(QKeySequence::Save);

    QAction *saveAllAction = new QAction(QIcon::fromTheme("document-save-all"), "Save All", this);
    connect(saveAllAction, &QAction::triggered, this, &MainWindow::saveAll);
    saveAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    QAction *undoAction = new QAction(QIcon::fromTheme("edit-undo"), "Undo", this);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undoLastChange);
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = new QAction(QIcon::fromTheme("edit-redo"), "Redo", this);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redoLastChange);
    redoAction->setShortcut(QKeySequence::Redo);

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Copy", this);
    connect(copyAction, &QAction::triggered, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            QGuiApplication::clipboard()->setText(tabDataMap[index].editor->textCursor().selectedText());
        }
    });
    copyAction->setShortcut(QKeySequence::Copy);

    QAction *cutAction = new QAction(QIcon::fromTheme("edit-cut"), "Cut", this);
    connect(cutAction, &QAction::triggered, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            QTextCursor cursor = tabDataMap[index].editor->textCursor();
            QGuiApplication::clipboard()->setText(cursor.selectedText());
            cursor.removeSelectedText();
        }
    });
    cutAction->setShortcut(QKeySequence::Cut);

    QAction *pasteAction = new QAction(QIcon::fromTheme("edit-paste"), "Paste", this);
    connect(pasteAction, &QAction::triggered, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            tabDataMap[index].editor->insertPlainText(QGuiApplication::clipboard()->text());
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
            int index = tabWidget->currentIndex();
            if (index >= 0 && tabDataMap.contains(index)) {
                encodeCurrentText(name);
            }
        });
        encodeMenu->addAction(encodeAction);

        QAction *decodeAction = new QAction(name, this);
        connect(decodeAction, &QAction::triggered, this, [this, name]() {
            int index = tabWidget->currentIndex();
            if (index >= 0 && tabDataMap.contains(index)) {
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
    int index = tabWidget->currentIndex();
    if (index < 0 || !tabDataMap.contains(index)) return;
    QTextCursor cursor = tabDataMap[index].editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    cursorLabel->setText(QString("Ln %1, Col %2").arg(line).arg(col));
}

void MainWindow::newTab() {
    QTextEdit *editor = new QTextEdit(this);
    int index = tabWidget->addTab(editor, "Untitled");
    tabWidget->setCurrentIndex(index);

    TabData data;
    data.editor = editor;
    data.isModified = false;
    tabDataMap[index] = data;
    connect(editor, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateCursorStatus);
    connect(editor, &QTextEdit::textChanged, this, [this, index]() {
        markModified(index, true);
    });
}

void MainWindow::markModified(int index, bool modified) {
    if (!tabDataMap.contains(index)) return;
    tabDataMap[index].isModified = modified;
    QString title = tabWidget->tabText(index);
    if (modified && !title.endsWith("*")) {
        tabWidget->setTabText(index, title + "*");
    } else if (!modified && title.endsWith("*")) {
        tabWidget->setTabText(index, title.left(title.length() - 1));
    }
}

void MainWindow::encodeCurrentText(const QString &encoderName) {
    int index = tabWidget->currentIndex();
    if (!tabDataMap.contains(index)) return;
    QTextEdit *editor = tabDataMap[index].editor;
    QString text = editor->toPlainText();
    QString result;

    if (encoderName == "Base64") {
        result = Base64Codec::transform(text, false);
    } else if (encoderName == "ROT13") {
        result = Rot13::transform(text);
    } else if (encoderName == "Caesar") {
        bool ok;
        int shift = QInputDialog::getInt(this, "Caesar Shift", "Shift: ", 3, -25, 25, 1, &ok);
        if (ok) result = CaesarCipher::transform(text, shift, false);
        else return;
    } else if (encoderName == "Binary") {
        result = BinaryCodec::transform(text, false);
    } else if (encoderName == "Hex") {
        result = HexCodec::transform(text, false);
    } else if (encoderName == "PigLatin") {
        result = PigLatin::transform(text, false);
    } else if (encoderName == "Atbash") {
        result = Atbash::transform(text);
    } else if (encoderName == "Morse") {
        result = MorseCodec::transform(text, false);
    } else if (encoderName == "AES") {
        bool ok;
        QString key = QInputDialog::getText(this, "AES Key", "Enter encryption key:", QLineEdit::Password, "", &ok);
        if (!ok || key.isEmpty()) return;
        result = AESCodec::encode(text, key);
    }

    editor->setPlainText(result);
    markModified(index, true);
}

void MainWindow::decodeCurrentText(const QString &decoderName) {
    int index = tabWidget->currentIndex();
    if (!tabDataMap.contains(index)) return;

    QString text = tabDataMap[index].editor->toPlainText();
    QString result;

    if (decoderName == "Base64") {
        result = Base64Codec::transform(text, true);
    } else if (decoderName == "ROT13") {
        result = Rot13::transform(text);
    } else if (decoderName == "Caesar") {
        bool ok;
        int shift = QInputDialog::getInt(this, "Caesar Shift", "Shift: ", 3, -25, 25, 1, &ok);
        if (ok) result = CaesarCipher::transform(text, shift, true);
        else return;
    } else if (decoderName == "Binary") {
        result = BinaryCodec::transform(text, true);
    } else if (decoderName == "Hex") {
        result = HexCodec::transform(text, true);
    } else if (decoderName == "PigLatin") {
        result = PigLatin::transform(text, true);
    } else if (decoderName == "Atbash") {
        result = Atbash::transform(text);
    } else if (decoderName == "Morse") {
        result = MorseCodec::transform(text, true);
    } else if (decoderName == "AES") {
        bool ok;
        QString key = QInputDialog::getText(this, "AES Key", "Enter decryption key:", QLineEdit::Password, "", &ok);
        if (!ok || key.isEmpty()) return;
        result = AESCodec::decode(text, key);
    }

    if (!result.isEmpty()) {
        tabDataMap[index].editor->setPlainText(result);
        markModified(index, true);
    }
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
        QMessageBox::warning(this, "Open Failed", "Could not open file: " + filePath);
        recentFiles.removeAll(filePath);
        updateRecentFilesMenu();
        saveRecentFiles();
        return;
    }
    QString content = QTextStream(&file).readAll();
    newTab();
    int index = tabWidget->currentIndex();
    tabDataMap[index].editor->setPlainText(content);
    tabWidget->setTabText(index, QFileInfo(filePath).fileName());
    tabDataMap[index].filePath = filePath;
    markModified(index, false);
    recentFiles.removeAll(filePath);
    recentFiles.prepend(filePath);
    if (recentFiles.size() > 10) recentFiles.removeLast();
    updateRecentFilesMenu();
    saveRecentFiles();
}

void MainWindow::saveFileAs() {
    int index = tabWidget->currentIndex();
    if (!tabDataMap.contains(index)) return;
    QString path = QFileDialog::getSaveFileName(this, "Save File As");
    if (path.isEmpty()) return;
    QFile testFile(path);
    if (!testFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed", "Cannot write to file: " + path);
        return;
    }
    testFile.close();
    tabDataMap[index].filePath = path;
    saveFile(index);
}

void MainWindow::saveFile(int index) {
    if (!tabDataMap.contains(index)) return;
    QString path = tabDataMap[index].filePath;
    if (path.isEmpty()) {
        saveFileAs();
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out << tabDataMap[index].editor->toPlainText();
    file.close();
    markModified(index, false);
    tabWidget->setTabText(index, QFileInfo(path).fileName());
}

void MainWindow::saveAll() {
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabDataMap.contains(i) && tabDataMap[i].isModified) {
            saveFile(i);
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

void MainWindow::saveSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    settings.setValue("window/maximized", isMaximized());
    settings.setValue("window/normalSize", normalGeometry().size());
    settings.setValue("window/normalPos", normalGeometry().topLeft());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    settings.setValue("count", tabWidget->count());

    if (settings.value("window/maximized", false).toBool()) {
        showMaximized();
    } else {
        resize(settings.value("window/size", QSize(800, 600)).toSize());
        move(settings.value("window/pos", QPoint(200, 200)).toPoint());
    }

    settings.setValue("ui/codecToolbarVisible", codecToolbar->isVisible());
    settings.setValue("ui/mainToolbarVisible", mainToolbar->isVisible());
    settings.setValue("ui/statusBarVisible", statusBar()->isVisible());
    settings.setValue("restorePreviousSession", restorePreviousSession);

    settings.beginGroup("session");
    QStringList paths;
    QStringList contents;

    for (const auto &tab : tabDataMap) {
        // Add file path, even if empty (new tab)
        paths << tab.filePath;
        if (tab.isModified) {
            // Save unsaved data
            contents << tab.editor->toPlainText();
        } else {
            // Indicates to load from disk
            contents << "";
        }
    }
    settings.setValue("filePaths", paths);
    settings.setValue("unsavedContents", contents);
    settings.endGroup();
}

void MainWindow::restoreSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    int count = settings.value("count").toInt();

    // Checks if session restoration is wanted, and loads the files if so
    restorePreviousSession = settings.value("restorePreviousSession", true).toBool();
    if (restorePreviousSession) {
        settings.beginGroup("session");
        QStringList paths = settings.value("filePaths").toStringList();
        QStringList contents = settings.value("unsavedContents").toStringList();

        for (int i = 0; i < paths.size(); ++i) {
            const QString &path = paths[i];
            const QString &content = contents[i];
            if (paths.size() != contents.size()) {
                qWarning("Session restore data is inconsistent. Skipping session restore.");
                return;
            }
            if (path.isEmpty()) {
                // Create new tab with content
                int index = tabWidget->addTab(new QTextEdit, "Untitled");
                QTextEdit *editor = qobject_cast<QTextEdit*>(tabWidget->widget(index));
                editor->setPlainText(content);
                editor->setFont(currentFont);
                editor->setTabStopDistance(currentTabSize * QFontMetrics(currentFont).horizontalAdvance(' '));

                TabData tabData;
                tabData.editor = editor;
                tabData.filePath.clear();
                tabData.isModified = true;
                tabDataMap[index] = tabData;
                tabWidget->setTabText(index, "Untitled*");
            } else {
                // load from disk
                if (!content.isEmpty()) {
                    int index = tabWidget->currentIndex();  // latest tab added
                    tabDataMap[index].editor->setPlainText(content);
                    markModified(index, true);
                } else {
                    loadFile(path);
                }
            }
        }
        settings.endGroup();
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
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    // make sure that there is at least one tab
    if (tabWidget->count() == 0) {
        newTab();
    }
}

void MainWindow::maybeSaveAndClose(int index) {
    if (!tabDataMap.contains(index)) return;
    if (tabDataMap[index].isModified) {
        auto response = QMessageBox::question(this, "Unsaved Changes",
                                              "Do you want to save changes before closing?");
        if (response == QMessageBox::Yes) {
            saveFile(index);
        } else if (response == QMessageBox::Cancel) {
            return;
        }
    }
    tabDataMap.remove(index);
    tabWidget->removeTab(index);
}

void MainWindow::closeTab(int index) {
    maybeSaveAndClose(index);
    if (tabWidget->count() == 0) {
        newTab();
    }
}

void MainWindow::undoLastChange() {
    int index = tabWidget->currentIndex();
    if (tabDataMap.contains(index)) {
        tabDataMap[index].editor->undo();
    }
}

void MainWindow::redoLastChange() {
    int index = tabWidget->currentIndex();
    if (tabDataMap.contains(index)) {
        tabDataMap[index].editor->redo();
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
