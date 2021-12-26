# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_extrude.py
    ------------
    Date                 : March 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '176c06ceefb5f555205e72b20c962740cc0ec183'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    height = alg.parameterAsDouble(parameters, 'height', context)
    height_column = alg.parameterAsString(parameters, 'height_column', context)
    if (height and height_column) or (not height and not height_column):
        return False, alg.tr("You need to set either a fixed height value or the height column!")

    return True, None