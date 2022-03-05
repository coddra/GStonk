#ifndef COMPILING_H
#define COMPILING_H
#include "parsing.h"

string compOP(context* c, opc* op, u f);
string compFun(context* c, u f);
string compGlb(context* c, u g);
string compStr(context* c, u s);
string compile(context* c);

#endif //COMPILING_H
