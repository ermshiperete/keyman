node = find_program('node', required: true)

common_test_files = [
  meson.current_source_dir() / 'emscripten_filesystem.cpp',
  meson.global_source_root() / '../common/include/test_color.cpp'
]

hextobin_root = meson.global_source_root() / '../common/tools/hextobin/build/hextobin.js'
hextobin_cmd = [node, hextobin_root]

kmx = executable('km_core_keyboard_api.tests',
    ['km_core_keyboard_api.tests.cpp', common_test_files],
    cpp_args: defns + warns,
    include_directories: [inc, libsrc],
    link_args: links,
    dependencies: [icu_uc, icu_i18n],
    objects: [lib.extract_all_objects(recursive: false), kmx_test_source_lib.extract_all_objects(recursive: false)])

subdir('json')
subdir('utftest')
subdir('kmnkbd')
subdir('kmx')
subdir('ldml')
