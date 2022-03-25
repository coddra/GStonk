#include "h/parser.h"
#include "h/preprocessor.h"
#include "h/linker.h"
#include "h/opcodes.h"
#include "h/diagnostics.h"

setDeclareDefaultName(char, string);
setDefineDefaultName(char, string);
setDefineVaListInt(char);
static bool parseHead(context* c, body* res, u r);
static bool skipBody(context* c, body* res);

set(char)* whitespace = NULL;
set(char)* letters = NULL;
set(char)* digits = NULL;
set(char)* hexDigits = NULL;
set(char)* octDigits = NULL;
set(char)* specialChars = NULL;
set(char)* escChars = NULL;
set(char)* newLine = NULL;
set(char)* stringLiteral = NULL;
set(char)* charLiteral = NULL;
set(char)* notWhitespace = NULL;
set(char)* opSymbols = NULL;
set(char)* modifiers = NULL;
set(char)* token = NULL;

list(string) consts = {0};

const int tabWidth = 4;

void init(PARSER) {
    whitespace = charAggregateFromArray(" \t\n", 3);
    letters = charSetAdd(charRangeNew('a', 'z'), charRangeNew('A', 'Z'));
    digits = charRangeNew('0', '9');
    hexDigits = charSetAdd(charSetAdd(charRangeNew('0', '9'), charRangeNew('a', 'f')), charRangeNew('A', 'F'));
    octDigits = charRangeNew('0', '7');
    specialChars = charAggregateFromArray("_<>", 3);
    escChars = charAggregateFromArray("abfnrtv\'\"\\?", 11);
    newLine = charAggregateFromVaList(1, '\n');//preprocessor will normalize line endings
    stringLiteral = charSetComplement(charAggregateFromArray("\n\\\"", 3));
    charLiteral = charSetComplement(charAggregateFromArray("\n\\\'", 3));
    notWhitespace = charSetComplement(whitespace);
    opSymbols = charAggregateFromArray("~!@%^&*()-_=+\\|:;,<.>/?`#", 27);
    modifiers = charAggregateFromArray("-`#", 3);
    token = charSetSubstract(notWhitespace, charAggregateFromArray("{}", 2));
    stringListAdd(&consts, statstr("$stdin"));
    stringListAdd(&consts, statstr("$stdout"));
    stringListAdd(&consts, statstr("$stderr"));
}

static bool next(context* c) {
    if (c->loc.cr >= c->text.len)
        return false;
    if (c->text.items[c->loc.cr] == '\n') {
        c->loc.ln++;
        c->loc.cl = 0;
    } else if (c->text.items[c->loc.cr] == '\t')
        c->loc.cl += tabWidth;
    else
        c->loc.cl++;
    c->loc.cr++;
    return true;
}
static bool prev(context* c) {
    if (c->loc.cr == 0 || c->text.items[c->loc.cr - 1] == '\n')
        return false;
    c->loc.cr--;
    if (c->text.items[c->loc.cr] == '\t')
        c->loc.cl -= tabWidth;
    else
        c->loc.cl--;
    return true;
}
static inline bool eof(context* c) {
    return c->text.len >= c->loc.cr;
}
static inline bool neof(context* c) {
    return c->loc.cr < c->text.len;
}
static inline bool parseC(context* c, char ch) {
    return neof(c) && c->text.items[c->loc.cr] == ch && next(c);
}
static inline bool parseCS(context* c, set(char)* cs) {
    return neof(c) && charSetContains(cs, c->text.items[c->loc.cr]) && next(c);
}
static inline bool parseCSR(context* c, set(char)* cs, char* res) {
    if (res)
        *res = c->text.items[c->loc.cr];
    return neof(c) && charSetContains(cs, c->text.items[c->loc.cr]) && next(c);
}
static inline bool parseAllCS(context* c, set(char)* cs) {
    bool res = false;
    while (neof(c) && charSetContains(cs, c->text.items[c->loc.cr]) && next(c))
        res = true;
    return res;
}
static inline bool parseAllNot(context* c, set(char)* cs) {
    bool res = false;
    while (neof(c) && !charSetContains(cs, c->text.items[c->loc.cr]) && next(c))
        res = true;
    return res;
}
static bool parseCptr(context* c, char* s) {
    loc o = c->loc;
    u i = 0;
    while (neof(c) && s[i] != 0 && c->text.items[c->loc.cr] == s[i] && next(c))
        i++;
    if (s[i] == 0)
        return true;
    c->loc = o;
    return false;
}

static bool parseHex(context* c, u64* res, u8 digits) {
    loc o = c->loc;
    if (digits == 0){
        if (!parseAllCS(c, hexDigits))
            return false;
    } else
        for (u i = 0; i < digits; i++)
            if (!parseCS(c, hexDigits)) {
                c->loc = o;
                return false;
            }
    if (res)
        *res = strtoul(cptrify(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL, 16);
    return true;
}
static bool parseES(context* c, char* res) {
    loc o = c->loc;
    if (!parseC(c, '\\'))
        return false;
    char r = 0;
    if (parseCSR(c, escChars, &r)) {
        if (res)
            *res = getEscChar(r);
    } else if (parseC(c, 'x') || parseC(c, 'X')) {
        u64 h = 0;
        if (!parseHex(c, &h, 2))
            addDgn(c, EUNRECESCSEQ, cptrify(codeFrom(c, o)));
        if (res)
            *res = *(u8*)h;
    } else
        addDgn(c, EUNRECESCSEQ, cptrify(codeFrom(c, o)));
    return true;
}
static inline bool parseCC(context* c, char* res, bool str) {
    return parseES(c, res) || parseCSR(c, str ? stringLiteral : charLiteral, res);
}
static inline bool parseCL(context* c, char* res) {
    return parseC(c, '\'') && parseCC(c, res, false) && parseC(c, '\'');
}
static bool parseIL(context* c, i64* res) {
    loc o = c->loc;
    parseC(c, '-');
    if (!parseAllCS(c, digits)) {
        c->loc = o;
        return false;
    }
    if (res)
        *res = strtol(cptrify(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL, 10);
    return true;
}
static bool parseUL(context* c, u64* res) {
    loc o = c->loc;
    if (parseC(c, '0')) {
        if (parseC(c, 'x') || parseC(c, 'X')) {
            if (!parseHex(c, res, 0))
                addDgn(c, EMISSINGSYNTAX, "value of hexadecimal number");
        } else {
            o = c->loc;
            if (parseAllCS(c, octDigits)) {
                if (res)
                    *res = strtoul(cptrify(codeFrom(c, o)), NULL, 8);
            } else {
                if (res)
                    *res = 0;
            }
        }
    } else if (parseAllCS(c, digits)) {
        if (res)
            *res = strtoul(cptrify(codeFrom(c, o)), NULL, 10);
    } else if (parseCL(c, (char*)res));
    else if (parseC(c, '$') && parseAllCS(c, letters)) {
        u i = stringListPos(consts, codeFrom(c, o));
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
    parseC(c, '-');
    if (!parseAllCS(c, digits) || !parseC(c, '.') || !parseAllCS(c, digits)) {
        c->loc = o;
        return false;
    }
    if (res)
        *res = strtod(cptrify(stringGetRange(c->text, o.cr, c->loc.cr - o.cr)), NULL);
    return true;
}
static bool parseSL(context* c, ref* res) {
    string s = {0};
    if (res)
        res->loc = c->loc;
    if (!parseC(c, '"'))
        return false;
    char r;
    bool dotflag = true;
    while (dotflag) {
        while (parseCC(c, &r, true))
            stringAdd(&s, r);
        if (!parseC(c, '"'))
            addDgn(c, EMISSINGTOKEN, "\"");
        dotflag = parseC(c, '.');
        if (dotflag) {
            parseAllCS(c, whitespace);
            if (!parseC(c, '"'))
                addDgn(c, EMISSINGTOKEN, "\"");
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
    if (!parseCS(c, specialChars) && !parseCS(c, letters))
        return false;
    while (parseCS(c, specialChars) || parseCS(c, letters) || parseCS(c, digits));
    if (res)
        res->sign = stringGetRange(c->text, res->loc.cr, c->loc.cr - res->loc.cr);
    return true;
}
static bool parseIdfr(context* c, name* res) {
    if (res)
        res->loc = c->loc;
    if (!parseName(c, NULL))
        return false;
    while (parseC(c, '.') && parseName(c, NULL));
    if (c->text.items[c->loc.cr - 1] == '.')
        prev(c);
    if (res)
        res->sign = codeFrom(c, res->loc);
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
    parseAllCS(c, whitespace);
    while (parseC(c, ',')) {
        stringAdd(res, ',');
        parseAllCS(c, whitespace);
        if (parseTypIdfr(c, &n)) {
            if (res)
                stringAddRange(res, n.sign);
            i = getTyp(c, n.sign, true);
            if (refs) {
                ref r = { i, n.loc };
                refListAdd(refs, r);
            }
            parseAllCS(c, whitespace);
        }
        else
            addDgn(c, EMISSINGSYNTAX, "type identifier");
    }
    return true;
}
static bool parseFunIdfr(context* c, name* res) {
    loc o = c->loc;
    if (!parseIdfr(c, res))
        return false;
    name n = {0};
    if (!parseC(c, '(')) {
        c->loc = o;
        return false;
    }
    if (res)
        stringAdd(&res->sign, '(');
    parseAllCS(c, whitespace);
    if (parseTypList(c, &n.sign, NULL) && res)
        stringAddRange(&res->sign, n.sign);
    if (res)
        stringAdd(&res->sign, ')');
    if (!parseC(c, ')'))
        addDgn(c, EMISSINGTOKEN, ")");
    if (res) {
        stringAdd(&res->sign, ':');
        stringAdd(&res->sign, ':');
    }
    if (!parseCptr(c, "::"))
        addDgn(c, EMISSINGTOKEN, "::");
    if (parseTypList(c, &n.sign, NULL) && res)
        stringAddRange(&res->sign, n.sign);
    return true;
}
static bool parseVarIdfr(context* c, name* res) {
    loc o = c->loc;
    if (!parseIdfr(c, res))
        return false;
    if (!parseCptr(c, "::")) {
        c->loc = o;
        return false;
    }
    if (res) {
        stringAdd(&res->sign, ':');
        stringAdd(&res->sign, ':');
    }
    name n = {0};
    if (!parseTypIdfr(c, &n))
        addDgn(c, EMISSINGSYNTAX, "type identifier");
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
        if ((kind & FFLD) > 0 && parseCptr(c, "->")) {
            if (!parseVar(c, &res[1].val.r, &c->typs.items[res->val.r.i].flds))
                addDgn(c, EMISSINGTOKEN, "->");
            else
                res->kind = KFLD;
        } else
            res->kind = KTYP;
    }
    else if ((kind & FLOC) > 0 && parseC(c, '`')) {
        res->val.r.loc = c->loc;
        if (parseUL(c, &res->val.r.i)) {
            if (res->val.r.i >= c->funs.items[r].locs.len)
                addDgn(c, EARGOUTOFRANGE, "local variable number");
            else
                c->funs.items[r].locs.items[res->val.r.i].flags |= FREFERENCED;
        } else if (parseVar(c, &res->val.r, &c->funs.items[r].locs))
            c->funs.items[r].locs.items[res->val.r.i].flags |= FREFERENCED;
        else
            addDgn(c, EMISSINGSYNTAX, "index or name of local variable");
        res->kind = KLOC;
    } else if ((kind & FARG) > 0 && parseC(c, '#')) {
        res->val.r.loc = c->loc;
        if (parseUL(c, &res->val.r.i)) {
            if (res->val.r.i >= c->funs.items[r].args.len)
                addDgn(c, EARGOUTOFRANGE, "argument number");
            else
                c->funs.items[r].args.items[res->val.r.i].flags |= FREFERENCED;
        } else if (parseVar(c, &res->val.r, &c->funs.items[r].args))
            c->funs.items[r].args.items[res->val.r.i].flags |= FREFERENCED;
        else
            addDgn(c, EMISSINGSYNTAX, "index or name of argument");
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
    if (!parseAllCS(c, letters))
        return false;
    res->kind = getAtt(codeFrom(c, res->loc));
    if (res->kind == ATTCOUNT)
        addDgnLoc(c, EUNRECATT, res->loc, cptrify(codeFrom(c, res->loc)));
    parseAllCS(c, whitespace);
    if (!parseC(c, '(')) {
        if (ATTRIBUTES[res->kind].arg != FNONE)
            addDgn(c, EMISSINGSYNTAX, "parameter for attribute");
        return true;
    }
    par p;
    if (parsePar(c, &p, FATTANY, 0)) {
        if (ATTRIBUTES[res->kind].arg & kindToFlag(p.kind))
            res->par = p;
        else
            addDgnLoc(c, EWRONGPAR, p.loc, ATTRIBUTES[res->kind].name);
    } else if (ATTRIBUTES[res->kind].arg != FNONE)
        addDgn(c, EMISSINGSYNTAX, "parameter for attribute");
    if (!parseC(c, ')'))
        addDgn(c, EMISSINGTOKEN, ")");
    return true;
}
static bool parseAttList(context* c, list(att)* res) {
    if (!parseC(c, '['))
        return false;
    parseAllCS(c, whitespace);
    att a = {0};
    bool commaflag = true;
    while (parseAtt(c, &a)) {
        attListAdd(res, a);
        if (!commaflag)
            addDgnLoc(c, EMISSINGTOKEN, a.loc, ",");
        parseAllCS(c, whitespace);
        commaflag = parseC(c, ',');
        if (commaflag)
            parseAllCS(c, whitespace);
    }
    if (!parseC(c, ']'))
        addDgn(c, EMISSINGTOKEN, "]");
    return true;
}

static bool parseVarDef(context* c, varDef* res) {
    if (!parseIdfr(c, &res->name))
        return false;
    if (!parseCptr(c, "::"))
        addDgn(c, EMISSINGTOKEN, "::");
    stringAdd(&res->name.sign, ':');
    stringAdd(&res->name.sign, ':');
    name n = {0};
    if (!parseTypIdfr(c, &n))
        addDgn(c, EMISSINGSYNTAX, "type of variable");
    stringAddRange(&res->name.sign, n.sign);
    res->type.loc = n.loc;
    res->type.i = getTyp(c, n.sign, true);
    res->flags |= FDEFINED;
    parseAllCS(c, whitespace);
    parseAttList(c, &res->attrs);
    return true;
}
static bool parseVarList(context* c, list(varDef)* res) {
    loc o = c->loc;
    if (!parseC(c, '('))
        return false;
    parseAllCS(c, whitespace);
    varDef v = {0};
    bool commaflag = true;
    while (parseVarDef(c, &v)) {
        u r = getVar(res, v.name.sign, false);
        if ((res->items[r].flags & FDEFINED))
            addDgn(c, ESECONDDECLARATION, cptrify(v.name.sign));
        res->items[r] = v;
        if (!commaflag)
            addDgnLoc(c, EMISSINGTOKEN, v.name.loc, ",");
        parseAllCS(c, whitespace);
        commaflag = parseC(c, ',');
        if (commaflag)
            parseAllCS(c, whitespace);
    }
    if (!parseC(c, ')')) {
        if (res->len > 0)
            addDgn(c, EMISSINGTOKEN, ")");
        else
            c->loc = o;
    }
    return c->loc.cr != o.cr;
}

static bool parseToken(context* c) {
    loc o = c->loc;
    bool res = parseAllCS(c, token);
    if (res)
        addDgnLoc(c, EUNRECTOKEN, o, cptrify(codeFrom(c, o)));
    return res;
}
static bool parseIf(context* c, bopc** res, u r) {
    parseHead(c, &(*res)->head, r);
    if (stops(as(opc, *res)))
        return true;
    if (parseBody(c, &(*res)->body, r))
        parseAllCS(c, whitespace);
    else
        addDgn(c, EMISSINGSYNTAX, "body of if statement");
    loc o = c->loc;
    if (parseCptr(c, "|?") || parseCptr(c, ".elif")) {
        parseAllCS(c, whitespace);
        bopc* b = new(bopc);
        b->loc = o;
        b->op = OPIF;
        parseIf(c, &b, r);
        if (stops(as(opc, b)))
            (*res)->els.flags |= FSTOPS;
        opcPtrListAdd(&(*res)->els.ops, as(opc, b));
    } else if (parseCptr(c, "|>") || parseCptr(c, ".else")) {
        parseAllCS(c, whitespace);
        parseBody(c, &(*res)->els, r);
    }
    return true;
}
static bool parseOPC(context* c, opc** res, u r) {
    loc o = c->loc;
    if (!parseC(c, '.') || !parseAllCS(c, letters))
            parseAllCS(c, opSymbols);
    if (charSetContains(modifiers, c->text.items[c->loc.cr - 1]) && parseCS(c, notWhitespace)) {
        prev(c);
        prev(c);
    }
    string s = codeFrom(c, o);
    if (s.len > 0 && charSetContains(letters, s.items[s.len - 1]))
        parseC(c, '.');
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
            addDgnLoc(c, EARGOUTOFRANGE, (*res)->loc, "argument count cannot be greater than 65536");
        else
            as(popc, *res)->argc = as(popc, *res)->par.val.u;
        if (!parseCptr(c, "::"))
            addDgn(c, EMISSINGTOKEN, "::");
        if (!parsePar(c, &as(popc, *res)->par2, FUINT, r))
            addDgn(c, EMISSINGSYNTAX, "return count of function");
        if (as(popc, *res)->par.val.u > max(u16))
            addDgnLoc(c, EARGOUTOFRANGE, (*res)->loc, "return count cannot be greater than 65536");
        else
            as(popc, *res)->retc = as(popc, *res)->par2.val.u;
    } else if (op == OPIF || op == OPTRY || op == OPWHILE) {
        *res = as(opc, new(bopc));
        (*res)->loc = o;
        (*res)->op = op;
        parseAllCS(c, whitespace);
        if (op == OPIF)
            parseIf(c, as(bopc*, res), r);
        else if (op == OPWHILE) {
            if (parseHead(c, &as(bopc, *res)->head, r))
                parseAllCS(c, whitespace);
            if (!stops(*res) && !parseBody(c, &as(bopc, *res)->body, r))
                addDgn(c, EMISSINGSYNTAX, "body of while loop");
        } else if (op == OPTRY) {
            if (parseBody(c, &as(bopc, *res)->body, r))
                parseAllCS(c, whitespace);
            else
                addDgn(c, EMISSINGSYNTAX, "body of try statement");
            if (parseCptr(c, "|>") || parseCptr(c, ".catch")) {
                parseAllCS(c, whitespace);
                if (!parseBody(c, &as(bopc, *res)->els, r))
                    addDgn(c, EMISSINGSYNTAX, "body of catch statement");
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
                parseAllCS(c, whitespace);
                loc o = c->loc;
                if (parseAllCS(c, token) || skipBody(c, NULL))
                    addDgnEmptyLoc(c, WUNREACHCODE, o);
                while (parseAllCS(c, whitespace) || parseAllCS(c, token) || skipBody(c, NULL));
                res->flags |= FSTOPS;
                t = false;
            }
        } else
            t = parseToken(c);
        if (t)
            parseAllCS(c, whitespace);
    }
    res->flags |= FPARSED;
    return res->loc.cr != c->loc.cr;
}
bool parseBody(context* c, body* res, u r) {
    loc o = c->loc;
    if (!parseC(c, '{'))
        return false;
    parseAllCS(c, whitespace);
    parseHead(c, res, r);
    res->loc = o;
    if (!parseC(c, '}'))
        addDgn(c, EMISSINGTOKEN, "}");
    return true;
}
static bool skipBody(context* c, body* res) {
    if (res)
        res->loc = c->loc;
    if (!parseC(c, '{'))
        return false;
    while (parseAllCS(c, token) || parseAllCS(c, whitespace) || skipBody(c, NULL));
    if (!parseC(c, '}'))
        addDgn(c, EMISSINGTOKEN, "}");
    if (res)
        res->text = codeFrom(c, res->loc);
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
    if (!parseCptr(c, "<>"))
        return false;
    varDef v = {0};
    if (!parseVarDef(c, &v))
        addDgn(c, EMISSINGSYNTAX, "variable definition");
    else {
        u r = getGlb(c, v.name.sign, false);
        if (c->glbs.items[r].flags & FDEFINED)
            addDgnLoc(c, ESECONDDECLARATION, v.name.loc, cptrify(v.name.sign));
        c->glbs.items[r] = v;
        if (!parseC(c, ';'))
            addDgn(c, EMISSINGTOKEN, ";");
    }
    return true;
}
static bool parseTypDef(context* c) {
    if (!parseCptr(c, "::"))
        return false;
    name n = {0};
    if (!parseIdfr(c, &n))
        addDgn(c, EMISSINGSYNTAX, "type identifier");
    u r = getTyp(c, n.sign, false);
    if (c->typs.items[r].flags & FDEFINED)
        addDgnLoc(c, ESECONDDECLARATION, n.loc, cptrify(n.sign));
    else
        c->typs.items[r].name = n;
    parseAllCS(c, whitespace);
    list(att) al = {0};
    if (parseAttList(c, &al))
        parseAllCS(c, whitespace);
    if ((c->typs.items[r].flags & FDEFINED) == 0)
        c->typs.items[r].attrs = al;
    list(varDef) vl = {0};
    if (parseVarList(c, &vl)) {
        parseAllCS(c, whitespace);
        if ((c->typs.items[r].flags & FDEFINED) == 0) {
            c->typs.items[r].flds = vl;
            c->typs.items[r].size = 8;
        }
    } else {
        if (!parseC(c, '('))
            addDgn(c, EMISSINGTOKEN, "(");
        u u;
        if (parseUL(c, &u)) {
            if (u == 1 || u == 2 || u == 4 || u == 8)
                c->typs.items[r].size = u;
            else
                addDgnEmpty(c, EWRONGSIZE);
        } else
            addDgn(c, EMISSINGSYNTAX, "fields or size of type");
        if (!parseC(c, ')'))
            addDgn(c, EMISSINGTOKEN, ")");
        parseAllCS(c, whitespace);
    }
    if (!parseC(c, ';'))
        addDgn(c, EMISSINGTOKEN, ";");
    c->typs.items[r].flags |= FDEFINED;
    return true;
}
static bool parseFunDef(context* c) {
    if (!parseCptr(c, "()"))
        return false;
    name n = {0};
    if (!parseIdfr(c, &n))
        addDgn(c, EMISSINGSYNTAX, "function identifier");
    parseAllCS(c, whitespace);
    list(varDef) vars = {0};
    if (!parseVarList(c, &vars))
        addDgn(c, EMISSINGSYNTAX, "argument list of function");
    else
        parseAllCS(c, whitespace);
    list(ref) rets = {0};
    string typlist = {0};
    if (parseCptr(c, "::")) {
        parseAllCS(c, whitespace);
        if (parseTypList(c, &typlist, &rets))
            parseAllCS(c, whitespace);
    }
    stringAdd(&n.sign, '(');
    for (u i = 0; i < vars.len; i++) {
        if (i != 0)
            stringAdd(&n.sign, ',');
        stringAddRange(&n.sign, c->typs.items[vars.items[i].type.i].name.sign);
    }
    addCptr(&n.sign, ")::");
    stringAddRange(&n.sign, typlist);
    u r = getFun(c, n.sign, false);
    if (c->funs.items[r].flags & FDEFINED)
        addDgnLoc(c, ESECONDDECLARATION, n.loc, cptrify(n.sign));
    else {
        c->funs.items[r].name = n;
        c->funs.items[r].args = vars;
        c->funs.items[r].ret = rets;
    }
    reset(&vars, list(varDef));
    if (parseVarList(c, &vars))
        parseAllCS(c, whitespace);
    if ((c->funs.items[r].flags & FDEFINED) == 0)
        c->funs.items[r].locs = vars;
    list(att) a = {0};
    if (parseAttList(c, &a))
        parseAllCS(c, whitespace);
    if ((c->funs.items[r].flags & FDEFINED) == 0)
        c->funs.items[r].attrs = a;
    body b = {0};
    if (export(c, as(def, &c->funs.items[r]))) {
        if (parseBody(c, &b, r)) {
            if ((c->funs.items[r].flags & FDEFINED) == 0)
                c->funs.items[r].body = b;
        } else
            addDgn(c, EMISSINGSYNTAX, "body of function");
    } else {
        if (skipBody(c, &b)) {
            if ((c->funs.items[r].flags & FDEFINED) == 0)
                c->funs.items[r].body = b;
        } else
            addDgn(c, EMISSINGSYNTAX, "body of function");
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
    parseAllCS(c, whitespace);
    attList a = {0};
    if (parseAttList(c, &a)) {
        for (u i = 0; i < a.len; i++)
            if (a.items[i].kind == ATTUSE && a.items[i].par.kind == KSTR)
                addFile(c, c->strs.items[a.items[i].par.val.r.i], a.items[i].par.loc);
        attListAddRange(&c->atts, a);
        parseAllCS(c, whitespace);
    }
    while(neof(c)) {
        if (!(parseFunDef(c) || parseTypDef(c) || parseGlbDef(c))) {
            loc o = c->loc;
            parseAllNot(c, whitespace);
            addDgnLoc(c, EUNRECTOKEN, o, cptrify(codeFrom(c, o)));
        }
        parseAllCS(c, whitespace);
    }
    free(c->text.items);
}
void parse(context* c) {
    for (u f = 0; f < c->inputs.len; f++)
        parseFile(c, f);
    c->loc.file = c->inputs.len;
}
