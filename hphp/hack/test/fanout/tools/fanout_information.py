# pyre-strict

import json
import re
from typing import List

import attr

# pyre-fixme[24]: Generic type `re.Pattern` expects 1 type parameter.
FANOUT_INFORMATION_RE: re.Pattern = re.compile(r"\[fanout_tests\]\s+(.*)$")


@attr.s(auto_attribs=True)
class FanoutInformation:
    tag: str
    hashes: List[str]

    @staticmethod
    def from_json(json_str: str) -> "FanoutInformation":
        o = json.loads(json_str)
        tag = str(o["tag"])
        hashes = [str(h) for h in o["hashes"]]
        return FanoutInformation(tag=tag, hashes=hashes)

    @staticmethod
    def extract_from_log_file(log_file: str) -> List["FanoutInformation"]:
        infos = []
        with open(log_file, "r") as fp:
            for line in fp:
                m = FANOUT_INFORMATION_RE.search(line)
                if m is not None:
                    infos.append(FanoutInformation.from_json(m.group(1)))
        return infos
