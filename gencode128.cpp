#include "gencode128.h"

#include <QList>
#include <QByteArray>
#include <QPaintEvent>
#include <QSize>

const int GenCode128::quietWidth = 10;
const int GenCode128::whScale = 40;

const char *GenCode128::codeset[107] = {
    "212222", "222122", "222221", "121223", "121322",  /*  0 -  4 */
    "131222", "122213", "122312", "132212", "221213",
    "221312", "231212", "112232", "122132", "122231",  /* 10 - 14 */
    "113222", "123122", "123221", "223211", "221132",
    "221231", "213212", "223112", "312131", "311222",  /* 20 - 24 */
    "321122", "321221", "312212", "322112", "322211",
    "212123", "212321", "232121", "111323", "131123",  /* 30 - 34 */
    "131321", "112313", "132113", "132311", "211313",
    "231113", "231311", "112133", "112331", "132131",  /* 40 - 44 */
    "113123", "113321", "133121", "313121", "211331",
    "231131", "213113", "213311", "213131", "311123",  /* 50 - 54 */
    "311321", "331121", "312113", "312311", "332111",
    "314111", "221411", "431111", "111224", "111422",  /* 60 - 64 */
    "121124", "121421", "141122", "141221", "112214",
    "112412", "122114", "122411", "142112", "142211",  /* 70 - 74 */
    "241211", "221114", "413111", "241112", "134111",
    "111242", "121142", "121241", "114212", "124112",  /* 80 - 84 */
    "124211", "411212", "421112", "421211", "212141",
    "214121", "412121", "111143", "111341", "131141",  /* 90 - 94 */
    "114113", "114311", "411113", "411311", "113141",
    "114131", "311141", "411131", "211412", "211214",  /* 100 - 104 */
    "211232", "2331112"
};

#define START_A 103
#define START_B 104
#define START_C 105
#define STOP    106
#define SHIFT    98 /* only A and B */
#define CODE_A  101 /* only B and C */
#define CODE_B  100 /* only A and C */
#define CODE_C   99 /* only A and B */
#define FUNC_1  102 /* all of them */
#define FUNC_2   97 /* only A and B */
#define FUNC_3   96 /* only A and B */

#define NEED_CODE_A(c) ((c)<32 || (c)==0x80)
#define NEED_CODE_B(c) ((c)>=96 && (c)<128)

GenCode128::GenCode128(bool quietZone, bool humanReadable, int barWeigth, QWidget *parent) :
    GenCode128(parent)
{
    this->quietZone = quietZone;
    this->humanReadable = humanReadable;
    this->barWeigth = barWeigth;
}

GenCode128::GenCode128(QWidget *parent):
    QLabel(parent)
{
    this->quietZone = true;
    this->humanReadable = true;
    this->barWeigth = 2;
}

QSize GenCode128::minimumSize()
{
    QSize s = QLabel::sizeHint();

    int height = barWeigth * whScale;
    int width = 0;
    // calculate barcode size
    QString barcode = getBarcodeAsString();
    for(int i=0;i<barcode.count();i++) {
        width += barcode[i].digitValue()*barWeigth;
    }

    if(quietZone)
        width += 2 * quietWidth * barWeigth;

    return QSize(qMax(s.width(),width),s.height() + height);
}

void GenCode128::setText(const QString text)
{
    QLabel::setText(text);
    this->update();
}

/**
 * @brief GenCode128::getBarcodeAsString
 * @return
 */
QString GenCode128::getBarcodeAsString()
{
    QString barcode;
    QList<int> codes = StringToCode128(text());

    for(int i=0;i<codes.count();i++) {
        barcode.append(codeset[codes[i]]);
    }

    return barcode;
}

/**
 * @brief GenCode128::paintEvent
 * @param event
 */
void GenCode128::paintEvent(QPaintEvent *event)
{
    QPainter p;
    QSize s = minimumSize();
    int height = event->rect().height();
    int x = (event->rect().width() - s.width())/2;
    x += (quietZone) ? quietWidth * barWeigth : 0;

    p.begin(this);
    p.setPen(QColor::fromRgb(0,0,0));
    p.fillRect(event->rect(),QBrush(QColor::fromRgb(255,255,255)));

    if(humanReadable){
        p.drawText(event->rect(),Qt::AlignHCenter|Qt::AlignBottom,text());
        // leave place for the text!
        height -= (this->fontMetrics().boundingRect(this->text()).height() + 5);
    }

    QString barcode = getBarcodeAsString();
    for(int i=0;i<barcode.count();i++) {
        int width = barcode[i].digitValue() * barWeigth;
        if( !(i & 0x01))
            p.fillRect(x, 0, width, height, QColor::fromRgb(0,0,0));
        x += width;
    }

    p.end();
}

/**
 * @brief GenCode128::StringToCode128
 * @param input
 * @return
 */
QList<int> GenCode128::StringToCode128(QString input)
{
    // convert the string into a byte array
    QByteArray inputeByte = input.toLocal8Bit();
    unsigned char *s = (unsigned char *)inputeByte.data();

    // the codes
    int code;
    QList<int> codes;

    if(s[2] == '\0' && QChar::isDigit(s[0]) && QChar::isDigit(s[1])){
        code = 'C';
    } else if (QChar::isDigit(s[0]) && QChar::isDigit(s[1]) &&
               QChar::isDigit(s[2]) && QChar::isDigit(s[3])) {
        code = 'C';
    } else {
        code = SelectBarcodeAorB(s[0]);
        if(!code) code = 'B';
    }

    codes.append(START_A + code - 'A');
    for(/*directly using s*/; *s; /*incrementing in the loop*/){
        switch (code) {
        case 'C':
                if(s[0] == 0xC1) {
                    codes.append(FUNC_1);
                    s++;
                }else if(QChar::isDigit(s[0]) && QChar::isDigit(s[1])){
                    // encode and consume the two digits
                    codes.append((s[0]-'0')*10 + s[1]-'0');
                    s += 2;
                } else {
                    // change code
                    code = SelectBarcodeAorB(s[0]);
                    if(!code) code = 'B';
                    codes.append((code == 'A') ? CODE_A : CODE_B);
                }
            break;

        case 'A':
        case 'B':
        {
            int i;
            for(i=0; QChar::isDigit(s[i]);i++)
                ;
            if(i>=4){
                // there are more than 4 digits --> trun to C
                if(i&1) {
                    // endcode the first odd in the old coding
                    codes.append(s[0]-' ');
                    s++;
                }
                codes.append(CODE_C);
                code='C';
            } else if (code == 'A' && NEED_CODE_B(s[0])) {
                // check if we switch or use SHIFT
                int next = SelectBarcodeAorB(s[1]);
                if(next == 'B') {
                    codes.append(CODE_B);
                    code = 'B';
                } else {
                    // add shift and consume byte
                    codes.append(SHIFT);
                    codes.append(BarcodeEncodeAs('B', s[0]));
                    s++;
                }
            } else if (code == 'B' && NEED_CODE_A(s[0])) {
                // check if we switch or use SHIFT
                int next = SelectBarcodeAorB(s[1]);
                if(next == 'A') {
                    codes.append(CODE_A);
                    code='A';
                }else{
                    // add shift and consume byte
                    codes.append(SHIFT);
                    codes.append(BarcodeEncodeAs('A', s[0]));
                    s++;
                }
            } else {
                codes.append(BarcodeEncodeAs('A', s[0]));
                s++;
            }

        }
        default:
            break;
        }
    }

    // add the checksum
    int checksum = codes[0];
    for(int i=1;i<codes.count();i++)
        checksum += i * codes[i];
    checksum %= 103;
    codes.append(checksum);
    codes.append(STOP);

    return codes;
}

/**
 * @brief GenCode128::SelectBarcodeAorB
 * @param c
 * @return
 */
int GenCode128::SelectBarcodeAorB(unsigned char c)
{
    if(NEED_CODE_A(c))
        return 'A';
    if(NEED_CODE_B(c))
        return 'B';
    return 0;
}

/**
 * @brief GenCode128::BarcodeEncodeAs
 * @param code
 * @param c
 * @return
 */
int GenCode128::BarcodeEncodeAs(unsigned char code, unsigned char c)
{
    // check special chars
    if(c == 0xC1) return FUNC_1;
    if(c == 0xC2) return FUNC_2;
    if(c == 0xC3) return FUNC_3;
    if(c == 0xC4){
        if(code == 'A')
            return CODE_A;
        else
            return CODE_B;
    }

    // and the ascii values
    if(c >= 0x20 && c <= 0x5F)
        return c - 0x20;
    if(c == 0x80) return 64;
    if(c < 0x20) return c+64;
    if(c >= 0x60) return c-0x20;
    return -1;

}
int GenCode128::getBarWeigth() const
{
    return barWeigth;
}

void GenCode128::setBarWeigth(int value)
{
    barWeigth = value;
}

bool GenCode128::getHumanReadable() const
{
    return humanReadable;
}

void GenCode128::setHumanReadable(bool value)
{
    humanReadable = value;
}

bool GenCode128::getQuietZone() const
{
    return quietZone;
}

void GenCode128::setQuietZone(bool value)
{
    quietZone = value;
}
