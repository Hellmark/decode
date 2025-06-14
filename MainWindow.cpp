#include "MainWindow.h"
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
#include "encoders/PigLatin.h"
#include "encoders/Atbash.h"
#include "encoders/MorseCodec.h"

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

    // Codec toolbar
    codecToolbar = addToolBar("Codec Toolbar");
    codecToolbar->setObjectName("codecToolbar");
    encoderSelector = new QComboBox;
    QPushButton *encodeButton = new QPushButton("Encode");
    QPushButton *decodeButton = new QPushButton("Decode");
    encoderSelector->addItems({"Base64", "Binary", "Caesar", "ROT13", "Morse", "Atbash", "Pig Latin"});
    codecToolbar->addWidget(encoderSelector);
    codecToolbar->addWidget(encodeButton);
    codecToolbar->addWidget(decodeButton);

    connect(encodeButton, &QPushButton::clicked, this, [this]() {
        encodeCurrentText(encoderSelector->currentText());
    });
    connect(decodeButton, &QPushButton::clicked, this, [this]() {
        decodeCurrentText(encoderSelector->currentText());
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
        int index = tabWidget->currentIndex();
        saveFile(index);
    });
    saveAction->setShortcut(QKeySequence::Save);

    QAction *saveAllAction = new QAction(QIcon::fromTheme("document-save-all"), "Save All", this);
    connect(saveAllAction, &QAction::triggered, this, &MainWindow::saveAll);
    saveAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

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
    mainToolbar->addAction(settingsAction);

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

    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(copyAction);
    editMenu->addAction(cutAction);
    editMenu->addAction(pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(settingsAction);

    QMenu *viewMenu = menuBar()->addMenu("View");
    toggleCodecToolbar = new QAction("Show Codec Toolbar", this);
    toggleCodecToolbar->setCheckable(true);
    toggleCodecToolbar->setChecked(true);
    viewMenu->addAction(toggleCodecToolbar);
    connect(toggleCodecToolbar, &QAction::toggled, codecToolbar, &QToolBar::setVisible);

    toggleMainToolbar = new QAction("Show Main Toolbar", this);
    toggleMainToolbar->setCheckable(true);
    toggleMainToolbar->setChecked(true);
    viewMenu->addAction(toggleMainToolbar);
    connect(toggleMainToolbar, &QAction::toggled, mainToolbar, &QToolBar::setVisible);

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
    } else if (encoderName == "PigLatin") {
        result = PigLatin::transform(text, false);
    } else if (encoderName == "Atbash") {
        result = Atbash::transform(text);
    } else if (encoderName == "Morse") {
        result = MorseCodec::transform(text, false);
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
    } else if (decoderName == "PigLatin") {
        result = PigLatin::transform(text, true);
    } else if (decoderName == "Atbash") {
        result = Atbash::transform(text);
    } else if (decoderName == "Morse") {
        result = MorseCodec::transform(text, true);
    }

    tabDataMap[index].editor->setPlainText(result);
    markModified(index, true);
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
    for (int i = 0; i < tabWidget->count(); ++i) {
        settings.setValue(QString("file_%1").arg(i), tabDataMap[i].filePath);
    }
    if (settings.value("window/maximized", false).toBool()) {
        showMaximized();
    } else {
        resize(settings.value("window/size", QSize(800, 600)).toSize());
        move(settings.value("window/pos", QPoint(200, 200)).toPoint());
    }

    settings.setValue("ui/codecToolbarVisible", codecToolbar->isVisible());
    settings.setValue("ui/mainToolbarVisible", mainToolbar->isVisible());
    settings.setValue("ui/statusBarVisible", statusBar()->isVisible());
}

void MainWindow::restoreSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    int count = settings.value("count").toInt();
    for (int i = 0; i < count; ++i) {
        QString file = settings.value(QString("file_%1").arg(i)).toString();
        if (!file.isEmpty()) loadFile(file);
    }
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
