#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "keymanutil.h"

#define TEST_FIXTURE "keymanutil-test"

void
delete_options_key(gchar* testname) {
  gchar *path = g_strdup_printf("%s%s/%s/", KEYMAN_DCONF_OPTIONS_PATH, TEST_FIXTURE, testname);
  GSettings *settings = g_settings_new_with_path(KEYMAN_DCONF_OPTIONS_CHILD_NAME, path);
  g_settings_reset(settings, KEYMAN_DCONF_OPTIONS_KEY);
  g_object_unref(G_OBJECT(settings));
  g_free(path);
}

void
set_options_key(gchar* testname, gchar** options) {
  gchar *path = g_strdup_printf("%s%s/%s/", KEYMAN_DCONF_OPTIONS_PATH, TEST_FIXTURE, testname);
  GSettings *settings = g_settings_new_with_path(KEYMAN_DCONF_OPTIONS_CHILD_NAME, path);
  g_settings_set_strv(settings, KEYMAN_DCONF_OPTIONS_KEY, (const gchar* const*)options);
  g_object_unref(G_OBJECT(settings));
  g_free(path);
}

gchar**
get_options_key(gchar* testname) {
  gchar* path = g_strdup_printf("%s%s/%s/", KEYMAN_DCONF_OPTIONS_PATH, TEST_FIXTURE, testname);
  GSettings* settings = g_settings_new_with_path(KEYMAN_DCONF_OPTIONS_CHILD_NAME, path);
  gchar** result = g_settings_get_strv(settings, KEYMAN_DCONF_OPTIONS_KEY);
  g_object_unref(G_OBJECT(settings));
  g_free(path);
  return result;
}

void
delete_kbds_key() {
  GSettings* settings = g_settings_new(KEYMAN_DCONF_ENGINE_NAME);
  g_settings_reset(settings, KEYMAN_DCONF_KEYBOARDS_KEY);
  g_object_unref(G_OBJECT(settings));
}

void
set_kbds_key(gchar** keyboards) {
  GSettings* settings = g_settings_new(KEYMAN_DCONF_ENGINE_NAME);
  g_settings_set_strv(settings, KEYMAN_DCONF_KEYBOARDS_KEY, (const gchar* const*)keyboards);
  g_object_unref(G_OBJECT(settings));
}

gchar**
get_kbds_key() {
  GSettings* settings = g_settings_new(KEYMAN_DCONF_ENGINE_NAME);
  gchar** result = g_settings_get_strv(settings, KEYMAN_DCONF_KEYBOARDS_KEY);
  g_object_unref(G_OBJECT(settings));
  return result;
}

void
test_keyman_put_options_todconf__new_key() {
  // Initialize
  gchar* testname = "test_keyman_put_options_todconf__new_key";
  delete_options_key(testname);
  gchar* value = g_strdup_printf("%d", g_test_rand_int());

  // Execute
  keyman_put_options_todconf(TEST_FIXTURE, testname, "new_key", value);

  // Verify
  gchar** options = get_options_key(testname);
  gchar* expected = g_strdup_printf("new_key=%s", value);
  g_assert_nonnull(options);
  g_assert_cmpstr(options[0], ==, expected);
  g_assert_null(options[1]);

  // Cleanup
  g_free(expected);
  g_free(value);
  g_strfreev(options);
  delete_options_key(testname);
}

void
test_keyman_put_options_todconf__other_keys() {
  // Initialize
  gchar* testname = "test_keyman_put_options_todconf__other_keys";
  delete_options_key(testname);
  gchar* existingKeys[] = {"key1=val1", "key2=val2", NULL};
  set_options_key(testname, existingKeys);
  gchar* value = g_strdup_printf("%d", g_test_rand_int());

  // Execute
  keyman_put_options_todconf(TEST_FIXTURE, testname, "new_key", value);

  // Verify
  gchar** options = get_options_key(testname);
  gchar* expected = g_strdup_printf("new_key=%s", value);
  g_assert_nonnull(options);
  g_assert_cmpstr(options[0], ==, "key1=val1");
  g_assert_cmpstr(options[1], ==, "key2=val2");
  g_assert_cmpstr(options[2], ==, expected);
  g_assert_null(options[3]);

  // Cleanup
  g_free(expected);
  g_free(value);
  g_strfreev(options);
  delete_options_key(testname);
}

void
test_keyman_put_options_todconf__existing_key() {
  // Initialize
  gchar* testname = "test_keyman_put_options_todconf__existing_key";
  delete_options_key(testname);
  gchar* existingKeys[] = {"key1=val1", "new_key=val2", NULL};
  set_options_key(testname, existingKeys);
  gchar* value = g_strdup_printf("%d", g_test_rand_int());

  // Execute
  keyman_put_options_todconf(TEST_FIXTURE, testname, "new_key", value);

  // Verify
  gchar** options = get_options_key(testname);
  gchar* expected = g_strdup_printf("new_key=%s", value);
  g_assert_nonnull(options);
  g_assert_cmpstr(options[0], ==, "key1=val1");
  g_assert_cmpstr(options[1], ==, expected);
  g_assert_null(options[2]);

  // Cleanup
  g_free(expected);
  g_free(value);
  g_strfreev(options);
  delete_options_key(testname);
}

void
test_keyman_set_custom_keyboards__new_key() {
  // Initialize
  delete_kbds_key();
  gchar* keyboards[] = {"fr:/tmp/test/test.kmx", NULL};

  // Execute
  keyman_set_custom_keyboards(keyboards);

  // Verify
  gchar** result = get_kbds_key();
  g_assert_nonnull(result);
  g_assert_cmpstrv(result, keyboards);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_set_custom_keyboards__overwrite_key() {
  // Initialize
  gchar* initialKbds[] = {"fr:/tmp/test/test.kmx", NULL};
  set_kbds_key(initialKbds);

  gchar* keyboards[] = {"fr:/tmp/test/test.kmx", "en:/tmp/foo/foo.kmx", NULL};

  // Execute
  keyman_set_custom_keyboards(keyboards);

  // Verify
  gchar** result = get_kbds_key();
  g_assert_nonnull(result);
  g_assert_cmpstrv(result, keyboards);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_set_custom_keyboards__delete_key_NULL() {
  // Initialize
  gchar* initialKbds[] = {"fr:/tmp/test/test.kmx", NULL};
  set_kbds_key(initialKbds);

  // Execute
  keyman_set_custom_keyboards(NULL);

  // Verify
  gchar** result = get_kbds_key();
  gchar** expected[] = {NULL};
  g_assert_nonnull(result);
  g_assert_cmpstrv(result, expected);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_set_custom_keyboards__delete_key_empty_array() {
  // Initialize
  gchar* initialKbds[] = {"fr:/tmp/test/test.kmx", NULL};
  set_kbds_key(initialKbds);

  gchar* keyboards[] = {NULL};

  // Execute
  keyman_set_custom_keyboards(keyboards);

  // Verify
  gchar** result     = get_kbds_key();
  g_assert_nonnull(result);
  g_assert_cmpstrv(result, keyboards);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboards__value() {
  // Initialize
  gchar* keyboards[] = {"fr:/tmp/test/test.kmx", NULL};
  set_kbds_key(keyboards);

  // Execute
  gchar** result = keyman_get_custom_keyboards();

  // Verify
  g_assert_nonnull(result);
  g_assert_cmpstrv(result, keyboards);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboards__no_key() {
  // Initialize
  delete_kbds_key();

  // Execute
  gchar** result = keyman_get_custom_keyboards();

  // Verify
  g_assert_null(result);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboards__empty() {
  // Initialize
  gchar* keyboards[] = {NULL};
  set_kbds_key(keyboards);

  // Execute
  gchar** result = keyman_get_custom_keyboards();

  // Verify
  g_assert_null(result);

  // Cleanup
  g_strfreev(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboard_dictionary__values() {
  // Initialize
  gchar* keyboards[] = {"fr:/tmp/test/test.kmx", "en:/tmp/foo/foo.kmx", "fr:/tmp/foo/foo.kmx", NULL};
  set_kbds_key(keyboards);

  // Execute
  GHashTable* result = keyman_get_custom_keyboard_dictionary();

  // Verify
  gchar* expected1[] = { "fr:/tmp/test/test.kmx", NULL};
  gchar* expected2[] = { "en:/tmp/foo/foo.kmx", "fr:/tmp/foo/foo.kmx", NULL};
  gchar* value = g_hash_table_lookup(result, "/tmp/test/test.kmx");
  g_assert_cmpstrv(value, expected1);
  value = g_hash_table_lookup(result, "/tmp/foo/foo.kmx");
  g_assert_cmpstrv(value, expected2);

  // Cleanup
  g_hash_table_destroy(result);
  g_free(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboard_dictionary__empty() {
  // Initialize
  gchar* keyboards[] = {NULL};
  set_kbds_key(keyboards);

  // Execute
  GHashTable* result = keyman_get_custom_keyboard_dictionary();

  // Verify
  g_assert_null(result);

  // Cleanup
  g_hash_table_destroy(result);
  g_free(result);
  delete_kbds_key();
}

void
test_keyman_get_custom_keyboard_dictionary__null() {
  // Initialize
  delete_kbds_key();

  // Execute
  GHashTable* result = keyman_get_custom_keyboard_dictionary();

  // Verify
  g_assert_null(result);

  // Cleanup
  g_hash_table_destroy(result);
  g_free(result);
  delete_kbds_key();
}

int
main(int argc, char* argv[]) {
  gtk_init(&argc, &argv);
  g_test_init(&argc, &argv, NULL);
  g_test_set_nonfatal_assertions();

  // Add tests
  g_test_add_func("/keymanutil/keyman_put_options_todconf/new_key", test_keyman_put_options_todconf__new_key);
  g_test_add_func("/keymanutil/keyman_put_options_todconf/other_keys", test_keyman_put_options_todconf__other_keys);
  g_test_add_func("/keymanutil/keyman_put_options_todconf/existing_key", test_keyman_put_options_todconf__existing_key);

  g_test_add_func("/keymanutil/keyman_set_custom_keyboards/new_key", test_keyman_set_custom_keyboards__new_key);
  g_test_add_func("/keymanutil/keyman_set_custom_keyboards/overwrite_key", test_keyman_set_custom_keyboards__overwrite_key);
  g_test_add_func("/keymanutil/keyman_set_custom_keyboards/delete_key_NULL", test_keyman_set_custom_keyboards__delete_key_NULL);
  g_test_add_func("/keymanutil/keyman_set_custom_keyboards/delete_key_empty_array", test_keyman_set_custom_keyboards__delete_key_empty_array);

  g_test_add_func("/keymanutil/keyman_get_custom_keyboards/value", test_keyman_get_custom_keyboards__value);
  g_test_add_func("/keymanutil/keyman_get_custom_keyboards/no_key", test_keyman_get_custom_keyboards__no_key);
  g_test_add_func("/keymanutil/keyman_get_custom_keyboards/empty", test_keyman_get_custom_keyboards__empty);

  g_test_add_func("/keymanutil/keyman_get_custom_keyboard_dictionary/values", test_keyman_get_custom_keyboard_dictionary__values);
  g_test_add_func("/keymanutil/keyman_get_custom_keyboard_dictionary/empty", test_keyman_get_custom_keyboard_dictionary__empty);
  g_test_add_func("/keymanutil/keyman_get_custom_keyboard_dictionary/null", test_keyman_get_custom_keyboard_dictionary__null);

  // Run tests
  int retVal = g_test_run();

  return retVal;
}
