#include "h/objects.h"

listDefine(opcPtr);
listDefine(att);
listDefine(varDef);
listDefine(ref);
listDefine(funDef);
listDefine(typDef);
listDefineDefault(dgnDscrPtr);

const attDef ATTRIBUTES[ATTCOUNT] = {
    { "signed",
        FNONE,
        FSINGLE,
        FTYP,
    },
    { "main",
        FNONE,
        FSINGLE,
        FFUN,
    },
    { "use",
        FSTR,
        FNOFLAGS,
        FFILE,
    },
    { "export",
        FNONE,
        FSINGLE,
        FFUN | FGLB,
    },
};
