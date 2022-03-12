# Documentaion

See [docs](https://lloydrochester.com/post/hardware/tf-mini-lidar-unix-agent/).

# Installation Instructions - Building from Source using the Tarball

# Build the binary
```
$ wget https://lloydrochester.com/code/tf_mini_plus_rpi-1.0.tar.gz
$ tar xf tf_mini_plus_rpi-1.1.tar.gz
$ cd tf_mini_plus_rpi-1.1/
$ ./configure
$ make
```

Install it system wide
```
$ sudo make install # sudo make uninstall to uninstall
```

# Options
```
$ tfmini --help
Usage: tfmini [OPTIONS]

A command line tool to interact with the TF Mini+.
OPTIONS:
-h, --help                       Print help
-r, --reset                      SW Reset
    --firmware-version           Read firmware version
    --measure-mm                 Set measurment units to mm
    --measure-cm                 Set measurment units to cm
    --disable-lidar-output       Disbale output of the LiDAR
    --enable-lidar-output        Enable output of the LiDAR
    --disable-lidar-output       Disbale output of the LiDAR
    --set-update-rate [1~1000Hz] Set Lidar Frame Rate
-x, --discard-bad-checksum       Discard LiDAR frames with bad checksums
-v, --verbose                    Verbose Output
-u, --socket-udp HOST:PORT     Output data to a UDP Socket
-p, --poll                     Poll the LiDAR and print to STDOUT
-d, --daemon                   Run as a Daemon

Version: 1.1
```

## Firmware Version

Here is what I have:

$ tfmini --firmware-version
LiDAR output disabled
V3.9.1

## Example Usage

```
$ tfmini --measure-cm --poll
LiDAR output disabled
Measurement unit set to centimeters
LiDAR output enabled
Distance: 04 Strength: 1640 Good Frames: 00000124 Bad Frames: 00000000
```

Or if you chose not to install system wide the binary is placed in the `src` folder.
```
./src/tfmini --help
```

# Running the Agent

After a system wide installation do a:
```
$ sudo systemctl daemon-reload
$ sudo systemctl start tfmini
$ sudo systemctl stop tfmini
```

# Building the Autotools Project

The tarball checked into this repo is the source distribution of an autotools project. It's recommended to untar it and install from the tarball because the autotools dependencies are not needed. If you want to clone the repo and build the autotools project. Do the following:

```
$ git code https://github.com/lloydroc/tfmini_agent.git
$ cd tfmini_agent
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```
