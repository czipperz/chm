#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_SET_BASE                                                          \
    uint64_t cap;                                                              \
    uint64_t count;                                                            \
    unsigned char* _flags

void hash_map_drop(void* hash_map);

void hash_map_reserve(void* hash_map, size_t key_size, size_t value_size,
                      uint64_t extra, uint64_t (*hashf)(const void*));
#define HASH_MAP_RESERVE(hash_map, extra, hashf)                               \
    (hash_map_reserve((hash_map), sizeof(*(hash_map)->keys),                   \
                      sizeof(*(hash_map)->values), (extra), (hashf)))

int hash_map_is_present(const void* hash_map, uint64_t index);

void* hash_map_get(const void* hash_map, size_t key_size, size_t value_size,
                   const void* key, uint64_t hash,
                   int (*comparef)(const void*, const void*));
#define HASH_MAP_GET(hash_map, key, hash, comparef)                            \
    (hash_map_get((hash_map), sizeof(*(hash_map)->keys),                       \
                  sizeof(*(hash_map)->values), (key), (hash), (comparef)))

void hash_map_insert(void* hash_map, size_t key_size, size_t value_size,
                     const void* key, uint64_t hash, const void* value);
#define HASH_MAP_INSERT(hash_map, key, hash, value)                            \
    (hash_map_insert((hash_map), sizeof(*(hash_map)->keys),                    \
                     sizeof(*(hash_map)->values), (key), (hash), (value)))

int hash_map_remove(void* hash_map, size_t key_size, size_t value_size,
                    const void* key, uint64_t hash, void* value_out,
                    int (*comparef)(const void*, const void*));
#define HASH_MAP_REMOVE(hash_map, key, hash, value_out, comparef)              \
    (hash_map_remove((hash_map), sizeof(*(hash_map)->keys),                    \
                     sizeof(*(hash_map)->values), (key), (hash), (value_out),  \
                     (comparef)))

#ifdef __cplusplus
}
#endif
