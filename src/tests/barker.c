#include <CUnit/CUnit.h>

#include "log.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"

static enum infected_decoder_error s_error;
static void on_error(struct infected_decoder *decoder, enum infected_decoder_error error)
{
	print_debug("on error called with error: %d\n", error);
	s_error = error;
}

static inline void assert_barker_found(struct infected_decoder decoder)
{
	CU_ASSERT_EQUAL(decoder.state, SIZE);
	CU_ASSERT_EQUAL(decoder.required_bytes, 3);
	CU_ASSERT_NSTRING_EQUAL(
		decoder.buf,
		infected_barker,
		sizeof(infected_barker)
	);
}

static void test_barker_valid(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	unsigned int i;

	char case_a[] = {0xca, 0xfe};
	char case_b[] = {0xca, 0xca, 0xfe};
	char case_c[] = {0xfe, 0xca, 0xca, 0xca, 0xfe};
	char *test_data[] = {case_a, case_b, case_c};
	/* Ugh, figure out a better way to do this */
	int sizes[] = {sizeof case_a, sizeof case_b, sizeof case_c};

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		infected_decoder_reset(&d);
		infected_decoder_write(&d, test_data[i], sizes[i]);
		assert_barker_found(d);
	}
}

static void test_no_barker(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	char no_start[] = {0xfe, 0xfe, 0xfe, 0xfe, 0xfe};
	unsigned long i;

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);

	for (i = 0; i < sizeof(no_start); i++) {
		infected_decoder_reset(&d);
		infected_decoder_write(&d, no_start, i);
		CU_ASSERT_EQUAL(d.state, BARKER);
	}
}

static void test_barker_across_writes(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	char data[] = {0xfe, 0xfe, 0xfe, 0xca, 0xfe};
	char *last = &data[sizeof(data) - 1];

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);
	/* write all but the last byte */
	infected_decoder_write(&d, data, sizeof(data) - 1);
	CU_ASSERT_EQUAL(d.state, BARKER);
	CU_ASSERT_EQUAL(infected_decoder_read_next(&d), 1);
	infected_decoder_write(&d, last, 1);
	assert_barker_found(d);
}

static void test_double_barker(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	char data[] = {0xca, 0xfe, 0xca, 0xfe, 0xaa};
	char *last = &data[sizeof(data) - 1];

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, on_error);
	s_error = NO_ERROR;
	infected_decoder_write(&d, data, sizeof(data));
	CU_ASSERT_EQUAL(d.state, SIZE);
	CU_ASSERT_EQUAL(s_error, HEC_ERROR);
	CU_ASSERT_EQUAL(infected_decoder_read_next(&d), 2);
	CU_ASSERT_EQUAL(*d.read_head, *last);
}

static void test_barker_across_boundaries(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	char data[] = {
		0xfe, 0xfe, 0xfe, 0xfe,
		0xfe, 0xfe, 0xfe, 0xfe,
		0xfe, 0xfe, 0xfe, 0xca, 0xfe
	};
	char *last = &data[sizeof(data) - 1];
	size_t ret;

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);
	/* write all but the last byte */
	ret = infected_decoder_write(&d, data, sizeof(data) - 1);
	CU_ASSERT_EQUAL(ret, INFECTED_MIN_FRAME_SIZE);
	CU_ASSERT_EQUAL(d.state, BARKER);
	CU_ASSERT_EQUAL(infected_decoder_read_next(&d), 1);
	infected_decoder_write(&d, last, 1);
	assert_barker_found(d);
}

const CU_TestInfo barker_tests[] = {
	{"No Barker", test_no_barker},
	{"Valid Barker", test_barker_valid},
	{"Barker Across Writes", test_barker_across_writes},
	{"Double Barker", test_double_barker},
	{"Barker Across Buffer Boundaries", test_barker_across_boundaries},
	CU_TEST_INFO_NULL
};
