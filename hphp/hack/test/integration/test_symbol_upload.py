# pyre-strict
from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
from typing import Any, ClassVar, Dict, List, Optional

import test_case
import utils
from common_tests import CommonTestDriver
from hh_paths import hh_server


class SymbolUploadTests(test_case.TestCase[CommonTestDriver]):
    write_repo: ClassVar[str] = "default_symbol_info_test_dir"
    valid_keys: ClassVar[Dict[str, List[object]]] = {}

    @classmethod
    def get_test_driver(cls) -> CommonTestDriver:
        return CommonTestDriver()

    def setUp(self) -> None:
        super().setUp()
        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "w") as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
incremental_init = true
enable_fuzzy_search = false
max_workers = 2
"""
            )

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        test_driver = cls._test_driver
        if test_driver is not None:
            cls.write_repo = os.path.join(test_driver.repo_dir, "symbol_info_test_dir")

        # hardcoded map of valid keys
        cls.valid_keys = {
            "hack.ClassDeclaration.1": ["name"],
            "hack.ClassDefinition.1": [
                "declaration",
                "is_abstract",
                "is_final",
                "members",
            ],
            "hack.DeclarationLocation.1": ["declaration", "file", "span"],
            "hack.FileXRefs.1": ["file", "xrefs"],
            "hack.InterfaceDeclaration.1": ["name"],
            "hack.InterfaceDefinition.1": ["declaration"],
            "hack.TraitDeclaration.1": ["name"],
            "hack.TraitDefinition.1": ["declaration"],
            "name": ["key"],
            "declaration": ["container", int],
            "container": ["class_", "interface_", "trait_"],
            "id": [int],
            "key": [str],
        }

    def verify_all_json(self) -> None:
        for filename in os.listdir(self.write_repo):
            if not filename.endswith(".json"):  # not a json for some reason - ignore
                continue

            with open(os.path.join(self.write_repo, filename)) as file:
                json_obj = json.load(file)
                self.verify_json_array(json_obj)

    def verify_json_array(self, json_array: List[utils.Json]) -> None:
        # starts off as json array
        all_preds = [
            "hack.ClassDeclaration.1",
            "hack.ClassDefinition.1",
            "hack.DeclarationLocation.1",
            "hack.FileXRefs.1",
            "hack.InterfaceDeclaration.1",
            "hack.InterfaceDefinition.1",
            "hack.TraitDeclaration.1",
            "hack.TraitDefinition.1",
        ]

        for json_obj in json_array:
            self.assertIn("predicate", json_obj, "Object is predicate")
            self.assertIn(json_obj["predicate"], all_preds, "Predicate is valid")

            valid_keys = self.valid_keys[json_obj["predicate"]]
            for json_fact in json_obj["facts"]:
                self.verify_json(json_fact, valid_keys)

    def verify_json(self, json_obj: Dict[str, Any], valid_keys: List[object]) -> None:
        for key, element in json_obj.items():
            # if element is not an object, just check that type(element)
            # in valid_keys[key]
            if key in self.valid_keys and type(element) in self.valid_keys[key]:
                continue

            if key == "key":
                self.verify_json(element, valid_keys)
            else:
                self.assertIn(key, valid_keys, "Object key is valid")
                if key in self.valid_keys:
                    self.verify_json(element, self.valid_keys[key])

    def start_hh_server(
        self,
        changed_files: Optional[List[str]] = None,
        saved_state_path: Optional[str] = None,
        args: Optional[List[str]] = None,
    ) -> None:
        """ Start an hh_server. changed_files is ignored here (as it
        has no meaning) and is only exposed in this API for the derived
        classes.
        """
        if changed_files is None:
            changed_files = []
        if args is None:
            args = []
        cmd = [hh_server, "--max-procs", "2", self.test_driver.repo_dir] + args
        self.test_driver.proc_call(cmd)

    def test_json_format(self) -> None:
        print("repo_contents : {}".format(os.listdir(self.test_driver.repo_dir)))
        args: Optional[List[str]] = None
        args = ["--write-symbol-info", self.write_repo]
        self.start_hh_server(args=args)
        self.verify_all_json()
