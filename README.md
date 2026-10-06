# nautilus-open-in-terminal

Adds **Open in Terminal** to the main right-click menu in Nautilus, for
folders and for the background of the current folder.

## Install

Build and install the Debian package:

```sh
dpkg-buildpackage -us -uc -b
sudo apt install ../nautilus-open-in-terminal_1.0.0_amd64.deb
nautilus -q
```

Or build and install from source (requires `meson` and
`libnautilus-extension-dev`):

```sh
meson setup build --prefix=/usr
meson compile -C build
sudo meson install -C build
nautilus -q
```

## Choose the terminal

GNOME Terminal is the default. To switch to Ptyxis:

```sh
gsettings set io.github.xlexemex.nautilus-open-in-terminal terminal ptyxis
```

To switch back:

```sh
gsettings set io.github.xlexemex.nautilus-open-in-terminal terminal gnome-terminal
```

## License

Copyright (C) 2026 xLexemeX

GPL-3.0-or-later
