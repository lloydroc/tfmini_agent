#ifndef OPTIONS_H
#define OPTIONS_H
#include "../config.h"
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "error.h"

extern int use_syslog;

struct options
{
  int help;
  int reset;
  int verbose;
  int read_firmware_version;
  int set_measurement_unit_cm;
  int set_measurement_unit_mm;
  int enable_lidar_output;
  int disable_lidar_output;
  int update_rate;
  int baud_rate;
  int uart_dev;
  int daemon;
  int poll;;
  int output_standard;
  struct sockaddr_in socket_udp_dest;
  int fd_socket_udp;
  int needs_output_disabled;
  int needs_output_enabled;
  int discard_bad_checksum;
};

void
usage(char *progname);

void
options_init(struct options *opts);

int
options_deinit(struct options *opts);

int
options_parse(struct options *opts, int argc, char *argv[]);

#endif
