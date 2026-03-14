project(
  'binary',
  'c',
  default_options: [
    'c_std=c23',
    'warning_level=3',
    'b_sanitize=address,undefined',
    'buildtype=debug'
  ]
)

add_project_arguments(
  [
    '-Wpedantic',
    '-Wconversion',
    '-Wsign-conversion',
    '-Wshadow',
    '-Wformat=2',
    '-Wundef',
    '-Wstrict-prototypes'
  ],
  language: 'c'
)

executable('binary', 'src/main.c')