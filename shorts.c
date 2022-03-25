#include "h/shorts.h"
#include <unistd.h>

listDefineDefault(u);
listDefineVaListInt(u);
listDefineDefaultName(char, string);
listDefineVaListIntName(char, string);

string substring(string str, u index) {
    return stringGetRange(str, index, str.len - index);
}
string stringify(char* str) {
    if (str)
        return stringFromArray(str, strlen(str));
    else
        return stringFromArray(NULL, 0);
}
char* cptrify(string str) {
    char* res = (char*)malloc((str.len + 1) * sizeof(char));
    memcpy(res, str.items, str.len);
    res[str.len] = 0;
    return res;
}
string statstr(char* str) {
    string res = { strlen(str), 0, str };
    res.cap = res.len;
    return res;
}

void addCptr(string* str, char* other) {
    stringAddRange(str, statstr(other));
}
void replaceAllCptr(string* str, char* orig, char* repl) {
    stringReplaceAll(str, statstr(orig), statstr(repl));
}

char* utos(u64 u) {
    char* res = (char*)malloc(64);
    snprintf(res, 64, "%zu", u);
    return res;
}
char* itos(i64 i) {
    char* res = (char*)malloc(64);
    snprintf(res, 64, "%zd", i);
    return res;
}

char* concat(char* str, char* other) {
    u len = strlen(str);
    u olen = strlen(other);
    char* res = (char*)malloc(len + olen + 1);
    memcpy(res, str, len);
    memcpy((void*)((ptr)res + len), other, olen);
    res[len + olen] = 0;
    return res;
}

char getEscChar(char c) {
    switch (c) {
        case 'a':
            return '\a';
            break;
        case 'b':
            return '\b';
            break;
        case 'f':
            return '\f';
            break;
        case 'n':
            return '\n';
            break;
        case 'r':
            return '\r';
            break;
        case 't':
            return '\t';
            break;
        case 'v':
            return '\v';
            break;
        case '\'':
            return '\'';
            break;
        case '"':
            return '\"';
            break;
        case '\\':
            return '\\';
            break;
        case '?':
            return '\?';
            break;
        default:
            return 0;
    }
}

char* ctcptr(char c) {
    char* res = malloc(2);
    res[0] = c;
    res[1] = 0;
    return res;
}

string readAllText(string file) {
    FILE* fp = fopen(cptrify(file), "r");
    fseek(fp, 0l, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0l, SEEK_SET);
    char* res = malloc(size);
    fread(res, 1, size, fp);
    return stringify(res);
}
void writeAllText(string file, string text) {
    FILE* f = fopen(cptrify(file), "w");
    fputs(cptrify(text), f);
    fclose(f);
}
bool fileExists(string file) {
    return access(cptrify(file), F_OK) == 0;
}
string absolutePath(string path) {
    return stringify(realpath(cptrify(path), NULL));
}
