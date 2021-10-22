#!/usr/bin/python3
import logging
import os
from gi.repository import Gio

from gi.overrides.GLib import Variant
import subprocess


class GnomeKeyboardsUtil():
    def __init__(self):
        self.input_sources = Gio.Settings.new("org.gnome.desktop.input-sources")

    def read_input_sources(self):
        sourcesVal = self.input_sources.get_value("sources")
        sources = self._convert_variant_to_array(sourcesVal)
        return sources

    def write_input_sources(self, sources):
        sourcesVal = self._convert_array_to_variant(sources)
        self.input_sources.set_value("sources", sourcesVal)

    def _convert_variant_to_array(self, variant):
        if variant is None:
            return []

        values = []
        # Process variant of type "a(ss)" (array of tuples with two strings)
        nChildren = variant.n_children()
        for i in range(nChildren):
            # Process variant of type "(ss)" (tuple with two strings)
            val = variant.get_child_value(i)
            typeVariant = val.get_child_value(0)
            type = typeVariant.get_string()
            idVariant = val.get_child_value(1)
            id = idVariant.get_string()
            values.append((type, id))
        return values

    def _convert_array_to_variant(self, array):
        if len(array) == 0:
            return Variant('a(ss)', None)

        children = []
        for (type, id) in array:
            typeVariant = Variant.new_string(type)
            idVariant = Variant.new_string(id)
            child = Variant.new_tuple(typeVariant, idVariant)
            children.append(child)
        return Variant.new_array(None, children)


__is_gnome_shell = None


def is_gnome_shell():
    global __is_gnome_shell

    if __is_gnome_shell is None:
        code = os.system('pidof gnome-shell >/dev/null 2>&1')
        __is_gnome_shell = (code == 0)
    return __is_gnome_shell


def _reset_gnome_shell():
    # used in unit tests
    global __is_gnome_shell

    __is_gnome_shell = None


def get_ibus_keyboard_id(keyboard, packageDir, language=None, ignore_language=False):
    kmx_file = os.path.join(packageDir, keyboard['id'] + ".kmx")
    if ignore_language:
        return kmx_file
    if language is not None and language != '':
        logging.debug(language)
        return "%s:%s" % (language, kmx_file)
    if "languages" in keyboard and len(keyboard["languages"]) > 0:
        logging.debug(keyboard["languages"][0])
        return "%s:%s" % (keyboard["languages"][0]['id'], kmx_file)
    return kmx_file


def gnome_change_to_keyboard(bus, keyboard_index):
    # https: //askubuntu.com/a/1039964
    subprocess.run(['/usr/bin/gdbus', 'call', '--session', '--dest', 'org.gnome.Shell',
                   '--object-path', '/org/gnome/Shell', '--method', 'org.gnome.Shell.Eval',
                    '"imports.ui.status.keyboard.getInputSourceManager().inputSources[%d].activate()"'
                    % keyboard_index])
