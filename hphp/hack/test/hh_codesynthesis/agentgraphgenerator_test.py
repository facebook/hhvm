#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
import unittest
from typing import List

from clingo.symbol import Number, Symbol
from hphp.hack.src.hh_codesynthesis import agentGraphGenerator
from hphp.hack.src.hh_codesynthesis.agentGraphGenerator import (
    AgentGraphClingoContext,
    AgentGraphGenerator,
)


class GeneratingAgentDistributionTest(unittest.TestCase):
    def test_single_level(self) -> None:
        exp = ["agents(0..2, 0)."]
        self.assertListEqual(
            exp, agentGraphGenerator.generating_agent_distribution([3])
        )

    def test_multiple_levels(self) -> None:
        exp = [
            "agents(0..4, 0).",
            "agents(5..14, 1).",
            "agents(15..24, 2).",
            "agents(25..34, 3).",
            "agents(35..44, 4).",
            "agents(45..54, 5).",
            "agents(55..64, 6).",
            "agents(65..79, 7).",
            "agents(80..99, 8).",
        ]
        self.assertListEqual(
            exp,
            agentGraphGenerator.generating_agent_distribution(
                [5, 10, 10, 10, 10, 10, 10, 15, 20]
            ),
        )

    def test_negative_distribution(self) -> None:
        # Given a distribution with non-positive number.
        with self.assertRaises(
            expected_exception=RuntimeError,
            msg="Agent distribution must have all positive integers.",
        ):
            agentGraphGenerator.generating_agent_distribution([2, -1, 3])

        with self.assertRaises(
            expected_exception=RuntimeError,
            msg="Agent distribution must have all positive integers.",
        ):
            agentGraphGenerator.generating_agent_distribution([2, 0, 3])


class GeneratingAnAgentGraphTest(unittest.TestCase):
    def test_small_agent_graph(self) -> None:
        # We are creating an agent graph with three levels, each level
        # has 2, 4, 4 agents respectively. The minimum number of infra
        # agents are 2, the minimum number of product agents are 6.
        # The number of leaves in the graph are 5. And each agent type
        # has its profile to describe the relationship boundary.
        agent_graph_generator = AgentGraphGenerator(
            agent_distribution=[2, 4, 4],
            solving_context=AgentGraphClingoContext(
                number_of_leaves=5,
                number_of_infra_agents=2,
                number_of_product_agents=6,
                infra_agent_profile={"in_degree": [0, 10], "out_degree": [1, 10]},
                product_agent_profile={"in_degree": [1, 10], "out_degree": [0, 5]},
            ),
        )
        agent_graph_generator.on_model = agent_graph_generator.generate

        agentGraphGenerator.generating_an_agent_graph(agent_graph_generator)
        self.assertTrue(agent_graph_generator.validate())

        # Change product agent profile, expect a product agent to depend on at
        # least two other agents.
        agent_graph_generator.solving_context.product_agent_profile["in_degree"][0] = 2
        agentGraphGenerator.generating_an_agent_graph(agent_graph_generator)
        self.assertTrue(agent_graph_generator.validate())

        # Change the number of infra agents to 3, and customize a indegree lower
        # bound function, so that infra_agent(N), N > 2 must depend on infra_agent(0)
        # and infra_agent(1).
        agent_graph_generator.solving_context.number_of_infra_agents = 3

        def customize_infra_agent_in_degree_low(agent: Number) -> Symbol:
            if agent.number < 2:
                return Number(0)
            else:
                return Number(2)

        def customize_validator(
            infra_agents: int,
            product_agents: int,
            in_degrees: List[int],
            out_degrees: List[int],
        ) -> bool:
            for agent_number, degree in enumerate(in_degrees):
                if agent_number in infra_agents and agent_number < 2 and degree > 0:
                    return False
                if agent_number in infra_agents and agent_number >= 2 and degree > 2:
                    return False
            return True

        agent_graph_generator.solving_context.infra_agent_in_degree_low = (
            customize_infra_agent_in_degree_low
        )

        agentGraphGenerator.generating_an_agent_graph(agent_graph_generator)
        self.assertTrue(agent_graph_generator.validate(customize_validator))

    def test_unsatisfiable_parameters(self) -> None:
        # The total of infra agents + product agents are greater than sum(agent_distribution).
        agent_graph_generator = AgentGraphGenerator(
            agent_distribution=[2, 4, 4],
            solving_context=AgentGraphClingoContext(
                number_of_leaves=5,
                number_of_infra_agents=5,
                number_of_product_agents=6,
                infra_agent_profile={"in_degree": [0, 10], "out_degree": [1, 10]},
                product_agent_profile={"in_degree": [1, 10], "out_degree": [0, 5]},
            ),
        )
        agent_graph_generator.on_model = agent_graph_generator.generate

        with self.assertRaises(expected_exception=RuntimeError, msg="Unsatisfiable."):
            agentGraphGenerator.generating_an_agent_graph(agent_graph_generator)
