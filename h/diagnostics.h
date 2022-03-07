#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include "objects.h"

extern const diagnDescr DIAGNOSTICS[DIAGNCOUNT];

void addDgnMultiLoc(context* c, DIAGNKIND desc, loc loc, list(charPtr) params);
void addDgnLoc(context* c, DIAGNKIND desc, loc loc, char* param);
void addDgnMulti(context* c, DIAGNKIND desc, list(charPtr) params);
void addDgnEmpty(context* c, DIAGNKIND desc);
void addDgnEmptyLoc(context* c, DIAGNKIND desc, loc loc);
void addDgn(context* c, DIAGNKIND desc, char* param);
string diagnToString(diagn d);
void printDgns(context* c);
bool checkErr(context* c);

#endif // DIAGNOSTICS_H
