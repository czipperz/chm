#include "chm.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct HashMap {
    HASH_SET_BASE;
    char* keys;
    char* values;
};

static int is_enabled(unsigned char* flags, size_t index) {
    unsigned char byte = flags[index / CHAR_BIT];
    size_t bit = (index & (CHAR_BIT - 1));
    return (byte >> bit) & 1;
}

static void set_enabled(unsigned char* flags, size_t index) {
    unsigned char* byte = &flags[index / CHAR_BIT];
    size_t bit = (index & (CHAR_BIT - 1));
    *byte |= (unsigned char)(1 << bit);
}

static void set_disabled(unsigned char* flags, size_t index) {
    unsigned char* byte = &flags[index / CHAR_BIT];
    size_t bit = (index & (CHAR_BIT - 1));
    *byte &= ~(unsigned char)(1 << bit);
}

static uint64_t next_power_of_two(uint64_t x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

int hash_map_is_present(const void* _hash_map, uint64_t index) {
    const struct HashMap* hash_map = (struct HashMap*) _hash_map;
    return is_enabled(hash_map->_flags, index * 2);
}

static void hash_map_set_present(const void* _hash_map, uint64_t index) {
    const struct HashMap* hash_map = (struct HashMap*) _hash_map;
    set_enabled(hash_map->_flags, index * 2);
    set_disabled(hash_map->_flags, index * 2 + 1);
}

static int hash_map_is_tombstone(const void* _hash_map, uint64_t index) {
    const struct HashMap* hash_map = (struct HashMap*) _hash_map;
    return is_enabled(hash_map->_flags, index * 2 + 1);
}

static void hash_map_set_tombstone(const void* _hash_map, uint64_t index) {
    const struct HashMap* hash_map = (struct HashMap*) _hash_map;
    set_disabled(hash_map->_flags, index * 2);
    set_enabled(hash_map->_flags, index * 2 + 1);
}

void hash_map_drop(void* _hash_map) {
    struct HashMap* hash_map = (struct HashMap*) _hash_map;
    free(hash_map->_flags);
    free(hash_map->keys);
    free(hash_map->values);
};

void hash_map_reserve(void* _hash_map, size_t key_size, size_t value_size, uint64_t extra, uint64_t (*hashf)(const void*)) {
    struct HashMap* hash_map = (struct HashMap*) _hash_map;
    if (hash_map->count + extra > hash_map->cap / 2) {
        struct HashMap new_hash_map;
        new_hash_map.cap = next_power_of_two((hash_map->count + extra) * 2 - 1);
        new_hash_map.count = 0;
        new_hash_map._flags = (unsigned char*) calloc((new_hash_map.cap * 2 + CHAR_BIT - 1) / CHAR_BIT, 1);
        new_hash_map.keys = (char*) malloc(new_hash_map.cap * key_size);
        new_hash_map.values = (char*) malloc(new_hash_map.cap * value_size);

        for (uint64_t i = 0; i < hash_map->cap; ++i) {
            if (hash_map_is_present(hash_map, i)) {
                hash_map_insert(&new_hash_map, key_size, value_size, &hash_map->keys[i * key_size], hashf(&hash_map->keys[i * key_size]), &hash_map->values[i * value_size]);
            }
        }

        hash_map_drop(hash_map);
        *hash_map = new_hash_map;
    }
}

void* hash_map_get(const void* _hash_map, size_t key_size, size_t value_size, const void* key, uint64_t hash, int (*comparef)(const void*, const void*)) {
    const struct HashMap* hash_map = (const struct HashMap*) _hash_map;
    if (hash_map->cap == 0) {
        return 0;
    }

    uint64_t index = hash & (hash_map->cap - 1);
    for (;; index = ((index + 1) & (hash_map->cap - 1))) {
        if (hash_map_is_present(hash_map, index) && comparef(&hash_map->keys[index * key_size], key) == 0) {
            return &hash_map->values[index * value_size];
        } else if (hash_map_is_present(hash_map, index) || hash_map_is_tombstone(hash_map, index)) {
            continue;
        } else {
            return 0;
        }
    }
}

void hash_map_insert(void* _hash_map, size_t key_size, size_t value_size, const void* key, uint64_t hash, const void* value) {
    struct HashMap* hash_map = (struct HashMap*) _hash_map;
    uint64_t index = hash & (hash_map->cap - 1);
    for (;; index = ((index + 1) & (hash_map->cap - 1))) {
        if (!hash_map_is_present(hash_map, index) || hash_map_is_tombstone(hash_map, index)) {
            memcpy(&hash_map->keys[index * key_size], key, key_size);
            memcpy(&hash_map->values[index * value_size], value, value_size);
            hash_map_set_present(hash_map, index);
            ++hash_map->count;
            return;
        }
    }
}

int hash_map_remove(void* _hash_map, size_t key_size, size_t value_size, const void* key, uint64_t hash, void* value_out, int (*comparef)(const void*, const void*)) {
    struct HashMap* hash_map = (struct HashMap*) _hash_map;
    if (hash_map->cap == 0) {
        return 0;
    }

    uint64_t index = hash & (hash_map->cap - 1);
    while (1) {
        if (!hash_map_is_present(hash_map, index) && !hash_map_is_tombstone(hash_map, index)) {
            return 0;
        }

        if (hash_map_is_present(hash_map, index) && comparef(&hash_map->keys[index * key_size], key) == 0) {
            memcpy(value_out, &hash_map->values[index * value_size], value_size);
            hash_map_set_tombstone(hash_map, index);
            --hash_map->count;
            return 1;
        }

        index = ((index + 1) & (hash_map->cap - 1));
    }
}
