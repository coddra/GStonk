#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include "objects.h"
#include "../ccom/diagnostics.h"

extern cDgnDscr ESECONDDECLARATION;
extern cDgnDscr EARGOUTOFRANGE;
extern cDgnDscr EUNRECTOKEN;
extern cDgnDscr EDEFNOTFOUND;
extern cDgnDscr EWRONGNUMOFPARAMS;
extern cDgnDscr EWRONGNUMOFARGS;
extern cDgnDscr EMULTIMAIN;
extern cDgnDscr ENOINPUT;
extern cDgnDscr EWRONGSIZE;
extern cDgnDscr ESTACKLOW;
extern cDgnDscr EUNRECATT;
extern cDgnDscr ESINGLEATT;
extern cDgnDscr ESTACKUNPRED;
extern cDgnDscr EFILENOTEXIST;
extern cDgnDscr ENOPAR;
extern cDgnDscr EWRONGPAR;
extern cDgnDscr EGCCFAILED;
extern cDgnDscr EWRONGTARGET;
extern cDgnDscr ENOTSTOPS;
extern cDgnDscr EPATHILLEGAL;

extern cDgnDscr WNOSIZE;
extern cDgnDscr WSTACKHIGH;
extern cDgnDscr WUNREACHCODE;
extern cDgnDscr WMULTIOUTPUT;

extern cDgnDscr MTOKENOMITTABLE;
extern cDgnDscr MMULTIFILE;
extern cDgnDscr MNOTREFERENCED;
extern cDgnDscr MSUCCESS;
extern cDgnDscr MUNRECFLAG;

#define DGNCOUNT 32
extern dgnDscrPtr DGNS[DGNCOUNT];

#endif // DIAGNOSTICS_H
