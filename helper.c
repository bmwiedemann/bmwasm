#include <ctype.h>

/* convert string to lower-case */
void strlwr(char *str)
{
    while(*str) {
        *str=tolower(*str);
        str++;
    }
}
