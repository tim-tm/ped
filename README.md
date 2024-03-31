# ped

The **p**erfect **ed**itor or **p**ossibly the worst **ed**itor.
Ped is a editor made for the terminal using ncurses.

## Getting started

### Package managers

As soon as ped gets usable, it will be available in the AUR. For now it is not available via. any package manager.

### Building on your on

**Prerequisites**

- autotools
- make
- gcc
- pkg-config
- ncurses (listed in pkg-config)

Cloning the repo
```sh
git clone https://github.com/tim-tm/ped.git && cd ped
```

Make the generation script executable (only if it isn't as of right now)
```sh
chmod +x autogen.sh
```

Generating build files
```sh
./autogen.sh
```

Everything needed to build ped will be stored in ./build, autoconf will generate a bunch of files that you shouldn't care that much about.

Building
```sh
make -C build
```
or
```sh
cd build && make
```

The ped executable will be stored in ./build

Installing ped
```sh
sudo make -C build install
```
or
```sh
cd build && sudo make install
```

**Distribution**

If you want to share ped with others, consider creating a tarball.
```sh
make -C build dist
```
or
```sh
cd build && make dist
```

There will now be a tarball named **ped-VERSION.tar.gz** that can be shared with your friends.

Unzipping the tarball
```sh
tar -xf ped-VERSION.tar.gz && cd ped-VERSION
```

Configuring and building ped from the tarball
```sh
./configure && make
```

Installing
```sh
sudo make install
```

## Contributing

Steps to contribution:

1. Make a fork of ped
2. create a feature branch (optional, may be done on bigger changes tho)
3. Add **and test** your changes inside of your fork
4. Open a pull request if you're happy with your contribution

Your changes should not be extremely huge, if you really want to contribute something that may break the editor or introduce a dramatical change, please open an issue first in order to discuss your ideas.

## License

Ped is licensed under the [MIT License](https://github.com/tim-tm/ped/tree/main/LICENSE).
