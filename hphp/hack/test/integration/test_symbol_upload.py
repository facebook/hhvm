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
    valid_keys: ClassVar[Dict[str, List[str]]] = {}

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
        # pyre-fixme[8]: Attribute has type `Dict[str, List[str]]`; used as
        #  `Dict[str, List[Type[Union[int, str]]]]`.
        cls.valid_keys = {
            "hackfull.symbolOccurrence.1": [
                "name_lowercase",
                "type",
                "position",
                "definition_pos",
            ],
            "hackfull.symbol.1": ["name_lowercase", "position", "declaration"],
            "hackfull.functionParameter.1": ["name", "type"],
            "hackfull.filename.1": ["filename", "filehash_id"],
            "hackfull.functionDeclaration.1": ["name", "params", "return_type"],
            "hackfull.classDeclaration.1": ["name", "is_abstract", "is_final"],
            "hackfull.typedefDeclaration.1": ["name", "is_visible"],
            "hackfull.gconstDeclaration.1": ["name", "type"],
            "type": ["key"],
            "name": ["key"],
            "declaration": ["class_", "typedef_", "gconst_", "function_"],
            "position": ["filename", "line_start", "line_end", "char_end"],
            "definition_pos": ["filename", "line_start", "line_end", "char_end"],
            "filename": ["filename", "filehash_id", str],
            "filehash_id": [str],
            "name_lowercase": [str],
            "class_": ["id", "key"],
            "typedef_": ["id", "key"],
            "gconst_": ["id", "key"],
            "function_": ["id", "key"],
            "id": [int],
            "key": [int, str],
        }

    def verify_all_json(self) -> bool:
        for filename in os.listdir(self.write_repo):
            if not filename.endswith(".json"):  # not a json for some reason - ignore
                continue

            with open(os.path.join(self.write_repo, filename)) as file:
                json_obj = json.load(file)
                if not self.verify_json_array(json_obj):
                    print("Error with file: {}".format(filename))
                    return False

        return True

    def verify_json_array(self, json_array: List[utils.Json]) -> bool:
        # starts off as json array
        all_preds = [
            "hackfull.symbolOccurrence.1",
            "hackfull.symbol.1",
            "hackfull.functionParameter.1",
            "hackfull.filename.1",
            "hackfull.functionDeclaration.1",
            "hackfull.classDeclaration.1",
            "hackfull.typedefDeclaration.1",
            "hackfull.gconstDeclaration.1",
        ]

        for json_obj in json_array:
            if "predicate" not in json_obj or json_obj["predicate"] not in all_preds:
                print("Predicate error: {}".format(json_obj["predicate"]))
                return False

            valid_keys = self.valid_keys[json_obj["predicate"]]
            for json_fact in json_obj["facts"]:
                if self.verify_json(json_fact, valid_keys):
                    return False

        return True

    def verify_json(self, json_obj: Dict[str, Any], valid_keys: List[str]) -> bool:
        for key, element in json_obj.items():
            # if element is not an object, just check that type(element)
            #   in valid_keys[key]
            if type(element) in self.valid_keys[key]:
                # must be a string or int as an element, no need to recurse
                continue

            if key not in valid_keys:
                return False

            next_valid_keys = self.valid_keys[key]
            if key == "key":
                # if didn't pass the above if, must be an object with any valid key
                next_valid_keys = self.valid_keys.keys()

            # pyre-fixme[6]: Expected `List[str]` for 2nd param but got
            #  `Union[List[str], KeysView[str]]`.
            if not self.verify_json(element, next_valid_keys):
                return False

        return True

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
        assert self.verify_all_json()
