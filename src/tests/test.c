#include <stdio.h>
#include <errno.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include <infected_decoder.h>

#define ARRAY_SIZE(array) \
	(sizeof(array) / sizeof(array[0]))

static struct infected_decoder *decoder;
static char buf[INFECTED_MAX_FRAME_SIZE];

void on_frame(struct infected_decoder *decoder, struct infected_frame frame)
{
	printf("Frame found with size %u\n", frame.size);
	infected_decoder_reset(decoder);
}

void on_error(struct infected_decoder *decoder, enum infected_decoder_error error)
{
	printf("Error: %d\n", error);
	infected_decoder_reset(decoder);
}

static void test_barker_positive(void)
{
	unsigned int i;

	char case_a[] = {0xca, 0xfe};
	char case_b[] = {0xca, 0xca, 0xfe};
	char case_c[] = {0xfe, 0xca, 0xca, 0xca, 0xfe};
	char *test_data[] = {case_a, case_b, case_c};
	/* Ugh, figure out a better way to do this */
	int sizes[] = {sizeof case_a, sizeof case_b, sizeof case_c};

	printf("********** Barker ***********\n");
	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		infected_decoder_reset(decoder);
		infected_decoder_write(decoder, test_data[i], sizes[i]);
	}
}

static void test_data_valid(void)
{
	unsigned int i;
	char test_data[][5] = {
		{0xca, 0xfe, 0x0, 0xff, 0x0},
		{0xca, 0xfe, 0x80, 0x0, 0x0},
	};

	printf("********** Valid Size ***********\n");
	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		infected_decoder_reset(decoder);
		infected_decoder_write(decoder, test_data[i], sizeof(test_data[0]));
	}
}

static void test_data_invalid(void)
{
	unsigned int i;
	char test_data[][7] = {
		{0xca, 0xfe, 0xca, 0xfe, 0x1, 0x0, 0x0},
		{0xca, 0xfe, 0x0, 0xca, 0xfe, 0x1, 0x0},
	};

	printf("********** Invalid Size ***********\n");
	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		infected_decoder_reset(decoder);
		infected_decoder_write(decoder, test_data[i], sizeof(test_data[0]));
	}
}

int init_suite1(void)
{
	decoder = malloc(infected_decoder_size());
	if (!decoder) {
		perror("decoder allocation");
		return -ENOMEM;
	}
	infected_decoder_init(decoder, buf, sizeof(buf), on_frame, on_error);
	return 0;
}

int clean_suite1(void)
{
	free(decoder);
	return 0;
}

int main()
{
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* add the tests to the suite */
	if ((NULL == CU_add_test(pSuite, "test barker", test_barker_positive)) ||
		(NULL == CU_add_test(pSuite, "test valid size", test_data_valid)) ||
		(NULL == CU_add_test(pSuite, "test invalid size", test_data_invalid)))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}