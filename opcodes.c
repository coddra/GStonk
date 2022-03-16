#include "h/opcodes.h"
#include "h/diagnostics.h"
#include "h/linker.h"

#define link(OP)                                \
    void link##OP(context* c, opc* o, u f, i64* s)

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
        "",
        &linkLDAT,
        FGENERIC,
        FINT | FUINT
    },

    { ">", ".stat",
        2, 0,
        "",
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
        "",
        &linkRET,
        FARGCRETC,
        FNONE
    },

    { "@", ".ldaddr",
        0, 1,
        "",
        NULL,
        FGENERIC | FARGCRETC,
        FFUN | FGLB | FFLD | FLOC | FARG
    },

    { ">", ".store",
        1, 0,
        "",
        NULL,
        FGENERIC | FARGCRETC,
        FGLB | FFLD | FLOC | FARG
    },

    { "??", ".if",
        1, 0,
        "",
        &linkIF,
        FNOFLAGS | FHASBODY,
        FNONE
    },

    { "@@", ".while",
        1, 0,
        "",
        &linkWHILE,
        FNOFLAGS | FHASBODY,
        FNONE
    },

    { "?!", ".try",
        0, 0,
        "",
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
        "",
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
