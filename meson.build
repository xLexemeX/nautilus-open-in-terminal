project('nautilus-open-in-terminal', 'c',
  version: '1.1.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>= 0.60.0',
  default_options: ['warning_level=2', 'c_std=gnu11'],
)

gio = dependency('gio-unix-2.0')
nautilus = dependency('libnautilus-extension-4')

shared_module('nautilus-open-in-terminal',
  'src/open-in-terminal.c',
  dependencies: [gio, nautilus],
  install: true,
  install_dir: nautilus.get_variable(pkgconfig: 'extensiondir'),
)

install_data('data/update-schema.sh',
  install_dir: get_option('libexecdir') / meson.project_name(),
  install_mode: 'rwxr-xr-x',
)

meson.add_install_script('data/update-schema.sh',
  get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas',
)
