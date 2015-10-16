from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import re
import signal
import subprocess
import sys

test_env = dict(os.environ, **{'HH_TEST_MODE': '1'})

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

def proc_call(args, env=None, stdin=None):
    """
    Invoke a subprocess, return stdout, send stderr to our stderr (for
    debugging)
    """
    env = {} if env is None else env
    print(" ".join(args), file=sys.stderr)
    proc = subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=dict(test_env, **env),
            universal_newlines=True)
    (stdout_data, stderr_data) = proc.communicate(stdin)
    sys.stderr.write(stderr_data)
    sys.stderr.flush()
    return stdout_data

def ensure_output_contains(f, s, timeout=20):
    """
    Looks for a match in a process' output, subject to a timeout in case the
    process hangs
    """
    lines = []
    def handler(signo, frame):
        raise AssertionError('Failed to find %s in the following output: %s' %
                (s, ''.join(lines)))

    try:
        signal.signal(signal.SIGALRM, handler)
        signal.alarm(timeout)
        while True:
            line = f.readline().decode('utf-8')
            if s in line:
                return
            lines.append(line)
    finally:
        signal.alarm(0)
