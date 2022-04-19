#include <limits.h>
#include "mcx/mcx.h"
#include "mcx/list.h"
#include "mcx/file.h"
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
            if (stringCompare(args.items[i], sstr(GDBFLAG)) == 0)
                c->flags |= FGDB;
            else if (stringCompare(args.items[i], sstr(FLYCHECKFLAG)) == 0)
                c->flags |= FFLYCHECK;
            else {
                u o = 0;
                for (u j = 1; j < args.items[i].len; j++) {
                    if (args.items[i].items[j] == OUTPUTFLAG) {
                        o++;
                        if (i + o >= args.len)
                            cAddDgn(c, &EMISSINGSYNTAX, "filename");
                        else if (c->output.len != 0)
                            cAddDgnEmpty(c, &WMULTIOUTPUT);
                        else {
                            c->output = args.items[i + o];
                            if (!isPathLegal(c->output))
                                cAddDgn(c, &EPATHILLEGAL, cptr(c->output));
                        }
                    } else if (args.items[i].items[j] == DGNFLAG) {
                        o++;
                        if (i + o >= args.len)
                            cAddDgn(c, &EMISSINGSYNTAX, "diagnostic id");
                        else
                            dgnDscrPtrListAdd(&c->ignoreDgns, getDgn(args.items[i + o]));
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
                        cAddDgn(c, &MUNRECFLAG, ctcptr(args.items[i].items[j]));
                }
                i += o;
            }
        } else {
            if (fileExists(args.items[i]))
                stringListAdd(&c->inputs, args.items[i]);
            else
                cAddDgn(c, &EFILENOTEXIST, cptr(args.items[i]));
        }
    if (c->inputs.len == 0)
        cAddDgnEmpty(c, &ENOINPUT);
    else if (c->output.len == 0) {
        c->output = stringClone(c->inputs.items[0]);
        u pos = stringLastPos(c->output, '.');
        stringRemoveRange(&c->output, pos, c->output.len - pos);
    }
}

void exitOnError(context* c) {
    if (cHighestLVL(c) == LVLERROR) {
        cPrintDgns(c, c->flags & FSO);
        exit(1);
    }
}

string getBin(char* argz) {
    string res = realPath(str(argz));
    if(res.len == 0) {
        char buffer[PATH_MAX + 8];
        FILE *pipe;

        pipe = popen("whereis gstonk", "r");

        fgets(buffer, sizeof(buffer), pipe);

        buffer[strlen(buffer) - 1] = '\0';

        pclose(pipe);
        res = str(buffer);
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

    list(string) ars = {0};
    for (u i = 1; i < argc; i++)
        stringListAdd(&ars, str(argv[i]));
    parseArgs(&c, ars);
    exitOnError(&c);

    parse(&c);
    exitOnError(&c);

    link(&c);
    exitOnError(&c);

    if ((c.flags & FFLYCHECK) == FFLYCHECK) {
        cPrintDgns(&c, true);
        exit(0);
    }

    string out;
    if (c.flags & FASM) {
        out = stringClone(c.output);
        catCptr(&out, ".s");
    } else {
        out = stringClone(c.bin);
        catCptr(&out, "out.s");
    }
    writeAllText(out, compile(&c));
    string cmd = str("gcc ");
    stringAddRange(&cmd, out);
    catCptr(&cmd, " -o ");
    stringAddRange(&cmd, c.output);
    if (c.flags & FGDB)
        catCptr(&cmd, " -ggdb -g3");
    else
        catCptr(&cmd, " -O3");
    int res = system(cptr(cmd));
    if (res)
        cAddDgn(&c, &EGCCFAILED, cptr(utos(res)));
    else
        cAddDgnEmpty(&c, &MSUCCESS);
    if ((c.flags & FASM) == 0)
        remove(cptr(out));
    cPrintDgns(&c, c.flags & FSO);
}
