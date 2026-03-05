# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
Dataclasses matching the HHVM printir JSON schema.

See hphp/doc/printir_json_schema.ts for the authoritative TypeScript schema.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional


@dataclass
class SSATmp:
    id: int
    type: str


@dataclass
class LabelInfo:
    id: int
    is_catch: bool
    hint: str  # "Unused", "Unlikely", "Neither", "Likely"


@dataclass
class TCRange:
    area: str  # "Main", "Cold", "Frozen"
    start: str  # hex address
    end: str  # hex address
    disasm: str


@dataclass
class PhiSrc:
    src: SSATmp
    label: LabelInfo


@dataclass
class PhiPseudoInstr:
    srcs: list[PhiSrc]
    dst: SSATmp


@dataclass
class ProfileData:
    offset: int
    name: str
    data: dict


@dataclass
class Instr:
    opcode_name: str
    offset: int
    id: Optional[int] = None
    type_param: Optional[str] = None
    guard: Optional[str] = None
    extra: Optional[str] = None
    srcs: list[SSATmp] = field(default_factory=list)
    counter_name: Optional[str] = None
    dsts: list[SSATmp] = field(default_factory=list)
    taken: Optional[LabelInfo] = None
    tc_ranges: Optional[list[TCRange]] = None
    phi_pseudo_instrs: list[PhiPseudoInstr] = field(default_factory=list)
    marker: Optional[str] = None
    profile_data: list[ProfileData] = field(default_factory=list)


@dataclass
class Block:
    label: LabelInfo
    area: str  # "Main", "Cold", "Frozen"
    prof_count: int
    preds: list[int] = field(default_factory=list)
    next: Optional[LabelInfo] = None
    instrs: list[Instr] = field(default_factory=list)


@dataclass
class SrcKey:
    func: str
    unit: str
    prologue: bool
    offset: int
    resume_mode: str
    has_this: bool


@dataclass
class TransContext:
    kind: str
    id: int
    opt_index: int
    src_key: SrcKey
    func_name: str
    source_file: str
    start_line: int
    end_line: int


@dataclass
class InliningDecision:
    was_inlined: bool
    offset: int
    caller: str
    callee: str
    reason: str


@dataclass
class Translation:
    context: TransContext
    blocks: list[Block]
    inlining_decisions: list[InliningDecision] = field(default_factory=list)
    opcode_stats: dict[str, int] = field(default_factory=dict)
