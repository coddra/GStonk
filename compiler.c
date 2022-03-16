#include "h/compiler.h"
#include "h/opcodes.h"
#include "h/linker.h"

typedef string (*compileFUN)(context*, opc*, u);

string compOP(context* c, opc* op, u f) {
    string res = stringDefault();
    addCptr(&res, ".addr");
    addCptr(&res, utos(c->addr++));
    addCptr(&res, ":\n");
    if (OPS[op->op].flags & FIMMEDIATE)
        addCptr(&res, (char*)OPS[op->op].comp);
    else
        stringAddRange(&res, ((compileFUN)(OPS[op->op].comp))(c, op, f));
    return res;
}
static string compFun(context* c, u f) {
    string res = stringDefault();
    addCptr(&res, ".globl ");
    stringAddRange(&res, c->funs.items[f].name.csign);
    stringAdd(&res, '\n');
    stringAddRange(&res, c->funs.items[f].name.csign);
    addCptr(&res, ":\t\t//");
    stringAddRange(&res, c->funs.items[f].name.sign);
    stringAdd(&res, '\n');
    if (c->funs.items[f].locs.len > 0) {
        addCptr(&res, "\tsubq\t\t$");
        addCptr(&res, utos(c->funs.items[f].locs.len * 8));
        addCptr(&res, ", %rsp\n");
    }
    addCptr(&res, "\tpushq\t\t%rbp\n"
            "\tmovq\t\t%rsp, %rbp\n");
    for (u i = 0; i < c->funs.items[f].body.ops.len; i++)
        stringAddRange(&res, compOP(c, c->funs.items[f].body.ops.items[i], f));
    return res;
}
static string compGlb(context* c, u g) {
    string res = stringDefault();
    addCptr(&res, ".globl\t");
    stringAddRange(&res, c->glbs.items[g].name.csign);
    stringAdd(&res, '\n');
    stringAddRange(&res, c->glbs.items[g].name.csign);
    addCptr(&res, ":\t\t//");
    stringAddRange(&res, c->glbs.items[g].name.sign);
    addCptr(&res, "\n"
            "\t.zero\t\t");
    addCptr(&res, utos(c->typs.items[c->glbs.items[g].type.i].size));
    stringAdd(&res, '\n');
    return res;
}
static string compStr(context* c, u s) {
    string res = stringDefault();
    addCptr(&res, ".str");
    addCptr(&res, utos(s));
    addCptr(&res, ":\t\t/*");
    stringAddRange(&res, c->strs.items[s]);
    addCptr(&res, "*/\n"
            "\t.byte\t\t");
    for (u i = 0; i < 8; i++) {
        addCptr(&res, utos((c->strs.items[s].len >> (i * 8)) % 256));
        if (i != 7 || c->strs.items[s].len > 0)
            stringAdd(&res, ',');
    }
    stringAdd(&res, '\t');
    for (u i = 0; i < c->strs.items[s].len; i++) {
        addCptr(&res, utos(c->strs.items[s].items[i]));
        if (i != c->strs.items[s].len - 1)
            stringAdd(&res, ',');
    }
    stringAdd(&res, '\n');
    return res;
}

string compile(context* c) {
    string res = stringDefault();
    addCptr(&res, ".section .text\n"
            ".globl main\n"
            "main:\n"
            "\tleaq\t\t.exchndlr(%rip), %rax\n"
            "\tpushq\t\t%rax\n"
            "\tpushq\t\t$0\n"
            "\tpushq\t\t$0\n"
            "\tmovq\t\t%rsp, .excrsp(%rip)\n"
            "\tcall\t\t");
    stringAddRange(&res, c->funs.items[c->main].name.csign);
    addCptr(&res, "\n"
            "\tmovq\t\t$0, %rdi\n"
            "\tmovq\t\t$60, %rax\n"
            "\tsyscall\n"
            ".exchndlr:\n"
            "\tmovq\t\t%rax, %r8\n"
            "\tmovq\t\t$1, %rax\n"
            "\tmovq\t\t$2, %rdi\n"
            "\tleaq\t\t.excmsg(%rip), %rsi\n"
            "\tmovq\t\t$35, %rdx\n"
            "\tsyscall\n"
            "\tmovq\t\t$1, %rax\n"
            "\tmovq\t\t$2, %rdi\n"
            "\tmovq\t\t(%r8), %rsi\n"
            "\tmovq\t\t(%rsi), %rdx\n"
            "\taddq\t\t$8, %rsi\n"
            "\tsyscall\n"
            "\tmovq\t\t$60, %rax\n"
            "\tmovq\t\t$1, %rdi\n"
            "\tsyscall\n");
    for (u i = 0; i < c->funs.len; i++)
        if (export(c, as(def, &c->funs.items[i])))
            stringAddRange(&res, compFun(c, i));
    addCptr(&res, ".addr");
    addCptr(&res, utos(c->addr));
    addCptr(&res, ":\n"
            ".section .data\n"
            ".excmsg:\n"
            "\t.ascii\t\t\"An unhandled exception was thrown: \"\n"
            ".excrsp:\n"
            "\t.zero\t\t8\n");
    for (u i = 0; i < c->glbs.len; i++)
        if (export(c, as(def, &c->glbs.items[i])))
            stringAddRange(&res, compGlb(c, i));
    for (u i = 0; i < c->strs.len; i++)
        stringAddRange(&res, compStr(c, i));
    return res;
}
