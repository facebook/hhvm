#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
#
from typing import List, Optional, Set, Union

import clingo
from clingo.symbol import Number, Symbol


class AgentGraphClingoContext:
    """ """

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
        self, source_agent, target_agent, source_level, target_level
    ) -> Symbol:
        src_agent_number = source_agent.number
        tgt_agent_number = target_agent.number
        src_level_number = source_level.number
        tgt_level_number = target_level.number
        if (
            src_agent_number < tgt_agent_number + 200
            and tgt_agent_number < src_agent_number
            and tgt_level_number < src_level_number
        ):
            return Number(1)
        else:
            return Number(0)

    def agent_type_connector(self, source_agent, target_agent) -> Symbol:
        src_agent_number = source_agent.number
        tgt_agent_number = target_agent.number
        if tgt_agent_number * 3 + 200 < src_agent_number:
            return Number(1)
        else:
            return Number(0)


class AgentGraphGenerator:
    def __init__(self):
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
        for key, value in enumerate(self.edges):
            for number, agent in enumerate(self.edges[key + 1 :]):
                if value == agent and len(value) != 0 and number not in checked_agents:
                    similar_agents += 1
                    checked_agents.add(number)
                    if agent not in similar_relations:
                        similar_relations.append(agent)

        print("There are {0} similar agents.".format(similar_agents))
        print(similar_relations)

    def on_model(self, m: clingo.Model) -> None:
        raise RuntimeError("Must specify either evaluate or generate.")


def generating_agent_distribution(agent_distribution: List[int]) -> List[str]:
    # The levels in the generated agent graph is the length of this list.
    return [
        "agents({0}..{1}, {2}).".format(
            sum(agent_distribution[:level]),
            sum(agent_distribution[:level]) + number_of_agents_at_this_level - 1,
            level,
        )
        for level, number_of_agents_at_this_level in enumerate(agent_distribution)
    ]


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
        on_model=agent_graph.on_model
    )
    if isinstance(result, clingo.solving.SolveResult):
        if result.unsatisfiable:
            raise RuntimeError("Unsatisfiable.")


if __name__ == "__main__":
    agent_graph = AgentGraphGenerator()
    agent_graph.on_model = agent_graph.evaluate

    scale = 10

    agent_distribution = [5, 10, 10, 10, 10, 10, 10, 15, 20]
    agent_distribution = [x * scale for x in agent_distribution]

    AgentGraphClingoContext.number_of_infra_agents = 20 * scale
    AgentGraphClingoContext.number_of_product_agents = 60 * scale

    generating_an_agent_graph(agent_distribution, agent_graph)
