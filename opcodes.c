#include "h/opcodes.h"

const opcDef OPS[] = {
    { OPADD,
        "+", ".add",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\taddq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPADDF,
        "+.", ".addf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\taddsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPINC,
        "++", ".inc",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tincq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSUB,
        "-", ".sub",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tsubq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSUBF,
        "-.", ".subf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tsubsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDEC,
        "--", ".dec",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tdecq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPNEG,
        "~", ".neg",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tnegq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPNEGF,
        "~.", ".negf",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\txorl\t\t%eax, %eax\n"
        "\tcvtsi2sd\t%rax, %xmm0\n"
        "\tsubsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMUL,
        "*", ".mul",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tmulq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMULS,
        "*-", ".muls",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\timulq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMULF,
        "*.", ".mulf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tmulsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDIV,
        "/", ".div",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDIVS,
        "/-", ".divs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDIVF,
        "/.", ".divf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$8, %rsp\n"
        "\tdivsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMOD,
        "%", ".mod",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rdx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMODS,
        "%-", ".mods",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rdx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMODF,
        "%.", ".modf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\tfprem\n"
        "\taddq\t\t$8, %rsp\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDM,
        "/%", ".dm",
        2, 2,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tdivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n"
        "\tpushq\t\t%rdx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDMS,
        "/%-", ".dms",
        2, 2,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tcqo\n"
        "\tidivq\t\t%rbx\n"
        "\tpushq\t\t%rax\n"
        "\tpushq\t\t%rdx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPNOT,
        "!!", ".not",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t$0, %rax\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPBNOT,
        "!", ".bnot",
        1, 1,
        "\tpopq\t\t%rax\n"
        "\tnotq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPAND,
        "&&", ".and",
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
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPBAND,
        "&", ".band",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tandq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPOR,
        "||", ".or",
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
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPBOR,
        "|", ".bor",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\torq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPXOR,
        "^^", ".xor",
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
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPBXOR,
        "^", ".bxor",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSHL,
        "<<", ".shl",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tshlq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSHR,
        ">>", ".shr",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tshrq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSAR,
        ">>-", ".sar",
        2, 1,
        "\tpopq\t\t%rcx\n"
        "\tpopq\t\t%rax\n"
        "\tsarq\t\t%cl, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPEQ,
        "==", ".eq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPEQF,
        "==.", ".eqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm0, %xmm1\n"
        "\tsete\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPNE,
        "!=", ".neq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetne\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPNEF,
        "!=.", ".neqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetne\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLT,
        "<", ".lt",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetb\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLTS,
        "<-", ".lts",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetl\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLTF,
        "<.", ".ltf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetb\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLE,
        "<=", ".leq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetbe\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLES,
        "<=-", ".leqs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetle\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLEF,
        "<=.", ".leqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetbe\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGT,
        ">", ".gt",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tseta\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGTS,
        ">-", ".gts",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetg\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGTF,
        ">.", ".gtf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, (%rsp)\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tseta\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGE,
        ">=", ".geq",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetae\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGES,
        ">=-", ".geqs",
        2, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tcmpq\t\t%rbx, %rax\n"
        "\tsetge\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPGEF,
        ">=.", ".geqf",
        2, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tmovsd\t\t8(%rsp), %xmm0\n"
        "\taddq\t\t$16, %rsp\n"
        "\txorl\t\t%ecx, %ecx\n"
        "\tucomisd\t\t%xmm1, %xmm0\n"
        "\tsetae\t\t%cl\n"
        "\tpushq\t\t%rcx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPFTI,
        ".->", ".fti",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm0\n"
        "\tcvtsd2si\t%xmm0, %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPITF,
        "->.", ".itf",
        1, 1,
        "\tmovq\t\t(%rsp), %rax\n"
        "\tcvtsi2sd\t%rax, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSQRT,
        "^.", ".sqrt",
        1, 1,
        "\tmovsd\t\t(%rsp), %xmm1\n"
        "\tsqrtsd\t\t%xmm1, %xmm0\n"
        "\tmovsd\t\t%xmm0, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPLDAT,
        "@->", ".ldat",
        1, 1,
        "",
        FGENERIC,
        FINT
    },

    { OPSTAT,
        "->@", ".stat",
        2, 0,
        "",
        FGENERIC,
        FUINT
    },

    { OPCLAT,
        "!@", ".clat",
        0, 1,
        "\tpopq\t\t%rax\n"
        "\tcall\t\t*%rax\n",
        FIMMEDIATE,
        FNONE
    },

    { OPRET,
        "<<|", ".ret",
        1, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPLDADDR,
        "@", ".ldaddr",
        0, 1,
        "",
        FGENERIC,
        FFUN | FGLB | FFLD | FLOC | FARG
    },

    { OPST,
        "->", ".store",
        1, 0,
        "",
        FGENERIC,
        FGLB | FFLD | FLOC | FARG
    },

    { OPIF,
        "??", ".if",
        1, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPELIF,
        "|?", ".elif",
        1, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPWHILE,
        "@@", ".while",
        1, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPTRY,
        "?!", ".try",
        0, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPELSE,
        "|>", ".else",
        0, 0,
        "",
        FNOFLAGS,
        FNONE
    },

    { OPTHROW,
        "!!!", ".throw",
        1, 0,
        "\tpopq\t\t%rax\n"
        "\tmovq\t\t.excrsp(%rip), %rsp\n"
        "\tpopq\t\t%rbx\n"
        "\tmovq\t\t%rbx, .excrsp(%rip)\n"
        "\tpopq\t\t%rbp\n"
        "\tpopq\t\t%rbx\n"
        "\tjmp\t\t\t*%rbx\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPEVAL,
        "", ".eval",
        0, 1,
        "",
        FGENERIC,
        FUINT | FINT | FDOUB | FSTR | FFUN | FGLB | FFLD | FLOC | FARG
    },

    { OPDROP,
        "\\\\", ".drop",
        1, 0,
        "\taddq\t\t$8, %rsp\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPCDROP,
        "?\\", ".cdrop",
        3, 1,
        "\tpopq\t\t%rbx\n"
        "\tpopq\t\t%rax\n"
        "\tpopq\t\t%rcx\n"
        "\tcmpq\t\t$0, %rcx\n"
        "\tcmoveq\t\t%rbx, %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPDUP,
        "**", ".dup",
        1, 2,
        "\tmovq\t\t(%rsp), %rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPSWAP,
        "<>", ".swap",
        2, 2,
        "\tmovq\t\t(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPROTL,
        "<<>", ".rotl",
        3, 3,
        "\tmovq\t\t(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\txchgq\t\t16(%rsp), %rax\n"
        "\tmovq\t\t%rax, (%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPROTR,
        "<>>", ".rotr",
        3, 3,
        "\tmovq\t\t16(%rsp), %rax\n"
        "\txchgq\t\t8(%rsp), %rax\n"
        "\txchgq\t\t(%rsp), %rax\n"
        "\tmovq\t\t%rax, 16(%rsp)\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPFLAGS,
        ".flgs", ".flags",
        0, 1,
        "\tpushfq\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPVERR,
        "?->", ".verr",
        1, 1,
        "\tpopq\t\t%rbx\n"
        "\txorl\t\t%eax, %eax\n"
        "\tverr\t\t%rbx\n"
        "\tsetzq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPVERW,
        "->?", ".verw",
        1, 1,
        "\tpopq\t\t%rbx\n"
        "\txorl\t\t%eax, %eax\n"
        "\tverw\t\t%rbx\n"
        "\tsetzq\t\t%rax\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPBUILTIN,
        "", "",
        1, 1,
        "",
        FNOFLAGS,
        FUINT
    },

    { OPPRINT,
        ".put", ".puts",
        2, 0,
        "\tpopq\t\t%rdi\n"
        "\tpopq\t\t%rsi\n"
        "\tmovq\t\t(%rsi), %rdx\n"
        "\taddq\t\t$8, %rsi\n"
        "\tmovq\t\t$1, %rax\n"
        "\tsyscall\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPMALLOC,
        ".mal", ".malloc",
        1, 1,
        "\tpopq\t\t%rdi\n"
        "\tcall\t\tmalloc\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPFREE,
        ".mfr", ".mfree",
        1, 0,
        "\tpopq\t\t%rdi\n"
        "\tcall\t\tfree\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPREALLOC,
        ".ral", ".realloc",
        2, 1,
        "\tpopq\t\t%rsi\n"
        "\tpopq\t\t%rdi\n"
        "\tcall\t\trealloc\n"
        "\tpushq\t\t%rax\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    },

    { OPEXIT,
        ".ext", ".exit",
        1, 0,
        "\tpopq\t\t%rdi\n"
        "\tmovq\t\t$60, %rax\n"
        "\tsyscall\n",
        FGENERIC | FIMMEDIATE,
        FNONE
    }
};
