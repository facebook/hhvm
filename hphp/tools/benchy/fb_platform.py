#!/usr/bin/env python
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import shlex
import subprocess

def _run_command(cmd, env=None, stdout=None):
    """Runs a command and checks the return code for errors.

    """
    cmd = shlex.split(cmd.encode('utf8'))
    subprocess.check_call(cmd, env=env, stdout=stdout)


class Platform(object):
    """Facebook-specific Platform object.

    """
    def __init__(self):
        self.name = "fb"

    def switch_to_branch(self, branch):
        """Switches the current repository to the specified branch.

        This function will always be invoked prior to building a branch.

        """
        _run_command('arc feature %s' % branch.name)

    def build_branch(self, branch):
        """Builds the specified branch.

        This function will always be invoked after switching to a branch.

        """
        build_dir = branch.build_dir()
        _run_command('fbmake clean')
        env = os.environ.copy()
        env['FBMAKE_BUILD_ROOT'] = build_dir
        _run_command('/usr/local/bin/fbmake --build-root "%s" '
                     '--ccache=off --distcc=on opt -j9000' % build_dir, env)
