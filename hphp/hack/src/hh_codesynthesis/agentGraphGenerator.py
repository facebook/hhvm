#!/usr/bin/env python3
# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
from typing import List, Optional, Union

import clingo
from clingo.symbol import Number


class Context:
    def na(self):
        return Number(1000)

    def nr(self):
        return Number(5)

    def nl(self):
        return Number(30)

    def nia(self):
        return Number(200)

    def npa(self):
        return Number(600)

    def infra_agent_out_degree_low(self, agent):
        agent_number = agent.number
        if agent_number < 100:
            return Number(0)
        else:
            return Number(5)

    def infra_agent_out_degree_high(self, agent):
        return Number(100)

    def infra_agent_in_degree_low(self, agent):
        return Number(0)

    def infra_agent_in_degree_high(self, agent):
        return Number(10)

    def product_agent_out_degree_low(self, agent):
        return Number(0)

    def product_agent_out_degree_high(self, agent):
        return Number(5)

    def product_agent_in_degree_low(self, agent):
        return Number(5)

    def product_agent_in_degree_high(self, agent):
        return Number(20)

    def main(prg):
        prg.ground([("base", [])], context=Context())
        prg.solve()


def on_model(m: clingo.Model) -> None:
    print(m.__str__())


# Logic programs for code synthesis.
asp_files = "hphp/hack/src/hh_codesynthesis"

# Clingo interfaces.
ctl = clingo.Control()

# Load LP for agent graph generating.
ctl.load(asp_files + "/agent_graph_generator.lp")
ctl.ground([("base", [])], context=Context())

result: Union[clingo.solving.SolveHandle, clingo.solving.SolveResult] = ctl.solve(
    on_model=on_model
)
if isinstance(result, clingo.solving.SolveResult):
    if result.unsatisfiable:
        raise RuntimeError("Unsatisfiable.")
