#ifndef COMPILER_H
#define COMPILER_H
#include "objects.h"

string compOP(context* c, opc* op, u f);
string compile(context* c);

#endif //COMPILER_H
