#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <type_traits>

#include <kmx/kmx_processevent.h>
#include <kmx/kmx_xstring.h>

#include "path.hpp"
#include "state.hpp"
#include "utfcodec.hpp"

#include "kmx_test_keyboard.hpp"

namespace km {
namespace tests {

KmxTestKeyboard::KmxTestKeyboard() {
}

// String trim functions from https://stackoverflow.com/a/217605/1836776
// trim from start (in place)
static inline void
ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void
rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
static inline void
trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

std::u16string
KmxTestKeyboard::parse_source_string(std::string const &s) {
  std::u16string t;
  for (auto p = s.begin(); p != s.end(); p++) {
    if (*p == '\\') {
      p++;
      km_kbp_usv v;
      assert(p != s.end());
      if (*p == 'u' || *p == 'U') {
        // Unicode value
        p++;
        size_t n;
        std::string s1 = s.substr(p - s.begin(), 8);
        v              = std::stoul(s1, &n, 16);
        // Allow deadkey_number (U+0001) characters and onward
        assert(v >= 0x0001 && v <= 0x10FFFF);
        p += n - 1;
        if (v < 0x10000) {
          t += km_kbp_cp(v);
        } else {
          t += km_kbp_cp(Uni_UTF32ToSurrogate1(v));
          t += km_kbp_cp(Uni_UTF32ToSurrogate2(v));
        }
      } else if (*p == 'd') {
        // Deadkey
        // TODO, not yet supported
        assert(false);
      }
    } else {
      t += *p;
    }
  }
  return t;
}

bool
KmxTestKeyboard::parse_option_string(std::string line, kmx_options &options, kmx_option_type type) {
  auto x = line.find('=');
  if (x == std::string::npos)
    return false;

  kmx_option o;

  o.type  = type;
  o.key   = parse_source_string(line.substr(0, x));
  o.value = parse_source_string(line.substr(x + 1));

  options.emplace_back(o);
  return true;
}

bool
KmxTestKeyboard::is_token(const std::string token, std::string &line) {
  if (line.compare(0, token.length(), token) == 0) {
    line = line.substr(token.length());
    trim(line);
    return true;
  }
  return false;
}

int
KmxTestKeyboard::load_source(
    const km::kbp::path &path,
    std::string &keys,
    std::u16string &expected,
    std::u16string &context,
    kmx_options &options,
    bool &expected_beep) {
  const std::string s_keys = "c keys: ";
  const std::string s_expected = "c expected: ";
  const std::string s_context = "c context: ";
  const std::string s_option = "c option: ";
  const std::string s_option_expected = "c expected option: ";
  const std::string s_option_saved = "c saved option: ";
  const std::string s_capsLock = "c capsLock: ";

  // Parse out the header statements in file.kmn that tell us (a) environment, (b) key sequence, (c) start context, (d) expected
  // result
  std::ifstream kmn(path.native());
  if (!kmn.good()) {
    std::cerr << "could not open file: " << path << std::endl;
    return __LINE__;
  }
  std::string line;
  while (std::getline(kmn, line)) {
    trim(line);

    if (!line.length())
      continue;
    if (line.compare(0, s_keys.length(), s_keys) == 0) {
      keys = line.substr(s_keys.length());
      trim(keys);
    } else if (is_token(s_expected, line)) {
      if (line == "\\b") {
        expected_beep = true;
      } else {
        expected = parse_source_string(line);
      }
    } else if (is_token(s_context, line)) {
      context = parse_source_string(line);
    } else if (is_token(s_option, line)) {
      if (!parse_option_string(line, options, KOT_INPUT))
        return __LINE__;
    } else if (is_token(s_option_expected, line)) {
      if (!parse_option_string(line, options, KOT_OUTPUT))
        return __LINE__;
    } else if (is_token(s_option_saved, line)) {
      if (!parse_option_string(line, options, KOT_SAVED))
        return __LINE__;
    } else if (is_token(s_capsLock, line)) {
      _caps_lock_on = parse_source_string(line).compare(u"1") == 0;
    }
  }

  if (keys == "") {
    // We must at least have a key sequence to run the test
    return __LINE__;
  }

  return 0;
}

km_kbp_option_item *
KmxTestKeyboard::get_keyboard_options(kmx_options options) {
  km_kbp_option_item *keyboard_opts = new km_kbp_option_item[options.size() + 1];

  int i = 0;
  for (auto it = options.begin(); it != options.end(); it++) {
    if (it->type != KOT_INPUT)
      continue;

    std::cout << "input option-key: " << it->key << std::endl;

    std::u16string key = it->key;
    if (key[0] == u'&') {
      // environment value (aka system store)
      key.erase(0, 1);
      keyboard_opts[i].scope = KM_KBP_OPT_ENVIRONMENT;
    } else {
      keyboard_opts[i].scope = KM_KBP_OPT_KEYBOARD;
    }

    km_kbp_cp *cp = new km_kbp_cp[key.length() + 1];
    key.copy(cp, key.length());
    cp[key.length()] = 0;

    keyboard_opts[i].key = cp;

    cp = new km_kbp_cp[it->value.length() + 1];
    it->value.copy(cp, it->value.length());
    cp[it->value.length()] = 0;

    keyboard_opts[i].value = cp;

    i++;
  }

  keyboard_opts[i] = KM_KBP_OPTIONS_END;
  return keyboard_opts;
}

key_event
KmxTestKeyboard::char_to_event(char ch) {
  assert(ch >= 32 && ch <= 127);
  return {
      km::kbp::kmx::s_char_to_vkey[(int)ch - 32].vk,
      (uint16_t)(km::kbp::kmx::s_char_to_vkey[(int)ch - 32].shifted ? KM_KBP_MODIFIER_SHIFT : 0)};
}

uint16_t
KmxTestKeyboard::get_modifier(std::string const m) {
  for (int i = 0; km::kbp::kmx::s_modifier_names[i].name; i++) {
    if (m == km::kbp::kmx::s_modifier_names[i].name) {
      return km::kbp::kmx::s_modifier_names[i].modifier;
    }
  }
  return 0;
}

km_kbp_virtual_key
KmxTestKeyboard::get_vk(std::string const &vk) {
  for (int i = 1; i < 256; i++) {
    if (vk == km::kbp::kmx::s_key_names[i]) {
      return i;
    }
  }
  return 0;
}

key_event
KmxTestKeyboard::vkey_to_event(std::string const &vk_event) {
  // vkey format is MODIFIER MODIFIER K_NAME
  // std::cout << "VK=" << vk_event << std::endl;

  std::stringstream f(vk_event);
  std::string s;
  uint16_t modifier_state = 0;
  km_kbp_virtual_key vk   = 0;
  while (std::getline(f, s, ' ')) {
    uint16_t modifier = get_modifier(s);
    if (modifier != 0) {
      modifier_state |= modifier;
    } else {
      vk = get_vk(s);
      break;
    }
  }

  // The string should be empty at this point
  assert(!std::getline(f, s, ' '));
  assert(vk != 0);

  return {vk, modifier_state};
}

key_event
KmxTestKeyboard::next_key(std::string &keys) {
  // Parse the next element of the string, chop it off, and return it
  if (keys.length() == 0)
    return {0, 0};
  char ch = keys[0];
  if (ch == '[') {
    if (keys.length() > 1 && keys[1] == '[') {
      keys.erase(0, 2);
      return char_to_event(ch);
    }
    auto n = keys.find(']');
    assert(n != std::string::npos);
    auto vkey = keys.substr(1, n - 1);
    keys.erase(0, n + 1);
    return vkey_to_event(vkey);
  } else {
    keys.erase(0, 1);
    return char_to_event(ch);
  }
}

}  // namespace tests
}  // namespace km