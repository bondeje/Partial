#include <stddef.h>

#ifndef PARTIAL_MAX_NARG
#define PARTIAL_MAX_NARG 16
#endif

#ifndef PARTIAL_ALIAS_MAP_SIZE
#define PARTIAL_ALIAS_MAP_SIZE (2*PARTIAL_MAX_NARG+1)
#endif

typedef struct Item {
    const char * key;
    unsigned int value;
} Item;

typedef struct AliasMap {
    Item bins[PARTIAL_ALIAS_MAP_SIZE];
    unsigned int size;
} AliasMap;

unsigned long long cstr_hash(const char * key, size_t);

void KeywordMap_init(AliasMap * map, unsigned int size);

// 0 for succes, -1 for failure
int KeywordMap_add(AliasMap * map, const char * key, unsigned int value);

unsigned int * KeywordMap_get(AliasMap * map, const char * key);

static inline unsigned int KeywordMap_size(AliasMap * map) {
    return map->size;
}