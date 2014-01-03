#ifndef GENCODE128_H
#define GENCODE128_H

#include <QObject>
#include <QArrayData>
#include <QPainter>
#include <QPixmap>


class GenCode128 : public QObject
{
    Q_OBJECT
public:
    explicit GenCode128(QString inputData, int barWeight, bool addQuietZone, QObject *parent = 0);
    ~GenCode128();

    void getBarcodeAsImage(QPaintDevice *surface);

    int getBarWeight() const;
    void setBarWeight(int value);

    bool getAddQuietZone() const;
    void setAddQuietZone(bool value);

    QString getInputData() const;
    void setInputData(const QString &value);

signals:

public slots:

private:
    QList<int> StringToCode128(QString input);

    int SelectBarcodeAorB(unsigned char c);
    int BarcodeEncodeAs(unsigned char code, unsigned char c);

private:
    static const int quietWidth;
    static const char *codeset[107];

    int barWeight;
    bool addQuietZone;
    QString inputData;

    QPainter *painter;
    QList<int> codeList;
};

#endif // GENCODE128_H
