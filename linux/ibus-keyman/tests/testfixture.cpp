#include <glib-object.h>
#include <glib.h>
#include <ibus.h>
#include <iostream>
#include <linux/input-event-codes.h>
#include "ibusimcontext.h"
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
  g_message("Engine: %s, %s", ibus_engine_desc_get_name(desc), ibus_engine_desc_get_longname(desc));
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

static void
test_simple(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  switch_keyboard(fixture, "en:/home/eberhard/.local/share/keyman/capslock/capslock.kmx");

  GdkEventKey keyEvent = {
      .type             = GDK_KEY_PRESS,
      .window           = window,
      .send_event       = 0,
      .time             = 0,
      .state            = 0,
      .keyval           = GDK_KEY_a,
      .length           = 0,
      .string           = NULL,
      .hardware_keycode = KEY_A,
      .group            = 0,
      .is_modifier      = 0};

  g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  keyEvent.type  = GDK_KEY_RELEASE;
  keyEvent.state = 0;
  g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  g_assert_cmpstr(ibus_im_test_get_text(fixture->ibuscontext), ==, "fail.");
}

static void
test_simple2(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  // switch_keyboard(fixture, "en:/home/eberhard/.local/share/keyman/mod_keep_context/mod_keep_context.kmx");
  switch_keyboard(fixture, "und:/home/eberhard/.local/share/keyman/test_kmx/048 - modifier keys keep context.kmx");

  GdkEventKey keyEvent = {
      .type             = GDK_KEY_PRESS,
      .window           = window,
      .send_event       = 0,
      .time             = 0,
      .state            = 0,
      .keyval           = GDK_KEY_a,
      .length           = 0,
      .string           = NULL,
      .hardware_keycode = KEY_A,
      .group            = 0,
      .is_modifier      = 0};

  g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  keyEvent.type  = GDK_KEY_RELEASE;
  keyEvent.state = 0;
  g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  keyEvent.keyval           = GDK_KEY_b;
  keyEvent.hardware_keycode = KEY_B;
  keyEvent.type             = GDK_KEY_PRESS;
  keyEvent.state            = 0;
  g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  keyEvent.type  = GDK_KEY_RELEASE;
  keyEvent.state = 0;
  g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  g_assert_cmpstr(ibus_im_test_get_text(fixture->ibuscontext), ==, "pass.");
}

static void
test_keyboard(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  auto testfile = (char *)user_data;
  auto kmxfile  = g_string_new();
  g_string_append_printf(kmxfile, "und:%s/%s.kmx", )
  // switch_keyboard(fixture, "und:/home/eberhard/.local/share/keyman/test_kmx/048 - modifier keys keep context.kmx");

  // GdkEventKey keyEvent = {
  //     .type             = GDK_KEY_PRESS,
  //     .window           = window,
  //     .send_event       = 0,
  //     .time             = 0,
  //     .state            = 0,
  //     .keyval           = GDK_KEY_a,
  //     .length           = 0,
  //     .string           = NULL,
  //     .hardware_keycode = KEY_A,
  //     .group            = 0,
  //     .is_modifier      = 0};

  // g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  // keyEvent.type  = GDK_KEY_RELEASE;
  // keyEvent.state = 0;
  // g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  // keyEvent.keyval           = GDK_KEY_b;
  // keyEvent.hardware_keycode = KEY_B;
  // keyEvent.type             = GDK_KEY_PRESS;
  // keyEvent.state            = 0;
  // g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  // keyEvent.type  = GDK_KEY_RELEASE;
  // keyEvent.state = 0;
  // g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  // g_assert_cmpstr(ibus_im_test_get_text(fixture->ibuscontext), ==, "pass.");
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
  int retVal           = 0;
  try {
    testEnvironment.Setup(directory, nTests, tests);

    // g_test_add(
    //     "/send-key", IBusKeymanTestsFixture, NULL, ibus_keyman_tests_fixture_set_up, test_simple,
    //     ibus_keyman_tests_fixture_tear_down);
    // g_test_add(
    //     "/delete-text", IBusKeymanTestsFixture, NULL, ibus_keyman_tests_fixture_set_up, test_simple2,
    //     ibus_keyman_tests_fixture_tear_down);

    for (int i = 0; i < nTests; i++)
    {
      auto filename = tests[i];
      if (strstr(filename, ".kmx") || strstr(filename, ".kmn")) {
        filename[strlen(filename) - 4] = '\0';
      }
      auto file     = g_file_new_for_commandline_arg(filename);
      auto testfile = g_file_get_basename(file);
      auto testname = g_string_new(NULL);
      g_string_append_printf(testname, "/%s", testfile);
      g_test_add(
        testname->str,
        IBusKeymanTestsFixture,
        testfile,
        ibus_keyman_tests_fixture_set_up,
        test_keyboard,
        ibus_keyman_tests_fixture_tear_down);
      g_object_unref(file);
    }

    retVal = g_test_run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  test_module_unuse(module);
  return retVal;
}