#include "h/diagnostics.h"
#include "h/linker.h"

const dgnDscr DGNS[DGNCOUNT] = {
    { "",
        "missing token '{0}'",
        LVLERROR,
    },
    { "",
        "missing {0}",
        LVLERROR,
    },
    { "",
        "unrecognized escapesequence '{0}'",
        LVLERROR,
    },
    { "",
        "second declaration of '{0}' found",
        LVLERROR,
    },
    { "",
        "{0} out of range",
        LVLERROR,
    },
    { "",
        "unrecognized token '{0}'",
        LVLERROR,
    },
    { "",
        "definition of '{0}' not found",
        LVLERROR,
    },
    { "",
        "wrong number of parameters provided for '{0}'",
        LVLERROR,
    },
    { "",
        "wrong number of arguments defined for '{0}'",
        LVLERROR,
    },
    { "",
        "multiple functions found with attributes 'std.main'",
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
        "there is no attribue with name '{0}'",
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
        "file '{0}' does not exist",
        LVLERROR,
    },
    { "",
        "opcode '{0}' does not take any argument",
        LVLERROR,
    },
    { "",
        "wrong kind of parameter provided for '{0}'",
        LVLERROR,
    },
    { "",
        "gcc returned exit status '{0}'",
        LVLERROR,
    },

    { "wnosize",
        "no size attribute is specified for this type",
        LVLWARNING,
    },
    { "wstackh",
        "{0} value(s) stay on the stack",
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
        "'{0}' is omittable",
        LVLMESSAGE,
    },

    { "mmultif",
        "'{0}' is included multiple times",
        LVLMESSAGE,
    },

    { "mnotref",
        "'{0}' is defined, but never referenced",
        LVLMESSAGE,
    },
    { "msccess",
        "executable successfully generated",
        LVLMESSAGE,
    },
};

void addDgnMultiLoc(context* c, DGNKIND desc, loc loc, list(charPtr) params) {
    dgn d = { desc, loc, params };
    dgnListAdd(&c->dgns, d);
}
void addDgnLoc(context* c, DGNKIND desc, loc loc, char* param) {
    list(charPtr) ps = charPtrListDefault();
    charPtrListAdd(&ps, param);
    addDgnMultiLoc(c, desc, loc, ps);
}
void addDgnMulti(context* c, DGNKIND desc, list(charPtr) params) {
    addDgnMultiLoc(c, desc, c->loc, params);
}
void addDgnEmpty(context* c, DGNKIND desc) {
    addDgnMultiLoc(c, desc, c->loc, charPtrListDefault());
}
void addDgnEmptyLoc(context* c, DGNKIND desc, loc loc) {
    addDgnMultiLoc(c, desc, loc, charPtrListDefault());
}
void addDgn(context* c, DGNKIND desc, char* param) {
    list(charPtr) ps = charPtrListDefault();
    charPtrListAdd(&ps, param);
    addDgnMultiLoc(c, desc, c->loc, ps);
}
string dgnToString(context* c, u d) {
    string res = DGNS[c->dgns.items[d].descr].lvl == LVLERROR ? stringify("error: ") : DGNS[c->dgns.items[d].descr].lvl == LVLWARNING ? stringify("warning: ") : stringify("message: ");
    addCptr(&res, DGNS[c->dgns.items[d].descr].msg);
    for (u i = 0; i < c->dgns.items[d].params.len; i++)
        replaceAllCptr(&res, concat(concat("{", utos(i)), "}"), c->dgns.items[d].params.items[i]);
    addCptr(&res, "\n");
    if (c->dgns.items[d].loc.file < c->inputs.len) {
        addCptr(&res, "\tin file '");
        stringAddRange(&res, c->inputs.items[c->dgns.items[d].loc.file]);
        addCptr(&res, "', at line: ");
        addCptr(&res, utos(c->dgns.items[d].loc.ln));
        addCptr(&res, ", column: ");
        addCptr(&res, utos(c->dgns.items[d].loc.cl));
    }
    if ((DGNS[c->dgns.items[d].descr].lvl & (LVLWARNING | LVLMESSAGE)) != 0) {
        addCptr(&res, " (-");
        addCptr(&res, DGNS[c->dgns.items[d].descr].id);
        addCptr(&res, ")");
    }
    return res;
}
LVL highestLVL(context* c) {
    LVL res = LVLMESSAGE;
    for (u i = 0; i < c->dgns.len && res != LVLERROR; i++)
        if (DGNS[c->dgns.items[i].descr].lvl > res)
            res = DGNS[c->dgns.items[i].descr].lvl;
    return res;
}
void printDgns(context* c) {
    LVL h = highestLVL(c);
    for (u i = 0; i < c->dgns.len; i++)
        if (DGNS[c->dgns.items[i].descr].lvl == h &&
            !((c->flags & FIGNOREMSGS) != 0 && DGNS[c->dgns.items[i].descr].lvl == LVLMESSAGE ||
              (c->flags & FIGNOREWRNGS) != 0 && DGNS[c->dgns.items[i].descr].lvl == LVLWARNING ||
              uListContains(c->ignoreDgns, c->dgns.items[i].descr)) &&
            (c->dgns.items[i].loc.file >= c->inputs.len || !isStd(c, c->inputs.items[c->dgns.items[i].loc.file])))
            puts(cptrify(dgnToString(c, i)));
}
