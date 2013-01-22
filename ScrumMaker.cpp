#include "ScrumMaker.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QString>
#include <QPainter>
#include <QFont>
#include <QPrinter>
#include <QMap>
#include <iostream>
ScrumMaker::ScrumMaker(QMap<QString, QString> param)
{
    makeScrumBoard(param);
    exit(0);
}

void ScrumMaker::showProgressBar(int percent)
{
    char percentBar[23];
    percentBar[22] = '\0';
    percentBar[21] = '|';
    percentBar[0] = '|';
    bool first = true;
    for (int i = 1; i < 21; ++i) {
        if (5*i <= percent || percent >= 95) {
            percentBar[i] = '=';
        } else {
            if (first) {
                percentBar[i] = '>';
                first = false;
            } else {
                percentBar[i] = '.';
            }
        }
    }
    std::cout << "\r" << percentBar;
}

void ScrumMaker::makeScrumBoard(QMap<QString, QString> param)
{
    m_widthPixel = (int)(300 * 2.36);
    m_heightPixel = (int)(300 * 3.87);
    QFile file(param["input"]);




    qDebug() << "Opening " << file.fileName() << "...";
    if (!file.exists()) {
        qDebug() << "File does not exist. Abort";
        exit(0);
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "File can not be read. Abort.";
        exit(0);
    }

    // Count line (only for showing the percent)
    int nbrOfLines = 0;
    while(!file.atEnd()) {
        file.readLine();
        ++nbrOfLines;
    }
    file.seek(0);
    // End of countline
    QList<QString> idToPrint;
    if (!param["id"].isEmpty()) {
        idToPrint = param["id"].split(",");
    }
    int pageWidth = (int)(11.69*300);
    int pageHeight = (int)(8.27*300);
    QImage* pageImage = new QImage(pageWidth, pageHeight, QImage::Format_RGB32); // 20x28.5 (A4)
    pageImage->fill(Qt::white);
    QPainter painter(pageImage);
    int i = 0, j = 0;
    QString data = file.readLine(); // First line

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(param["output"]);
    printer.setResolution(300);
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    QPainter painterPrint(&printer);

    int currentLine = 1;
    while (!file.atEnd()) {
        showProgressBar(int(currentLine++/(float)nbrOfLines *100));
        data = file.readLine();
        data.replace(QString("\"\""), QString(""));
        data.replace(QString("\\E9"), QChar(233)); // é
        QStringList list = data.split(",");
        if (list.count() < 7) {
            continue;
        }
        QMap<QString, QString> map;
        int item = 0;
        // #,Assignee,Tracker,Status,Subject,Target version,Priority,% Done,Description
        map["id"] = list.at(item++);
        map["assignee"] = list.at(item++);
        map["tracker"] = list.at(item++);
        map["status"] = list.at(item++);
        map["subject"] = list.at(item++);
        if ( map["subject"].left(1).compare("\"") == 0 &&
             map["subject"].right(1).compare("\"") != 0) {
            do {
                map["subject"].append(", ");
                map["subject"].append(list.at(item++));
            } while (map["subject"].count("\"") %2 != 0);
        }
        map["subject"] = specialTrim(map["subject"]);
        map["target"] = specialTrim(list.at(item++));
        map["priority"] = specialTrim(list.at(item++));
        map["done"] = list.at(item++);
        QString description;
        for (int ii = item; ii < list.count(); ++ii) {
            description.append(list.at(ii));
        }

        if (!file.atEnd()) {
        // Peek next line;

        char buff[5];
        bool number = true;
        do {
            file.peek(buff, sizeof(buff));
            number = isNumber(buff, sizeof(buff));
            if (!number) {
                data = file.readLine();
                ++currentLine;
                description.append(data);
            }
        } while(!file.atEnd() && !number);
        map["description"] = specialTrim(description.trimmed());
        }

//        if (map["id"].contains("1783")) {
//        qDebug() << map["id"];//  = list.at(0);
//        qDebug() << map["assignee"];// = list.at(1);
//        qDebug() << map["tracker"];// = list.at(2);
//        qDebug() << map["status"];// = list.at(3);
//        qDebug() << map["subject"];// = list.at(4);
//        qDebug() << map["priority"];// = list.at(5);
//        qDebug() << map["done"];//  = list.at(6)
//        qDebug() << map["description"];
//        qDebug() << "-----\n";
//        }
        if (!param["user"].isEmpty() && !map["assignee"].contains(param["user"], Qt::CaseInsensitive)){
            continue;
        }

        if ( !param["type"].contains(map["tracker"].left(1), Qt::CaseInsensitive))
        {
            // Special for safety
            if (map["tracker"].contains("Safety", Qt::CaseInsensitive))
            continue;
        }

        if (idToPrint.count() > 0 && !idToPrint.contains(map["id"])) {
            continue;
        }
        QImage* fiche = createFiche(map);
        painter.drawImage(i*m_widthPixel, j*m_heightPixel, *fiche);
        ++i;
        if ((i + 1)*m_widthPixel > pageWidth) {
            i = 0;
            ++j;
            if ((j + 1)*m_heightPixel > pageHeight) {
                i = 0;
                j = 0;
                painterPrint.drawImage(QPoint(0, 0), *pageImage);
                pageImage->fill(Qt::white);
                if (!file.atEnd()) {
                    printer.newPage();
                }
            }
        }       
    }
    painterPrint.drawImage(QPoint(0, 0), *pageImage);
    painter.end();
    painterPrint.end();
    delete pageImage;
    showProgressBar(100);
    std::cout << std::endl;
    std::cout << "Done" << std::endl;
}

QImage *ScrumMaker::createFiche(QMap<QString, QString> map)
{


    int headerHeight = 150;
    int bottomHeight = 80;
    QFont font;
    QFontMetrics fm(font);

    QImage* pixmap = new QImage(m_widthPixel, m_heightPixel, QImage::Format_RGB32);
    pixmap->fill(Qt::white);
    QPainter painter(pixmap);
    QPen pen;
    // Draw background
    pen.setWidth(2);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.setBrush(Qt::white);
    painter.drawRect(0,0, m_widthPixel - 1, m_heightPixel - 1);

    if ( map["priority"].compare("Urgent", Qt::CaseInsensitive) == 0) {
        // First write background "Urgent"
        font = QFont("Arial", 200);
        painter.setFont(font);
        painter.setPen(Qt::lightGray);
        painter.save();
        painter.translate(20, 300);
        painter.rotate(60);
        painter.drawText(0, 0, "Urgent");
        painter.setPen(Qt::red);
        painter.restore();
        painter.setPen(Qt::black);
    }

    // Draw header
    if (map["tracker"].contains("Technical", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(100,205,255));
    } else if (map["tracker"].contains("User", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(255, 255, 175));
    } else if (map["tracker"].contains("Scenario", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(95, 255, 60));
    } else if (map["tracker"].contains("Problem", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(255, 155, 155));
    } else if (map["tracker"].contains("Change", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(255, 175, 0));
    } else if (map["tracker"].contains("Safety", Qt::CaseInsensitive)) {
        painter.setBrush(QColor::fromRgb(220, 90, 255));
    } else {
        painter.setBrush(QColor::fromRgb(200, 200, 200));
    }
    painter.drawRect(0, 0, m_widthPixel -1, headerHeight);

    // - Draw tracker
    font = QFont("Arial", 22);
    painter.setFont(font);
    painter.drawText(20, 40, map["tracker"]);

    // - Draw Priority (in the header)

    if (!map["priority"].trimmed().isEmpty()) {
        if ( map["priority"].compare("Urgent", Qt::CaseInsensitive) == 0 ||
             map["priority"].compare("High", Qt::CaseInsensitive) == 0) {
            painter.setPen(Qt::red);
        } else if  (map["priority"].compare("Low", Qt::CaseInsensitive) == 0) {
            painter.setPen(Qt::blue);
        }
        fm = QFontMetrics(font);
        painter.drawText(m_widthPixel -20 - fm.width(map["priority"]), 40,  map["priority"]);
        // Reset the pen to black
        painter.setPen(Qt::black);
    }

    // - Draw assignee

    font = QFont("Arial", 24);
    fm = QFontMetrics(font);
    painter.setFont(font);
    painter.drawText(m_widthPixel - 20 - fm.width(map["assignee"]), 125, map["assignee"]);


    // - Draw id
    font = QFont("Arial", 48);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(50, 125, map["id"]);
    font.setBold(false);

    // End of header

    // Draw title
    font = QFont("Arial", 30);
    font.setBold(true);
    if ( map["priority"].compare("Urgent", Qt::CaseInsensitive) == 0 ||
         map["priority"].compare("High", Qt::CaseInsensitive) == 0) {
        painter.setPen(Qt::red);
    } else if  (map["priority"].compare("Low", Qt::CaseInsensitive) == 0) {
        painter.setPen(Qt::blue);
    }
    painter.setFont(font);
    QRect* boundingBox = new QRect;
    painter.drawText(QRect(10, headerHeight + 20, m_widthPixel - 20, 300), Qt::TextWordWrap | Qt::AlignHCenter , map["subject"], boundingBox);
    int subjectHeight =  boundingBox->height();
    delete boundingBox;
    font.setBold(false);
    // Reset the pen to black
    painter.setPen(Qt::black);

    // Draw description
    if ( map["priority"].compare("Urgent", Qt::CaseInsensitive) == 0) {
        painter.setPen(Qt::red);
    }

    if (map["description"].count() < 1000){
        font = QFont("Arial", 26);
    } else {
        font = QFont("Arial", 20);
    }

    painter.setFont(font);
    painter.drawText(QRect(20, headerHeight + 40 + subjectHeight, m_widthPixel - 40, m_heightPixel - (headerHeight + 30 + subjectHeight) - bottomHeight), map["description"]);
    // Reset the pen to black
    painter.setPen(Qt::black);

    // Draw bottom
    painter.setBrush(Qt::white);
    font = QFont("Arial", 20);
    painter.setFont(font);
    fm = QFontMetrics(font);
    painter.drawRect(0, m_heightPixel - bottomHeight, m_widthPixel - 1, bottomHeight - 1);
    painter.drawText(m_widthPixel - fm.width(map["target"]) - 20, m_heightPixel - 20, map["target"]);
    painter.end();

    return pixmap;

}

bool ScrumMaker::isNumber(char *buff, const int size)
{
    bool atLeastOneNumber = false;
    for (int i = 0; i < size; ++i) {
        if (buff[i] == ',' && atLeastOneNumber) {
            return true;
        } else if (!(buff[i] >= '0' && buff[i] <= '9')) {
            return false;
        } else {
            atLeastOneNumber = true;
        }
    }
    return true;
}


QString ScrumMaker::specialTrim(QString string)
{
    if ( string.left(1).compare("\"") == 0 &&
         string.right(1).compare("\"") == 0) {
        string[string.length() - 1] = ' ';
        return string.mid(1);
    } else {
        return string;
    }
}

