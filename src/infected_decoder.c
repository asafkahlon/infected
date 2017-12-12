#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "infected_decoder.h"

static char infected_barker[2] = {0xca, 0xfe};

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a <= _b ? _a : _b; })

enum infected_decoder_state {
	BARKER,
	SIZE,
	CONTENT_CRC,
};

struct infected_decoder {
	size_t				size;
	size_t				read_next;
	enum infected_decoder_state	state;
	char				*buf;
	char				*read_head;
	char				*write_head;
	infected_decoder_valid_frame_cb	on_frame;
	infected_decoder_error_cb	on_error;
	struct infected_frame frame;
};

static inline void __decoder_reset_state(struct infected_decoder *decoder)
{
	decoder->state = BARKER;
	decoder->read_next = sizeof(infected_barker);
	decoder->frame.content = NULL;
	decoder->frame.size = 0;
}

static inline size_t __decoder_available_bytes(struct infected_decoder *decoder)
{
	return (size_t)(decoder->write_head - decoder->read_head);
}

static size_t __decoder_write(struct infected_decoder *decoder,
		const char *buf, size_t size)
{
	size_t to_add = min(size, infected_decoder_free_space(decoder));

	memcpy(decoder->write_head, buf, to_add);
	decoder->write_head += to_add;
	return to_add;
}

static inline void __on_error(struct infected_decoder *decoder,
		enum infected_decoder_error error)
{
	if (decoder->on_error)
		decoder->on_error(decoder, error);
}

static void __decoder_find_barker(struct infected_decoder *decoder)
{
	char *found = NULL, *last = decoder->write_head - 1;

	while (__decoder_available_bytes(decoder) >= sizeof(infected_barker)) {
		if (decoder->read_head[0] == infected_barker[0] &&
				decoder->read_head[1] == infected_barker[1]) {
			found = decoder->read_head;
			break;
		} else {
			decoder->read_head +=
				(decoder->read_head[1] == infected_barker[0]) ?
				1 : sizeof(infected_barker);
		}
	}

	if (found != NULL) {
		printf("Barker found\n");
		if (found > decoder->buf) {
			/* Align the barker on the buffer start */
			infected_decoder_reset(decoder);
			__decoder_write(decoder, found, last - found + 1);
		}
		decoder->read_head += sizeof(infected_barker);
		decoder->state = SIZE;
		decoder->read_next = 3; /* Size(2) + HEC(1) */
		return;
	}

	/**
	 * if the last available byte is the first byte of the barker, we copy
	 * it as the beginning of the buffer and wait for the next batch of
	 * data.
	 */
	if (*last == infected_barker[0]) {
		infected_decoder_reset(decoder);
		__decoder_write(decoder, last, sizeof(char));
		decoder->read_next = 1; /* we are missing only one byte */
		return;
	}

	decoder->read_next = sizeof(infected_barker);
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
		printf("Size corrupted beyond repair\n");
		/**
		 * Reset the state, but don't discard any data. As long as the
		 * size is not verified, we are still looking for a frame.
		 * Maybe the size + hec bytes actually contains the barker of a
		 * valid frame.
		 */
		__decoder_reset_state(decoder);
		return;
	}
	decoder->read_head += (sizeof(size) + sizeof(hec));
	printf("size=%hd, HEC=%hd\n", size, hec);
	if (size < INFECTED_MIN_FRAME_SIZE) {
		__decoder_reset_state(decoder);
		__on_error(decoder, INVALID_SIZE);
		return;
	}

	decoder->frame.content = (struct infected_content *)decoder->read_head;
	/* Adjust the frame size so it will reflect only the payload size */
	decoder->frame.size = size - sizeof(uint16_t) - sizeof(struct infected_content);
	decoder->state = CONTENT_CRC;
	decoder->read_next = size;
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
	decoder->read_head += sizeof(struct infected_content) + decoder->frame.size;
	crc = htons(*(uint16_t *)(decoder->read_head));
	decoder->read_head += sizeof(crc);

	if (__verify_crc(decoder->frame.content, decoder->frame.size, crc)) {
		printf("CRC Error\n");
		__on_error(decoder, CRC_ERROR);
		goto out;
	}

	printf("valid frame found\n");
	if (decoder->on_frame) {
		decoder->on_frame(decoder, decoder->frame);
	}

out:
	__decoder_reset_state(decoder);
}

void __decoder_decode(struct infected_decoder *decoder)
{
	while (__decoder_available_bytes(decoder) >= decoder->read_next) {
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
}

inline size_t infected_decoder_read_next(struct infected_decoder *decoder)
{
	return decoder->read_next;
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
	assert(size > INFECTED_MIN_FRAME_SIZE);
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

	printf("wrote %ld bytes to the decoder\n", ret);
	__decoder_decode(decoder);
	return ret;
}

inline size_t infected_decoder_size()
{
	return sizeof(struct infected_decoder);
}