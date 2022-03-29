#include <limits.h>
#include "MCX/mcx.h"
#include "h/objects.h"
#include "h/diagnostics.h"
#include "h/parser.h"
#include "h/linker.h"
#include "h/compiler.h"

char OUTPUTFLAG = 'o';
char MALLFLAG = 'm';
char WALLFLAG = 'w';
char DGNFLAG = 'd';
char EXPORTFLAG = 'e';
char ASMFLAG = 's';
char SOFLAG = 'S';
char* GDBFLAG = "--gdb";
char* FLYCHECKFLAG = "--flycheck";

void parseArgs(context* c, list(string) args) {
    for (u i = 0; i < args.len; i++)
        if (args.items[i].len > 0 && args.items[i].items[0] == '-') {
            if (stringEquals(args.items[i], statstr(GDBFLAG)))
                c->flags |= FGDB;
            else if (stringEquals(args.items[i], statstr(FLYCHECKFLAG)))
                c->flags |= FFLYCHECK;
            else {
                u o = 0;
                for (u j = 1; j < args.items[i].len; j++) {
                    if (args.items[i].items[j] == OUTPUTFLAG) {
                        o++;
                        if (i + o >= args.len)
                            addDgn(c, EMISSINGSYNTAX, "filename");
                        else if (c->output.len != 0)
                            addDgnEmpty(c, WMULTIOUTPUT);
                        else {
                            c->output = args.items[i + o];
                            if (!isPathLegal(c->output))
                                addDgn(c, EPATHILLEGAL, cptrify(c->output));
                        }
                    } else if (args.items[i].items[j] == DGNFLAG) {
                        o++;
                        if (i + o >= args.len)
                            addDgn(c, EMISSINGSYNTAX, "diagnostic id");
                        else
                            uListAdd(&c->ignoreDgns, getDgn(args.items[i + o]));
                    } else if (args.items[i].items[j] == MALLFLAG)
                        c->flags |= FIGNOREMSGS;
                    else if (args.items[i].items[j] == WALLFLAG)
                        c->flags |= FIGNOREWRNGS;
                    else if (args.items[i].items[j] == EXPORTFLAG)
                        c->flags |= FEXPORTALL;
                    else if (args.items[i].items[j] == ASMFLAG)
                        c->flags |= FASM;
                    else if (args.items[i].items[j] == SOFLAG)
                        c->flags |= FSO;
                    else
                        addDgn(c, MUNRECFLAG, ctcptr(args.items[i].items[j]));
                }
                i += o;
            }
        } else {
            if (fileExists(args.items[i]))
                stringListAdd(&c->inputs, args.items[i]);
            else
                addDgn(c, EFILENOTEXIST, cptrify(args.items[i]));
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
    init(MCX);
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
