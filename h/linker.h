#ifndef LINKER_H
#define LINKER_H
#include "objects.h"
#include "../ccom/linker.h"
#include <string.h>

string getCsign(string sign);
char getPostfix(u size);
string getRegister(char name, u size);

bool hasAtt(list(att) atts, ATTKIND kind, att* res);

bool export(context* c, def* d);

u getFun(context* c, string sign, bool r);
u getTyp(context* c, string sign, bool r);
u getGlb(context* c, string sign, bool r);
dgnDscrPtr getDgn(string sign);
ATTKIND getAtt(string sign);
u getVar(list(varDef)* l, string sign, bool r);

AFLAG kindToFlag(AKIND k);
bool isGOP(context* c, string code, par* pars, OP* op);

bool addFile(context* c, string path, loc loc);
bool isStd(context* c, string path);

bool stops(opc* op);

void linkBody(context* c, body* b, u f, i64* s);
void link(context* c);

#endif //LINKER_H
