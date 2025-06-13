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
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    setCentralWidget(tabWidget);

    QToolBar *toolbar = addToolBar("Toolbar");
    encoderSelector = new QComboBox;
    encoderSelector->addItems({"Base64", "Rot13", "Caesar", "Binary", "PigLatin", "Atbash", "Morse"});
    toolbar->addWidget(encoderSelector);

    decodeCheckBox = new QCheckBox("Decode");
    toolbar->addWidget(decodeCheckBox);

    QPushButton *applyButton = new QPushButton("Apply");
    toolbar->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        encodeCurrentText(encoderSelector->currentText());
    });

    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *newTabAction = fileMenu->addAction("New Tab");
    QAction *openAction = fileMenu->addAction("Open");

    QMenu *recentMenu = fileMenu->addMenu("Recent Files");
    for (int i = 0; i < 10; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        recentMenu->addAction(recentFileActions[i]);
    }

    QAction *saveAction = fileMenu->addAction("Save");
    QAction *saveAsAction = fileMenu->addAction("Save As");
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    connect(newTabAction, &QAction::triggered, this, &MainWindow::newTab);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, [this]() {
        saveFile(tabWidget->currentIndex());
    });
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    QMenu *editMenu = menuBar()->addMenu("Edit");
    QAction *undoAction = editMenu->addAction("Undo");
    QAction *redoAction = editMenu->addAction("Redo");
    editMenu->addSeparator();
    QAction *copyAction = editMenu->addAction("Copy");
    QAction *cutAction = editMenu->addAction("Cut");
    QAction *pasteAction = editMenu->addAction("Paste");
    editMenu->addSeparator();
    QAction *settingsAction = editMenu->addAction("Settings");

    connect(undoAction, &QAction::triggered, this, &MainWindow::undoLastChange);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redoLastChange);
    connect(copyAction, &QAction::triggered, this, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            tabDataMap[index].editor->copy();
        }
    });
    connect(cutAction, &QAction::triggered, this, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            tabDataMap[index].editor->cut();
        }
    });
    connect(pasteAction, &QAction::triggered, this, [this]() {
        int index = tabWidget->currentIndex();
        if (tabDataMap.contains(index)) {
            tabDataMap[index].editor->paste();
        }
    });
    connect(settingsAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Settings", "Settings dialog not implemented yet.");
    });

    QMenu *codecsMenu = menuBar()->addMenu("Codecs");
    QMenu *encodeMenu = codecsMenu->addMenu("Encode");
    QMenu *decodeMenu = codecsMenu->addMenu("Decode");
    const QStringList codecs = {"Base64", "Rot13", "Caesar", "Binary", "PigLatin", "Atbash", "Morse"};
    for (const QString &codec : codecs) {
        QAction *enc = encodeMenu->addAction(codec);
        connect(enc, &QAction::triggered, this, [this, codec]() {
            decodeCheckBox->setChecked(false);
            encodeCurrentText(codec);
        });
        QAction *dec = decodeMenu->addAction(codec);
        connect(dec, &QAction::triggered, this, [this, codec]() {
            decodeCheckBox->setChecked(true);
            encodeCurrentText(codec);
        });
    }

    statusBar()->showMessage("Ready");
    cursorLabel = new QLabel("Ln 1, Col 1");
    statusBar()->addPermanentWidget(cursorLabel);
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
    bool decode = decodeCheckBox->isChecked();

    if (encoderName == "Base64") {
        result = Base64Codec::transform(text, decode);
    } else if (encoderName == "Rot13") {
        result = Rot13::transform(text);
    } else if (encoderName == "Caesar") {
        bool ok;
        int shift = QInputDialog::getInt(this, "Caesar Shift", "Shift: ", 3, -25, 25, 1, &ok);
        if (ok) result = CaesarCipher::transform(text, shift, decode);
        else return;
    } else if (encoderName == "Binary") {
        result = BinaryCodec::transform(text, decode);
    } else if (encoderName == "PigLatin") {
        result = PigLatin::transform(text, decode);
    } else if (encoderName == "Atbash") {
        result = Atbash::transform(text);
    } else if (encoderName == "Morse") {
        result = MorseCodec::transform(text, decode);
    }

    editor->setPlainText(result);
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
    settings.setValue("count", tabWidget->count());
    for (int i = 0; i < tabWidget->count(); ++i) {
        settings.setValue(QString("file_%1").arg(i), tabDataMap[i].filePath);
    }
}

void MainWindow::restoreSession() {
    QSettings settings("Hellmark Programming Group", "Decode");
    int count = settings.value("count").toInt();
    for (int i = 0; i < count; ++i) {
        QString file = settings.value(QString("file_%1").arg(i)).toString();
        if (!file.isEmpty()) loadFile(file);
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
    QMainWindow::closeEvent(event);
}
