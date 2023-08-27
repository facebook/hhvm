# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import atexit
import errno
import os
import shutil
import sys
import tempfile
import time

import pywatchman

from . import path_utils as path


global_temp_dir = None


class TempDir:
    """
    This is a helper for locating a reasonable place for temporary files.
    When run in the watchman test suite, we compute this up-front and then
    store everything under that temporary directory.
    When run under the FB internal test runner, we infer a reasonable grouped
    location from the process group environmental variable exported by the
    test runner.
    """

    def __init__(self, keepAtShutdown: bool = False) -> None:
        # We'll put all our temporary stuff under one dir so that we
        # can clean it all up at the end.

        parent_dir = tempfile.gettempdir()
        prefix = "watchmantest"

        self.temp_dir = path.get_canonical_filesystem_path(
            tempfile.mkdtemp(dir=parent_dir, prefix=prefix)
        )

        if os.name != "nt":
            # On some platforms, setting the setgid bit on a directory doesn't
            # work if the user isn't a member of the directory's group. Set the
            # group explicitly to avoid this.
            os.chown(self.temp_dir, -1, os.getegid())
            # Some environments have a weird umask that can leave state
            # directories too open and break tests.
            os.umask(0o022)
        # Redirect all temporary files to that location
        tempfile.tempdir = os.fsdecode(self.temp_dir)

        self.keep = keepAtShutdown

        def cleanup():
            if self.keep:
                sys.stdout.write("Preserving output in %s\n" % self.temp_dir)
                return
            self._retry_rmtree(self.temp_dir)

        atexit.register(cleanup)

    def get_dir(self):
        return self.temp_dir

    def set_keep(self, value) -> None:
        self.keep = value

    def _retry_rmtree(self, top) -> None:
        # Keep trying to remove it; on Windows it may take a few moments
        # for any outstanding locks/handles to be released
        for _ in range(1, 10):
            shutil.rmtree(top, onerror=_remove_readonly)
            if not os.path.isdir(top):
                return
            sys.stdout.write("Waiting to remove temp data under %s\n" % top)
            time.sleep(0.2)
        sys.stdout.write("Failed to completely remove %s\n" % top)


def _remove_readonly(func, path, exc_info) -> None:
    # If we encounter an EPERM or EACCESS error removing a file try making its parent
    # directory writable and then retry the removal.  This is necessary to clean up
    # eden mount point directories after the checkout is unmounted, as these directories
    # are made read-only by "eden clone"
    _ex_type, ex, _traceback = exc_info
    if not (
        isinstance(ex, EnvironmentError) and ex.errno in (errno.EACCES, errno.EPERM)
    ):
        # Just ignore other errors.  This will be retried by _retry_rmtree()
        return

    try:
        parent_dir = os.path.dirname(path)
        os.chmod(parent_dir, 0o755)
        # func() is the function that failed.
        # This is usually os.unlink() or os.rmdir().
        func(path)
    except OSError:
        return


def get_temp_dir(keep=None):
    global global_temp_dir
    if global_temp_dir:
        return global_temp_dir
    if keep is None:
        keep = os.environ.get("WATCHMAN_TEST_KEEP", "0") == "1"
    global_temp_dir = TempDir(keep)
    return global_temp_dir
