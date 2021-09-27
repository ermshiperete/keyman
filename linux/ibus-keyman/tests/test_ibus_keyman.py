#!/usr/bin/python3

import gi
import unittest

from unittest import mock

gi.require_version('Gtk', '3.0')
gi.require_version('Gdk', '3.0')
from gi.repository import Gtk, Gdk, Gio
gi.require_version('IBus', '1.0')
from gi.repository import IBus

from mock_engine import MockEngine, MockProperty, MockPropList


@unittest.skipIf(Gdk.Display.open('') is None, 'Display cannot be opened.')
class IbusKeymanTestCase(unittest.TestCase):
  '''
  Test cases for ibus-typing-booster
  '''
  engine_patcher = mock.patch.object(IBus, 'Engine', new=MockEngine)
  property_patcher = mock.patch.object(IBus, 'Property', new=MockProperty)
  prop_list_patcher = mock.patch.object(IBus, 'PropList', new=MockPropList)
  ibus_engine = IBus.Engine
  ibus_property = IBus.Property
  ibus_prop_list = IBus.PropList

  def setUp(self):
    # Patch the IBus stuff with the mock classes:
    self.engine_patcher.start()
    self.property_patcher.start()
    self.prop_list_patcher.start()
    assert IBus.Engine is not self.ibus_engine
    assert IBus.Engine is MockEngine
    assert IBus.Property is not self.ibus_property
    assert IBus.Property is MockProperty
    assert IBus.PropList is not self.ibus_prop_list
    assert IBus.PropList is MockPropList
    self.bus = IBus.Bus()

  def tearDown(self):
    # Remove the patches from the IBus stuff:
    self.engine_patcher.stop()
    self.property_patcher.stop()
    self.prop_list_patcher.stop()
    assert IBus.Engine is self.ibus_engine
    assert IBus.Engine is not MockEngine
    assert IBus.Property is self.ibus_property
    assert IBus.Property is not MockProperty
    assert IBus.PropList is self.ibus_prop_list
    assert IBus.PropList is not MockPropList

  def test_dummy(self):
    self.assertTrue(True)

  @unittest.expectedFailure
  def test_expected_failure(self):
    self.assertTrue(False)

  # def test_single_char_commit_with_space(self):
  #   self.engine.do_process_key_event(IBus.KEY_a, 0, 0)
  #   self.engine.do_process_key_event(IBus.KEY_space, 0, 0)
  #   self.assertEqual(self.engine.mock_committed_text, 'a ')
