#include <CUnit/CUnit.h>

#include "log.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"

/**
 * Test that when writing more data than the decoder's buffer can hold, no
 * buffer overflow ocurres.
 */
void test_write_overflow(void)
{
	struct infected_decoder d;
	char buf[] = {
		0xca, 0xfe, 0x0, 0x7,
		0x0, 0xff, 0xff, 0xaa,
		0xaa, 0x0, 0x11, 0x22,
		0x33, 0x44, 0x0, 0x0
	}; /* A valid frame */
	size_t ret;

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, NULL);
	ret = infected_decoder_write(&d, buf, sizeof(buf));
	CU_ASSERT_EQUAL(ret, INFECTED_MIN_FRAME_SIZE);
	CU_ASSERT_EQUAL(infected_decoder_free_space(&d), 0);
}

void test_free_space(void)
{
	struct infected_decoder d;
	char buf[] = {
		0xca, 0xfe, 0x0, 0x7,
		0x0, 0xff, 0xff, 0xaa,
		0xaa, 0x0, 0x0, 0x0
	}; /* A valid, empty frame */
	unsigned long i;

	infected_decoder_init(&d, buf, sizeof(buf), NULL, NULL);
	CU_ASSERT_EQUAL(infected_decoder_free_space(&d), sizeof(buf));
	for (i = 0; i < sizeof(buf); i++) {
		infected_decoder_reset(&d);
		infected_decoder_write(&d, buf, i);
		print_debug("free space: %zu\n\n", infected_decoder_free_space(&d));
		CU_ASSERT_EQUAL(infected_decoder_free_space(&d), sizeof(buf) - i);
		CU_ASSERT_PTR_EQUAL(d.write_head, d.buf + i);
	}
}

void test_decoder_size(void)
{
	CU_ASSERT_EQUAL(
		infected_decoder_size(),
		sizeof(struct infected_decoder)
	);
}

const CU_TestInfo general_tests[] = {
	{"decoder struct size", test_decoder_size},
	{"free space", test_free_space},
	{"write overflow", test_write_overflow},
	CU_TEST_INFO_NULL
};
