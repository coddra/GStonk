#include "h/linker.h"
#include "h/opcodes.h"
#include "h/diagnostics.h"
#include "h/parser.h"
#include "mcx/file.h"

string getCsign(string sign) {
    string res = stringClone(sign);
    stringReplaceAll(&res, sstr("("), sstr("$$.$"));
    stringReplaceAll(&res, sstr(")"), sstr("$.$$"));
    stringReplaceAll(&res, sstr("<"), sstr("$.$."));
    stringReplaceAll(&res, sstr(">"), sstr("$$.."));
    stringReplaceAll(&res, sstr(","), sstr("$..$"));
    stringReplaceAll(&res, sstr("::"), sstr("$$$$"));
    return res;
}
char getPostfix(u size) {
    return size == 1 ? 'b' : size == 2 ? 'w' : size <= 4 ? 'l' : 'q';
}
string getRegister(char name, u size) {
    string res = {0};
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
    for (; res < c->funs.len; res++)
        if (stringEquals(c->funs.items[res].name.sign, sign)) {
            if (r) {
                if ((c->funs.items[res].flags & FDEFINED) != 0 && (c->funs.items[res].body.flags & FPARSED) == 0)
                    completeBody(c, &c->funs.items[res].body, res);
                c->funs.items[res].flags |= FREFERENCED;
            }
            return res;
        }
    funDef f = {0};
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
    typDef t = {0};
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
    varDef v = {0};
    v.name.sign = sign;
    v.flags = r * FREFERENCED;
    varDefListAdd(&c->glbs, v);
    return res;
}
ATTKIND getAtt(string sign) {
    u i = 0;
    for ( ; i < ATTCOUNT && !stringEquals(sstr(ATTRIBUTES[i].name), sign); i++);
    return i;
}
DGNKIND getDgn(string sign) {
    u i = 0;
    for ( ; i < DGNCOUNT && !stringEquals(sstr(DGNS[i].id), sign); i++);
    return i;
}
u getVar(list(varDef)* l, string sign, bool r) {
    u res = 0;
    for ( ; res < l->len; res++)
        if (stringEquals(l->items[res].name.sign, sign)) {
            l->items[res].flags |= r * FREFERENCED;
            return res;
        }
    varDef v = {0};
    v.name.sign = sign;
    v.flags = r * FREFERENCED;
    varDefListAdd(l, v);
    return res;
}

bool export(context* c, def* d) {
    return ((c->flags & FFLYCHECK) == FFLYCHECK && d->name.loc.file == 0) ||
        (c->flags & FFLYCHECK) != FFLYCHECK && ((d->flags & FREFERENCED) != 0 || (c->flags & FEXPORTALL) != 0 ||
         hasAtt(c->atts, ATTEXPORT, NULL) || hasAtt(d->attrs, ATTEXPORT, NULL) || hasAtt(d->attrs, ATTMAIN, NULL));
}

AFLAG kindToFlag(AKIND k) {
    return k == KNONE ? FNONE : 1 << (k - 1);
}
bool isGOP(context* c, string code, par* pars, OP* op) {
    for (u i = 0; i < OPCOUNT; i++)
        if (((kindToFlag(pars[0].kind) & OPS[i].arg) != 0 || pars[0].kind == KNONE && OPS[i].arg == FNONE) && (stringEquals(sstr(OPS[i].code), code) || stringEquals(sstr(OPS[i].alias), code))) {
            *op = i;
            return (OPS[i].flags & FGENERIC) != 0;
        }
    for (u i = 0; i < OPCOUNT; i++)
        if (stringEquals(sstr(OPS[i].code), code) || stringEquals(sstr(OPS[i].alias), code)) {
            *op = i;
            if (OPS[i].arg == FNONE)
                addDgnLoc(c, ENOPAR, pars[0].loc, cptr(code));
            else {
                if (pars[0].kind == KNONE)
                    addDgn(c, EMISSINGSYNTAX, "parameter for opcode");
                else
                    addDgnLoc(c, EWRONGPAR, pars[0].loc, cptr(code));
            }
            return (OPS[i].flags & FGENERIC) != 0;
        }
    *op = OPCOUNT;
    return false;
}

string codeFrom(context* c, loc o) {
    return stringGetRange(c->text, o.cr, c->loc.cr - o.cr);
}
static bool hasFile(context* c, string path) {
    u i = 0;
    for (; i < c->inputs.len && !stringEquals(realPath(path), realPath(c->inputs.items[i])); i++);
    return i != c->inputs.len;
}
bool addFile(context* c, string path, loc loc) {
    if (fileExists(path)) {
        bool res = !hasFile(c, path);
        if (res)
            stringListAdd(&c->inputs, path);
        else if (c->loc.file >= c->inputs.len)
            addDgn(c, MMULTIFILE, cptr(path));
        return res;
    } else {
        string tmp = stringClone(c->bin);
        catCptr(&tmp, "std/");
        stringAddRange(&tmp, path);
        if (fileExists(tmp)) {
            return addFile(c, tmp, loc);
        } else {
            addDgnLoc(c, EFILENOTEXIST, loc, cptr(path));
            return false;
        }
    }
}
bool isStd(context* c, string path) {
    return stringStartsWith(path, c->bin);
}

bool stops(opc* op) {
    return op->op == OPRET || op->op == OPTHROW || op->op == OPEXIT ||
        ((OPS[op->op].flags & FHASBODY) != 0 &&
         ((as(bopc, op)->head.flags & FSTOPS) != 0 ||
          (as(bopc, op)->body.flags & as(bopc, op)->els.flags & FSTOPS) != 0));
}

static bool emitNoRef(context* c, def* d) {
    return (d->flags & FREFERENCED) == 0 && !((c->flags & FHASMAIN) == 0 || c->funs.items[c->main].name.loc.file != 0);
}
static void linkAtt(context* c, list(att) atts, AFLAG t) {
    list(u) ul = {0};
    for (u i = 0; i < atts.len; i++) {
        if (ATTRIBUTES[atts.items[i].kind].flags & FSINGLE) {
            if (uListContains(ul, atts.items[i].kind))
                addDgnEmptyLoc(c, ESINGLEATT, atts.items[i].loc);
            else
                uListAdd(&ul, atts.items[i].kind);
        }
        if ((ATTRIBUTES[atts.items[i].kind].trgt & t) == 0)
            addDgnLoc(c, EWRONGTARGET, atts.items[i].loc, ATTRIBUTES[atts.items[i].kind].name);
        atts.items[i].flags |= FLINKED;
    }
}
static void linkVar(context* c, list(varDef) vars, AFLAG t) {
    u64 lastDef = vars.len;
    for (u i = 0 ; i < vars.len; i++)
        if (vars.items[i].flags & FDEFINED) {
            if ((c->typs.items[vars.items[i].type.i].flags & FDEFINED) == 0)
                addDgnLoc(c, EDEFNOTFOUND, vars.items[i].type.loc, cptr(c->typs.items[vars.items[i].type.i].name.sign));
            if (lastDef == vars.len)
                vars.items[i].offset = 0;
            else
                vars.items[i].offset = vars.items[lastDef].offset + c->typs.items[vars.items[lastDef].type.i].size;
            lastDef = i;
            linkAtt(c, vars.items[i].attrs, t);
            vars.items[i].name.csign = getCsign(vars.items[i].name.sign);
            if ((t & (FGLB | FFLD)) != 0 && emitNoRef(c, as(def, &vars.items[i])) || (t & (FGLB | FFLD)) == 0 && (vars.items[i].flags & FREFERENCED) == 0)
                addDgnLoc(c, MNOTREFERENCED, vars.items[i].name.loc, cptr(vars.items[i].name.sign));
            vars.items[i].flags |= FLINKED;
        }
}
static void linkTyp(context* c) {
    for (u i = 0; i < c->typs.len; i++)
        if (c->typs.items[i].flags & FDEFINED) {
            linkAtt(c, c->typs.items[i].attrs, FTYP);
            linkVar(c, c->typs.items[i].flds, FFLD);
            c->typs.items[i].name.csign = getCsign(c->typs.items[i].name.sign);
            if (emitNoRef(c, as(def, &c->typs.items[i])))
                addDgnLoc(c, MNOTREFERENCED, c->typs.items[i].name.loc, cptr(c->typs.items[i].name.sign));
            c->typs.items[i].flags |= FLINKED;
        }
}
static void linkTypList(context* c, list(ref) typs) {
    for (u i = 0; i < typs.len; i++)
        if ((c->typs.items[typs.items[i].i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, typs.items[i].loc, cptr(c->typs.items[typs.items[i].i].name.sign));
}
static void linkParam(context* c, opc* o, u f) {
    if (as(popc, o)->par.kind == KFUN && (c->funs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptr(c->funs.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KTYP || as(popc, o)->par.kind == KFLD) {
        if ((c->typs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptr(c->funs.items[as(popc, o)->par.val.r.i].name.sign));
        else if (as(popc, o)->par.kind == KFLD && (c->typs.items[as(popc, o)->par.val.r.i].flds.items[as(popc, o)->par2.val.r.i].flags & FDEFINED) == 0)
            addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par2.val.r.loc, cptr(c->typs.items[as(popc, o)->par.val.r.i].flds.items[as(popc, o)->par2.val.r.i].name.sign));
    } else if (as(popc, o)->par.kind == KGLB && (c->glbs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc, cptr(c->glbs.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KARG && (c->funs.items[f].args.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc,
                  c->funs.items[f].args.items[as(popc, o)->par.val.r.i].name.sign.len == 0 ?
                  strcat("#", cptr(utos(as(popc, o)->par.val.r.i))) :
                  cptr(c->funs.items[f].args.items[as(popc, o)->par.val.r.i].name.sign));
    else if (as(popc, o)->par.kind == KLOC && (c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].flags & FDEFINED) == 0)
        addDgnLoc(c, EDEFNOTFOUND, as(popc, o)->par.val.r.loc,
                  c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].name.sign.len == 0 ?
                  strcat("`", cptr(utos(as(popc, o)->par.val.r.i))) :
                  cptr(c->funs.items[f].locs.items[as(popc, o)->par.val.r.i].name.sign));
}
void linkBody(context* c, body* b, u f, i64* s) {
    b->retc = *s;
    for (u i = 0; i < b->ops.len; i++) {
        if (OPS[b->ops.items[i]->op].arg != FNONE)
            linkParam(c, b->ops.items[i], f);
        if (OPS[b->ops.items[i]->op].link != NULL)
            (*OPS[b->ops.items[i]->op].link)(c, b->ops.items[i], f, s);
        if (OPS[b->ops.items[i]->op].flags & FARGCRETC)
            *s -= as(popc, b->ops.items[i])->argc;
        else if ((OPS[b->ops.items[i]->op].flags & FHASBODY) == 0)
            *s -= OPS[b->ops.items[i]->op].argc;
        if (*s < 0){
            addDgnEmptyLoc(c, ESTACKLOW, b->ops.items[i]->loc);
            *s = 0;
        }
        if (OPS[b->ops.items[i]->op].flags & FARGCRETC)
            *s += as(popc, b->ops.items[i])->retc;
        else if ((OPS[b->ops.items[i]->op].flags & FHASBODY) == 0)
            *s += OPS[b->ops.items[i]->op].retc;
        if ((b->ops.items[i]->op == OPRET || b->ops.items[i]->op == OPTHROW || b->ops.items[i]->op == OPEXIT) && i != b->ops.len - 1)
            addDgnEmptyLoc(c, WUNREACHCODE, b->ops.items[i + 1]->loc);
    }
    b->retc = *s - b->retc;
}
static void linkFun(context* c) {
    for (u i = 0; i < c->funs.len; i++)
        if (c->funs.items[i].flags & FDEFINED) {
            linkAtt(c, c->funs.items[i].attrs, FFUN);
            linkVar(c, c->funs.items[i].args, FARG);
            linkVar(c, c->funs.items[i].locs, FLOC);
            linkTypList(c, c->funs.items[i].ret);
            if (c->funs.items[i].body.flags & FPARSED) {
                i64 b = 0;
                linkBody(c, &c->funs.items[i].body, i, &b);
                if (b > 0)
                    addDgnLoc(c, WSTACKHIGH, c->funs.items[i].name.loc, cptr(utos(b)));
                if ((c->funs.items[i].body.flags & FSTOPS) == 0)
                    addDgnEmptyLoc(c, ENOTSTOPS, c->funs.items[i].name.loc);
            }
            c->funs.items[i].name.csign = getCsign(c->funs.items[i].name.sign);
            if (hasAtt(c->funs.items[i].attrs, ATTMAIN, NULL)) {
                if (c->flags & FHASMAIN)
                    addDgnEmptyLoc(c, EMULTIMAIN, c->funs.items[i].name.loc);
                else {
                    c->flags |= FHASMAIN;
                    c->main = i;
                }
            } else if (emitNoRef(c, as(def, &c->funs.items[i])))
                addDgnLoc(c, MNOTREFERENCED, c->funs.items[i].name.loc, cptr(c->funs.items[i].name.sign));
            c->funs.items[i].flags |= FLINKED;
        }
}
void link(context* c) {
    linkTyp(c);
    linkFun(c);
    linkVar(c, c->glbs, FGLB);
    if (c->flags & FHASMAIN) {
        if (c->funs.items[c->main].args.len != 0)
            addDgnLoc(c, EWRONGNUMOFARGS, c->funs.items[c->main].name.loc, cptr(c->funs.items[c->main].name.sign));
    } else
        addDgn(c, EMISSINGSYNTAX, "function with attribute 'main'");
    c->flags |= FLINKED;
}
