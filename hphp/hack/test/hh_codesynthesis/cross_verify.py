#!/usr/bin/env/python3
# pyre-strict
import argparse
import glob
import os
import subprocess
import tempfile
from typing import Dict, List, Set


class DependencyEdges:
    """A simple internal representation to categorize DependencyEdges"""

    edge_types = ["Extends", "Type", "Method", "SMethod", "Fun"]

    def __init__(self, lines: List[str]) -> None:
        super(DependencyEdges, self).__init__()
        self.objs: Dict[str, List[str]] = dict.fromkeys(self.edge_types, [])
        self.category_edges(lines)

    def category_edges(self, lines: List[str]) -> None:
        for line in lines:
            # Required input formatting to get "Extends A -> Type B, Type C, Type D".
            # And split get
            # lhs = "Extends A"
            # rhs = "Type B, Type C, Type D"
            # Skip the empty line at the end of the file.
            if not line:
                continue
            result = line.strip().split("->")
            (lhs, rhs) = result

            lhs = lhs.split()
            # The lhs length must be 2.
            if len(lhs) != 2:
                raise RuntimeError("Unexpected lhs.")

            # T94428437 Temporary skipping all built-in functions for now.
            if lhs[1].startswith("HH\\"):
                continue

            if lhs[0] in self.edge_types:
                self.objs[lhs[0]].append(line)

    def __le__(self, obj: "DependencyEdges") -> bool:
        for edges in self.edge_types:
            compare_result(set(self.objs[edges]), set(obj.objs[edges]))
        return True


def compare_result(lhs: Set[str], rhs: Set[str]) -> None:
    if not lhs.issubset(rhs):
        RuntimeError("Unmatched lhs and rhs, expected lhs be a subset of rhs.")


def invoke_sub_process(cmd: List[str], std_in: str) -> str:
    try:
        output = subprocess.check_output(
            cmd,
            stderr=None,
            cwd=".",
            universal_newlines=True,
            input=std_in,
            timeout=60.0,
            errors="replace",
        )
    except subprocess.TimeoutExpired as e:
        output = "Timed out. " + str(e.output)
    except subprocess.CalledProcessError as e:
        # we don't care about nonzero exit codes... for instance, type
        # errors cause hh_single_type_check to produce them
        output = str(e.output)
    return output


# Cross verify the hh_codesynthesis binary produce the Hack code has identical
# dependency graph as the sample Hack code.
def cross_verify(args: argparse.Namespace, file_name: str) -> bool:
    # 0. Skip unsupported cases for now. (ToDo: T92593014)
    if os.path.exists(file_name + ".skip_synthesis_tests"):
        return True

    # 1. Invoke hh_simple_type_checker to produce a dependency graph on sample.
    with tempfile.NamedTemporaryFile(mode="w") as fp:
        tmp_file_name = fp.name
        cmd = [
            args.typechecker,
            file_name,
            "--dump-deps",
            "--no-builtins",
        ]
        dep_graph = invoke_sub_process(cmd, "")

        # 2. Invoke hh_codesynthesis binary to produce a Hack code given dep_graph.
        cmd = [
            args.synthesis,
            "--target_lang=hack",
            f"--output_file={tmp_file_name}",
        ]
        invoke_sub_process(cmd, dep_graph)

        # 3. Invoke hh_simple_type_checker on
        cmd = [
            args.typechecker,
            f"{tmp_file_name}",
            "--dump-deps",
            "--no-builtins",
        ]
        dep_output = invoke_sub_process(cmd, "")

    # 4. Compare the dep_graph with dep_output.
    dep_graph = dep_graph.replace(",\n", ",").split("\n")
    original_extends_edges = DependencyEdges(dep_graph)

    dep_output = dep_output.replace(",\n", ",").split("\n")
    generate_extends_edges = DependencyEdges(dep_output)
    return original_extends_edges <= generate_extends_edges


# Cross verify the hh_codesynthesis binary produce the Hack code has identical
# dependency graph as the sample parameters.
def cross_verify_with_parameters(args: argparse.Namespace) -> bool:
    with tempfile.NamedTemporaryFile(mode="w") as fp:
        tmp_file_name = fp.name
        # 0. Invoke hh_codesynthesis binary to produce a Hack code from parameters.
        cmd = [
            args.synthesis,
            "--target_lang=hack",
            "--n=12",
            "--avg_width=3",
            "--min_classes=3",
            "--min_interfaces=4",
            "--lower_bound=1",
            "--higher_bound=5",
            "--min_depth=0",
            f"--output_file={tmp_file_name}",
        ]
        invoke_sub_process(cmd, "")

        # 1. Reuse cross_verify using tmp_file_name as input.
        return cross_verify(args, tmp_file_name)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("test_path", help="A file or a directory. ")
    parser.add_argument("--typechecker", type=os.path.abspath)
    parser.add_argument("--synthesis", type=os.path.abspath)

    args: argparse.Namespace = parser.parse_args()

    # Cross verify synthesized from given raph.
    for file_name in glob.glob(args.test_path + "/*.php"):
        cross_verify(args, file_name)

    # Cross verify synthesized from parameters.
    cross_verify_with_parameters(args)
