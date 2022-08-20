# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import re


def parse_version(vstr) -> int:
    res = 0
    for n in vstr.split("."):
        res = res * 1000
        res = res + int(n)
    return res


cap_versions = {
    "cmd-watch-del-all": "3.1.1",
    "cmd-watch-project": "3.1",
    "relative_root": "3.3",
    "term-dirname": "3.1",
    "term-idirname": "3.1",
    "wildmatch": "3.7",
}


def check(version, name: str):
    if name in cap_versions:
        return version >= parse_version(cap_versions[name])
    return False


def synthesize(vers, opts):
    """Synthesize a capability enabled version response
    This is a very limited emulation for relatively recent feature sets
    """
    parsed_version = parse_version(vers["version"])
    vers["capabilities"] = {}
    for name in opts["optional"]:
        vers["capabilities"][name] = check(parsed_version, name)
    failed = False  # noqa: F841 T25377293 Grandfathered in
    for name in opts["required"]:
        have = check(parsed_version, name)
        vers["capabilities"][name] = have
        if not have:
            vers["error"] = (
                "client required capabilities ["
                + name
                + "] not supported by this server"
            )
    return vers
