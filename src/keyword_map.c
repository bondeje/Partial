// goal here is to create a simple map of char * to int
#include <string.h>
#include "keyword_map.h"
#ifdef DEVELOPMENT
    #include <stdio.h>
#endif

/* // possibly useful in partial.c
static int cstr_citer_cmp(const char * a, const char * b_st, const char * b_end) {
    while (*a != '\0' && b_st != b_end && *a == *b_st) {
        a++;
        b_st++;
    }
    if (*a != *b_st) {
        return 1;
    }
    return 0;
}
*/

// going to try to use open addressing

unsigned long long cstr_hash(const char * key, size_t size) {
    unsigned long long hash = 5381;
    int c;
    unsigned char * str = (unsigned char *) key;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % size;
}

void KeywordMap_init(AliasMap * map, unsigned int size) {
#ifdef DEVELOPMENT
    printf("initializing KeywordMap\n");
#endif
    map->size = size;
    for (unsigned int i = 0; i < map->size; i++) {
        map->bins[i].key = NULL;
        map->bins[i].value = 0;
    }
}

// 0 for succes, -1 for failure
int KeywordMap_add(AliasMap * map, const char * key, unsigned int value) {
#ifdef DEVELOPMENT
    printf("adding keyword %s\n", key);
#endif
    unsigned long long bin = cstr_hash(key, map->size);
    while (map->bins[bin].key) {
        bin = ((bin + 1) % map->size);
    }
    map->bins[bin].key = key;
    map->bins[bin].value = value;
#ifdef DEVELOPMENT
    printf("assigned bin %llu, with (key, value) = (%s, %u)\n", bin, key, value);
#endif
    return 0;
}

unsigned int * KeywordMap_get(AliasMap * map, const char * key) {
#ifdef DEVELOPMENT
    printf("getting keyword...");
#endif
    unsigned long long bin = cstr_hash(key, map->size);
    unsigned int i = 0;
    if (!map->bins[bin].key) {
        return NULL;
    }
    while (i < map->size && strcmp(key, map->bins[bin].key)) {
        bin = ((bin + 1) % map->size);
        i++;
    }
    if (i == map->size) {
        return NULL;
    }
#ifdef DEVELOPMENT
    printf("%u\n", map->bins[bin].value);
#endif
    return &map->bins[bin].value;
}