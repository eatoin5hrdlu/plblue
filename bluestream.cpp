/*
 * C program to stream data from Bluetooth serial device
 * Compile with:
 * Linux      gcc -Wno-write-strings -o bluetest bluetest.cpp -lbluetooth
 * Windows     "          "               "           "       -lwsock32
 */
#include <errno.h>

#ifdef WINDOWS
#include "plbluewindows.h"
#else
#include "plbluelinux.h"
#endif
int g_tries = 10;

int get_socket() {
  int s = -1;
  struct timeval timeout;      
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  while (s == -1 && 0 < g_tries--) {
#ifdef WINDOWS
    s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
#else
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#endif
    if (s == -1) {
      fprintf(stderr, "Trying to create a Bluetooth socket...%d\n", get_error);
      sleep(2);
    }
  }
  if (setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
    {
      perror("setsockopt read timeout failed\n");
    }
  if (setsockopt (s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
    {
      perror("setsockopt send timeout failed\n");
    }
  return s;
}


int bluetoothSocket(char *dest) {

  initialize;
  str2ba(dest, &addr.rc_bdaddr);
  btport(1);
  int s = get_socket();

  while ( s != -1 && connect(s, (struct sockaddr *)&addr, sizeof(addr)) && 0<g_tries--) {
    fprintf(stderr, "Trying to connect to %s error %d\n", dest, get_error);
#ifdef LINUX
    close(s);
#else
    closesocket(s);
#endif
    sleep(1);
    s = get_socket();
  }
  return s;
}

/* Write cmd to socket, wait, and then return response */

void bluestream(int s) {
  static char buf[1024];
  int bytes_read;
  while(1)
    {
      memset(buf,0,1024);
      sleep(0.1);
      bytes_read = read(s, buf, sizeof(buf));
      if( bytes_read > 0 )
	fprintf(stdout,"%s",buf);
      else
	sleep(0.2);
      if (bytes_read < 0 && errno != EAGAIN)
	return;
    }
}

/* 98:D3:31:FC:31:67 */
int main(int argc, char **argv)
{
  if (argc < 2 || strlen(argv[1]) != 17) {
    fprintf(stderr, "usage:  bluetest <BLUETOOTH-MACADDRESS>\n");
    fprintf(stderr, "Try:\nbluetest 98:D3:31:FC:31:67\n");
    exit(0);
  }
  int s = bluetoothSocket(argv[1]);
  bluestream(s);
  sleep(1);
  return 0;
}

