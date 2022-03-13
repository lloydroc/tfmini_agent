#include "../config.h"
#include <stdio.h>

#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "become_daemon.h"
#include "error.h"
#include "tf_mini_plus.h"
#include "gpio.h"
#include "options.h"

struct options opts;
struct TF_MINI dev;

static
void signal_handler(int sig)
{
  int exit_status;

  if(opts.daemon)
    info_output("daemon stopping pid=%d", getpid());

  options_deinit(&opts);
  exit_status = tf_mini_deinit(&dev, &opts);
  exit(exit_status);
}

int
main(int argc, char *argv[])
{
  int ret = 0;

  if (signal(SIGINT, signal_handler) == SIG_ERR)
    err_output("installing SIGNT signal handler");
  if (signal(SIGTERM, signal_handler) == SIG_ERR)
    err_output("installing SIGTERM signal handler");

  options_init(&opts);
  ret = options_parse(&opts, argc, argv);

  if(opts.daemon && opts.poll)
  {
    err_output("Bad option combination: choose poll or daemon");
    return ret;
  }
  else if(ret || opts.help)
  {
    usage(argv[0]);
    return ret;
  }
  else if(opts.help)
  {
    usage(argv[0]);
    return EXIT_SUCCESS;
  }
  else if(opts.update_rate != -1 && (opts.update_rate == 0 || opts.update_rate > 1000))
  {
    fprintf(stderr, "invalid setting: lidar update rate must be between 1-1000Hz\n");
    return EXIT_FAILURE;
  }
  else if(opts.baud_rate != -1)
  {
    fprintf(stderr, "setting baud rate currently not supported\n");
    return EXIT_FAILURE;
  }
  else if(opts.enable_lidar_output && opts.disable_lidar_output)
  {
    fprintf(stderr, "cannot disable and enable lidar output\n");
    return EXIT_FAILURE;
  }
  else if(opts.set_measurement_unit_cm && opts.set_measurement_unit_mm)
  {
    fprintf(stderr, "cannot set measurement units to both cm and mm\n");
    return EXIT_FAILURE;
  }

  ret = tf_mini_init(&dev, &opts);
  if(ret)
    goto cleanup;

  // disable output first
  if(opts.needs_output_disabled)
  {
    // check if we read a lidar frame
    if(tf_mini_set_enable_output(&dev, 0) == 2)
    {
      // set the output again
      if(tf_mini_set_enable_output(&dev, 0))
      {
        err_output("unable to disable LiDAR output\n");
        goto cleanup;
      }
    }
  }

  // send downlink frames that require disabled output
  if(opts.reset)
  {
    ret |= tf_mini_system_reset(&dev);
  }
  if(opts.update_rate != -1)
  {
    ret |= tf_mini_set_update_rate(&dev, opts.update_rate);
  }
  if(opts.set_measurement_unit_mm)
  {
    ret |= tf_mini_set_measure_unit(&dev, 0);
  }
  if(opts.set_measurement_unit_cm)
  {
    ret |= tf_mini_set_measure_unit(&dev, 1);
  }
  if(opts.read_firmware_version)
  {
    ret |= tf_mini_read_firmware_version(&dev);
  }

  if(opts.daemon)
  {
    become_daemon();
  }

  // enable output if needed
  if(opts.needs_output_enabled)
  {
    tf_mini_set_enable_output(&dev, 1);
    dev.output_enabled = 1;
  }

  // poll for uplink lidar frames
  if(opts.daemon || opts.poll || opts.fd_socket_udp != -1)
  {
    ret |= tf_mini_poll(&dev, &opts);
  }

cleanup:
  ret |= tf_mini_deinit(&dev, &opts);

  return ret;
}
