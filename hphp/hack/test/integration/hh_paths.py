from __future__ import absolute_import, division, print_function, unicode_literals

import os
import sys
from typing import List


# Any python_unittest target which directly or indirectly uses the hh_paths library
# must set these four environment variables:
# env = {
#   "HACKFMT_TEST_PATH": "$(exe_target //hphp/hack/src:hackfmt)",
#   "HH_CLIENT_PATH": "$(exe_target //hphp/hack/src:hh_client_precopy)",
#   "HH_FANOUT_PATH": "$(exe_target //hphp/hack/src/hh_fanout:hh_fanout)",
#   "HH_SERVER_PATH": "$(exe_target //hphp/hack/src:hh_server_precopy)",
# },
# Then, they can use hh_paths.{hackfmt,hh_client,hh_fanout,hh_server} confident in the
# knowledge that these will be absolute paths which point to existing binaries,
# and moreover that hh_server and hh_client are in the same directory.
# It's an all-or-nothing deal for convenience -- even tests that only use some of these binaries
# must use all of them.
#
# How does it work? Because the unit test has $(exe_target), then buck2 knows that even if the exe
# had been built on a different machine, it must still be fetched from CAS and placed into
# the appropriate path in buck-out on the machine that's running the unit test.
#
# Why use $(exe_target) ?
# $(exe) - for tools needed for doing work on the host machine; is typically NOSAN
# $(exe_target) - for the binaries you aim to test; in dev mode is ASAN by default.
# $(location) - doesn't specify anything about the binary!
#
# GOTCHA 1. clientStart.ml will use HH_SERVER_PATH if defined, and failing that will look
# for "hh_server" in "dirname(realpath(hh_client))/hh_server", and if that doesn't exist
# will just use unqualified "hh_server" hence looking it up in the path.
# That typically means going via /usr/local/bin/hh_server, which is find_hh.sh.
# Also, part of the python tests set HH_HOME=dirname(hh_client) and hence find_hh will use $HH_HOME/hh_server.
# But,
# we will just use the HH_SERVER_PATH approach! which is already defined by the
# unit-test target, and all we do here is verify that it really exists. So we're supporting
# two mechanisms:
# (1) for any tests which launch hh_client, hh_client will get to use HH_SERVER_PATH
# (2) for any tests which launch hh_paths.hh_server, they will launch the correct binary
#
# GOTCHA 2. serverFormat.ml will use HACKFMT_TEST_PATH if defined and found, otherwise it will
# use "/usr/local/bin/hackfmt" (which again is usually find_hh.sh). Our test target is assumed
# to have defined it, and all we're doing is verifying that it points to something that exists
# on disk.
#
# GOTCHA 3. Some authorities claim that $(exe/location :foo) produces a repo-relative path
# and that our test is run with CWD=repo-root. Other authorites suggest that it produces
# an absolute path. We will use os.path.abspath to cover both cases.

hackfmt: str = os.path.abspath(os.getenv("HACKFMT_TEST_PATH", "?"))
hh_client: str = os.path.abspath(os.getenv("HH_CLIENT_PATH", "?"))
hh_server: str = os.path.abspath(os.getenv("HH_SERVER_PATH", "?"))

errors: List[str] = []
if not os.path.exists(hackfmt):
    errors.append("Not found HACKFMT_TEST_PATH")
if not os.path.exists(hh_client):
    errors.append("Not found HH_CLIENT_PATH")
if not os.path.exists(hh_server):
    errors.append("Not found HH_SERVER_PATH")
if len(errors) > 0:
    errors: List[str] = [
        "CWD=" + os.getcwd(),
        "HACKFMT_TEST_PATH=" + os.getenv("HACKFMT_TEST_PATH", "?") + " => " + hackfmt,
        "HH_SERVER_PATH=" + os.getenv("HH_SERVER_PATH", "?") + " => " + hh_server,
        "HH_CLIENT_PATH=" + os.getenv("HH_CLIENT_PATH", "?") + " => " + hh_client,
        "-------------",
    ] + errors
    print("\n".join(errors), file=sys.stderr)
    exit(1)
