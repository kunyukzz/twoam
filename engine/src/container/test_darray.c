#include "test_darray.h"
#include "core/fmt.h"
#include "darray.h"

#include <stdlib.h>

#define TEST_START(name) pfmt("Testing %s...\n", name)
#define TEST_PASS() pfmt("PASS\n")

static int *make_base_array(void)
{
    int *arr = da_create(int);
    da_push(arr, 42);
    for (int i = 1; i <= 9; ++i) da_push(arr, i * 10);
    return arr;
}

void dynamic_array_test(void)
{
    pfmt("\n");

    TEST_START("1. da_create and basic properties");
    int *arr = da_create(int);

    AM2_ASSERT(arr != NULL);
    AM2_ASSERT(da_capacity(arr) == DA_DEFAULT_CAPACITY);
    AM2_ASSERT(da_length(arr) == 0);
    AM2_ASSERT(da_stride(arr) == sizeof(int));

    da_destroy(arr);
    TEST_PASS();

    // #####################################################

    TEST_START("2. da_push single element");
    int *arr2 = da_create(int);

    da_push(arr2, 42);

    AM2_ASSERT(da_length(arr2) == 1);
    AM2_ASSERT(arr2[0] == 42);

    da_destroy(arr2);
    TEST_PASS();

    // #####################################################

    TEST_START("3. da_push multiple elements with resize");
    int *arr3 = da_create(int);

    for (int i = 1; i <= 10; ++i) da_push(arr3, i * 10);

    AM2_ASSERT(da_length(arr3) == 10);
    for (int i = 0; i < 10; ++i) AM2_ASSERT(arr3[i] == (i + 1) * 10);

    da_destroy(arr3);
    TEST_PASS();

    // #####################################################

    TEST_START("4. da_pop");
    int *arr4 = make_base_array(); // 42,10..90
    int popped;

    da_pop(arr4, &popped);

    AM2_ASSERT(popped == 90);
    AM2_ASSERT(da_length(arr4) == 9);

    da_destroy(arr4);
    TEST_PASS();

    // #####################################################

    TEST_START("5. da_insert_at middle");
    int *arr5 = make_base_array(); // len = 10

    da_insert_at(arr5, 5, 999);

    AM2_ASSERT(da_length(arr5) == 11);
    AM2_ASSERT(arr5[5] == 999);
    AM2_ASSERT(arr5[6] == 50);
    AM2_ASSERT(arr5[10] == 90);

    da_destroy(arr5);
    TEST_PASS();

    // #####################################################

    TEST_START("6. da_insert_at beginning");
    int *arr6 = make_base_array();

    da_insert_at(arr6, 0, -1);

    AM2_ASSERT(da_length(arr6) == 11);
    AM2_ASSERT(arr6[0] == -1);
    AM2_ASSERT(arr6[1] == 42);
    AM2_ASSERT(arr6[2] == 10);

    da_destroy(arr6);
    TEST_PASS();

    // #####################################################

    TEST_START("7. da_insert_at last position");
    int *arr7 = make_base_array();

    da_insert_at(arr7, da_length(arr7), 1000);

    AM2_ASSERT(da_length(arr7) == 11);
    AM2_ASSERT(arr7[9] == 90);
    AM2_ASSERT(arr7[10] == 1000);

    da_destroy(arr7);
    TEST_PASS();

    // #####################################################

    TEST_START("8. da_insert_at last position");
    int *arr8 = make_base_array();

    da_insert_at(arr8, da_length(arr8), 1000);

    AM2_ASSERT(da_length(arr8) == 11);
    AM2_ASSERT(arr8[10] == 1000);

    da_destroy(arr8);
    TEST_PASS();

    // #####################################################

    TEST_START("9. da_pop_at middle");
    int *arr9 = make_base_array();
    int popped2;

    da_pop_at(arr9, 5, &popped2);

    AM2_ASSERT(popped2 == 50);
    AM2_ASSERT(da_length(arr9) == 9);
    AM2_ASSERT(arr9[5] == 60);

    da_destroy(arr9);
    TEST_PASS();

    // #####################################################

    TEST_START("da fuzz test");
    int *arr10 = da_create(int);

    for (int i = 0; i < 10000; ++i)
    {
        int op = rand() % 3;
        u64 len = da_length(arr10);

        if (op == 0)
        {
            da_push(arr10, i);
        }
        else if (op == 1 && len > 0)
        {
            da_pop(arr10, NULL);
        }
        else
        {
            da_insert_at(arr10, (u64)rand() % (len + 1), i);
        }

        AM2_ASSERT(da_length(arr10) <= da_capacity(arr10));
    }

    da_destroy(arr10);
    TEST_PASS();

    pfmt("\n");
}
