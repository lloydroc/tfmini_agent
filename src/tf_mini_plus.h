#ifndef TF_MINI_DEF
#define TF_MINI_DEF

#include <poll.h>
#include <string.h>
#include <sys/time.h>
#include "options.h"
#include "gpio.h"
#include "uart.h"

struct TF_MINI
{
  int verbose;
  int uart_fd;
  int firmware_version;
  int frame_rate;
  int measure_unit;
  int output_enabled;
};

struct TF_MINI_FRAME
{
  uint8_t uplink[9];
  uint8_t downlink[9];
  uint8_t uplink_size;
  uint8_t downlink_size;
  uint8_t calc_downlink_checksum;
};

struct TF_MINI_LIDAR_FRAME
{
  /* what comes in the frame from the uart */
  uint8_t header1;
  uint8_t header2;
  uint16_t distance;
  uint16_t strength;
  uint16_t temp;
  uint8_t checksum;
};

int
tf_mini_init(struct TF_MINI *dev, struct options *opts);

int
tf_mini_deinit(struct TF_MINI *dev, struct options *opts);

int
tf_mini_read_firmware_version(struct TF_MINI *dev);

int
tf_mini_system_reset(struct TF_MINI *dev);

int
tf_mini_set_update_rate(struct TF_MINI *dev, uint16_t update_rate);

int
tf_mini_set_measure_unit(struct TF_MINI *dev, int centimeters);

int
tf_mini_set_baud_rate(struct TF_MINI *dev, uint32_t baudrate);

int
tf_mini_set_enable_output(struct TF_MINI *dev, int output);

/*
TODO currently unimplemented
int
tf_mini_restore_factory_settings(struct TF_MINI *dev);

int
tf_mini_save_settings(struct TF_MINI *dev);

*/

int
tf_mini_poll(struct TF_MINI *dev, struct options* opts);

#endif
