#ifndef OBJECTS_H
#define OBJECTS_H
#include "../MCX/mcx.h"

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
    FFILE   = 1 << KSTR,
    FANY    = FFUN | FTYP | FGLB | FFLD | FLOC | FARG | FINT | FUINT | FDOUB | FSTR,
    FATTANY = FFUN | FTYP | FGLB | FINT | FUINT | FDOUB | FSTR,
} AFLAG;//argument flag
typedef enum {
    FNOFLAGS      = 0,
    FDEFINED      = 1 << 0,
    FGENERIC      = 1 << 1,
    FIMMEDIATE    = 1 << 2,
    FLINKED       = 1 << 3,
    FPARSED       = 1 << 4,
    FIGNOREMSGS   = 1 << 5,
    FIGNOREWRNGS  = FIGNOREMSGS | (1 << 6),
    FHASMAIN      = 1 << 7,
    FREFERENCED   = 1 << 8,
    FSINGLE       = 1 << 9,
    FARGCRETC     = 1 << 10,
    FHASBODY      = 1 << 11,
    FASM          = 1 << 12,
    FGDB          = FASM | (1 << 13),
    FSTOPS        = 1 << 14,
    FEXPORTALL    = 1 << 15,
    FSO           = 1 << 16,
    FFLYCHECK     = FSO | (1 << 17),
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

    OPIF,     OPWHILE,  OPTRY,//incomplete
    OPTHROW,

    OPEVAL,//incomplete

    OPDROP,   OPCDROP,
    OPDUP,    OPSWAP,   OPROTL,   OPROTR,   OPOVER,

    OPFLAGS,

    OPPRINT,  OPMALLOC, OPFREE,   OPREALLOC, OPEXIT,

    OPCOUNT,//number of opcodes
    OPBUILTIN = OPPRINT,//uint, will be used on handling bytecode
} OP;//operation codes
typedef enum {
    ATTSIGNED,
    ATTMAIN,
    ATTUSE,
    ATTEXPORT,
    ATTCOUNT,
} ATTKIND;
typedef enum {
    EMISSINGTOKEN,
    EMISSINGSYNTAX,
    EUNRECESCSEQ,
    ESECONDDECLARATION,
    EARGOUTOFRANGE,
    EUNRECTOKEN,
    EDEFNOTFOUND,
    EWRONGNUMOFPARAMS,
    EWRONGNUMOFARGS,
    EMULTIMAIN,
    ENOINPUT,
    EWRONGSIZE,
    ESTACKLOW,
    EUNRECATT,
    ESINGLEATT,
    ESTACKUNPRED,
    EFILENOTEXIST,
    ENOPAR,
    EWRONGPAR,
    EGCCFAILED,
    EWRONGTARGET,
    ENOTSTOPS,
    EPATHILLEGAL,

    WNOSIZE,
    WSTACKHIGH,
    WUNREACHCODE,
    WMULTIOUTPUT,

    MTOKENOMITTABLE,
    MMULTIFILE,
    MNOTREFERENCED,
    MSUCCESS,
    MUNRECFLAG,

    DGNCOUNT,
} DGNKIND;

typedef struct loc_s {
    u cr;
    u ln;
    u cl;
    u file;
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

typedef struct att_s {
    loc       loc;
    ATTKIND   kind;
    par       par;
    FLAGS     flags;
} att;//attribute

#define opcDerive                               \
    OP op;                                      \
    loc loc
typedef struct opc_s {
    opcDerive;
} opc;//operation code
typedef opc* opcPtr;
listDeclare(opcPtr);
listDeclareVaList(opcPtr);
typedef struct body_s {
    loc          loc;
    string       text;
    list(opcPtr) ops;
    FLAGS        flags;
    i64          retc;
} body;
typedef struct popc_s {
    opcDerive;
    par par;
    par par2;
    u16 argc;
    u16 retc;
} popc;//operation code with compile-time parameter
typedef struct bopc_s {
    opcDerive;
    body head;
    body body;
    body els;
} bopc;//operation code with body

struct context_s;
typedef struct context_s context;
typedef struct opcDef_s {
    char* code;
    char* alias;
    u8 argc;
    u8 retc;
    void* comp;
    void (*link)(context* c, opc* o, u f, i64* s);
    FLAGS flags;
    AFLAG arg;
} opcDef;//operation code definition
typedef struct attDef_s {
    char* name;
    AFLAG arg;
    FLAGS flags;
    AFLAG trgt;
} attDef;

listDeclare(ref);
typedef struct name_s {
    string sign;
    string csign;
    loc    loc;
} name;
listDeclare(att);
#define defDerive                               \
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
    body         body;
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

typedef struct dgnDscr_s {
    char* id;
    char* msg;
    LVL lvl;
} dgnDscr;//diagnostic descriptor
typedef char* charPtr;
typedef struct dgn_s {
    DGNKIND kind;
    loc     loc;
    char*   prm;
} dgn;//diagnostic

listDeclare(typDef);
listDeclare(funDef);
listDeclare(dgn);
listDeclareEquals(string);
typedef struct context_s {
    string       bin;
    string       text;
    loc          loc;
    u64          addr;
    list(funDef) funs;
    list(typDef) typs;
    list(varDef) glbs;
    list(string) strs;
    list(dgn)    dgns;
    list(u)      ignoreDgns;
    list(att)    atts;
    u            main;
    list(string) inputs;
    string       output;
    FLAGS        flags;
} context;//i don't think i have to explain this


extern const attDef ATTRIBUTES[ATTCOUNT];

#endif//OBJECTS_H
