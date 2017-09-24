#include "mainwindow.h"
#include <QApplication>
#include <iostream>



int main(int argc, char *argv[])
{  
    QApplication a(argc, argv);

    bool isClient = false;

    QStringList args = a.arguments();
    if (args.count() == 2) {

        if(args.at(1) == "--client") {
            isClient = true;
        }
    }


    MainWindow app(isClient);

    app.show();


    return a.exec();
}
