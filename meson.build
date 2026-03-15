project(
  'Libporter',
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

cjson_sources = files('libs/cJSON.c')
funcs_sources = files('libs/funcs.c')
github_sources = files('libs/Github.c')

executable('Libporter', 'src/main.c',
  sources: [funcs_sources, cjson_sources, github_sources],
  dependencies: [
    dependency('libcurl'),
    dependency('uuid')
  ]
)