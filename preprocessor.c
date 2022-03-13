#include "h/preprocessor.h"

void preprocess(context* c) {
    replaceAllCptr(&c->text, "\r\n", "\n");
    replaceAllCptr(&c->text, "\r", "\n");
    for (char i = '\0'; i < ' '; i++)
        if (i != '\n' && i != '\t')
            charListRemoveAll(&c->text, i);
    u cs = c->text.len;
    for (u i = 0; i < c->text.len - 1; i++) {
        if (cs >= c->text.len && c->text.items[i] == ';' && c->text.items[i + 1] == ';')
            cs = i;
        if (cs < c->text.len && c->text.items[i] == '\n') {
            stringRemoveRange(&c->text, cs, i - cs);
            i = cs;
            cs = c->text.len;
        }
    }
}
