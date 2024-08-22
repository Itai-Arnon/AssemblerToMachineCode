

#ifndef M14_UTILS_H
#define M14_UTILS_H

/*colors*/
#define BLACK_COLOR "\033[0m"
#define RED_COLOR "\x1b[38;2;247;84;100m"

#define SPACES  " \t\v\f"

#include "shared.h"








/*check is the rest of the line is filled with unwanted char*/
/*the first unwanted char disqualifies the sentence*/
int isRestOfLineEmpty(char *);

/*find the first occurane of chars in sep[]*/
int findSeperator(char *str, char sep);


int convertToOctal(int num);



/* seperastor char array checks all given amount by __seperators__amont__ */
int is_char_separator(char c, char separators[], int separators_amount);

int extra_char_at_end( char line[], int loc);

/*purposed to count non null terminated functions*/
int nonNullTerminatedLength(char *arr);
/*uses the ENUM CHECK_LEGAL_NAME*/
/*checks chars containing str, alphanum , alpha*/
int checkLegalName(char *str, check_legal_name type);

char *removeColon(char *symbol_name);

void removeFrontalWhitespace(char *buffer, int *pos);

/*strips whitespace in both sides*/
char *strstrip(char *s);

/*counts commas to verify separator struct*/
int countCommas(char *str);

int convertOrCheckStringToNum(char *str, convert_func_t type);

int countNumbersInString( char *str);

int checkQuotes(char *str);

#endif /* M14_UTILS_H*/
