#include "options.h"

// tells error.c when we print to use stdout, stderr, or syslog
int use_syslog = 0;

void
usage(char *progname)
{
  printf("Usage: %s [OPTIONS]\n\n", progname);
  printf("A command line tool to interact with the TF Mini+.\n");
  printf("OPTIONS:\n\
-h, --help                  Print help\n\
-r, --reset                 SW Reset\n\
    --firmware-version      Read firmware version\n\
    --measure-mm            Set measurment units to mm\n\
    --measure-cm            Set measurment units to cm\n\
    --disable-lidar-output  Disbale output of the LiDAR\n\
    --enable-lidar-output   Enable output of the LiDAR\n\
    --disable-lidar-output  Disbale output of the LiDAR\n\
    --set-update-rate RATE  Set Lidar Frame Rate [1-1000]Hz\n\
-x, --discard-bad-checksum  Discard LiDAR frames with bad checksums\n\
-v, --verbose               Verbose Output\n\
-u, --socket-udp HOST:PORT  Output data to a UDP Socket\n\
-p, --poll                  Poll the LiDAR and print to STDOUT\n\
-d, --daemon                Run as a Daemon\n\
");
  printf("\nVersion: %s\n", PACKAGE_VERSION);
}

void
options_init(struct options *opts)
{
  opts->reset = 0;
  opts->help = 0;
  opts->verbose = 0;
  opts->read_firmware_version = 0;
  opts->set_measurement_unit_cm = 0;
  opts->set_measurement_unit_mm = 0;
  opts->update_rate = -1;
  opts->baud_rate = -1;
  opts->enable_lidar_output = 0;
  opts->disable_lidar_output = 0;
  opts->uart_dev = 0;
  opts->poll = 0;
  opts->daemon = 0;
  opts->fd_socket_udp = -1;
  opts->needs_output_disabled = 0;
  opts->needs_output_enabled = 0;
  opts->discard_bad_checksum = 0;
}

int
options_deinit(struct options *opts)
{
  int ret;
  ret = 0;
  if(opts->daemon)
    closelog();

  if(opts->fd_socket_udp != -1)
    close(opts->fd_socket_udp);

  return ret;
}

static
int
options_open_socket_udp(struct options *opts, char *optarg)
{
  struct hostent *he;
  int sockfd, rc, optval;

  int prt;
  size_t len;
  char *index;
  char host[1024];
  char port[6];

  // separate the host and port by the :

  len = strnlen(optarg, 1024);
  if(len < 3 || len == 1024)
    return 1;

  index = rindex(optarg, ':');
  if(index == NULL)
    return 1;

  strncpy(port, index+1, 6);
  len = strnlen(port, 6);
  if(len < 1 || len == 6)
    return 1;

  prt = atoi(port);
  *index = '\0';

  strncpy(host, optarg, 1024);
  len = strnlen(host, 1024);
  if(len == 0 || len == 1024)
    return 1;

  if ( (he = gethostbyname(host) ) == NULL )
  {
      err_output("gethostbyname");
      return 1;
  }

  bzero(&opts->socket_udp_dest, sizeof(struct sockaddr_in));
  memcpy(&opts->socket_udp_dest.sin_addr, he->h_addr_list[0], he->h_length);
  opts->socket_udp_dest.sin_family = AF_INET;
  opts->socket_udp_dest.sin_port = htons(prt);

  sockfd = socket(opts->socket_udp_dest.sin_family, SOCK_DGRAM, 0);
  if(sockfd == -1)
  {
    err_output("socket");
  }

  rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if(rc)
  {
    err_output("setsockopt");
    return 1;
  }

  opts->fd_socket_udp = sockfd;

  return 0;
}

int
options_parse(struct options *opts, int argc, char *argv[])
{
  int c;
  int option_index;
  int ret = 0;

  static struct option long_options[] =
  {
    {"help",                       no_argument, 0, 'h'},
    {"reset",                      no_argument, 0, 'r'},
    {"firmware-version",           no_argument, 0,   0},
    {"measure-mm",                 no_argument, 0,   0},
    {"measure-cm",                 no_argument, 0,   0},
    {"set-update-rate",      required_argument, 0,   0},
    {"set-baud-rate",        required_argument, 0,   0},
    {"enable-lidar-output",        no_argument, 0,   0},
    {"disable-lidar-output",       no_argument, 0,   0},
    {"discard-bad-checksum",       no_argument, 0, 'x'},
    {"verbose",                    no_argument, 0, 'v'},
    {"socket-udp",           required_argument, 0, 'u'},
    {"binary",                     no_argument, 0, 'b'},
    {"poll",                       no_argument, 0, 'p'},
    {"daemon",                     no_argument, 0, 'd'},
    {0,                                      0, 0,   0}
  };

  while(1)
  {
    option_index = 0;
    c = getopt_long(argc, argv, "hrvxpdu:", long_options, &option_index);

    if(c == -1)
      break;

    switch(c)
    {
    case 0:
      if(strcmp("enable-lidar-output", long_options[option_index].name) == 0)
        opts->enable_lidar_output = 1;
      else if(strcmp("disable-lidar-output", long_options[option_index].name) == 0)
        opts->disable_lidar_output = 1;
      else if(strcmp("discard-bad-checksum", long_options[option_index].name) == 0)
        opts->discard_bad_checksum = 1;
      else if(strcmp("measure-mm", long_options[option_index].name) == 0)
        opts->set_measurement_unit_mm = 1;
      else if(strcmp("measure-cm", long_options[option_index].name) == 0)
        opts->set_measurement_unit_cm = 1;
      else if(strcmp("set-update-rate", long_options[option_index].name) == 0)
        opts->update_rate = atoi(optarg);
      else if(strcmp("set-baud-rate", long_options[option_index].name) == 0)
        opts->baud_rate = atoi(optarg);
      else if(strcmp("firmware-version", long_options[option_index].name) == 0)
        opts->read_firmware_version = 1;
      else if(strcmp("poll", long_options[option_index].name) == 0)
        opts->poll = 1;
      else if(strcmp("reset", long_options[option_index].name) == 0)
        opts->reset = 1;
      else if(strcmp("socket-udp", long_options[option_index].name) == 0)
        ret |= options_open_socket_udp(opts, optarg);
      break;
    case 'h':
      opts->help = 1;
      break;
    case 'r':
      opts->reset = 1;
      break;
    case 'x':
      opts->discard_bad_checksum = 1;
      break;
    case 'v':
      opts->verbose = 1;
      break;
    case 'u':
      ret |= options_open_socket_udp(opts, optarg);
      break;
    case 'p':
      opts->poll = 1;
      break;
    case 'd':
      opts->daemon = 1;
      break;
    }
  }

  opts->needs_output_disabled |= opts->read_firmware_version;
  opts->needs_output_disabled |= opts->set_measurement_unit_mm;
  opts->needs_output_disabled |= opts->set_measurement_unit_cm;
  opts->needs_output_disabled |= opts->read_firmware_version;
  opts->needs_output_disabled |= opts->disable_lidar_output;
  opts->needs_output_disabled |= opts->reset;

  opts->needs_output_enabled |= opts->daemon;
  opts->needs_output_enabled |= opts->poll;
  opts->needs_output_enabled |= opts->fd_socket_udp != -1;
  opts->needs_output_enabled |= opts->enable_lidar_output;

  // this option sets whether we log to syslog or not
  // and should be checked first
  if(opts->daemon)
  {
    use_syslog = 1;
    openlog("tf_mini", 0, LOG_DAEMON);
  }

  return ret;
}
