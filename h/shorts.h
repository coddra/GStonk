#ifndef SHORTS_H
#define SHORTS_H
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t u;
typedef uintptr_t ptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef double d;

#define list(type) type##List
#define listDeclare(type)                                               \
    typedef struct {                                                    \
        u len;                                                          \
        u cap;                                                          \
        type* items;                                                    \
    } list(type);                                                       \
    list(type) type##ListDefault();                                     \
    list(type)* type##ListNew();                                        \
    list(type) type##ListFromArray(type* items, u count);               \
    list(type) type##ListClone(list(type) original);                    \
    void type##ListAdd(list(type)* list, type item);                    \
    void type##ListAddRange(list(type)* list, list(type) other);        \
    void type##ListInsert(list(type)* list, type item, u index);        \
    void type##ListInsertRange(list(type)* list, list(type) other, u index); \
    void type##ListRemove(list(type)* list, u index);                   \
    void type##ListRemoveRange(list(type)* list, u index, u count);     \
    list(type) type##ListGetRange(list(type) list, u index, u count);
#define listDeclareVaList(type)                                         \
    list(type) type##ListFromVaList(u count, ...);
#define listDeclareEquals(type)                                         \
    listDeclare(type);                                                  \
    bool type##ListContains(list(type) list, type item);                \
    bool type##ListRangeEquals(list(type) list, list(type) other, u index); \
    bool type##ListEquals(list(type) list, list(type) other);           \
    bool type##ListStartsWith(list(type) list, list(type) other);       \
    bool type##ListEndsWith(list(type) list, list(type) other);         \
    void type##ListReplaceAll(list(type)* list, list(type) original, list(type) replacement)
#define listDeclareDefault(type)                                        \
    listDeclareEquals(type);                                            \
    bool type##Equals(type left, type right)

#define listDefine(type)                                                \
    list(type) type##ListDefault() {                                    \
        list(type) res = { 0 };                                         \
        return res;                                                     \
    }                                                                   \
    list(type) type##ListGetForUse() {                                  \
        list(type) res = { 0, 16, (type*)malloc(16 * sizeof(type)) };   \
        return res;                                                     \
    }                                                                   \
    list(type)* type##ListNew() {                                       \
        list(type)* res = malloc(sizeof(list(type)));                   \
        *res = type##ListDefault();                                     \
        return res;                                                     \
    }                                                                   \
    list(type) type##ListFromArray(type* items, u count) {              \
        list(type) res = { count, 16, NULL };                           \
        while(res.len > res.cap)                                        \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        memcpy(res.items, items, res.len);                              \
        return res;                                                     \
    }                                                                   \
    list(type) type##ListClone(list(type) original) {                   \
        if (original.cap == 0)                                           \
            return original;                                            \
        list(type) res = { original.len, original.cap, (type*)malloc(original.cap * sizeof(type)) }; \
        memcpy(res.items, original.items, original.len);                \
        return res;                                                     \
    }                                                                   \
    void type##ListAdd(list(type)* list, type item) {                   \
        if (list->cap == 0)                                               \
            *list = type##ListGetForUse();                              \
        list->len++;                                                     \
        if (list->len > list->cap) {                                      \
            list->cap <<= 1;                                             \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
        list->items[list->len - 1] = item;                                \
    }                                                                   \
    void type##ListAddRange(list(type)* list, list(type) other) {       \
        if (other.len == 0)                                              \
            return;                                                     \
        if (list->cap == 0)                                               \
            *list = type##ListGetForUse();                              \
        list->len += other.len;                                          \
        if (list->len > list->cap) {                                      \
            while (list->len > list->cap)                                 \
                list->cap <<= 1;                                         \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
        memcpy((void*)((ptr)list->items + (list->len - other.len) * sizeof(type)), \
               (void*)other.items,                                      \
               other.len * sizeof(type));                               \
    }                                                                   \
    void type##ListInsert(list(type)* list, type item, u index) {       \
        if (list->cap == 0)                                               \
            *list = type##ListGetForUse();                              \
        list->len++;                                                     \
        if (list->len > list->cap) {                                      \
            list->cap <<= 1;                                             \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
        memcpy((void*)((ptr)list->items + (index + 1) * sizeof(type)),   \
               (void*)((ptr)list->items + index * sizeof(type)),         \
               list->len - index - 1);                                   \
        list->items[index] = item;                                       \
    }                                                                   \
    void type##ListInsertRange(list(type)* list, list(type) other, u index) { \
        if (other.len == 0)                                              \
            return;                                                     \
        if (list->cap == 0)                                               \
            *list = type##ListGetForUse();                              \
        list->len += other.len;                                         \
        if (list->len > list->cap) {                                     \
            while (list->len > list->cap)                                \
                list->cap <<= 1;                                         \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
        memcpy((void*)((ptr)list->items + (index + other.len) * sizeof(type)), \
               (void*)((ptr)list->items + index * sizeof(type)),         \
               list->len - index - other.len);                           \
        memcpy((void*)((ptr)list->items + index * sizeof(type)),         \
               other.items, other.len);                                 \
    }                                                                   \
    void type##ListRemove(list(type)* list, u index) {                  \
        memcpy((void*)((ptr)(list->items) + index * sizeof(type)),       \
               (void*)((ptr)(list->items) + (index + 1) * sizeof(type)), \
               list->len - index - 1);                                   \
        list->len--;                                                     \
        if (list->len < list->cap >> 1 && list->cap > 16) {             \
            while (list->len < list->cap >> 1 && list->cap > 16)          \
                list->cap >>=1;                                          \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
    }                                                                   \
    void type##ListRemoveRange(list(type)* list, u index, u count) {    \
        memcpy((void*)((ptr)(list->items) + index * sizeof(type)),       \
               (void*)((ptr)(list->items) + (index + count) * sizeof(type)), \
               list->len - index - count);                               \
        list->len -= count;                                              \
        if (list->len < list->cap >> 1 && list->cap > 16) {                \
            while (list->len < list->cap >> 1 && list->cap > 16)           \
                list->cap >>=1;                                          \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
    }                                                                   \
    list(type) type##ListGetRange(list(type) list, u index, u count) {  \
        if (count == 0) {                                               \
            list(type) res = { 0 };                                    \
            return res;                                                 \
        }                                                               \
        list(type) res = { count, 16, NULL };                           \
        while (res.len > res.cap)                                       \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        memcpy(res.items, &list.items[index], count);                   \
        return res;                                                     \
    }
#define listDefineVaList(type)                                          \
    list(type) type##ListFromVaList(u count, ...) {                     \
        va_list args;                                                   \
        va_start(args, count);                                          \
        list(type) res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = va_arg(args, typ);                           \
        va_end(args);                                                   \
        return res;                                                     \
    }
#define listDefineVaListInt(type)                                       \
    list(type) type##ListFromVaList(u count, ...) {                     \
        va_list args;                                                   \
        va_start(args, count);                                          \
        list(type) res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = va_arg(args, int);                           \
        va_end(args);                                                   \
        return res;                                                     \
    }
#define listDefineEquals(type)                                          \
    listDefine(type)                                                    \
        bool type##ListContains(list(type) list, type item) {           \
        for (u i = 0; i < list.len; i++)                                \
            if (type##Equals(list.items[i], item))                      \
                return true;                                            \
        return false;                                                   \
    }                                                                   \
    bool type##ListRangeEquals(list(type) list, list(type) other, u index) { \
        if (other.len + index > list.len)                               \
            return false;                                               \
        if (list.len == 0 && other.len == 0)                              \
            return true;                                                \
        for (u i = 0; i < other.len; i++)                               \
            if (!type##Equals(list.items[i + index], other.items[i]))   \
                return false;                                           \
        return true;                                                    \
    }                                                                   \
    bool type##ListEquals(list(type) list, list(type) other) {   \
        return list.len == other.len && type##ListRangeEquals(list, other, 0); \
    }                                                                   \
    bool type##ListStartsWith(list(type) list, list(type) other) { \
        return other.len <= list.len && type##ListRangeEquals(list, other, 0); \
    }                                                                   \
    bool type##ListEndsWith(list(type) list, list(type) other) { \
        return (other.len <= list.len) && type##ListRangeEquals(list, other, list.len - other.len); \
    }                                                                   \
    void type##ListReplaceAll(list(type)* list, list(type) original, list(type) replacement) { \
        if (original.len == 0 || list->len < original.len)                \
            return;                                                     \
        uList indexes = uListDefault();                                 \
        for (u i = 0; i <= list->len - original.len; i++)                \
            if (type##ListRangeEquals(*list, original, i)) {            \
                uListAdd(&indexes, i);                                  \
                i += original.len - 1;                                  \
            }                                                           \
        for (u i = 0; i < indexes.len; i++) {                           \
            type##ListRemoveRange(list, indexes.items[indexes.len - 1 - i], original.len); \
            type##ListInsertRange(list, replacement, indexes.items[indexes.len - 1 - i]); \
        }                                                               \
    }
#define listDefineDefault(type)                                         \
    listDefineEquals(type)                                              \
    bool type##Equals(type left, type right) {                   \
        return left == right;                                           \
    }

typedef enum {
    SCSADD,
    SCSSUB,
    SCSCRS,
} SETCOMBINATIONSTYLE;
#define set(type) type##Set
#define aggregate(type) type##Aggregate
#define range(type) type##Range
#define setDeclare(type)                                                \
    struct type##Set_s;                                                 \
    typedef struct type##Set_s set(type);                               \
    struct type##Set_s {                                                \
        bool (*contains)(set(type)* set, type item);                    \
    };                                                                  \
    typedef struct {                                                    \
        bool (*contains)(set(type)* set, type item);                    \
        list(type) items;                                               \
    } aggregate(type);                                                  \
    typedef struct {                                                    \
        bool (*contains)(set(type)* set, type item);                    \
        type##Set* orgnl;                                               \
    } type##ComplementSet;                                              \
    typedef struct {                                                    \
        bool (*contains)(set(type)* set, type item);                    \
        type##Set* left;                                                \
        type##Set* right;                                               \
        SETCOMBINATIONSTYLE scs;                                        \
    } type##CombinedSet;                                                \
    bool type##AggregateContains(set(type)* set, type item);            \
    bool type##ComplementSetContains(set(type)* set, type item);        \
    bool type##CombinedSetContains(set(type)* set, type item);          \
    aggregate(type) type##AggregateDefault();                           \
    type##ComplementSet type##ComplementSetDefault();                   \
    type##CombinedSet type##CombinedSetDefault();                       \
    set(type)* type##AggregateNew(list(type) list);                     \
    set(type)* type##AggregateFromArray(type* items, u count);          \
    set(type)* type##ComplementSetNew(set(type)* set);                  \
    set(type)* type##CombinedSetNew(set(type)* left, set(type)* right, SETCOMBINATIONSTYLE style); \
    set(type)* type##SetAdd(set(type)* left, set(type)* right);         \
    set(type)* type##SetSubstract(set(type)* left, set(type)* right);   \
    set(type)* type##Cross(set(type)* left, set(type)* right);          \
    set(type)* type##SetComplement(set(type)* set);                     \
    bool type##SetContains(set(type)* set, type item)
#define setDeclareVaList(type)                                          \
    set(type)* type##AggregateFromVaList(u count, ...);
#define setDeclareCompare(type)                                         \
    setDeclare(type);                                                   \
    typedef struct {                                                    \
        bool (*contains)(set(type)* set, type item);                    \
        bool inclMin;                                                   \
        bool inclMax;                                                   \
        type min;                                                       \
        type max;                                                       \
    } range(type);                                                      \
    bool type##RangeContains(set(type)* set, type item);                \
    range(type) type##RangeDefault();                                   \
    type##Set* type##RangeNew(type min, type max)
#define setDeclareDefault(type)                                         \
    setDeclareCompare(type);                                            \
    bool type##LessThan(type left, type right);                         \
    bool type##GreaterThan(type left, type right)

#define setDefine(type)                                                 \
    bool type##AggregateContains(set(type)* set, type item) {           \
        return type##ListContains(as(aggregate(type), set)->items, item); \
    }                                                                   \
    bool type##ComplementSetContains(set(type)* set, type item) {       \
        return !(*as(type##ComplementSet, set)->orgnl->contains)(as(type##ComplementSet, set)->orgnl, item); \
    }                                                                   \
    bool type##CombinedSetContains(set(type)* set, type item) {         \
        bool left = (*as(type##CombinedSet, set)->left->contains)(as(type##CombinedSet, set)->left, item); \
        bool right = (*as(type##CombinedSet, set)->right->contains)(as(type##CombinedSet, set)->right, item); \
        return (as(type##CombinedSet, set)->scs == SCSADD && (left || right)) || \
               (as(type##CombinedSet, set)->scs == SCSSUB && (left && !right)) || \
               (as(type##CombinedSet, set)->scs == SCSCRS && (left && right)); \
    }                                                                   \
    aggregate(type) type##AggregateDefault() {                          \
        list(type) tmp = { 0 };                                         \
        aggregate(type) res = { &type##AggregateContains, tmp };        \
        return res;                                                     \
    }                                                                   \
    type##ComplementSet type##ComplementSetDefault() {                  \
        type##ComplementSet res = { &type##ComplementSetContains, NULL }; \
        return res;                                                     \
    }                                                                   \
    type##CombinedSet type##CombinedSetDefault() {                      \
        type##CombinedSet res = { &type##CombinedSetContains, NULL, NULL, SCSADD }; \
        return res;                                                     \
    }                                                                   \
    type##Set* type##AggregateNew(list(type) items) {                   \
        set(type)* res = as(set(type), alloc(aggregate(type)));         \
        *as(aggregate(type), res) = type##AggregateDefault();           \
        as(aggregate(type), res)->items = items;                         \
        return res;                                                     \
    }                                                                   \
    type##Set* type##ComplementSetNew(set(type)* set) {                 \
        set(type)* res = as(set(type), alloc(type##ComplementSet));     \
        *as(type##ComplementSet, res) = type##ComplementSetDefault();   \
        as(type##ComplementSet, res)->orgnl = set;                       \
        return res;                                                     \
    }                                                                   \
    type##Set* type##CombinedSetNew(set(type)* left, set(type)* right, SETCOMBINATIONSTYLE style) { \
        set(type)* res = as(set(type), alloc(type##CombinedSet));       \
        *as(type##CombinedSet, res) = type##CombinedSetDefault();       \
        as(type##CombinedSet, res)->left = left;                         \
        as(type##CombinedSet, res)->right = right;                       \
        as(type##CombinedSet, res)->scs = style;                         \
        return res;                                                     \
    }                                                                   \
    set(type)* type##AggregateFromArray(type* items, u count) {         \
        return type##AggregateNew(type##ListFromArray(items, count));   \
    }                                                                   \
    set(type)* type##SetAdd(set(type)* left, set(type)* right) {        \
        return type##CombinedSetNew(left, right, SCSADD);               \
    }                                                                   \
    set(type)* type##SetSubstract(set(type)* left, set(type)* right) {  \
        return type##CombinedSetNew(left, right, SCSSUB);               \
    }                                                                   \
    set(type)* type##Cross(set(type)* left, set(type)* right) {         \
        return type##CombinedSetNew(left, right, SCSCRS);               \
    }                                                                   \
    set(type)* type##SetComplement(set(type)* set) {                    \
        return type##ComplementSetNew(set);                             \
    }                                                                   \
    bool type##SetContains(set(type)* set, type item) {                 \
        return (*set->contains)(set, item);                             \
    }
#define setDefineVaList(type)                                           \
    set(type)* type##AggregateFromVaList(u count, ...) {                \
        va_list args;                                                   \
        va_start(args, count);                                          \
        list(type) res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                               \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = va_arg(args, type);                          \
        va_end(args);                                                   \
        return type##AggregateNew(res);                                 \
    }
#define setDefineVaListInt(type)                                        \
    set(type)* type##AggregateFromVaList(u count, ...) {                \
        va_list args;                                                   \
        va_start(args, count);                                          \
        list(type) res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = (type)va_arg(args, u);                       \
        va_end(args);                                                   \
        return type##AggregateNew(res);                                 \
    }
#define setDefineCompare(type)                                          \
    setDefine(type);                                                    \
    bool type##RangeContains(set(type)* set, type item) {               \
        return (type##LessThan(as(range(type), set)->min, item) || as(range(type), set)->inclMin && as(range(type), set)->min == item) && \
               (type##GreaterThan(as(range(type), set)->max, item) || as(range(type), set)->inclMax && as(range(type), set)->max == item); \
    }                                                                   \
    range(type) type##RangeDefault() {                                  \
        range(type) res = { 0 };                                        \
        res.contains = &type##RangeContains;                            \
        res.inclMin = true;                                             \
        res.inclMax = true;                                             \
        return res;                                                     \
    }                                                                   \
    type##Set* type##RangeNew(type min, type max) {                     \
        set(type)* res = as(set(type), alloc(range(type)));             \
        *as(range(type), res) = type##RangeDefault();                   \
        as(range(type), res)->min = min;                                 \
        as(range(type), res)->max = max;                                 \
        return res;                                                     \
    }
#define setDefineDefault(type)                                          \
    setDefineCompare(type);                                             \
    bool type##LessThan(type left, type right) {                        \
        return left < right;                                            \
    }                                                                   \
    bool type##GreaterThan(type left, type right) {                     \
        return left > right;                                            \
    }

enum {
    objectType,
};
#define objectDerive                            \
    u TYPE
typedef struct {
    objectDerive;
} object;
void* newObject(u, u);
#define new(type) ((type*)newObject(sizeof(type), type##Type))
#define alloc(type) ((type*)malloc(sizeof(type)))
#define as(type, obj) ((type*)obj)
#define is(type, obj) ((obj)->TYPE == type##Type)
#define max(type) (2 << (sizeof(type) * 8 - 1))

#define init(h) h##Init()//just to make it stand out

listDeclareDefault(u);
listDeclareVaList(u);
listDeclareDefault(char);
listDeclareVaList(char);
typedef charList string;
string stringDefault();
string* stringNew();
string stringFromArray(char* str, u count);
string stringClone(string str);
void stringAdd(string* str, char item);
void stringAddRange(string* str, string other);
void stringInsert(string* str, char item, u index);
void stringInsertRange(string* str, string other, u index);
void stringRemove(string* str, u index);
void stringRemoveRange(string* str, u index, u count);
string stringGetRange(string str, u index, u count);
bool stringContains(string str, char item);
bool stringRangeEquals(string str, string other, u index);
bool stringStartsWith(string str, string other);
bool stringEndsWith(string str, string other);
void stringReplaceAll(string* str, string old, string repl);
bool stringEquals(string str, string other);
string substring(string str, u index);
string stringify(char* str);
char* cptrify(string str);
string statstr(char* str);

void addCptr(string* str, char* other);
void replaceAllCptr(string* str, char* orig, char* repl);

char* utos(u64 u);
char* itos(i64 i);

char* concat(char* str, char* other);

char getEscChar(char c);
char* ctcptr(char c);

string readAllText(string file);
void writeAllText(string file, string text);
bool fileExists(string file);

#endif //SHORTS_H
