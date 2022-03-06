#ifndef COMPILER_H
#define COMPILER_H
#include "objects.h"

string compOP(context* c, opc* op, u f);
string compFun(context* c, u f);
string compGlb(context* c, u g);
string compStr(context* c, u s);
string compile(context* c);

#endif //COMPILER_H
