#include "h/preprocessor.h"

void preprocess(context* c) {
    normalize(c);
    u cs = c->text.len;
    for (u i = 0; i < c->text.len - 1; i++) {
        if (c->text.items[i] == '\'' && (i == 0 || c->text.items[i - 1] != '\\'))
            while (i++ < c->text.len && (c->text.items[i] != '\'' || c->text.items[i - 1] == '\\'));
        else if (c->text.items[i] == '"' && (i == 0 || c->text.items[i - 1] != '\\'))
            while (i++ < c->text.len && (c->text.items[i] != '"' || c->text.items[i - 1] == '\\'));
        else {
            if (cs >= c->text.len && c->text.items[i] == ';' && c->text.items[i + 1] == ';')
                cs = i;
            if (cs < c->text.len && c->text.items[i] == '\n') {
                stringRemoveRange(&c->text, cs, i - cs);
                i = cs;
                cs = c->text.len;
            }
        }
    }
}
