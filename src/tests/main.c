#include <stdio.h>
#include <errno.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "tests.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"


static struct infected_decoder decoder;
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

static void test_data_valid(void)
{
	unsigned int i;
	char test_data[][5] = {
		{0xca, 0xfe, 0x0, 0xff, 0x0},
		{0xca, 0xfe, 0x80, 0x0, 0x0},
	};

	printf("********** Valid Size ***********\n");
	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		infected_decoder_reset(&decoder);
		infected_decoder_write(&decoder, test_data[i], sizeof(test_data[0]));
		CU_ASSERT_EQUAL(decoder.state, CONTENT_CRC);
	}
}

static void test_data_invalid(void)
{
	char just_bad[] = {0xca, 0xfe, 0xbe, 0xef, 0x1, 0x0, 0x0};
	char size_is_barker[] = {0xca, 0xfe, 0x0, 0xca, 0xfe, 0x1, 0x0};

	infected_decoder_reset(&decoder);
	infected_decoder_write(&decoder, just_bad, sizeof(just_bad));
	CU_ASSERT_EQUAL(decoder.state, BARKER);

	infected_decoder_reset(&decoder);
	infected_decoder_write(&decoder, size_is_barker, sizeof(size_is_barker));
	CU_ASSERT_EQUAL(decoder.state, SIZE);
}

static CU_TestInfo size_tests[] = {
	{"Valid Size", test_data_valid},
	{"Invalid Size", test_data_invalid},
	CU_TEST_INFO_NULL
};

extern CU_TestInfo barker_tests[];
extern CU_TestInfo general_tests[];
static CU_SuiteInfo suites[] = {
	{"General", NULL, NULL, NULL, NULL, general_tests},
	{"Barker", NULL, NULL, NULL, NULL, barker_tests},
	CU_SUITE_INFO_NULL,
};

int main()
{
	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		goto out;

	if (CUE_SUCCESS != CU_register_suites(suites))
		goto err_cleanup;

	infected_decoder_init(&decoder, buf, sizeof(buf), on_frame, on_error);
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

err_cleanup:
	CU_cleanup_registry();
out:
	return CU_get_error();
}