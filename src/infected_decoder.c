#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <arpa/inet.h>

#include "log.h"
#include "infected_decoder.h"
#include "infected_decoder_private.h"

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a <= _b ? _a : _b; })

char infected_barker[2] = {0xca, 0xfe};

static inline void __decoder_reset_state(struct infected_decoder *decoder)
{
	decoder->state = BARKER;
	decoder->required_bytes = sizeof(infected_barker);
	infected_frame_reset(&decoder->frame);
}

static inline void __decoder_consume(struct infected_decoder *decoder, size_t size)
{
	print_debug("want to consume %zu bytes, available %zu\n",
		size, __decoder_available_bytes(decoder));
	assert(size <= __decoder_available_bytes(decoder));
	decoder->read_head += size;
}

static size_t __decoder_write(struct infected_decoder *decoder,
		const char *buf, size_t size)
{
	size_t to_add = min(size, infected_decoder_free_space(decoder));

	memcpy(decoder->write_head, buf, to_add);
	decoder->write_head += to_add;
	print_debug("added %zu/%zu bytes", to_add, size);
	return to_add;
}

static inline void __on_error(struct infected_decoder *decoder,
		enum infected_decoder_error error)
{
	if (decoder->on_error)
		decoder->on_error(decoder, error);
}

static inline void __decoder_realign(struct infected_decoder *decoder)
{
	char *p = decoder->read_head;
	size_t size = decoder->write_head - p;

	if (p == decoder->buf || !size)
		return;
	infected_decoder_reset(decoder);
	__decoder_write(decoder, p, size);
}

static void __decoder_find_barker(struct infected_decoder *decoder)
{
	int missing;

	print_debug("read head: 0x%hhx%hhx\n", decoder->read_head[0], decoder->read_head[1]);
	if (decoder->read_head[0] == infected_barker[0] &&
			decoder->read_head[1] == infected_barker[1]) {
		print_debug("%s\n", "Barker found");
		__decoder_realign(decoder);
		__decoder_consume(decoder, sizeof(infected_barker));
		decoder->state = SIZE;
		decoder->required_bytes = 3; /* Size(2) + HEC(1) */
		return;
	}

	print_debug("%s\n", "Barker not found");
	missing = (decoder->read_head[1] == infected_barker[0]) ? 1 : sizeof(infected_barker);
	__decoder_consume(decoder, missing);

	if (__decoder_available_bytes(decoder) < decoder->required_bytes) {
		__decoder_realign(decoder);
	}
}

static int __decoder_verify_size(struct infected_decoder *decoder, uint16_t *size, uint8_t hec)
{
	return hec == 0 ? 0 : 1; /* TODO: Implement HEC calculation */
}

static void __decoder_size_state(struct infected_decoder *decoder)
{
	uint16_t size;
	uint8_t hec;

	size = htons(*(uint16_t *)(decoder->read_head));
	hec = *(uint8_t *)(decoder->read_head + sizeof(uint16_t));

	if (__decoder_verify_size(decoder, &size, hec)) {
		print_error("%s\n", "Size corrupted beyond repair");
		/**
		 * Reset the state, but don't discard any data. As long as the
		 * size is not verified, we are still looking for a frame.
		 * Maybe the size + hec bytes actually contains the barker of a
		 * valid frame.
		 */
		__decoder_reset_state(decoder);
		__on_error(decoder, HEC_ERROR);
		return;
	}
	__decoder_consume(decoder, sizeof(size) + sizeof(hec));
	print_debug("size=%hd, HEC=%hhd\n", size, hec);

	if (size < INFECTED_CONTENT_HEADER_SIZE + INFECTED_CRC_SIZE) {
		/* Incomplete frame */
		print_error("incomplete frame, size=%hd\n", size);
		__decoder_reset_state(decoder);
		__on_error(decoder, INVALID_SIZE);
		return;
	}

	if (size > infected_decoder_free_space(decoder) +
			__decoder_available_bytes(decoder)) {
		/* Frame cannot fit in the given buffer */
		print_error("frame too big, size=%hd\n", size);
		__decoder_reset_state(decoder);
		__on_error(decoder, INVALID_SIZE);
		return;
	}

	decoder->frame.content = (struct infected_content *)decoder->read_head;
	/* Adjust the frame size so it will reflect only the payload size */
	decoder->frame.size = size - INFECTED_CRC_SIZE - INFECTED_CONTENT_HEADER_SIZE;
	decoder->state = CONTENT_CRC;
	decoder->required_bytes = size;
}

static int __verify_crc(const void *buf, size_t size, uint16_t crc)
{
	/* TODO: Implement actual CRC check */
	return crc == 0 ? 0 : 1;
}

static void __decoder_read_content(struct infected_decoder *decoder)
{
	uint16_t crc;

	/* When this function is invoked, all the data is already available */
	__decoder_consume(decoder, sizeof(struct infected_content) + decoder->frame.size);
	crc = htons(*(uint16_t *)(decoder->read_head));
	__decoder_consume(decoder, sizeof(crc));

	if (__verify_crc(decoder->frame.content, decoder->frame.size, crc)) {
		print_error("CRC Error. Received 0x%hx Expected 0x%hx\n", crc, 0);
		__on_error(decoder, CRC_ERROR);
		goto out;
	}

	print_debug("%s\n", "valid frame found");
	if (decoder->on_frame) {
		decoder->on_frame(decoder, decoder->frame);
	}

out:
	__decoder_reset_state(decoder);
}

void __decoder_decode(struct infected_decoder *decoder)
{
	while (__decoder_available_bytes(decoder) >= decoder->required_bytes) {
		print_debug("available data: %zu bytes\n", __decoder_available_bytes(decoder));
		print_debug("required: %zu bytes\n", decoder->required_bytes);
		switch (decoder->state) {
		case BARKER:
			__decoder_find_barker(decoder);
			break;
		case SIZE:
			__decoder_size_state(decoder);
			break;
		case CONTENT_CRC:
			__decoder_read_content(decoder);
			break;
		default:
			/* WTF */
			break;
		}
	}

	if (decoder->state == BARKER && decoder->read_head != decoder->buf) {

	}
}

inline size_t infected_decoder_read_next(struct infected_decoder *decoder)
{
	return decoder->required_bytes - __decoder_available_bytes(decoder);
}

inline char * infected_decoder_write_head(struct infected_decoder *decoder)
{
	return decoder->write_head;
}

inline size_t infected_decoder_free_space(struct infected_decoder *decoder)
{
	return (size_t)((decoder->buf + decoder->size) - decoder->write_head);
}

int infected_decoder_init(
		struct infected_decoder *decoder,
		char *buf, size_t size,
		infected_decoder_valid_frame_cb on_frame,
		infected_decoder_error_cb on_error)
{
	infected_decoder_set_buffer(decoder, buf, size);
	decoder->on_frame = on_frame;
	decoder->on_error = on_error;
	return 0;
}

void infected_decoder_set_buffer(
		struct infected_decoder *decoder,
		char *buf, size_t size)
{
	assert(size >= INFECTED_MIN_FRAME_SIZE);
	decoder->buf = buf;
	decoder->size = size;
	infected_decoder_reset(decoder);
}

void infected_decoder_reset(struct infected_decoder *decoder)
{
	decoder->read_head = decoder->buf;
	decoder->write_head = decoder->buf;
	__decoder_reset_state(decoder);
}

void infected_decoder_mark_write(struct infected_decoder *decoder, size_t size)
{
	assert(size <= infected_decoder_free_space(decoder));
	decoder->write_head += size;

	__decoder_decode(decoder);
}

size_t infected_decoder_write(struct infected_decoder *decoder,
		const char *buf, size_t size)
{
	size_t ret = __decoder_write(decoder, buf, size);

	__decoder_decode(decoder);
	return ret;
}

inline size_t infected_decoder_size()
{
	return sizeof(struct infected_decoder);
}