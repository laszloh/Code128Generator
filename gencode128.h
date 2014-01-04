#ifndef GENCODE128_H
#define GENCODE128_H

#include <QLabel>
#include <QArrayData>
#include <QPainter>


class GenCode128 : public QLabel
{
    Q_OBJECT
public:
    explicit GenCode128(bool quietZone, bool humanReadable, int barWeigth, QWidget *parent = 0);
    explicit GenCode128(QWidget *parent = 0);

    QSize minimumSize();
    void setText(const QString text);

    QString getBarcodeAsString();

    bool getQuietZone() const;
    void setQuietZone(bool value);

    bool getHumanReadable() const;
    void setHumanReadable(bool value);

    int getBarWeigth() const;
    void setBarWeigth(int value);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *event);

    QList<int> StringToCode128(QString input);

    int SelectBarcodeAorB(unsigned char c);
    int BarcodeEncodeAs(unsigned char code, unsigned char c);

private:
    static const int quietWidth;
    static const int whScale;
    static const char *codeset[107];

    bool quietZone;
    bool humanReadable;
    int barWeigth;
};

#endif // GENCODE128_H
