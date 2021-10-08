#include "ibusimcontext.h"
#include <glib-object.h>
#include <glib.h>
#include <ibus.h>
#include <linux/input-event-codes.h>
#include "testmodule.h"

typedef struct {
  GTypeModule *module;
  IBusBus *bus;
  IBusEngineDesc *previousEngine;
  GtkIMContext *context;
} IBusKeymanTestsFixture;

static gboolean loaded   = FALSE;
static GdkWindow *window = NULL;

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

  ibus_init();

  fixture->module = test_module_new(module_register);
  g_assert_nonnull(fixture->module);

  /* Not loaded until we call ref for the first time */
  class = g_type_class_peek(IBUS_TYPE_IM_CONTEXT);
  g_assert_null(class);
  g_assert_false(loaded);

  fixture->bus                = ibus_bus_new();
  GDBusConnection *connection = ibus_bus_get_connection(fixture->bus);

  fixture->previousEngine = ibus_bus_get_global_engine(fixture->bus);
  ibus_bus_set_global_engine(fixture->bus, "en:/home/eberhard/.local/share/keyman/capslock/capslock.kmx");
  IBusEngineDesc *desc = ibus_bus_get_global_engine(fixture->bus);
  g_debug("Engine: %s, %s", ibus_engine_desc_get_name(desc), ibus_engine_desc_get_longname(desc));
  g_object_unref(desc);

  fixture->context = (GtkIMContext *)ibus_im_context_new();
}

static void
ibus_keyman_tests_fixture_tear_down(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  ibus_bus_set_global_engine(fixture->bus, ibus_engine_desc_get_name(fixture->previousEngine));
  g_clear_object(&fixture->previousEngine);
  g_clear_object(&fixture->bus);
  g_clear_object(&fixture->context);
  test_module_unuse(fixture->module);
}

static void
test_simple(IBusKeymanTestsFixture *fixture, gconstpointer user_data) {
  gtk_im_context_set_client_window(fixture->context, window);
  gtk_im_context_focus_in(fixture->context);

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
}

int
main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  g_test_init(&argc, &argv, NULL);

  g_test_add("/send-key", IBusKeymanTestsFixture, NULL, ibus_keyman_tests_fixture_set_up,
    test_simple, ibus_keyman_tests_fixture_tear_down);

  return g_test_run();
}
