#!/usr/bin/python3

import glob
import os

from gi import require_version
require_version('IBus', '1.0')
from gi.repository import IBus

IBUS_CAP_PREEDIT_TEXT = 1 << 0
IBUS_CAP_AUXILIARY_TEXT = 1 << 1
IBUS_CAP_LOOKUP_TABLE = 1 << 2
IBUS_CAP_FOCUS = 1 << 3
IBUS_CAP_PROPERTY = 1 << 4
IBUS_CAP_SURROUNDING_TEXT = 1 << 5


class MyInputContext(IBus.InputContext):
  def __init__(self, bus):
    self.__bus = bus
    self.__path = bus.create_input_context("MyInputContext")
    super(MyInputContext, self).__init__()
    # self.input_context = super(MyInputContext, self).get_input_context("MyInputContext", self.__bus.get_connection())

    self.id_no = 0
    self.preediting = False
    self.lookup_table = None

    self.connect('commit-text', commit_text_cb)
    self.connect('update-preedit-text', update_preedit_text_cb)
    self.connect('show-preedit-text', show_preedit_text_cb)
    self.connect('hide-preedit-text', hide_preedit_text_cb)
    self.connect('update-auxiliary-text', update_auxiliary_text_cb)
    self.connect('show-auxiliary-text', show_auxiliary_text_cb)
    self.connect('hide-auxiliary-text', hide_auxiliary_text_cb)
    self.connect('update-lookup-table', update_lookup_table_cb)
    self.connect('show-lookup-table', show_lookup_table_cb)
    self.connect('hide-lookup-table', hide_lookup_table_cb)
    self.connect('page-up-lookup-table', page_up_lookup_table_cb)
    self.connect('page-down-lookup-table', page_down_lookup_table_cb)
    self.connect('cursor-up-lookup-table', cursor_up_lookup_table_cb)
    self.connect('cursor-down-lookup-table', cursor_down_lookup_table_cb)
    self.connect('enabled', enabled_cb)
    self.connect('disabled', disabled_cb)
    try:
        self.connect('forward-key-event', forward_key_event_cb)
    except TypeError:
        pass
    try:
        self.connect('delete-surrounding-text', delete_surrounding_text_cb)
    except TypeError:
        pass

########################################################################
# Callbacks
########################################################################


def commit_text_cb(ic, text):
  print('(ibus-commit-text-cb %d "%s")' % (ic.id_no, text.text))


def update_preedit_text_cb(ic, text, cursor_pos, visible):
  preediting = len(text.text) > 0
  if preediting or ic.preediting:
    attrs = ['%s %d %d %d' %
             (["nil", "'underline", "'foreground", "'background"][attr.type],
              attr.value & 0xffffff, attr.start_index, attr.end_index)
             for attr in text.attributes]
    print('(ibus-update-preedit-text-cb %d "%s" %d %s %s)' %
          (ic.id_no, text.text, cursor_pos, visible, ' '.join(attrs)))
  ic.preediting = preediting


def show_preedit_text_cb(ic):
    print('(ibus-show-preedit-text-cb %d)' % (ic.id_no))


def hide_preedit_text_cb(ic):
    print('(ibus-hide-preedit-text-cb %d)' % (ic.id_no))


def update_auxiliary_text_cb(ic, text, visible):
    print('(ibus-update-auxiliary-text-cb %d "%s" %s)' %
          (ic.id_no, text.text, visible))


def show_auxiliary_text_cb(ic):
    print('(ibus-show-auxiliary-text-cb %d)' % (ic.id_no))


def hide_auxiliary_text_cb(ic):
    print('(ibus-hide-auxiliary-text-cb %d)' % (ic.id_no))


def update_lookup_table_cb(ic, lookup_table, visible):
    ic.lookup_table = lookup_table
    print("ibus-update-lookup-table-cb %d" % visible)


def show_lookup_table_cb(ic):
    print("(ibus-show-lookup-table-cb %d '(%s) %s)" %
          (ic.id_no, " ".join(map(lambda item: '"%s"' % item.text,
                                  ic.lookup_table.get_candidates_in_current_page())),
           ic.lookup_table.get_cursor_pos_in_current_page()))


def hide_lookup_table_cb(ic):
    print('(ibus-hide-lookup-table-cb %d)' % (ic.id_no))


def page_up_lookup_table_cb(ic):
    print('(ibus-log "page up lookup table")')


def page_down_lookup_table_cb(ic):
    print('(ibus-log "page down lookup table")')


def cursor_up_lookup_table_cb(ic):
    print('(ibus-log "cursor up lookup table")')


def cursor_down_lookup_table_cb(ic):
    print('(ibus-log "cursor down lookup table")')


def enabled_cb(ic):
    print('(ibus-status-changed-cb %d "%s")' % (ic.id_no, ic.get_engine().name))


def disabled_cb(ic):
    print('(ibus-status-changed-cb %d nil)' % ic.id_no)


def forward_key_event_cb(ic, keyval, keycode, modifiers):
    print('(ibus-forward-key-event-cb %d %d %d)' %
          (ic.id_no, keyval, modifiers))


def delete_surrounding_text_cb(ic, offset, n_chars):
    print('(ibus-delete-surrounding-text-cb %d %d %d)' %
          (ic.id_no, offset, n_chars))


def _message_filter(connection, message, incoming, user_data):
  if incoming:
    direction = "Received"
  else:
    direction = "Send"
  print("%s message: serial=%d, sender=%s, interface=%s, path=%s, member=%s, reply=%d" % (
    direction, message.get_serial(), message.get_sender(), message.get_interface(),
    message.get_path(), message.get_member(), message.get_reply_serial()))
  message.print_(2)
  return message


def _commit_text(connection, sender_name, object_path, interface_name, signal_name, parameters, user_data):
  print("got signal from %s, path=%s, iface=%s, signal=%s" % (sender_name, object_path, interface_name, signal_name))


class MockClient:
  def __init__(self):
    self._set_ibus_address()
    self._bus = IBus.Bus()
    if not self._bus.is_connected():
      print("Can not connect to ibus-daemon")
      exit(1)

    print(self._bus.hello())
    self._connection = self._bus.get_connection()
    val = self._bus.add_match("type='signal'")
    if not val:
      print("Can't add match")

    # self._connection.add_filter(_message_filter, None)
    # self._connection.signal_subscribe(None, "org.freedesktop.IBus.InputContext", "CommitText", None, None,
    self._connection.signal_subscribe(None, None, None, None, None,
                                      0, _commit_text, None)
    # self._context = MyInputContext(self._bus)
    self._context = self._bus.create_input_context("MyTestingInputContext")
    self._context.set_capabilities(IBUS_CAP_FOCUS | IBUS_CAP_PREEDIT_TEXT |
                                   IBUS_CAP_AUXILIARY_TEXT | IBUS_CAP_LOOKUP_TABLE |
                                   IBUS_CAP_PROPERTY | IBUS_CAP_SURROUNDING_TEXT)
    # self._context.connect("commit-text", self.__commit_text_cb)
    # self._context.connect("cursor-down-lookup-table", self.__cursor_down_lookup_table_cb)
    # self._context.connect("cursor-up-lookup-table", self.__cursor_up_lookup_table_cb)
    # self._context.connect("delete-surrounding-text", self.__delete_surrounding_text_cb)
    # self._context.connect("disabled", self.__disabled_cb)
    # self._context.connect("enabled", self.__enabled_cb)
    # self._context.connect("forward-key-event", self.__forward_key_event_cb)
    # self._context.connect("hide-auxiliary-text", self.__hide_auxiliary_text_cb)
    # self._context.connect("hide-lookup-table", self.__hide_lookup_table_cb)
    # self._context.connect("hide-preedit-text", self.__hide_preedit_text_cb)
    # self._context.connect("page-down-lookup-table", self.__page_down_lookup_table_cb)
    # self._context.connect("page-up-lookup-table", self.__page_up_lookup_table_cb)
    # self._context.connect("register-properties", self.__register_properties_cb)
    # self._context.connect("show-auxiliary-text", self.__show_auxiliary_text_cb)
    # self._context.connect("show-lookup-table", self.__show_lookup_table_cb)
    # self._context.connect("show-preedit-text", self.__show_preedit_text_cb)
    # self._context.connect("update-auxiliary-text", self.__update_auxiliary_text_cb)
    # self._context.connect("update-lookup-table", self.__update_lookup_table_cb)
    # self._context.connect("update-preedit-text", self.__update_preedit_text_cb)
    # self._context.connect("update-property", self.__update_property_cb)
    print("end of init")

  def __commit_text_cb(context, text):
    print("commit-text: %s" % text.text)

  def __cursor_down_lookup_table_cb(self, context):
    print("cursor_down_lookup_table_cb")

  def __cursor_up_lookup_table_cb(self, context):
    print("cursor_up_lookup_table_cb")

  def __delete_surrounding_text_cb(self, context, n_chars):
    print("delete_surrounding_text_cb: %d" % n_chars)

  def __disabled_cb(self, context):
    print("disabled_cb")

  def __enabled_cb(self, context):
    print("enabled_cb")

  def __forward_key_event_cb(self, context, keyval, keycode, modifiers):
    print("forward_key_event_cb: keyval=%d, keycode=%d, modifiers=%d" % (keyval, keycode, modifiers))

  def __hide_auxiliary_text_cb(self, context):
    print("hide_auxiliary_text_cb")

  def __hide_lookup_table_cb(self, context):
    print("hide_lookup_table_cb")

  def __hide_preedit_text_cb(self, context):
    print("hide_preedit_text_cb")

  def __page_down_lookup_table_cb(self, context):
    print("page_down_lookup_table_cb")

  def __page_up_lookup_table_cb(self, context):
    print("page_up_lookup_table_cb")

  def __register_properties_cb(self, context, properties):
    print("register_properties_cb")

  def __show_auxiliary_text_cb(self, context):
    print("show_auxiliary_text_cb")

  def __show_lookup_table_cb(self, context):
    print("show_lookup_table_cb")

  def __show_preedit_text_cb(self, context):
    print("show_preedit_text_cb")

  def __update_auxiliary_text_cb(self, context, text, arg2):
    print("update_auxiliary_text_cb: %s (%d)" % (text.text, arg2))

  def __update_lookup_table_cb(self, context, table, visible):
    print("update_lookup_table_cb: visible=%d" % visible)

  def __update_preedit_text_cb(self, context, text, cursor_pos, visible):
    print("update_preedit_text_cb: %s at %d (visible=%d)" % (text.text, cursor_pos, visible))

  def __update_property_cb(self, context, prop):
    print("update_property_cb")

  def type(self, keycode, modifiers):
    self._context.process_key_event(keycode, keycode, modifiers)

  def set_global_engine(self, name):
    self._context.set_engine(name)
    return self._bus.set_global_engine(name)

  def get_global_engine(self):
    engine = self._bus.get_global_engine()
    return engine.get_name()

  def focus_in(self):
    self._context.focus_in()

  def focus_out(self):
    self._context.focus_out()

  # dynamically set IBUS_ADDRESS based on config file for $DISPLAY
  def _set_ibus_address(self):
    address = self._read_ibus_address()
    os.environ["IBUS_ADDRESS"] = address

  def _read_ibus_address(self):
    config_home = os.getenv("XDG_CONFIG_HOME", os.path.join(os.getenv("HOME", ""), ".config"))

    display = os.environ.get("DISPLAY")
    if display:
      if display[0] == ":":
        display = display[1:]
      else:
        print("Unexpected value for DISPLAY: %s" % display)
        exit(1)
    else:

      display = "0"

    pattern = os.path.join(config_home, "ibus", "bus", ("*-%s" % display))
    files = glob.glob(pattern)
    if not files or len(files) < 1:
      print("No files matching %s" % pattern)
      exit(2)
    if len(files) > 1:
      print("More than one file matching %s" % pattern)
      exit(3)
    with open(files[0]) as f:
      while True:
        line = f.readline()
        if not line:
          break
        if line.startswith("IBUS_ADDRESS"):
          index = line.find("=")
          if index > -1:
            return line[index + 1:]
    print("IBUS_ADDRESS not found")
    exit(4)


if __name__ == '__main__':
  client = MockClient()
  client.focus_in()
  client.set_global_engine("am:/home/eberhard/.local/share/keyman/gff_amharic/gff_amharic.kmx")
  # print(client.get_global_engine())
  client.type(0x1e, 0)
  print("final end")
