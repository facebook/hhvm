# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import signal
from types import FrameType
from typing import BinaryIO, Iterable, Mapping, Union, _ForwardRef


JsonObject = Mapping[str, _ForwardRef("Json")]
JsonArray = Iterable[_ForwardRef("Json")]
Json = Union[JsonObject, JsonArray, str, int, float, bool, None]


def touch(fn: str) -> None:
    with open(fn, "a"):
        os.utime(fn, None)


def write_files(files: Mapping[str, str], dir_path: str) -> None:
    """
    Write a bunch of files into the directory at dir_path.

    files: dict of file name => file contents
    """
    for fn, content in files.items():
        path = os.path.join(dir_path, fn)
        with open(path, "w") as f:
            f.write(content)


def ensure_output_contains(f: BinaryIO, s: str, timeout: int = 20) -> None:
    """
    Looks for a match in a process' output, subject to a timeout in case the
    process hangs
    """
    lines = []

    def handler(signo: int, frame: FrameType) -> None:
        raise AssertionError(
            "Failed to find %s in the following output: %s" % (s, "".join(lines))
        )

    try:
        signal.signal(signal.SIGALRM, handler)
        signal.alarm(timeout)
        while True:
            line = f.readline().decode("utf-8")
            if s in line:
                return
            lines.append(line)
    finally:
        signal.alarm(0)


def interpolate_variables(payload: Json, variables: Mapping[str, str]) -> Json:
    def replace_variable(json: Json, variable: str, text: str) -> Json:
        if isinstance(json, dict):
            return {
                # pyre-ignore: pyre can't track this recursive call
                replace_variable(k, variable, text): replace_variable(v, variable, text)
                for k, v in json.items()
            }
        elif isinstance(json, list):
            return [replace_variable(i, variable, text) for i in json]
        elif isinstance(json, str):
            return json.replace("${" + variable + "}", text)
        else:
            return json

    for variable, value in variables.items():
        payload = replace_variable(payload, variable, value)
    return payload
