#include "h/parser.h"
#include "mcx/file.h"
#include "h/preprocessor.h"
#include "h/linker.h"
#include "h/opcodes.h"
#include "h/diagnostics.h"

static bool parseHead(context* c, body* res, u r);
static bool skipBody(context* c, body* res);

set(char)* specialChars = NULL;
set(char)* opSymbols = NULL;
set(char)* modifiers = NULL;
set(char)* token = NULL;

list(string) consts = {0};

void init(PARSER) {
    init(CPARSER);
    specialChars = charAggregateFromArray("_<>", 3);
    opSymbols = charAggregateFromArray("~!@%^&*()-_=+\\|:;,<.>/?`#", 27);
    modifiers = charAggregateFromArray("-`#", 3);
    token = charSetSubstract(notWhitespace, charAggregateFromArray("{}", 2));
    stringListAdd(&consts, sstr("$stdin"));
    stringListAdd(&consts, sstr("$stdout"));
    stringListAdd(&consts, sstr("$stderr"));
}

static bool parseHex(context* c, u64* res, u8 digits) {
    loc o = c->loc;
    if (digits == 0){
        if (!cParseAllCS(c, hexDigits))
            return false;
    } else
        for (u i = 0; i < digits; i++)
            if (!cParseCS(c, hexDigits)) {
                c->loc = o;
                return false;
            }
    if (res)
        *res = strtoul(cptr(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL, 16);
    return true;
}
static bool parseES(context* c, char* res) {
    loc o = c->loc;
    if (!cParseC(c, '\\'))
        return false;
    char r = 0;
    if (cParseCSR(c, escChars, &r)) {
        if (res)
            *res = getEscChar(r);
    } else if (cParseC(c, 'x') || cParseC(c, 'X')) {
        u64 h = 0;
        if (!parseHex(c, &h, 2))
            cAddDgn(c, &EUNRECESCSEQ, cptr(cCodeFrom(c, o)));
        if (res)
            *res = *(u8*)h;
    } else
        cAddDgn(c, &EUNRECESCSEQ, cptr(cCodeFrom(c, o)));
    return true;
}
static inline bool parseCC(context* c, char* res, bool str) {
    return parseES(c, res) || cParseCSR(c, str ? stringLiteral : charLiteral, res);
}
static inline bool parseCL(context* c, char* res) {
    return cParseC(c, '\'') && parseCC(c, res, false) && cParseC(c, '\'');
}
static bool parseIL(context* c, i64* res) {
    loc o = c->loc;
    cParseC(c, '-');
    if (!cParseAllCS(c, digits)) {
        c->loc = o;
        return false;
    }
    if (res)
        *res = strtol(cptr(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL, 10);
    return true;
}
static bool parseUL(context* c, u64* res) {
    loc o = c->loc;
    if (cParseC(c, '0')) {
        if (cParseC(c, 'x') || cParseC(c, 'X')) {
            if (!parseHex(c, res, 0))
                cAddDgn(c, &EMISSINGSYNTAX, "value of hexadecimal number");
        } else {
            o = c->loc;
            if (cParseAllCS(c, octDigits)) {
                if (res)
                    *res = strtoul(cptr(cCodeFrom(c, o)), NULL, 8);
            } else {
                if (res)
                    *res = 0;
            }
        }
    } else if (cParseAllCS(c, digits)) {
        if (res)
            *res = strtoul(cptr(cCodeFrom(c, o)), NULL, 10);
    } else if (parseCL(c, (char*)res));
    else if (cParseC(c, '$') && cParseAllCS(c, letters)) {
        u i = stringListPos(consts, cCodeFrom(c, o));
        if (i == consts.len) {
            c->loc = o;
            return false;
        } else if (res)
            *res = i;
    } else {
        c->loc = o;
        return false;
    }
    return true;
}
static bool parseDL(context* c, d* res) {
    loc o = c->loc;
    cParseC(c, '-');
    if (!cParseAllCS(c, digits) || !cParseC(c, '.') || !cParseAllCS(c, digits)) {
        c->loc = o;
        return false;
    }
    if (res)
        *res = strtod(cptr(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL);
    return true;
}
static bool parseSL(context* c, ref* res) {
    string s = {0};
    if (res)
        res->loc = c->loc;
    if (!cParseC(c, '"'))
        return false;
    char r;
    bool dotflag = true;
    while (dotflag) {
        while (parseCC(c, &r, true))
            stringAdd(&s, r);
        if (!cParseC(c, '"'))
            cAddDgn(c, &EMISSINGTOKEN, "\"");
        dotflag = cParseC(c, '.');
        if (dotflag) {
            cParseAllCS(c, whitespace);
            if (!cParseC(c, '"'))
                cAddDgn(c, &EMISSINGTOKEN, "\"");
        }
    }
    if (res) {
        res->i = stringListPos(c->strs, s);
        if (res->i == c->strs.len)
            stringListAdd(&c->strs, s);
    }
    return true;
}

static bool parseName(context* c, name* res) {
    if (res)
        res->loc = c->loc;
    if (!cParseCS(c, specialChars) && !cParseCS(c, letters))
        return false;
    while (cParseCS(c, specialChars) || cParseCS(c, letters) || cParseCS(c, digits));
    if (res)
        res->sign = stringGetRange(c->text, res->loc.cr, c->loc.cr - res->loc.cr);
    return true;
}
static bool parseIdfr(context* c, name* res) {
    if (res)
        res->loc = c->loc;
    if (!parseName(c, NULL))
        return false;
    while (cParseC(c, '.') && parseName(c, NULL));
    if (c->text.items[c->loc.cr - 1] == '.')
        cPrev(c);
    if (res)
        res->sign = cCodeFrom(c, res->loc);
    return true;
}

static inline bool parseTypIdfr(context* c, name* res) {
    return parseIdfr(c, res);
}
static bool parseTypList(context* c, string* res, list(ref) *refs) {
    name n = {0};
    if (!parseTypIdfr(c, &n))
        return false;
    if (res)
        *res = n.sign;
    u i = getTyp(c, n.sign, true);
    if (refs) {
        ref r = { i, n.loc };
        refListAdd(refs, r);
    }
    cParseAllCS(c, whitespace);
    while (cParseC(c, ',')) {
        stringAdd(res, ',');
        cParseAllCS(c, whitespace);
        if (parseTypIdfr(c, &n)) {
            if (res)
                stringAddRange(res, n.sign);
            i = getTyp(c, n.sign, true);
            if (refs) {
                ref r = { i, n.loc };
                refListAdd(refs, r);
            }
            cParseAllCS(c, whitespace);
        }
        else
            cAddDgn(c, &EMISSINGSYNTAX, "type identifier");
    }
    return true;
}
static bool parseFunIdfr(context* c, name* res) {
    loc o = c->loc;
    if (!parseIdfr(c, res))
        return false;
    name n = {0};
    if (!cParseC(c, '(')) {
        c->loc = o;
        return false;
    }
    if (res)
        stringAdd(&res->sign, '(');
    cParseAllCS(c, whitespace);
    if (parseTypList(c, &n.sign, NULL) && res)
        stringAddRange(&res->sign, n.sign);
    if (res)
        stringAdd(&res->sign, ')');
    if (!cParseC(c, ')'))
        cAddDgn(c, &EMISSINGTOKEN, ")");
    if (res) {
        stringAdd(&res->sign, ':');
        stringAdd(&res->sign, ':');
    }
    if (!cParseCptr(c, "::"))
        cAddDgn(c, &EMISSINGTOKEN, "::");
    if (parseTypList(c, &n.sign, NULL) && res)
        stringAddRange(&res->sign, n.sign);
    return true;
}
static bool parseVarIdfr(context* c, name* res) {
    loc o = c->loc;
    if (!parseIdfr(c, res))
        return false;
    if (!cParseCptr(c, "::")) {
        c->loc = o;
        return false;
    }
    if (res) {
        stringAdd(&res->sign, ':');
        stringAdd(&res->sign, ':');
    }
    name n = {0};
    if (!parseTypIdfr(c, &n))
        cAddDgn(c, &EMISSINGSYNTAX, "type identifier");
    else if (res)
        stringAddRange(&res->sign, n.sign);
    return true;
}

static bool parseFun(context* c, ref* res) {
    if (res)
        res->loc = c->loc;
    name n = {0};
    if (parseFunIdfr(c, &n)) {
        u i = getFun(c, n.sign, true);
        if (res)
            res->i = i;
    } else
        return false;
    return true;
}
static bool parseTyp(context* c, ref* res) {
    if (res)
        res->loc = c->loc;
    name n = {0};
    if (parseTypIdfr(c, &n) && res) {
        u i = getTyp(c, n.sign, true);
        if (res)
            res->i = i;
    }
    else
        return false;
    return true;
}
static bool parseGlb(context* c, ref* res) {
    if (res)
        res->loc = c->loc;
    name n = {0};
    if (parseVarIdfr(c, &n)) {
        u i = getGlb(c, n.sign, true);
        if (res)
            res->i = i;
    } else
        return false;
    return true;
}
static bool parseVar(context* c, ref* res, list(varDef)* l) {
    name n = {0};
    if (!parseVarIdfr(c, &n))
        return false;
    else if (res) {
        res->i = getVar(l, n.sign, true);
        res->loc = c->loc;
    }
    return true;
}
static bool parsePar(context* c, par* res, AFLAG kind, u r) {
    res->loc = c->loc;
    if ((kind & FFUN) > 0 && parseFun(c, &res->val.r))
        res->kind = KFUN;
    else if ((kind & FGLB) > 0 && parseGlb(c, &res->val.r))
        res->kind = KGLB;
    else if (((kind & FTYP) > 0 || (kind & FFLD) > 0) && parseTyp(c, &res->val.r)) {
        if ((kind & FFLD) > 0 && cParseCptr(c, "->")) {
            if (!parseVar(c, &res[1].val.r, &c->typs.items[res->val.r.i].flds))
                cAddDgn(c, &EMISSINGTOKEN, "->");
            else
                res->kind = KFLD;
        } else
            res->kind = KTYP;
    }
    else if ((kind & FLOC) > 0 && cParseC(c, '`')) {
        res->val.r.loc = c->loc;
        if (parseUL(c, &res->val.r.i)) {
            if (res->val.r.i >= c->funs.items[r].locs.len)
                cAddDgn(c, &EARGOUTOFRANGE, "local variable number");
            else
                c->funs.items[r].locs.items[res->val.r.i].flags |= FREFERENCED;
        } else if (parseVar(c, &res->val.r, &c->funs.items[r].locs))
            c->funs.items[r].locs.items[res->val.r.i].flags |= FREFERENCED;
        else
            cAddDgn(c, &EMISSINGSYNTAX, "index or name of local variable");
        res->kind = KLOC;
    } else if ((kind & FARG) > 0 && cParseC(c, '#')) {
        res->val.r.loc = c->loc;
        if (parseUL(c, &res->val.r.i)) {
            if (res->val.r.i >= c->funs.items[r].args.len)
                cAddDgn(c, &EARGOUTOFRANGE, "argument number");
            else
                c->funs.items[r].args.items[res->val.r.i].flags |= FREFERENCED;
        } else if (parseVar(c, &res->val.r, &c->funs.items[r].args))
            c->funs.items[r].args.items[res->val.r.i].flags |= FREFERENCED;
        else
            cAddDgn(c, &EMISSINGSYNTAX, "index or name of argument");
        res->kind = KARG;
    } else if ((kind & FDOUB) > 0 && parseDL(c, &res->val.d))
        res->kind = KDOUB;
    else if ((kind & FUINT) > 0 && parseUL(c, &res->val.u))
        res->kind = KUINT;
    else if ((kind & FINT) > 0 && parseIL(c, &res->val.i))
        res->kind = KINT;
    else if ((kind & FSTR) > 0 && parseSL(c, &res->val.r))
        res->kind = KSTR;
    else {
        res->kind = KNONE;
        return false;
    }
    return true;
}

static bool parseAtt(context* c, att* res) {
    res->loc = c->loc;
    if (!cParseAllCS(c, letters))
        return false;
    res->kind = getAtt(cCodeFrom(c, res->loc));
    if (res->kind == ATTCOUNT)
        cAddDgnLoc(c, &EUNRECATT, res->loc, cptr(cCodeFrom(c, res->loc)));
    cParseAllCS(c, whitespace);
    if (!cParseC(c, '(')) {
        if (ATTRIBUTES[res->kind].arg != FNONE)
            cAddDgn(c, &EMISSINGSYNTAX, "parameter for attribute");
        return true;
    }
    par p;
    if (parsePar(c, &p, FATTANY, 0)) {
        if (ATTRIBUTES[res->kind].arg & kindToFlag(p.kind))
            res->par = p;
        else
            cAddDgnLoc(c, &EWRONGPAR, p.loc, ATTRIBUTES[res->kind].name);
    } else if (ATTRIBUTES[res->kind].arg != FNONE)
        cAddDgn(c, &EMISSINGSYNTAX, "parameter for attribute");
    if (!cParseC(c, ')'))
        cAddDgn(c, &EMISSINGTOKEN, ")");
    return true;
}
static bool parseAttList(context* c, list(att)* res) {
    if (!cParseC(c, '['))
        return false;
    cParseAllCS(c, whitespace);
    att a = {0};
    bool commaflag = true;
    while (parseAtt(c, &a)) {
        attListAdd(res, a);
        if (!commaflag)
            cAddDgnLoc(c, &EMISSINGTOKEN, a.loc, ",");
        cParseAllCS(c, whitespace);
        commaflag = cParseC(c, ',');
        if (commaflag)
            cParseAllCS(c, whitespace);
    }
    if (!cParseC(c, ']'))
        cAddDgn(c, &EMISSINGTOKEN, "]");
    return true;
}

static bool parseVarDef(context* c, varDef* res) {
    if (!parseIdfr(c, &res->name))
        return false;
    if (!cParseCptr(c, "::"))
        cAddDgn(c, &EMISSINGTOKEN, "::");
    stringAdd(&res->name.sign, ':');
    stringAdd(&res->name.sign, ':');
    name n = {0};
    if (!parseTypIdfr(c, &n))
        cAddDgn(c, &EMISSINGSYNTAX, "type of variable");
    stringAddRange(&res->name.sign, n.sign);
    res->type.loc = n.loc;
    res->type.i = getTyp(c, n.sign, true);
    res->flags |= FDEFINED;
    cParseAllCS(c, whitespace);
    parseAttList(c, &res->attrs);
    return true;
}
static bool parseVarList(context* c, list(varDef)* res) {
    loc o = c->loc;
    if (!cParseC(c, '('))
        return false;
    cParseAllCS(c, whitespace);
    varDef v = {0};
    bool commaflag = true;
    while (parseVarDef(c, &v)) {
        u r = getVar(res, v.name.sign, false);
        if ((res->items[r].flags & FDEFINED))
            cAddDgn(c, &ESECONDDECLARATION, cptr(v.name.sign));
        res->items[r] = v;
        if (!commaflag)
            cAddDgnLoc(c, &EMISSINGTOKEN, v.name.loc, ",");
        cParseAllCS(c, whitespace);
        commaflag = cParseC(c, ',');
        if (commaflag)
            cParseAllCS(c, whitespace);
    }
    if (!cParseC(c, ')')) {
        if (res->len > 0)
            cAddDgn(c, &EMISSINGTOKEN, ")");
        else
            c->loc = o;
    }
    return c->loc.cr != o.cr;
}

static bool parseToken(context* c) {
    loc o = c->loc;
    bool res = cParseAllCS(c, token);
    if (res)
        cAddDgnLoc(c, &EUNRECTOKEN, o, cptr(cCodeFrom(c, o)));
    return res;
}
static bool parseIf(context* c, bopc** res, u r) {
    parseHead(c, &(*res)->head, r);
    if (stops(as(opc, *res)))
        return true;
    if (parseBody(c, &(*res)->body, r))
        cParseAllCS(c, whitespace);
    else
        cAddDgn(c, &EMISSINGSYNTAX, "body of if statement");
    loc o = c->loc;
    if (cParseCptr(c, "|?") || cParseCptr(c, ".elif")) {
        cParseAllCS(c, whitespace);
        bopc* b = new(bopc);
        b->loc = o;
        b->op = OPIF;
        parseIf(c, &b, r);
        if (stops(as(opc, b)))
            (*res)->els.flags |= FSTOPS;
        opcPtrListAdd(&(*res)->els.ops, as(opc, b));
    } else if (cParseCptr(c, "|>") || cParseCptr(c, ".else")) {
        cParseAllCS(c, whitespace);
        parseBody(c, &(*res)->els, r);
    }
    return true;
}
static bool parseOPC(context* c, opc** res, u r) {
    loc o = c->loc;
    if (!cParseC(c, '.') || !cParseAllCS(c, letters))
            cParseAllCS(c, opSymbols);
    if (charSetContains(modifiers, c->text.items[c->loc.cr - 1]) && cParseCS(c, notWhitespace)) {
        cPrev(c);
        cPrev(c);
    }
    string s = cCodeFrom(c, o);
    if (s.len > 0 && charSetContains(letters, s.items[s.len - 1]))
        cParseC(c, '.');
    par pars[2] = { 0 };
    if (!parsePar(c, pars, FANY, r) && s.len == 0)
        return false;
    OP op;
    if (isGOP(c, s, pars, &op)) {
        if (OPS[op].arg == FNONE) {
            *res = new(opc);
            (*res)->loc = o;
            (*res)->op = op;
        } else {
            *res = as(opc, new(popc));
            (*res)->loc = o;
            (*res)->op = op;
            as(popc, *res)->argc = OPS[op].argc;
            as(popc, *res)->retc = OPS[op].retc;
            as(popc, *res)->par = pars[0];
            as(popc, *res)->par2 = pars[1];
            if (as(popc, *res)->par.kind == KFLD)
                as(popc, *res)->argc++;
        }
    } else if (op == OPCOUNT) {
            c->loc = o;
            return false;
    } else if (op == OPRET) {
        *res = as(opc, new(popc));
        (*res)->loc = o;
        (*res)->op = op;
        as(popc, *res)->argc = c->funs.items[r].ret.len;
        as(popc, *res)->retc = 0;
    } else if (op == OPCLAT) {
        *res = as(opc, new(popc));
        (*res)->loc = o;
        (*res)->op = op;
        as(popc, *res)->par = pars[0];
        if (as(popc, *res)->par.val.u > max(u16))
            cAddDgnLoc(c, &EARGOUTOFRANGE, (*res)->loc, "argument count cannot be greater than 65536");
        else
            as(popc, *res)->argc = as(popc, *res)->par.val.u;
        if (!cParseCptr(c, "::"))
            cAddDgn(c, &EMISSINGTOKEN, "::");
        if (!parsePar(c, &as(popc, *res)->par2, FUINT, r))
            cAddDgn(c, &EMISSINGSYNTAX, "return count of function");
        if (as(popc, *res)->par.val.u > max(u16))
            cAddDgnLoc(c, &EARGOUTOFRANGE, (*res)->loc, "return count cannot be greater than 65536");
        else
            as(popc, *res)->retc = as(popc, *res)->par2.val.u;
    } else if (op == OPIF || op == OPTRY || op == OPWHILE) {
        *res = as(opc, new(bopc));
        (*res)->loc = o;
        (*res)->op = op;
        cParseAllCS(c, whitespace);
        if (op == OPIF)
            parseIf(c, as(bopc*, res), r);
        else if (op == OPWHILE) {
            if (parseHead(c, &as(bopc, *res)->head, r))
                cParseAllCS(c, whitespace);
            if (!stops(*res) && !parseBody(c, &as(bopc, *res)->body, r))
                cAddDgn(c, &EMISSINGSYNTAX, "body of while loop");
        } else if (op == OPTRY) {
            if (parseBody(c, &as(bopc, *res)->body, r))
                cParseAllCS(c, whitespace);
            else
                cAddDgn(c, &EMISSINGSYNTAX, "body of try statement");
            if (cParseCptr(c, "|>") || cParseCptr(c, ".catch")) {
                cParseAllCS(c, whitespace);
                if (!parseBody(c, &as(bopc, *res)->els, r))
                    cAddDgn(c, &EMISSINGSYNTAX, "body of catch statement");
            }
        }
    }
    return s.len != 0 || as(popc, *res)->par.kind != KNONE;
}
static bool parseHead(context* c, body* res, u r) {
    reset(res, body);
    res->loc = c->loc;
    bool t = true;
    while (t) {
        opc* op;
        if (parseOPC(c, &op, r)) {
            opcPtrListAdd(&res->ops, op);
            if (stops(op)) {
                cParseAllCS(c, whitespace);
                loc o = c->loc;
                if (cParseAllCS(c, token) || skipBody(c, NULL))
                    cAddDgnEmptyLoc(c, &WUNREACHCODE, o);
                while (cParseAllCS(c, whitespace) || cParseAllCS(c, token) || skipBody(c, NULL));
                res->flags |= FSTOPS;
                t = false;
            }
        } else
            t = parseToken(c);
        if (t)
            cParseAllCS(c, whitespace);
    }
    res->flags |= FPARSED;
    return res->loc.cr != c->loc.cr;
}
bool parseBody(context* c, body* res, u r) {
    loc o = c->loc;
    if (!cParseC(c, '{'))
        return false;
    cParseAllCS(c, whitespace);
    parseHead(c, res, r);
    res->loc = o;
    if (!cParseC(c, '}'))
        cAddDgn(c, &EMISSINGTOKEN, "}");
    return true;
}
static bool skipBody(context* c, body* res) {
    if (res)
        res->loc = c->loc;
    if (!cParseC(c, '{'))
        return false;
    while (cParseAllCS(c, token) || cParseAllCS(c, whitespace) || skipBody(c, NULL));
    if (!cParseC(c, '}'))
        cAddDgn(c, &EMISSINGTOKEN, "}");
    if (res)
        res->text = cCodeFrom(c, res->loc);
    return true;
}
void completeBody(context* c, body* b, u r) {
    loc tmpL = c->loc;
    string tmpT = c->text;
    c->loc = b->loc;
    c->loc.cr = 0;
    c->text = b->text;
    parseBody(c, b, r);
    free(c->text.items);
    c->loc = tmpL;
    c->text = tmpT;
}

static bool parseGlbDef(context* c) {
    if (!cParseCptr(c, "<>"))
        return false;
    varDef v = {0};
    if (!parseVarDef(c, &v))
        cAddDgn(c, &EMISSINGSYNTAX, "variable definition");
    else {
        u r = getGlb(c, v.name.sign, false);
        if (c->glbs.items[r].flags & FDEFINED)
            cAddDgnLoc(c, &ESECONDDECLARATION, v.name.loc, cptr(v.name.sign));
        c->glbs.items[r] = v;
        if (!cParseC(c, ';'))
            cAddDgn(c, &EMISSINGTOKEN, ";");
    }
    return true;
}
static bool parseTypDef(context* c) {
    if (!cParseCptr(c, "::"))
        return false;
    name n = {0};
    if (!parseIdfr(c, &n))
        cAddDgn(c, &EMISSINGSYNTAX, "type identifier");
    u r = getTyp(c, n.sign, false);
    if (c->typs.items[r].flags & FDEFINED)
        cAddDgnLoc(c, &ESECONDDECLARATION, n.loc, cptr(n.sign));
    else
        c->typs.items[r].name = n;
    cParseAllCS(c, whitespace);
    list(att) al = {0};
    if (parseAttList(c, &al))
        cParseAllCS(c, whitespace);
    if ((c->typs.items[r].flags & FDEFINED) == 0)
        c->typs.items[r].attrs = al;
    list(varDef) vl = {0};
    if (parseVarList(c, &vl)) {
        cParseAllCS(c, whitespace);
        if ((c->typs.items[r].flags & FDEFINED) == 0) {
            c->typs.items[r].flds = vl;
            c->typs.items[r].size = 8;
        }
    } else {
        if (!cParseC(c, '('))
            cAddDgn(c, &EMISSINGTOKEN, "(");
        u u;
        if (parseUL(c, &u)) {
            if (u == 1 || u == 2 || u == 4 || u == 8)
                c->typs.items[r].size = u;
            else
                cAddDgnEmpty(c, &EWRONGSIZE);
        } else
            cAddDgn(c, &EMISSINGSYNTAX, "fields or size of type");
        if (!cParseC(c, ')'))
            cAddDgn(c, &EMISSINGTOKEN, ")");
        cParseAllCS(c, whitespace);
    }
    if (!cParseC(c, ';'))
        cAddDgn(c, &EMISSINGTOKEN, ";");
    c->typs.items[r].flags |= FDEFINED;
    return true;
}
static bool parseFunDef(context* c) {
    if (!cParseCptr(c, "()"))
        return false;
    name n = {0};
    if (!parseIdfr(c, &n))
        cAddDgn(c, &EMISSINGSYNTAX, "function identifier");
    cParseAllCS(c, whitespace);
    list(varDef) vars = {0};
    if (!parseVarList(c, &vars))
        cAddDgn(c, &EMISSINGSYNTAX, "argument list of function");
    else
        cParseAllCS(c, whitespace);
    list(ref) rets = {0};
    string typlist = {0};
    if (cParseCptr(c, "::")) {
        cParseAllCS(c, whitespace);
        if (parseTypList(c, &typlist, &rets))
            cParseAllCS(c, whitespace);
    }
    stringAdd(&n.sign, '(');
    for (u i = 0; i < vars.len; i++) {
        if (i != 0)
            stringAdd(&n.sign, ',');
        stringAddRange(&n.sign, c->typs.items[vars.items[i].type.i].name.sign);
    }
    catCptr(&n.sign, ")::");
    stringAddRange(&n.sign, typlist);
    u r = getFun(c, n.sign, false);
    if (c->funs.items[r].flags & FDEFINED)
        cAddDgnLoc(c, &ESECONDDECLARATION, n.loc, cptr(n.sign));
    else {
        c->funs.items[r].name = n;
        c->funs.items[r].args = vars;
        c->funs.items[r].ret = rets;
    }
    reset(&vars, list(varDef));
    if (parseVarList(c, &vars))
        cParseAllCS(c, whitespace);
    if ((c->funs.items[r].flags & FDEFINED) == 0)
        c->funs.items[r].locs = vars;
    list(att) a = {0};
    if (parseAttList(c, &a))
        cParseAllCS(c, whitespace);
    if ((c->funs.items[r].flags & FDEFINED) == 0)
        c->funs.items[r].attrs = a;
    body b = {0};
    if (export(c, as(def, &c->funs.items[r]))) {
        if (parseBody(c, &b, r)) {
            if ((c->funs.items[r].flags & FDEFINED) == 0)
                c->funs.items[r].body = b;
        } else
            cAddDgn(c, &EMISSINGSYNTAX, "body of function");
    } else {
        if (skipBody(c, &b)) {
            if ((c->funs.items[r].flags & FDEFINED) == 0)
                c->funs.items[r].body = b;
        } else
            cAddDgn(c, &EMISSINGSYNTAX, "body of function");
    }
    c->funs.items[r].flags |= FDEFINED;
    return true;
}

static void parseFile(context* c, u f) {
    c->loc.file = f;
    c->text = readAllText(c->inputs.items[f]);
    c->loc.cl = 0;
    c->loc.ln = 1;
    c->loc.cr = 0;
    preprocess(c);
    cParseAllCS(c, whitespace);
    attList a = {0};
    if (parseAttList(c, &a)) {
        for (u i = 0; i < a.len; i++)
            if (a.items[i].kind == ATTUSE && a.items[i].par.kind == KSTR)
                addFile(c, c->strs.items[a.items[i].par.val.r.i], a.items[i].par.loc);
        attListAddRange(&c->atts, a);
        cParseAllCS(c, whitespace);
    }
    while(cNeof(c)) {
        if (!(parseFunDef(c) || parseTypDef(c) || parseGlbDef(c))) {
            loc o = c->loc;
            cParseAllNot(c, whitespace);
            cAddDgnLoc(c, &EUNRECTOKEN, o, cptr(cCodeFrom(c, o)));
        }
        cParseAllCS(c, whitespace);
    }
    free(c->text.items);
}
void parse(context* c) {
    for (u f = 0; f < c->inputs.len; f++)
        parseFile(c, f);
    c->loc.file = c->inputs.len;
}
