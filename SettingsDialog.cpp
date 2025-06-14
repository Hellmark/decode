#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QFontDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Settings");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout;

    fontButton = new QPushButton("Choose Font");
    connect(fontButton, &QPushButton::clicked, this, &SettingsDialog::chooseFont);
    formLayout->addRow("Font:", fontButton);

    tabSizeSpin = new QSpinBox;
    tabSizeSpin->setRange(2, 16);
    formLayout->addRow("Tab Size:", tabSizeSpin);

    restoreSessionCheck = new QCheckBox("Restore previous session");
    formLayout->addRow(restoreSessionCheck);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    clearRecentFilesButton = new QPushButton("Clear Recent Files");
    resetUILayoutButton = new QPushButton("Reset UI");
    buttonLayout->addWidget(clearRecentFilesButton);
    buttonLayout->addWidget(resetUILayoutButton);
    mainLayout->addLayout(buttonLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    mainLayout->addWidget(buttonBox);

    connect(clearRecentFilesButton, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Confirm", "Clear recent files list?") == QMessageBox::Yes)
            emit clearRecentFiles();
    });

    connect(resetUILayoutButton, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Confirm", "Reset UI layout to default?") == QMessageBox::Yes) {
            QFont defaultFont = QFont("Monospace", 10);
            int defaultTabSize = 4;
            selectedFont = defaultFont;
            fontButton->setText(selectedFont.family());
            tabSizeSpin->setValue(defaultTabSize);
            emit resetUILayout();
        }
    });
}

void SettingsDialog::chooseFont() {
    bool ok;
    QFont selected = QFontDialog::getFont(&ok, selectedFont, this);
    if (ok) {
        selectedFont = selected;
        fontButton->setText(selectedFont.family());
    }
}

void SettingsDialog::setFont(const QFont &f) {
    selectedFont = f;
    fontButton->setText(selectedFont.family());
}

QFont SettingsDialog::getFont() const {
    return selectedFont;
}

int SettingsDialog::getTabSize() const {
    return tabSizeSpin->value();
}

void SettingsDialog::setTabSize(int size) {
    tabSizeSpin->setValue(size);
}

bool SettingsDialog::shouldRestoreSession() const {
    return restoreSessionCheck->isChecked();
}

void SettingsDialog::setRestoreSession(bool restore) {
    restoreSessionCheck->setChecked(restore);
}
