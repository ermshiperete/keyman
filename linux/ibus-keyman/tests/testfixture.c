#include "ibusimcontext.h"
#include <glib-object.h>
#include <glib.h>
#include <ibus.h>
#include <linux/input-event-codes.h>
#include "testmodule.h"

typedef struct {
  IBusBus *bus;
  IBusEngineDesc *previousEngine;
  GtkIMContext *context;
  IBusIMContext *ibuscontext;
} IBusKeymanTestsFixture;

static gboolean loaded   = FALSE;
static GdkWindow *window = NULL;
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
  IBusIMContextClass *class;

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
    class = g_type_class_peek(IBUS_TYPE_IM_CONTEXT);
    g_assert_null(class);

    loaded = TRUE;
  }

  fixture->bus                = ibus_bus_new();
  GDBusConnection *connection = ibus_bus_get_connection(fixture->bus);

  fixture->previousEngine = ibus_bus_get_global_engine(fixture->bus);
  if (fixture->previousEngine)
    g_object_ref_sink(fixture->previousEngine);
}

static void
ibus_keyman_tests_fixture_tear_down(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  g_main_loop_unref(thread_loop);
  thread_loop = NULL;

  if (fixture->previousEngine)
    ibus_bus_set_global_engine(fixture->bus, ibus_engine_desc_get_name(fixture->previousEngine));
  g_clear_object(&fixture->previousEngine);
  g_clear_object(&fixture->bus);
  g_clear_object(&fixture->context);
}

static void
switch_keyboard(IBusKeymanTestsFixture *fixture, const gchar *keyboard) {
  ibus_bus_set_global_engine(fixture->bus, keyboard);
  IBusEngineDesc *desc = ibus_bus_get_global_engine(fixture->bus);
  g_object_ref_sink(desc);
  g_debug("Engine: %s, %s", ibus_engine_desc_get_name(desc), ibus_engine_desc_get_longname(desc));
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
    .is_modifier      = 0
  };

  g_assert_true(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  keyEvent.type  = GDK_KEY_RELEASE;
  keyEvent.state = 0;
  g_assert_false(gtk_im_context_filter_keypress(fixture->context, &keyEvent));

  g_assert_cmpstr(ibus_im_test_get_text(fixture->ibuscontext), ==, "fail.");
}

static void
test_simple2(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  switch_keyboard(fixture, "en:/home/eberhard/.local/share/keyman/mod_keep_context/mod_keep_context.kmx");

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

int
main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  g_test_init(&argc, &argv, NULL);

  g_test_add("/send-key", IBusKeymanTestsFixture, NULL, ibus_keyman_tests_fixture_set_up,
    test_simple, ibus_keyman_tests_fixture_tear_down);
  g_test_add("/delete-text", IBusKeymanTestsFixture, NULL, ibus_keyman_tests_fixture_set_up,
    test_simple2, ibus_keyman_tests_fixture_tear_down);

  int retVal = g_test_run();
  test_module_unuse(module);
  return retVal;
}
