#include "gencode128.h"

#include <QList>
#include <QByteArray>

const int GenCode128::quietWidth = 10;

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

GenCode128::GenCode128(QString inputData, int barWeight, bool addQuietZone, QObject *parent) :
    QObject(parent)
{
    this->barWeight = barWeight;
    this->addQuietZone = addQuietZone;
    this->inputData = inputData;
}

GenCode128::~GenCode128()
{
    delete painter;
}

void GenCode128::getBarcodeAsImage(QPaintDevice *surface)
{

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
    for(unsigned char *ptr=s; *ptr; /*incrementing in the loop*/){
        switch (code) {
        case 'C':
                if(s[0] == 0xC1) {
                    codes.append(FUNC_1);
                    ptr++;
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

QString GenCode128::getInputData() const
{
    return inputData;
}

void GenCode128::setInputData(const QString &value)
{
    inputData = value;
}

bool GenCode128::getAddQuietZone() const
{
    return addQuietZone;
}

void GenCode128::setAddQuietZone(bool value)
{
    addQuietZone = value;
}

int GenCode128::getBarWeight() const
{
    return barWeight;
}

void GenCode128::setBarWeight(int value)
{
    barWeight = value;
}


