#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
# A library/binary take the output from hh_single_type_check --dump-deps.
# And synthesis code based on the dependency graph.
#
# The synthesized code should produce the same dependency graph as given input.
#
# This library will construct a logic program using the given dependency graph,
# invoke Clingo to solve the logic program to get stable models, and we
# interpret the stable model to produce valid code.
#
# Currently, we are synthesising Hack code only, we will support more
# languages like C#, Java later on.

from typing import List

# Extract logic rules from file format.
def extract_logic_rules(lines: List[str]) -> List[str]:
    rules = []
    symbols = set()

    for line in lines:
        # Required input formatting to get "Extends A -> Type B, Type C, Type D".
        # And split get
        # lhs = "Extends A"
        # rhs = "Type B, Type C, Type D"
        line = line.strip().split("->")
        if len(line) != 2:
            # ToDo: Add logging if we needed to track wrong format on missing "->".
            continue
        (lhs, rhs) = line

        lhs = lhs.split()
        # The lhs length must be 2.
        if len(lhs) != 2:
            # ToDo: Add logging if we needed to track wrong format on lhs.
            continue

        # ToDo: Dict{"lhs[0]": "function to convert"} for later extension.
        if lhs[0] == "Extends":
            # Collecting symbols.
            symbols.add(lhs[1].lower())
            # Processing each deps ["Type B", "Type C", "Type D"]
            for dep in rhs.rstrip("\n").split(","):
                # dep = "Type X"
                dep = dep.split()
                if len(dep) != 2:
                    # ToDo: Add logging if we needed to track wrong format on rhs.
                    continue
                (dep_type, dep_symbol) = dep
                symbols.add(dep_symbol.lower())
                rules.append(
                    "extends_to({}, {}).".format(lhs[1].lower(), dep_symbol.lower())
                )

    rules.append("symbols({}).".format(";".join(sorted(symbols))))
    return rules
