#!/usr/bin/env python
"""Miscellaneous convenience utilities.

"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import shlex
import subprocess
import sys

import benchy_config as config

def run_command(cmd, env=None, stdout=None, stderr=None):
    """Runs a command and checks the return code for errors. If stdout is not
    specified, redirects stdout of command to stderr.

    """
    cmd = shlex.split(cmd.encode('utf8'))
    try:
        saved_stdout = sys.stdout
        sys.stdout = sys.stderr
        if config.verbose():
            print(cmd)
        subprocess.check_call(
            cmd,
            env=env,
            stdout=(stdout or sys.stdout),
            stderr=(stderr or sys.stderr))
    finally:
        sys.stdout = saved_stdout
