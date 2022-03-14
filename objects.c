#include "h/objects.h"

listDefine(opcPtr);
listDefine(att);
listDefine(varDef);
listDefine(ref);
listDefine(funDef);
listDefine(typDef);
listDefine(dgn);
listDefineEquals(string);

const attDef ATTRIBUTES[ATTCOUNT] = {
    { "signed",
        FNONE,
        FSINGLE,
        FFLD,
    },
    { "main",
        FNONE,
        FSINGLE,
        FFUN,
    },
    { "use",
        FSTR,
        FNOFLAGS,
        FFILE,
    },
};

loc locDefault() {
    loc res = { 0 };
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
    res.par = parDefault();
    return res;
}
opc opcDefault() {
    opc res = { 0 };
    res.loc = locDefault();
    return res;
}
popc popcDefault() {
    popc res = { 0 };
    res.loc = locDefault();
    res.par = parDefault();
    res.par2 = parDefault();
    return res;
}
bopc bopcDefault() {
    bopc res = { 0 };
    res.body = opcPtrListDefault();
    res.body2 = opcPtrListDefault();
    res.head = opcPtrListDefault();
    res.loc = locDefault();
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
    return res;
}
varDef varDefDefault() {
    varDef res = { 0 };
    *((def*)(&res)) = defDefault();
    return res;
}
funDef funDefDefault() {
    funDef res = { 0 };
    *((def*)(&res)) = defDefault();
    res.args = varDefListDefault();
    res.locs = varDefListDefault();
    res.body = opcPtrListDefault();
    return res;
}
typDef typDefDefault() {
    typDef res = { 0 };
    *((def*)&res) = defDefault();
    res.flds = varDefListDefault();
    return res;
}
dgnDscr dgnDscrDefault() {
    dgnDscr res = { 0 };
    return res;
}
dgn dgnDefault() {
    dgn res = { 0 };
    return res;
}
context contextDefault() {
    context res = { 0 };
    res.bin = stringDefault();
    res.dgns = dgnListDefault();
    res.funs = funDefListDefault();
    res.typs = typDefListDefault();
    res.glbs = varDefListDefault();
    res.ignoreDgns = uListDefault();
    res.inputs = stringListDefault();
    res.atts = attListDefault();
    res.output = stringDefault();
    res.strs = stringListDefault();
    res.text = stringDefault();
    res.loc = locDefault();
    return res;
}

string codeFrom(context* c, loc o) {
    return stringGetRange(c->text, o.cr, c->loc.cr - o.cr);
}
