#ifndef LINKING_H
#define LINKING_H
#include "opcodes.h"
#include <string.h>

string getCsign(string sign);
char getPostfix(u size);
string getRegister(char name, u size);

bool hasAtt(list(att) atts, ATTKIND kind, att* res);

u getFun(context* c, string sign, bool r);
u getTyp(context* c, string sign, bool r);
u getGlb(context* c, string sign, bool r);
ATTKIND getAtt(string sign);
u getVar(list(varDef)* l, string sign, bool r);

bool isGOP(string code, OP* op);

void linkAtt(context* c, list(att) atts);
void linkVar(context* c, list(varDef) vars);
void linkTyp(context* c, list(typDef) typs);
void linkTypList(context* c, list(ref) typs);
i64 linkBody(context* c, list(opcPtr) b, u f, i64 s);
void linkFun(context* c, list(funDef) funs);
void link(context* c);

#endif //LINKING_H
