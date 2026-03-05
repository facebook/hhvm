# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
Format SSA IR translations for agent-readable output.

Two main modes:
  - summary: compact overview for scanning many translations
  - detail: full SSA IR with block structure, types, and annotations
Optional disasm flag includes machine code from tc_ranges.
"""

from __future__ import annotations

import io

from hphp.tools.ssa_inspector.ir_model import Instr, PhiPseudoInstr, SSATmp, Translation


def _fmt_tmp(tmp: SSATmp) -> str:
    return f"t{tmp.id}:{tmp.type}"


def _fmt_tmp_ref(tmp: SSATmp) -> str:
    return f"t{tmp.id}"


def _fmt_label_ref(label_id: int) -> str:
    return f"B{label_id}"


def _count_instrs(translation: Translation) -> int:
    return sum(len(b.instrs) for b in translation.blocks)


def _area_counts(translation: Translation) -> dict[str, int]:
    counts: dict[str, int] = {}
    for b in translation.blocks:
        counts[b.area] = counts.get(b.area, 0) + 1
    return counts


def _prof_count_range(translation: Translation) -> tuple[int, int]:
    if not translation.blocks:
        return (0, 0)
    counts = [b.prof_count for b in translation.blocks]
    return (min(counts), max(counts))


def format_summary(translation: Translation) -> str:
    """Compact overview of a translation."""
    out = io.StringIO()
    ctx = translation.context
    area_counts = _area_counts(translation)
    pc_min, pc_max = _prof_count_range(translation)

    out.write(f"=== Translation #{ctx.id} ===\n")
    out.write(f"Function: {ctx.func_name}\n")
    out.write(
        f"Kind: {ctx.kind} | Source: {ctx.source_file}:{ctx.start_line}"
        f" (offset {ctx.src_key.offset})\n"
    )

    area_str = ", ".join(f"{k}: {v}" for k, v in sorted(area_counts.items()))
    out.write(
        f"Blocks: {len(translation.blocks)} ({area_str})"
        f" | Instructions: {_count_instrs(translation)}\n"
    )
    out.write(f"ProfCount range: {pc_min:,} - {pc_max:,}\n")

    # Inlining summary
    if translation.inlining_decisions:
        inlined = [d for d in translation.inlining_decisions if d.was_inlined]
        not_inlined = [d for d in translation.inlining_decisions if not d.was_inlined]
        parts = []
        for d in inlined:
            parts.append(f"{d.callee} [yes]")
        for d in not_inlined:
            parts.append(f'{d.callee} [no: "{d.reason}"]')
        out.write(
            f"Inlined {len(inlined)}/{len(translation.inlining_decisions)}"
            f" callees: {', '.join(parts)}\n"
        )

    # Block stats
    out.write("\nBlock stats:\n")
    for b in translation.blocks:
        flags = []
        if b.area == "Cold" and b.prof_count > 0:
            # Highlight hot code in cold area
            flags.append("hot in cold area")
        if b.label.hint == "Unlikely" and b.prof_count > pc_max * 0.5:
            flags.append("hot but Unlikely")
        if b.prof_count == 0 and ctx.kind == "TransOptimize":
            flags.append("dead?")

        flag_str = f"  <- {', '.join(flags)}" if flags else ""
        out.write(
            f"  {_fmt_label_ref(b.label.id):4s}"
            f" ({b.area}, {b.label.hint})"
            f"   profCount={b.prof_count:<10,}"
            f"  {len(b.instrs)} instrs"
            f"{flag_str}\n"
        )

    return out.getvalue()


def _format_phi(phi: PhiPseudoInstr) -> str:
    srcs = ", ".join(
        f"{_fmt_tmp_ref(ps.src)}@{_fmt_label_ref(ps.label.id)}" for ps in phi.srcs
    )
    return f"  phi {_fmt_tmp(phi.dst)} = [{srcs}]"


def _format_instr(instr: Instr, show_disasm: bool = False) -> str:
    """Format a single instruction in HHIR-like text format."""
    out = io.StringIO()

    # Destination(s)
    dsts_str = ""
    if instr.dsts:
        dsts_str = ", ".join(_fmt_tmp(d) for d in instr.dsts) + " = "

    # Opcode with optional type param / guard / extra
    opcode = instr.opcode_name
    params = []
    if instr.type_param:
        params.append(instr.type_param)
    if instr.guard and instr.guard != "unused":
        params.append(instr.guard)
    if instr.extra:
        params.append(instr.extra)
    param_str = f"<{', '.join(params)}>" if params else ""

    # Sources
    if instr.counter_name is not None:
        srcs_str = f' "{instr.counter_name}"'
    elif instr.srcs:
        srcs_str = " " + ", ".join(_fmt_tmp_ref(s) for s in instr.srcs)
    else:
        srcs_str = ""

    # Taken branch
    taken_str = ""
    if instr.taken is not None:
        taken_str = f" -> {_fmt_label_ref(instr.taken.id)}"

    id_str = f"({instr.id:03d})" if instr.id is not None else "(---)"
    out.write(f"  {id_str} {dsts_str}{opcode}{param_str}{srcs_str}{taken_str}")

    # Disassembly
    if show_disasm and instr.tc_ranges:
        for r in instr.tc_ranges:
            out.write(f"\n       {r.area}: {r.start} - {r.end}")
            if r.disasm:
                for asm_line in r.disasm.strip().split("\n"):
                    out.write(f"\n         {asm_line}")

    return out.getvalue()


def format_detail(
    translation: Translation,
    show_disasm: bool = False,
) -> str:
    """Full SSA IR in readable text format with annotations."""
    out = io.StringIO()
    ctx = translation.context

    out.write(f"=== Translation #{ctx.id}: {ctx.func_name} ===\n")
    out.write(f"Kind: {ctx.kind} | {ctx.source_file}:{ctx.start_line}\n")
    out.write(
        f"OptIndex: {ctx.opt_index}"
        f" | SrcKey: offset={ctx.src_key.offset}"
        f" prologue={ctx.src_key.prologue}"
        f" resumeMode={ctx.src_key.resume_mode!r}"
        f" hasThis={ctx.src_key.has_this}\n"
    )
    out.write("\n")

    # Blocks
    for b in translation.blocks:
        preds_str = ", ".join(_fmt_label_ref(p) for p in b.preds) if b.preds else "[]"
        next_str = f", next->{_fmt_label_ref(b.next.id)}" if b.next else ""
        out.write(
            f"Block {_fmt_label_ref(b.label.id)}"
            f" [{b.area}, {b.label.hint}]"
            f" (profCount={b.prof_count:,}"
            f", preds=[{preds_str}]"
            f"{next_str}):\n"
        )

        for instr in b.instrs:
            # Phi pseudo-instructions (on DefLabel)
            for phi in instr.phi_pseudo_instrs:
                out.write(_format_phi(phi) + "\n")

            out.write(_format_instr(instr, show_disasm) + "\n")

        out.write("\n")

    # Inlining decisions
    if translation.inlining_decisions:
        out.write("Inlining Decisions:\n")
        for d in translation.inlining_decisions:
            marker = "\u2713" if d.was_inlined else "\u2717"
            status = "inlined" if d.was_inlined else f'not inlined: "{d.reason}"'
            out.write(f"  {marker} {d.callee} at offset {d.offset} ({status})\n")
        out.write("\n")

    # Opcode distribution
    if translation.opcode_stats:
        out.write("Opcode Distribution:\n")
        sorted_ops = sorted(translation.opcode_stats.items(), key=lambda x: -x[1])
        parts = [f"{op}: {count}" for op, count in sorted_ops]
        # Wrap at ~80 chars per line
        line = "  "
        for i, part in enumerate(parts):
            sep = " | " if i > 0 else ""
            if len(line) + len(sep) + len(part) > 80:
                out.write(line + "\n")
                line = "  " + part
            else:
                line += sep + part
        if line.strip():
            out.write(line + "\n")

    return out.getvalue()


def format_translation(
    translation: Translation,
    fmt: str = "detail",
    show_disasm: bool = False,
) -> str:
    """Format a translation in the specified mode."""
    if fmt == "summary":
        return format_summary(translation)
    else:
        return format_detail(translation, show_disasm=show_disasm)
