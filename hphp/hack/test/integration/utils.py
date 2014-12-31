from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import re
import subprocess
import sys

def touch(fn):
    with open(fn, 'a'):
        os.utime(fn, None)

def write_files(files, dir_path):
    """
    Write a bunch of files into the directory at dir_path.

    files: dict of file name => file contents
    """
    for fn, content in files.items():
        path = os.path.join(dir_path, fn)
        with open(path, 'w') as f:
            f.write(content)

def proc_call(args, stdin=None):
    """
    Invoke a subprocess, return stdout, send stderr to our stderr (for
    debugging)
    """
    print(" ".join(args), file=sys.stderr)
    proc = subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True)
    (stdout_data, stderr_data) = proc.communicate(stdin)
    sys.stderr.write(stderr_data)
    sys.stderr.flush()
    return stdout_data
