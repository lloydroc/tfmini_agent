// file uart.c
#include "uart.h"

static int
tty_open(char *ptyName)
{
  int UART;
  struct termios ttyOrig;

  UART = open(ptyName, O_RDWR | O_NOCTTY);
  if(UART == -1)
  {
    err_output("error opening terminal %s\n", ptyName);
    close(UART);
    return UART;
  }

  if(tcgetattr(UART, &ttyOrig) == -1)
  {
    err_output("unable to get tty attributes for %s\n", ptyName);
    close(UART);
    return -1;
  }

  cfsetispeed(&ttyOrig, B115200);
  cfsetospeed(&ttyOrig, B115200);
  cfmakeraw(&ttyOrig);

  ttyOrig.c_cflag |= CREAD | CLOCAL;

  // blocking read - see man termios
  ttyOrig.c_cc[VMIN] = 1;
  ttyOrig.c_cc[VTIME] = 0;

  if(tcsetattr(UART, TCSANOW, &ttyOrig) == -1)
  {
    err_output("error setting terminal attributes");
    close(UART);
    return -1;
  }

  tcflush(UART, TCIFLUSH);
  tcdrain(UART);

  return UART;
}

int
uart_open()
{
  int FILE;
  /* check the uart exits */
  if(access(UART0, F_OK) == -1)
    return -2;
  FILE = tty_open(UART0);
  return FILE;
}
