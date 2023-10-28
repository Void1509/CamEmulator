#ifndef __APP_H__
#define __APP_H__
#include <stdint.h>

#define RBUF_MAX 		256
#define XCORE_HEADER 0xAA
#define XCORE_HEADER_ANSWER 0x55
#define XCORE_TERMINATOR_1b 0xEB // first byte
#define XCORE_TERMINATOR_2b 0xAA // second byte

#define VISCA_HEADER		0x80	
#define VISCA_BROADCAST		0x88
#define VISCA_TERMINATOR	0xff

enum Flags_e{
	xcore_la = 0,
	visca = 1
};

struct CmdLine_s {
	int argc;
	char **argv;
};

struct ReadBuf_s {
	uint16_t inx;
	uint8_t buf[RBUF_MAX];
	int tty;
};

struct AppConfig_s {
	char *drvName;
	int baud;
	enum Flags_e flags;
	struct {
		unsigned run:1;
		unsigned single:1;
	};
//	char run;
	char cmd;
	char ans;
};

struct ProtocolType_s {
	char (*parse_buf_func)(struct ReadBuf_s *bf);
	void (*command_send)(int ch, uint8_t cmd);
};

#ifdef __MAIN_C__
char *devName = NULL;
struct AppConfig_s appConfig = {NULL, 0, 0, 0};
struct ReadBuf_s rBuf = {.inx = 0, .tty = 0};
struct ProtocolType_s protType;
#else
extern char *devName;
extern struct AppConfig_s appConfig;
extern struct ReadBuf_s rBuf;
extern struct ProtocolType_s protType;
#endif

void hex_dump(unsigned char *buf, unsigned short len, unsigned char col);
char parse_buf(struct ReadBuf_s *rb);
void test_conn(int ch, uint8_t cmd);
char visca_buf(struct ReadBuf_s *rb);
void visca_cmd(int ch, uint8_t cmd);
#endif
