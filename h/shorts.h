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
#define listDeclareName(type, name)                                     \
    typedef struct {                                                    \
        u len;                                                          \
        u cap;                                                          \
        type* items;                                                    \
    } name;                                                             \
    name name##Default();                                               \
    name* name##New();                                                  \
    name name##FromArray(type* items, u count);                         \
    name name##Clone(name original);                                    \
    void name##Add(name* list, type item);                              \
    void name##AddRange(name* list, name other);                        \
    void name##Insert(name* list, type item, u index);                  \
    void name##InsertRange(name* list, name other, u index);            \
    void name##Remove(name* list, u index);                             \
    void name##RemoveRange(name* list, u index, u count);               \
    name name##GetRange(name list, u index, u count);
#define listDeclareVaListName(type, name)       \
    name name##FromVaList(u count, ...);
#define listDeclareEqualsName(type, name)                               \
    listDeclareName(type, name);                                        \
    bool name##Contains(name list, type item);                          \
    bool name##RangeEquals(name list, name other, u index);             \
    bool name##Equals(name list, name other);                           \
    bool name##StartsWith(name list, name other);                       \
    bool name##EndsWith(name list, name other);                         \
    void name##RemoveAll(name* list, type item);                        \
    void name##ReplaceAll(name* list, name original, name replacement); \
    u name##Pos(name list, type item);                                  \
    u name##LastPos(name list, type item);
#define listDeclareDefaultName(type, name)                              \
    listDeclareEqualsName(type, name);                                  \
    bool type##Equals(type left, type right)
#define listDeclare(type) listDeclareName(type, type##List)
#define listDeclareVaList(type) listDeclareVaListName(type, type##List)
#define listDeclareDefault(type) listDeclareDefaultName(type, type##List)
#define listDeclareEquals(type) listDeclareEqualsName(type, type##List)

#define listDefineName(type, name)                                      \
    name name##Default() {                                    \
        name res = { 0 };                                         \
        return res;                                                     \
    }                                                                   \
    name name##GetForUse() {                                  \
        name res = { 0, 16, (type*)malloc(16 * sizeof(type)) };   \
        return res;                                                     \
    }                                                                   \
    name* name##New() {                                       \
        name* res = malloc(sizeof(name));                   \
        *res = name##Default();                                     \
        return res;                                                     \
    }                                                                   \
    name name##FromArray(type* items, u count) {              \
        if (count == 0)                                                  \
            return name##Default();                                 \
        name res = { count, 16, NULL };                           \
        while(res.len > res.cap)                                        \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        memcpy(res.items, items, res.len);                              \
        return res;                                                     \
    }                                                                   \
    name name##Clone(name original) {                   \
        if (original.cap == 0)                                           \
            return original;                                            \
        name res = { original.len, original.cap, (type*)malloc(original.cap * sizeof(type)) }; \
        memcpy(res.items, original.items, original.len);                \
        return res;                                                     \
    }                                                                   \
    void name##Add(name* list, type item) {                   \
        if (list->cap == 0)                                               \
            *list = name##GetForUse();                              \
        list->len++;                                                     \
        if (list->len > list->cap) {                                      \
            list->cap <<= 1;                                             \
            list->items = (type*)realloc(list->items, list->cap * sizeof(type)); \
        }                                                               \
        list->items[list->len - 1] = item;                                \
    }                                                                   \
    void name##AddRange(name* list, name other) {       \
        if (other.len == 0)                                              \
            return;                                                     \
        if (list->cap == 0)                                               \
            *list = name##GetForUse();                              \
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
    void name##Insert(name* list, type item, u index) {       \
        if (list->cap == 0)                                               \
            *list = name##GetForUse();                              \
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
    void name##InsertRange(name* list, name other, u index) { \
        if (other.len == 0)                                              \
            return;                                                     \
        if (list->cap == 0)                                               \
            *list = name##GetForUse();                              \
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
    void name##Remove(name* list, u index) {                  \
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
    void name##RemoveRange(name* list, u index, u count) {    \
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
    name name##GetRange(name list, u index, u count) {  \
        if (count == 0) {                                               \
            name res = { 0 };                                    \
            return res;                                                 \
        }                                                               \
        name res = { count, 16, NULL };                           \
        while (res.len > res.cap)                                       \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        memcpy(res.items, &list.items[index], count);                   \
        return res;                                                     \
    }
#define listDefineVaListName(type, name)                                \
    name name##FromVaList(u count, ...) {                     \
        va_list args;                                                   \
        va_start(args, count);                                          \
        name res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = va_arg(args, typ);                           \
        va_end(args);                                                   \
        return res;                                                     \
    }
#define listDefineVaListIntName(type, name)                             \
    name name##FromVaList(u count, ...) {                     \
        va_list args;                                                   \
        va_start(args, count);                                          \
        name res = { count, 16, NULL };                           \
        while (count > res.cap)                                         \
            res.cap <<= 1;                                              \
        res.items = (type*)malloc(res.cap * sizeof(type));              \
        for (u i = 0; i < count; i++)                                   \
            res.items[i] = va_arg(args, int);                           \
        va_end(args);                                                   \
        return res;                                                     \
    }
#define listDefineEqualsName(type, name)                                \
    listDefineName(type, name)                                          \
        bool name##Contains(name list, type item) {           \
        for (u i = 0; i < list.len; i++)                                \
            if (type##Equals(list.items[i], item))                      \
                return true;                                            \
        return false;                                                   \
    }                                                                   \
    bool name##RangeEquals(name list, name other, u index) { \
        if (other.len + index > list.len)                               \
            return false;                                               \
        if (list.len == 0 && other.len == 0)                              \
            return true;                                                \
        for (u i = 0; i < other.len; i++)                               \
            if (!type##Equals(list.items[i + index], other.items[i]))   \
                return false;                                           \
        return true;                                                    \
    }                                                                   \
    bool name##Equals(name list, name other) {   \
        return list.len == other.len && name##RangeEquals(list, other, 0); \
    }                                                                   \
    bool name##StartsWith(name list, name other) { \
        return other.len <= list.len && name##RangeEquals(list, other, 0); \
    }                                                                   \
    bool name##EndsWith(name list, name other) { \
        return (other.len <= list.len) && name##RangeEquals(list, other, list.len - other.len); \
    }                                                                   \
    void name##RemoveAll(name* list, type item) {             \
        for (u i = list->len; i > 0; i--)                                \
            if (type##Equals(list->items[i - 1], item))                  \
                name##Remove(list, i - 1);                          \
    }                                                                   \
    void name##ReplaceAll(name* list, name original, name replacement) { \
        if (original.len == 0 || list->len < original.len)                \
            return;                                                     \
        uList indexes = uListDefault();                                 \
        for (u i = 0; i <= list->len - original.len; i++)                \
            if (name##RangeEquals(*list, original, i)) {            \
                uListAdd(&indexes, i);                                  \
                i += original.len - 1;                                  \
            }                                                           \
        for (u i = 0; i < indexes.len; i++) {                           \
            name##RemoveRange(list, indexes.items[indexes.len - 1 - i], original.len); \
            name##InsertRange(list, replacement, indexes.items[indexes.len - 1 - i]); \
        }                                                               \
    }                                                                   \
    u name##Pos(name list, type item) {                      \
        u i = 0;                                                        \
        for (; i < list.len && !type##Equals(list.items[i], item); i++); \
        return i;                                                       \
    }                                                                   \
    u name##LastPos(name list, type item) {                  \
        u i = list.len;                                                 \
        for (; i > 0 && !type##Equals(list.items[i - 1], item); i--);   \
        if (i == 0)                                                      \
            return list.len;                                            \
        return i - 1;                                                   \
    }
#define listDefineDefaultName(type, name)                               \
    listDefineEqualsName(type, name)                                    \
        bool type##Equals(type left, type right) {                      \
        return left == right;                                           \
    }
#define listDefine(type) listDefineName(type, type##List)
#define listDefineVaList(type) listDefineVaListName(type, type##List)
#define listDefineVaListInt(type) listDefineVaListIntName(type, type##List)
#define listDefineEquals(type) listDefineEqualsName(type, type##List)
#define listDefineDefault(type) listDefineDefaultName(type, type##List)

typedef enum {
    SCSADD,
    SCSSUB,
    SCSCRS,
} SETCOMBINATIONSTYLE;
#define set(type) type##Set
#define aggregate(type) type##Aggregate
#define range(type) type##Range
#define setDeclareName(type, listName)                                  \
    struct type##Set_s;                                                 \
    typedef struct type##Set_s set(type);                               \
    struct type##Set_s {                                                \
        bool (*contains)(set(type)* set, type item);                    \
    };                                                                  \
    typedef struct {                                                    \
        bool (*contains)(set(type)* set, type item);                    \
         listName items;                                               \
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
    set(type)* type##AggregateNew(listName list);                       \
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
#define setDeclareCompareName(type, listName)                           \
    setDeclareName(type, listName);                                     \
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
#define setDeclareDefaultName(type, listName)                           \
    setDeclareCompareName(type, listName);                              \
    bool type##LessThan(type left, type right);                         \
    bool type##GreaterThan(type left, type right)
#define setDeclare(type) setDeclareName(type, type##List);
#define setDeclareCompare(type) setDeclareCompareName(type, type##List)
#define setDeclareDefault(type) setDeclareDefaultName(type, type##List)

#define setDefineName(type, listName)                                   \
    bool type##AggregateContains(set(type)* set, type item) {           \
        return listName##Contains(as(aggregate(type), set)->items, item); \
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
        listName tmp = { 0 };                                         \
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
    type##Set* type##AggregateNew(listName items) {                   \
        set(type)* res = as(set(type), new(aggregate(type)));           \
        *as(aggregate(type), res) = type##AggregateDefault();           \
        as(aggregate(type), res)->items = items;                         \
        return res;                                                     \
    }                                                                   \
    type##Set* type##ComplementSetNew(set(type)* set) {                 \
        set(type)* res = as(set(type), new(type##ComplementSet));     \
        *as(type##ComplementSet, res) = type##ComplementSetDefault();   \
        as(type##ComplementSet, res)->orgnl = set;                       \
        return res;                                                     \
    }                                                                   \
    type##Set* type##CombinedSetNew(set(type)* left, set(type)* right, SETCOMBINATIONSTYLE style) { \
        set(type)* res = as(set(type), new(type##CombinedSet));       \
        *as(type##CombinedSet, res) = type##CombinedSetDefault();       \
        as(type##CombinedSet, res)->left = left;                         \
        as(type##CombinedSet, res)->right = right;                       \
        as(type##CombinedSet, res)->scs = style;                         \
        return res;                                                     \
    }                                                                   \
    set(type)* type##AggregateFromArray(type* items, u count) {         \
        return type##AggregateNew(listName##FromArray(items, count));   \
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
#define setDefineCompareName(type, listName)                                \
    setDefineName(type, listName);                                      \
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
        set(type)* res = as(set(type), new(range(type)));             \
        *as(range(type), res) = type##RangeDefault();                   \
        as(range(type), res)->min = min;                                 \
        as(range(type), res)->max = max;                                 \
        return res;                                                     \
    }
#define setDefineDefaultName(type, listName)                            \
    setDefineCompareName(type, listName);                               \
    bool type##LessThan(type left, type right) {                        \
        return left < right;                                            \
    }                                                                   \
    bool type##GreaterThan(type left, type right) {                     \
        return left > right;                                            \
    }
#define setDefine(type) setDefineName(type, type##List)
#define setDefineCompare(type) setDefineCompareName(type, type##List)
#define setDefineDefault(type) setDefineDefaultName(type, type##List)

#define reset(obj, type) memset(obj, 0, sizeof(type))
#define new(type) ((type*)reset(malloc(sizeof(type)), type))
#define as(type, obj) ((type*)obj)
#define max(type) ((1 << (sizeof(type) * 8)) - 1)

#define init(h) h##Init()//just to make it stand out

listDeclareDefault(u);
listDeclareVaList(u);
listDeclareDefaultName(char, string);
listDeclareVaListName(char, string);
typedef string charList;

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
string absolutePath(string path);

#endif //SHORTS_H
