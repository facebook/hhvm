#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
# This library will construct a logic program using the given parameter,
# invoke Clingo to solve the logic program to produce an agent graph,
# and we create multiple agents in parallel to produce valid codebase.
import argparse
import sys

from typing import List, Optional, Set, Union

import clingo
from clingo.symbol import Number, Symbol


class AgentGraphClingoContext:
    """Context class interact with Python and Clingo.
    We can customize each value, range, and function to experiment on
    different settings. Refer to the test case `test_small_agent_graph`
    for more detail usages."""

    number_of_leaves = 30
    number_of_infra_agents = 20
    number_of_product_agents = 60
    infra_agent_profile = {"in_degree": [0, 10], "out_degree": [5, 100]}
    product_agent_profile = {"in_degree": [5, 20], "out_degree": [0, 5]}

    def nl(self) -> Symbol:
        return Number(self.number_of_leaves)

    def nia(self) -> Symbol:
        return Number(self.number_of_infra_agents)

    def npa(self) -> Symbol:
        return Number(self.number_of_product_agents)

    def infra_agent_out_degree_low(self, agent: Number) -> Symbol:
        return Number(self.infra_agent_profile["out_degree"][0])

    def infra_agent_out_degree_high(self, agent: Number) -> Symbol:
        return Number(self.infra_agent_profile["out_degree"][1])

    def infra_agent_in_degree_low(self, agent: Number) -> Symbol:
        return Number(self.infra_agent_profile["in_degree"][0])

    def infra_agent_in_degree_high(self, agent: Number) -> Symbol:
        return Number(self.infra_agent_profile["in_degree"][1])

    def product_agent_out_degree_low(self, agent: Number) -> Symbol:
        return Number(self.product_agent_profile["out_degree"][0])

    def product_agent_out_degree_high(self, agent: Number) -> Symbol:
        return Number(self.product_agent_profile["out_degree"][1])

    def product_agent_in_degree_low(self, agent: Number) -> Symbol:
        return Number(self.product_agent_profile["in_degree"][0])

    def product_agent_in_degree_high(self, agent: Number) -> Symbol:
        return Number(self.product_agent_profile["in_degree"][1])

    def agent_connector(
        self,
        source_agent: Number,
        target_agent: Number,
        source_level: Number,
        target_level: Number,
    ) -> Symbol:
        src_agent_number = source_agent.number
        tgt_agent_number = target_agent.number
        src_level_number = source_level.number
        tgt_level_number = target_level.number
        # Since clingo doesn't support Boolean return, we are returning
        # Number(1) for true, and Number(0) for false.
        if (
            src_agent_number < tgt_agent_number + 200
            and tgt_agent_number < src_agent_number
            and tgt_level_number < src_level_number
        ):
            return Number(1)
        else:
            return Number(0)

    def agent_type_connector(
        self, source_agent: Number, target_agent: Number
    ) -> Symbol:
        src_agent_number = source_agent.number
        tgt_agent_number = target_agent.number
        # Since clingo doesn't support Boolean return, we are returning
        # Number(1) for true, and Number(0) for false.
        if tgt_agent_number * 3 + 200 < src_agent_number:
            return Number(1)
        else:
            return Number(0)


class AgentGraphGenerator:
    """A generator that could produce an agent graph. We are providing an
    evaluation function along with an actual generate function. So that
    the user can do a dry run to evaluate the quality of actual graph.
    The user must specify which on_model is going to use."""

    _raw_model = ""

    def __init__(self) -> None:
        self.infra_agents: List[int] = []
        self.product_agents: List[int] = []
        self.edges: List[Set] = []

    def infra_agent_evaluate(self, agent_number: int) -> None:
        self.infra_agents.append(agent_number)

    def product_agent_evaluate(self, agent_number: int) -> None:
        self.product_agents.append(agent_number)

    def edge_evaluate(self, left_agent: int, right_agent: int) -> None:
        self.edges[left_agent].add(right_agent)

    def evaluate(self, m: clingo.Model) -> None:
        self.infra_agents = []
        self.product_agents = []

        predicates = m.symbols(atoms=True)
        node_func = {
            "infra_agent": self.infra_agent_evaluate,
            "product_agent": self.product_agent_evaluate,
        }
        edge_func = {"depends_on": self.edge_evaluate}

        for predicate in predicates:
            if predicate.name in node_func:
                node_func[predicate.name](predicate.arguments[0].number)

        self.edges = [
            set() for x in range(len(self.infra_agents) + len(self.product_agents))
        ]

        for predicate in predicates:
            if predicate.name in edge_func:
                edge_func[predicate.name](
                    predicate.arguments[0].number, predicate.arguments[1].number
                )

        print("Number of infra agents: {0}".format(len(self.infra_agents)))
        print("Number of product agents: {0}".format(len(self.product_agents)))
        print("Number of edges: {0}".format(sum([len(x) for x in self.edges])))

        similar_agents = 0
        checked_agents = set()
        similar_relations = []
        for outer_agent, outer_dependents in enumerate(self.edges):
            for inner_agent, inner_dependents in enumerate(
                self.edges[outer_agent + 1 :]
            ):
                if (
                    outer_dependents == inner_dependents
                    and len(outer_dependents) != 0
                    and inner_agent not in checked_agents
                ):
                    similar_agents += 1
                    checked_agents.add(inner_agent)
                    if inner_dependents not in similar_relations:
                        similar_relations.append(inner_dependents)

        print("There are {0} similar agents.".format(similar_agents))
        print("Common dependents set is {0}".format(similar_relations))

    def generate_raw(self, m: clingo.Model) -> None:
        self._raw_model = m.__str__()

    def on_model(self, m: clingo.Model) -> None:
        raise RuntimeError("Must specify a valid method.")


# Generate a set of agents using given distribution. The distribution has the
# critical path requirement (levels) as well as how many roots are in the graph.
def generating_agent_distribution(agent_distribution: List[int]) -> List[str]:
    if not all(i > 0 for i in agent_distribution):
        raise RuntimeError("Agent distribution must have all positive integers.")
    # The levels in the generated agent graph is the length of this list.
    return [
        "agents({0}..{1}, {2}).".format(
            sum(agent_distribution[:level]),
            sum(agent_distribution[:level]) + number_of_agents_at_this_level - 1,
            level,
        )
        for level, number_of_agents_at_this_level in enumerate(agent_distribution)
    ]


# Take in an agent distribution and a generator to create an agent graph.
def generating_an_agent_graph(
    agent_distribution: List[int], generator: AgentGraphGenerator
) -> None:
    # Logic programs for code synthesis.
    asp_files = "hphp/hack/src/hh_codesynthesis"

    # Clingo interfaces.
    ctl = clingo.Control()

    # Load LP for agent graph generating.
    ctl.load(asp_files + "/agent_graph_generator.lp")
    # Load LP for agent distribution.
    ctl.add("base", [], "\n".join(generating_agent_distribution(agent_distribution)))
    ctl.ground([("base", [])], context=AgentGraphClingoContext())

    result: Union[clingo.solving.SolveHandle, clingo.solving.SolveResult] = ctl.solve(
        on_model=generator.on_model
    )
    if isinstance(result, clingo.solving.SolveResult):
        if result.unsatisfiable:
            raise RuntimeError("Unsatisfiable.")


def main() -> int:
    agent_graph = AgentGraphGenerator()

    agent_graph.on_model = agent_graph.generate_raw

    # Parse the arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--agents", nargs="+", type=int)

    parser.add_argument("--number_of_infra_agents", type=int)
    parser.add_argument("--number_of_product_agents", type=int)
    parser.add_argument("--number_of_leaves", type=int)

    parser.add_argument("--infra_agent_indegrees", nargs=2, type=int)
    parser.add_argument("--infra_agent_outdegrees", nargs=2, type=int)
    parser.add_argument("--product_agent_indegrees", nargs=2, type=int)
    parser.add_argument("--product_agent_outdegrees", nargs=2, type=int)

    parser.add_argument('--evaluate', action=argparse.BooleanOptionalAction)

    args: argparse.Namespace = parser.parse_args()

    agent_distribution = args.agents or [5, 10, 10, 10, 10, 10, 10, 15, 20]
    AgentGraphClingoContext.number_of_infra_agents = args.number_of_infra_agents or 20
    AgentGraphClingoContext.number_of_product_agents = (
        args.number_of_product_agents or 60
    )
    AgentGraphClingoContext.number_of_leaves = args.number_of_leaves or 30
    AgentGraphClingoContext.infra_agent_profile[
        "in_degree"
    ] = args.infra_agent_indegrees or [0, 10]
    AgentGraphClingoContext.infra_agent_profile[
        "out_degree"
    ] = args.infra_agent_outdegrees or [5, 100]
    AgentGraphClingoContext.product_agent_profile[
        "in_degree"
    ] = args.product_agent_indegrees or [5, 20]
    AgentGraphClingoContext.product_agent_profile[
        "out_degree"
    ] = args.product_agent_outdegrees or [0, 5]

    if args.evaluate:
        agent_graph.on_model = agent_graph.evaluate
    generating_an_agent_graph(agent_distribution, agent_graph)
    print(agent_graph._raw_model)


if __name__ == "__main__":
    sys.exit(main())
