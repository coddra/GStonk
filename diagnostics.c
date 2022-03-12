#include "h/diagnostics.h"

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
        "wrong kind of parameter provided for opcode '{0}'",
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
    }
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
string dgnToString(dgn d) {
    string res = DGNS[d.descr].lvl == LVLERROR ? stringify("error: ") : DGNS[d.descr].lvl == LVLWARNING ? stringify("warning: ") : stringify("message: ");
    addCptr(&res, DGNS[d.descr].msg);
    for (u i = 0; i < d.params.len; i++)
        replaceAllCptr(&res, concat(concat("{", utos(i)), "}"), d.params.items[i]);
    addCptr(&res, "\n");
    if (d.loc.file.len != 0) {
        addCptr(&res, "\tin file '");
        stringAddRange(&res, d.loc.file);
        addCptr(&res, "', at line: ");
        addCptr(&res, utos(d.loc.ln));
        addCptr(&res, ", column: ");
        addCptr(&res, utos(d.loc.cl));
    }
    if ((DGNS[d.descr].lvl & (LVLWARNING | LVLMESSAGE)) != 0) {
        addCptr(&res, " (-");
        addCptr(&res, DGNS[d.descr].id);
        addCptr(&res, ")");
    }
    stringAdd(&res, '\n');
    return res;
}
void printDgns(context* c) {
    for (size_t i = 0; i < c->dgns.len; i++)
        if (!((c->flags & FIGNOREMSGS) != 0 && DGNS[c->dgns.items[i].descr].lvl == LVLMESSAGE || (c->flags & FIGNOREWRNGS) != 0 && DGNS[c->dgns.items[i].descr].lvl == LVLWARNING || uListContains(c->ignoreDgns, c->dgns.items[i].descr)))
            puts(cptrify(dgnToString(c->dgns.items[i])));
}
bool checkErr(context* c) {
    size_t i;
    for (i = 0; i < c->dgns.len && DGNS[c->dgns.items[i].descr].lvl != LVLERROR; i++);
    return i < c->dgns.len;
}
