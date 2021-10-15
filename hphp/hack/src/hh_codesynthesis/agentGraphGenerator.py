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
from typing import Callable, Dict, List, Optional, Set, Union

import clingo
from clingo.symbol import Number, Symbol


class AgentGraphClingoContext:
    """Context class interact with Python and Clingo.
    We can customize each value, range, and function to experiment on
    different settings. Refer to the test case `test_small_agent_graph`
    for more detail usages."""

    def __init__(
        self,
        number_of_leaves: int,
        number_of_infra_agents: int,
        number_of_product_agents: int,
        infra_agent_profile: Dict[str, List[int]],
        product_agent_profile: Dict[str, List[int]],
    ) -> None:
        if (
            number_of_leaves <= 0
            or number_of_infra_agents <= 0
            or number_of_product_agents <= 0
            or len(infra_agent_profile) < 2
            or len(product_agent_profile) < 2
        ):
            raise RuntimeError("Invalid agent graph metrics.")
        self.number_of_leaves = number_of_leaves
        self.number_of_infra_agents = number_of_infra_agents
        self.number_of_product_agents = number_of_product_agents
        self.infra_agent_profile = infra_agent_profile
        self.product_agent_profile = product_agent_profile

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

    # A connector that decide whether or not two agents can be connected using
    # their number and level. We are not connecting two agents here, only added
    # a choice to the solver, the solver can decide how to connect two agents
    # based on other constraints.
    # @param source_agent, the number of source agent.
    # @param target_agent, the number of target agent.
    # @param source_level, the level of source agent.
    # @param target_level, the level of target agent.
    # @return 1 if source_argent can be connected to target_agent.
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

    # Similar to `agent_connector`, this connector works for deciding a product
    # agent could have an edge to an infra agent or not. We are not connecting
    # two agents here, only added a choice to the solver, the solver can decide
    # how to connect two agents based on other constraints.
    # @param source_agent, the number of source agent.
    # @param target_agent, the number of target agent.
    # @return 1 if product argent can be connected to infra agent.
    def product_and_infra_agent_connector(
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

    def __init__(
        self, agent_distribution: List[int], solving_context: AgentGraphClingoContext
    ) -> None:
        self.infra_agents: List[int] = []
        self.product_agents: List[int] = []
        self.edges: List[Set] = []
        self._raw_model = ""
        self.agent_distribution = agent_distribution
        self.solving_context = solving_context

    def add_infra_agent(self, agent_number: int) -> None:
        self.infra_agents.append(agent_number)

    def add_product_agent(self, agent_number: int) -> None:
        self.product_agents.append(agent_number)

    def add_edge(self, left_agent: int, right_agent: int) -> None:
        self.edges[left_agent].add(right_agent)

    def validate_range_in_profile(self, degrees: List[int], direction: str) -> bool:
        for node, degree in enumerate(degrees):
            # Select a right profile to use.
            if node in self.infra_agents:
                agent_profile = self.solving_context.infra_agent_profile
            elif node in self.product_agents:
                agent_profile = self.solving_context.product_agent_profile
            else:
                raise RuntimeError("Can't find a profile for agent {0}".format(node))

            # Check the degree within the range or not.
            assert (
                degree >= agent_profile[direction][0]
                and degree < agent_profile[direction][1]
            ), "Node {0}'s {1}: {2} is out of range {3}, {4}.".format(
                node,
                direction,
                degree,
                agent_profile[direction][0],
                agent_profile[direction][1],
            )

        return True

    def validate(
        self,
        customize_validator: Optional[
            Callable[[int, int, List[int], List[int]], bool]
        ] = None,
    ) -> bool:
        # Graph validation using the constraints specified in the context.
        assert (
            len(self.infra_agents) >= self.solving_context.number_of_infra_agents
        ), "Expected to get at least {0}, but only have {1} infra agents.".format(
            len(self.infra_agents), self.solving_context.number_of_infra_agents
        )
        assert (
            len(self.product_agents) >= self.solving_context.number_of_product_agents
        ), "Expected to get at least {0}, but only have {1} product agents.".format(
            len(self.product_agents), self.solving_context.number_of_product_agents
        )

        # Compute in/out degree.
        in_degrees = [0] * sum(self.agent_distribution)
        out_degrees = [0] * sum(self.agent_distribution)
        # Iterate through adjacency matrix.
        for node, depends_on in enumerate(self.edges):
            in_degrees[node] += len(depends_on)
            for x in depends_on:
                out_degrees[x] += 1

        # Validate the degrees are in the range specified by the profile.
        self.validate_range_in_profile(in_degrees, "in_degree")
        self.validate_range_in_profile(out_degrees, "out_degree")

        # Customized range function needs a customized validator.
        if customize_validator is not None:
            return customize_validator(
                self.infra_agents, self.product_agents, in_degrees, out_degrees
            )

        return True

    def evaluate(self, m: clingo.Model) -> None:
        self.generate(m)

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

    def generate(self, m: clingo.Model) -> None:
        self._raw_model = m.__str__()

        self.infra_agents = []
        self.product_agents = []

        predicates = m.symbols(atoms=True)
        node_func = {
            "infra_agent": self.add_infra_agent,
            "product_agent": self.add_product_agent,
        }
        edge_func = {"depends_on": self.add_edge}

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

        print(self._raw_model)

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
def generating_an_agent_graph(generator: AgentGraphGenerator) -> None:
    # Logic programs for code synthesis.
    asp_files = "hphp/hack/src/hh_codesynthesis"

    # Clingo interfaces.
    ctl = clingo.Control()

    # Load LP for agent graph generating.
    ctl.load(asp_files + "/agent_graph_generator.lp")
    # Load LP for agent distribution.
    ctl.add(
        "base",
        [],
        "\n".join(generating_agent_distribution(generator.agent_distribution)),
    )
    ctl.ground([("base", [])], context=generator.solving_context)

    result: Union[clingo.solving.SolveHandle, clingo.solving.SolveResult] = ctl.solve(
        on_model=generator.on_model
    )
    if isinstance(result, clingo.solving.SolveResult):
        if result.unsatisfiable:
            raise RuntimeError("Unsatisfiable.")


def main() -> None:
    # Parse the arguments.
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--agents",
        nargs="+",
        type=int,
        default=[5, 10, 10, 10, 10, 10, 10, 15, 20],
        help=(
            "A sequence of numbers indicating the number of agents at each level. For"
            " example, [2, 4, 4] means the agent graph has three levels in it, and each"
            " level has 2, 4, 4 agents respectively."
        ),
    )

    parser.add_argument(
        "--number_of_infra_agents",
        type=int,
        default=20,
        help="Number of infra agents in the generated agent graph.",
    )
    parser.add_argument(
        "--number_of_product_agents",
        type=int,
        default=60,
        help="Number of product agents in the generated agent graph.",
    )
    parser.add_argument(
        "--number_of_leaves",
        type=int,
        default=30,
        help="Number of leaves in the generated agent graph.",
    )

    parser.add_argument(
        "--infra_agent_indegrees",
        nargs=2,
        type=int,
        default=[0, 10],
        help=(
            "A boundary for describing one infra agent can dependent on how many other"
            " agents."
        ),
    )
    parser.add_argument(
        "--infra_agent_outdegrees",
        nargs=2,
        type=int,
        default=[5, 100],
        help=(
            "A boundary for describing how many other agents can depent on one infra"
            " agent."
        ),
    )
    parser.add_argument(
        "--product_agent_indegrees",
        nargs=2,
        type=int,
        default=[5, 20],
        help=(
            "A boundary for describing one product agent can dependent on how many"
            " other agents."
        ),
    )
    parser.add_argument(
        "--product_agent_outdegrees",
        nargs=2,
        type=int,
        default=[0, 5],
        help=(
            "A boundary for describing how many other agents can depent on one product"
            " agent."
        ),
    )

    parser.add_argument("--evaluate", action=argparse.BooleanOptionalAction)

    args: argparse.Namespace = parser.parse_args()

    # Setup generator and context.
    agent_graph_generator = AgentGraphGenerator(
        agent_distribution=args.agents,
        solving_context=AgentGraphClingoContext(
            number_of_infra_agents=args.number_of_infra_agents,
            number_of_product_agents=args.number_of_product_agents,
            number_of_leaves=args.number_of_leaves,
            infra_agent_profile={
                "in_degree": args.infra_agent_indegrees,
                "out_degree": args.infra_agent_outdegrees,
            },
            product_agent_profile={
                "in_degree": args.product_agent_indegrees,
                "out_degree": args.product_agent_outdegrees,
            },
        ),
    )
    agent_graph_generator.on_model = agent_graph_generator.generate
    if args.evaluate:
        agent_graph_generator.on_model = agent_graph_generator.evaluate
    generating_an_agent_graph(agent_graph_generator)


if __name__ == "__main__":
    sys.exit(main())
