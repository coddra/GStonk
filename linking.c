#include "h/linking.h"

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
    for ( ; i < ATTCOUNT && stringEquals(statstr(ATTRIBUTES[i].name), sign); i++);
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

bool isGOP(string code, OP* op) {
    for (u i = 0; i < OPCOUNT; i++)
        if (stringEquals(statstr(OPS[i].code), code) || stringEquals(statstr(OPS[i].alias), code)) {
            *op = i;
            return (OPS[i].flags & FGENERIC) > 0;
        }
    *op = OPCOUNT;
    return false;
}

void linkAtt(context* c, list(att) atts) {
    static list(u) ul = {0};
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
void linkVar(context* c, list(varDef) vars) {
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
            vars.items[i].flags |= FLINKED;
        }
}
void linkTyp(context* c, list(typDef) typs) {
    for (u i = 0; i < typs.len; i++)
        if (typs.items[i].flags & FDEFINED) {
            linkAtt(c, typs.items[i].attrs);
            linkVar(c, typs.items[i].flds);
            typs.items[i].name.csign = getCsign(typs.items[i].name.sign);
            typs.items[i].flags |= FLINKED;
        }
}
void linkTypList(context* c, list(ref) typs) {
    for (u i = 0; i < typs.len; i++)
        if ((c->typs.items[typs.items[i].i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, typs.items[i].loc, cptrify(c->typs.items[typs.items[i].i].name.sign));
}
i64 linkBody(context* c, list(opcPtr) b, u f, i64 s) {
    for (u i = 0; i < b.len; i++)
        if (is(popc, b.items[i])) {
            if (b.items[i]->op == OPLDAT) {
                if (as(popc, b.items[i])->par.val.i == -8)
                    as(popc, b.items[i])->par.val.i = 8;
                if (as(popc, b.items[i])->par.val.i != -4 && as(popc, b.items[i])->par.val.i != -2 && as(popc, b.items[i])->par.val.i != -1 &&
                    as(popc, b.items[i])->par.val.i != 1 && as(popc, b.items[i])->par.val.i != 2 && as(popc, b.items[i])->par.val.i != 4 && as(popc, b.items[i])->par.val.i != 8)
                    addDgnEmpty(c, EARGOUTOFRANGE);
            } else if (b.items[i]->op == OPSTAT) {
                if (as(popc, b.items[i])->par.val.u != 1 && as(popc, b.items[i])->par.val.u != 2 && as(popc, b.items[i])->par.val.i != 4 && as(popc, b.items[i])->par.val.i != 8)
                    addDgnEmpty(c, EARGOUTOFRANGE);
            } else if (b.items[i]->op == OPRET) {
                if (i != b.len - 1)
                    addDgnEmptyLoc(c, WUNREACHCODE, b.items[i]->loc);
                as(popc, b.items[i])->argc = c->funs.items[f].ret.len;
                as(popc, b.items[i])->retc = 0;
            }
            if (as(popc, b.items[i])->par.kind == KFUN) {
                if ((c->funs.items[as(popc, b.items[i])->par.val.r.i].flags & FDEFINED) == 0)
                    addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par.val.r.loc, cptrify(c->funs.items[as(popc, b.items[i])->par.val.r.i].name.sign));
                else {
                    as(popc, b.items[i])->argc = c->funs.items[as(popc, b.items[i])->par.val.r.i].args.len;
                    as(popc, b.items[i])->retc = c->funs.items[as(popc, b.items[i])->par.val.r.i].ret.len;
                }
            } else if (as(popc, b.items)->par.kind == KTYP && (c->typs.items[as(popc, b.items[i])->par.val.r.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par.val.r.loc, cptrify(c->funs.items[as(popc, b.items[i])->par.val.r.i].name.sign));
            else if (as(popc, b.items[i])->par.kind == KGLB && (c->glbs.items[as(popc, b.items[i])->par.val.r.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par.val.r.loc, cptrify(c->glbs.items[as(popc, b.items[i])->par.val.r.i].name.sign));
            else if (as(popc, b.items[i])->par.kind == KARG && (c->funs.items[f].args.items[as(popc, b.items[i])->par.val.r.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par.val.r.loc,
                          c->funs.items[f].args.items[as(popc, b.items[i])->par.val.r.i].name.sign.len == 0 ?
                          strcat("#", utos(as(popc, b.items[i])->par.val.r.i)) :
                          cptrify(c->funs.items[f].args.items[as(popc, b.items[i])->par.val.r.i].name.sign));
            else if (as(popc, b.items[i])->par.kind == KLOC && (c->funs.items[f].locs.items[as(popc, b.items[i])->par.val.r.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par.val.r.loc,
                          c->funs.items[f].locs.items[as(popc, b.items[i])->par.val.r.i].name.sign.len == 0 ?
                          strcat("`", utos(as(popc, b.items[i])->par.val.r.i)) :
                          cptrify(c->funs.items[f].locs.items[as(popc, b.items[i])->par.val.r.i].name.sign));
            if (as(popc, b.items[i])->par.kind == KFLD && (c->typs.items[as(popc, b.items[i])->par.val.r.i].flds.items[as(popc, b.items[i])->par2.val.r.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, as(popc, b.items[i])->par2.val.r.loc, cptrify(c->typs.items[as(popc, b.items[i])->par.val.r.i].flds.items[as(popc, b.items[i])->par2.val.r.i].name.sign));
            s -= as(popc, b.items[i])->argc;
            if (s < 0){
                addDgnEmptyLoc(c, ESTACKLOW, b.items[i]->loc);
                s = 0;
            }
            s += as(popc, b.items[i])->retc;
        } else if (is(bopc, b.items[i])) {
            i64 h;
            if (b.items[i]->op == OPIF || b.items[i]->op == OPWHILE) {
                h = linkBody(c, as(bopc, b.items[i])->head, f, s) - s;
                s += h;
                if (s == 0)
                    addDgnEmptyLoc(c, ESTACKLOW, b.items[i]->loc);
                else
                    s--;
            }
            if (b.items[i]->op == OPWHILE) {
                i64 tmp = s;
                s = linkBody(c, as(bopc, b.items[i])->body, f, s);
                if (h + s - tmp != 1)
                    addDgnEmptyLoc(c, ESTACKUNPRED, b.items[i]->loc);
                s = tmp;
            }
            if (b.items[i]->op == OPIF || b.items[i]->op == OPTRY) {
                i64 s1 = linkBody(c, as(bopc, b.items[i])->body, f, b.items[i]->op == OPIF ? s : 0);
                i64 s2 = as(bopc, b.items[i])->body2.len == 0 ? (b.items[i]->op == OPIF ? s : 0) : linkBody(c, as(bopc, b.items[i])->body2, f, b.items[i]->op == OPIF ? s : 1);
                if (b.items[i]->op == OPTRY) {
                    as(bopc, b.items[i])->retc = s1;
                    as(bopc, b.items[i])->retc2 = s2;
                }
                if (s1 != s2)
                    addDgnEmptyLoc(c, ESTACKUNPRED, b.items[i]->loc);
                if (s2 < s1)
                    s1 = s2;
                s = b.items[i]->op == OPIF ? s1 : s + s1;
            }
        } else {
            if (b.items[i]->op == OPTHROW && i != b.len - 1)
                addDgnEmptyLoc(c, WUNREACHCODE, b.items[i]->loc);
            s -= OPS[b.items[i]->op].argc;
            if (s < 0) {
                addDgnEmptyLoc(c, ESTACKLOW, b.items[i]->loc);
                s = 0;
            }
            s += OPS[b.items[i]->op].retc;
        }
    return s;
}
void linkFun(context* c, list(funDef) funs) {
    for (u i = 0; i < funs.len; i++)
        if (funs.items[i].flags & FDEFINED) {
            linkAtt(c, funs.items[i].attrs);
            linkVar(c, funs.items[i].args);
            linkVar(c, funs.items[i].locs);
            linkTypList(c, funs.items[i].ret);
            u b = linkBody(c, funs.items[i].body, i, 0);
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
            funs.items[i].flags |= FLINKED;
        }
}
void link(context* c) {
    linkTyp(c, c->typs);
    linkFun(c, c->funs);
    linkVar(c, c->glbs);
    c->flags |= FLINKED;
}
