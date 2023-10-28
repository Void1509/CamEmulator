#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "app.h"

// cmd table
static const uint8_t cmdGetSN[] = {3, 00, 00, 00};
static const uint8_t cmdGetPN[] = {3, 00, 01, 00};
static const uint8_t cmdGetMatrixW[] = {3, 00, 02, 00};
static const uint8_t cmdGetMatrixH[] = {3, 00, 03, 00};
static const uint8_t cmdGetZoom[] = {3, 00, 0x2A, 00};
static const uint8_t cmdSetZoom1[] = {12, 00, 0x2A, 01, 00, 00, 00, 00, 00, 01, 0x33, 01, 0x33};
static const uint8_t cmdSetZoom2[] = {12, 00, 0x2A, 01, 01, 0xC0, 0x0C, 0xC0, 0x0C, 0x40, 0x26, 0x40, 0x26};
static const uint8_t cmdSetZoom4[] = {12, 00, 0x2A, 01, 02, 0x20, 0x13, 0x20, 0x13, 0xE0, 0x1F, 0xE0, 0x1F};
static const uint8_t cmdGetPalitra[] = {3, 00, 0x2d, 00};

// answer table
static const uint8_t ansGetSN[] = {0x0c, 00, 00, 0x33, 0x30, 0x31, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 00};
// Name XCORE_LA
static const uint8_t ansGetPN[] = {0x17, 00, 01, 0x33, 0x4c, 0x41, 0x37, 0x31, 0x31, 0x33, 0x30, 0x30, 0x32, 0x59, 
								   0x30, 0x33, 0x35, 0x31, 0x30, 0x58, 0x45, 0x4e, 0x50, 0x58};

static const uint8_t ansGetMatrixW[] = {5, 00, 02, 0x33, 0x80, 0x01 };
static const uint8_t ansGetMatrixH[] = {5, 00, 03, 0x33, 0x20, 0x01 };
static const uint8_t ansGetZoom[] = {0x5, 00, 0x2a, 0x33, 0x64, 00};
static const uint8_t ansGetPalitra[] = {0x4, 00, 0x2d, 0x33, 0x03};


static uint8_t* build_packet(const uint8_t *data, uint8_t ans);

static void nop_func(struct ReadBuf_s *rb);
static void get_sn_func(struct ReadBuf_s *rb);
static void get_pn_func(struct ReadBuf_s *rb);
static void get_matrixW_func(struct ReadBuf_s *rb);
static void get_matrixH_func(struct ReadBuf_s *rb);
static void get_zoom_func(struct ReadBuf_s *rb);
static void get_palitra_func(struct ReadBuf_s *rb);
static void set_zoom1_func(struct ReadBuf_s *rb);
static void set_zoom2_func(struct ReadBuf_s *rb);
static void set_zoom4_func(struct ReadBuf_s *rb);

static struct ParsePack_s{
	const uint8_t *cmd;
	void (*cmd_func)(struct ReadBuf_s *rb);
}parsePack[] = { {cmdGetSN, get_sn_func}, {cmdGetPN, get_pn_func}, {cmdGetPalitra, get_palitra_func},
	{cmdGetMatrixW, get_matrixW_func}, {cmdGetMatrixH, get_matrixH_func}, {cmdGetZoom, get_zoom_func},
	{cmdSetZoom1, set_zoom1_func},{cmdSetZoom2, set_zoom2_func},{cmdSetZoom4, set_zoom4_func},
	{NULL, NULL}};


static uint8_t calcSC(uint8_t *bf) {
	uint16_t ui1 = 0;
	uint8_t sz;

	sz = bf[1] + 1;

	for (int i = 0; i < sz; i++)
		ui1 += bf[i];

	return((uint8_t)(ui1 & 0xff));
}
/*
 * data - data to send
 * ans - flag 0 - cmd; 1 - answer
 */
static uint8_t* build_packet(const uint8_t *data, uint8_t ans) {
	uint8_t len = data[0];
	uint8_t *ret = (uint8_t*)malloc(len + 5);
	ret[0] = (ans)?XCORE_HEADER_ANSWER:XCORE_HEADER;
	ret[1] = len + 1;
	memcpy(&ret[2], &data[1], len);
	{
		uint8_t pack_crc = 0;
		for(uint8_t i = 0; i < (len + 2); i++) {
			pack_crc += ret[i];
		}
		ret[len + 2] = pack_crc;
	}
	ret[len + 3] = XCORE_TERMINATOR_1b;
	ret[len + 4] = XCORE_TERMINATOR_2b;

	return ret;
}


// parse functions ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void get_sn_func(struct ReadBuf_s *rb) {

	fprintf(stdout, "Get packet Get SN:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetSN, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}

static void get_pn_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Get packet Get PN:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetPN, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}

static void get_palitra_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Get packet Get Palitra:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetPalitra, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}


static void get_matrixW_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Get packet Get MatrixW:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetMatrixW, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}


static void get_matrixH_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Get packet Get MatrixH:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetMatrixH, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}


static void get_zoom_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Get packet Get Zoom:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
	uint8_t *wbuf = build_packet(ansGetZoom, 1);
	size_t wb = 0;

	wb = write(rb->tty, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fprintf(stdout, "Send answer:");
		hex_dump(wbuf, wb, wb+1);
		fsync(rb->tty);
	}
	free(wbuf);	
}

static void set_zoom1_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Set Zoom1:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
}

static void set_zoom2_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Set Zoom1:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
}

static void set_zoom4_func(struct ReadBuf_s *rb) {
	fprintf(stdout, "Set Zoom1:");
	hex_dump(rb->buf, rb->inx, rb->inx + 1);
}

static void nop_func(struct ReadBuf_s *rb) {
	(void)rb;
}

static void parse_cmd(int ch, uint8_t *bf, uint8_t len) {

	struct ParsePack_s *pp = parsePack;
	struct ReadBuf_s trb;

	memcpy(trb.buf, bf, len);
	trb.inx = len;
	trb.tty = ch;
	while(pp->cmd) {

		if (!memcmp(bf, &pp->cmd[1], pp->cmd[0])) {
			pp->cmd_func(&trb);
			break;
		}
		pp++;
	}
}
/*
static void parse_pack(int ch, struct ReadBuf_s *rb) {
	struct ParsePack_s *pp = parsePack;
	while(pp->cmd) {

		if (rb->inx >= (pp->cmd[0] + 5)) {
			if (!memcmp(&rb->buf[2], &pp->cmd[1], pp->cmd[0])) {
				pp->cmd_func(ch, rb);
				break;
			}
		}

		pp++;
	}
	rb->inx = 0;
}
*/
char parse_buf(struct ReadBuf_s *rb) {
	char ret = 0;
	int16_t sInx = 0;

	while(sInx < rb->inx) {
		if (rb->buf[sInx] == 0xaa) {
			uint8_t sz = rb->buf[sInx + 1];
			if (rb->inx >= (sz + 4)) {
				uint8_t crc = calcSC(&rb->buf[sInx]);
				if (crc == rb->buf[sInx + sz + 1]) {
					parse_cmd(rb->tty, &rb->buf[sInx + 2], sz - 1);
					sInx += (sz + 4);
					ret = 1;
				} else {
					sInx++;
				}
			} else {
				sInx++;
			}
		} else {
			if (rb->buf[sInx] == 0x55) {
				uint8_t sz = rb->buf[sInx + 1];
				printf("CMD Answer:");
				hex_dump(&rb->buf[sInx], sz + 4, 16);
				sInx += (sz + 4);
			} else {
				sInx++;
			}
		}
	}
/*	
	if (ret) {
		hex_dump(rb->buf, rb->inx, 32);
		parse_pack(rb->tty, rb);
	}
*/
	rb->inx = 0;
	return ret;
}
// parse functions --------------------------------------------------------------------------------

void test_conn(int ch, uint8_t cmd) {
	uint8_t *wbuf = build_packet(parsePack[cmd].cmd, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(wbuf[1]+4));
	if (wb >= 0) {
		fsync(ch);
		printf("Send buffer:");
		hex_dump(wbuf, (unsigned short)(wbuf[1] + 4), 32);
	}
	free(wbuf);
	rBuf.inx = 0;
}

