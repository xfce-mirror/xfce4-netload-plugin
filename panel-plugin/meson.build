plugin_sources = [
  'monitor-label.c',
  'monitor-label.h',
  'net.c',
  'net.h',
  'netload.c',
  'os.h',
  'utils.c',
  'utils.h',
  'wormulon.h',
  xfce_revision_h,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_module(
  'netload',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-netload-plugin'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gtk,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    network_deps,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'netload.desktop.in',
  output: 'netload.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)

if get_option('tests')
  commandline_sources = [
    'commandline.c',
    'net.c',
    'net.h',
    'utils.c',
    'utils.h',
  ]

  commandline = executable(
    'commandline',
    commandline_sources,
    include_directories: [
      include_directories('..'),
    ],
    dependencies: [
      glib,
      libxfce4util,
    ],
    install: false,
  )

  add_test_setup(
    'default',
    exclude_suites: 'commandline',
    is_default: true,
  )
  add_test_setup(
    'full',
  )

  # To trigger this test with `meson test`, you need to pass a device to --test-args:
  # meson test --interactive --suite commandline --test-args wlp0s20f3
  test('commandline', commandline, suite: 'commandline')
endif
