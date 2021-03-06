#ifndef FRAMES_H
#define FRAMES_H

uint8_t bad_crc_frame[] = {
	0xca, 0xfe, 0x0, 0x7, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x0, 0x1 /* non-zero CRC */
};

uint8_t empty_frame[] = {
	0xca, 0xfe, 0x0, 0x7, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x0, 0x0 /* CRC */
};

uint8_t empty_frame_offset[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, /* filler */
	0xca, 0xfe, 0x0, 0x7, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x0, 0x0 /* CRC */
};

uint8_t non_empty_frame[] = {
	0xca, 0xfe, 0x0, 0x7, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0xaa, 0xbb, 0xcc, 0xdd, /* Payload */
	0x0, 0x0 /* CRC */
};

uint8_t two_frames[] = {
	0xca, 0xfe, 0x0, 0xb, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x11, 0x22, 0x33, 0x44, /* Payload */
	0x0, 0x0, /* CRC */
	0x0, 0x0, 0x0, 0x0, 0x0, /* filler */
	0xca, 0xfe, 0x0, 0xb, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x11, 0x22, 0x33, 0x44, /* Payload */
	0x0, 0x0 /* CRC */
};

uint8_t two_adjacent_frames[] = {
	0xca, 0xfe, 0x0, 0xb, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x11, 0x22, 0x33, 0x44, /* Payload */
	0x0, 0x0, /* CRC */
	0xca, 0xfe, 0x0, 0xb, 0x0, /* Barker, Size, HEC */
	0xaa, 0xbb, 0xcc, 0xdd, 0xe, /* DST, SRC, flags */
	0x11, 0x22, 0x33, 0x44, /* Payload */
	0x0, 0x0 /* CRC */
};
#endif /* FRAMES_H */
