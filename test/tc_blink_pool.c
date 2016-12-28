/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_pool.h"
#include <string.h>

int setup_pool(void **user)
{
    static uint8_t heap[128U];
    static struct blink_pool pool;
    *user = (blink_pool_t)BLINK_Pool_init(&pool, heap, sizeof(heap));
    return 0;
}

void test_BLINK_Pool_init(void **user)
{
    struct blink_pool pool;
    uint8_t heap[128U];
    assert_true(BLINK_Pool_init(&pool, heap, sizeof(heap)) == &pool);    
}

void test_BLINK_Pool_calloc_zeroSize(void **user)
{
    uint8_t *ptr = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), 0);

    assert_true(ptr == NULL);
}

void test_BLINK_Pool_calloc_all(void **user)
{
    uint8_t *first = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), 128U);
    uint8_t *second = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), 1U);

    assert_true(first != NULL);
    assert_true(second == NULL);
}

void test_BLINK_Pool_calloc_multi(void **user)
{
    uint8_t *first = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), sizeof(long));
    uint8_t *second = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), sizeof(char));
    uint8_t *third = (uint8_t *)BLINK_Pool_calloc((blink_pool_t)(*user), sizeof(long));

    assert_true(first != NULL);
    assert_true(((long)first % sizeof(long)) == 0);

    assert_true(second != NULL);
    assert_true(((long)second % sizeof(long)) == 0);
    assert_ptr_not_equal(first, second);

    assert_true(third != NULL);
    assert_true(((long)third % sizeof(long)) == 0);
    assert_ptr_not_equal(first, third);
    assert_ptr_not_equal(second, third);
}

void test_BLINK_Pool_getFreeSpace(void **user)
{
    size_t allSpace = BLINK_Pool_getFreeSpace((blink_pool_t)(*user));

    assert_true(allSpace > 0);

    (void)BLINK_Pool_calloc((blink_pool_t)(*user), sizeof(long));

    assert_true(BLINK_Pool_getFreeSpace((blink_pool_t)(*user)) < allSpace);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Pool_init),        
        cmocka_unit_test_setup(test_BLINK_Pool_calloc_all, setup_pool),        
        cmocka_unit_test_setup(test_BLINK_Pool_calloc_zeroSize, setup_pool),        
        cmocka_unit_test_setup(test_BLINK_Pool_calloc_multi, setup_pool),        
        cmocka_unit_test_setup(test_BLINK_Pool_getFreeSpace, setup_pool),        
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
