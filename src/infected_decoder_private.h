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
	char				*buf;
	char				*read_head;
	char				*write_head;
	infected_decoder_valid_frame_cb	on_frame;
	infected_decoder_error_cb	on_error;
	struct infected_frame frame;
};

extern char infected_barker[2];

#endif /* INFECTED_DECODER_PRIVATE_H */
