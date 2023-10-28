#define __MAIN_C__
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include "app.h"

static int ttyFd = -1;

static struct {
	int baud;
	int br;
}baudRate[] = {{9600, B9600}, {19200, B19200}, {38400, B38400}, {57600, B57600}, {115200, B115200}, {0,0}};

extern char *optarg;
extern int optind, opterr, optopt;

// service functions ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int open_tty(char *drv) {
	speed_t sp = 0;

	for(int i = 0; baudRate[i].baud != 0; i++){
		if (appConfig.baud == baudRate[i].baud) {
			sp = baudRate[i].br;
			break;
		}
	}
	if (!sp) {
		puts("error baud !");
		return -1;
	}
	int fd = open(drv, O_RDWR | O_NOCTTY | O_NONBLOCK);
//	int fd = open(drv, O_RDWR | O_NOCTTY);
	if (fd > 0)	{
		struct termios tc;

		tcgetattr(fd, &tc);
//			cfmakeraw(&tc);
		tc.c_iflag &= ~(BRKINT | ICRNL | IMAXBEL | INLCR | IGNBRK | IGNCR | IXON);
		tc.c_oflag &= ~(OPOST | ONLCR);
		tc.c_cflag &= ~(CSIZE | PARENB);
		tc.c_cflag |= CS8;
		tc.c_lflag &= ~(ECHO | ICANON | ISIG | ECHOE);
		tc.c_cc[VMIN] = 1;
		tc.c_cc[VTIME] = 200;
		cfsetispeed(&tc, sp);
		cfsetospeed(&tc, sp);
		tcsetattr(fd, TCSANOW, &tc);
		printf("tty speed:%d\n",appConfig.baud);
	}
	return fd;
}

void hex_dump(unsigned char *buf, unsigned short len, unsigned char col) {
	unsigned inx = 0;
	uint8_t cl = 0;

	fprintf(stdout, "%04X : ", inx);
	while(inx < len) {
		fprintf(stdout, "%02x ", buf[inx++]);
		cl++;
		if (cl == col) {
			cl = 0;
			fprintf(stdout, "\n%04X : ", inx);
		}
	}
	fprintf(stdout, "\r\n");
}
static void timer1_func() {
	if (appConfig.cmd) {
		protType.command_send(ttyFd, appConfig.cmd);
		appConfig.ans = 1;
	}
}

static unsigned long get_tstamp() {
	struct timeval tm;
	static unsigned long old = 0;
	unsigned long ul1 ,ret = 0;

	if (!gettimeofday(&tm, NULL)) {
		ul1 = (((unsigned long)tm.tv_sec) * 1000l) + ((unsigned long)tm.tv_usec / 1000l);
		if (!old)
			old = ul1;
		ret = ul1 - old;
		old = ul1;
	}
	return ret;
}
// service functions ------------------------------------------------------------------------------
static char read_bytes(int fd, short cnd, void *p) {
//	puts("Read bytes");
	struct ReadBuf_s *rb = p;
	size_t rbytes;
	char ret = 1;

//	rbytes = read(fd, &rb->buf[rb->inx], (size_t)(RBUF_MAX - rb->inx));
	rbytes = read(fd, &rb->buf[rb->inx], 1);
	if (rbytes >= 0) {
		rb->inx += rbytes;

		if (rb->inx >= RBUF_MAX) rb->inx = 0;

//		hex_dump(rb->buf, rb->inx, 32);
//		protType.parse_buf_func(rb);
	}
	if (rbytes > 0)
		ret = 1;

	if (rbytes == 0)
		ret = 0;

	if (rbytes < 0)
		ret = -1;

	return ret;
}
/*
static gboolean app_activate(struct CmdLine_s *cmd) {
	g_io_add_watch(ttyFd, G_IO_IN | G_IO_PRI | G_IO_HUP, read_bytes, &rBuf);
	g_timeout_add(1000, timer_func, NULL);
	if (!appConfig.visca) {
		test_conn(ttyFd);
	}
	return TRUE;
}
*/
static void usage(char *prg) {
	printf("Usage: %s [-d /dev/ttyUSB0] [-b 115200] [-v] [-c]\n"
			"  -d <device name>\n  -b <baudrate>\n  -s - Single command" 	
			"  -v - Enable Visca\n  -c - Send command\n", prg);
}

char parse_cmdline(int argc, char **argv) {
	int res;
	char ret = 1;

	appConfig.flags = xcore_la;
	appConfig.cmd = 0;
	appConfig.single = 0;
	while((res = getopt(argc, argv, "c:vd:b:")) != -1) {
		switch(res) {
			case 'c':
				appConfig.cmd = atoi(optarg);
				break;
			case 'v':
				appConfig.flags = visca;
				break;
			case 's':
				appConfig.single = 1;
				break;
			case 'd':
				appConfig.drvName = optarg;
				break;
			case 'b':
				appConfig.baud = atoi(optarg);
				break;
			default:
				usage(argv[0]);
				ret = 0;
				break;
		}
	}

	if (!appConfig.drvName) {
		appConfig.drvName = "/dev/ttyUSB0";
	}
	if (!appConfig.baud) {
		appConfig.baud = 115200;
	} 
	if (appConfig.flags == xcore_la) {
		protType.parse_buf_func = parse_buf;
		protType.command_send = test_conn;
	}
	if (appConfig.flags == visca) {
		protType.parse_buf_func = visca_buf;
		protType.command_send = visca_cmd;
	}
	return ret;
}

int main(int argc, char *argv[]) {
	if (!parse_cmdline(argc, argv))
		return -1;

//	struct CmdLine_s cmdLine = {.argc = argc, .argv = argv};

	static unsigned long tstamp, timer1 = 0, tlim1 = 1000;
	puts(appConfig.drvName);
	ttyFd = open_tty(appConfig.drvName);
	rBuf.tty = ttyFd;
	if (ttyFd <= 0) {
		fprintf(stderr, "Error open device: %s\n", appConfig.drvName);
		return -1;
	} else {
		if (appConfig.cmd) {
			protType.command_send(ttyFd, appConfig.cmd);
			appConfig.ans = 1;
		}
		appConfig.run = 1;
		struct pollfd pfd = {ttyFd, POLLIN | POLLPRI | POLLHUP, 0};
		while(appConfig.run) {
			int rt = poll(&pfd, 1, 50);
			if (rt > 0) {
				if (pfd.revents & POLLIN) {
//					if (appConfig.ans == 1) appConfig.ans = 2;
					if (!read_bytes(ttyFd, pfd.revents, &rBuf))
						appConfig.run = 0;
				}
				if (pfd.revents & POLLHUP)
					appConfig.run = 0;
			}
			if (rt == 0) {
				if (rBuf.inx)
					protType.parse_buf_func(&rBuf);
				if (appConfig.ans == 1) {
					puts("answer timeout !");
					appConfig.ans = 0;
				}
				if (appConfig.ans == 2) {
	//				puts("answer recive !");
					appConfig.ans = 0;
	//				printf("Recive :");
				}
				if (!appConfig.single) {
					tstamp = get_tstamp();
					timer1 += tstamp;
					if (timer1 >= tlim1) {
						timer1_func();
						timer1 = 0;
					}
				}
//				printf("Time stamp: %lu\n", tstamp);
			}
			if (rt < 0)
				appConfig.run = 0;
		}
		close(ttyFd);
	}

	return 0;
}

