#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#define maxBufferLength 200
#define literalArrayLength 7

#define pg_selected_l           '<'
#define pg_selected_r           '>'

#define pg_border_angle_1       "╔"
#define pg_border_angle_2       "╚"
#define pg_border_angle_3       "╗"
#define pg_border_angle_4       "╝"
#define pg_border_vert          "║"
#define pg_border_hor           "═"

#define pg_winborder_angle_1    "┌"
#define pg_winborder_angle_2    "└"
#define pg_winborder_angle_3    "┐"
#define pg_winborder_angle_4    "┘"
#define pg_winborder_vert       "│"
#define pg_winborder_hor        "─"

#define wt_inputfield 0
#define wt_button 1

#define switchWindow(window) { focusedElement = 0; _selected_window = window; redrawScene();}

#define recalculateStrSize(string, height, width, h, w) { get_string_size(string, &w, &h); height += h; if(w + 4 > width) width = w + 4; }

#define printCenteredStr(string, length, hww, currY, offset) { pToXY(hww - length/2, currY, string); (currY) += offset; }


static char literals[literalArrayLength] = {
        'I', 'V', 'X', 'L', 'C', 'D', 'M'
};

static int literalNumbers[literalArrayLength] = {
        1, 5, 10, 50, 100, 500, 1000
};

static char* bases[4][3] = {
        {
                "единица",
                "единицы",
                "единиц",
        },
        {
                "десяток",
                "десятка",
                "десятков"
        },
        {
                "сотня",
                "сотни",
                "сотен"
        },
        {
                "тысяча",
                "тысячи",
                "тысяч"
        }
};

static char* exceptions[2] = {
        "одна", "две"
};

static char* smallNumbers[9] = {
        "один", "два", "три", "четыре", "пять", "шесть", "семь", "восемь", "девять"
};

static char* dozens[9] = {
        "десять", "двадцать", "тридцать", "сорок", "пятьдесят", "шестьдесят", "семьдесят", "восемьдесят", "девяносто"
};

static char* tens[9] = {
        "одинадцать", "двенадцать", "тринадцать", "четырнадцать", "пятнадцать", "шестнадцать", "семнадцать", "восемнадцать", "девятнадцать"
};

static char* hundreds[9] = {
        "сто", "двести", "триста", "четыреста", "пятьсот", "шестьсот", "семьсот", "восемьсот", "девятьсот"
};

int textToNumber(char* text, int __expt_mode) {
    if(text[strlen(text) - 1] == '\n') text[strlen(text) - 1] = '\0';
    if(strlen(text) == 0) return -1;
    if(__expt_mode != 1 && __expt_mode != 0) return -2;
    char** words = malloc(5 * sizeof(char*));
    int wordsCount = 0;
    int p = 0;
    int offset = 0;
    int result = 0;
    for(int i = 0; i <= strlen(text); i++){
        if(text[i] == ' ' || text[i] == '\0') {
            words[p] = malloc(i + 1);
            words[p][i] = '\0';
            memcpy(words[p++], text + offset, i - offset);
            offset = i + 1;
            wordsCount++;
        }
    }
    int * isCorrect;
    int hasLastBase = 0;
    int multiplier = 0;
    switch (wordsCount) {
        case 1:
            isCorrect = malloc(1 * sizeof(int));
            *isCorrect = 0;
            for(int i = 0; i < 4; i++)
                if(strcmp(words[0], bases[i][0]) == 0) {
                    result = pow(10, i);
                    *isCorrect = 1;
                    break;
                }
            if(!*isCorrect) {
                for(int i = 0; i < 9; i++) {
                    if(strcmp(words[0], smallNumbers[i]) == 0) {
                        result += i + 1;
                        *isCorrect = 1;
                        break;
                    }
                }
                if(!*isCorrect)  {
                    if(__expt_mode)
                        for(int i = 0; i < 2; i++)
                            if(strcmp(words[0], exceptions[i]) == 0) {
                                result += i + 1;
                                *isCorrect = 1;
                                break;
                            }
                    if(!*isCorrect) {
                        for(int i = 0; i < 9; i++)
                            if(strcmp(words[0], dozens[i]) == 0) {
                                result += (i + 1) * 10;
                                *isCorrect = 1;
                                break;
                            }
                        if(!*isCorrect) {
                            for(int i = 0; i < 9; i++)
                                if(strcmp(words[0], tens[i]) == 0) {
                                    result += 11 + i;
                                    *isCorrect = 1;
                                    break;
                                }
                            if(!*isCorrect) {
                                for(int i = 0; i < 9; i++)
                                    if(strcmp(words[0], hundreds[i]) == 0) {
                                        result += (i + 1) * 100;
                                        *isCorrect = 1;
                                        break;
                                    }
                                if(!*isCorrect) {
                                    if(strcmp(words[0], bases[3][0]) == 0)
                                        result += 1000;
                                    else result = -1;
                                }
                            }
                        }
                    }
                }

            }
            break;
        case 2:
            isCorrect = malloc(2 * sizeof(int));
            isCorrect[0] = 0;
            isCorrect[1] = 0;

            hasLastBase = 0;
            multiplier = -1;
            //-------------------------------LAST-BASE-CHECK------------------------------------------------------------
            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 3; j++)
                    if(strcmp(words[wordsCount - 1], bases[i][j]) == 0) {
                        hasLastBase = 1;
                        multiplier = textToNumber(words[0], 1);
                    }
            if(hasLastBase && multiplier != -1) { // bases-check
                switch ((multiplier % 10 < 10) ? multiplier % 10 : 5) {
                    case 1:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][0]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 2 ... 4:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][1]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 0:
                    case 5 ... 9:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][2]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    default: result = -1;
                }
            }
            //----------------------------------END-OF-LAST-BASE-CHECK--------------------------------------------------
            if(!hasLastBase) {
                if(strcmp(words[0], bases[3][0]) == 0) {
                    result += 1000;
                    isCorrect[0] = 1;
                } else {
                    for(int i = 0; i < 9; i++)
                        if(strcmp(words[0], hundreds[i]) == 0) {
                            result += (i + 1) * 100;
                            isCorrect[0] = 1;
                            break;
                        }
                    if(!isCorrect[0]) {
                        for(int i = 1; i < 9; i++)
                            if(strcmp(words[0], dozens[i]) == 0) {
                                result += (i + 1) * 10;
                                isCorrect[0] = 1;
                                break;
                            }
                        if(!isCorrect[0]) result = -1;
                    }
                }
                if(isCorrect[0]) {
                    if(result == 1000)
                        for(int i = 0; i < 9; i++) {
                            if(strcmp(words[1], hundreds[i]) == 0) {
                                result += (i + 1) * 100;
                                isCorrect[1] = 1;
                                break;
                            }
                        }
                    if(!isCorrect[1]) {
                        if(result >= 100) {
                            for(int i = 0; i < 9; i++) {
                                if(strcmp(words[wordsCount - 1], dozens[i]) == 0) {
                                    result += (i + 1) * 10;
                                    isCorrect[1] = 1;
                                    break;
                                }
                                if(strcmp(words[wordsCount - 1], tens[i]) == 0) {
                                    result += 11 + i;
                                    isCorrect[1] = 1;
                                    break;
                                }
                            }
                        }
                        if(!isCorrect[1]) {
                            for(int i = 0; i < 9; i++)
                                if(strcmp(words[wordsCount - 1], smallNumbers[i]) == 0) {
                                    result += i + 1;
                                    isCorrect[1] = 1;
                                    break;
                                }
                            if(!isCorrect[1]) result = -1;
                        }
                    }
                }
            }
            break;
        case 3:
            isCorrect = malloc(3 * sizeof(int));
            isCorrect[0] = 0;
            isCorrect[1] = 0;
            isCorrect[2] = 0;

            hasLastBase = 0;
            multiplier = -1;
            //-------------------------------LAST-BASE-CHECK------------------------------------------------------------
            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 3; j++)
                    if(strcmp(words[wordsCount - 1], bases[i][j]) == 0) {
                        hasLastBase = 1;
                        int mTextLength = strlen(words[0]) + strlen(words[1]) + 1;
                        char * multiplierText = malloc(mTextLength + 1);
                        strncpy(multiplierText, text, mTextLength);
                        multiplierText[mTextLength] = '\0';
                        multiplier = textToNumber(multiplierText, 1);
                        free(multiplierText);
                    }
            if(hasLastBase && multiplier != -1) {
                switch ((multiplier % 10 < 10) ? multiplier % 10 : 5) {
                    case 1:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[2], bases[i][0]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 2 ... 4:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[2], bases[i][1]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 0:
                    case 5 ... 9:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[2], bases[i][2]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    default: result = -1;
                }
            }
            //------------------------------------END-OF-LAST-BASE-CHECK------------------------------------------------
            //----------------------------------------IN-BASE-CHECK-----------------------------------------------------
            if(!isCorrect[0])
                for (int cursor = 0; cursor < wordsCount; cursor++)
                    for (int i = 0; i < 4; i++)
                        for (int j = 0; j < 3; j++)
                            if (strcmp(words[cursor], bases[i][j]) == 0) {
                                int mFirstPartLength =
                                        ((cursor >= 0) ? strlen(words[0]) : 0) +
                                        ((cursor >= 1) ? strlen(words[1]) : 0) + cursor;
                                int mSecondPartLength = strlen(text) - mFirstPartLength - 1;
                                char *firstPart = malloc((mFirstPartLength + 1));
                                char *secondPart = malloc((mSecondPartLength + 1));
                                strncpy(firstPart, text, mFirstPartLength);
                                strncpy(secondPart, text + mFirstPartLength + 1, mSecondPartLength);
                                firstPart[mFirstPartLength] = '\0';
                                secondPart[mSecondPartLength] = '\0';
                                int first = textToNumber(firstPart, 1);
                                int second = textToNumber(secondPart, 1);
                                result = (first != -1 && second != -1) ? first + second : -1;
                                free(firstPart);
                                free(secondPart);
                                isCorrect[0] = 1;
                                break;
                            }
            //------------------------------------END-OF-IN-BASE-CHECK--------------------------------------------------
            //---------------------------------------NATURAL-NUMBER-CHECK-----------------------------------------------
            if (!isCorrect[0]) {

                if (strcmp(words[0], bases[3][0]) == 0) {
                    result += 1000;
                    isCorrect[0] = 1;
                } else {
                    for (int i = 0; i < 9; i++)
                        if (strcmp(words[0], hundreds[i]) == 0) {
                            result += (i + 1) * 100;
                            isCorrect[0] = 1;
                            break;
                        }
                    if (!isCorrect[0]) result = -1;

                }
                if (isCorrect[0]) {
                    if (result == 1000)
                        for (int i = 0; i < 9; i++)
                            if (strcmp(words[1], hundreds[i]) == 0) {
                                result += (i + 1) * 100;
                                isCorrect[1] = 1;
                                break;
                            }
                    if (!isCorrect[1]) {
                        if (result >= 100)
                            for (int i = 0; i < 9; i++)
                                if (strcmp(words[1], dozens[i]) == 0) {
                                    result += (i + 1) * 10;
                                    isCorrect[1] = 1;
                                    break;
                                }
                        if (!isCorrect[1]) result = -1;
                    }
                    if (isCorrect[1]) {
                        if (result % 100 == 0) {
                            for (int i = 0; i < 9; i++)
                                if (strcmp(words[2], dozens[i]) == 0) {
                                    result += (i + 1) * 10;
                                    isCorrect[2] = 1;
                                }
                            if (!isCorrect[2]) {
                                for (int i = 0; i < 9; i++)
                                    if (strcmp(words[2], tens[i]) == 0) {
                                        result += 11 + i;
                                        isCorrect[2] = 1;
                                    }
                            }
                        }
                        if (!isCorrect[2]) {
                            for (int i = 0; i < 9; i++)
                                if (strcmp(words[2], smallNumbers[i]) == 0) {
                                    result += i + 1;
                                    isCorrect[2] = 1;
                                    break;
                                }
                            if (!isCorrect[2]) result = -1;
                        }
                    }
                }
            }
            //---------------------------------------END-OF-NATURAL-NUMBER-CHECK----------------------------------------
            break;
        case 4:
            isCorrect = malloc(4 * sizeof(int));
            isCorrect[0] = 0;
            isCorrect[1] = 0;
            isCorrect[2] = 0;
            isCorrect[3] = 0;

            hasLastBase = 0;
            multiplier = -1;
            //-------------------------------LAST-BASE-CHECK------------------------------------------------------------
            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 3; j++)
                    if(strcmp(words[wordsCount - 1], bases[i][j]) == 0) {
                        hasLastBase = 1;
                        int mTextLength = strlen(words[0]) + strlen(words[1]) + strlen(words[2]) + 2;
                        char * multiplierText = malloc(mTextLength + 1);
                        strncpy(multiplierText, text, mTextLength);
                        multiplierText[mTextLength] = '\0';
                        multiplier = textToNumber(multiplierText, 1);
                        free(multiplierText);
                    }
            if(hasLastBase && multiplier != -1) {
                switch ((multiplier % 10 < 10) ? multiplier % 10 : 5) {
                    case 1:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][0]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 2 ... 4:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][1]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 0:
                    case 5 ... 9:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][2]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    default: result = -1;
                }
            }
            //------------------------------------END-OF-LAST-BASE-CHECK------------------------------------------------
            //----------------------------------------IN-BASE-CHECK-----------------------------------------------------
            if(!isCorrect[0])
                for (int cursor = 0; cursor < wordsCount; cursor++)
                    for (int i = 0; i < 4; i++)
                        for (int j = 0; j < 3; j++)
                            if (strcmp(words[cursor], bases[i][j]) == 0) {
                                int mFirstPartLength =
                                        ((cursor >= 0) ? strlen(words[0]) : 0) +
                                        ((cursor >= 1) ? strlen(words[1]) : 0) +
                                        ((cursor >= 2) ? strlen(words[2]) : 0) + (cursor);
                                int mSecondPartLength = strlen(text) - mFirstPartLength - 1;
                                char *firstPart = malloc((mFirstPartLength + 1));
                                char *secondPart = malloc((mSecondPartLength + 1));
                                strncpy(firstPart, text, mFirstPartLength);
                                strncpy(secondPart, text + mFirstPartLength + 1, mSecondPartLength);
                                firstPart[mFirstPartLength] = '\0';
                                secondPart[mSecondPartLength] = '\0';
                                int first = textToNumber(firstPart, 1);
                                int second = textToNumber(secondPart, 1);
                                result = (first != -1 && second != -1) ? first + second : -1;
                                free(firstPart);
                                free(secondPart);
                                isCorrect[0] = 1;
                                break;
                            }
            //----------------------------------END-OF-IN-BASE-CHECK----------------------------------------------------
            //---------------------------------------NATURAL-NUMBER-CHECK-----------------------------------------------
            if (!isCorrect[0]) {
                if (strcmp(words[0], bases[3][0]) == 0) {
                    result += 1000;
                    isCorrect[0] = 1;
                }
                if (isCorrect[0]) {
                    for(int i = 0; i < 9; i++)
                        if (strcmp(words[1], hundreds[i]) == 0) {
                            result += (i + 1) * 100;
                            isCorrect[1] = 1;
                        }
                    if (isCorrect[1]) {
                        for(int i = 0; i < 9; i++)
                            if (strcmp(words[2], dozens[i]) == 0) {
                                result += (i + 1) * 10;
                                isCorrect[2] = 1;
                            }
                        if(isCorrect[2]) {
                            for(int i = 0; i < 9; i++)
                                if (strcmp(words[3], smallNumbers[i]) == 0) {
                                    result += i + 1;
                                    isCorrect[3] = 1;
                                }
                            if(!isCorrect[3]) result = -1;
                        }
                    }
                } else result = -1;
            }
            //---------------------------------------END-OF-NATURAL-NUMBER-CHECK----------------------------------------
            break;
        case 5:
            isCorrect = malloc(5 * sizeof(int));
            isCorrect[0] = 0;
            isCorrect[1] = 0;
            isCorrect[2] = 0;
            isCorrect[3] = 0;
            isCorrect[4] = 0;

            hasLastBase = 0;
            multiplier = -1;
            //-------------------------------LAST-BASE-CHECK------------------------------------------------------------
            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 3; j++)
                    if(strcmp(words[wordsCount - 1], bases[i][j]) == 0) {
                        hasLastBase = 1;
                        int mTextLength = strlen(words[0]) + strlen(words[1]) + strlen(words[2]) + strlen(words[3]) + 3;
                        char * multiplierText = malloc(mTextLength + 1);
                        strncpy(multiplierText, text, mTextLength);
                        multiplierText[mTextLength] = '\0';
                        multiplier = textToNumber(multiplierText, 1);
                        free(multiplierText);
                    }
            if(hasLastBase && multiplier != -1) {
                switch ((multiplier % 10 < 10) ? multiplier % 10 : 5) {
                    case 1:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][0]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 2 ... 4:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][1]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    case 0:
                    case 5 ... 9:
                        for(int i = 0; i < 4; i++)
                            if(strcmp(words[wordsCount - 1], bases[i][2]) == 0) {
                                result += multiplier * pow(10, i);
                                isCorrect[0] = 1;
                                break;
                            }
                        break;
                    default: result = -1;
                }
            }
            //------------------------------------END-OF-LAST-BASE-CHECK------------------------------------------------
            //----------------------------------------IN-BASE-CHECK-----------------------------------------------------
            if(!isCorrect[0])
                for (int cursor = 0; cursor < wordsCount; cursor++)
                    for (int i = 0; i < 4; i++)
                        for (int j = 0; j < 3; j++)
                            if (strcmp(words[cursor], bases[i][j]) == 0) {
                                int mFirstPartLength =
                                        ((cursor >= 0) ? strlen(words[0]) : 0) +
                                        ((cursor >= 1) ? strlen(words[1]) : 0) +
                                        ((cursor >= 2) ? strlen(words[2]) : 0) +
                                        ((cursor >= 3) ? strlen(words[3]) : 0) + (cursor);
                                int mSecondPartLength = strlen(text) - mFirstPartLength - 1;
                                char *firstPart = malloc((mFirstPartLength + 1));
                                char *secondPart = malloc((mSecondPartLength + 1));
                                strncpy(firstPart, text, mFirstPartLength);
                                strncpy(secondPart, text + mFirstPartLength + 1, mSecondPartLength);
                                firstPart[mFirstPartLength] = '\0';
                                secondPart[mSecondPartLength] = '\0';
                                int first = textToNumber(firstPart, 1);
                                int second = textToNumber(secondPart, 1);
                                result = (first != -1 && second != -1) ? first + second : -1;
                                free(firstPart);
                                free(secondPart);
                                isCorrect[0] = 1;
                                break;
                            }
            //----------------------------------END-OF-IN-BASE-CHECK----------------------------------------------------
            //---------------------------------------NATURAL-NUMBER-CHECK-----------------------------------------------
            if (!isCorrect[0]) {
                if (strcmp(words[0], bases[3][0]) == 0) {
                    result += 1000;
                    isCorrect[0] = 1;
                }
                if (isCorrect[0]) {
                    for(int i = 0; i < 9; i++)
                        if (strcmp(words[1], hundreds[i]) == 0) {
                            result += (i + 1) * 100;
                            isCorrect[1] = 1;
                        }
                    if (isCorrect[1]) {
                        for(int i = 0; i < 9; i++)
                            if (strcmp(words[2], dozens[i]) == 0) {
                                result += (i + 1) * 10;
                                isCorrect[2] = 1;
                            }
                        if(isCorrect[2]) {
                            for(int i = 0; i < 9; i++)
                                if (strcmp(words[3], smallNumbers[i]) == 0) {
                                    result += i + 1;
                                    isCorrect[3] = 1;
                                }
                            if(!isCorrect[3]) result = -1;
                        }
                    }
                } else result = -1;
            }
            //---------------------------------------END-OF-NATURAL-NUMBER-CHECK----------------------------------------
            break;
        default: return -1;
    }
    //----------------------------------------MEMORY-CLEAR--------------------------------------------------------------
    free(isCorrect);
    for(int i = 0; i < wordsCount; i++) free(words[i]);
    free(words);
    return result;
}

int getNearestLiteral(int number)
{
    int min = 0xFFFFF;
    int index = 0;

    for(int i = 0; i < literalArrayLength; i++) {
        if(abs(literalNumbers[i] - number) < min) {
            min = abs(literalNumbers[i] - number);
            index = i;
        }
    }
    return index;
}

char * NumberToRoman(int number)
{
    int nearestIndex = getNearestLiteral(number);

    if(number == 0) {
        char * string = malloc(1);
        string[0] = '\0';
        return string;
    }

    if(literalNumbers[nearestIndex] == number) {
        char * string = malloc(2);
        string[0] = literals[nearestIndex];
        string[1] = '\0';
        return string;
    }

    int delta = literalNumbers[nearestIndex] - number;
    if(delta > 0) {

        if(delta == 200 || delta == 20 || delta == 2)
        {
            delta += delta / 2;
            nearestIndex--;
        } else {

            //надо уменьшать
            char * decreaseNumber = NumberToRoman(abs(delta));
            int length = strlen(decreaseNumber);
            char * buffer = malloc(length + 2);

            memcpy(buffer, decreaseNumber, length);
            buffer[length] = literals[nearestIndex];
            buffer[length + 1] = '\0';

            free(decreaseNumber);

            return buffer;
        }
    }

    //надо увеличевать
    char * incNumber = NumberToRoman(abs(delta));
    int length = strlen(incNumber);
    char * buffer = malloc(length + 2);

    buffer[0] = literals[nearestIndex];
    memcpy(buffer + 1, incNumber, length);
    buffer[length + 1] = '\0';

    free(incNumber);

    return buffer;
}

char * getRimean(int n) {
    if(n >= 4000) return "Result is unreachable";
    int numbers[6];
    int offset = 0;
    int radix = 0;
    char * result = malloc(200);

    while(n != 0) {
        numbers[radix++] = n % 10;
        n /= 10;
    }

    for(int i = radix - 1; i >= 0; i--) {
        int number = numbers[i] * pow(10, i);
        char * romanNumber = NumberToRoman(number);

        memcpy(result + offset, romanNumber, strlen(romanNumber));
        offset += strlen(romanNumber);

        free(romanNumber);
    }

    result[offset] = '\0';
    return result;
}

char* processTextAndArabicNumber(char* text, int iNumber) {
    char * buff = malloc(maxBufferLength);
    int number = textToNumber(text, 0);
    if(number == iNumber) sprintf(buff, "Answer: \n%s", getRimean(number));
    else {
        if(number == -1) sprintf(buff, "Gramatic error has been found");
        else {
            sprintf(buff, "Числа %d и %d не равны!", iNumber, number);
        }
    }
    return buff;
}

int win_width;
int win_height;

void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    win_width =  w.ws_col;
    win_height = w.ws_row;
}

void pToXY(int x, int y, char * text) {
    printf("\n\033[%d;%dH%s", y, x, text);
}

void clear() {
    printf("\033[H\033[J");
}

int getch(void) {
    int c = 0;
    struct termios org_opts, new_opts;
    int res = 0;
    res = tcgetattr(STDIN_FILENO, &org_opts);
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c = getchar();
    res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    return(c);
}

void get_string_size(char * str, int * w, int * h) {
    int _w = -1, _h = 1;
    int m = 0;
    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == '\n') {
            _h++;
            if(m > _w) {
                _w = m;
                m = 0;
            }

        } else m++;
    }
    if(_w == -1) _w = m;
    (*w) = _w;
    (*h) = _h;
}

typedef struct {
    char * label;

    void (*onClick)(void);
} window_button_t;

typedef struct {
    int maxLen;

    int bufferLen;

    char * text;

    void (*callbackFn)(char);

    int inputType;
} window_inputfield_t;

typedef struct {
    char * caption;

    char * description;

    int dataCount;

    int * types;

    void ** data;
} window_content_t;

window_content_t * _selected_window;

int focusedElement;

window_inputfield_t * initInptField(char * defaultValue, void (*callbackFn)(char), int maxLen, int inputType) {
    window_inputfield_t * field = malloc(sizeof(window_inputfield_t));

    char * nstr = malloc(strlen(defaultValue) + 1);
    strcpy(nstr, defaultValue);

    field->text = nstr;
    field->callbackFn = callbackFn;
    field->maxLen = maxLen;
    field->bufferLen = (inputType == 2 ? 2*maxLen : maxLen);
    field->inputType = inputType;
    return field;
}

window_button_t * initBtn(char * label, void (*fn)(void)) {
    window_button_t * btn = malloc(sizeof(window_button_t));
    btn->label = label;
    btn->onClick = fn;
    return btn;
}

window_content_t * initWindow(char* caption, char* description, int typesc, ...)
{
    va_list argp;
    va_start(argp, typesc);

    window_content_t * win;
    win = malloc(sizeof(window_content_t));
    win->caption = caption;
    win->dataCount = typesc;
    win->description = description;
    win->types = malloc(typesc * sizeof(int));
    win->data = malloc(typesc * sizeof(void *));

    for(int i = 0; i < typesc; i++) win->types[i] = va_arg(argp, int);
    for(int i = 0; i < typesc; i++) win->data[i] = va_arg(argp, void *);

    va_end(argp);

    return win;
}

void takeString(char * string, int s, int e, int hww, int * currY, int offset) {
    char * nstr = malloc(e - s + 2);
    memcpy(nstr, string + s, e + 1);
    nstr[e - s + 1] = '\0';
    printCenteredStr(nstr, strlen(nstr),  hww, *currY, offset);
    free(nstr);
}

void seperateString(char * string, int hww, int * currY, int offset)
{
    int s = 0, e = 0;
    for(int i = 0; i < strlen(string); i++) {
        if(string[i] == '\n') {
            e = i - 1;
            takeString(string, s, e, hww, currY, 1);
            s = i + 1;
        }
    }
    takeString(string, s, strlen(string), hww, currY, offset);
}

void draw_window(window_content_t * content) {

    const int xOffset = 0;
    const int yOffset = -1;

    const int hww = win_width / 2;
    const int hwh = win_height / 2;

    int width = 20;
    int height = 4;

    int w, h;

    recalculateStrSize(content->caption, height, width, h, w);
    recalculateStrSize(content->description, height, width, h, w);
    for(int i = 0 ; i < content->dataCount; i++) {
        if(content->types[i] == wt_button) {
            window_button_t * btn_ptr = (window_button_t*)content->data[i];
            recalculateStrSize(btn_ptr->label, height, width, h, w);
        } else if(content->types[i] == wt_inputfield) {
            window_inputfield_t * field_ptr = (window_inputfield_t*)content->data[i];
            height += 2;
            if(field_ptr->bufferLen + 4 > width) width = field_ptr->bufferLen + 4;
        }
    }

    pToXY(hww - width / 2, hwh - height / 2 + yOffset, pg_winborder_angle_1);
    for(int x = hww - width / 2 + 1; x < hww + width / 2; x++) {
        pToXY(x, hwh - height / 2 + yOffset, pg_winborder_hor);
    }
    pToXY(hww + width / 2, hwh - height / 2 + yOffset, pg_winborder_angle_3);

    for(int y = hwh - height / 2 + 1; y < hwh + height / 2; y++) {
        pToXY(hww - width / 2, y + yOffset, pg_winborder_vert);
        pToXY(hww + width / 2, y + yOffset, pg_winborder_vert);
    }

    pToXY(hww - width / 2, hwh + height / 2 + yOffset, pg_winborder_angle_2);
    for(int x = hww - width / 2 + 1; x < hww + width / 2; x++) {
        pToXY(x, hwh + height / 2 + yOffset, pg_winborder_hor);
    }
    pToXY(hww + width / 2, hwh + height / 2 + yOffset, pg_winborder_angle_4);

    int currY = hwh - height / 2;

    printCenteredStr(content->caption, strlen(content->caption), hww, currY, 2);

    seperateString(content->description, hww, &currY, 2);

    for(int i = 0 ; i < content->dataCount; i++) {
        if(content->types[i] == wt_button) {
            window_button_t * btn_ptr = (window_button_t*)content->data[i];

            if(focusedElement == i) {

                int length = strlen(btn_ptr->label);
                char * newStr = malloc(length + 1 + 2);
                newStr[0] = pg_selected_l;
                memcpy(newStr + 1, btn_ptr->label, length);
                newStr[length + 1] = pg_selected_r;
                newStr[length + 2] = '\0';

                printCenteredStr(newStr, strlen(newStr), hww, currY, 1);

                free(newStr);

            } else {    
                printCenteredStr(btn_ptr->label, strlen(btn_ptr->label), hww, currY, 1);
            }
        } else if(content->types[i] == wt_inputfield) {
            window_inputfield_t * field_ptr = (window_inputfield_t * )content->data[i];
            if(field_ptr->inputType == 2) {
                int length = (field_ptr->bufferLen + strlen(field_ptr->text))/2;
                char * newStr = malloc(length + 3);
                //memset(newStr + 1, '_', length);
                strncpy(newStr + 1, field_ptr->text, strlen(field_ptr->text));
                int toInsert =  length - strlen(field_ptr->text)/(field_ptr->inputType == 2? 2 : 1);
                memset(newStr + 1 + strlen(field_ptr->text), '_', toInsert);
                if(focusedElement == i) {
                    newStr[0] = pg_selected_l;
                    newStr[length + 1] = pg_selected_r;
                } else {
                    newStr[0] = '|';
                    newStr[length + 1] = '|';
                }
                newStr[length + 2] = '\0';
                printCenteredStr(newStr, field_ptr->maxLen, hww, currY, 1);
                free(newStr);
            } else {
                    int length = field_ptr->bufferLen;
                char * newStr = malloc(length + 3);
                //memset(newStr + 1, '_', length);
                strncpy(newStr + 1, field_ptr->text, strlen(field_ptr->text));
                int toInsert =  length - strlen(field_ptr->text)/(field_ptr->inputType == 2? 2 : 1);
                memset(newStr + 1 + strlen(field_ptr->text), '_', toInsert);
                if(focusedElement == i) {
                    newStr[0] = pg_selected_l;
                    newStr[length + 1] = pg_selected_r;
                } else {
                    newStr[0] = '|';
                    newStr[length + 1] = '|';
                }
                newStr[length + 2] = '\0';
                printCenteredStr(newStr, field_ptr->maxLen, hww, currY, 1);
                free(newStr);
            }
            
        }
    }
}

void draw_border()
{
    pToXY(1, 0, pg_border_angle_1);
    for(int x = 2; x < win_width; x++) {
        pToXY(x, 0, pg_border_hor);
    }
    pToXY(win_width, 0, pg_border_angle_3);
    for(int y = 2; y < win_height; y++) {
        pToXY(1, y, pg_border_vert);
        pToXY(win_width, y, pg_border_vert);
    }
    pToXY(1, win_height - 1, pg_border_angle_2);
    for(int x = 2; x < win_width; x++) {
        pToXY(x, win_height - 1, pg_border_hor);
    }
    pToXY(win_width, win_height - 1, pg_border_angle_4);
}

void redrawScene() {
    clear();
    draw_border();
    draw_window(_selected_window);
}

void inputHandler(char * c) {
    if(_selected_window->types[focusedElement] == wt_inputfield) {
        window_inputfield_t * field_prt = _selected_window->data[focusedElement];

        int l = (field_prt->inputType == 2 ? strlen(field_prt->text)/2 : strlen(field_prt->text));
        if(c[0] == 127) {
            if(l > 0) {
                switch (field_prt->inputType) {
                    case 2:
                        field_prt->text[2 * l - 2] = '\0';
                        break;
                    default: 
                        field_prt->text[l - 1] = '\0';
                        break;
                }
            }

        } else {
            switch (field_prt->inputType) {
                case 0:
                    if(c[0] >= '0' && c[0] <= '9') {
                        if(l + 1 <= field_prt->maxLen) {    
                            field_prt->text = realloc(field_prt->text, l + 2);
                            field_prt->text[l] = *c;
                            field_prt->text[l + 1] = '\0';
                        }
                    }
                    break;
                case 1:
                    if(c[0] >= 'a' && c[0] <= 'z') {
                        if(l + 1 <= field_prt->maxLen) {    
                            field_prt->text = realloc(field_prt->text, l + 2);
                            field_prt->text[l] = *c;
                            field_prt->text[l + 1] = '\0';
                        }
                    }
                    break;
                case 2:
                    if(l + 1 <= field_prt->maxLen) {
                        field_prt->text = realloc(field_prt->text, 2 * l + 2);
                        strncpy(field_prt->text + 2 * l, c, strlen(c));
                        field_prt->text[2 * l + strlen(c)] = '\0';
                    }
                    break;
                default:
                    break;
            }

        }
    }
}

void enterHandler() {
    if(_selected_window->types[focusedElement] == wt_button) {
        window_button_t * btn_prt = _selected_window->data[focusedElement];
        btn_prt->onClick();
    }
}

void arrowHandler(int c) {
    if(c == 66) {
        focusedElement++;
        if(focusedElement == _selected_window->dataCount) {
            focusedElement = 0;
        }
    } else if(c == 65) {
        focusedElement--;
        if(focusedElement < 0) {
            focusedElement = _selected_window->dataCount - 1;
        }
    }
}

void handleEvents() {
    int a = getch();
    char * buff;
    switch (a) {
        case 27:
            if(getch() == 91) arrowHandler(getch());
            break;
        case 10:
            enterHandler();
            break;
        case 127:
            1;
            buff = malloc(2);
            buff[0] = (char)127;
            buff[1] = '\0';
            inputHandler(buff);
            free(buff);
            break;
        case 48 ... 57:
            1;
            buff = malloc(2);
            buff[0] = (char)a;
            buff[1] = '\0';
            inputHandler(buff);
            free(buff);
            break;
        case 208:
            switch(getch()) {
                case 176: inputHandler("а"); break;
                case 177: inputHandler("б"); break;
                case 178: inputHandler("в"); break;
                case 179: inputHandler("г"); break;
                case 180: inputHandler("д"); break;
                case 181: inputHandler("е"); break;
                case 182: inputHandler("ж"); break;
                case 183: inputHandler("з"); break;
                case 184: inputHandler("и"); break;
                case 185: inputHandler("й"); break;
                case 186: inputHandler("к"); break;
                case 187: inputHandler("л"); break;
                case 188: inputHandler("м"); break;
                case 189: inputHandler("н"); break;
                case 190: inputHandler("о"); break;
                case 191: inputHandler("п"); break;
                default: break;
            }
            break;
        case 209:
            switch(getch()) {
                case 128: inputHandler("р"); break;
                case 129: inputHandler("с"); break;
                case 130: inputHandler("т"); break;
                case 131: inputHandler("у"); break;
                case 132: inputHandler("ф"); break;
                case 133: inputHandler("х"); break;
                case 134: inputHandler("ц"); break;
                case 135: inputHandler("ч"); break;
                case 136: inputHandler("ш"); break;
                case 137: inputHandler("щ"); break;
                case 138: inputHandler("ъ"); break;
                case 139: inputHandler("ы"); break;
                case 140: inputHandler("ь"); break;
                case 141: inputHandler("э"); break;
                case 142: inputHandler("ю"); break;
                case 143: inputHandler("я"); break;
                case 145: inputHandler("ё"); break;
                default: break;
            }
        default: break;
    }
}

window_content_t * main_window;
window_content_t * info_window;
window_content_t * input_window;
window_content_t * result_window;
window_content_t * error_window;

window_inputfield_t * m_input_field_1;
window_inputfield_t * m_input_field_2;

void btnact_start() { switchWindow(input_window); }

void btnact_info() { switchWindow(info_window); }

void btnact_exit() { exit(0); }

void btnact_back() { switchWindow(main_window) }

void btnact_run() {
    for(int i = 0; i < strlen(m_input_field_1->text); i++) {
        if(m_input_field_1->text[i] < '0' ||
           m_input_field_1->text[i] > '9') {

            error_window->description = "Incorrect number\nNumber can contain ONLY chars\n from '0' to '9'";
            switchWindow(error_window);
            return;
        }
    }

    int arabicNumber = atoi(m_input_field_1->text);
    if(arabicNumber == 0 || arabicNumber > 10000) {
        error_window->description = "Number is too big\nor equals zero";
        switchWindow(error_window);
        return;
    }

    if(result_window->description != NULL) {
        result_window->description = realloc(result_window->description, 10000);
    } else {
        result_window->description = malloc(10000);
    }


    char * result = processTextAndArabicNumber(m_input_field_2->text, arabicNumber);
    strcpy(result_window->description, result);
    free(result);
    switchWindow(result_window);
}
void field_callback(char c) {
}

//Функція створення \ ініціалізації вікон
void init_windows() {
    m_input_field_1 = initInptField("10", field_callback, 10, 0);

    m_input_field_2 = initInptField("", field_callback, 50, 2);

    main_window = initWindow("DKR №1", "Press any button below", 3, wt_button, wt_button, wt_button,
                             initBtn("Start", btnact_start), initBtn("Info", btnact_info), initBtn("Exit", btnact_exit));

    info_window = initWindow("Info", "Author:\nDalechyn Vladislav\nFAM KV-81, 2018\nDKR №1", 1, wt_button,
                             initBtn("Back", btnact_back));

    input_window = initWindow("Algorithm", "Enter number as a WORD\n and in ARABIC word\nand press\"Launch\"", 4,
            wt_inputfield, wt_inputfield, wt_button, wt_button, m_input_field_1, m_input_field_2,
            initBtn("Run", btnact_run), initBtn("Back", btnact_back));

    error_window = initWindow("Input error", "", 1, wt_button,
                              initBtn("Back", btnact_start));

    result_window = initWindow("Done", NULL, 2, wt_button, wt_button,
                               initBtn("Again", btnact_start), initBtn("To Menu", btnact_back));
    switchWindow(main_window);
}

int main(int argc, char *argv[]) {
    get_terminal_size();
    init_windows();
    redrawScene();

    while(1) {
        pToXY(win_width, win_height - 1, "\n");
        //Считуємо дії користувача
        handleEvents();

        //І після пересовуємо екран
        redrawScene();
    }
    }