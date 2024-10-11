#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#ifdef __cplusplus
#define EXTERN extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define EXTERN EMSCRIPTEN_KEEPALIVE
#endif

#include <keyman_core.h>

namespace em = emscripten;

constexpr km_core_attr const engine_attrs = {
  256,
  KM_CORE_LIB_CURRENT,
  KM_CORE_LIB_AGE,
  KM_CORE_LIB_REVISION,
  KM_CORE_TECH_KMX,
  "SIL International"
};

EMSCRIPTEN_KEEPALIVE km_core_attr const & tmp_wasm_attributes() {
  return engine_attrs;
}

#if defined(__cplusplus)
extern "C" {
#endif

EMSCRIPTEN_KEEPALIVE km_core_status
km_core_keyboard_load_from_blob_wasm(std::string kb_name, uintptr_t blob, const unsigned int blob_size, uintptr_t keyboard) {
  return km_core_keyboard_load_from_blob(
      kb_name.c_str(),
      reinterpret_cast<void*>(blob),
      blob_size,
      reinterpret_cast<km_core_keyboard**>(keyboard));
}

#if defined(__cplusplus)
}
#endif

EMSCRIPTEN_BINDINGS(core_interface) {

  em::value_object<km_core_attr>("km_core_attr")
    .field("max_context", &km_core_attr::max_context)
    .field("current", &km_core_attr::current)
    .field("revision", &km_core_attr::revision)
    .field("age", &km_core_attr::age)
    .field("technology", &km_core_attr::technology)
    //.field("vendor", &km_core_attr::vendor, em::allow_raw_pointers())
    ;

  em::function("tmp_wasm_attributes", &tmp_wasm_attributes);

  em::enum_<km_core_status_codes>("km_core_status_codes")
      .value("OK", KM_CORE_STATUS_OK)
      .value("NO_MEM", KM_CORE_STATUS_NO_MEM)
      .value("IO_ERROR", KM_CORE_STATUS_IO_ERROR)
      .value("INVALID_ARGUMENT", KM_CORE_STATUS_INVALID_ARGUMENT)
      .value("KEY_ERROR", KM_CORE_STATUS_KEY_ERROR)
      .value("INSUFFICENT_BUFFER", KM_CORE_STATUS_INSUFFICENT_BUFFER)
      .value("INVALID_UTF", KM_CORE_STATUS_INVALID_UTF)
      .value("INVALID_KEYBOARD", KM_CORE_STATUS_INVALID_KEYBOARD)
      .value("NOT_IMPLEMENTED", KM_CORE_STATUS_NOT_IMPLEMENTED)
      .value("OS_ERROR", KM_CORE_STATUS_OS_ERROR);

  em::function("km_core_keyboard_load_from_blob", &km_core_keyboard_load_from_blob_wasm, em::allow_raw_pointers());
}
#endif
