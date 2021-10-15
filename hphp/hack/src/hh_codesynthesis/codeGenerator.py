#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
# A family of code generators,
# CodeGenerator just emitting LP code(raw Stable Model)
# HackGenerator class extends CodeGenerator to emit its own code.
from typing import List, Optional

import clingo
from clingo import Number, Symbol


class ClingoContext:
    """Context class interact with Python and Clingo."""

    def __init__(
        self,
        number_of_nodes: int = 0,
        min_depth: int = 1,
        min_classes: int = 1,
        min_interfaces: int = 1,
        lower_bound: int = 1,
        higher_bound: int = 1,
        min_stub_classes: int = 0,
        min_stub_interfaces: int = 0,
        degree_distribution: List[int] = [],
    ) -> None:
        self.number_of_nodes = number_of_nodes
        self.min_depth = min_depth
        self.min_classes = min_classes
        self.min_interfaces = min_interfaces
        self.lower_bound = lower_bound
        self.higher_bound = higher_bound
        self.min_stub_classes = min_stub_classes
        self.min_stub_interfaces = min_stub_interfaces
        self.degree_distribution = degree_distribution

    def n(self) -> Symbol:
        return Number(self.number_of_nodes)

    def d(self) -> Symbol:
        return Number(self.min_depth)

    def c(self) -> Symbol:
        return Number(self.min_classes)

    def i(self) -> Symbol:
        return Number(self.min_interfaces)

    def lb(self) -> Symbol:
        return Number(self.lower_bound)

    def hb(self) -> Symbol:
        return Number(self.higher_bound)

    def sc(self) -> Symbol:
        return Number(self.min_stub_classes)

    def si(self) -> Symbol:
        return Number(self.min_stub_interfaces)


class CodeGenerator(object):
    """A base generator to emit raw model from Clingo output only
    The children classes can extend the functionality to produce
    corresponding Hack/Java/C# code.
    """

    def __init__(self, solving_context: Optional[ClingoContext] = None) -> None:
        super(CodeGenerator, self).__init__()
        self._raw_model = ""
        self.solving_context = (
            ClingoContext() if not solving_context else solving_context
        )

    def __str__(self) -> str:
        return self._raw_model

    """
    Callback function for Clingo on_model event.
    """

    def on_model(self, m: clingo.Model) -> None:
        self._raw_model = m.__str__()
