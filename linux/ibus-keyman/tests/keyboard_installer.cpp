#include "keyboard_installer.hpp"

KeyboardInstaller::KeyboardInstaller() {

}

void KeyboardInstaller::Install(char* directory, int nKeyboards, char* keyboards[]) {
  // Set GSETTINGS_BACKEND to memory!
  auto inputSources = Gio::Settings::create("org.gnome.desktop.input-sources");
  inputSources->get_value("sources", _originalSource);
  // auto inputSources = g_settings_new("org.gnome.desktop.input-sources");
  // auto sources = g_settings_get_value(inputSources, "sources");

  auto newSources = ConvertVariantToVector(_originalSource);

  for (size_t i = 0; i < nKeyboards; i++) {
    auto source = Glib::ustring();
    newSources.push_back(source.sprintf("en:%s/%s", directory, keyboards[i]));
  }
  inputSources->set_value("sources", ConvertVectorToVariant(newSources));
}

void KeyboardInstaller::Restore() {
  auto inputSources = Gio::Settings::create("org.gnome.desktop.input-sources");
  inputSources->set_value("sources", _originalSource);
}

std::vector<Glib::ustring>
KeyboardInstaller::ConvertVariantToVector(Glib::VariantBase& variant) {
  auto sourcesVector =
      Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>>(variant);
  auto size = sourcesVector.get_n_children();

  auto result = std::vector<Glib::ustring>();
  for (int i = 0; i < size; i++) {
    auto tuple = sourcesVector.get_child(i);
    result.push_back(std::get<1>(tuple));
  }
  return result;
}

Glib::VariantBase
KeyboardInstaller::ConvertVectorToVariant(std::vector<Glib::ustring>& vector) {
  if (!vector.size()) {
    return Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>(NULL);
  }

  auto array = std::vector<std::tuple<Glib::ustring, Glib::ustring>>();

  for (auto var : vector) {
    array.push_back(std::make_tuple("ibus", var));
  }

  return Glib::Variant<std::vector<std::tuple<Glib::ustring, Glib::ustring>>>().create(array);
}
