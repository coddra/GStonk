#include "h/linker.h"
#include "h/opcodes.h"
#include "h/diagnostics.h"

string getCsign(string sign) {
    string res = stringClone(sign);
    replaceAllCptr(&res, "(", "$$.$");
    replaceAllCptr(&res, ")", "$.$$");
    replaceAllCptr(&res, "<", "$.$.");
    replaceAllCptr(&res, ">", "$$..");
    replaceAllCptr(&res, ",", "$..$");
    replaceAllCptr(&res, "::", "$$$$");
    return res;
}
char getPostfix(u size) {
    return size == 1 ? 'b' : size == 2 ? 'w' : size <= 4 ? 'l' : 'q';
}
string getRegister(char name, u size) {
    string res = stringDefault();
    if (size > 2)
        stringAdd(&res, size <= 4 ? 'e' : 'r');
    stringAdd(&res, name);
    stringAdd(&res, size == 1 ? 'l' : 'x');
    return res;
}

bool hasAtt(list(att) atts, ATTKIND kind, att* res) {
    for (u i = 0; i < atts.len; i++)
        if (atts.items[i].kind == kind) {
            if (res)
                *res = atts.items[i];
            return true;
        }
    return false;
}

u getFun(context* c, string sign, bool r) {
    u res = 0;
    for ( ; res < c->funs.len; res++)
        if (stringEquals(c->funs.items[res].name.sign, sign)) {
            if (r)
                c->funs.items[res].flags |= FREFERENCED;
            return res;
        }
    funDef f = funDefDefault();
    f.name.sign = sign;
    if (r)
        f.flags = FREFERENCED;
    funDefListAdd(&c->funs, f);
    return res;
}
u getTyp(context* c, string sign, bool r) {
    u res = 0;
    for ( ; res < c->typs.len; res++)
        if (stringEquals(c->typs.items[res].name.sign, sign)) {
            c->typs.items[res].flags |= r * FREFERENCED;
            return res;
        }
    typDef t = typDefDefault();
    t.name.sign = sign;
    t.flags = r * FREFERENCED;
    typDefListAdd(&c->typs, t);
    return res;
}
u getGlb(context* c, string sign, bool r) {
    u res = 0;
    for ( ; res < c->glbs.len; res++)
        if (stringEquals(c->glbs.items[res].name.sign, sign)) {
            c->glbs.items[res].flags |= r * FREFERENCED;
            return res;
        }
    varDef v = varDefDefault();
    v.name.sign = sign;
    v.flags = r * FREFERENCED;
    varDefListAdd(&c->glbs, v);
    return res;
}
ATTKIND getAtt(string sign) {
    u i = 0;
    for ( ; i < ATTCOUNT && !stringEquals(statstr(ATTRIBUTES[i].name), sign); i++);
    return i;
}
DGNKIND getDgn(string sign) {
    u i = 0;
    for ( ; i < DGNCOUNT && !stringEquals(statstr(DGNS[i].id), sign); i++);
    return i;
}
u getVar(list(varDef)* l, string sign, bool r) {
    u res = 0;
    for ( ; res < l->len; res++)
        if (stringEquals(l->items[res].name.sign, sign)) {
            l->items[res].flags |= r * FREFERENCED;
            return res;
        }
    varDef v = varDefDefault();
    v.name.sign = sign;
    v.flags = r * FREFERENCED;
    varDefListAdd(l, v);
    return res;
}

AFLAG kindToFlag(AKIND k) {
    return k == KNONE ? FNONE : 1 << (k - 1);
}
bool isGOP(context* c, string code, par* pars, OP* op) {
    for (u i = 0; i < OPCOUNT; i++)
        if (((kindToFlag(pars[0].kind) & OPS[i].arg) != 0 || pars[0].kind == KNONE && OPS[i].arg == FNONE) && (stringEquals(statstr(OPS[i].code), code) || stringEquals(statstr(OPS[i].alias), code))) {
            *op = i;
            return (OPS[i].flags & FGENERIC) != 0;
        }
    for (u i = 0; i < OPCOUNT; i++)
        if (stringEquals(statstr(OPS[i].code), code) || stringEquals(statstr(OPS[i].alias), code)) {
            *op = i;
            if (OPS[i].arg == FNONE)
                addDgnLoc(c, ENOPAR, pars[0].loc, cptrify(code));
            else {
                if (pars[0].kind == KNONE)
                    addDgn(c, EMISSINGSYNTAX, "parameter for opcode");
                else
                    addDgnLoc(c, EWRONGPAR, pars[0].loc, cptrify(code));
            }
            return (OPS[i].flags & FGENERIC) != 0;
        }
    *op = OPCOUNT;
    return false;
}

static void linkAtt(context* c, list(att) atts) {
    list(u) ul = {0};
    if (ul.cap == 0)
        ul = uListDefault();
    else
        ul.len = 0;
    for (u i = 0; i < atts.len; i++) {
        if (atts.items[i].flags & FSINGLE) {
            if (uListContains(ul, atts.items[i].kind))
                addDgnEmptyLoc(c, ESINGLEATT, atts.items[i].loc);
            else
                uListAdd(&ul, atts.items[i].kind);
        }
        atts.items[i].flags |= FLINKED;
    }
}
static void linkVar(context* c, list(varDef) vars) {
    i64 lastDef = -1;
    for (u i = 0 ; i < vars.len; i++)
        if (vars.items[i].flags & FDEFINED) {
            if ((c->typs.items[vars.items[i].type.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, vars.items[i].type.loc, cptrify(c->typs.items[vars.items[i].type.i].name.sign));
            if (lastDef >= 0)
                vars.items[i].offset = vars.items[lastDef].offset + c->typs.items[vars.items[lastDef].type.i].size;
            else
                vars.items[i].offset = 0;
            lastDef = i;
            linkAtt(c, vars.items[i].attrs);
            vars.items[i].name.csign = getCsign(vars.items[i].name.sign);
            if ((vars.items[i].flags & FREFERENCED) == 0)
                addDgnLoc(c, MNOTREFERENCED, vars.items[i].name.loc, cptrify(vars.items[i].name.sign));
            vars.items[i].flags |= FLINKED;
        }
}
static void linkTyp(context* c, list(typDef) typs) {
    for (u i = 0; i < typs.len; i++)
        if (typs.items[i].flags & FDEFINED) {
            linkAtt(c, typs.items[i].attrs);
            linkVar(c, typs.items[i].flds);
            typs.items[i].name.csign = getCsign(typs.items[i].name.sign);
            if ((typs.items[i].flags & FREFERENCED) == 0)
                addDgnLoc(c, MNOTREFERENCED, typs.items[i].name.loc, cptrify(typs.items[i].name.sign));
            typs.items[i].flags |= FLINKED;
        }
}
static void linkTypList(context* c, list(ref) typs) {
    for (u i = 0; i < typs.len; i++)
        if ((c->typs.items[typs.items[i].i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, typs.items[i].loc, cptrify(c->typs.items[typs.items[i].i].name.sign));
}
static void linkParam(context* c, opc* o, u f) {
    if (as(popc, o)->par.kind == KFUN && (c->funs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptrify(c->funs.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KTYP || as(popc, o)->par.kind == KFLD) {
        if ((c->typs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptrify(c->funs.items[as(popc, o)->par.val.r.i].name.sign));
        else if (as(popc, o)->par.kind == KFLD && (c->typs.items[as(popc, o)->par.val.r.i].flds.items[as(popc, o)->par2.val.r.i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par2.val.r.loc, cptrify(c->typs.items[as(popc, o)->par.val.r.i].flds.items[as(popc, o)->par2.val.r.i].name.sign));
    } else if (as(popc, o)->par.kind == KGLB && (c->glbs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptrify(c->glbs.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KARG && (c->funs.items[f].args.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc,
                  c->funs.items[f].args.items[as(popc, o)->par.val.r.i].name.sign.len == 0 ?
                  strcat("#", utos(as(popc, o)->par.val.r.i)) :
                  cptrify(c->funs.items[f].args.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KLOC && (c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc,
                  c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].name.sign.len == 0 ?
                  strcat("`", utos(as(popc, o)->par.val.r.i)) :
                  cptrify(c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].name.sign));
}
void linkBody(context* c, list(opcPtr) b, u f, i64* s) {
    for (u i = 0; i < b.len; i++) {
        if (OPS[b.items[i]->op].arg != FNONE)
            linkParam(c, b.items[i], f);
        if (OPS[b.items[i]->op].link != NULL)
            (*OPS[b.items[i]->op].link)(c, b.items[i], f, s);
        if (OPS[b.items[i]->op].flags & FARGCRETC)
            *s -= as(popc, b.items[i])->argc;
        else if ((OPS[b.items[i]->op].flags & FHASBODY) == 0)
            *s -= OPS[b.items[i]->op].argc;
        if (*s < 0){
            addDgnEmptyLoc(c, ESTACKLOW, b.items[i]->loc);
            *s = 0;
        }
        if (OPS[b.items[i]->op].flags & FARGCRETC)
            *s += as(popc, b.items[i])->retc;
        else if ((OPS[b.items[i]->op].flags & FHASBODY) == 0)
            *s += OPS[b.items[i]->op].retc;
        if ((b.items[i]->op == OPRET || b.items[i]->op == OPTHROW || b.items[i]->op == OPEXIT) && i != b.len - 1)
            addDgnEmptyLoc(c, WUNREACHCODE, b.items[i + 1]->loc);
    }
}
static void linkFun(context* c, list(funDef) funs) {
    for (u i = 0; i < funs.len; i++)
        if (funs.items[i].flags & FDEFINED) {
            linkAtt(c, funs.items[i].attrs);
            linkVar(c, funs.items[i].args);
            linkVar(c, funs.items[i].locs);
            linkTypList(c, funs.items[i].ret);
            i64 b = 0;
            linkBody(c, funs.items[i].body, i, &b);
            if (b > 0)
                addDgnLoc(c, WSTACKHIGH, funs.items[i].name.loc, utos(b));
            funs.items[i].name.csign = getCsign(funs.items[i].name.sign);
            if (hasAtt(funs.items[i].attrs, ATTMAIN, NULL)) {
                if (c->flags & FHASMAIN)
                    addDgnEmptyLoc(c, EMULTIMAIN, funs.items[i].name.loc);
                else {
                    c->flags |= FHASMAIN;
                    c->main = i;
                }
            }
            if ((funs.items[i].flags & FREFERENCED) == 0)
                addDgnLoc(c, MNOTREFERENCED, funs.items[i].name.loc, cptrify(funs.items[i].name.sign));
            funs.items[i].flags |= FLINKED;
        }
}
void link(context* c) {
    linkTyp(c, c->typs);
    linkFun(c, c->funs);
    linkVar(c, c->glbs);
    if (c->flags & FHASMAIN) {
        if (c->funs.items[c->main].args.len != 0)
            addDgnLoc(c, EWRONGNUMOFARGS, c->funs.items[c->main].name.loc, cptrify(c->funs.items[c->main].name.sign));
    } else
        addDgn(c, EMISSINGSYNTAX, "function with attribute \"main\"");
    c->flags |= FLINKED;
}
