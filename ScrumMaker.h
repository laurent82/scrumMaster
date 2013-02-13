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
    void showProgressBar(int percent);
    void makeScrumBoard(QMap<QString, QString> param);
    QString replaceComma(QString string);

    QImage *createFiche(QMap<QString, QString> map);
    bool isNumber(char* buff, const int size);
    QString specialTrim(QString);

    int m_widthPixel;
    int m_heightPixel;

};

#endif // SCRUMMAKER_H
