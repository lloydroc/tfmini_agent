// file: uart.h
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "error.h"

#define UART0 "/dev/serial0"

int uart_open();
