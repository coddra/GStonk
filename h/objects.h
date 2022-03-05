#ifndef OBJECTS_H
#define OBJECTS_H
#include "shorts.h"
#include <stdio.h>

typedef enum {
    LVLMESSAGE,
    LVLWARNING,
    LVLERROR,
} LVL;//level
typedef enum {
    KNONE,
    KFUN,
    KTYP,
    KGLB,
    KFLD,
    KLOC,
    KARG,
    KINT,
    KUINT,
    KDOUB,
    KSTR,
} AKIND;//argument kind
typedef enum {
    FNONE   = 0,
    FFUN    = 1 << (KFUN - 1),
    FTYP    = 1 << (KTYP - 1),
    FGLB    = 1 << (KGLB - 1),
    FFLD    = 1 << (KFLD - 1),
    FLOC    = 1 << (KLOC - 1),
    FARG    = 1 << (KARG - 1),
    FINT    = 1 << (KINT - 1),
    FUINT   = 1 << (KUINT - 1),
    FDOUB   = 1 << (KDOUB - 1),
    FSTR    = 1 << (KSTR - 1),
} AFLAG;//argument flag
typedef enum {
    FNOFLAGS      = 0,
    FDEFINED      = 1 << 0,
    FGENERIC      = 1 << 1,
    FIMMEDIATE    = 1 << 2,
    FLINKED       = 1 << 3,
    FWARNINGSOFF  = 1 << 4,
    FHASMAIN      = 1 << 5,
    FGDB          = 1 << 6,
    FREFERENCED   = 1 << 7,
    FSINGLE       = 1 << 7,
} FLAGS;//flags
typedef enum {
    OPADD,    OPADDF,   OPINC,
    OPSUB,    OPSUBF,   OPDEC,    OPNEG,    OPNEGF,
    OPMUL,    OPMULS,   OPMULF,
    OPDIV,    OPDIVS,   OPDIVF,
    OPMOD,    OPMODS,   OPMODF,
    OPDM,     OPDMS,

    OPNOT,    OPBNOT,
    OPAND,    OPBAND,
    OPOR,     OPBOR,
    OPXOR,    OPBXOR,

    OPSHL,    OPSHR,    OPSAR,

    OPEQ,     OPEQF,    OPNE,     OPNEF,
    OPLT,     OPLTS,    OPLTF,    OPLE,     OPLES,    OPLEF,
    OPGT,     OPGTS,    OPGTF,    OPGE,     OPGES,    OPGEF,

    OPFTI,    OPITF,

    OPSQRT,

    OPLDAT,//int
    OPSTAT,//uint
    OPCLAT,//incomplete but immediate

    OPRET,//not immediate


    OPLDADDR, OPST,//incomplete

    OPIF,     OPELIF,   OPWHILE,  OPTRY,    OPELSE,//incomplete
    OPTHROW,

    OPEVAL,//incomplete

    OPDROP,   OPCDROP,
    OPDUP,    OPSWAP,   OPROTL,   OPROTR,

    OPFLAGS,  OPVERR,   OPVERW,

    OPBUILTIN,//uint
    OPPRINT,  OPMALLOC, OPFREE,   OPREALLOC, OPEXIT,

    OPCOUNT,//number of opcodes
} OP;//operation codes
typedef enum {
    ATTSIGNED,
    ATTMAIN,
    ATTCOUNT
} ATTKIND;
enum {
    opcType = objectType + 1,
    popcType,
    bopcType,
    defType,
    funDefType,
    typDefType,
    varDefType,
};

typedef struct loc_s {
    u cr;
    u ln;
    u cl;
    string file;
} loc;//location

typedef struct ref_s {
    u i;
    loc loc;
} ref;//function, type, variable or string reference
typedef union upar_u {
    i64 i;
    u64 u;
    d d;
    ref r;
} upar;//parameter union
typedef struct par_s {
    loc loc;
    AKIND kind;
    upar val;
} par;//compiletime parameter of operation or attribute

listDeclare(par);
typedef struct att_s {
    loc       loc;
    ATTKIND   kind;
    list(par) prms;
    FLAGS     flags;
} att;//attribute

#define opcDerive                               \
    u TYPE;                                     \
    OP op;                                      \
    loc loc
typedef struct opc_s {
    opcDerive;
} opc;//operation code
typedef opc* opcPtr;
listDeclare(opcPtr);
listDeclareVaList(opcPtr);
typedef struct popc_s {
    opcDerive;
    par par;
    par par2;
    u16 argc;
    u16 retc;
} popc;//operation code with compile-time parameter
typedef struct bopc_s {
    opcDerive;
    list(opcPtr) head;
    list(opcPtr) body;
    list(opcPtr) body2;
    i64 retc;
    i64 retc2;
} bopc;//operation code with body

typedef struct opcDef_s {
    OP id;
    char* code;
    char* alias;
    u8 argc;
    u8 retc;
    char* comp;
    FLAGS flags;
    AFLAG arg;
} opcDef;//operation code definition
typedef struct attDef_s {
    char* name;
    AFLAG arg;
    FLAGS flags;
} attDef;

listDeclare(ref);
typedef struct name_s {
    string sign;
    string csign;
    loc    loc;
} name;
listDeclare(att);
#define defDerive                               \
    objectDerive;                               \
    name name;                                  \
    list(att) attrs;                            \
    FLAGS  flags
typedef struct def_s {
    defDerive;
} def;//common struct for definitions
typedef struct varDef_s {
    defDerive;
    ref type;
    u offset;
} varDef;//variable definition
listDeclare(varDef);
typedef struct funDef_s {
    defDerive;
    list(opcPtr) body;
    list(varDef) args;
    list(varDef) locs;
    list(ref)    ret;
} funDef;//function definition
typedef struct typDef_s {
    defDerive;
    ref parent;
    u8  size;
    list(varDef) flds;
} typDef;//type definition

typedef struct diagnDescr_s {
    char* id;
    char* msg;
    LVL lvl;
} diagnDescr;//diagnostic descriptor
typedef char* charPtr;
listDeclareEquals(charPtr);
bool charPtrEquals(charPtr, charPtr);
typedef struct diagn_s {
    diagnDescr descr;
    loc loc;
    list(charPtr) params;
} diagn;//diagnostic

listDeclare(typDef);
listDeclare(funDef);
listDeclare(diagn);
listDeclareEquals(string);
typedef struct context_s {
    string text;
    loc loc;
    u64 addr;
    list(funDef) funs;
    list(typDef) typs;
    list(varDef) glbs;
    list(string) strs;
    list(diagn)  dgns;
    list(string) ignorew;
    u main;
    list(string) inputs;
    string output;
    FLAGS flags;
} context;//i don't think i have to explain this

extern const diagnDescr EMISSINGTOKEN;
extern const diagnDescr EMISSINGSYNTAX;
extern const diagnDescr EUNRECESCSEQ;
extern const diagnDescr ESECONDDECLARATION;
extern const diagnDescr EARGOUTOFRANGE;
extern const diagnDescr EUNRECTOKEN;
extern const diagnDescr EDEFNOTFOUND;
extern const diagnDescr EWRONGNUMOFPARAMS;
extern const diagnDescr EWRONGNUMOFARGS;
extern const diagnDescr EMULTIMAIN;
extern const diagnDescr ENOINPUT;
extern const diagnDescr EWRONGSIZE;
extern const diagnDescr ESTACKLOW;
extern const diagnDescr EUNRECATT;
extern const diagnDescr ESINGLEATT;
extern const diagnDescr ESTACKUNPRED;
extern const diagnDescr EFILENOTEXIST;

extern const diagnDescr WNOSIZE;
extern const diagnDescr WSTACKHIGH;
extern const diagnDescr WUNREACHCODE;
extern const diagnDescr WMULTIOUTPUT;

extern const diagnDescr MTOKENOMITTABLE;

extern const attDef ATTRIBUTES[ATTCOUNT];

loc locDefault();
par parDefault();
ref refDefault();
att attDefault();
bopc bopcDefault();
popc popcDefault();
opc opcDefault();
opcDef opcDefDefault();
attDef attDefDefault();
name nameDefault();
def defDefault();
varDef varDefDefault();
funDef funDefDefault();
typDef typDefDefault();
diagnDescr diagnDescrDefault();
diagn diagnDefault();
context contextDefault();

void addDgnMultiLoc(context* c, diagnDescr desc, loc loc, list(charPtr) params);
void addDgnLoc(context* c, diagnDescr desc, loc loc, char* param);
void addDgnMulti(context* c, diagnDescr desc, list(charPtr) params);
void addDgnEmpty(context* c, diagnDescr desc);
void addDgnEmptyLoc(context* c, diagnDescr desc, loc loc);
void addDgn(context* c, diagnDescr desc, char* param);
string diagnToString(diagn d);
void printDgns(context* c);
bool checkErr(context* c);

string codeFrom(context* c, loc o);

#endif//OBJECTS_H
