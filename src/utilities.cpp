#include "utilities.h"

String getJsonString(const String &src, const char *key)
{
    String pattern = String("\"") + key + "\":\"";
    int a = src.indexOf(pattern);
    if (a < 0)
        return "";
    a += pattern.length();
    int b = src.indexOf('"', a);
    if (b < 0 || b <= a)
        return "";
    return src.substring(a, b);
}