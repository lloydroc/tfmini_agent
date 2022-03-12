#include "tf_mini_plus.h"

static int
tf_mini_send_command(struct TF_MINI *dev, struct TF_MINI_FRAME *frame)
{
  ssize_t bytes, total_bytes;
  uint8_t *buf_ptr;
  uint16_t checksum = 0;
  uint16_t calc_checksum = 0;
  int downlink_checksum_index = frame->downlink_size-1;

  if(frame->calc_downlink_checksum)
  {
    frame->downlink[downlink_checksum_index] = 0;
    for(int i=0; i<downlink_checksum_index; i++)
    {
      calc_checksum += frame->downlink[i];
    }
    frame->downlink[downlink_checksum_index] = calc_checksum & 0xFF;
  }

  tcflush(dev->uart_fd, TCIFLUSH);

  bytes = write(dev->uart_fd, frame->downlink, frame->downlink_size);
  if(bytes == -1)
  {
    err_output("unable to write to UART\n");
    return -1;
  }

  if(dev->verbose)
  {
    info_output("writing: ");
    for(int i=0; i<frame->downlink_size; i++)
    {
      info_output("0x%02hhX ", frame->downlink[i]);
    }
    puts("");
  }

  bytes = 0;
  total_bytes = 0;
  buf_ptr = frame->uplink;
  do
  {
    bytes = read(dev->uart_fd, buf_ptr, 1);
    if(bytes == -1)
    {
      err_output("unable to read from UART\n");
      return -1;
    }
    total_bytes++;
    buf_ptr += (bytes != 0);
  }
  while(total_bytes < frame->uplink_size);

  // if output was previously enabled we may read
  // a lidar frame which was in the UART's buffer
  if(frame->uplink[0] == 0x59)
  {
    return 2;
  }

  if(dev->verbose)
  {
    info_output("read:    ");
    for(int i=0; i<frame->uplink_size; i++)
    {
      info_output("0x%02hhX ", frame->uplink[i]);
    }
    puts("");
  }

  for(int i=0; i<frame->uplink_size-1; i++)
  {
    checksum += frame->uplink[i];
  }
  checksum &= 0xFF;

  if(checksum != frame->uplink[frame->uplink_size-1])
  {
    err_output("checksum does not match\n");
    return 1;
  }

  return 0;
}

int
tf_mini_system_reset(struct TF_MINI *dev)
{
  struct TF_MINI_FRAME frame;
  int ret;
  int reset_failed;

  frame.downlink_size = 4;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x04;
  frame.downlink[2] = 0x02;
  frame.downlink[3] = 0x60;
  frame.calc_downlink_checksum = 0;

  frame.uplink_size = 5;

  ret = tf_mini_send_command(dev, &frame);
  if(ret)
  {
    err_output("unable to reset %d\n", ret);
    return ret;
  }

  reset_failed = frame.uplink[3];

  if(reset_failed)
    warn_output("System Reset Failed\n");
  else
    info_output("System Reset Success\n");

  return reset_failed;
}

int
tf_mini_set_update_rate(struct TF_MINI *dev, int update_rate)
{
  struct TF_MINI_FRAME frame;
  int ret;
  int reset_failed;

  frame.downlink_size = 5;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x06;
  frame.downlink[2] = 0x03;
  frame.downlink[3] = (uint8_t) (update_rate >> 8);
  frame.downlink[4] = (uint8_t) (update_rate && 0xFF00);
  frame.calc_downlink_checksum = 1;

  frame.uplink_size = 5;

  ret = tf_mini_send_command(dev, &frame);
  if(ret)
  {
    err_output("unable to set update rate %d\n", ret);
    return ret;
  }

  reset_failed = frame.uplink[4];

  if(reset_failed)
    warn_output("update rate to %d failed\n", update_rate);
  else
    info_output("update rate to %d success\n", update_rate);

  return reset_failed;
}

static int
tf_mini_valid_lidar_checksum(struct TF_MINI_LIDAR_FRAME *frame)
{
  uint16_t checksum;

  checksum  = frame->header1;
  checksum += frame->header2;

  if(checksum != (0x59+0x59))
  {
    fprintf(stderr, "nb ");
  }
  checksum += frame->distance & 0xFF;
  checksum += frame->distance >> 8;
  checksum += frame->strength & 0xFF;
  checksum += frame->strength >> 8;
  checksum += frame->temp & 0xFF;
  checksum += frame->temp >> 8;

  checksum &= 0xFF;

  if(checksum != frame->checksum)
  {
    printf("%03d != %03d ", checksum, frame->checksum);
  }

  return checksum == frame->checksum;
}

int
tf_mini_read_firmware_version(struct TF_MINI *dev)
{
  struct TF_MINI_FRAME frame;
  int ret;

  frame.downlink_size = 4;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x04;
  frame.downlink[2] = 0x01;
  frame.downlink[3] = 0x5F;
  frame.calc_downlink_checksum = 0;

  frame.uplink_size = 7;

  ret = tf_mini_send_command(dev, &frame);
  if(ret)
    return ret;

  info_output("V%d.%d.%d\n", frame.uplink[3], frame.uplink[4], frame.uplink[5]);
  return ret;
}

int
tf_mini_set_enable_output(struct TF_MINI *dev, int output)
{
  struct TF_MINI_FRAME frame;
  int ret;

  frame.downlink_size = 5;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x05;
  frame.downlink[2] = 0x07;
  frame.downlink[3] = 0x00;
  frame.downlink[4] = 0x00;
  frame.calc_downlink_checksum = 1;

  if(output)
    frame.downlink[3] = 0x01;

  frame.uplink_size = 5;

  ret = tf_mini_send_command(dev, &frame);
  if(ret)
    return ret;

  if(frame.uplink[3])
  {
    info_output("LiDAR output enabled\n");
  }
  else
  {
    info_output("LiDAR output disabled\n");
  }

  return ret;

}

int
tf_mini_set_measure_unit(struct TF_MINI *dev, int centimeters)
{
  struct TF_MINI_FRAME frame;
  int ret;

  frame.downlink_size = 5;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x05;
  frame.downlink[2] = 0x05;
  frame.calc_downlink_checksum = 1;

  if(centimeters)
    frame.downlink[3] = 0x01;
  else
    frame.downlink[3] = 0x06;

  frame.uplink_size = 5;

  ret = tf_mini_send_command(dev, &frame);
  if(ret)
    return ret;

  if(frame.uplink[3] == 0x01)
  {
    info_output("Measurement unit set to centimeters\n");
  }
  else if(frame.uplink[3] == 0x06)
  {
    info_output("Measurement unit set to millimeters\n");
  }
  else
  {
    err_output("Unknown measurement unit\n");
  }

  return ret;
}

int
tf_mini_set_baud_rate(struct TF_MINI *dev, uint32_t baudrate)
{
  struct TF_MINI_FRAME frame;
  uint32_t *baudrate_ptr;

  frame.downlink_size = 8;
  frame.downlink[0] = 0x5A;
  frame.downlink[1] = 0x08;
  frame.downlink[2] = 0x06;
  frame.calc_downlink_checksum = 1;

  baudrate_ptr = (uint32_t*) &frame.downlink[3];
  *baudrate_ptr = baudrate;

  frame.uplink_size = 8;

  return tf_mini_send_command(dev, &frame);
}

/*
int
tf_mini_restore_factory_settings(struct TF_MINI *dev);

int
tf_mini_save_settings(struct TF_MINI *dev);
*/

///////////////////

static int
tf_mini_init_uart(struct TF_MINI *dev)
{
  dev->uart_fd = uart_open();
  return dev->uart_fd;
}

int
tf_mini_init(struct TF_MINI *dev, struct options *opts)
{
  int ret;

  ret = tf_mini_init_uart(dev);
  if(ret == -1)
    return ret;

  dev->verbose = opts->verbose;

  return 0;
}

int
tf_mini_deinit(struct TF_MINI *dev, struct options* opts)
{
  int ret;
  ret = 0;

  ret = close(dev->uart_fd);

  return ret;
}

static
int
tf_mini_write_socket_udp(struct TF_MINI_LIDAR_FRAME *frame, struct options *opts)
{
  ssize_t buffrx;
  socklen_t servsock_len;
  servsock_len = sizeof(opts->socket_udp_dest);

  buffrx = sendto(opts->fd_socket_udp, frame, sizeof(struct TF_MINI_LIDAR_FRAME), 0, (struct sockaddr *)&opts->socket_udp_dest, servsock_len);

  return buffrx != sizeof(struct TF_MINI_LIDAR_FRAME);
}

int
tf_mini_poll(struct TF_MINI *dev, struct options *opts)
{
  ssize_t bytes, num_bytes_read;
  struct TF_MINI_LIDAR_FRAME lidar_frame;
  size_t lidar_frame_size = 9;
  long int good_frames = 0;
  long int bad_frames = 0;
  void *buf_ptr;

  while(1)
  {
    bytes = 0;
    num_bytes_read = 0;
    buf_ptr = &lidar_frame;
    do
    {
      bytes = read(dev->uart_fd, buf_ptr, lidar_frame_size);
      if(bytes == -1)
      {
        errno_output("reading from uart\n");
        continue;
      }
      num_bytes_read += bytes;
      buf_ptr += bytes;
    }
    while(num_bytes_read < lidar_frame_size);

    if(tf_mini_valid_lidar_checksum(&lidar_frame))
    {
      good_frames++;
    }
    else
    {
      bad_frames++;
      if(opts->discard_bad_checksum)
      {
        continue;
      }
    }

    if(opts->poll)
    {
      printf("\rDistance: %02d Strength: %02d Good Frames: %08ld Bad Frames: %08ld", lidar_frame.distance, lidar_frame.strength, good_frames, bad_frames);
      fflush(stdout);
    }

    if(opts->fd_socket_udp != -1)
    {
      if(tf_mini_write_socket_udp(&lidar_frame, opts))
      {
        err_output("error writing to udp socket\n");
      }
    }
  }

  return 0;
}
