#ifndef SCRUMMAKER_H
#define SCRUMMAKER_H

#include <QString>
#include <QPixmap>
#include <QImage>

class ScrumMaker
{
public:
    ScrumMaker(QMap<QString, QString> param);
private:
    void makeScrumBoard(QMap<QString, QString> param);
    QImage *createFiche(QMap<QString, QString> map);
    bool isNumber(char* buff, const int size);
    QString specialTrim(QString);

    int m_widthPixel;
    int m_heightPixel;

};

#endif // SCRUMMAKER_H
