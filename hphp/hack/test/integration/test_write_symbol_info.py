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
    DeclarationComment,
    DeclarationLocation,
    DeclarationSpan,
    EnumDeclaration,
    EnumDefinition,
    Enumerator,
    FileDeclarations,
    FileXRefs,
    FunctionDeclaration,
    FunctionDefinition,
    GlobalConstDeclaration,
    GlobalConstDefinition,
    InterfaceDeclaration,
    InterfaceDefinition,
    MethodDeclaration,
    MethodDefinition,
    NamespaceQName,
    PropertyDeclaration,
    PropertyDefinition,
    QName,
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

    def tearDown(self) -> None:
        try:
            # driver.tearDown() can throw if the env is not as expected
            super().tearDown()
        except Exception as e:
            print("Error during test teardown : {}".format(str(e)))

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
                                # pyre-fixme[6]: Expected
                                #  `Type[Variable[thrift.py3.serializer.sT (bound to
                                #  thrift.py3.types.Struct)]]` for 1st param but got
                                #  `Optional[Type[thrift.py3.types.Struct]]`.
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
        ver = 3
        predicate_dict = {
            "hack.ClassConstDeclaration.{}".format(ver): ClassConstDeclaration,
            "hack.ClassConstDefinition.{}".format(ver): ClassConstDefinition,
            "hack.ClassDeclaration.{}".format(ver): ClassDeclaration,
            "hack.ClassDefinition.{}".format(ver): ClassDefinition,
            "hack.DeclarationComment.{}".format(ver): DeclarationComment,
            "hack.DeclarationLocation.{}".format(ver): DeclarationLocation,
            "hack.DeclarationSpan.{}".format(ver): DeclarationSpan,
            "hack.EnumDeclaration.{}".format(ver): EnumDeclaration,
            "hack.EnumDefinition.{}".format(ver): EnumDefinition,
            "hack.Enumerator.{}".format(ver): Enumerator,
            "hack.FileDeclarations.{}".format(ver): FileDeclarations,
            "hack.FileXRefs.{}".format(ver): FileXRefs,
            "hack.FunctionDeclaration.{}".format(ver): FunctionDeclaration,
            "hack.FunctionDefinition.{}".format(ver): FunctionDefinition,
            "hack.GlobalConstDeclaration.{}".format(ver): GlobalConstDeclaration,
            "hack.GlobalConstDefinition.{}".format(ver): GlobalConstDefinition,
            "hack.InterfaceDeclaration.{}".format(ver): InterfaceDeclaration,
            "hack.InterfaceDefinition.{}".format(ver): InterfaceDefinition,
            "hack.MethodDeclaration.{}".format(ver): MethodDeclaration,
            "hack.MethodDefinition.{}".format(ver): MethodDefinition,
            "hack.NamespaceQName.{}".format(ver): NamespaceQName,
            "hack.PropertyDeclaration.{}".format(ver): PropertyDeclaration,
            "hack.PropertyDefinition.{}".format(ver): PropertyDefinition,
            "hack.QName.{}".format(ver): QName,
            "hack.TraitDeclaration.{}".format(ver): TraitDeclaration,
            "hack.TraitDefinition.{}".format(ver): TraitDefinition,
            "hack.TypeConstDeclaration.{}".format(ver): TypeConstDeclaration,
            "hack.TypeConstDefinition.{}".format(ver): TypeConstDefinition,
            "hack.TypedefDeclaration.{}".format(ver): TypedefDeclaration,
            "src.FileLines.1": FileLines,
        }
        return predicate_dict.get(predicate_name)

    def start_hh_server(
        self,
        changed_files: Optional[List[str]] = None,
        saved_state_path: Optional[str] = None,
        args: Optional[List[str]] = None,
    ) -> None:
        """Start an hh_server. changed_files is ignored here (as it
        has no meaning) and is only exposed in this API for the derived
        classes.
        """
        if changed_files is None:
            changed_files = []
        if args is None:
            args = []
        cmd = [
            hh_server,
            "--max-procs",
            "2",
            "--config",
            "symbolindex_search_provider=NoIndex",
            self.test_driver.repo_dir,
        ] + args
        self.test_driver.proc_call(cmd)

    def test_json_format(self) -> None:
        print("repo_contents : {}".format(os.listdir(self.test_driver.repo_dir)))
        args: Optional[List[str]] = None
        args = ["--write-symbol-info", self.write_repo]
        self.start_hh_server(args=args)
        self.verify_json()
