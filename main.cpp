#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/decode.svg"));
    MainWindow window;
    window.show();
    return app.exec();
}
