#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFont>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QFontDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class QSpinBox;
class QCheckBox;
class QPushButton;
class QFontComboBox;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr);

    void setFont(const QFont &font);
    void setFontSize(int size);
    void setTabSize(int size);
    void setRestoreSession(bool restore);

    QFont getFont() const;

    int getFontSize() const;
    int getTabSize() const;
    bool shouldRestoreSession() const;

signals:
    void clearRecentFiles();
    void resetUILayout();

private:
    QFont selectedFont;
    QFontComboBox *fontCombo;
    QSpinBox *fontSizeSpin;
    QSpinBox *tabSizeSpin;
    QCheckBox *restoreSessionCheck;
    QPushButton *fontButton;
    QPushButton *clearRecentFilesButton;
    QPushButton *resetUILayoutButton;

private slots:
    void chooseFont();
};

#endif // SETTINGSDIALOG_H
