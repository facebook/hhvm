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

    def na(self) -> Symbol:
        return Number(100)

    def nr(self) -> Symbol:
        return Number(5)

    def nl(self) -> Symbol:
        return Number(30)

    def ne(self) -> Symbol:
        return Number(100)

    def nia(self) -> Symbol:
        return Number(20)

    def npa(self) -> Symbol:
        return Number(60)

    def infra_agent_out_degree_low(self, agent: Number) -> Symbol:
        agent_number = agent.number
        if agent_number < 10:
            return Number(0)
        else:
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

        similar_agents = 0
        checked_agents = set()
        for key, value in enumerate(self.edges):
            for number, agent in enumerate(self.edges[key + 1 :]):
                if value == agent and len(value) != 0 and number not in checked_agents:
                    similar_agents += 1
                    checked_agents.add(number)

        print("There are {0} similar agents.".format(similar_agents))


agent_graph = AgentGraphGenerator()

# Logic programs for code synthesis.
asp_files = "hphp/hack/src/hh_codesynthesis"

# Clingo interfaces.
ctl = clingo.Control()

# Load LP for agent graph generating.
ctl.load(asp_files + "/agent_graph_generator.lp")
ctl.ground([("base", [])], context=AgentGraphClingoContext())

result: Union[clingo.solving.SolveHandle, clingo.solving.SolveResult] = ctl.solve(
    on_model=agent_graph.evaluate
)
if isinstance(result, clingo.solving.SolveResult):
    if result.unsatisfiable:
        raise RuntimeError("Unsatisfiable.")
