#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "app.h"

/*
 * brc - broadcast
 * cmd - command
 * inq - inquiry
 */
static const uint8_t brcAdrSet[] = {2, 0x30, 01};
static const uint8_t brcClearIF[] = {3, 01, 00, 01};
static const uint8_t inqGetPowerOn[] = {3, 0x09, 0x04, 0};
static const uint8_t inqGetCamID[] = {3, 0x09, 0x04, 0x22};
static const uint8_t inqGetCamModel[] = {3, 0x09, 0, 0x37};
static const uint8_t inqGetCamVersion[] = {3, 0x09, 0, 2};
static const uint8_t inqGetCamZoom[] = {3, 0x09, 4, 0x47};
static const uint8_t inqGetCamFocus[] = {3, 0x09, 4, 0x48};
static const uint8_t setCamRes[] = {6, 1, 4, 0x24, 0x72, 0, 8};
static const uint8_t cmdCamZoomTele[] = {4, 01, 04, 07, 02};
static const uint8_t cmdCamZoomWide[] = {4, 01, 04, 07, 03};

static const uint8_t answerCamModel[] = {7, 0x50, 00, 0x7e, 0x52, 0,0, 0xff};
static const uint8_t answerCamVersion[] = {9, 0x50, 00, 0x78, 4, 0x67, 1, 0x23, 0, 0xff};

static uint8_t* build_packet(const uint8_t *data, uint8_t br);
static uint8_t devAddr = 1;
static void (*getAnswer_func)(uint8_t *bf, uint8_t len) = NULL;


void ansGetCamZoom(uint8_t *bf, uint8_t len);
void ansGetCamFocus(uint8_t *bf, uint8_t len);
void ansSetCamRes(uint8_t *bf, uint8_t len);

static void getPowerOnAns(uint8_t *bf, uint8_t len, int ch);
static void getCamIDAns(uint8_t *bf, uint8_t len, int ch);
static void getCamModelAns(uint8_t *bf, uint8_t len, int ch);
static void getCamVersionAns(uint8_t *bf, uint8_t len, int ch);
static void getCamZoomAns(uint8_t *bf, uint8_t len, int ch);
static void getCamFocusAns(uint8_t *bf, uint8_t len, int ch);

struct {
	const uint8_t *inc;
	void (*inq_func)(uint8_t *buf, uint8_t len, int ch);
}parseInq[] = {{inqGetPowerOn, getPowerOnAns}, {inqGetCamID, getCamIDAns}, {inqGetCamModel, getCamModelAns}, 
	{inqGetCamVersion, getCamVersionAns},{inqGetCamZoom, getCamZoomAns}, {inqGetCamFocus, getCamFocusAns}, {NULL, NULL}};

struct appFlags {
	unsigned answer:1;
}flags = {0};

/*
 * build package
 * data - package data
 * br - package broadcast
 */
static uint8_t* build_packet(const uint8_t *data, uint8_t br) {
	uint8_t len = data[0];
	uint8_t *ret = (uint8_t*)malloc(len + 2);
	if (br) {
		ret[0] = 0x88;
	} else {
		ret[0] = 0x80 | devAddr;
	}
	memcpy(&ret[1], &data[1], len);
	ret[len + 1] = 0xff;

	return ret;
}

static void parse_inq(uint8_t *bf, uint8_t len, int ch) {
	uint8_t ui1 = 0;
	while(parseInq[ui1].inc) {
		if (!memcmp(&parseInq[ui1].inc[1], &bf[1], (size_t) parseInq[ui1].inc[0])) {
			parseInq[ui1].inq_func(bf, len, ch);
			break;
		}
		ui1++;
	}
}

static void getPowerOnAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbuf[7];
	size_t wb;

	wbuf[0] = 0x80 | ((bf[0] & 7) << 4);
	wbuf[1] = 0x50;
	wbuf[2] = 0x02;
	wbuf[3] = 0xff;

	wb = write(ch, wbuf, 4);
	fsync(ch);
//	printf("Send PowerOn:");
//	hex_dump(wbuf, 4, 16);
}
static void getCamIDAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbuf[7];
	size_t wb;

	wbuf[0] = 0x80 | ((bf[0] & 7) << 4);
	wbuf[1] = 0x50;
	wbuf[2] = 0x08;
	wbuf[3] = 0x01;
	wbuf[4] = 0x08;
	wbuf[5] = 0x01;
	wbuf[6] = 0xff;

	wb = write(ch, wbuf, 7);
	fsync(ch);
//	printf("Send CamID:");
//	hex_dump(wbuf, 7, 16);
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
static void getCamModelAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbf[16];

	memcpy(wbf, answerCamModel, 8);

	wbf[0] = 0x80 | ((bf[0] & 7) << 4);
	write(ch, wbf, 8);
	fsync(ch);
//	printf("Send CamVersion:");
//	hex_dump(wbf, 16, 16);
}
static void getCamVersionAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbf[16];
	uint8_t ln = answerCamVersion[0] + 1;

	memcpy(wbf, answerCamVersion, ln);

	wbf[0] = 0x80 | ((bf[0] & 7) << 4);
	(void)write(ch, wbf, ln);
	fsync(ch);
	printf("Send CamVersion:");
	hex_dump(wbf, ln, 16);
}
#pragma GCC diagnostic pop
static void getCamZoomAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbuf[7];
	size_t wb;

	wbuf[0] = 0x80 | ((bf[0] & 7) << 4);
	wbuf[1] = 0x50;
	wbuf[2] = 0x01;
	wbuf[3] = 0x02;
	wbuf[4] = 0x08;
	wbuf[5] = 0x01;
	wbuf[6] = 0xff;

	wb = write(ch, wbuf, 7);
	fsync(ch);
//	printf("Send CamZoom:");
//	hex_dump(wbuf, 7, 16);
}
static void getCamFocusAns(uint8_t *bf, uint8_t len, int ch) {
	uint8_t wbuf[7];
	size_t wb;

	wbuf[0] = 0x80 | ((bf[0] & 7) << 4);
	wbuf[1] = 0x50;
	wbuf[2] = 0x01;
	wbuf[3] = 0x00;
	wbuf[4] = 0x01;
	wbuf[5] = 0x0a;
	wbuf[6] = 0xff;

	wb = write(ch, wbuf, 7);
	fsync(ch);
//	printf("Send CamFocus:");
//	hex_dump(wbuf, 7, 16);
}

void ansSetCamRes(uint8_t *bf, uint8_t len) {
	printf("Answer setCamRes:");
	hex_dump(bf, len, 16);
}

char visca_buf(struct ReadBuf_s *rb) {
	char ret = 0;
	uint8_t tstChar = 0x80 | devAddr;
	uint8_t sInx = 0;	// start inx
	uint8_t eInx = 0;	// end inx

	printf("Recive:");
	hex_dump(rb->buf, rb->inx, 16);
/*	
	if (!memcmp(rb->buf, "\x81\x01", 2)) {
		printf("Recive:");
	}
*/
	do {
		eInx = 0;	// end inx
		while(sInx < rb->inx) {
			if (rb->buf[sInx] == tstChar) {
				break;
			} else {
				sInx++;
			}
		}
		if (sInx < rb->inx) {
			uint8_t len;
			for(len = 0; (len + sInx) < rb->inx; len++) {
				if (rb->buf[sInx + len] == 0xff) {
					eInx = len;
					break;
				}
			}
			if (eInx) {
				uint8_t *tbf = &rb->buf[sInx];
//				printf("Recive2:");
//				hex_dump(tbf, eInx, 16);
				if (flags.answer) {
					if (getAnswer_func) {
						getAnswer_func(tbf, eInx);
						getAnswer_func = NULL;
					}
				} else {
					parse_inq(tbf, eInx, rb->tty);
				}
			}
			sInx += len;
		}
	}while(sInx < rb->inx);

	rb->inx = 0;
	flags.answer = 0;
	return ret;
}

static void send_brcAdrSet(int ch) {
	uint8_t *wbuf = build_packet(brcAdrSet, 1);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(brcAdrSet[0] + 2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, brcAdrSet[0] + 2, 16);
	}
	free(wbuf);
}
static void send_brcClearIF(int ch) {
	uint8_t *wbuf = build_packet(brcClearIF, 1);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(brcClearIF[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, brcClearIF[0] + 2, 16);
	}
	free(wbuf);
}
static void send_inqGetCamID(int ch) {
	uint8_t *wbuf = build_packet(inqGetCamID, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(inqGetCamID[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, inqGetCamID[0] + 2, 16);
	}
	free(wbuf);
}
static void send_inqGetCamModel(int ch) {
	uint8_t *wbuf = build_packet(inqGetCamModel, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(inqGetCamModel[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, inqGetCamModel[0] + 2, 16);
	}
	free(wbuf);
}
static void send_inqGetCamVersion(int ch) {
	uint8_t *wbuf = build_packet(inqGetCamVersion, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(inqGetCamVersion[0]+2));
	if (wb >= 0) {
		fsync(ch);
		printf("Send buffer:");
		hex_dump(wbuf, inqGetCamModel[0] + 2, 16);
	}
	free(wbuf);
}
static void send_cmdCamZoomTele(int ch) {
	uint8_t *wbuf = build_packet(cmdCamZoomTele, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(cmdCamZoomTele[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, cmdCamZoomTele[0] + 2, 16);
	}
	free(wbuf);
}
static void send_cmdCamZoomWide(int ch) {
	uint8_t *wbuf = build_packet(cmdCamZoomWide, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(cmdCamZoomWide[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, cmdCamZoomWide[0] + 2, 16);
	}
	free(wbuf);
}

static void send_inqGetCamZoom(int ch) {
	uint8_t *wbuf = build_packet(inqGetCamZoom, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(inqGetCamZoom[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, inqGetCamZoom[0] + 2, 16);
		getAnswer_func = ansGetCamZoom;
	}
	free(wbuf);
}

void ansGetCamZoom(uint8_t *bf, uint8_t len) {

		uint16_t pqrs = 0;
		pqrs = bf[2] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[3] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[4] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[5] & 0xf;
		printf("Zoom pqrs:%hx\n", pqrs);
}

static void send_inqGetCamFocus(int ch) {
	uint8_t *wbuf = build_packet(inqGetCamFocus, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(inqGetCamFocus[0]+2));
	if (wb >= 0) {
		fsync(ch);
//		printf("Send buffer:");
//		hex_dump(wbuf, inqGetCamFocus[0] + 2, 16);
		getAnswer_func = ansGetCamFocus;
	}
	free(wbuf);
}

static void send_setCamRes(int ch) {
	uint8_t *wbuf = build_packet(setCamRes, 0);
	size_t wb;

	wb = write(ch, wbuf, (size_t)(setCamRes[0]+2));
	if (wb >= 0) {
		fsync(ch);
		printf("Send buffer:");
		hex_dump(wbuf, setCamRes[0] + 2, 16);
		getAnswer_func = ansSetCamRes;
	}
	free(wbuf);
}

void ansGetCamFocus(uint8_t *bf, uint8_t len) {

		uint16_t pqrs = 0;
		pqrs = bf[2] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[3] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[4] & 0xf;
		pqrs <<= 4;
		pqrs |= bf[5] & 0xf;
		printf("Focus pqrs:%hx\n", pqrs);
}

void visca_cmd(int ch, uint8_t cmd) {
	printf("CMD: %hhu\n", cmd);
	flags.answer = 1;
	switch(cmd) {
		case 1:
			send_brcAdrSet(ch);
			break;
		case 2:
			send_brcClearIF(ch); 
			break;
		case 3:
			send_inqGetCamID(ch);
			break;
		case 4:
			send_inqGetCamModel(ch);
			break;
		case 5:
			send_inqGetCamVersion(ch);
			break;
		case 6:
			send_cmdCamZoomTele(ch);
			break;
		case 7:
			send_cmdCamZoomWide(ch);
			break;
		case 8:
			send_inqGetCamZoom(ch);
			break;
		case 9:
			send_inqGetCamFocus(ch);
			break;
		case 10:
			send_setCamRes(ch);
			break;
	}
	//	send_brcAdrSet(ch);
	//	send_brcClearIF(ch); 
	//	send_inqGetCamID(ch);
	rBuf.inx = 0;
}

