// Copyright (c) 2024 SIL International
// This software is licensed under the MIT license (http://opensource.org/licenses/MIT)

#include <keyman/keyman_core_api.h>
#include <test_assert.h>
#include <test_color.h>

void test_load_from_file() {
  std::cout << __FUNCTION__ << std::endl;
  km_core_keyboard* keyboard = NULL;
  try_status(km_core_keyboard_load("kmx/k_020___deadkeys_and_backspace.kmx", &keyboard));
  assert(keyboard != NULL);
}

void test_load_from_blob() {
  std::cout << __FUNCTION__ << std::endl;
  km_core_keyboard* keyboard = NULL;
  FILE* fp;
  fp = fopen("kmx/k_020___deadkeys_and_backspace.kmx", "rb");
  fseek(fp, 0, SEEK_END);
  auto length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  KMX_BYTE* buf = new KMX_BYTE[length];
  long nRead = fread(buf, 1, length, fp);
  assert(nRead == length);
  fclose(fp);

  try_status(km_core_keyboard_load_from_blob(buf, &keyboard));
  assert(keyboard != NULL);
  delete[] buf;
}

int main(int argc, char* argv[]) {
  int first_arg = 1;

  auto arg_color = argc > 1 ? std::string(argv[1]) == "--color" : false;
  if (arg_color) {
    first_arg++;
    if (argc < 2) {
      std::cerr << "Not enough arguments." << std::endl;
      return 1;
    }
  }
  console_color::enabled = console_color::isaterminal() || arg_color;

  test_load_from_file();
  test_load_from_blob();

  return 0;
}
