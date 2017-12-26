#include <CUnit/CUnit.h>

#include "debug.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"


static enum infected_decoder_error s_error;

static void on_error(struct infected_decoder *decoder, enum infected_decoder_error error)
{
	debug_print("on error called with error: %d\n", error);
	s_error = error;
}

static void sanity(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	char data[] = {0xca, 0xfe, 0x0, 0x7, 0x0};

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);
	infected_decoder_write(&d, data, sizeof(data));
	CU_ASSERT_EQUAL(d.state, CONTENT_CRC);
	CU_ASSERT_EQUAL(infected_decoder_read_next(&d), 7);
}

static void test_invalid_size(uint16_t size)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	const uint8_t hec = 0;

	s_error = NO_ERROR;
	size = htons(size);
	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, on_error);
	infected_decoder_write(&d, infected_barker, sizeof(infected_barker));
	infected_decoder_write(&d, (char *)&size, sizeof(size));
	infected_decoder_write(&d, (char *)&hec, sizeof(hec));
	CU_ASSERT_EQUAL(d.state, BARKER);
	CU_ASSERT_EQUAL(s_error, INVALID_SIZE);
	s_error = NO_ERROR;
}

static void size_too_big(void)
{
	test_invalid_size(INFECTED_MIN_FRAME_SIZE - INFECTED_FRAME_HEADER_SIZE + 1);
}

static void size_too_small(void)
{
	test_invalid_size(INFECTED_MIN_FRAME_SIZE - INFECTED_FRAME_HEADER_SIZE - 1);
}

static void corrupted_size_unfixable(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];
	const uint8_t hec = 1;
	const uint16_t size = htons(INFECTED_MIN_FRAME_SIZE - INFECTED_FRAME_HEADER_SIZE);

	s_error = NO_ERROR;
	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, on_error);
	infected_decoder_write(&d, infected_barker, sizeof(infected_barker));
	infected_decoder_write(&d, (char *)&size, sizeof(size));
	infected_decoder_write(&d, (char *)&hec, sizeof(hec));
	CU_ASSERT_EQUAL(d.state, BARKER);
	CU_ASSERT_EQUAL(s_error, HEC_ERROR);
	s_error = NO_ERROR;
}

const CU_TestInfo size_tests[] = {
	{"Sanity", sanity},
	{"Size too big", size_too_big},
	{"Size too small", size_too_small},
	{"Corrupted size unfixable", corrupted_size_unfixable},
	/* {"Corrupted size fixable", corrupted_size_fixable}, */
	CU_TEST_INFO_NULL
};
