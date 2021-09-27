#!/usr/bin/python3
#
# Copyright (c) 2016-2019 Mike FABIAN <mfabian@redhat.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# based on code from https://github.com/mike-fabian/ibus-typing-booster
'''
 Define some mock classes for the unittests.
'''

from typing import Any
from gi import require_version  # type: ignore
require_version('IBus', '1.0')
from gi.repository import IBus  # type: ignore


class MockEngine:
  def __init__(self, engine_name='', connection=None, object_path=''):
    self.mock_auxiliary_text = ''
    self.mock_preedit_text = ''
    self.mock_preedit_text_cursor_pos = 0
    self.mock_preedit_text_visible = True
    self.mock_preedit_focus_mode = IBus.PreeditFocusMode.COMMIT
    self.mock_committed_text = ''
    self.mock_committed_text_cursor_pos = 0
    self.client_capabilities = (
      IBus.Capabilite.PREEDIT_TEXT |
      IBus.Capabilite.AUXILIARY_TEXT |
      IBus.Capabilite.LOOKUP_TABLE |
      IBus.Capabilite.FOCUS |
      IBus.Capabilite.PROPERTY
    )
    # There are lots of weird problems with surrounding text
    # which makes this hard to test. Therefore this mock
    # engine does not try to support surrounding text, i.e.
    # we omit “| IBus.Capabilite.SURROUNDING_TEXT” here.

  def update_auxiliary_text(self, text, visible):
    self.mock_auxiliary_text = text.text

  def commit_text(self, text):
    self.mock_committed_text = (
      self.mock_committed_text[
        :self.mock_committed_text_cursor_pos] + text.text + self.mock_committed_text[
        self.mock_committed_text_cursor_pos:])
    self.mock_committed_text_cursor_pos += len(text.text)

  def forward_key_event(self, val, code, state):
    if (val == IBus.KEY_Left and self.mock_committed_text_cursor_pos > 0):
      self.mock_committed_text_cursor_pos -= 1
      return
    unicode = IBus.keyval_to_unicode(val)
    if unicode:
      self.mock_committed_text = (
        self.mock_committed_text[
          :self.mock_committed_text_cursor_pos] + unicode + self.mock_committed_text[
          self.mock_committed_text_cursor_pos:])
      self.mock_committed_text_cursor_pos += len(unicode)

  def update_lookup_table(self, table, visible):
    pass

  def update_preedit_text(self, text, cursor_pos, visible):
    self.mock_preedit_text = text.get_text()
    self.mock_preedit_text_cursor_pos = cursor_pos
    self.mock_preedit_text_visible = visible

  def update_preedit_text_with_mode(self, text, cursor_pos, visible, focus_mode):
    self.mock_preedit_focus_mode = focus_mode
    self.update_preedit_text(text, cursor_pos, visible)

  def register_properties(self, property_list):
    pass

  def update_property(self, property):
    pass

  def hide_lookup_table(self):
    pass

  def get_surrounding_text(self):
    pass

  def connect(self, signal: str, callback_function: Any):
    pass


class MockPropList:
  def append(self, property):
    pass


class MockProperty:
  def __init__(self, *args, **kwargs):
    pass

  def set_label(self, ibus_text):
    pass

  def set_symbol(self, ibus_text):
    pass

  def set_tooltip(self, ibus_text):
    pass

  def set_sensitive(self, sensitive):
    pass

  def set_visible(self, visible):
    pass

  def set_state(self, visible):
    pass
