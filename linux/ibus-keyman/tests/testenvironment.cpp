#include <filesystem>
#include <fstream>
#include <giomm/settings.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "testenvironment.hpp"

TestEnvironment::TestEnvironment(const char* currentExecutable) {
  _currentExecutable = currentExecutable;
  _ibus_pid          = 0;
  _keyman_pid        = 0;
}

void
TestEnvironment::Setup(const char* directory, int nKeyboards, char* keyboards[]) {
  // Set GSETTINGS_BACKEND to tmpfile!
  // auto inputSources = Gio::Settings::create("org.gnome.desktop.input-sources");
  // auto ibusgeneral = Gio::Settings::create("org.freedesktop.ibus.general", "/desktop/ibus/general");
  // // auto inputSources = g_settings_new("org.gnome.desktop.input-sources");
  // // auto sources = g_settings_get_value(inputSources, "sources");

  // auto newSources = std::vector<std::tuple<Glib::ustring, Glib::ustring>>();

  // for (size_t i = 0; i < nKeyboards; i++) {
  //   auto source = Glib::ustring();
  //   newSources.push_back(std::make_tuple("ibus",
  //     Glib::ustring().sprintf("en:%s/%s.kmx", directory, keyboards[i])));
  // }
  // inputSources->set_value("sources",
  //   Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>().create(newSources));
  // ibusgeneral->set_value("preload-engines",
  //   Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>().create(newSources));

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

  // LaunchIbus();
  LaunchIbusKeyman();

  // Give ibus and keyman time to start up
  sleep(1);
}

void
TestEnvironment::Restore() {
  if (_ibus_pid > 0) {
    kill(_ibus_pid, SIGTERM);
    _ibus_pid = 0;
  }
  if (_keyman_pid > 0) {
    kill(_keyman_pid, SIGTERM);
    _keyman_pid = 0;
  }
}

void
TestEnvironment::LaunchIbus() {
  _ibus_pid = fork();
  if (_ibus_pid == 0) {
    // Child process
    g_message("Launching ibus-daemon");
    const char* const args[] = {"ibus-daemon", "--panel=disable", "--config=/usr/libexec/ibus-memconf", NULL};
    if (execvp("ibus-daemon", (char* const*)args) < 0){
      g_warning("Launching ibus-daemon failed with %d: %s", errno, strerror(errno));
    }
    exit(1);
  }
}

void
TestEnvironment::LaunchIbusKeyman() {
  _keyman_pid = fork();
  if (_keyman_pid == 0) {
    // Child process
    // strip `tests/ibus-keyman-tests` from _currentExecutable
    auto parent = g_file_get_parent(g_file_get_parent(g_file_new_for_path(_currentExecutable)));
    auto ibusKeymanFile = g_file_get_child(parent, "src/ibus-engine-keyman");
    auto ibusKeyman = g_file_get_path(ibusKeymanFile);
    g_message("Launching %s", ibusKeyman);

    const char* const args[] = {"ibus-engine-keyman", NULL};
    if (execv(ibusKeyman, (char* const*)args) < 0) {
      g_warning("Launching ibus-engine-keyman failed with %d: %s", errno, strerror(errno));
    }
    exit(2);
  }
}
