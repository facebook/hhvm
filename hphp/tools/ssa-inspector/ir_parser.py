# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
Parse JSON blobs from tcprint into ir_model dataclass instances.

Handles camelCase -> snake_case mapping and edge cases in the JSON schema
(e.g. IncStat's srcs being {counterName: ...} instead of an array).
"""

from __future__ import annotations

import json
from typing import Optional

from hphp.tools.ssa_inspector.ir_model import (
    Block,
    InliningDecision,
    Instr,
    LabelInfo,
    PhiPseudoInstr,
    PhiSrc,
    ProfileData,
    SrcKey,
    SSATmp,
    TCRange,
    TransContext,
    Translation,
)


def parse_ssa_tmp(data: dict) -> SSATmp:
    return SSATmp(id=data["id"], type=data["type"])


def parse_label_info(data: dict) -> LabelInfo:
    return LabelInfo(
        id=data["id"],
        is_catch=data.get("isCatch", False),
        hint=data.get("hint", "Neither"),
    )


def parse_tc_range(data: dict) -> TCRange:
    return TCRange(
        area=data["area"],
        start=data["start"],
        end=data["end"],
        disasm=data.get("disasm", ""),
    )


def parse_phi_src(data: dict) -> PhiSrc:
    return PhiSrc(
        src=parse_ssa_tmp(data["src"]),
        label=parse_label_info(data["label"]),
    )


def parse_phi_pseudo_instr(data: dict) -> PhiPseudoInstr:
    return PhiPseudoInstr(
        srcs=[parse_phi_src(s) for s in data.get("srcs", [])],
        dst=parse_ssa_tmp(data["dst"]),
    )


def parse_profile_data(data: dict) -> ProfileData:
    return ProfileData(
        offset=data.get("offset", 0),
        name=data.get("name", ""),
        data=data.get("data", {}),
    )


def parse_instr(data: dict) -> Instr:
    # Handle srcs: can be {counterName: ...} or [SSATmp, ...]
    srcs: list[SSATmp] = []
    counter_name: Optional[str] = None
    raw_srcs = data.get("srcs", [])
    if isinstance(raw_srcs, dict):
        counter_name = raw_srcs.get("counterName")
    elif isinstance(raw_srcs, list):
        srcs = [parse_ssa_tmp(s) for s in raw_srcs]

    taken = None
    if data.get("taken") is not None:
        taken = parse_label_info(data["taken"])

    tc_ranges = None
    if data.get("tc_ranges") is not None:
        tc_ranges = [parse_tc_range(r) for r in data["tc_ranges"]]

    marker = None
    if data.get("marker") is not None:
        marker = data["marker"].get("raw")

    return Instr(
        opcode_name=data["opcodeName"],
        offset=data.get("offset", 0),
        id=data.get("id"),
        type_param=data.get("typeParam"),
        guard=data.get("guard"),
        extra=data.get("extra"),
        srcs=srcs,
        counter_name=counter_name,
        dsts=[parse_ssa_tmp(d) for d in data.get("dsts", [])],
        taken=taken,
        tc_ranges=tc_ranges,
        phi_pseudo_instrs=[
            parse_phi_pseudo_instr(p) for p in data.get("phiPseudoInstrs", [])
        ],
        marker=marker,
        profile_data=[parse_profile_data(p) for p in data.get("profileData", [])],
    )


def parse_block(data: dict) -> Block:
    next_label = None
    if data.get("next") is not None:
        next_label = parse_label_info(data["next"])

    return Block(
        label=parse_label_info(data["label"]),
        area=data.get("area", "Main"),
        prof_count=data.get("profCount", 0),
        preds=data.get("preds", []),
        next=next_label,
        instrs=[parse_instr(i) for i in data.get("instrs", [])],
    )


def parse_src_key(data: dict) -> SrcKey:
    return SrcKey(
        func=data.get("func", ""),
        unit=data.get("unit", ""),
        prologue=data.get("prologue", False),
        offset=data.get("offset", 0),
        resume_mode=data.get("resumeMode", ""),
        has_this=data.get("hasThis", False),
    )


def parse_trans_context(data: dict) -> TransContext:
    return TransContext(
        kind=data.get("kind", ""),
        id=data.get("id", 0),
        opt_index=data.get("optIndex", 0),
        src_key=parse_src_key(data.get("srcKey", {})),
        func_name=data.get("funcName", ""),
        source_file=data.get("sourceFile", ""),
        start_line=data.get("startLine", 0),
        end_line=data.get("endLine", 0),
    )


def parse_inlining_decision(data: dict) -> InliningDecision:
    return InliningDecision(
        was_inlined=data.get("wasInlined", False),
        offset=data.get("offset", 0),
        caller=data.get("caller", ""),
        callee=data.get("callee", ""),
        reason=data.get("reason", ""),
    )


def parse_translation(data: dict) -> Translation:
    """Parse a full translation JSON blob (the Unit type) into a Translation."""
    return Translation(
        context=parse_trans_context(data.get("translation", {})),
        blocks=[parse_block(b) for b in data.get("blocks", [])],
        inlining_decisions=[
            parse_inlining_decision(d) for d in data.get("inliningDecisions", [])
        ],
        opcode_stats=data.get("opcodeStats", {}),
    )


def parse_json_blob(json_str: str) -> Translation:
    """Parse a JSON string into a Translation."""
    data = json.loads(json_str)
    return parse_translation(data)
