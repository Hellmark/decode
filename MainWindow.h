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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void newTab();
    void closeTab(QTextEdit *editor);
    void saveFile(QTextEdit *editor);
    void saveFileAs(QTextEdit *editor);
    void openFile();
    void openFile(const QString &path);
    void openRecentFile();
    void maybeSaveAndClose(QTextEdit *editor);
    void undoLastChange();
    void redoLastChange();
    void encodeCurrentText(const QString &method);
    void decodeCurrentText(const QString &method);
    void saveAll();

    void setTabSize(int size);

    void setRestoreSession(bool restore);

    void setFont(const QFont &f);

    void clearRecentFiles();
    void resetUILayout();
    void clearSession();

private:
    QTabWidget *tabWidget;
    QComboBox *encoderSelector;
    QCheckBox *decodeCheckBox;
    QLabel *cursorLabel;
    QAction *encodeAction;
    QAction *decodeAction;

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

    QFont getFont() const;
    QFont currentFont;
    int currentTabSize;
    bool restorePreviousSession;
    int getTabSize() const;
    QString defaultRSAPrivateKeyPath;
    QString defaultRSAPublicKeyPath;

    bool shouldRestoreSession() const;

    int indexForEditor(QTextEdit *editor) const;

    const QString configName = "Hellmark";

    QSettings settings;

    struct TabData {
        QTextEdit *editor;
        QString filePath;
        bool isModified = false;
        QStack<QString> undoStack;
        QStack<QString> redoStack;
        QString lastKnownText;
    };

    QMap<QTextEdit*, TabData> tabDataMap;

    void setupUI();
    void connectSignals();
    void markModified(QTextEdit *editor, bool modified);
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
    void showAboutDialog();
    QString promptForKeyFile(const QString &title, const QString &defaultPath);

    private slots:
        void openSettingsDialog();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

};

#endif // MAINWINDOW_H
