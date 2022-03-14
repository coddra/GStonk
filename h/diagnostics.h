#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include "objects.h"

extern const dgnDscr DGNS[DGNCOUNT];

void addDgnLoc(context* c, DGNKIND kind, loc loc, char* prm);
void addDgnEmpty(context* c, DGNKIND kind);
void addDgnEmptyLoc(context* c, DGNKIND kind, loc loc);
void addDgn(context* c, DGNKIND kind, char* prm);
string dgnToString(context* c, u d);
LVL highestLVL(context* c);
void printDgns(context* c);

#endif // DIAGNOSTICS_H
