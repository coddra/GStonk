#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include "objects.h"

extern const dgnDscr DGNS[DGNCOUNT];

void addDgnMultiLoc(context* c, DGNKIND desc, loc loc, list(charPtr) params);
void addDgnLoc(context* c, DGNKIND desc, loc loc, char* param);
void addDgnMulti(context* c, DGNKIND desc, list(charPtr) params);
void addDgnEmpty(context* c, DGNKIND desc);
void addDgnEmptyLoc(context* c, DGNKIND desc, loc loc);
void addDgn(context* c, DGNKIND desc, char* param);
string dgnToString(dgn d);
void printDgns(context* c);
bool checkErr(context* c);

#endif // DIAGNOSTICS_H
