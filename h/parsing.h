#ifndef PARSING_H
#define PARSING_H

#include "linking.h"

void init(PARSING);

setDeclareDefault(char);
setDeclareVaList(char);

extern set(char)* whitespace;
extern set(char)* letters;
extern set(char)* digits;
extern set(char)* hexDigits;
extern set(char)* specialChars;
extern set(char)* escChars;
extern set(char)* newLine;
extern set(char)* stringCloser;
extern set(char)* stringLiteral;
extern set(char)* charLiteral;
extern set(char)* notWhitespace;
extern set(char)* opSymbols;

extern const int tabWidth;

bool next(context* c);//returns a value mainly to make it easier to skip a char when returning a value from another function
bool eof(context* c);
bool neof(context* c);
bool parseC(context* c, char ch);
bool parseCS(context* c, set(char)* cs);
bool parseCSR(context* c, set(char)* cs, char* res);
bool parseAllCS(context* c, set(char)* cs);
bool parseAllNot(context* c, set(char)* cs);
bool parseCptr(context* c, char* s);
bool lookingAtC(context* c, char ch);
bool nlookingAtC(context* c, char ch);
bool lookingAtCS(context* c, set(char)* cs);
bool nlookingAtCS(context* c, set(char)* cs);
bool parseHex(context* c, u64* res, u8 digits);
bool parseES(context* c, char* res);
bool parseCC(context* c, char* res, bool str);
bool parseCL(context* c, char* res);
bool parseIL(context* c, i64* res);
bool parseUL(context* c, u64* res);
bool parseDL(context* c, d* res);
bool parseSL(context* c, ref* res);
bool parseName(context* c, name* res);
bool parseIdfr(context* c, name* res);
bool parseTypIdfr(context* c, name* res);
bool parseTypList(context* c, string* res, list(ref)* refs);
bool parseFunIdfr(context* c, name* res);
bool parseVarIdfr(context* c, name* res);
bool parseFun(context* c, ref* res);
bool parseTyp(context* c, ref* res);
bool parseGlb(context* c, ref* res);
bool parseVar(context* c, ref* res, list(varDef)* l);
bool parsePar(context* c, par* res, AFLAG kind, u r);
bool parseAtt(context* c, att* res);
bool parseAttList(context* c, list(att)* res);
bool parseVarDef(context* c, varDef* res);
bool parseVarList(context* c, list(varDef)* res);
bool parseOP(context* c, OP op);
bool parseOPC(context* c, opc** res, u r);
bool parseHead(context* c, list(opcPtr)* res, u r);
bool parseBody(context* c, list(opcPtr)* res, u r);
bool parseGlbDef(context* c);
bool parseTypDef(context* c);
bool parseFunDef(context* c);
void parse(context* c);

#endif //PARSING_H
