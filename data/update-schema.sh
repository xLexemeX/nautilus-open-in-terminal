#!/bin/sh
# Writes the Open in Terminal GSettings schema so the terminal list holds
# every installed terminal (desktop entries in the TerminalEmulator
# category), then recompiles the schemas.
# Copyright (C) 2026 xLexemeX
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

# Packaging installs into a staging directory; the schema is written on the
# target system instead.
[ -z "$DESTDIR" ] || exit 0

SCHEMA_DIR=${1:-/usr/share/glib-2.0/schemas}
SCHEMA_ID=io.github.xlexemex.nautilus-open-in-terminal
APP_DIRS="/usr/local/share/applications /usr/share/applications /var/lib/flatpak/exports/share/applications /var/lib/snapd/desktop/applications"
PREFERRED=org.gnome.Terminal

# Prints the desktop ID if the entry is a terminal that can be shown and run.
is_terminal () {
    awk -v id="$2" '
        /^\[/ { in_entry = ($0 == "[Desktop Entry]"); next }
        !in_entry { next }
        /^Type=/ { type = substr($0, 6) }
        /^Categories=/ { if ((";" substr($0, 12) ";") ~ /;TerminalEmulator;/) terminal = 1 }
        /^NoDisplay=true/ || /^Hidden=true/ { hidden = 1 }
        /^TryExec=/ { tryexec = substr($0, 9) }
        END {
            if (type != "Application" || !terminal || hidden)
                exit
            if (tryexec != "" && system("command -v \"" tryexec "\" >/dev/null 2>&1") != 0)
                exit
            print id
        }' "$1"
}

found=
for dir in $APP_DIRS; do
    [ -d "$dir" ] || continue
    for file in "$dir"/*.desktop; do
        [ -f "$file" ] || continue
        id=$(basename "$file" .desktop)
        case " $found " in *" $id "*) continue ;; esac
        found="$found $(is_terminal "$file" "$id")"
    done
done

terminals=$(printf '%s\n' $found | sort -u)
case "$terminals" in
    *"$PREFERRED"*) terminals="$PREFERRED $(printf '%s\n' $terminals | grep -vx "$PREFERRED" || true)" ;;
esac

set -- $terminals
if [ $# -eq 0 ]; then
    set -- "$PREFERRED"
fi
default=$1

{
    echo '<?xml version="1.0" encoding="UTF-8"?>'
    echo '<schemalist>'
    echo "  <enum id=\"$SCHEMA_ID.terminal\">"
    value=0
    for terminal in "$@"; do
        echo "    <value nick=\"$terminal\" value=\"$value\"/>"
        value=$((value + 1))
    done
    echo '  </enum>'
    echo "  <schema id=\"$SCHEMA_ID\" path=\"/io/github/xlexemex/nautilus-open-in-terminal/\">"
    echo "    <key name=\"terminal\" enum=\"$SCHEMA_ID.terminal\">"
    echo "      <default>'$default'</default>"
    echo '      <summary>Terminal to open</summary>'
    echo '    </key>'
    echo '  </schema>'
    echo '</schemalist>'
} > "$SCHEMA_DIR/$SCHEMA_ID.gschema.xml"

glib-compile-schemas "$SCHEMA_DIR"
