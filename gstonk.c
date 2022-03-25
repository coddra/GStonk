#include <limits.h>
#include "MCX/mcx.h"
#include "h/objects.h"
#include "h/diagnostics.h"
#include "h/parser.h"
#include "h/linker.h"
#include "h/compiler.h"

char* OFLAG = "-o";
char* MFLAG = "-m";
char* WFLAG = "-w";
char* EXPORTFLAG = "-expall";
char* GDBFLAG = "-gdb";
char* ASMFLAG = "-s";
char* FLYCHECKFLAG = "-flycheck";

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
        else if (stringEquals(args.items[i], statstr(EXPORTFLAG)))
            c->flags |= FEXPORTALL;
        else if (stringEquals(args.items[i], statstr(FLYCHECKFLAG)))
            c->flags |= FFLYCHECK;
        else
            addFile(c, args.items[i], c->loc);
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

string getBin(char* argz) {
    string res = absolutePath(stringify(argz));
    if(res.len == 0) {
        char buffer[PATH_MAX + 8];
        FILE *pipe;

        pipe = popen("whereis gstonk", "r");

        fgets(buffer, sizeof(buffer), pipe);

        buffer[strlen(buffer) - 1] = '\0';

        pclose(pipe);
        res = stringify(buffer);
        stringRemoveRange(&res, 0, stringPos(res, ' '));
        u pos = stringPos(res, ' ');
        if (pos < res.len)
            stringRemoveRange(&res, pos, res.len - pos);
    }
    u pos = stringLastPos(res, '/') + 1;
    stringRemoveRange(&res, pos, res.len - pos);
    return res;
}

int main(int argc, char** argv) {
    init(PARSER);

    context c = {0};
    c.loc.file = UINT64_MAX;
    c.bin = getBin(argv[0]);

    list(string) ars = stringListDefault();
    for (u i = 1; i < argc; i++)
        stringListAdd(&ars, stringify(argv[i]));
    parseArgs(&c, ars);
    exitOnError(&c);

    parse(&c);
    exitOnError(&c);

    link(&c);
    exitOnError(&c);

    if (c.flags & FFLYCHECK) {
        printDgns(&c);
        exit(0);
    }

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
        addCptr(&cmd, " -ggdb -g3");
    else
        addCptr(&cmd, " -O3");
    int res = system(cptrify(cmd));
    if (res)
        addDgn(&c, EGCCFAILED, utos(res));
    else
        addDgnEmpty(&c, MSUCCESS);
    if ((c.flags & FASM) == 0)
        remove(cptrify(out));
    printDgns(&c);
}
