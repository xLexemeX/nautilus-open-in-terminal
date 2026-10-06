# nautilus-open-in-terminal

Adds **Open in Terminal** to the main right-click menu in Nautilus, for
folders and for the background of the current folder.

## Install

Download the `.deb` from [Releases](https://github.com/xLexemeX/nautilus-open-in-terminal/releases) and install it:

```sh
sudo apt install ./nautilus-open-in-terminal_1.1.0_amd64.deb
nautilus -q
```

Or build and install the Debian package yourself:

```sh
dpkg-buildpackage -us -uc -b
sudo apt install ../nautilus-open-in-terminal_1.1.0_amd64.deb
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

When installed from source, rerun
`sudo /usr/libexec/nautilus-open-in-terminal/update-schema.sh` after
installing or removing a terminal.

## Choose the terminal

Every installed terminal is offered, and terminals installed later are
added automatically. GNOME Terminal is the default when it's installed.

Pick one in dconf Editor under
`/io/github/xlexemex/nautilus-open-in-terminal/terminal`, or from the
command line using the terminal's desktop ID:

```sh
gsettings set io.github.xlexemex.nautilus-open-in-terminal terminal org.gnome.Ptyxis
```

## License

Copyright (C) 2026 xLexemeX

GPL-3.0-or-later
