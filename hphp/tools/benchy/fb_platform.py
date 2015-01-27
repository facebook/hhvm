#!/usr/bin/env python
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import benchy_config as config
import benchy_utils as utils
import multiprocessing
import os
import shlex
import shutil
import subprocess


class Platform(object):
    """Facebook-specific Platform object.

    """
    def __init__(self):
        self.name = "fb"
        self.build_internal_path = os.path.join(
            config.SRCROOT_PATH[1:], '_build', 'opt', 'hphp')

    def switch_to_branch(self, branch):
        """Switches the current repository to the specified branch.

        This function will always be invoked prior to building a branch.

        """
        utils.run_command('arc feature %s' % branch.name)

    def build_branch(self, branch):
        """Builds the specified branch.

        This function will always be invoked after switching to a branch.

        """
        build_dir = branch.build_dir()
        build_link = os.path.join(config.SRCROOT_PATH, '_build')
        if os.path.islink(build_link):
            os.remove(build_link)
        else:
            shutil.rmtree(build_link, ignore_errors=True)
        env = os.environ.copy()
        env['FBMAKE_BUILD_ROOT'] = build_dir
        cpus = multiprocessing.cpu_count()
        utils.run_command('/usr/local/bin/fbmake --build-root "%s" '
                        '--distcc=off opt -j%d' % (build_dir, cpus), env=env)
