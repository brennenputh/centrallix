#ifndef _HP_FONT_METRICS_H
#define _HP_FONT_METRICS_H

/** plain, italic, bold, bold+italic; 60ths **/
static double hp_times_font_metrics[][4] =
    {
    {30, 30, 30, 30},	/* space */
    {34, 34, 34, 34},	/* bang */
    {46, 48, 46, 52},	/* dbl quote */
    {50, 50, 50, 50},	/* pound sign */
    {50, 50, 50, 50},	/* dollar sign */
    {88, 88, 88, 88},	/* percent sign */
    {78, 78, 82, 78},	/* & */
    {34, 34, 34, 34},	/* ' */
    {34, 34, 34, 34},	/* ( */
    {34, 34, 34, 34},	/* ) */
    {50, 50, 50, 50},	/* * */
    {88, 88, 88, 88},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},

    {50, 50, 50, 50},	/* 0 through ? */
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {50, 50, 50, 50},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {100, 100, 100, 100},
    {88, 88, 88, 88},
    {100, 100, 100, 100},
    {44, 50, 50, 50},

    {88, 88, 88, 88},	/* @ through O */
    {72, 60, 72, 66},
    {62, 60, 66, 66},
    {68, 66, 72, 66},
    {74, 72, 72, 72},
    {62, 60, 66, 66},
    {60, 58, 60, 60},
    {72, 72, 78, 72},
    {74, 72, 78, 78},
    {34, 34, 38, 38},
    {38, 38, 48, 50},
    {70, 66, 78, 66},
    {62, 56, 66, 60},
    {90, 82, 94, 88},
    {74, 66, 72, 72},
    {72, 72, 78, 72},

    {58, 60, 60, 60},	/* P through _ */
    {72, 72, 78, 70},
    {66, 60, 72, 66},
    {54, 50, 56, 56},
    {62, 56, 66, 60},
    {74, 72, 72, 72},
    {72, 60, 72, 66},
    {94, 82,100, 88},
    {72, 60, 72, 66},
    {72, 56, 72, 60},
    {64, 56, 66, 60},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {50, 50, 50, 50},
    {50, 50, 50, 50},

    {34, 34, 34, 34},	/* ` through o */
    {44, 50, 50, 50},
    {50, 50, 56, 50},
    {44, 44, 44, 44},
    {50, 50, 56, 50},
    {44, 44, 44, 44},
    {34, 28, 36, 36},
    {50, 50, 50, 50},
    {50, 50, 56, 56},
    {28, 28, 28, 28},
    {28, 28, 34, 28},
    {50, 50, 56, 50},
    {28, 28, 28, 28},
    {78, 72, 82, 78},
    {50, 50, 56, 56},
    {50, 50, 50, 50},

    {50, 50, 56, 50},	/* p through ~ */
    {50, 50, 56, 50},
    {36, 38, 44, 38},
    {38, 38, 38, 38},
    {28, 28, 34, 28},
    {50, 50, 56, 56},
    {50, 44, 50, 44},
    {72, 66, 72, 66},
    {50, 44, 50, 50},
    {50, 44, 50, 44},
    {44, 38, 44, 38},
    {44, 44, 50, 50},
    {50, 50, 50, 50},
    {44, 44, 50, 50},
    {100,100,100,100},
    };


/** plain italic bold italic-bold; 60ths of a char-width **/
static double hp_helvetica_font_metrics[][4] =
    {
    {34, 34, 34, 34},	/* ' ' through '/' */
    {34, 34, 34, 34},
    {50, 50, 52, 54},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {100,100,100,100},
    {76, 76, 76, 76},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {62, 62, 62, 62},
    {100,100,100,100},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},

    {62, 62, 62, 62},	/* 0 through ? */
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {62, 62, 62, 62},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {100,100,100,100},
    {100,100,100,100},
    {100,100,100,100},
    {52, 52, 52, 52},

    {100,100,100,100},	/* @ through O */
    {74, 74, 74, 74},
    {62, 62, 62, 62},
    {70, 70, 70, 70},
    {72, 72, 72, 72},
    {58, 58, 58, 58},
    {56, 56, 56, 56},
    {74, 74, 74, 74},
    {72, 72, 72, 72},
    {28, 28, 28, 28},
    {56, 56, 56, 56},
    {66, 66, 66, 66},
    {54, 54, 54, 54},
    {90, 90, 90, 90},
    {72, 72, 72, 72},
    {76, 76, 76, 76},

    {60, 60, 60, 60},	/* P through _ */
    {78, 78, 78, 78},
    {64, 64, 64, 64},
    {64, 64, 64, 64},
    {62, 62, 62, 62},
    {72, 72, 72, 72},
    {72, 72, 72, 72},
    {100,100,100,100},
    {72, 72, 72, 72},
    {68, 68, 68, 68},
    {60, 60, 60, 60},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {34, 34, 34, 34},
    {50, 50, 50, 50},
    {50, 50, 50, 50},

    {34, 34, 34, 34},	/* ` through o */
    {54, 54, 54, 54},
    {58, 58, 58, 58},
    {54, 54, 54, 54},
    {58, 58, 58, 58},	/* d */
    {54, 54, 54, 54},
    {36, 36, 36, 36},
    {58, 58, 58, 58},
    {58, 58, 58, 58},
    {24, 24, 24, 24},
    {24, 24, 24, 24},
    {54, 54, 54, 54},
    {24, 24, 24, 24},
    {86, 86, 86, 86},
    {58, 58, 58, 58},
    {58, 58, 58, 58},

    {58, 58, 58, 58},	/* p through ~ */
    {58, 58, 58, 58},
    {36, 36, 36, 36},
    {50, 50, 50, 50},
    {36, 36, 36, 36},
    {58, 58, 58, 58},
    {56, 56, 56, 56},
    {86, 86, 86, 86},
    {56, 56, 56, 56},
    {56, 56, 56, 56},
    {48, 48, 48, 48},
    {44, 44, 50, 50},
    {50, 50, 50, 50},
    {44, 44, 50, 50},
    {100,100,100,100},
    };

#endif
