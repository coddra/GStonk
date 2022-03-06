#include "h/compiler.h"
#include "h/opcodes.h"
#include "h/linker.h"

string compOP(context* c, opc* op, u f) {
    string res = stringDefault();
    addCptr(&res, ".addr");
    addCptr(&res, utos(c->addr++));
    addCptr(&res, ":\t\t\t\t//");
    addCptr(&res, (char*)((ptr)OPS[op->op].alias + 1));
    stringAdd(&res, '\n');
    if (OPS[op->op].flags & FIMMEDIATE)
        addCptr(&res, OPS[op->op].comp);
    else if (op->op == OPRET) {
        if (c->funs.items[f].ret.len == 0) {
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos(c->funs.items[f].locs.len * 8 + 8));
            addCptr(&res, "(%rbp), %rax\n"
                    "\tmovq\t\t%rbp, %rsp\n"
                    "\tmovq\t\t(%rbp), %rbp\n"
                    "\taddq\t\t$");
            addCptr(&res, utos((c->funs.items[f].locs.len + 1 + c->funs.items[f].args.len) * 8));
            addCptr(&res, ", %rsp\n"
                    "\tjmp\t\t\t*%rax\n");
        } else {
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos(c->funs.items[f].locs.len * 8 + 8));
            addCptr(&res, "(%rbp), %r12\n"
                    "\tmovq\t\t%rsp, %rsi\n"
                    "\tmovq\t\t%rbp, %rdi\n");
            if (c->funs.items[f].locs.len + c->funs.items[f].args.len + 2 != c->funs.items[f].ret.len) {
                addCptr(&res, "\taddq\t\t$");
                addCptr(&res, itos((c->funs.items[f].locs.len + c->funs.items[f].args.len + 2 - c->funs.items[f].ret.len) * 8));
                addCptr(&res, ", %rdi\n");
            }
            addCptr(&res, "\tmovq\t\t$");
            addCptr(&res, utos(c->funs.items[f].ret.len * 8));
            addCptr(&res, ", %rdx\n"
                    "\tcall\t\tmemcpy\n"
                    "\tmovq\t\t(%rbp), %rbp\n"
                    "\taddq\t\t$");
            addCptr(&res, utos((c->funs.items[f].locs.len + c->funs.items[f].args.len + 2) * 8));
            addCptr(&res, ", %rsp\n"
                    "\tjmp\t\t\t*%r12\n");
        }
    } else if (op->op == OPLDADDR) {
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
    } else if (op->op == OPST) {
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
    } else if (op->op == OPEVAL) {
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
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos(as(popc, op)->par.val.r.i * 8 + 8));
            addCptr(&res, "(%rbp), %rax\n"
                    "\tpushq\t\t%rax\n");
        } else if (as(popc, op)->par.kind == KARG) {
            addCptr(&res, "\tmovq\t\t");
            addCptr(&res, utos((c->funs.items[f].args.len + 1 + c->funs.items[f].locs.len - as(popc, op)->par.val.r.i) * 8));
            addCptr(&res, "(%rbp), %rax\n"
                    "\tpushq\t\t%rax\n");
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
            addCptr(&res, "\tmovq\t\t$");
            addCptr(&res, utos(as(popc, op)->par.val.u));
            addCptr(&res, ", %rax\n"
                    "\tpushq\t\t%rax\n");
        }
    } else if (op->op == OPLDAT) {
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
    } else if (op->op == OPSTAT) {
        addCptr(&res, "\tpopq\t\t%rbx\n"
                "\tpopq\t\t%rax\n"
                "\tmov");
        stringAdd(&res, getPostfix(as(popc, op)->par.val.u));
        addCptr(&res, "\t\t%");
        stringAddRange(&res, getRegister('b', as(popc, op)->par.val.u));
        addCptr(&res, ", (%rax)\n");
    } else if (op->op == OPIF) {
        for (u i = 0; i < as(bopc, op)->head.len; i++)
            stringAddRange(&res, compOP(c, as(bopc, op)->head.items[i], f));
        string tmp = stringDefault();
        for (u i = 0; i < as(bopc, op)->body.len; i++)
            stringAddRange(&tmp, compOP(c, as(bopc, op)->body.items[i], f));
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tcmpq\t\t$0, %rax\n"
                "\tje\t\t\t.addr");
        addCptr(&res, utos(c->addr));
        stringAdd(&res, '\n');
        stringAddRange(&res, tmp);
        if (as(bopc, op)->body2.len > 0) {
            tmp.len = 0;
            for (u i = 0; i < as(bopc, op)->body2.len; i++)
                stringAddRange(&tmp, compOP(c, as(bopc, op)->body2.items[i], f));
            addCptr(&res, "\tjmp\t\t\t.addr");
            addCptr(&res, utos(c->addr));
            addCptr(&res, "\n"
                      "\t\t\t\t\t\t//else\n");
            stringAddRange(&res, tmp);
        }
    } else if (op->op == OPWHILE) {
        u64 head = c->addr;
        for (u i = 0; i < as(bopc, op)->head.len; i++)
            stringAddRange(&res, compOP(c, as(bopc, op)->head.items[i], f));
        string tmp = stringDefault();
        for (u i = 0; i < as(bopc, op)->body.len; i++)
            stringAddRange(&tmp, compOP(c, as(bopc, op)->body.items[i], f));
        addCptr(&res, "\tpopq\t\t%rax\n"
                "\tcmpq\t\t$0, %rax\n"
                "\tje\t\t\t.addr");
        addCptr(&res, utos(c->addr));
        addCptr(&res, "\n"
                "\t\t\t\t\t\t//body\n");
        stringAddRange(&res, tmp);
        addCptr(&res, "\tjmp\t\t\t.addr");
        addCptr(&res, utos(head));
        stringAdd(&res, '\n');
    } else if (op->op == OPTRY) {
        addCptr(&res, "\tleaq\t\t.addr");
        u addr = c->addr++;
        addCptr(&res, utos(addr));
        addCptr(&res, "(%rip), %rax\n"
                "\tpushq\t\t%rax\n"
                "\tpushq\t\t%rbp\n"
                "\tmovq\t\t.excrsp(%rip), %rax\n"
                "\tpushq\t\t%rax\n"
                "\tmovq\t\t%rsp, .excrsp(%rip)\n");
        for (u i = 0; i < as(bopc, op)->body.len; i++)
            stringAddRange(&res, compOP(c, as(bopc, op)->body.items[i], f));
        addCptr(&res, "\tmovq\t\t.excrsp(%rip), %rax\n"
                "\tmovq\t\t(%rax), %rax\n"
                "\tmovq\t\t%rax, .excrsp(%rip)\n");
        if (as(bopc, op)->retc > 0) {
            addCptr(&res, "\tmovq\t\t%rsp, %rsi\n"
                    "\tmovq\t\t$");
            addCptr(&res, utos(as(bopc, op)->retc * 8));
            addCptr(&res, ", %rdx\n"
                    "\taddq\t\t%rdx, %rsi\n"
                    "\tmovq\t\t%rsi, %rdi\n"
                    "\taddq\t\t$24, %rdi\n"
                    "\tcall\t\tmemcpy\n"
                    "\taddq\t\t$24, %rsp\n");
        }
        if (as(bopc, op)->body2.len > 0) {
            string tmp = stringDefault();
            for (u i = 0; i < as(bopc, op)->body2.len; i++)
                stringAddRange(&tmp, compOP(c, as(bopc, op)->body2.items[i], f));
            addCptr(&res, "\tjmp\t\t.addr");
            addCptr(&res, utos(c->addr));
            addCptr(&res, "\n"
                    ".addr");
            addCptr(&res, utos(addr));
            addCptr(&res, ":\t\t\t\t;catch\n"
                    "\tpushq\t\t%rax\n");
            stringAddRange(&res, tmp);
        }
    }
    return res;
}
string compFun(context* c, u f) {
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
    for (u i = 0; i < c->funs.items[f].body.len; i++)
        stringAddRange(&res, compOP(c, c->funs.items[f].body.items[i], f));
    return res;
}
string compGlb(context* c, u g) {
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
string compStr(context* c, u s) {
    string res = stringDefault();
    addCptr(&res, ".str");
    addCptr(&res, utos(s));
    addCptr(&res, ":\t\t/*");
    stringAddRange(&res, c->strs.items[s]);
    addCptr(&res, "*/\n"
            "\t.byte\t\t");
    for (u i = 0; i < 8; i++) {
        addCptr(&res, utos((c->strs.items[s].len >> (i * 8)) % 256));
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
        stringAddRange(&res, compGlb(c, i));
    for (u i = 0; i < c->strs.len; i++)
        stringAddRange(&res, compStr(c, i));
    return res;
}
