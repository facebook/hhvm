#!/usr/bin/env python3

# Utility to generate or verify `php.bzl` from `php.txt`

import os.path
import sys


def _read_file_and_gen_bzl(path: str) -> str:
    r = ""
    r += f"# This file is {'@'}generated from `php.txt`.\n"
    r += "# Do not edit manually, run `python3 hphp/system/php_bzl.py` instead.\n"
    r += "SYSTEMLIB_SRCS = [\n"
    prefix = "hphp/system/"
    with open(path) as f:
        for line in f.readlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith(prefix):
                line = line[len(prefix) :]
            r += f'    "{line}",\n'
    r += "]\n"
    return r


def _generate():
    php_txt = os.path.dirname(__file__) + "/php.txt"
    php_bzl = os.path.dirname(__file__) + "/php.bzl"
    contents = _read_file_and_gen_bzl(php_txt)
    with open(php_bzl, mode="w") as f:
        f.write(contents)


def _verify(php_txt: str, php_bzl: str):
    with open(php_bzl, mode="rb") as f:
        expected = f.read()
    assert isinstance(expected, bytes)
    actual = _read_file_and_gen_bzl(php_txt)
    actual = bytes(actual, "utf-8")
    assert expected == actual, "file need to be regenerated"


def main() -> None:
    if len(sys.argv) == 1:
        # This is to be invoked manually when `php.txt` changes
        _generate()
    elif sys.argv[1] == "verify":
        # This is invoked as Buck test
        [php_txt, php_bzl] = sys.argv[2:]
        _verify(php_txt, php_bzl)
    else:
        raise Exception("unknown mode")


if __name__ == "__main__":
    main()
