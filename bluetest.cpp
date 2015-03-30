/*
 * C program to test portability (Windows/Linux/RPi) of Bluetooth interface
 * Compile with:
 * Linux      gcc -Wno-write-strings -o bluetest bluetest.cpp -lbluetooth
 * Windows     "          "               "           "       -lwsock32
 */
#define LINUX 1
// #define WINDOWS 1

#ifdef WINDOWS
#include "plbluewindows.h"
#else
#include "plbluelinux.h"
#endif

int bluetoothSocket(char *dest) {

  initialize;
  str2ba(dest, &addr.rc_bdaddr);
  btport(1);

  int s = -1;
  while (s == -1) {
#ifdef WINDOWS
    s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
#else
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#endif
    if (s == -1) {
      fprintf(stderr, "Trying to create a Bluetooth socket...%d\n", get_error);
      sleep(3);
    }
  }

  while ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) ) {
    fprintf(stderr, "Trying to connect to %s WSA error %d\n", dest, get_error);
    sleep(4);
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
  fprintf(stderr,"%s\n", buf);
  if( bytes_read > 0 ) return buf;

  return (char *)"NAK";  /* indicate communication failure */
}

int main(int argc, char **argv)
{
  if (argc < 2 || strlen(argv[1]) != 17) {
    fprintf(stderr, "usage:  bluetest <BLUETOOTH-MACADDRESS>\n");
    exit(0);
  }
  int s = bluetoothSocket(argv[1]);

  converse(s, "v\n");
  converse(s, "d\n");
  return 0;
}

