#!/usr/bin/env python3
import json
import os

timing = {}


with open("timing.txt.TEMP") as fh:
    for line in fh:
        fields = line.split()
        try:
            if fields[0] in {"real", "user", "sys"}:
                assert len(fields) >= 2, \
                    "field with real/user/sys in it does not have value"
                timing[fields[0]] = fields[1]

            if fields[0].endswith(":"):
                assert len(fields) >= 2, \
                    "field matching --debug-time output does not have value"
                timing[fields[0].rstrip(":").lower()] = fields[1]
        except IndexError:
            pass

with open("timing.json.TEMP", "wb") as fh:
    fh.write(bytes(json.dumps(timing), "utf-8"))
    fh.write(b"\n")

res = os.system("scribe_cat perfpipe_hh_ffp_cold_start < timing.json.TEMP")
assert not res, "Failed to write to scribe table!"
