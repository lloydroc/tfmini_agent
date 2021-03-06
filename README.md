# Documentaion

See [docs](https://lloydrochester.com/post/hardware/tf-mini-lidar-unix-agent/).

# Installation Instructions - Building from Source using the Tarball

Build the binary
```
$ tar xvf tf_mini_plus_rpi-1.0.tar.gz
$ cd tf_mini_plus_rpi
$ ./configure
$ make
```

Install it system wide
```
$ sudo make install # sudo make uninstall to uninstall
$ tfmini --help
$ tfmini --firmware-version
LiDAR output disabled
V3.9.1
$ tfmini --
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
