#!/usr/bin/env python
"""Provides a layer of indirection for loading singleton Platform objects.

Dynamically loads a singleton Platform object using the config.PLATFORM
setting in .benchy.

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import benchy_config as config

def platform():
    """Returns a singleton Platform object.

    Lazily instantiates a singleton Platform object based on the
    config.PLATFORM settings.

    """
    if platform._singleton is not None:
        return platform._singleton
    module = __import__(config.PLATFORM)
    platform._singleton = module.Platform()
    return platform._singleton
platform._singleton = None
