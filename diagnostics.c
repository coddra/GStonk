#include "h/diagnostics.h"
#include "h/linker.h"

const dgnDscr DGNS[DGNCOUNT] = {
    { "",
        "missing token '$'",
        LVLERROR,
    },
    { "",
        "missing $",
        LVLERROR,
    },
    { "",
        "unrecognized escapesequence '$'",
        LVLERROR,
    },
    { "",
        "second declaration of '$' found",
        LVLERROR,
    },
    { "",
        "$ out of range",
        LVLERROR,
    },
    { "",
        "unrecognized token '$'",
        LVLERROR,
    },
    { "",
        "definition of '$' not found",
        LVLERROR,
    },
    { "",
        "wrong number of parameters provided for '$'",
        LVLERROR,
    },
    { "",
        "wrong number of arguments defined for '$'",
        LVLERROR,
    },
    { "",
        "multiple functions found with attribute 'main'",
        LVLERROR,
    },
    { "",
        "no input specified",
        LVLERROR,
    },
    { "",
        "size of type must be one of the following: 0, 1, 2, 4 or 8",
        LVLERROR,
    },
    { "",
        "not enough arguments",
        LVLERROR,
    },
    { "",
        "there is no attribue with name '$'",
        LVLERROR,
    },
    { "",
        "this attribute can only be attached once",
        LVLERROR,
    },
    { "",
        "stack unpredictable",
        LVLERROR,
    },
    { "",
        "file '$' does not exist",
        LVLERROR,
    },
    { "",
        "opcode '$' does not take any argument",
        LVLERROR,
    },
    { "",
        "wrong kind of parameter provided for '$'",
        LVLERROR,
    },
    { "",
        "gcc returned exit status '$'",
        LVLERROR,
    },
    { "",
        "attribute '$' cannot be attached to this kind of object",
        LVLERROR,
    },
    { "",
        "not all codepaths return the controlflow",
        LVLERROR,
    },

    { "wnosize",
        "no size attribute is specified for this type",
        LVLWARNING,
    },
    { "wstackh",
        "$ value(s) stay on the stack",
        LVLWARNING,
    },
    { "wunreco",
        "unreachable code detected",
        LVLWARNING,
    },
    { "wmultio",
        "output already specified",
        LVLWARNING,
    },

    { "mtokomi",
        "'$' is omittable",
        LVLMESSAGE,
    },
    { "mmultif",
        "'$' is included multiple times",
        LVLMESSAGE,
    },
    { "mnotref",
        "'$' is defined, but never referenced",
        LVLMESSAGE,
    },
    { "msccess",
        "executable successfully generated",
        LVLMESSAGE,
    },
};

void addDgnLoc(context* c, DGNKIND kind, loc loc, char* prm) {
    dgn d = { kind, loc, prm };
    dgnListAdd(&c->dgns, d);
}
void addDgnEmpty(context* c, DGNKIND kind) {
    addDgnLoc(c, kind, c->loc, NULL);
}
void addDgnEmptyLoc(context* c, DGNKIND kind, loc loc) {
    addDgnLoc(c, kind, loc, NULL);
}
void addDgn(context* c, DGNKIND kind, char* prm) {
    addDgnLoc(c, kind, c->loc, prm);
}
string dgnToString(context* c, u d) {
    string res;
    if (c->flags & FFLYCHECK) {
        res = stringClone(c->inputs.items[c->dgns.items[d].loc.file]);
        stringAdd(&res, ':');
        addCptr(&res, utos(c->dgns.items[d].loc.ln));
        stringAdd(&res, ':');
        addCptr(&res, utos(c->dgns.items[d].loc.cl + 1));
        addCptr(&res, DGNS[c->dgns.items[d].kind].lvl == LVLERROR ? ":error:" : DGNS[c->dgns.items[d].kind].lvl == LVLWARNING ? ":warning:" : ":message:");
        addCptr(&res, DGNS[c->dgns.items[d].kind].msg);
        if (c->dgns.items[d].prm != NULL)
            replaceAllCptr(&res, "$", c->dgns.items[d].prm);
    } else {
        res = stringify(DGNS[c->dgns.items[d].kind].lvl == LVLERROR ? "error: " : DGNS[c->dgns.items[d].kind].lvl == LVLWARNING ? "warning: " : "message: ");
        addCptr(&res, DGNS[c->dgns.items[d].kind].msg);
        if (c->dgns.items[d].prm != NULL)
            replaceAllCptr(&res, "$", c->dgns.items[d].prm);
        addCptr(&res, "\n");
        if (c->dgns.items[d].loc.file < c->inputs.len) {
            addCptr(&res, "\tin file '");
            stringAddRange(&res, c->inputs.items[c->dgns.items[d].loc.file]);
            addCptr(&res, "', at line: ");
            addCptr(&res, utos(c->dgns.items[d].loc.ln));
            addCptr(&res, ", column: ");
            addCptr(&res, utos(c->dgns.items[d].loc.cl));
        }
        if ((DGNS[c->dgns.items[d].kind].lvl & (LVLWARNING | LVLMESSAGE)) != 0) {
            addCptr(&res, " (-");
            addCptr(&res, DGNS[c->dgns.items[d].kind].id);
            addCptr(&res, ")");
        }
    }
    return res;
}
static bool includeDgn(context* c, u d, u h) {
    return ((c->flags & FFLYCHECK) != 0 && c->dgns.items[d].loc.file == 0) ||
            ((c->flags & FFLYCHECK) == 0 &&
             ((DGNS[c->dgns.items[d].kind].lvl == h || h > LVLERROR) &&
              !((c->flags & FIGNOREMSGS) != 0 && DGNS[c->dgns.items[d].kind].lvl == LVLMESSAGE ||
                (c->flags & FIGNOREWRNGS) != 0 && DGNS[c->dgns.items[d].kind].lvl == LVLWARNING ||
                uListContains(c->ignoreDgns, c->dgns.items[d].kind)) &&
              (h >= LVLERROR || c->dgns.items[d].loc.file >= c->inputs.len || !isStd(c, c->inputs.items[c->dgns.items[d].loc.file]))));
}
LVL highestLVL(context* c) {
    LVL res = LVLMESSAGE;
    for (u i = 0; i < c->dgns.len && res != LVLERROR; i++)
        if (DGNS[c->dgns.items[i].kind].lvl > res && includeDgn(c, i, LVLERROR + 1))
            res = DGNS[c->dgns.items[i].kind].lvl;
    return res;
}
void printDgns(context* c) {
    LVL h = highestLVL(c);
    for (u i = 0; i < c->dgns.len; i++)
        if (includeDgn(c, i, h))
            puts(cptrify(dgnToString(c, i)));
}
