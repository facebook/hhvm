from __future__ import absolute_import, unicode_literals

import json
import os
import tempfile
import time
from os import path
from typing import List, Optional, Tuple

from common_tests import CommonTestDriver
from hh_paths import hh_client
from test_case import TestCase


class GlobalInferenceDriver(CommonTestDriver):
    def run_hh_global_inference(
        self, submode: str, files: Optional[List[str]] = None
    ) -> Tuple[str, str, int]:
        files = [] if files is None else files
        return self.proc_call(
            [hh_client, self.repo_dir, "--global-inference", submode] + files,
            stdin=None,
        )

    def get_file_path(self, name):
        root = self.repo_dir + os.path.sep
        return name.format(root=root)


class TestPushOptionOutGlobalInference(TestCase[GlobalInferenceDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/global_inference/push_option_out"

    @classmethod
    def get_test_driver(cls) -> GlobalInferenceDriver:
        return GlobalInferenceDriver()

    def test(self) -> None:
        self.test_driver.start_hh_server(
            args=["--config", "global_inference=true", "--config", "timeout=20"]
        )
        self.test_driver.check_cmd(["No errors!"])


class TestDynamicNonnullHasMember(TestCase[GlobalInferenceDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/global_inference/dynamic_nonnull_has_member"

    @classmethod
    def get_test_driver(cls) -> GlobalInferenceDriver:
        return GlobalInferenceDriver()

    def test(self) -> None:
        self.test_driver.start_hh_server(
            args=["--config", "global_inference=true", "--config", "timeout=20"]
        )
        self.test_driver.check_cmd(["No errors!"])


class TestThreeFilesGlobalInference(TestCase[GlobalInferenceDriver]):
    """
    Test if we got no datarace. This test might be non deterministic:
    we try 10 times the same process and fail if one execution fails
    """

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/global_inference/www_three_files"

    @classmethod
    def get_test_driver(cls) -> GlobalInferenceDriver:
        return GlobalInferenceDriver()

    def execute_once(self):
        temp_dir = tempfile.mkdtemp()

        self.test_driver.start_hh_server(args=["--config", "global_inference=true"])
        logpath, _, _ = self.test_driver.proc_call(
            [hh_client, self.test_driver.repo_dir, "--logname"]
        )
        artifacts_path = None
        with open(logpath.strip()) as log:
            for line in log.readlines():
                if "Global artifacts path" in line:
                    artifacts_path = line.split()[-1]

        if artifacts_path is None:
            self.fail("Couldn't retrieve the artifacts")

        self.test_driver.run_hh_global_inference(
            "merge", [artifacts_path, path.join(temp_dir, "env")]
        )
        self.test_driver.run_hh_global_inference(
            "export-json", [path.join(temp_dir, "env"), path.join(temp_dir, "env.json")]
        )
        time.sleep(1)

        # When merging all global environments, some type variables are
        # actually aliases. When merging, only one will contain the actual
        # upper/lower bounds, the others will point to that one.
        def is_alias(node) -> bool:
            lbs = node["lower_bounds"]
            ubs = node["upper_bounds"]
            if len(lbs) != 1 or len(ubs) != 1:
                return False
            lb, ub = lbs[0], ubs[0]
            if lb[:1] != "#" or ub[:1] != "#":
                return False
            lb_n, ub_n = int(lb[1:]), int(ub[1:])
            return lb_n == ub_n

        with open(path.join(temp_dir, "env.json")) as json_file:
            content = json_file.read().replace("\\", "\\\\")
            graph_json = json.loads(content)
            graph_json = {
                key: node
                for key, node in graph_json.items()
                if node["filename"]
                and "hhi" not in node["filename"]
                and not is_alias(node)
            }
            return graph_json

    def check_node_is_correct(
        self,
        node,
        start_line: int,
        start_column: int,
        end_line: int,
        end_column: int,
        expect_lower_bounds: List[str],
        expect_upper_bounds: List[str],
        expect_file: str,
    ) -> None:
        if (
            int(node["start_line"]) == start_line
            and int(node["start_column"]) == start_column
            and int(node["end_line"]) == end_line
            and int(node["end_column"]) == end_column
            and set(expect_lower_bounds) == set(node["lower_bounds"])
            and set(expect_upper_bounds) == set(node["upper_bounds"])
            and expect_file in node["filename"]
        ):
            pass
        else:
            self.fail("Node {} incorrect".format(node))

    def test_graph_correctness(self) -> None:
        graph = self.execute_once()
        keys = ["", "", ""]
        for key, node in graph.items():
            id = len(set(node["lower_bounds"])) - 1
            if id < 0 or id > 2:
                self.fail("In this test we should have between 1 and 3 lower bounds")
            keys[id] = key
        if any(k == "" for k in keys):
            self.fail("We should have three different nodes")
        self.check_node_is_correct(
            graph[keys[0]], 4, 18, 4, 21, ["int"], [keys[1], keys[2]], "three.php"
        )
        self.check_node_is_correct(
            graph[keys[1]], 4, 18, 4, 21, ["int", keys[0]], [keys[2]], "two.php"
        )
        self.check_node_is_correct(
            graph[keys[2]], 4, 18, 4, 21, ["int", keys[0], keys[1]], [], "one.php"
        )

    def test_datarace(self) -> None:
        for _ in range(10):
            if len(self.execute_once()) != 3:
                self.fail(
                    "Data race detected, we should only have three variables here"
                )


class TestGlobalInferenceCorrectness(TestCase[GlobalInferenceDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/global_inference/www"

    @classmethod
    def get_test_driver(cls) -> GlobalInferenceDriver:
        return GlobalInferenceDriver()

    def execute_once(self):
        temp_dir = tempfile.mkdtemp()

        self.test_driver.start_hh_server(args=["--config", "global_inference=true"])
        logpath, _, _ = self.test_driver.proc_call(
            [hh_client, self.test_driver.repo_dir, "--logname"]
        )
        artifacts_path = None
        with open(logpath.strip()) as log:
            for line in log.readlines():
                if "Global artifacts path" in line:
                    artifacts_path = line.split()[-1]

        if artifacts_path is None:
            self.fail("Couldn't retrieve the artifacts")

        self.test_driver.run_hh_global_inference(
            "merge", [artifacts_path, path.join(temp_dir, "globalenv")]
        )
        self.test_driver.run_hh_global_inference(
            "solve", [path.join(temp_dir, "globalenv"), path.join(temp_dir, "env")]
        )
        self.test_driver.run_hh_global_inference(
            "rewrite", [path.join(temp_dir, "env")]
        )

    def test_correctness(self) -> None:
        self.execute_once()
        self.maxDiff = None

        for root, _, files in os.walk(self.test_driver.repo_dir):
            for filename in files:
                filename_no_ext, ext = os.path.splitext(filename)
                if ext == ".php":
                    outname = filename_no_ext + ".out"
                    file_content = ""
                    with open(path.join(root, filename)) as f:
                        file_content = f.read()
                    out_content = ""
                    with open(path.join(root, outname)) as f:
                        out_content = f.read()
                    self.assertMultiLineEqual(
                        file_content,
                        out_content,
                        "Rewritten {} does not match with {}".format(filename, outname),
                    )
