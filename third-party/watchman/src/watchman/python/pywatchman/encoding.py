# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import sys


"""Module to deal with filename encoding on the local system, as returned by
Watchman."""


default_local_errors = "surrogateescape"


def get_local_encoding() -> str:
    if sys.platform == "win32":
        # Watchman always returns UTF-8 encoded strings on Windows.
        return "utf-8"
    # On the Python 3 versions we support, sys.getfilesystemencoding never
    # returns None.
    return sys.getfilesystemencoding()


def encode_local(s):
    return s.encode(get_local_encoding(), default_local_errors)


def decode_local(bs):
    return bs.decode(get_local_encoding(), default_local_errors)
