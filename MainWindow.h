#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QMap>
#include <QString>
#include <QComboBox>
#include <QCheckBox>
#include <QStack>
#include <QLabel>
#include <QAction>
#include <QMessageBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void newTab();
    void closeTab(int index);
    void saveFile(int index);
    void saveFileAs();
    void openFile();
    void openRecentFile();
    void maybeSaveAndClose(int index);
    void undoLastChange();
    void redoLastChange();
    void encodeCurrentText(const QString &method);

private:
    QTabWidget *tabWidget;
    QComboBox *encoderSelector;
    QCheckBox *decodeCheckBox;
    QStringList recentFiles;
    QAction *recentFileActions[10];
    QMenu *recentMenu;
    QLabel *cursorLabel;


    struct TabData {
        QTextEdit *editor;
        QString filePath;
        bool isModified = false;
        QStack<QString> undoStack;
        QStack<QString> redoStack;
    };

    QMap<int, TabData> tabDataMap;

    void setupUI();
    void connectSignals();
    void markModified(int index, bool modified);
    QString encodeText(const QString &input, const QString &method);
    void updateRecentFilesMenu();
    void loadFile(const QString &filePath);
    void saveRecentFiles();
    void loadRecentFiles();
    void restoreSession();
    void saveSession();
    void closeEvent(QCloseEvent *event) override;
    void updateCursorStatus();

};
#endif // MAINWINDOW_H
