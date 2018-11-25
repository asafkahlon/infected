#ifndef INFECTED_DECODER_H
#define INFECTED_DECODER_H

#include <stdint.h>

#define INFECTED_BARKER_SIZE sizeof(uint16_t)
#define INFECTED_CRC_SIZE sizeof(uint16_t)
#define INFECTED_FRAME_HEADER_SIZE 5 /* Barker, Size HEC */
#define INFECTED_CONTENT_HEADER_SIZE 5 /* DST, SRC, Flags */
#define INFECTED_HEADER_SIZE (INFECTED_FRAME_HEADER_SIZE + INFECTED_CONTENT_HEADER_SIZE)

#define INFECTED_MIN_FRAME_SIZE (INFECTED_HEADER_SIZE + INFECTED_CRC_SIZE)
#define INFECTED_MAX_FRAME_SIZE 1600

#define ARRAY_SIZE(array) \
	(sizeof(array) / sizeof((array)[0]))

enum infected_decoder_error {
	NO_ERROR = 0,
	INVALID_SIZE,
	HEC_ERROR,
	CRC_ERROR,
};

struct infected_content {
	uint16_t dst;
	uint16_t src;
	uint8_t flags;
	char payload[0];
} __attribute__((__packed__));

struct infected_frame {
	struct infected_content *content;
	uint16_t size; /* The size of the payload */
};

static inline void infected_frame_reset(struct infected_frame *frame)
{
	frame->content = NULL;
	frame->size = 0;
}

/* Opaque type */
struct infected_decoder;

/**
 * Return the size of the infected_decoder struct.
 * This enables the application to allocate memory for `infected_decoder`
 * objects on its own.
 */
size_t infected_decoder_size(void);

typedef void (*infected_decoder_valid_frame_cb)
	(struct infected_decoder *decoder, struct infected_frame frame);

typedef void (*infected_decoder_error_cb)
	(struct infected_decoder *decoder, enum infected_decoder_error error);

/**
 * Initialize infected decoder object.
 * 
 * @param decoder: the decoder object.
 * @buf: A buffer that will be shared between the application and the decoder.
 * 	The application is expected to _put_ data in the buffer, while the
 * 	decoder will _scan_ the buffer in search for a frame.
 * @size: The buffer size. The size must be at least INFECTED_MIN_FRAME_SIZE.
 * @on_frame: A callback function that will be invoked when a valid frame will
 * 	be detected by the decoder.
 * @on_error: A callback that will be invoked by the decoder if some error
 * 	occure during the decode process.
 */
int infected_decoder_init(
		struct infected_decoder *decoder,
        uint8_t *buf, size_t size,
		infected_decoder_valid_frame_cb on_frame,
		infected_decoder_error_cb on_error);

/**
 * Sets a new buffer for the decoder to work on.
 * The application might call this function in order to avoid copying the
 * contents of the buffer.
 */
void infected_decoder_set_buffer(
		struct infected_decoder *decoder,
        uint8_t *buf, size_t size);

/**
 * Resets the decoder to its initial state.
 * All the existing data in the buffer is discarded.
 */
void infected_decoder_reset(struct infected_decoder *decoder);

uint8_t * infected_decoder_write_head(struct infected_decoder *decoder);

size_t infected_decoder_read_next(struct infected_decoder *decoder);

size_t infected_decoder_free_space(struct infected_decoder *decoder);

/**
 * Appends data to the decoder's buffer.
 * 
 * @param decoder: The decoder.
 * @param buf: A buffer containing the data to add to the decoder.
 * @param size: How many bytes from `buf` to add.
 * 
 * @return: How many bytes were actually added. If there is not enough space in
 * 	the decoder's buffer, not all the data will be added.
 */
size_t infected_decoder_write(struct infected_decoder *decoder, const uint8_t *buf, size_t size);

/**
 * Tells the decoder that `size` bytes have been written to its buffer.
 * Application needs to call this method if they wrote data directly to the
 * buffer using a pointer returned from `infected_decoder_write_head`.
 * 
 * @param decoder: The decoder.
 * @param size: How many bytes were added to the buffer.
 */
void infected_decoder_mark_write(struct infected_decoder *decoder, size_t size);
#endif /* INFECTED_DECODER_H */
