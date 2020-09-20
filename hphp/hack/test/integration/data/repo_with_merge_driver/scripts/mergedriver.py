from __future__ import absolute_import, division, print_function

import json
import os
import subprocess


# This merge driver just calls Hack Build. We want to make sure that
# the interdependencies between these pieces doesn't deadlock

# This mergedriver requires a mergedriver_test_env.json to be generated
# by the test runner. That's where the correct HH_TMPDIR gets wired in so
# we locate the correct socket, as well as HH_HOME so we find the
# hh_client binary for this test run.


def preprocess(ui, repo, hooktype, mergestate, wctx, labels):
    repo.ui.status("* preprocess called\n")
    for f in mergestate:
        mergestate.mark(f, "d")


def conclude(ui, repo, hooktype, mergestate, wctx, labels):
    repo.ui.status("* conclude called\n")
    with open(os.path.join("scripts", "mergedriver_test_env.json"), "r") as f:
        env_json = f.read()
    test_env = json.loads(env_json)
    hh_client = os.environ.get("HH_HOME") + "/hh_client"
    build_cmd = [hh_client, "check", "--force-dormant-start", "true", "."]
    subprocess.check_call(build_cmd, env=test_env)
