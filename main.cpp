#include <QApplication>
#include <QDebug>
#include <QMap>
#include "ScrumMaker.h"

void showHelp() {
    qDebug() << "How to use:\nscrumMaster [-t psutc , -u userName] fileFromRedmin.csv [outputFile.pdf]";
    qDebug() << "-t : choose types to generate (default: all):";
    qDebug() << "\tp Problem report\n\tc Change request\n\ts Scenario\n\tu User story\n\tt Technical Story";
    qDebug() << "-u : choose user";
    qDebug() << "Example: ";
    qDebug() << "scrumMaster -t ut -u Engels fileFromRedmin.csv output.pdf";
    exit(0);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc < 2) {
        showHelp();
    } else {
        QMap<QString, QString> param;
        param["type"] = "psutc";
        param["output"] = "output.pdf";

        // Default parameters:
        for (int i = 1; i < argc; ++i) {
            QString str(argv[i]);
            if (str.compare("-t") == 0) {
                if (++i < argc) {
                    param["type"] = argv[i];
                } else {
                    showHelp();
                }
            } else if(str.compare("-u") == 0) {
                if (++i < argc) {
                    param["user"] = argv[i];
                } else {
                    showHelp();
                }
            } else if (str.contains(".csv")) {
                param["input"] = str;
            } else if (str.contains(".pdf")) {
                param["output"] = str;
            } else {
                showHelp();
            }
        }
        if (param["input"].isNull()) {
            showHelp();
        } else {
            ScrumMaker* maker = new ScrumMaker(param);
        }
    }

    return a.exec();
}
