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
import clingo


class CodeGenerator(object):
    """A base generator to emit raw model from Clingo output only
    The children classes can extend the functionality to produce
    corresponding Hack/Java/C# code.
    """

    def __init__(self) -> None:
        super(CodeGenerator, self).__init__()
        self._raw_model = ""

    def __str__(self) -> str:
        return self._raw_model

    """
    Callback function for Clingo on_model event.
    """

    def on_model(self, m: clingo.Model) -> None:
        self._raw_model = m.__str__()
