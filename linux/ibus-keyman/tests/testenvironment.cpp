#include <filesystem>
#include <giomm/settings.h>
#include <unistd.h>
#include "testenvironment.hpp"

TestEnvironment::TestEnvironment(const char* currentExecutable) {
  _currentExecutable = currentExecutable;
}

void
TestEnvironment::Setup(const char* directory, int nKeyboards, char* keyboards[]) {
  // Set GSETTINGS_BACKEND to tmpfile!
  auto inputSources = Gio::Settings::create("org.gnome.desktop.input-sources");
  // auto inputSources = g_settings_new("org.gnome.desktop.input-sources");
  // auto sources = g_settings_get_value(inputSources, "sources");

  auto newSources = std::vector<std::tuple<Glib::ustring, Glib::ustring>>();

  for (size_t i = 0; i < nKeyboards; i++) {
    auto source = Glib::ustring();
    newSources.push_back(std::make_tuple("ibus",
      Glib::ustring().sprintf("en:%s/%s.kmx", directory, keyboards[i])));
  }
  inputSources->set_value("sources",
    Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>().create(newSources));

  LaunchIbus();
  LaunchIbusKeyman();
}

void
TestEnvironment::LaunchIbus() {
  if (fork() == 0) {
    // Child process
    g_message("Launching ibus-daemon");
    const char* const args[] = {"--panel=disable", "--daemonize", NULL};
    execv("ibus-daemon", (char* const*)args);
  }
}

void
TestEnvironment::LaunchIbusKeyman() {
  if (fork() == 0) {
    // Child process
    // strip exe name and tests subfolder
    auto parent = g_file_get_parent(g_file_get_parent(g_file_new_for_path(_currentExecutable)));
    auto ibusKeymanFile = g_file_get_child(parent, "ibus-engine-keyman");
    auto ibusKeyman = g_file_get_path(ibusKeymanFile);
    g_message("Launching %s", ibusKeyman);

    const char* const args[] = {NULL};
    execv(ibusKeyman, (char* const*)args);
  }
}
