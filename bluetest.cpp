/*
 * C program to test portability (Windows/Linux/RPi) of Bluetooth interface
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

char *converse(int s, char const *cmd) {
  static char buf[1024];
  int bytes_read;
  memset(buf,0,1024);
  write(s, cmd, strlen(cmd));

  sleep(4);  // Give the guy a chance to respond fully

  bytes_read = read(s, buf, sizeof(buf));
  if( bytes_read > 0 ) return buf;

  return (char *)"timeout.\r\nend_of_data\r\n";  /* indicate communication failure */
}

int main(int argc, char **argv)
{
  if (argc < 2 || strlen(argv[1]) != 17) {
    fprintf(stderr, "usage:  bluetest <BLUETOOTH-MACADDRESS>\n");
    exit(0);
  }
  int s = bluetoothSocket(argv[1]);

  int ncmds = 3;
  char *cmds[3] = { "i\n", 
		    "t\n",
		    "b\n" };
  for (int i =0; i<ncmds; i++) {
    fprintf(stderr, "sent[%s] got[%s]\n", cmds[i], converse(s, cmds[i]));
    sleep(1);
  }
  sleep(1);
  return 0;
}

