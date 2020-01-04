#include "chm.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define ASSERT(x)                                                              \
    do {                                                                       \
        if (!(x)) {                                                            \
            fprintf(stderr, "%s:%d: Assertion failed: %s\n", __FILE__,         \
                    __LINE__, STRINGIFY(x));                                   \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_CASE_NAME(name) hash_map_test_##name
#define TEST_CASE(name) static int TEST_CASE_NAME(name)(void)
#define RUN_TEST_CASE(name) ret += TEST_CASE_NAME(name)()

struct IntToIntMap {
    HASH_SET_BASE;
    int* keys;
    int* values;
};

static uint64_t hash_int(const void* _v) {
    const int* v = (const int*) _v;
    return *v;
}

static int compare_int(const void* _left, const void* _right) {
    const int* left = (const int*) _left;
    const int* right = (const int*) _right;
    return *left - *right;
}

TEST_CASE(empty_reserve) {
    struct IntToIntMap map = {};
    HASH_MAP_RESERVE(&map, 1, hash_int);
    ASSERT(map.cap == 2);
    ASSERT(map.count == 0);
    ASSERT(map._flags);
    ASSERT(map.keys);
    ASSERT(map.values);
    hash_map_drop(&map);
    return 0;
}

TEST_CASE(empty_get_fails) {
    struct IntToIntMap map = {};
    int key = 1;
    int* pvalue;

    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue == 0);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(empty_after_reserve_get_fails) {
    struct IntToIntMap map = {};
    int key = 1;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 1, hash_int);
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue == 0);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(empty_remove) {
    struct IntToIntMap map = {};
    int key = 1;
    int value;
    int removed;

    key = -4;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(!removed);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(get_after_insert) {
    struct IntToIntMap map = {};
    int key = 1;
    int value = 3;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 2, hash_int);
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 3);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(get_after_insert_2) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 2, hash_int);

    key = 1;
    value = 3;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    value = 7;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 7);

    key = 1;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 3);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(get_after_insert_2_reverse) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 2, hash_int);

    key = 2;
    value = 7;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 1;
    value = 3;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 7);

    key = 1;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 3);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(insert_collision) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 2, hash_int);
    ASSERT(map.cap == 4);

    key = 1;
    value = 3;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 5;
    value = 7;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 5;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 7);

    key = 1;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 3);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(insert_reserve_insert) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int* pvalue;

    HASH_MAP_RESERVE(&map, 2, hash_int);
    ASSERT(map.cap == 4);

    key = 1;
    value = 13;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    value = -77;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    HASH_MAP_RESERVE(&map, 2, hash_int);
    ASSERT(map.cap == 8);

    key = 42;
    value = 1239;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = -4;
    value = 9062;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 1;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 13);

    key = 2;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == -77);

    key = 42;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 1239);

    key = -4;
    pvalue = (int*) HASH_MAP_GET(&map, &key, hash_int(&key), compare_int);
    ASSERT(pvalue != 0);
    ASSERT(*pvalue == 9062);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(remove_all) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int removed;

    HASH_MAP_RESERVE(&map, 4, hash_int);

    key = 1;
    value = 13;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    value = -77;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 42;
    value = 1239;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = -4;
    value = 9062;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 1;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 13);

    key = 2;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == -77);

    key = 42;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 1239);

    key = -4;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 9062);

    ASSERT(map.count == 0);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(remove_all_intersperse_nonexistant) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int removed;

    HASH_MAP_RESERVE(&map, 4, hash_int);

    key = 1;
    value = 13;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    value = -77;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 42;
    value = 1239;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = -4;
    value = 9062;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 1;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 13);

    key = 182481;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(!removed);

    key = 2;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == -77);

    key = 42;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 1239);

    key = -4;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 9062);

    ASSERT(map.count == 0);

    hash_map_drop(&map);
    return 0;
}

TEST_CASE(remove_nonexistant_then_all) {
    struct IntToIntMap map = {};
    int key;
    int value;
    int removed;

    HASH_MAP_RESERVE(&map, 4, hash_int);

    key = 1;
    value = 13;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 2;
    value = -77;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 42;
    value = 1239;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = -4;
    value = 9062;
    HASH_MAP_INSERT(&map, &key, hash_int(&key), &value);

    key = 182481;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(!removed);

    key = 1;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 13);

    key = 2;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == -77);

    key = 42;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 1239);

    key = -4;
    removed = HASH_MAP_REMOVE(&map, &key, hash_int(&key), &value, compare_int);
    ASSERT(removed);
    ASSERT(value == 9062);

    ASSERT(map.count == 0);

    hash_map_drop(&map);
    return 0;
}

int main() {
    int ret = 0;

    RUN_TEST_CASE(empty_reserve);
    RUN_TEST_CASE(empty_get_fails);
    RUN_TEST_CASE(empty_after_reserve_get_fails);
    RUN_TEST_CASE(empty_remove);
    RUN_TEST_CASE(get_after_insert);
    RUN_TEST_CASE(get_after_insert_2);
    RUN_TEST_CASE(get_after_insert_2_reverse);
    RUN_TEST_CASE(insert_collision);
    RUN_TEST_CASE(insert_reserve_insert);
    RUN_TEST_CASE(remove_all);
    RUN_TEST_CASE(remove_all_intersperse_nonexistant);
    RUN_TEST_CASE(remove_nonexistant_then_all);

    if (ret == 0) {
        printf("Success\n");
    } else {
        printf("Errors %d\n", ret);
    }
    return ret;
}

#ifdef __cplusplus
}
#endif
