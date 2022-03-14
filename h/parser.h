#ifndef PARSER_H
#define PARSER_H

#include "objects.h"

void init(PARSER);
bool parseBody(context* c, body* res, u r);
void completeBody(context* c, body* b, u r);
void parse(context* c);

#endif //PARSER_H
