#include <glib-object.h>
#include <glib.h>
#include <ibus.h>
#include <iostream>
#include <keyman/keyboardprocessor.h>
#include <linux/input-event-codes.h>
#include <memory>
#include <string>
#include <stdexcept>
#include "ibusimcontext.h"
#include "keycodes.h"
#include "kmx_test_keyboard.hpp"
#include "testenvironment.hpp"
#include "testmodule.h"

typedef struct {
  IBusBus *bus;
  GtkIMContext *context;
  IBusIMContext *ibuscontext;
} IBusKeymanTestsFixture;

static gboolean loaded        = FALSE;
static GdkWindow *window      = NULL;
static GMainLoop *thread_loop = NULL;
static GTypeModule *module    = NULL;

static void
module_register(GTypeModule *module) {
  ibus_im_context_register_type(module);
}

void
destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

static void
ibus_keyman_tests_fixture_set_up(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  IBusIMContextClass *classClass;

  if (!window) {
    GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(widget, "destroy", G_CALLBACK(destroy), NULL);
    window = gtk_widget_get_window(widget);
  }

  if (!loaded) {
    ibus_init();

    module = test_module_new(module_register);
    g_assert_nonnull(module);

    /* Not loaded until we call ref for the first time */
    classClass = static_cast<IBusIMContextClass *>(g_type_class_peek(IBUS_TYPE_IM_CONTEXT));
    g_assert_null(classClass);

    loaded = TRUE;
  }

  fixture->bus = ibus_bus_new();
}

static void
ibus_keyman_tests_fixture_tear_down(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  if (thread_loop) {
    g_main_loop_unref(thread_loop);
    thread_loop = NULL;
  }

  g_clear_object(&fixture->bus);
  g_clear_object(&fixture->context);
}

static void
switch_keyboard(IBusKeymanTestsFixture *fixture, const gchar *keyboard) {
  ibus_bus_set_global_engine(fixture->bus, keyboard);

  IBusEngineDesc *desc = ibus_bus_get_global_engine(fixture->bus);
  g_object_ref_sink(desc);
  g_message("Switched to engine: '%s'", ibus_engine_desc_get_name(desc));
  g_assert_cmpstr(keyboard, ==, ibus_engine_desc_get_name(desc));
  g_clear_object(&desc);

  if (thread_loop) {
    g_main_loop_unref(thread_loop);
    thread_loop = NULL;
  }
  if (fixture->context) {
    g_clear_object(&fixture->context);
  }
  fixture->ibuscontext = ibus_im_context_new();
  fixture->context     = (GtkIMContext *)fixture->ibuscontext;

  thread_loop = g_main_loop_new(NULL, TRUE);
  ibus_im_test_set_thread_loop(fixture->ibuscontext, thread_loop);
}

template <typename... Args>
std::string
string_format(const std::string &format, Args... args) {
  // from https://stackoverflow.com/a/26221725/487503
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
  if (size_s <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  auto buf  = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}

static unsigned short vk_to_keycode(unsigned short vk) {
  for (int i = 0; i < sizeof(keycode_to_vk); i++) {
    if (keycode_to_vk[i] == vk)
      return i;
  }
  return -1;
}

static void
test_keyboard(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  auto sourcefile = string_format("%s.kmn", (char *)user_data);
  auto kmxfile = string_format("und:%s.kmx", (char*)user_data);

  km::tests::KmxTestKeyboard test_keyboard;
  std::string keys        = "";
  std::u16string expected = u"", context = u"";
  km::tests::kmx_options options;
  bool expected_beep = false;
  test_keyboard.load_source(sourcefile.c_str(), keys, expected, context, options, expected_beep);

  switch_keyboard(fixture, kmxfile.c_str());

  auto contextStr = (gunichar2 *)context.c_str();
  ibus_im_test_set_text(fixture->ibuscontext, g_utf16_to_utf8(contextStr, context.length(), NULL, NULL, NULL));

  for (auto p = test_keyboard.next_key(keys); p.vk != 0; p = test_keyboard.next_key(keys)) {
    // Because a normal system tracks caps lock state itself,
    // we mimic that in the tests. We assume caps lock state is
    // updated on key_down before the processor receives the
    // event.
    if (p.vk == KM_KBP_VKEY_CAPS) {
      test_keyboard.toggle_caps_lock_state();
    }

    GdkEventKey keyEvent = {
        .type             = GDK_KEY_PRESS,
        .window           = window,
        .send_event       = 0,
        .time             = 0,
        .state            = (unsigned int)(p.modifier_state | test_keyboard.caps_lock_state()),
        .keyval           = p.vk,
        .length           = 0,
        .string           = NULL,
        .hardware_keycode = vk_to_keycode(p.vk),
        .group            = 0,
        .is_modifier      = 0
    };
    gtk_im_context_filter_keypress(fixture->context, &keyEvent);

    keyEvent.type  = GDK_KEY_RELEASE;
    keyEvent.state = p.modifier_state | test_keyboard.caps_lock_state();
    gtk_im_context_filter_keypress(fixture->context, &keyEvent);
  }

  auto expectedText = g_utf16_to_utf8((gunichar2*)expected.c_str(), expected.length(), NULL, NULL, NULL);
  g_assert_cmpstr(ibus_im_test_get_text(fixture->ibuscontext), ==, expectedText);
}

void
print_usage() {
  printf("Usage: %s --directory <keyboarddir> test1 [test2 ...]", g_get_prgname());
}

int
main(int argc, char *argv[]) {
  if (argc < 4) {
    print_usage();
    return 1;
  }

  if (strcmp(argv[1], "--directory") != 0) {
    print_usage();
    return 2;
  }

  char *directory = argv[2];
  int nTests      = argc - 3;
  char **tests    = &argv[3];

  gtk_init(&argc, &argv);
  g_test_init(&argc, &argv, NULL);

  auto testEnvironment = TestEnvironment();
  testEnvironment.Setup(directory, nTests, tests);

  // Add tests
  for (int i = 0; i < nTests; i++)
  {
    auto filename = tests[i];
    if (strstr(filename, ".kmx") || strstr(filename, ".kmn")) {
      filename[strlen(filename) - 4] = '\0';
    }
    auto file     = g_file_new_for_commandline_arg(filename);
    auto testfilebase = g_file_get_basename(file);
    auto testname = g_string_new(NULL);
    g_string_append_printf(testname, "/%s", testfilebase);
    auto testfile = g_file_new_build_filename(directory, testfilebase, NULL);
    g_test_add(
        testname->str, IBusKeymanTestsFixture, g_file_get_parse_name(testfile), ibus_keyman_tests_fixture_set_up, test_keyboard,
        ibus_keyman_tests_fixture_tear_down);
    g_object_unref(file);
    g_object_unref(testfile);
    g_string_free(testname, TRUE);
  }

  // Run tests
  int retVal = g_test_run();

  test_module_unuse(module);
  return retVal;
}
