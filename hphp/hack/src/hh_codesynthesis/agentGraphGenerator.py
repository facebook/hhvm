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

    scale = 10

    def na(self) -> Symbol:
        return Number(100 * self.scale)

    def nr(self) -> Symbol:
        return Number(5)

    def nl(self) -> Symbol:
        return Number(30)

    def nia(self) -> Symbol:
        return Number(20 * self.scale)

    def npa(self) -> Symbol:
        return Number(60 * self.scale)

    def s(self) -> Symbol:
        return Number(self.scale)

    def infra_agent_out_degree_low(self, agent: Number) -> Symbol:
        return Number(5)

    def infra_agent_out_degree_high(self, agent: Number) -> Symbol:
        return Number(100)

    def infra_agent_in_degree_low(self, agent: Number) -> Symbol:
        return Number(0)

    def infra_agent_in_degree_high(self, agent: Number) -> Symbol:
        return Number(10)

    def product_agent_out_degree_low(self, agent: Number) -> Symbol:
        return Number(0)

    def product_agent_out_degree_high(self, agent: Number) -> Symbol:
        return Number(5)

    def product_agent_in_degree_low(self, agent: Number) -> Symbol:
        return Number(5)

    def product_agent_in_degree_high(self, agent: Number) -> Symbol:
        return Number(20)

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


def generating_an_agent_graph(generator: AgentGraphGenerator):
    # Logic programs for code synthesis.
    asp_files = "hphp/hack/src/hh_codesynthesis"

    # Clingo interfaces.
    ctl = clingo.Control()

    # Load LP for agent graph generating.
    ctl.load(asp_files + "/agent_graph_generator.lp")
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

    generating_an_agent_graph(agent_graph)
