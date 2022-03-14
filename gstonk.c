#include "h/shorts.h"
#include "h/objects.h"
#include "h/diagnostics.h"
#include "h/parser.h"
#include "h/linker.h"
#include "h/compiler.h"

char* OFLAG = "-o";
char* MFLAG = "-m";
char* WFLAG = "-w";
char* GDBFLAG = "-gdb";
char* ASMFLAG = "-s";

void parseArgs(context* c, list(string) args) {
    for (u i = 0; i < args.len; i++) {
        if (stringStartsWith(args.items[i], statstr(OFLAG))) {
            if (c->output.len != 0)
                addDgnEmpty(c, WMULTIOUTPUT);
            else
                c->output = substring(args.items[i], 2);
        } else if (stringStartsWith(args.items[i], statstr(WFLAG))) {
            if (stringEquals(args.items[i], statstr(WFLAG)))
                c->flags |= FIGNOREWRNGS;
            else
                uListAdd(&c->ignoreDgns, getDgn(substring(args.items[i], 1)));
        } else if (stringStartsWith(args.items[i], statstr(MFLAG))) {
            if (stringEquals(args.items[i], statstr(MFLAG)))
                c->flags |= FIGNOREMSGS;
            else
                uListAdd(&c->ignoreDgns, getDgn(substring(args.items[i], 1)));
        } else if (stringEquals(args.items[i], statstr(GDBFLAG)))
            c->flags |= FGDB;
        else if (stringEquals(args.items[i], statstr(ASMFLAG)))
            c->flags |= FASM;
        else
            addFile(c, args.items[i]);
    }
    if (c->inputs.len == 0)
        addDgnEmpty(c, ENOINPUT);
    else if (c->output.len == 0) {
        c->output = stringClone(c->inputs.items[0]);
        u pos = stringLastPos(c->output, '.');
        stringRemoveRange(&c->output, pos, c->output.len - pos);
    }
}

void exitOnError(context* c) {
    if (highestLVL(c) == LVLERROR) {
        printDgns(c);
        exit(1);
    }
}

int main(int argc, char** argv) {
    init(PARSER);

    context c = contextDefault();
    c.bin = absolutePath(stringify(argv[0]));
    u pos = stringLastPos(c.bin, '/') + 1;
    stringRemoveRange(&c.bin, pos, c.bin.len - pos);

    list(string) ars = stringListDefault();
    for (u i = 1; i < argc; i++)
        stringListAdd(&ars, stringify(argv[i]));
    parseArgs(&c, ars);
    exitOnError(&c);

    parse(&c);
    exitOnError(&c);

    link(&c);
    exitOnError(&c);

    string out;
    if (c.flags & FASM) {
        out = stringClone(c.output);
        addCptr(&out, ".s");
    } else {
        out = stringClone(c.bin);
        addCptr(&out, "out.s");
    }
    writeAllText(out, compile(&c));
    string cmd = stringify("gcc ");
    stringAddRange(&cmd, out);
    addCptr(&cmd, " -o ");
    stringAddRange(&cmd, c.output);
    if (c.flags & FGDB)
        addCptr(&out, " -ggdb -g3");
    int res = system(cptrify(cmd));
    if (res)
        addDgn(&c, EGCCFAILED, utos(res));
    else
        addDgnEmpty(&c, MSUCCESS);
    if ((c.flags & FASM) == 0)
        remove(cptrify(out));
    printDgns(&c);
}
