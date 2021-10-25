#include <filesystem>
#include <fstream>
#include <giomm/settings.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "testenvironment.hpp"

TestEnvironment::TestEnvironment() {
}

void
TestEnvironment::Setup(const char* directory, int nKeyboards, char* keyboards[]) {
  auto sources = Glib::ustring("[");
  auto preloadEngines = Glib::ustring("[");
  for (size_t i = 0; i < nKeyboards; i++) {
    sources += Glib::ustring::sprintf("('ibus', 'und:%s/%s.kmx'),", directory, keyboards[i]);
    preloadEngines += Glib::ustring::sprintf("'und:%s/%s.kmx',", directory, keyboards[i]);
  }

  sources += "]";
  preloadEngines += "]";

  auto stream = std::ofstream();
  stream.open("/tmp/keyfile", std::ofstream::out | std::ofstream::trunc);
  stream << "[org/gnome/desktop/input-sources]" << std::endl;
  stream << "sources=" << sources << std::endl;
  stream << std::endl;
  stream << "[desktop/ibus/general]" << std::endl;
  stream << "preload-engines=" << preloadEngines << std::endl;
  stream.close();
}
