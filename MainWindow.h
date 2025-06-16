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
#include <QToolBar>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>

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
    void decodeCurrentText(const QString &method);
    void saveAll();

    void setTabSize(int size);
    int getTabSize() const;

    void setRestoreSession(bool restore);
    bool shouldRestoreSession() const;

    QFont getFont() const;
    void setFont(const QFont &f);

    void clearRecentFiles();
    void resetUILayout();

private:
    QTabWidget *tabWidget;
    QComboBox *encoderSelector;
    QCheckBox *decodeCheckBox;
    QLabel *cursorLabel;
    QPushButton *encodeButton;
    QPushButton *decodeButton;

    QToolBar *codecToolbar;
    QToolBar *mainToolbar;

    QMenu *recentMenu;
    QAction *recentFileActions[10];
    QAction *saveAsAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *copyAction;
    QAction *cutAction;
    QAction *pasteAction;
    QAction *settingsAction;

    QAction *toggleCodecToolbar;
    QAction *toggleMainToolbar;
    QAction *toggleStatusBar;

    QStringList recentFiles;

    QFont currentFont;
    int currentTabSize;
    bool restorePreviousSession;

    QSettings settings;

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
    QString decodeText(const QString &input, const QString &method);
    void updateRecentFilesMenu();
    void loadFile(const QString &filePath);
    void saveRecentFiles();
    void loadRecentFiles();
    void restoreSession();
    void saveSession();
    void updateCursorStatus();
    void closeEvent(QCloseEvent *event) override;
    void applyEditorSettings();
    void clearSession();
    void showAboutDialog();


    private slots:
        void openSettingsDialog();

};

#endif // MAINWINDOW_H
