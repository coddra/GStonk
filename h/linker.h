#ifndef LINKER_H
#define LINKER_H
#include "objects.h"
#include <string.h>

string getCsign(string sign);
char getPostfix(u size);
string getRegister(char name, u size);

bool hasAtt(list(att) atts, ATTKIND kind, att* res);

u getFun(context* c, string sign, bool r);
u getTyp(context* c, string sign, bool r);
u getGlb(context* c, string sign, bool r);
ATTKIND getAtt(string sign);
DGNKIND getDgn(string sign);
u getVar(list(varDef)* l, string sign, bool r);

bool isGOP(context* c, string code, par* pars, OP* op);

void linkBody(context* c, list(opcPtr) b, u f, i64* s);
void link(context* c);

#endif //LINKER_H
