from __future__ import absolute_import, division, print_function, unicode_literals

import filecmp
import os
import sys
from typing import List


# Any python_unittest target which directly or indirectly uses the hh_paths library
# must set these four environment variables:
# env = {
#     "HACKFMT_TEST_PATH": "$(exe //hphp/hack/src:hackfmt)",
#     "HH_CLIENT_PATH": "$(location //hphp/hack/src:hh_client)",
#     "HH_FANOUT_PATH": "$(exe //hphp/hack/src/hh_fanout:hh_fanout)",
#     "HH_SERVER_PATH": "$(location //hphp/hack/src:hh_server)",
# },
# Then, they can use hh_paths.{hackfmt,hh_client,hh_fanout,hh_server} confident in the
# knowledge that these will be absolute paths which point to existing binaries,
# and moreover that hh_server and hh_client are in the same directory.
# It's an all-or-nothing deal for convenience -- even tests that only use some of these binaries
# must use all of them.
#
# How does it work? Because the unit test has $(exe), then buck2 knows that even if the exe
# had been built on a different machine, it must still be fetched from CAS and placed into
# the appropriate path in buck-out on the machine that's running the unit test.
# We prefer $(exe) because buck2 prefers those for binaries, e.g. to match up machine architecture.
#
# Why $(location) for :hh_server and :hh_client? Because these two targets aren't binary targets;
# they're genrules which have the side effect of placing symlinks to the binaries into the same
# location as $(location :hh_client).
#
# GOTCHA 1. clientStart.ml requires that hh_server be in the same directory has hh_client.
# Also, part of the python tests set HH_HOME=dirname(hh_client) and then clientStart.ml
# invokes unqualified "hh_server" which falls through to find_hh.sh which invokes "$HH_HOME/hh_server".
# Recall that we're using the side-effect that :hh_server places a symlink into the right directory.
# (1) We will use "dirname($location :hh_client)/hh_server" as our path to HH_SERVER
# (2) We will still expect the unit test to set "HH_SERVER=($location :hh_server)" so they don't
# forget to have a dependency on :hh_server and its side-effect. Note that :hh_server has labels
# "uses_local_filesystem_abspaths" and "writes_to_buck_out" so it will be run on the same machine
# as the unit-test, i.e. will create its symlinks on that machine.
# (3) We'll ensure that the two paths to hh_server point to identical binaries, to avoid the case
# where an old symlink happened to be left behind.
#
# GOTCHA 2. serverFormat.ml will use HACKFMT_TEST_PATH if defined and found, otherwise it will
# use "/usr/local/bin/hackfmt". The test environment is assumed to have defined it (which will
# be inherited by processes we spawn) and here and now we're verifying that it can be found.
#
# GOTCHA 3. Some authorities claim that $(exe/location :foo) produces a repo-relative path
# and that our test is run with CWD=repo-root. Other authorites suggest that it produces
# an absolute path. We will use os.path.abspath to cover both cases.

hackfmt: str = os.path.abspath(os.getenv("HACKFMT_TEST_PATH", "?"))
hh_fanout: str = os.path.abspath(os.getenv("HH_FANOUT_PATH", "?"))
hh_client: str = os.path.abspath(os.getenv("HH_CLIENT_PATH", "?"))
hh_server0: str = os.path.abspath(os.getenv("HH_SERVER_PATH", "?"))
hh_server: str = os.path.join(os.path.dirname(hh_client), "hh_server")

errors: List[str] = []
if not os.path.exists(hackfmt):
    errors.append("Not found HACKFMT_TEST_PATH")
if not os.path.exists(hh_fanout):
    errors.append("Not found HH_FANOUT_PATH")
if not os.path.exists(hh_client):
    errors.append("Not found HH_CLIENT_PATH")
if not os.path.exists(hh_server0):
    errors.append("Not found HH_SERVER_PATH")
if not os.path.exists(hh_server):
    errors.append("Not found dirname(HH_CLIENT_PATH)/hh_server")
elif not filecmp.cmp(hh_server0, hh_server, shallow=False):
    errors.append("HH_SERVER_PATH and dirname(HH_CLIENT_PATH)/hh_server are different")
if len(errors) > 0:
    errors: List[str] = [
        "CWD=" + os.getcwd(),
        "HACKFMT_TEST_PATH=" + os.getenv("HACKFMT_TEST_PATH", "?") + " => " + hackfmt,
        "HH_FANOUT_PATH=" + os.getenv("HH_FANOUT_PATH", "?") + " => " + hh_fanout,
        "HH_SERVER_PATH=" + os.getenv("HH_SERVER_PATH", "?") + " => " + hh_server0,
        "HH_CLIENT_PATH=" + os.getenv("HH_CLIENT_PATH", "?") + " => " + hh_client,
        "dirname(HH_CLIENT_PATH)/hh_server => " + hh_server,
        "-------------",
    ] + errors
    print("\n".join(errors), file=sys.stderr)
    exit(1)
