# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
# A family of generators to create Hack constructs like class, function, interface, etc
#
# We are assuming all classes/interfaces/functions are properly defined before
# creating any implements/extends dependency. This is guaranteed by preceding
# step. In our case, it's the logic rule checked all constraints for us,
# therefore, the reasoning result from Clingo can be converted to code
# straightforward.
#
# _HackClassGenerator maintains each class definition.
# _HackInterfaceGenerator maintains each interface definition.
# _HackFunctionGenerator maintains each function definition.
# HackGenerator extends CodeGenerator combines all _Hack*Generator to
# emit Hack code on clingo output.
from typing import Set, Dict, Any

import clingo
from hphp.hack.src.hh_codesynthesis.codeGenerator import CodeGenerator


class _HackInterfaceGenerator(object):
    """A generator to emit Hack Interface definition."""

    def __init__(self, name: str, **kwargs: Dict[str, Any]) -> None:
        super(_HackInterfaceGenerator, self).__init__()
        self.name = name
        # A set of extends relationship in this interface.
        self.extends: Set[str] = set()

    def add_extend(self, extend_from: str) -> None:
        self.extends.add(extend_from)

    def _print_extends(self) -> str:
        if len(self.extends) == 0:
            return ""
        return "extends {}".format(",".join(sorted(self.extends)))

    def _print_body(self) -> str:
        return "{}"

    def __str__(self) -> str:
        return f"interface {self.name} {self._print_extends()} {self._print_body()}"


class _HackClassGenerator(object):
    """A generator to emit Hack Class definition."""

    def __init__(self, name: str, **kwargs: Dict[str, Any]) -> None:
        super(_HackClassGenerator, self).__init__()
        self.name = name
        # Extend relationship could only be one parent class.
        self.extend: str = ""
        # A set of implements relationship in this class.
        self.implements: Set[str] = set()

    def set_extend(self, extend_from: str) -> None:
        self.extend = extend_from

    def add_implement(self, implement: str) -> None:
        self.implements.add(implement)

    def _print_extend(self) -> str:
        if self.extend == "":
            return ""
        return "extends {}".format(self.extend)

    def _print_implements(self) -> str:
        if len(self.implements) == 0:
            return ""
        return "implements {}".format(",".join(sorted(self.implements)))

    def _print_body(self) -> str:
        return "{}"

    def __str__(self) -> str:
        return (
            f"class {self.name} {self._print_extend()} "
            + f"{self._print_implements()} {self._print_body()}"
        )


class HackCodeGenerator(CodeGenerator):
    """A wrapper generator encapsulates each _Hack*Generator to emit Hack Code"""

    def __init__(self) -> None:
        super(HackCodeGenerator, self).__init__()
        self.class_objs: Dict[str, _HackClassGenerator] = {}
        self.interface_objs: Dict[str, _HackInterfaceGenerator] = {}

    def _add_class(self, name: str) -> None:
        self.class_objs[name] = _HackClassGenerator(name)

    def _add_interface(self, name: str) -> None:
        self.interface_objs[name] = _HackInterfaceGenerator(name)

    def _add_extend(self, name: str, extend: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].set_extend(extend)
        if name in self.interface_objs:
            self.interface_objs[name].add_extend(extend)

    def _add_implement(self, name: str, implement: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_implement(implement)

    def __str__(self) -> str:
        return (
            "<?hh\n"
            + "\n".join(str(x) for x in self.class_objs.values())
            + "\n"
            + "\n".join(str(x) for x in self.interface_objs.values())
            + "\n"
        )

    def on_model(self, m: clingo.Model) -> None:
        # Separate into 'class(?)', 'interface(?)', 'implements(?, ?)', 'extends(?, ?)'
        predicates = m.symbols(atoms=True)
        node_func = {"class": self._add_class, "interface": self._add_interface}
        edge_func = {"extends": self._add_extend, "implements": self._add_implement}
        # Two passes,
        #   First pass creates individual nodes like class, interface.
        for predicate in predicates:
            if predicate.name in node_func:
                node_func[predicate.name](predicate.arguments[0].name.upper())

        #   Second pass creates edge between two nodes.
        for predicate in predicates:
            if predicate.name in edge_func:
                edge_func[predicate.name](
                    predicate.arguments[0].name.upper(),
                    predicate.arguments[1].name.upper(),
                )
