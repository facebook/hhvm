# pyre-strict
from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
from typing import ClassVar, Dict, List, Optional, Type

import test_case
from common_tests import CommonTestDriver
from glean.schema.hack.types import (
    ClassConstDeclaration,
    ClassConstDefinition,
    ClassDeclaration,
    ClassDefinition,
    DeclarationLocation,
    EnumDeclaration,
    EnumDefinition,
    FileXRefs,
    FunctionDeclaration,
    FunctionDefinition,
    GlobalConstDeclaration,
    GlobalConstDefinition,
    InterfaceDeclaration,
    InterfaceDefinition,
    MethodDeclaration,
    MethodDefinition,
    PropertyDeclaration,
    PropertyDefinition,
    TraitDeclaration,
    TraitDefinition,
    TypeConstDeclaration,
    TypeConstDefinition,
    TypedefDeclaration,
)
from glean.schema.src.types import FileLines
from hh_paths import hh_server
from thrift.py3 import Protocol, Struct, deserialize


class WriteSymbolInfoTests(test_case.TestCase[CommonTestDriver]):
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

    def verify_json(self) -> None:
        for filename in os.listdir(self.write_repo):
            self.assertTrue(
                filename.endswith(".json"), "All output files are in JSON format"
            )

            with open(os.path.join(self.write_repo, filename)) as file:
                json_predicate_list = json.load(file)
                for pred_obj in json_predicate_list:
                    self.assertTrue(
                        "predicate" in pred_obj and "facts" in pred_obj,
                        "JSON predicate has correct form",
                    )

                    fact_type = self.predicate_name_to_type(pred_obj["predicate"])
                    if fact_type is None:
                        self.fail(
                            "Could not find matching Thrift definition for {}".format(
                                pred_obj["predicate"]
                            )
                        )

                    for fact in pred_obj["facts"]:
                        try:
                            deserialize(
                                fact_type,
                                json.dumps(fact).encode("UTF-8"),
                                protocol=Protocol.JSON,
                            )
                        except Exception as e:
                            self.fail(
                                "Could not deserialize {} fact JSON: {}\nDeserialization error: {}".format(
                                    fact_type, fact, e
                                )
                            )

    def predicate_name_to_type(self, predicate_name: str) -> Optional[Type[Struct]]:
        predicate_dict = {
            "hack.ClassConstDeclaration.1": ClassConstDeclaration,
            "hack.ClassConstDefinition.1": ClassConstDefinition,
            "hack.ClassDeclaration.1": ClassDeclaration,
            "hack.ClassDefinition.1": ClassDefinition,
            "hack.DeclarationLocation.1": DeclarationLocation,
            "hack.EnumDeclaration.1": EnumDeclaration,
            "hack.EnumDefinition.1": EnumDefinition,
            "hack.FileXRefs.1": FileXRefs,
            "hack.FunctionDeclaration.1": FunctionDeclaration,
            "hack.FunctionDefinition.1": FunctionDefinition,
            "hack.GlobalConstDeclaration.1": GlobalConstDeclaration,
            "hack.GlobalConstDefinition.1": GlobalConstDefinition,
            "hack.InterfaceDeclaration.1": InterfaceDeclaration,
            "hack.InterfaceDefinition.1": InterfaceDefinition,
            "hack.MethodDeclaration.1": MethodDeclaration,
            "hack.MethodDefinition.1": MethodDefinition,
            "hack.PropertyDeclaration.1": PropertyDeclaration,
            "hack.PropertyDefinition.1": PropertyDefinition,
            "hack.TraitDeclaration.1": TraitDeclaration,
            "hack.TraitDefinition.1": TraitDefinition,
            "hack.TypeConstDeclaration.1": TypeConstDeclaration,
            "hack.TypeConstDefinition.1": TypeConstDefinition,
            "hack.TypedefDeclaration.1": TypedefDeclaration,
            "src.FileLines.1": FileLines,
        }
        return predicate_dict.get(predicate_name)

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
        self.verify_json()
