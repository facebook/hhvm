#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
import unittest

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
        agent_graph_generator = AgentGraphGenerator()
        agent_graph_generator.on_model = agent_graph_generator.generate_raw
        agent_distribution = [2, 4, 4]

        AgentGraphClingoContext.number_of_infra_agents = 2
        AgentGraphClingoContext.number_of_product_agents = 6
        AgentGraphClingoContext.number_of_leaves = 5
        AgentGraphClingoContext.infra_agent_profile = {
            "in_degree": [0, 10],
            "out_degree": [1, 10],
        }
        AgentGraphClingoContext.product_agent_profile = {
            "in_degree": [1, 10],
            "out_degree": [0, 5],
        }

        exp = (
            "infra_agent(0) infra_agent(1) product_agent(2) product_agent(3)"
            " product_agent(4) product_agent(5) product_agent(6) product_agent(7)"
            " product_agent(8) product_agent(9) depends_on(2,0) depends_on(3,0)"
            " depends_on(4,1) depends_on(5,0) depends_on(6,1) depends_on(7,0)"
            " depends_on(8,1) depends_on(9,1)"
        )

        agentGraphGenerator.generating_an_agent_graph(
            agent_distribution, agent_graph_generator
        )
        self.assertListEqual(
            sorted(exp.split("\n")),
            sorted(agent_graph_generator._raw_model.split("\n")),
        )

        # Change product agent profile, expect a product agent to depend on at
        # least two other agents.
        AgentGraphClingoContext.product_agent_profile["in_degree"][0] = 2

        exp = (
            "infra_agent(0) infra_agent(1) infra_agent(5) product_agent(2)"
            " product_agent(3) product_agent(4) product_agent(6) product_agent(7)"
            " product_agent(8) product_agent(9) depends_on(2,0) depends_on(2,1)"
            " depends_on(3,0) depends_on(3,1) depends_on(4,0) depends_on(4,1)"
            " depends_on(5,1) depends_on(6,0) depends_on(6,5) depends_on(7,0)"
            " depends_on(7,1) depends_on(8,0) depends_on(8,1) depends_on(9,0)"
            " depends_on(9,2) depends_on(9,4)"
        )
        agentGraphGenerator.generating_an_agent_graph(
            agent_distribution, agent_graph_generator
        )
        self.assertListEqual(
            sorted(exp.split("\n")),
            sorted(agent_graph_generator._raw_model.split("\n")),
        )

        # Change the number of infra agents to 3, and customize a indegree lower
        # bound function, so that infra_agent(N), N > 2 must depend on infra_agent(0)
        # and infra_agent(1).
        AgentGraphClingoContext.number_of_infra_agents = 3

        def customize_infra_agent_in_degree_low(self, agent: Number) -> Symbol:
            if agent.number < 2:
                return Number(0)
            else:
                return Number(2)

        AgentGraphClingoContext.infra_agent_in_degree_low = (
            customize_infra_agent_in_degree_low
        )

        exp = (
            "infra_agent(0) infra_agent(1) infra_agent(3) product_agent(2)"
            " product_agent(4) product_agent(5) product_agent(6) product_agent(7)"
            " product_agent(8) product_agent(9) depends_on(2,0) depends_on(2,1)"
            " depends_on(3,0) depends_on(3,1) depends_on(4,0) depends_on(4,1)"
            " depends_on(5,0) depends_on(5,1) depends_on(6,1) depends_on(6,3)"
            " depends_on(6,4) depends_on(6,5) depends_on(7,1) depends_on(7,3)"
            " depends_on(7,4) depends_on(7,5) depends_on(8,0) depends_on(8,3)"
            " depends_on(8,4) depends_on(8,5) depends_on(9,0) depends_on(9,3)"
            " depends_on(9,4) depends_on(9,5)"
        )

        agentGraphGenerator.generating_an_agent_graph(
            agent_distribution, agent_graph_generator
        )
        self.assertListEqual(
            sorted(exp.split("\n")),
            sorted(agent_graph_generator._raw_model.split("\n")),
        )

    def test_unsatisfiable_parameters(self) -> None:
        agent_graph_generator = AgentGraphGenerator()
        agent_graph_generator.on_model = agent_graph_generator.generate_raw
        agent_distribution = [2, 4, 4]

        # The total of infra agents + product agents are greater than sum(agent_distribution).
        AgentGraphClingoContext.number_of_infra_agents = 5
        AgentGraphClingoContext.number_of_product_agents = 6
        AgentGraphClingoContext.number_of_leaves = 5
        AgentGraphClingoContext.infra_agent_profile = {
            "in_degree": [0, 10],
            "out_degree": [1, 10],
        }
        AgentGraphClingoContext.product_agent_profile = {
            "in_degree": [1, 10],
            "out_degree": [0, 5],
        }

        with self.assertRaises(expected_exception=RuntimeError, msg="Unsatisfiable."):
            agentGraphGenerator.generating_an_agent_graph(
                agent_distribution, agent_graph_generator
            )
