project('nautilus-open-in-terminal', 'c',
  version: '1.0.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>= 0.60.0',
  default_options: ['warning_level=2', 'c_std=gnu11'],
)

gio = dependency('gio-2.0')
nautilus = dependency('libnautilus-extension-4')

shared_module('nautilus-open-in-terminal',
  'src/open-in-terminal.c',
  dependencies: [gio, nautilus],
  install: true,
  install_dir: nautilus.get_variable(pkgconfig: 'extensiondir'),
)

install_data('data/io.github.xlexemex.nautilus-open-in-terminal.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
)

gnome = import('gnome')
gnome.post_install(glib_compile_schemas: true)
