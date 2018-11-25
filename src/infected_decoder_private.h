#ifndef INFECTED_DECODER_PRIVATE_H
#define INFECTED_DECODER_PRIVATE_H
#include "infected_decoder.h"

enum infected_decoder_state {
	BARKER,
	SIZE,
	CONTENT_CRC,
};

struct infected_decoder {
	enum infected_decoder_state	state;
	size_t				size;
	size_t				required_bytes;
    uint8_t				*buf;
    uint8_t				*read_head;
    uint8_t				*write_head;
	infected_decoder_valid_frame_cb	on_frame;
	infected_decoder_error_cb	on_error;
	struct infected_frame frame;
};

extern uint8_t infected_barker[2];

static inline size_t __decoder_available_bytes(struct infected_decoder *decoder)
{
	return (size_t)(decoder->write_head - decoder->read_head);
}

#endif /* INFECTED_DECODER_PRIVATE_H */
