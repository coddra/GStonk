#include "h/opcodes.h"
#include "h/diagnostics.h"
#include "h/linker.h"
#include "h/compiler.h"

#define link(OP)                                \
    void link##OP(context* c, opc* o, u f, i64* s)
#define compile(OP)                             \
    string compile##OP(context* c, opc* op, u f)

link(LDAT) {
    if (as(popc, o)->par.val.i == -8)
        as(popc, o)->par.val.i = 8;
    if (as(popc, o)->par.val.i != -4 && as(popc, o)->par.val.i != -2 && as(popc, o)->par.val.i != -1 &&
        as(popc, o)->par.val.i != 1 && as(popc, o)->par.val.i != 2 && as(popc, o)->par.val.i != 4 && as(popc, o)->par.val.i != 8)
        addDgnEmpty(c, EARGOUTOFRANGE);
}
link(STAT) {
    if (as(popc, o)->par.val.u != 1 && as(popc, o)->par.val.u != 2 && as(popc, o)->par.val.i != 4 && as(popc, o)->par.val.i != 8)
        addDgnEmpty(c, EARGOUTOFRANGE);
}
link(RET) {
    as(popc, o)->argc = c->funs.items[f].ret.len;
    as(popc, o)->retc = 0;
}
link(IF) {
    linkBody(c, &as(bopc, o)->head, f, s);
    if (s == 0)
        addDgnEmptyLoc(c, ESTACKLOW, o->loc);
    else
        (*s)--;
    i64 e = *s;
    linkBody(c, &as(bopc, o)->body, f, s);
    if (as(bopc, o)->els.ops.len != 0)
        linkBody(c, &as(bopc, o)->els, f, &e);
    if (e != *s)
        addDgnEmptyLoc(c, ESTACKUNPRED, o->loc);
}
link(WHILE) {
    i64 h = *s;
    linkBody(c, &as(bopc, o)->head, f, s);
    h = *s - h;
    if (s == 0)
        addDgnEmptyLoc(c, ESTACKLOW, o->loc);
    else
        (*s)--;
    i64 tmp = *s;
    linkBody(c, &as(bopc, o)->body, f, s);
    if (h + *s - tmp != 1)
        addDgnEmptyLoc(c, ESTACKUNPRED, o->loc);
    *s = tmp;
}
link(TRY) {
    i64 s1 = 0;
    linkBody(c, &as(bopc, o)->body, f, &s1);
    i64 s2 = 1;
    if (as(bopc, o)->els.ops.len != 0)
        linkBody(c, &as(bopc, o)->els, f, &s2);
    else
        s2 = 0;
    as(bopc, o)->body.retc = s1;
    if (s1 != s2)
        addDgnEmptyLoc(c, ESTACKUNPRED, o->loc);
    if (s2 < s1)
        s1 = s2;
    s = s + s1;
}
link(EVAL) {
    if (as(popc, o)->par.kind == KFUN) {
        as(popc, o)->argc = c->funs.items[as(popc, o)->par.val.r.i].args.len;
        as(popc, o)->retc = c->funs.items[as(popc, o)->par.val.r.i].ret.len;
    }
}

compile(RET) {
    string res = stringDefault();
    if (c->funs.items[f].ret.len == 0) {
        addCptr(&res, "\tmovq\t\t");
        addCptr(&res, utos(c->funs.items[f].locs.len * 8 + 8));
        addCptr(&res, "(%rbp), %rax\n"
                "\tmovq\t\t%rbp, %rsp\n"
                "\tmovq\t\t(%rbp), %rbp\n"
                "\taddq\t\t$");
        addCptr(&res, utos((c->funs.items[f].locs.len + c->funs.items[f].args.len + 2) * 8));
        addCptr(&res, ", %rsp\n"
                "\tjmp\t\t\t*%rax\n");
    } else {
        addCptr(&res, "\tmovq\t\t");
        addCptr(&res, utos(c->funs.items[f].locs.len * 8 + 8));
        addCptr(&res, "(%rbp), %rax\n"
                "\tmovq\t\t(%rbp), %rbx\n");
        for (u i = 0; i < c->funs.items[f].ret.len; i++) {
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos((c->funs.items[f].ret.len - i - 1) * 8));
            addCptr(&res, "(%rsp), %rcx\n"
                    "\tmovq\t\t%rcx, ");
            addCptr(&res, itos((c->funs.items[f].locs.len + c->funs.items[f].args.len + 1 - i) * 8));
            addCptr(&res, "(%rbp)\n");
        }
        addCptr(&res, "\tmovq\t\t%rbx, %rbp\n"
                "\taddq\t\t$");
        addCptr(&res, utos((c->funs.items[f].locs.len + c->funs.items[f].args.len + 2) * 8));
        addCptr(&res, ", %rsp\n"
                "\tjmp\t\t\t*%rax\n");
    }
    return res;
}
compile(LDADDR) {
    string res = stringDefault();
    if (as(popc, op)->par.kind == KFUN) {
        addCptr(&res, "\tleaq\t\t");
        stringAddRange(&res, c->funs.items[as(popc, op)->par.val.r.i].name.csign);
        addCptr(&res, "(%rip), %rax\n"
                "\tpushq\t\t%rax\n");
    } else if (as(popc, op)->par.kind == KGLB) {
        addCptr(&res, "\tleaq\t\t");
        stringAddRange(&res, c->glbs.items[as(popc, op)->par.val.r.i].name.csign);
        addCptr(&res, "(%rip), %rax\n"
                "\tpushq\t\t%rax\n");
    } else if (as(popc, op)->par.kind == KLOC) {
        addCptr(&res, "\tmovq\t\t%rbp, %rax\n"
                "\taddq\t\t$");
        addCptr(&res, utos(as(popc, op)->par.val.r.i * 8 + 8));
        addCptr(&res, ", %rax\n"
                "\tpusq\t\t%rax\n");
    } else if (as(popc, op)->par.kind == KARG) {
        addCptr(&res, "\tmovq\t\t%rbp, %rax\n"
                "\taddq\t\t$");
        addCptr(&res, utos((c->funs.items[f].locs.len + 1 + c->funs.items[f].args.len - as(popc, op)->par.val.r.i) * 8));
        addCptr(&res, ", %rax\n"
                "\tpusq\t\t%rax\n");
    } else if (as(popc, op)->par.kind == KFLD) {
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\taddq\t\t$");
        addCptr(&res, utos(c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].offset));
        addCptr(&res, ", %rax\n"
                "\tpushq\t\t%rax\n");
    }
    return res;
}
compile(ST) {
    string res = stringDefault();
    if (as(popc, op)->par.kind == KGLB) {
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tmov");
        stringAdd(&res, getPostfix(c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size));
        addCptr(&res, "\t\t%");
        stringAddRange(&res, getRegister('a', c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size));
        addCptr(&res, ", ");
        stringAddRange(&res, c->glbs.items[as(popc, op)->par.val.r.i].name.csign);
        addCptr(&res, "(%rip)\n");
    } else if (as(popc, op)->par.kind == KLOC) {
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tmovq\t\t%rax, ");
        addCptr(&res, utos(as(popc, op)->par.val.r.i * 8 + 8));
        addCptr(&res, "(%rbp)\n");
    } else if (as(popc, op)->par.kind == KARG) {
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tmovq\t\t%rax, ");
        addCptr(&res, utos((c->funs.items[f].args.len + 1 + c->funs.items[f].locs.len - as(popc, op)->par.val.r.i) * 8));
        addCptr(&res, "(%rbp)\n");
    } else if (as(popc, op)->par.kind == KFLD) {
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tpopq\t\t%rbx\n"
                "\tmov");
        stringAdd(&res, getPostfix(c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size));
        addCptr(&res, "\t\t%");
        stringAddRange(&res, getRegister('a', c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size));
        addCptr(&res, ", ");
        addCptr(&res, utos(c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].offset));
        addCptr(&res, "(%rbx)\n");
    }
    return res;
}
compile(EVAL) {
    string res = stringDefault();
    if (as(popc, op)->par.kind == KFUN) {
        addCptr(&res, "\tcall\t\t");
        stringAddRange(&res, c->funs.items[as(popc, op)->par.val.r.i].name.csign);
        stringAdd(&res, '\n');
    } else if (as(popc, op)->par.kind == KGLB) {
        if (c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size < 8 && hasAtt(c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].attrs, ATTSIGNED, NULL)) {
            addCptr(&res, "\tmovq\t\t");
            stringAddRange(&res, c->glbs.items[as(popc, op)->par.val.r.i].name.csign);
            addCptr(&res, "(%rip), %rax\n"
                    "\tshlq\t\t$");
            char* shift = utos((8 - c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size) * 8);
            addCptr(&res, shift);
            addCptr(&res, ", %rax\n"
                    "\tsarq\t\t$");
            addCptr(&res, shift);
            addCptr(&res, ", %rax\n"
                    "\tpushq\t\t%rax\n");
        } else {
            if (c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size < 4)
                addCptr(&res, "\txorl\t\t%eax, %eax\n");
            addCptr(&res, "\tmov");
            stringAdd(&res, getPostfix(c->typs.items[c->glbs.items[as(popc, op)->par.val.r.i].type.i].size));
            addCptr(&res, "\t\t");
            stringAddRange(&res, c->glbs.items[as(popc, op)->par.val.r.i].name.csign);
            addCptr(&res, "(%rip), %");
            stringAddRange(&res, getRegister('a', c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size));
            addCptr(&res, "\n"
                    "\tpushq\t\t%rax\n");
        }
    } else if (as(popc, op)->par.kind == KLOC) {
        addCptr(&res, "\tpushq\t\t");
        addCptr(&res, utos(as(popc, op)->par.val.r.i * 8 + 8));
        addCptr(&res, "(%rbp)\n");
    } else if (as(popc, op)->par.kind == KARG) {
        addCptr(&res, "\tpushq\t\t");
        addCptr(&res, utos((c->funs.items[f].args.len + 1 + c->funs.items[f].locs.len - as(popc, op)->par.val.r.i) * 8));
        addCptr(&res, "(%rbp)\n");
    } else if (as(popc, op)->par.kind == KFLD) {
        if (c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size < 8 && hasAtt(c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].attrs, ATTSIGNED, NULL)) {
            addCptr(&res, "\tpopq\t\t%rbx\n"
                    "\tmovq\t\t");
            addCptr(&res, utos(c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].offset));
            addCptr(&res, "(%rbx), %rax\n"
                    "\tshlq\t\t$");
            char* shift = utos((8 - c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size) * 8);
            addCptr(&res, shift);
            addCptr(&res, ", %rax\n"
                    "\tsarq\t\t$");
            addCptr(&res, shift);
            addCptr(&res, ", %rax\n"
                    "\tpushq\t\t%rax\n");
        } else {
            if (c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size < 4)
                addCptr(&res, "\txorl\t\t%eax, %eax\n");
            addCptr(&res, "\tpopq\t\t%rbx\n"
                    "\tmov");
            stringAdd(&res, getPostfix(c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size));
            addCptr(&res, "\t\t");
            addCptr(&res, utos(c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].offset));
            addCptr(&res, "(%rbx), %");
            stringAddRange(&res, getRegister('a', c->typs.items[c->typs.items[as(popc, op)->par.val.r.i].flds.items[as(popc, op)->par2.val.r.i].type.i].size));
            addCptr(&res, "\n"
                    "\tpushq\t\t%rax\n");
        }
    } else if (as(popc, op)->par.kind == KSTR) {
        addCptr(&res, "\tleaq\t\t.str");
        addCptr(&res, utos(as(popc, op)->par.val.r.i));
        addCptr(&res, "(%rip), %rax\n"
                "\tpushq\t\t%rax\n");
    } else {
        if (as(popc, op)->par.val.i <= 2147483647 && as(popc, op)->par.val.i >= -2147483648) {
            addCptr(&res, "\tpushq\t\t$");
            addCptr(&res, itos(as(popc, op)->par.val.i));
            stringAdd(&res, '\n');
        } else {
            addCptr(&res, "\tmovq\t\t$");
            addCptr(&res, utos(as(popc, op)->par.val.u));
            addCptr(&res, ", %rax\n"
                    "\tpushq\t\t%rax\n");
        }
    }
    return res;
}
compile(LDAT) {
    string res = stringDefault();
    if (as(popc, op)->par.val.i < 0) {
        addCptr(&res, "\tpopq\t\t%rbx\n"
                "\tmovq\t\t(%rbx), %rax\n"
                "\tshlq\t\t$");
        addCptr(&res, utos((8 + as(popc, op)->par.val.i) * 8));
        addCptr(&res, ", %rax\n"
                "\tsarq\t\t$");
        addCptr(&res, utos((8 + as(popc, op)->par.val.i) * 8));
        addCptr(&res, ", %rax\n"
                "\tpushq\t\t%rax\n");
    } else {
        if (as(popc, op)->par.val.i == 1 || as(popc, op)->par.val.i == 2)
            addCptr(&res, "\txorl\t\t%eax, %eax\n");
        addCptr(&res, "\tpopq\t\t%rbx\n"
                "\tmov");
        stringAdd(&res, getPostfix(as(popc, op)->par.val.u));
        addCptr(&res, "\t\t(%rbx), %");
        stringAddRange(&res, getRegister('a', as(popc, op)->par.val.u));
        addCptr(&res, "\n"
                "\tpushq\t\t%rax\n");
    }
    return res;
}
compile(STAT) {
    string res = stringDefault();
    addCptr(&res, "\tpopq\t\t%rbx\n"
            "\tpopq\t\t%rax\n"
            "\tmov");
    stringAdd(&res, getPostfix(as(popc, op)->par.val.u));
    addCptr(&res, "\t\t%");
    stringAddRange(&res, getRegister('b', as(popc, op)->par.val.u));
    addCptr(&res, ", (%rax)\n");
    return res;
}
compile(IF) {
    u addr = c->addr;
    string res = stringDefault();
    for (u i = 0; i < as(bopc, op)->head.ops.len; i++)
        stringAddRange(&res, compOP(c, as(bopc, op)->head.ops.items[i], f));
    string tmp = stringDefault();
    for (u i = 0; i < as(bopc, op)->body.ops.len; i++)
        stringAddRange(&tmp, compOP(c, as(bopc, op)->body.ops.items[i], f));
    addCptr(&res, "\tpopq\t\t%rax\n"
            "\tcmpq\t\t$0, %rax\n"
            "\tje\t\t\t.else");
    addCptr(&res, utos(addr));
    stringAdd(&res, '\n');
    stringAddRange(&res, tmp);
    if (as(bopc, op)->els.ops.len > 0) {
        tmp.len = 0;
        for (u i = 0; i < as(bopc, op)->els.ops.len; i++)
            stringAddRange(&tmp, compOP(c, as(bopc, op)->els.ops.items[i], f));
        addCptr(&res, "\tjmp\t\t\t.endf");
        addCptr(&res, utos(addr));
        stringAdd(&res, '\n');
        addCptr(&res, ".else");
        addCptr(&res, utos(addr));
        addCptr(&res, ":\n");
        stringAddRange(&res, tmp);
        addCptr(&res, ".endf");
        addCptr(&res, utos(addr));
        addCptr(&res, ":\n");
    } else {
        addCptr(&res, ".else");
        addCptr(&res, utos(addr));
        addCptr(&res, ":\n");
    }
    return res;
}
compile(WHILE) {
    string res = stringDefault();
    u64 head = c->addr;
    for (u i = 0; i < as(bopc, op)->head.ops.len; i++)
        stringAddRange(&res, compOP(c, as(bopc, op)->head.ops.items[i], f));
    string tmp = stringDefault();
    for (u i = 0; i < as(bopc, op)->body.ops.len; i++)
        stringAddRange(&tmp, compOP(c, as(bopc, op)->body.ops.items[i], f));
    addCptr(&res, "\tpopq\t\t%rax\n"
            "\tcmpq\t\t$0, %rax\n"
            "\tje\t\t\t.endw");
    addCptr(&res, utos(head));
    stringAdd(&res, '\n');
    stringAddRange(&res, tmp);
    addCptr(&res, "\tjmp\t\t\t.addr");
    addCptr(&res, utos(head));
    stringAdd(&res, '\n');
    addCptr(&res, ".endw");
    addCptr(&res, utos(head));
    addCptr(&res, ":\n");
    return res;
}
compile(TRY) {
    string res = stringDefault();
    addCptr(&res, "\tleaq\t\t.addr");
    u addr = c->addr++;
    addCptr(&res, utos(addr));
    addCptr(&res, "(%rip), %rax\n"
            "\tpushq\t\t%rax\n"
            "\tpushq\t\t%rbp\n"
            "\tmovq\t\t.excrsp(%rip), %rax\n"
            "\tpushq\t\t%rax\n"
            "\tmovq\t\t%rsp, .excrsp(%rip)\n");
    for (u i = 0; i < as(bopc, op)->body.ops.len; i++)
        stringAddRange(&res, compOP(c, as(bopc, op)->body.ops.items[i], f));
    addCptr(&res, "\tmovq\t\t.excrsp(%rip), %rax\n"
            "\tmovq\t\t(%rax), %rax\n"
            "\tmovq\t\t%rax, .excrsp(%rip)\n");
    if (as(bopc, op)->body.retc > 0) {
        for (u i = 1; i <= (u)as(bopc, op)->body.retc; i++) {
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos((as(bopc, op)->body.retc - i) * 8));
            addCptr(&res, "(%rsp), %rax\n"
                    "\tmovq\t\t%rax, ");
            addCptr(&res, utos((as(bopc, op)->body.retc - i + 3) * 8));
            addCptr(&res, "(%rsp)\n");
        }
        addCptr(&res, "\taddq\t\t$24, %rsp\n");
    }
    if (as(bopc, op)->els.ops.len > 0) {
        string tmp = stringDefault();
        for (u i = 0; i < as(bopc, op)->els.ops.len; i++)
            stringAddRange(&tmp, compOP(c, as(bopc, op)->els.ops.items[i], f));
        addCptr(&res, "\tjmp\t\t.endt");
        addCptr(&res, utos(c->addr));
        addCptr(&res, "\n"
                ".addr");
        addCptr(&res, utos(addr));
        addCptr(&res, ":\n"
                "\tpushq\t\t%rax\n");
        stringAddRange(&res, tmp);
        addCptr(&res, ".endt");
        addCptr(&res, utos(c->addr));
        addCptr(&res, ":\n");
    }
    return res;
}

const opcDef OPS[] = {
    { "+", ".add",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\taddq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "+.", ".addf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\taddsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "++", ".inc",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tincq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "-", ".sub",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tsubq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "-.", ".subf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tsubsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "--", ".dec",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tdecq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "~", ".neg",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tnegq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "~.", ".negf",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\txorl\t\t%eax, %eax\n"
        "\tcvtsi2sd\t%rax, %xmm0\n"
        "\tsubsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "*", ".mul",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tmulq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "*-", ".muls",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\timulq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "*.", ".mulf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tmulsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "/", ".div",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "/-", ".divs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "/.", ".divf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tdivsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "%", ".mod",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rdx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "%-", ".mods",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rdx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "%.", ".modf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\tfprem\n"
        "\taddq\t\t$8, %rsp\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "/%", ".dm",
        2, 2,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n"
        "\tpushq\t\t%rdx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "/%-", ".dms",
        2, 2,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n"
        "\tpushq\t\t%rdx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "!!", ".not",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t$0, %rax\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "!", ".bnot",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tnotq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "&&", ".and",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%edx, %edx\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t$0, %rbx\n"
        "\tsetne\t\t%dl\n"
        "\tcmpq\t\t$0, %rax\n"
        "\tsetne\t\t%cl\n"
        "\tandb\t\t%dl, %cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "&", ".band",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tandq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "||", ".or",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%edx, %edx\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t$0, %rbx\n"
        "\tsetne\t\t%dl\n"
        "\tcmpq\t\t$0, %rax\n"
        "\tsetne\t\t%cl\n"
        "\torb\t\t%dl, %cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "|", ".bor",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\torq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "^^", ".xor",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%edx, %edx\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t$0, %rbx\n"
        "\tsetne\t\t%dl\n"
        "\tcmpq\t\t$0, %rax\n"
        "\tsetne\t\t%cl\n"
        "\txorb\t\t%dl, %cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "^", ".bxor",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<<", ".shl",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tshlq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">>", ".shr",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tshrq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">>-", ".sar",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tsarq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "==", ".eq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "==.", ".eqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm0, %xmm1\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "!=", ".neq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetne\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "!=.", ".neqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetne\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<", ".lt",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetb\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<-", ".lts",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetl\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<.", ".ltf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetb\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<=", ".leq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetbe\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<=-", ".leqs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetle\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<=.", ".leqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetbe\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">", ".gt",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tseta\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">-", ".gts",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetg\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">.", ".gtf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, (%rsp)\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tseta\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">=", ".geq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetae\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">=-", ".geqs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetge\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ">=.", ".geqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetae\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".->", ".fti",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm0\n"
        "\tcvtsd2si\t%xmm0, %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "->.", ".itf",
        1, 1,
        "\tmovq\t\t(%rsp), %rax\n"
        "\tcvtsi2sd\t%rax, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "^.", ".sqrt",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tsqrtsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "@", ".ldat",
        1, 1,
        &compileLDAT,
        &linkLDAT,
        FGENERIC,
        FINT | FUINT
    },

    { ">", ".stat",
        2, 0,
        &compileSTAT,
        &linkSTAT,
        FGENERIC,
        FUINT
    },

    { "(@)", ".clat",
        0, 1,
        "\tpopq\t\t%rax\n"
        "\tcall\t\t*%rax\n",
        NULL,
        FIMMEDIATE | FARGCRETC,
        FUINT
    },

    { "<<|", ".ret",
        0, 0,
        &compileRET,
        &linkRET,
        FARGCRETC,
        FNONE
    },

    { "@", ".ldaddr",
        0, 1,
        &compileLDADDR,
        NULL,
        FGENERIC | FARGCRETC,
        FFUN | FGLB | FFLD | FLOC | FARG
    },

    { ">", ".store",
        1, 0,
        &compileST,
        NULL,
        FGENERIC | FARGCRETC,
        FGLB | FFLD | FLOC | FARG
    },

    { "??", ".if",
        1, 0,
        &compileIF,
        &linkIF,
        FNOFLAGS | FHASBODY,
        FNONE
    },

    { "@@", ".while",
        1, 0,
        &compileWHILE,
        &linkWHILE,
        FNOFLAGS | FHASBODY,
        FNONE
    },

    { "?!", ".try",
        0, 0,
        &compileTRY,
        &linkTRY,
        FNOFLAGS | FHASBODY,
        FNONE
    },

    { "!!!", ".throw",
        1, 0,
        "\tpopq\t\t%rax\n"
        "\tmovq\t\t.excrsp(%rip), %rsp\n"
        "\tpopq\t\t%rbx\n"
        "\tmovq\t\t%rbx, .excrsp(%rip)\n"
        "\tpopq\t\t%rbp\n"
        "\tpopq\t\t%rbx\n"
        "\tjmp\t\t\t*%rbx\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "", ".eval",
        0, 1,
        &compileEVAL,
        &linkEVAL,
        FGENERIC | FARGCRETC,
        FUINT | FINT | FDOUB | FSTR | FFUN | FGLB | FFLD | FLOC | FARG
    },

    { "\\\\", ".drop",
        1, 0,
        "\taddq\t\t$8, %rsp\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "?\\", ".cdrop",
        3, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tpopq\t\t%rcx\n"
        "\tcmpq\t\t$0, %rcx\n"
        "\tcmoveq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "**", ".dup",
        1, 2,
        "\tmovq\t\t(%rsp), %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<>", ".swap",
        2, 2,
        "\tmovq\t\t(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<<>", ".rotl",
        3, 3,
        "\tmovq\t\t(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\txchgq\t\t16(%rsp), %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "<>>", ".rotr",
        3, 3,
        "\tmovq\t\t16(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\txchgq\t\t(%rsp), %rax\n"
        "\tmovq\t\t%rax, 16(%rsp)\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "^>", ".over",
        2, 3,
        "\tmovq\t\t8(%rsp), %rax\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { "##", ".flags",
        0, 1,
        "\tpushfq\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".put", ".puts",
        2, 0,
        "\tpopq\t\t%rdi\n"
        "\tpopq\t\t%rsi\n"
        "\tmovq\t\t(%rsi), %rdx\n"
        "\taddq\t\t$8, %rsi\n"
        "\tmovq\t\t$1, %rax\n"
        "\tsyscall\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".mal", ".malloc",
        1, 1,
        "\tpopq\t\t%rdi\n"
        "\tcall\t\tmalloc\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".mfr", ".mfree",
        1, 0,
        "\tpopq\t\t%rdi\n"
        "\tcall\t\tfree\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".ral", ".realloc",
        2, 1,
        "\tpopq\t\t%rsi\n"
        "\tpopq\t\t%rdi\n"
        "\tcall\t\trealloc\n"
        "\tpushq\t\t%rax\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { ".ext", ".exit",
        1, 0,
        "\tpopq\t\t%rdi\n"
        "\tmovq\t\t$60, %rax\n"
        "\tsyscall\n",
        NULL,
        FGENERIC | FIMMEDIATE,
        FNONE
    }
};
