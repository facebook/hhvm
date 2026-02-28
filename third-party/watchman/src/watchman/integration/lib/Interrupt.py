# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


interrupted = False


def wasInterrupted() -> bool:
    global interrupted
    return interrupted


def setInterrupted() -> None:
    global interrupted
    interrupted = True


def checkInterrupt() -> None:
    """
    If an interrupt was detected, raise it now.
    We use this to defer interrupt processing until we're
    in the right place to handle it.
    """
    if wasInterrupted():
        raise KeyboardInterrupt()
