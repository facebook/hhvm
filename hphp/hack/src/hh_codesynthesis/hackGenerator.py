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
# HackGenerator extends CodeGenerator combines all _Hack*Generator to emit Hack code on clingo output.

from typing import Set


class _HackInterfaceGenerator(object):
    """A generator to emit Hack Interface definition."""

    def __init__(self, name: str) -> None:
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
