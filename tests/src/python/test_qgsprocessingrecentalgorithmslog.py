# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProcessingRecentAlgorithmLog.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-07'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '176c06ceefb5f555205e72b20c962740cc0ec183'

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsSettings
from qgis.gui import QgsProcessingRecentAlgorithmLog, QgsGui
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsProcessingRecentAlgorithmLog(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsNewGeoPackageLayerDialog.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsNewGeoPackageLayerDialog")
        QgsSettings().clear()

    def test_log(self):
        log = QgsProcessingRecentAlgorithmLog()
        self.assertFalse(log.recentAlgorithmIds())
        spy = QSignalSpy(log.changed)

        log.push('test')
        self.assertEqual(log.recentAlgorithmIds(), ['test'])
        self.assertEqual(len(spy), 1)
        log.push('test')
        self.assertEqual(log.recentAlgorithmIds(), ['test'])
        self.assertEqual(len(spy), 1)

        log.push('test2')
        self.assertEqual(log.recentAlgorithmIds(), ['test2', 'test'])
        self.assertEqual(len(spy), 2)

        log.push('test')
        self.assertEqual(log.recentAlgorithmIds(), ['test', 'test2'])
        self.assertEqual(len(spy), 3)

        log.push('test3')
        self.assertEqual(log.recentAlgorithmIds(), ['test3', 'test', 'test2'])
        self.assertEqual(len(spy), 4)

        log.push('test4')
        self.assertEqual(log.recentAlgorithmIds(), ['test4', 'test3', 'test', 'test2'])
        self.assertEqual(len(spy), 5)

        log.push('test5')
        self.assertEqual(log.recentAlgorithmIds(), ['test5', 'test4', 'test3', 'test', 'test2'])
        self.assertEqual(len(spy), 6)

        log.push('test6')
        self.assertEqual(log.recentAlgorithmIds(), ['test6', 'test5', 'test4', 'test3', 'test'])
        self.assertEqual(len(spy), 7)

        log.push('test3')
        self.assertEqual(log.recentAlgorithmIds(), ['test3', 'test6', 'test5', 'test4', 'test'])
        self.assertEqual(len(spy), 8)

        log.push('test3')
        self.assertEqual(log.recentAlgorithmIds(), ['test3', 'test6', 'test5', 'test4', 'test'])
        self.assertEqual(len(spy), 8)

        # test that log has been saved to QgsSettings
        log2 = QgsProcessingRecentAlgorithmLog()
        self.assertEqual(log2.recentAlgorithmIds(), ['test3', 'test6', 'test5', 'test4', 'test'])

    def test_gui_instance(self):
        self.assertIsNotNone(QgsGui.instance().processingRecentAlgorithmLog())


if __name__ == '__main__':
    unittest.main()