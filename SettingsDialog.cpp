#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QFontDialog>
#include <QFileDialog>
#include <QFileDevice>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>
#include <QLineEdit>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Settings");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout;

    // Sets the labels to be aligned to the left, so they're not all willy nilly.
    formLayout->setLabelAlignment(Qt::AlignLeft);

    // Font picker
    fontButton = new QPushButton("Choose Font");
    connect(fontButton, &QPushButton::clicked, this, &SettingsDialog::chooseFont);
    formLayout->addRow("Font:", fontButton);

    // Tab Size
    tabSizeSpin = new QSpinBox;
    tabSizeSpin->setRange(2, 16);
    formLayout->addRow("Tab Size:", tabSizeSpin);

    // Checkbox for session restoration
    restoreSessionCheck = new QCheckBox("Restore previous session");
    formLayout->addRow(restoreSessionCheck);

    // RSA key fields, done in a compact grid
    QLabel *rsaPublicLabel = new QLabel("RSA Public Key:", this);
    rsaPublicKeyEdit = new QLineEdit(this);
    browseRSAPublicKeyButton = new QPushButton("Browse...", this);

    QLabel *rsaPrivateLabel = new QLabel("RSA Private Key:", this);
    rsaPrivateKeyEdit = new QLineEdit(this);
    browseRSAPrivateKeyButton = new QPushButton("Browse...", this);

    QGridLayout *rsaGrid = new QGridLayout;
    rsaGrid->setVerticalSpacing(4);
    rsaGrid->setHorizontalSpacing(8);
    rsaGrid->addWidget(rsaPublicLabel, 0, 0);
    rsaGrid->addWidget(rsaPublicKeyEdit, 0, 1);
    rsaGrid->addWidget(browseRSAPublicKeyButton, 0, 2);

    rsaGrid->addWidget(rsaPrivateLabel, 1, 0);
    rsaGrid->addWidget(rsaPrivateKeyEdit, 1, 1);
    rsaGrid->addWidget(browseRSAPrivateKeyButton, 1, 2);

    // Wrap the grid layout in a QWidget so it can be added to formLayout as one row
    QWidget *rsaWidget = new QWidget(this);
    rsaWidget->setLayout(rsaGrid);

    // Now insert into the form layout
    formLayout->addRow(rsaWidget);

    mainLayout->addLayout(formLayout);

    // Clearing recent files and resetting UI, to help with any potential issues
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

// Functions for passing info to and from the main window
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

QString SettingsDialog::getRSAPublicKeyPath() const {
    return rsaPublicKeyEdit->text();
}
QString SettingsDialog::getRSAPrivateKeyPath() const {
    return rsaPrivateKeyEdit->text();
}
void SettingsDialog::setRSAPublicKeyPath(const QString &path) {
    rsaPublicKeyEdit->setText(path);
}
void SettingsDialog::setRSAPrivateKeyPath(const QString &path) {
    rsaPrivateKeyEdit->setText(path);
}
