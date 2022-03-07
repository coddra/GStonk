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
    res.ignoreDgns = uListDefault();
    res.inputs = stringListDefault();
    res.output = stringDefault();
    res.strs = stringListDefault();
    res.text = stringDefault();
    res.loc = locDefault();
    return res;
}

string codeFrom(context* c, loc o) {
    return stringGetRange(c->text, o.cr, c->loc.cr - o.cr);
}
