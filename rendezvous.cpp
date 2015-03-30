/*
 * C program to run Rendezvous in Space
 *
 * 0) Connect to shuttle via Bluetooth
 * 1) Check for UP/DOWN button presses
 * 2) Send appropriate bluetooth command
 *            calculate rough speed adjustment
 *            Output PWM value for orbital speed of shuttle
 *
 * 3) If no button presses,
 *            request altitude from shuttle and make exact speed calculation
 *            Output PWM value for orbital speed of shuttle
 *
 * Compile with:
 * Linux (RPi)      gcc -Wno-write-strings -o rendezvous rendezvous.cpp -lbluetooth
 * Windows           "          "               "           "           -lwsock32
 *
 */
#define LINUX 1     // In theory Linux other than RPi would work, but would need some GPIO (via arduino?)
#define RPI 1       // Can be compiled for other Linux platforms, but need GPIO th
// #define WINDOWS 1

#ifdef WINDOWS
#include "plbluewindows.h"
#else
#include "plbluelinux.h"
#endif
int g_tries = 10;

int get_socket() {
  int s = -1;
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
  return s;
}
#ifdef RPI
#include <wiringPi.h>
#endif


int bluetoothSocket(char *dest) {

  initialize;
  str2ba(dest, &addr.rc_bdaddr);
  btport(1);
  int s = get_socket();

  while ( s != -1 && connect(s, (struct sockaddr *)&addr, sizeof(addr)) && 0<g_tries--) {
    fprintf(stderr, "Trying to connect to %s error %d\n", dest, get_error);
    close(s);
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
  fprintf(stderr,"%s\n", buf);
  if( bytes_read > 0 ) return buf;

  return (char *)"NAK";  /* indicate communication failure */
}


#ifdef RPi
const int pwmPin  = 18; // PWM LED - Broadcom pin 18, P1 pin 12
const int upPin   = 23;   // Active-low button - Broadcom pin 23, P1 pin 16
const int downPin = 24; // Active-low button - Broadcom pin 22, P1 pin 15

const int maxSpeed = 500;
const int minSpeed = 100;

const int max_altitude = 600;
const int min_altitude = 0;
const int velocity_factor = 10;

int velocity, actual_altitude;

int rendezvous(char *addr)
{
  fprintf(stderr, "Entering rendezvous connecting...");
  int s = bluetoothSocket(addr);
  fprintf(stderr, "connected at %d",s);
  wiringPiSetupGpio();          // Broadcom pin numbers
  fprintf(stderr, "wpi setup");

    pinMode(pwmPin, PWM_OUTPUT);  // Set Shuttle speed as PWM output
    pinMode(upPin, INPUT);        // Increase Altitude
    pinMode(downPin, INPUT);      // Decrease Altitude

    pullUpDnControl(upPin, PUD_UP);   // Enable pull-up resistors
    pullUpDnControl(downPin, PUD_UP);
  fprintf(stderr, "start loop");
    while(1) {
      fprintf(stderr, "check buttons");
      while (digitalRead(upPin) && !digitalRead(downPin)) {
	converse(s, "v\n");
        sleep(1);
	fprintf(stderr, "up\n");
      }
      while (digitalRead(downPin) && !digitalRead(upPin)) {
	converse(s, "d\n");
        sleep(1);
	fprintf(stderr, "down\n");
      }
      fprintf(stderr, "buttons checked");
      sleep(1);

      /* Re-adjust shuttle velocity according to actual altitude */

      actual_altitude = atoi(converse(s, "a\n"));
      fprintf(stderr, "Shuttle says altitude is %d\n", actual_altitude);

      velocity = minSpeed + ((max_altitude - actual_altitude)/velocity_factor);
      fprintf(stderr, "Computed velocity is %d\n", velocity);

      if (velocity < minSpeed) velocity = minSpeed;
      if (velocity > maxSpeed) velocity = maxSpeed;
      fprintf(stderr, "Adjusted for range as %d\n", velocity);

      pwmWrite(pwmPin, velocity);
 }
    return 0;
}

int main(int argc, char **argv)
{
  if (argc < 2 || strlen(argv[1]) != 17) {
    fprintf(stderr, "usage:  bluetest <BLUETOOTH-MACADDRESS>\n");
    exit(0);
  }
  rendezvous(argv[1]);
  fprintf(stderr, "returned from rendezvous. WTF?\n");
  return 0;
}

