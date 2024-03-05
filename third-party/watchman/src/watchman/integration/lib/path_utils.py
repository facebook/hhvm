# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import ctypes
import os
import os.path
import platform


if os.name == "nt":

    def open_file_win(path):

        create_file = ctypes.windll.kernel32.CreateFileW

        c_path = ctypes.create_unicode_buffer(path)
        access = 0
        mode = 7  # FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE
        disposition = 3  # OPEN_EXISTING
        flags = 33554432  # FILE_FLAG_BACKUP_SEMANTICS

        h = create_file(c_path, access, mode, 0, disposition, flags, 0)
        if h == -1:
            raise WindowsError("Failed to open file: " + path)

        return h

    def get_canonical_filesystem_path(name):
        gfpnbh = ctypes.windll.kernel32.GetFinalPathNameByHandleW
        close_handle = ctypes.windll.kernel32.CloseHandle

        h = open_file_win(name)
        try:
            gfpnbh = ctypes.windll.kernel32.GetFinalPathNameByHandleW
            numwchars = 1024
            while True:
                buf = ctypes.create_unicode_buffer(numwchars)
                result = gfpnbh(h, buf, numwchars, 0)
                if result == 0:
                    raise Exception("unknown error while normalizing path")

                # The first four chars are //?/
                if result <= numwchars:
                    return buf.value[4:].replace("\\", "/")

                # Not big enough; the result is the amount we need
                numwchars = result + 1
        finally:
            close_handle(h)

elif platform.system() == "Darwin":
    import ctypes.util

    F_GETPATH = 50
    libc = ctypes.CDLL(ctypes.util.find_library("c"), use_errno=True)
    getpath_fcntl = libc.fcntl
    getpath_fcntl.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
    getpath_fcntl.restype = ctypes.c_int

    def get_canonical_filesystem_path(name):
        fd = os.open(name, os.O_RDONLY, 0)
        try:
            numchars = 1024  # MAXPATHLEN
            # The kernel caps this routine to MAXPATHLEN, so there is no
            # point in over-allocating or trying again with a larger buffer
            buf = ctypes.create_string_buffer(numchars)
            ctypes.set_errno(0)
            result = getpath_fcntl(fd, F_GETPATH, buf)
            if result != 0:
                raise OSError(ctypes.get_errno())
            # buf is a bytes buffer, so normalize it if necessary
            ret = buf.value
            if isinstance(name, str):
                ret = os.fsdecode(ret)
            return ret
        finally:
            os.close(fd)

else:

    def get_canonical_filesystem_path(name):
        return os.path.normpath(name)


def norm_relative_path(path):
    # TODO: in the future we will standardize on `/` as the
    # dir separator so we can remove the replace call from here.
    # We do not need to normcase because all of our tests are
    # using the appropriate case already, and watchman returns
    # paths in the canonical file replace case anyway.
    return path.replace("\\", "/")


def norm_absolute_path(path):
    # TODO: in the future we will standardize on `/` as the
    # dir separator so we can remove the replace call.
    return path.replace("\\", "/")
