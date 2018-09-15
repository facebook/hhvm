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
import subprocess


def _run_command(cmd, env=None, stdout=None):
    """Runs a command and checks the return code for errors.

    """
    cmd = shlex.split(cmd.encode('utf8'))
    subprocess.check_call(cmd, env=env, stdout=stdout)


class Platform(object):
    """Open source Platform object that uses git and cmake.

    """
    def __init__(self):
        self.name = "oss"
        self.build_internal_path = 'hphp'

    def switch_to_branch(self, branch):
        """Switches the current repository to the specified branch.

        This function will always be invoked prior to building a branch.

        """
        utils.run_command('git checkout %s' % branch.name)

    def build_branch(self, branch):
        """Builds the specified branch.

        This function will always be invoked after switching to a branch.

        """
        before_dir = os.getcwd()
        build_dir = branch.build_dir()
        try:
            print(build_dir)
            os.chdir(build_dir)
            utils.run_command('cmake %s' % config.SRCROOT_PATH)
            utils.run_command('make -j%d' % multiprocessing.cpu_count())
        finally:
            os.chdir(before_dir)
