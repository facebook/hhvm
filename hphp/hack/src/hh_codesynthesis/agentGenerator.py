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
import importlib
import logging
import os
import sys
from typing import List, Optional, Union

import clingo
from hphp.hack.src.hh_codesynthesis.codeGenerator import ClingoContext, CodeGenerator
from hphp.hack.src.hh_codesynthesis.hackGenerator import HackCodeGenerator

# If libfb.py library exists, we run in the internal environment.
try:
    importlib.util.find_spec("libfb.py")
    from libfb.py import parutil

    g_internal_run = True
except ModuleNotFoundError:
    g_internal_run = False


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


class SMethodEdgeHandler(DependencyEdgeHandler):
    def parse(self, lhs: str) -> List[str]:
        return self.lhs_parser(lhs, "::")

    def rule_writer(self, lhs: List[str], rhs: str) -> str:
        return f'static_method("{lhs[0]}", "{lhs[1]}", "{rhs}").'


class FunEdgeHandler(DependencyEdgeHandler):
    def parse(self, lhs: str) -> List[str]:
        return self.lhs_parser(lhs, None)

    def rule_writer(self, lhs: List[str], rhs: str) -> str:
        return f'invoked_by("{lhs[0]}", "{rhs}").'


# Generate logic rules based on given parameters.
def generate_logic_rules(
    solving_context: ClingoContext, agent_name: str = ""
) -> List[str]:
    rules: List[str] = []

    if solving_context.number_of_nodes > 0 and (
        solving_context.min_depth > solving_context.number_of_nodes
        or sum(solving_context.degree_distribution) > solving_context.number_of_nodes
    ):
        raise RuntimeError("Received unreasonable parameters.")

    # Creating n symbols.
    symbols = []
    for i in range(solving_context.number_of_nodes):
        # The number part is easier for reasoning to generate the graph. We are
        # adding a "S" prefix to each symbol to construct a string. So that the
        # synthesized code will has a valid class/interface name.
        symbols.append(f'"{agent_name}S{i}", {i}')
    # The actual rule will be like,
    # internal_symbols("S0", 0; "S1", 1; "S2", 2).
    rules.append("internal_symbols({}).".format(";".join(symbols)))

    # Creating backbone hierarchy with minimum depth using normal Distribution.
    # We separated the below part from "graph_generator.lp" to avoid "grounding bottleneck."
    # And we are using normal distrubution to create a sequence of extends_to among n nodes.
    interval = solving_context.number_of_nodes // solving_context.min_depth or 1
    for i in range(interval, solving_context.number_of_nodes, interval):
        rules.append(f'extends_to("S{i-interval}", "S{i}").')

    # Creating a node distribution for each degree.
    # We separated the below part from "graph_generator.lp" to narrow down the
    # search scope.
    for degree, minimum_nodes in enumerate(solving_context.degree_distribution):
        rules.append(f":- #count{{X : in_degree(X, {degree})}} < {minimum_nodes}.")

    return rules


# Extract logic rules from file format.
def extract_logic_rules(lines: List[str]) -> List[str]:
    rules = []
    symbols = set()
    funcs = set()
    handlers = {
        "Extends": ExtendEdgeHandler(),
        "Type": TypeEdgeHandler(),
        "Method": MethodEdgeHandler(),
        "SMethod": SMethodEdgeHandler(),
        "Fun": FunEdgeHandler(),
    }
    collectors = {
        "Extends": symbols,
        "Type": symbols,
        "Method": symbols,
        "SMethod": symbols,
        "Fun": funcs,
    }

    for line in lines:
        # Required input formatting to get "Extends A -> Type B, Type C, Type D".
        # And split get
        # lhs = "Extends A"
        # rhs = "Type B, Type C, Type D"

        # T94428437 Temporary skipping all built-in functions for now.
        # T92593014 We do not support namespace at this moment.
        # HH\ PHP\ FB\Vec namespace\class etc.
        if "\\" in line:
            continue

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

        lhs_tokens = handler.parse(lhs[1])
        # Updating collections.
        collector = collectors.get(lhs[0], symbols)
        collector.add(f'"{lhs_tokens[0]}"')
        # Processing each deps ["Type B", "Type C", "Type D", "Fun E"].
        for dep in rhs.rstrip("\n").split(","):
            # dep = "Type X" / "Fun X".
            dep = dep.split()
            if len(dep) != 2:
                # ToDo: Add logging if we needed to track wrong format on rhs.
                continue
            (dep_type, dep_symbol) = dep
            # Right hand side could only be "Type"/"Fun".
            if dep_type not in ["Type", "Fun"]:
                raise NotImplementedError(
                    f"Not supported {dep_type} on the right hand side."
                )
            collector = collectors.get(dep_type, symbols)
            collector.add(f'"{dep_symbol}"')
            rules.append(handler.rule_writer(lhs_tokens, dep_symbol))

    rules.append("symbols({}).".format(";".join(sorted(symbols))))
    if len(funcs) != 0:
        rules.append("funcs({}).".format(";".join(sorted(funcs))))

    return rules


# Take in a dependency graph and a code generator to emit code.
def do_reasoning(additional_programs: List[str], generator: CodeGenerator) -> None:
    # Logic programs for code synthesis.

    asp_files = "hphp/hack/src/hh_codesynthesis"
    if g_internal_run:
        # Check if we are running in the internal environment.
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

    ctl.ground([("base", [])], context=generator.solving_context)
    # ToDo: Hardcode the number of threads for now, change to parameter later.
    # Pyre-ignore: [16] Configuration not in pyre stubs since it's dynamic
    ctl.configuration.solve.parallel_mode = "4"
    # Pyre-ignore: [16] Configuration not in pyre stubs since it's dynamic
    ctl.configuration.solve.models = generator.model_count

    logging.info("Finished grounding.")
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
    generators = {"raw": CodeGenerator, "hack": HackCodeGenerator}

    # Parse the arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--input_file", type=os.path.abspath)
    parser.add_argument("--target_lang", type=str)
    parser.add_argument("--output_file", type=os.path.abspath)
    parser.add_argument("--n", type=int, default=0)
    parser.add_argument("--min_depth", type=int, default=1)
    parser.add_argument("--min_classes", type=int, default=1)
    parser.add_argument("--min_interfaces", type=int, default=1)
    parser.add_argument("--min_stub_classes", type=int, default=0)
    parser.add_argument("--min_stub_interfaces", type=int, default=0)
    # Parameters that narrow the search space to speed up the computation.
    parser.add_argument("--degree_distribution", nargs="*", default=[], type=int)
    parser.add_argument("--lower_bound", type=int, default=1)
    parser.add_argument("--higher_bound", type=int, default=1)
    parser.add_argument("--log", type=str)
    args: argparse.Namespace = parser.parse_args()

    # Setup log level and mark the start of our program.
    log_level = getattr(logging, args.log.upper(), logging.WARN)
    logging.basicConfig(
        format="%(asctime)s %(message)s",
        datefmt="%Y/%m/%d %I:%M:%S %p",
        level=log_level,
    )
    logging.info("Started.")

    # Set graph generating parameters. (If any)
    solving_context = ClingoContext(
        number_of_nodes=args.n,
        min_depth=args.min_depth,
        min_classes=args.min_classes,
        min_interfaces=args.min_interfaces,
        min_stub_classes=args.min_stub_classes,
        min_stub_interfaces=args.min_stub_interfaces,
        degree_distribution=args.degree_distribution,
        lower_bound=args.lower_bound,
        higher_bound=args.higher_bound,
    )

    # Load dependency graph.
    lines = read_from_file_or_stdin(filename=args.input_file)
    # T92303034 Temporary handle for the multiple lines like,
    # Extend A -> Type A,
    #             Type B,
    #             Type C,
    #             Type D
    graph = "".join(lines).replace(",\n", ",").split("\n")

    # Output target language.
    generator = generators.get(args.target_lang, CodeGenerator)(solving_context)

    combined_rules = generate_logic_rules(solving_context) + extract_logic_rules(graph)

    logging.info("Extracted all rules.")
    logging.info(f"Number of depedency edges extracted: {len(combined_rules)}")

    do_reasoning(combined_rules, generator)
    logging.info("Finished reasoning.")
    return output_to_file_or_stdout(generator=generator, filename=args.output_file)


def invoke_main() -> None:
    sys.exit(main())


if __name__ == "__main__":
    invoke_main()
