#include "h/objects.h"

listDefine(par);
listDefine(opcPtr);
listDefine(att);
listDefine(varDef);
listDefine(ref);
listDefine(funDef);
bool charPtrEquals(charPtr left, charPtr right) {
    return strcmp(left, right) == 0;
}
listDefineEquals(charPtr);
listDefine(typDef);
listDefine(diagn);
listDefineEquals(string);

const diagnDescr EMISSINGTOKEN = { "emistok", "missing token '{0}'", LVLERROR };
const diagnDescr EMISSINGSYNTAX = { "emissyn", "missing {0}", LVLERROR };
const diagnDescr EUNRECESCSEQ = { "eunrece", "unrecognized escapesequence '{0}'", LVLERROR };
const diagnDescr ESECONDDECLARATION = { "esecdec", "second declaration of '{0}' found", LVLERROR };
const diagnDescr EARGOUTOFRANGE = { "eargout", "{0} out of range", LVLERROR };
const diagnDescr EUNRECTOKEN = { "eunrect", "unrecognized token '{0}'", LVLERROR };
const diagnDescr EDEFNOTFOUND = { "edefnof", "definition of '{0}' not found", LVLERROR };
const diagnDescr EWRONGNUMOFPARAMS = { "ewrnump", "wrong number of parameters provided for '{0}'", LVLERROR };
const diagnDescr EWRONGNUMOFARGS = { "ewrnuma", "wrong number of arguments defined for '{0}'", LVLERROR };
const diagnDescr EMULTIMAIN = { "emumain", "multiple functions found with attributes 'std.main'", LVLERROR };
const diagnDescr ENOINPUT = { "enoinpt", "no input specified", LVLERROR };
const diagnDescr EWRONGSIZE = { "ewrsize", "size of type must be one of the following: 0, 1, 2, 4 or 8", LVLERROR };
const diagnDescr ESTACKLOW = { "estackl", "not enough arguments", LVLERROR };
const diagnDescr EUNRECATT = { "eunratt", "there is no attribue with name '{0}'", LVLERROR };
const diagnDescr ESINGLEATT = { "esinatt", "this attribute can only be attached once", LVLERROR };
const diagnDescr ESTACKUNPRED = { "estacku", "stack unpredictable", LVLERROR };
const diagnDescr EFILENOTEXIST = { "efnexst", "file '{0}' does not exist", LVLERROR };
const diagnDescr ENOPAR = { "enoparm", "opcode '{0}' does not take any argument", LVLERROR };
const diagnDescr EWRONGPAR = { "ewrparm", "wrong kind of parameter provided for opcode '{0}'", LVLERROR };

const diagnDescr WNOSIZE = { "wnosize", "no size attribute is specified for this type", LVLWARNING };
const diagnDescr WSTACKHIGH = { "wstackh", "{0} value(s) stay on the stack", LVLWARNING };
const diagnDescr WUNREACHCODE = { "wunreco", "unreachable code detected", LVLWARNING };
const diagnDescr WMULTIOUTPUT = { "wmultio", "output already specified", LVLWARNING };

const diagnDescr MTOKENOMITTABLE = { "mtokomi", "'{0}' is omittable", LVLMESSAGE };

const attDef ATTRIBUTES[ATTCOUNT] = {
    { "signed",
        FNONE,
        FSINGLE
    },
    { "main",
        FNONE,
        FSINGLE
    }
};

loc locDefault() {
    loc res = { 0 };
    res.file = stringDefault();
    return res;
}
par parDefault() {
    par res = { 0 };
    res.loc = locDefault();
    return res;
}
ref refDefault() {
    ref res = { 0 };
    res.loc = locDefault();
    return res;
}
att attDefault() {
    att res = { 0 };
    res.loc = locDefault();
    res.prms = parListDefault();
    return res;
}
opc opcDefault() {
    opc res = { 0 };
    res.loc = locDefault();
    res.TYPE = opcType;
    return res;
}
popc popcDefault() {
    popc res = { 0 };
    res.loc = locDefault();
    res.par = parDefault();
    res.par2 = parDefault();
    res.TYPE = popcType;
    return res;
}
bopc bopcDefault() {
    bopc res = { 0 };
    res.body = opcPtrListDefault();
    res.body2 = opcPtrListDefault();
    res.head = opcPtrListDefault();
    res.loc = locDefault();
    res.TYPE = bopcType;
    return res;
}
opcDef opcDefDefault() {
    opcDef res = { 0 };
    return res;
}
attDef attDefDefault() {
    attDef res = { 0 };
    return res;
}
name nameDefault() {
    name res = { 0 };
    res.sign = stringDefault();
    res.csign = stringDefault();
    res.loc = locDefault();
    return res;
}
def defDefault() {
    def res = { 0 };
    res.attrs = attListDefault();
    res.name = nameDefault();
    res.TYPE = defType;
    return res;
}
varDef varDefDefault() {
    varDef res = { 0 };
    *((def*)(&res)) = defDefault();
    res.TYPE = varDefType;
    return res;
}
funDef funDefDefault() {
    funDef res = { 0 };
    *((def*)(&res)) = defDefault();
    res.args = varDefListDefault();
    res.locs = varDefListDefault();
    res.body = opcPtrListDefault();
    res.TYPE = funDefType;
    return res;
}
typDef typDefDefault() {
    typDef res = { 0 };
    *((def*)&res) = defDefault();
    res.flds = varDefListDefault();
    res.TYPE = typDefType;
    return res;
}
diagnDescr diagnDescrDefault() {
    diagnDescr res = { 0 };
    return res;
}
diagn diagnDefault() {
    diagn res = { 0 };
    res.loc = locDefault();
    res.params = charPtrListDefault();
    return res;
}
context contextDefault() {
    context res = { 0 };
    res.dgns = diagnListDefault();
    res.funs = funDefListDefault();
    res.typs = typDefListDefault();
    res.glbs = varDefListDefault();
    res.ignorew = stringListDefault();
    res.inputs = stringListDefault();
    res.output = stringDefault();
    res.strs = stringListDefault();
    res.text = stringDefault();
    res.loc = locDefault();
    return res;
}

void addDgnMultiLoc(context* c, diagnDescr desc, loc loc, list(charPtr) params) {
    if (stringListContains(c->ignorew, statstr(desc.msg)))
        return;
    diagn d = { desc, loc, params };
    diagnListAdd(&c->dgns, d);
}
void addDgnLoc(context* c, diagnDescr desc, loc loc, char* param) {
    list(charPtr) ps = charPtrListDefault();
    charPtrListAdd(&ps, param);
    addDgnMultiLoc(c, desc, loc, ps);
}
void addDgnMulti(context* c, diagnDescr desc, list(charPtr) params) {
    addDgnMultiLoc(c, desc, c->loc, params);
}
void addDgnEmpty(context* c, diagnDescr desc) {
    addDgnMultiLoc(c, desc, c->loc, charPtrListDefault());
}
void addDgnEmptyLoc(context* c, diagnDescr desc, loc loc) {
    addDgnMultiLoc(c, desc, loc, charPtrListDefault());
}
void addDgn(context* c, diagnDescr desc, char* param) {
    list(charPtr) ps = charPtrListDefault();
    charPtrListAdd(&ps, param);
    addDgnMultiLoc(c, desc, c->loc, ps);
}
string diagnToString(diagn d) {
    string res = d.descr.lvl == LVLERROR ? stringify("error: ") : d.descr.lvl == LVLWARNING ? stringify("warning: ") : stringify("message: ");
    addCptr(&res, d.descr.msg);
    for (u i = 0; i < d.params.len; i++)
        replaceAllCptr(&res, concat(concat("{", utos(i)), "}"), d.params.items[i]);
    addCptr(&res, "\n");
    if (d.loc.file.len != 0) {
        addCptr(&res, "\tin file '");
        stringAddRange(&res, d.loc.file);
        addCptr(&res, "', at line: ");
        addCptr(&res, utos(d.loc.ln));
        addCptr(&res, ", column: ");
        addCptr(&res, utos(d.loc.cl));
    }
    if (d.descr.lvl == LVLWARNING) {
        addCptr(&res, " (-");
        addCptr(&res, d.descr.id);
        addCptr(&res, ")");
    }
    stringAdd(&res, '\n');
    return res;
}
void printDgns(context* c) {
    for (size_t i = 0; i < c->dgns.len; i++)
        puts(cptrify(diagnToString(c->dgns.items[i])));
}
bool checkErr(context* c) {
    size_t i;
    for (i = 0; i < c->dgns.len && c->dgns.items[i].descr.lvl != LVLERROR; i++);
    return i < c->dgns.len;
}

string codeFrom(context* c, loc o) {
    return stringGetRange(c->text, o.cr, c->loc.cr - o.cr);
}
