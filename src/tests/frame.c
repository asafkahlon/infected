#include <arpa/inet.h>

#include <CUnit/CUnit.h>

#include "log.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"
#include "frames.h"

struct frame_attrs {
	char *payload;
	size_t payload_size;
	uint16_t dst;
	uint16_t src;
	uint8_t flags;
};

static unsigned int s_frame_counter;
static struct frame_attrs s_expected_frames[10];
static enum infected_decoder_error s_expected_error = NO_ERROR;

static inline void expect_error(enum infected_decoder_error error)
{
	s_expected_error = error;
}

static inline void expect_frame(unsigned int index, uint16_t dst, uint16_t src,
		 uint8_t flags, char *payload, size_t payload_size)
{
	s_expected_frames[index].dst = dst;
	s_expected_frames[index].src = src;
	s_expected_frames[index].flags = flags;
	s_expected_frames[index].payload = payload;
	s_expected_frames[index].payload_size = payload_size;
}

static void on_error(struct infected_decoder *decoder, enum infected_decoder_error error)
{
	print_error("on error called with error: %d\n", error);
	CU_ASSERT_EQUAL(error, s_expected_error);
}

static void on_frame(struct infected_decoder *decoder, struct infected_frame frame)
{
	struct frame_attrs attrs = s_expected_frames[s_frame_counter];
	s_frame_counter++;

	print_debug("on frame called for frame with payload size: %d\n", frame.size);
	CU_ASSERT_EQUAL(frame.size, attrs.payload_size);
	CU_ASSERT_PTR_NOT_NULL(frame.content);
	CU_ASSERT_EQUAL(frame.content->dst, attrs.dst);
	CU_ASSERT_EQUAL(frame.content->src, attrs.src);
	CU_ASSERT_EQUAL(frame.content->flags, attrs.flags);
}

static void test_bad_crc(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MIN_FRAME_SIZE];

	infected_decoder_init(&d, buf, INFECTED_MIN_FRAME_SIZE, NULL, on_error);

	expect_error(CRC_ERROR);
	infected_decoder_write(&d, bad_crc_frame, sizeof(bad_crc_frame));
	CU_ASSERT_EQUAL(d.state, BARKER);
}

static void test_empty_frame(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MAX_FRAME_SIZE];

	s_frame_counter = 0;
	expect_error(NO_ERROR);
	infected_decoder_init(&d, buf, sizeof(buf), on_frame, on_error);
	expect_frame(0, htons(0xaabb), htons(0xccdd), 0xe, NULL, 0);
	expect_frame(1, htons(0xaabb), htons(0xccdd), 0xe, NULL, 0);
	infected_decoder_write(&d, empty_frame, sizeof(empty_frame));
	CU_ASSERT_EQUAL(s_frame_counter, 1);
	infected_decoder_write(&d, empty_frame_offset, sizeof(empty_frame_offset));
	CU_ASSERT_EQUAL(s_frame_counter, 2);
}

static void test_several_frames_in_buffer(void)
{
	struct infected_decoder d;
	char buf[INFECTED_MAX_FRAME_SIZE];
	char payload[] = {0x11, 0x22, 0x33, 0x44};

	expect_error(NO_ERROR);
	infected_decoder_init(&d, buf, sizeof(buf), on_frame, on_error);
	expect_frame(0, htons(0xaabb), htons(0xccdd), 0xe, payload, sizeof(payload));
	expect_frame(1, htons(0xaabb), htons(0xccdd), 0xe, payload, sizeof(payload));

	s_frame_counter = 0;
	infected_decoder_write(&d, two_frames, sizeof(two_frames));
	CU_ASSERT_EQUAL(s_frame_counter, 2);

	s_frame_counter = 0;
	infected_decoder_write(&d, two_adjacent_frames, sizeof(two_adjacent_frames));
	CU_ASSERT_EQUAL(s_frame_counter, 2);
}

const CU_TestInfo frame_tests[] = {
	{"Bad CRC", test_bad_crc},
	{"Empty frame", test_empty_frame},
	{"Several Frmaes in Buffer", test_several_frames_in_buffer},
	CU_TEST_INFO_NULL
};
