#ifndef INFECTED_SOCKET_H
#define INFECTED_SOCKET_H

#include <stdlib.h>

/*
 * INFECTED_SOCK_DGRAM: Datagram socket. Able to send and receive full datagrams
 * in a connection-less mode.
 * The only reliability feature this socket provides, is the correctness of each
 * datagram.
 */
#define INFECTED_SOCK_DGRAM	0

/*
 * INFECTED_SOCK_STREAM: Stream socket. Full-duplex, reliable stream socket.
 * Much like TCP.
 */
#define INFECTED_SOCK_STREAM	1

/*
 * INFECTED_SOCK_SEQPACKET: Packet socket. Full-duplex, reliable packet socket.
 * Preserves the frame boundaries.
 */
#define INFECTED_SOCK_SEQPACKET	2

/*
 * INFECTED_SOCK_MULTIPART: Large-transfer socket. Used for sending and
 * receiving large chunks of data (such as files) that are split across
 * multiple packets. This socket type uses some special functions.
 */
#define INFECTED_SOCK_MULTIPART	3


#define INFECTED_PROTOCOL_CUSTOM	0
#define INFECTED_PROTOCOL_MUSH		1
#define INFECTED_PROTOCOL_ETH		2
#define INFECTED_PROTOCOL_PICASSO	3

#define INFECTED_MULTIPART_START	(1 << 7)
#define INFECTED_MULTIPART_END		(1 << 6)

struct infected_sk {
	int	fd; /* The underlying fd */
	int	type;
	int	addr;
	struct {
		char	*data;
		char	*rhead;
		char	*whead;
		size_t	size;
	} buf;
};

/**
 * Opens a new Infected socket.
 * @param sock: The socket struct to be initialized. The application must
 *		allocate the space for this structure and make sure it exist as
 *		long as it is used.
 * @param type: The socket type. One of the `INFECTED_SOCK_*` constants.
 * @param proto: The protocol used for this socket.
 * 	For DGRAM socket this may be any protocol you like. It 
 * 		For SEQPACKET
 * @return: 0 on success, otherwise on failure.
 */
int infected_open(struct infected_sk *sock, int type, unit8_t proto);

/**
 * Binds a socket on a specific address and port.
 * The socket will now accept a connection on the specified address and port.
 * Notice that this function is only available for the SEQPACKET, STREAM and
 * MULTIPART socket types.
 * 
 * @param sock: The socket to bind.
 * @param addr: The Infected address for this socket (as a Big Endian, 16 bit
 * 		integer.)
 * @param port: The port to bind on.
 * 
 * @return: 0 on success, otherwise on failure.
 */
int infected_bind(struct infected_sk *sock, unit16_t addr, uint8_t port);

/**
 * Connect to another Infected socket.
 * Notice that this function is only available for the SEQPACKET, STREAM and
 * MULTIPART socket types.
 * 
 * @param sock: The socket requesting the connection.
 * @param addr: The Infected address of the other socket.
 * @param port: The port of the other socket.
 */
int infected_connect(struct infected_sk *sock, uint16_t addr, uint8_t port);

/**
 * Sends data on a socket.
 * @param sock: The socket to send data on.
 * @param buf: Pointer to the data to send.
 * @param size: The amount of bytes to send.
 *
 * @return: The number of bytes actually sent. Negative number indicates an
 * 		error.
 */
int infected_send(struct infected_sk *sock, const void *buf, size_t size);

/**
 * Sends data to an Infected host.
 * Only DGRAM sockets can be used for this function.
 * 
 * @param sock: The socket to send data on.
 * @param addr: The Infected host address to send the data to.
 * @param buf: Pointer to the data to send.
 * @param size: The amount of bytes to send.
 *
 * @return: The number of bytes actually sent. Negative number indicates an
 * 		error.
 */
int infected_sendto(struct infected_sk *sock, uint16_t addr, const void *buf,
		    size_t size);
/**
 * Sends one part of a multipart message.
 * @param sock: The socket to send data on.
 * @param buf: Pointer to the data to send.
 * @param size: The amount of bytes to send.
 * @param msg_id: Identifier of the larger message. It has no meaning other than
 *		grouping together all the parts of the same message.
 *		If you want to send multiple messages simultaneously, pick a
 *		different `msg_id` for each. Otherwise you can just pass 0 or
 *		any other number, as long as it is the same for all the parts
 *		of the message.
 * @param flasg: Bitwise OR of some INFECTED_MULTIPART_* flags. The flags are
 *		used to covey the start and the end of the entire message.
 *		The first part should have the INFECTED_MULTIPART_START bit set.
 *		The last part should have the INFECTED_MULTIPART_END bit set.
 *		Single-part messages shoult have both bits sets for that part.
 *
 * @return: The number of bytes actually sent.
 */
int infected_send_multipart(struct infected_sk *sock, const void *buf,
			    size_t size, uint16_t msg_id, uint8_t flags);

/**
 * Receive data from a socket.
 * @param sock: The socket to receive data from.
 * @param buf: A Pointer to hold the data. Must have a length of at least
 *		`size` bytes.
 * @param size: The maximum amount of bytes to read. Note that depending on the
 *		socket type, this parameter has a slightly different semantics:
 *		For steam socket, you'll get up to `size` bytes from the
 *		available data.
 *		For datagram/packet socket, you'll get up to `size` bytes from
 *		the _next available packet_. If the packet is larger than
 *		`size`, the remaining bytes of the packet are discarded.
 *
 * @return: The number of bytes actually read.
 */
int infected_recv(struct infected_sk *sock, void *buf, size_t size);

/**
 * Receive a single part of a message from a socket.
 * @param sock: The socket to receive data from.
 * @param buf: A Pointer to hold the data. Must have a length of at least
 *		`size` bytes.
 * @param size: The maximum amount of bytes to read. See `infected_recv`.
 * @param msg_id: will hold the message id of that part. Must not be NULL.
 * @param flags: will hold the multipart flags for this part. Must not be NULL.
 *
 * @return: The number of bytes actually read.
 */
int infected_recv_multipart(struct infected_sk *sock, void *buf, size_t size,
			    uint16_t *msg_id, uint8_t *flags);

/**
 * Close an Infected socket.
 * @param sock: The socket to close.
 */
void infected_close(struct infected_sk *sock);

#endif /* INFECTED_SOCKET_H */
