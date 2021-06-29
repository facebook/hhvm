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
import argparse
import os
import sys
from typing import List, Optional, Union

import clingo
from clingo import Number, Symbol
from hphp.hack.src.hh_codesynthesis.codeGenerator import CodeGenerator
from hphp.hack.src.hh_codesynthesis.hackGenerator import HackCodeGenerator
from libfb.py import parutil


class ClingoContext:
    """Context class interact with Python and Clingo."""

    number_of_nodes = 0
    avg_width = 0
    min_depth = 1
    min_classes = 1
    min_interfaces = 1
    lower_bound = 1
    higher_bound = 10

    def n(self) -> Symbol:
        return Number(self.number_of_nodes)

    def w(self) -> Symbol:
        return Number(self.avg_width)

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


# Helper classes to handle each dependency edge.
# lhs_parser parse the left hand side symbols into potential depends on the
# separator. For example,
#       Class::Method with "::" will produce [Class, Method]
#       Class with None will produce [Class]
class DependencyEdgeHandler:
    def lhs_parser(self, lhs: str, separator: Optional[str]) -> List[str]:
        return lhs.split(separator)


# The following rule_writers takes the return value from `lhs_parser` along
# with a rhs dep_symbol to output a rule accordingly.
class ExtendEdgeHandler(DependencyEdgeHandler):
    def parse(self, lhs: str) -> List[str]:
        return self.lhs_parser(lhs, None)

    def rule_writer(self, lhs: List[str], rhs: str) -> str:
        return f'extends_to("{lhs[0]}", "{rhs}").'


class TypeEdgeHandler(DependencyEdgeHandler):
    def parse(self, lhs: str) -> List[str]:
        return self.lhs_parser(lhs, None)

    def rule_writer(self, lhs: List[str], rhs: str) -> str:
        return f'type("{lhs[0]}", "{rhs}").'


class MethodEdgeHandler(DependencyEdgeHandler):
    def parse(self, lhs: str) -> List[str]:
        return self.lhs_parser(lhs, "::")

    def rule_writer(self, lhs: List[str], rhs: str) -> str:
        return f'method("{lhs[0]}", "{lhs[1]}", "{rhs}").'


# Generate logic rules based on given parameters.
def generate_logic_rules() -> List[str]:
    rules: List[str] = []

    if (
        ClingoContext.number_of_nodes > 0
        and ClingoContext.min_depth > ClingoContext.number_of_nodes
    ):
        raise RuntimeError("Received unreasonable parameters.")

    # Creating n symbols.
    symbols = []
    for i in range(ClingoContext.number_of_nodes):
        # The number part is easier for reasoning to generate the graph. We are
        # adding a "S" prefix to each symbol to construct a string. So that the
        # synthesized code will has a valid class/interface name.
        symbols.append(f'"S{i}", {i}')
    # The actual rule will be like,
    # internal_symbols("S0", 0; "S1", 1; "S2", 2).
    rules.append("internal_symbols({}).".format(";".join(symbols)))

    # Creating backbone hierarchy with minimum depth using normal Distribution.
    # We separated the below part from "graph_generator.lp" to avoid "grounding bottleneck."
    # And we are using normal distrubution to create a sequence of extends_to among n nodes.
    interval = ClingoContext.number_of_nodes // ClingoContext.min_depth or 1
    for i in range(interval, ClingoContext.number_of_nodes, interval):
        rules.append(f'extends_to("S{i-interval}", "S{i}").')

    return rules


# Extract logic rules from file format.
def extract_logic_rules(lines: List[str]) -> List[str]:
    rules = []
    symbols = set()
    handlers = {
        "Extends": ExtendEdgeHandler(),
        "Type": TypeEdgeHandler(),
        "Method": MethodEdgeHandler(),
    }

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

        # Dict{"lhs[0]": "handler to convert"}.
        if lhs[0] not in handlers:
            continue
        handler = handlers[lhs[0]]

        # T94428437 Temporary skipping all built-in functions for now.
        if lhs[1].startswith("HH\\"):
            continue

        lhs_tokens = handler.parse(lhs[1])
        # Collecting symbols.
        symbols.add(f'"{lhs_tokens[0]}"')
        # Processing each deps ["Type B", "Type C", "Type D"]
        for dep in rhs.rstrip("\n").split(","):
            # dep = "Type X"
            dep = dep.split()
            if len(dep) != 2:
                # ToDo: Add logging if we needed to track wrong format on rhs.
                continue
            (dep_type, dep_symbol) = dep
            symbols.add(f'"{dep_symbol}"')
            rules.append(handler.rule_writer(lhs_tokens, dep_symbol))

    rules.append("symbols({}).".format(";".join(sorted(symbols))))
    return rules


# Take in a dependency graph and a code generator to emit code.
def do_reasoning(additional_programs: List[str], generator: CodeGenerator) -> None:
    # Logic programs for code synthesis.
    asp_files = os.path.join(
        parutil.get_dir_path("hphp/hack/src/hh_codesynthesis/"), "asp_code"
    )

    # Clingo interfaces.
    ctl = clingo.Control()

    # Load LP for code emitting.
    ctl.load(asp_files + "/dep_graph_reasoning.lp")
    # Load LP for graph generating.
    with open(asp_files + "/graph_generator.lp") as fp:
        ctl.add("base", [], fp.read())
    # Load extra dependency graph given by the user.
    ctl.add("base", [], "\n".join(additional_programs))

    ctl.ground([("base", [])], context=ClingoContext())
    result: Union[clingo.solving.SolveHandle, clingo.solving.SolveResult] = ctl.solve(
        on_model=generator.on_model
    )
    if isinstance(result, clingo.solving.SolveResult):
        if result.unsatisfiable:
            raise RuntimeError("Unsatisfiable.")


# Read dependency graph from file or stdin.
def read_from_file_or_stdin(filename: Optional[str] = None) -> List[str]:
    if filename:
        with open(filename) as fp:
            return fp.readlines()
    # No filename, try stdin.
    return sys.stdin.readlines()


# Write code to file or stdout.
def output_to_file_or_stdout(
    generator: CodeGenerator, filename: Optional[str] = None
) -> int:
    if filename:
        with open(filename, "w") as fp:
            fp.write(str(generator))
    else:
        print(generator)
    return 0


def main() -> int:
    generators = {
        "raw": CodeGenerator,
        "hack": HackCodeGenerator,
    }

    # Parse the arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--input_file", type=os.path.abspath)
    parser.add_argument("--target_lang", type=str)
    parser.add_argument("--output_file", type=os.path.abspath)
    parser.add_argument("--n", type=int)
    parser.add_argument("--avg_width", type=int)
    parser.add_argument("--min_depth", type=int)
    parser.add_argument("--min_classes", type=int)
    parser.add_argument("--min_interfaces", type=int)
    parser.add_argument("--lower_bound", type=int)
    parser.add_argument("--higher_bound", type=int)
    args: argparse.Namespace = parser.parse_args()

    # Set graph generating parameters. (If any)
    ClingoContext.number_of_nodes = args.n or 0
    ClingoContext.avg_width = args.avg_width or 0
    ClingoContext.min_depth = args.min_depth or 1
    ClingoContext.min_classes = args.min_classes or 1
    ClingoContext.min_interfaces = args.min_interfaces or 1
    ClingoContext.lower_bound = args.lower_bound or 1
    ClingoContext.higher_bound = args.higher_bound or 1

    # Load dependency graph.
    lines = read_from_file_or_stdin(filename=args.input_file)
    # T92303034 Temporary handle for the multiple lines like,
    # Extend A -> Type A,
    #             Type B,
    #             Type C,
    #             Type D
    graph = "".join(lines).replace(",\n", ",").split("\n")

    # Output target language.
    generator = generators.get(args.target_lang, CodeGenerator)()

    combined_rules = generate_logic_rules() + extract_logic_rules(graph)

    do_reasoning(combined_rules, generator)
    return output_to_file_or_stdout(generator=generator, filename=args.output_file)


if __name__ == "__main__":
    sys.exit(main())
