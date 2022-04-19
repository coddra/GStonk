#include "h/diagnostics.h"
#include "h/linker.h"

cDgnDscr ESECONDDECLARATION = { "secondDeclaration", "second declaration of '$' found", LVLERROR };
cDgnDscr EARGOUTOFRANGE = { "argOutOfRange", "$ out of range", LVLERROR };
cDgnDscr EUNRECTOKEN = { "unrecToken", "unrecognized token '$'", LVLERROR };
cDgnDscr EDEFNOTFOUND = { "defNotFound", "definition of '$' not found", LVLERROR };
cDgnDscr EWRONGNUMOFPARAMS = { "wrongNumOfParams", "wrong number of parameters provided for '$'", LVLERROR };
cDgnDscr EWRONGNUMOFARGS = { "wrongNumOfArgs", "wrong number of arguments defined for '$'", LVLERROR };
cDgnDscr EMULTIMAIN = { "multiMain", "multiple functions found with attribute 'main'", LVLERROR };
cDgnDscr ENOINPUT = { "noInput", "no input specified", LVLERROR };
cDgnDscr EWRONGSIZE = { "wrongSize", "size of type must be one of the following: 0, 1, 2, 4 or 8", LVLERROR };
cDgnDscr ESTACKLOW = { "stackLow", "not enough parameters", LVLERROR };
cDgnDscr EUNRECATT = { "unrecAtt", "there is no attribue with name '$'", LVLERROR };
cDgnDscr ESINGLEATT = { "singleAtt", "this attribute can only be attached once", LVLERROR };
cDgnDscr ESTACKUNPRED = { "stackUnpred", "stack unpredictable", LVLERROR };
cDgnDscr EFILENOTEXIST = { "fileNotExist", "file '$' does not exist", LVLERROR };
cDgnDscr ENOPAR = { "noPar", "opcode '$' does not take any arguments", LVLERROR };
cDgnDscr EWRONGPAR = { "wrongPar", "wrong kind of parameter provided for '$'", LVLERROR };
cDgnDscr EGCCFAILED = { "gccFailed", "gcc returned exit status '$'", LVLERROR };
cDgnDscr EWRONGTARGET = { "wrongTarget", "attribute '$' cannot be attached to this kind of object", LVLERROR };
cDgnDscr ENOTSTOPS = { "notStops", "not all codepaths return the controlflow", LVLERROR };
cDgnDscr EPATHILLEGAL = { "pathIllegal", "path '$' is illegal", LVLERROR };

cDgnDscr WNOSIZE = { "noSize", "no size attribute is specified for this type", LVLWARNING };
cDgnDscr WSTACKHIGH = { "stackHigh", "$ value(s) stay on the stack", LVLWARNING };
cDgnDscr WUNREACHCODE = { "unreachableCode", "unreachable code detected", LVLWARNING };
cDgnDscr WMULTIOUTPUT = { "multiOut", "output already specified", LVLWARNING };

cDgnDscr MTOKENOMITTABLE = { "tokenOmit", "'$' is omittable", LVLMESSAGE };
cDgnDscr MMULTIFILE = { "multiFile", "'$' is included multiple times", LVLMESSAGE };
cDgnDscr MNOTREFERENCED = { "notReferenced", "'$' is defined, but never referenced", LVLMESSAGE };
cDgnDscr MSUCCESS = { "success", "executable successfully generated", LVLMESSAGE };
cDgnDscr MUNRECFLAG = { "unrecFlag", "flag '$' is not recognized", LVLMESSAGE };

dgnDscrPtr DGNS[DGNCOUNT] = {
    &EMISSINGTOKEN,
    &EMISSINGSYNTAX,
    &EUNRECESCSEQ,
    &ESECONDDECLARATION,
    &EARGOUTOFRANGE,
    &EUNRECTOKEN,
    &EDEFNOTFOUND,
    &EWRONGNUMOFPARAMS,
    &EWRONGNUMOFARGS,
    &EMULTIMAIN,
    &ENOINPUT,
    &EWRONGSIZE,
    &ESTACKLOW,
    &EUNRECATT,
    &ESINGLEATT,
    &ESTACKUNPRED,
    &EFILENOTEXIST,
    &ENOPAR,
    &EWRONGPAR,
    &EGCCFAILED,
    &EWRONGTARGET,
    &ENOTSTOPS,
    &EPATHILLEGAL,

    &WNOSIZE,
    &WSTACKHIGH,
    &WUNREACHCODE,
    &WMULTIOUTPUT,

    &MTOKENOMITTABLE,
    &MMULTIFILE,
    &MNOTREFERENCED,
    &MSUCCESS,
    &MUNRECFLAG,
};

bool includeDgn(cContext* c, cDgn d, u h) {
    return ((((context*)c)->flags & FFLYCHECK) == FFLYCHECK && d.loc.file == 0) ||
            ((((context*)c)->flags & FFLYCHECK) != FFLYCHECK &&
             ((d.kind->lvl == h || h > LVLERROR) &&
              !((((context*)c)->flags & FIGNOREMSGS) != 0 && d.kind->lvl == LVLMESSAGE ||
                (((context*)c)->flags & FIGNOREWRNGS) != 0 && d.kind->lvl == LVLWARNING ||
                dgnDscrPtrListContains(((context*)c)->ignoreDgns, d.kind)) &&
              (h >= LVLERROR || d.loc.file >= ((context*)c)->inputs.len || !isStd(((context*)c), ((context*)c)->inputs.items[d.loc.file]))));
}
