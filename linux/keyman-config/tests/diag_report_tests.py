#!/usr/bin/env python3
import os
import unittest
from unittest.mock import Mock, patch, MagicMock

from keyman_config.diag_report import DiagReport, get_diagnostic_report


class DiagReportTests(unittest.TestCase):

    def test_get_keyman_version_returns_dict(self):
        # Execute
        diag = DiagReport()
        result = diag._get_keyman_version()

        # Verify
        self.assertIsInstance(result, dict)
        self.assertIn('version', result)
        self.assertIn('version_with_tag', result)
        self.assertIn('package_version', result)
        self.assertIn('tier', result)

    @patch('subprocess.run')
    def test_get_package_version_success(self, patched_subprocess_run):
        # Setup
        patched_subprocess_run.return_value = Mock(
            stdout=b'19.0.201-1',
            returncode=0
        )

        # Execute
        diag = DiagReport()
        result = diag._get_package_version()

        # Verify
        self.assertEqual(result, '19.0.201-1')
        patched_subprocess_run.assert_called_once_with(
            ['dpkg-query', '-W', '-f', '${Version}', 'keyman'],
            capture_output=True, check=False)

    @patch('subprocess.run')
    def test_get_package_version_not_installed(self, patched_subprocess_run):
        # Setup
        patched_subprocess_run.return_value = Mock(
            stdout=b'',
            returncode=1
        )

        # Execute
        diag = DiagReport()
        result = diag._get_package_version()

        # Verify
        self.assertIsNone(result)

    @patch('subprocess.run')
    def test_get_fcitx_version_success(self, patched_subprocess_run):
        # Setup
        patched_subprocess_run.return_value = Mock(
            stdout=b'fcitx5 version: 5.0.23\n',
            returncode=0
        )

        # Execute
        diag = DiagReport()
        result = diag._get_fcitx_version()

        # Verify
        self.assertEqual(result, '5.0.23')

    @patch('subprocess.run')
    def test_get_fcitx_version_not_installed(self, patched_subprocess_run):
        # Setup
        patched_subprocess_run.side_effect = FileNotFoundError()

        # Execute
        diag = DiagReport()
        result = diag._get_fcitx_version()

        # Verify
        self.assertIsNone(result)

    def test_get_os_info_returns_dict(self):
        # Execute
        diag = DiagReport()
        result = diag._get_os_info()

        # Verify
        self.assertIsInstance(result, dict)
        self.assertIn('platform', result)
        self.assertIn('system', result)
        self.assertIn('release', result)
        self.assertIn('machine', result)

    @patch.dict(os.environ, {'XDG_SESSION_TYPE': 'wayland', 'WAYLAND_DISPLAY': 'wayland-0', 'DISPLAY': ':0'})
    def test_get_display_server(self):
        # Execute
        diag = DiagReport()
        result = diag._get_display_server()

        # Verify
        self.assertEqual(result['session_type'], 'wayland')
        self.assertEqual(result['wayland_display'], 'wayland-0')
        self.assertEqual(result['display'], ':0')

    @patch.dict(os.environ, {'XDG_CURRENT_DESKTOP': 'GNOME', 'XDG_SESSION_DESKTOP': 'gnome'})
    def test_get_desktop_environment(self):
        # Execute
        diag = DiagReport()
        result = diag._get_desktop_environment()

        # Verify
        self.assertEqual(result['xdg_current_desktop'], 'GNOME')
        self.assertEqual(result['xdg_session_desktop'], 'gnome')

    @patch.dict(os.environ, {'GTK_IM_MODULE': 'ibus', 'QT_IM_MODULE': 'ibus', 'XMODIFIERS': '@im=ibus'})
    def test_get_input_method(self):
        # Execute
        diag = DiagReport()
        result = diag._get_input_method()

        # Verify
        self.assertEqual(result['gtk_im_module'], 'ibus')
        self.assertEqual(result['qt_im_module'], 'ibus')
        self.assertEqual(result['xmodifiers'], '@im=ibus')

    @patch('keyman_config.diag_report.get_installed_kmp')
    def test_get_installed_keyboards(self, patched_get_installed_kmp):
        # Setup
        patched_get_installed_kmp.return_value = {
            'sil_euro_latin': {
                'name': 'EuroLatin',
                'kmpversion': '2.0.1'
            }
        }

        # Execute
        diag = DiagReport()
        result = diag._get_installed_keyboards()

        # Verify
        self.assertIsInstance(result, dict)
        self.assertIn('user', result)
        self.assertIn('shared', result)
        self.assertIn('os', result)

    @patch('keyman_config.diag_report.get_installed_kmp')
    def test_get_installed_keyboards_with_keyboards(self, patched_get_installed_kmp):
        # Setup - only return data for user area
        def mock_get_installed_kmp(location):
            from keyman_config.get_kmp import InstallLocation
            if location == InstallLocation.User:
                return {
                    'sil_euro_latin': {
                        'name': 'EuroLatin',
                        'kmpversion': '2.0.1'
                    }
                }
            return {}

        patched_get_installed_kmp.side_effect = mock_get_installed_kmp

        # Execute
        diag = DiagReport()
        result = diag._get_installed_keyboards()

        # Verify
        self.assertEqual(len(result['user']), 1)
        self.assertEqual(result['user'][0]['id'], 'sil_euro_latin')
        self.assertEqual(result['user'][0]['name'], 'EuroLatin')
        self.assertEqual(result['user'][0]['version'], '2.0.1')

    @patch('keyman_config.diag_report.get_ibus_version')
    @patch('keyman_config.diag_report.get_installed_kmp')
    def test_get_report_returns_string(self, patched_get_installed_kmp, patched_ibus_version):
        # Setup
        patched_ibus_version.return_value = '1.5.28'
        patched_get_installed_kmp.return_value = {}

        # Execute
        diag = DiagReport()
        result = diag._get_report()

        # Verify
        self.assertIsInstance(result, str)
        self.assertIn('Keyman Diagnostic Report', result)
        self.assertIn('Keyman Version', result)
        self.assertIn('IBus', result)
        self.assertIn('Operating System', result)
        self.assertIn('Display Server', result)

    @patch('keyman_config.diag_report.get_ibus_version')
    @patch('keyman_config.diag_report.get_installed_kmp')
    def test_get_diagnostic_report_convenience_function(self, patched_get_installed_kmp, patched_ibus_version):
        # Setup
        patched_ibus_version.return_value = '1.5.28'
        patched_get_installed_kmp.return_value = {}

        # Execute
        result = get_diagnostic_report()

        # Verify
        self.assertIsInstance(result, str)
        self.assertIn('Keyman Diagnostic Report', result)


if __name__ == '__main__':
    unittest.main()
