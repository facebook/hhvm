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
import sys
from typing import Any, Dict, List

from hphp.hack.src.hh_codesynthesis import agentGenerator
from hphp.hack.src.hh_codesynthesis.agentGraphGenerator import (
    AgentGraphGenerator,
    AgentGraphClingoContext,
    generating_an_agent_graph,
)
from hphp.hack.src.hh_codesynthesis.codeGenerator import CodeGenerator, ClingoContext
from hphp.hack.src.hh_codesynthesis.hackGenerator import HackCodeGenerator


class Agent(object):
    """To hold all information belongs to one agent."""

    def __init__(self, generator: CodeGenerator, solving_context: ClingoContext):
        super(Agent, self).__init__()
        self.generator = generator
        self.solving_context = solving_context


def create_agent(
    agents: List[Agent], agent_numbers: List[int], profiles: List[Dict[str, Any]]
):
    for index, agent_number in enumerate(agent_numbers):
        # The number of agents may greater than the number of profiles
        # So, we are round robin through each profile for the agent here.
        profile = profiles[index % len(profiles)]
        solving_context = ClingoContext(
            number_of_nodes=profile["number_of_nodes"],
            min_depth=profile["min_depth"],
            min_classes=profile["min_classes"],
            min_interfaces=profile["min_interfaces"],
            lower_bound=profile["lower_bound"],
            higher_bound=profile["higher_bound"],
            min_stub_classes=profile["min_stub_classes"],
            min_stub_interfaces=profile["min_stub_interfaces"],
            degree_distribution=profile["degree_distribution"],
        )
        generator = HackCodeGenerator(solving_context)
        # To avoid two agents using same profile to produce the same output,
        # we are using model_count to enumerate the next solution with this profile.
        generator.model_count = index // len(profiles)
        combined_rules = agentGenerator.generate_logic_rules(
            solving_context, f"A{agent_number}"
        )

        agents[agent_number] = Agent(generator, solving_context)
        agentGenerator.do_reasoning(combined_rules, generator)


def main() -> None:
    # [ToDo] Parse the JSON configuration file later. Mock an object for now.
    config = {
        "number_of_agents": 100,
        "number_of_infra_agents": 20,
        "number_of_product_agents": 40,
        "number_of_leaves": 30,
        "agent_distribution": [5, 10, 10, 10, 10, 10, 10, 15, 20],
        "infra_agent_indegrees": [0, 10],
        "infra_agent_outdegrees": [1, 10],
        "product_agent_indegrees": [1, 10],
        "product_agent_outdegrees": [0, 5],
        "infra_agent_profiles": [
            {
                "number_of_nodes": 12,
                "min_depth": 3,
                "min_classes": 3,
                "min_interfaces": 4,
                "lower_bound": 1,
                "higher_bound": 5,
                "min_stub_classes": 4,
                "min_stub_interfaces": 1,
                "degree_distribution": [1, 3, 5],
            },
            {
                "number_of_nodes": 12,
                "min_depth": 3,
                "min_classes": 3,
                "min_interfaces": 4,
                "lower_bound": 1,
                "higher_bound": 5,
                "min_stub_classes": 4,
                "min_stub_interfaces": 1,
                "degree_distribution": [1, 3, 5],
            },
        ],
        "product_agent_profiles": [
            {
                "number_of_nodes": 12,
                "min_depth": 3,
                "min_classes": 3,
                "min_interfaces": 4,
                "lower_bound": 1,
                "higher_bound": 5,
                "min_stub_classes": 4,
                "min_stub_interfaces": 1,
                "degree_distribution": [1, 3, 5],
            },
            {
                "number_of_nodes": 12,
                "min_depth": 3,
                "min_classes": 3,
                "min_interfaces": 4,
                "lower_bound": 1,
                "higher_bound": 5,
                "min_stub_classes": 4,
                "min_stub_interfaces": 1,
                "degree_distribution": [1, 3, 5],
            },
            {
                "number_of_nodes": 12,
                "min_depth": 3,
                "min_classes": 3,
                "min_interfaces": 4,
                "lower_bound": 1,
                "higher_bound": 5,
                "min_stub_classes": 4,
                "min_stub_interfaces": 1,
                "degree_distribution": [1, 3, 5],
            },
        ],
    }

    # Setup agent graph generator and context.
    agent_graph_generator = AgentGraphGenerator(
        agent_distribution=config["agent_distribution"],
        solving_context=AgentGraphClingoContext(
            number_of_infra_agents=config["number_of_infra_agents"],
            number_of_product_agents=config["number_of_product_agents"],
            number_of_leaves=config["number_of_leaves"],
            infra_agent_profile={
                "in_degree": config["infra_agent_indegrees"],
                "out_degree": config["infra_agent_outdegrees"],
            },
            product_agent_profile={
                "in_degree": config["product_agent_indegrees"],
                "out_degree": config["product_agent_outdegrees"],
            },
        ),
    )
    agent_graph_generator.on_model = agent_graph_generator.generate

    # Creating agent graph.
    generating_an_agent_graph(agent_graph_generator)

    # Using agent graph to create each agent.
    agents: List[Agent] = [None] * config["number_of_agents"]

    # [ToDo] Parallel run this section.
    create_agent(
        agents, agent_graph_generator.infra_agents, config["infra_agent_profiles"]
    )
    create_agent(
        agents, agent_graph_generator.product_agents, config["product_agent_profiles"]
    )

    # Connecting each agent.


if __name__ == "__main__":
    sys.exit(main())
