#include "h/shorts.h"
#include "h/objects.h"
#include "h/parser.h"
#include "h/linker.h"
#include "h/compiler.h"
char* OFLAG = "-o";
char* WFLAG = "-w";
char* WOFF = "woff";
char* GDBFLAG = "-gdb";

void parseArgs(context* c, list(string) args) {
    for (u i = 0; i < args.len; i++) {
        if (stringStartsWith(args.items[i], statstr(OFLAG))) {
            if (c->output.len != 0)
                addDgnEmpty(c, WMULTIOUTPUT);
            else
                c->output = substring(args.items[i], 2);
        } else if (stringStartsWith(args.items[i], statstr(WFLAG))) {
            if (stringEquals(substring(args.items[i], 1), statstr(WOFF)))
                c->flags |= FWARNINGSOFF;
            else
                stringListAdd(&c->ignorew, substring(args.items[i], 1));
        } else if (stringEquals(args.items[i], statstr(GDBFLAG)))
            c->flags |= FGDB;
        else {
            if (fileExists(args.items[i]))
                stringListAdd(&c->inputs, args.items[i]);
            else
                addDgn(c, EFILENOTEXIST, cptrify(args.items[i]));
        }
    }
    if (c->inputs.len == 0)
        addDgnEmpty(c, ENOINPUT);
    if (c->output.items == NULL)
        c->output = stringFromArray("out", 3);
}

void exitOnError(context* c) {
    if (checkErr(c)) {
        printDgns(c);
        exit(1);
    }
}

int main(int argc, char** args) {
    init(PARSER);
    context c = contextDefault();
    list(string) ars = stringListDefault();
    for (u i = 1; i < argc; i++)
        stringListAdd(&ars, stringify(args[i]));
    parseArgs(&c, ars);
    exitOnError(&c);
    parse(&c);
    exitOnError(&c);
    link(&c);
    exitOnError(&c);
    printDgns(&c);

    string tmp = stringClone(c.output);
    addCptr(&tmp, ".s");
    writeAllText(tmp, compile(&c));
    tmp.len = 0;
    addCptr(&tmp, "gcc ");
    stringAddRange(&tmp, c.output);
    addCptr(&tmp, ".s -o ");
    stringAddRange(&tmp, c.output);
    if (c.flags & FGDB)
        addCptr(&tmp, " -ggdb -g3");
    system(cptrify(tmp));
}
