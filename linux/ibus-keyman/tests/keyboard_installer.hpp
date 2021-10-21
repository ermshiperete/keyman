#include <giomm/settings.h>

class KeyboardInstaller
{
  Glib::VariantBase _originalSource;

  std::vector<Glib::ustring> ConvertVariantToVector(Glib::VariantBase& variant);
  Glib::VariantBase ConvertVectorToVariant(std::vector<Glib::ustring>& vector);

public:
  KeyboardInstaller();
  void Install(char* directory, int nKeyboards, char* keyboards[]);
  void Restore();
};
