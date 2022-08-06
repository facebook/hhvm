// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Print Functions
//!
//! These functions are used to print the IR in a pseudo-assembly format.
//!
//! The functions in this file (as opposed to formatters.rs) print full (or
//! more) lines of IR. As a result they mostly will be in the form
//! `print_thing(thing: &Thing)`.
//!

use std::fmt::Display;
use std::fmt::Error;
use std::fmt::Result;
use std::fmt::Write;

use ir_core::class::Property;
use ir_core::class::TraitReqKind;
use ir_core::instr::BaseOp;
use ir_core::instr::ContCheckOp;
use ir_core::instr::FCallArgsFlags;
use ir_core::instr::FinalOp;
use ir_core::instr::HasLoc;
use ir_core::instr::Hhbc;
use ir_core::instr::IncDecOp;
use ir_core::instr::IncludeKind;
use ir_core::instr::IrToBc;
use ir_core::instr::IsLogAsDynamicCallOp;
use ir_core::instr::MOpMode;
use ir_core::instr::MemberKey;
use ir_core::instr::OODeclExistsOp;
use ir_core::instr::QueryMOp;
use ir_core::instr::ReadonlyOp;
use ir_core::instr::SetRangeOp;
use ir_core::instr::Special;
use ir_core::instr::SwitchKind;
use ir_core::instr::Terminator;
use ir_core::instr::Tmp;
use ir_core::string_intern::StringInterner;
use ir_core::*;
use itertools::Itertools;

use crate::formatters::*;
use crate::util::FmtSep;

pub(crate) struct FuncContext<'a, 'b> {
    pub(crate) cur_loc_id: LocId,
    pub(crate) live_instrs: InstrIdSet,
    pub(crate) strings: &'b StringInterner<'a>,
    pub(crate) verbose: bool,
}

fn print_binary_op(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    func: &Func<'_>,
    op: &Hhbc,
) -> Result {
    let (prefix, infix, lhs, rhs) = match *op {
        Hhbc::Add([lhs, rhs], _) => ("add", " +", lhs, rhs),
        Hhbc::BitAnd([lhs, rhs], _) => ("bit_and", " &", lhs, rhs),
        Hhbc::BitOr([lhs, rhs], _) => ("bit_or", " |", lhs, rhs),
        Hhbc::BitXor([lhs, rhs], _) => ("bit_xor", " |", lhs, rhs),
        Hhbc::Cmp([lhs, rhs], _) => ("cmp", " <=>", lhs, rhs),
        Hhbc::CmpOp([lhs, rhs], op, _) => {
            use instr::CmpOp;
            let op = match op {
                CmpOp::Eq => " ==",
                CmpOp::Gt => " >",
                CmpOp::Gte => " >=",
                CmpOp::Lt => " <",
                CmpOp::Lte => " <=",
                CmpOp::NSame => " !==",
                CmpOp::Neq => " !=",
                CmpOp::Same => " ===",
            };
            ("cmp", op, lhs, rhs)
        }
        Hhbc::Concat([lhs, rhs], _) => ("concat", " +", lhs, rhs),
        Hhbc::Div([lhs, rhs], _) => ("div", " /", lhs, rhs),
        Hhbc::IsTypeStructC([lhs, rhs], op, _) => {
            let op = match op {
                instr::TypeStructResolveOp::Resolve => " resolve",
                instr::TypeStructResolveOp::DontResolve => " dont_resolve",
                _ => panic!("bad TypeStructResolveOp value"),
            };
            ("is_type_struct_c", op, lhs, rhs)
        }
        Hhbc::Modulo([lhs, rhs], _) => ("mod", ",", lhs, rhs),
        Hhbc::Mul([lhs, rhs], _) => ("mul", " *", lhs, rhs),
        Hhbc::Pow([lhs, rhs], _) => ("pow", " *", lhs, rhs),
        Hhbc::Shl([lhs, rhs], _) => ("shl", " <<", lhs, rhs),
        Hhbc::Shr([lhs, rhs], _) => ("shr", " >>", lhs, rhs),
        Hhbc::Sub([lhs, rhs], _) => ("sub", " -", lhs, rhs),
        _ => unreachable!(),
    };
    write!(
        w,
        "{} {}{} {}",
        prefix,
        FmtVid(func, lhs, ctx.verbose),
        infix,
        FmtVid(func, rhs, ctx.verbose)
    )
}

fn print_call(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    func: &Func<'_>,
    call: &Call,
) -> Result {
    let verbose = ctx.verbose;
    use instr::CallDetail;
    match &call.detail {
        CallDetail::FCallClsMethod { log } => {
            let dc = match *log {
                IsLogAsDynamicCallOp::LogAsDynamicCall => " log_as_dc",
                IsLogAsDynamicCallOp::DontLogAsDynamicCall => "",
                _ => panic!("bad IsLogAsDynamicCallOp value"),
            };
            write!(
                w,
                "call {}::{}{}",
                FmtVid(func, call.detail.class(&call.operands), verbose),
                FmtVid(func, call.detail.method(&call.operands), verbose),
                dc
            )?;
        }
        CallDetail::FCallClsMethodD { clsid, method } => {
            write!(
                w,
                "call direct {}::{}",
                FmtIdentifierId(clsid.id, ctx.strings),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        CallDetail::FCallClsMethodM { method, log } => {
            let dc = match *log {
                IsLogAsDynamicCallOp::LogAsDynamicCall => " log_as_dc",
                IsLogAsDynamicCallOp::DontLogAsDynamicCall => "",
                _ => panic!("bad IsLogAsDynamicCallOp value"),
            };
            write!(
                w,
                "call method {}::{}{}",
                FmtVid(func, call.detail.class(&call.operands), verbose),
                FmtIdentifierId(method.id, ctx.strings),
                dc
            )?;
        }
        CallDetail::FCallClsMethodS { clsref } => {
            write!(
                w,
                "call direct {}::{}",
                FmtSpecialClsRef(*clsref),
                FmtVid(func, call.detail.method(&call.operands), verbose),
            )?;
        }
        CallDetail::FCallClsMethodSD { clsref, method } => {
            write!(
                w,
                "call direct {}::{}",
                FmtSpecialClsRef(*clsref),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        CallDetail::FCallCtor => {
            write!(
                w,
                "call {}->ctor",
                FmtVid(func, call.detail.obj(&call.operands), verbose)
            )?;
        }
        CallDetail::FCallFunc => {
            write!(
                w,
                "call direct {}",
                FmtVid(func, call.detail.target(&call.operands), verbose)
            )?;
        }
        CallDetail::FCallFuncD { func } => {
            write!(w, "call direct {}", FmtIdentifierId(func.id, ctx.strings))?;
        }
        CallDetail::FCallObjMethod { .. } => {
            write!(
                w,
                "call method {}->{}",
                FmtVid(func, call.detail.obj(&call.operands), verbose),
                FmtVid(func, call.detail.method(&call.operands), verbose)
            )?;
        }
        CallDetail::FCallObjMethodD { method, .. } => {
            write!(
                w,
                "call method {}->{}",
                FmtVid(func, call.detail.obj(&call.operands), verbose),
                FmtIdentifierId(method.id, ctx.strings)
            )?;
        }
    }

    let mut inout_iter = call
        .inouts
        .as_ref()
        .map_or_else(|| [].iter(), |inouts| inouts.iter())
        .copied()
        .peekable();

    let mut readonly_iter = call
        .readonly
        .as_ref()
        .map_or_else(|| [].iter(), |readonly| readonly.iter())
        .copied()
        .peekable();

    let args = call.args().iter().enumerate().map(|(idx, arg)| {
        let idx = idx as u32;
        let inout = inout_iter.next_if_eq(&idx).is_some();
        let readonly = readonly_iter.next_if_eq(&idx).is_some();
        (arg, inout, readonly)
    });

    write!(
        w,
        "({})",
        FmtSep::comma(args, |w, (arg, inout, readonly)| {
            let inout = if inout { "inout " } else { "" };
            let readonly = if readonly { "readonly " } else { "" };
            write!(w, "{}{}{}", readonly, inout, FmtVid(func, *arg, verbose))
        })
    )?;

    let num_inouts: u32 = call.inouts.as_ref().map_or(0, |inouts| inouts.len()) as u32;
    if call.num_rets != 1 + num_inouts {
        write!(w, " num_rets({})", call.num_rets)?;
    }
    if call.flags.contains(FCallArgsFlags::HasUnpack) {
        write!(w, " has_unpack")?;
    }
    if call.flags.contains(FCallArgsFlags::HasGenerics) {
        write!(w, " has_generics")?;
    }
    if call.flags.contains(FCallArgsFlags::LockWhileUnwinding) {
        write!(w, " lock_while_unwinding")?;
    }
    if call.flags.contains(FCallArgsFlags::EnforceMutableReturn) {
        write!(w, " enforce_mutable_return")?;
    }
    if call.flags.contains(FCallArgsFlags::EnforceReadonlyThis) {
        write!(w, " enforce_readonly_this")?;
    }
    if call.flags.contains(FCallArgsFlags::SkipRepack) {
        write!(w, " skip_repack")?;
    }
    if call.flags.contains(FCallArgsFlags::SkipCoeffectsCheck) {
        write!(w, " skip_coeffects_check")?;
    }
    if call.flags.contains(FCallArgsFlags::ExplicitContext) {
        write!(w, " explicit_context")?;
    }
    if call.flags.contains(FCallArgsFlags::HasInOut) {
        write!(w, " has_in_out")?;
    }
    if call.flags.contains(FCallArgsFlags::EnforceInOut) {
        write!(w, " enforce_in_out")?;
    }
    if call.flags.contains(FCallArgsFlags::EnforceReadonly) {
        write!(w, " enforce_readonly")?;
    }
    if call.flags.contains(FCallArgsFlags::HasAsyncEagerOffset) {
        write!(w, " has_async_eager_offset")?;
    }
    if call.flags.contains(FCallArgsFlags::NumArgsStart) {
        write!(w, " num_args_start")?;
    }
    write!(w, " {}", FmtQuotedStringId(call.context, ctx.strings))?;
    Ok(())
}

fn print_call_async(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    func: &Func<'_>,
    call: &Call,
    targets: &[BlockId; 2],
) -> Result {
    write!(w, "async ")?;
    print_call(w, ctx, func, call)?;
    write!(
        w,
        " to {} eager {}",
        FmtBid(func, targets[0], ctx.verbose),
        FmtBid(func, targets[1], ctx.verbose),
    )
}

fn print_class(w: &mut dyn Write, class: &Class<'_>, strings: &StringInterner<'_>) -> Result {
    writeln!(
        w,
        "class {} {} {{",
        FmtIdentifierId(class.name.id, strings),
        FmtAttr(class.flags)
    )?;

    if let Some(doc_comment) = class.doc_comment.as_ref() {
        writeln!(w, "  doc_comment {}", FmtQuotedStr(doc_comment))?;
    }

    if let Some(base) = class.base {
        writeln!(w, "  extends {}", FmtIdentifierId(base.id, strings))?;
    }

    for implement in &class.implements {
        writeln!(w, "  implements {}", FmtIdentifierId(implement.id, strings))?;
    }

    if let Some(et) = class.enum_type.as_ref() {
        writeln!(w, "  enum_type {}", FmtType(et))?;
    }

    for (name, tys) in &class.upper_bounds {
        writeln!(
            w,
            "  upper_bound {}: [{}]",
            FmtIdentifier(name.as_ref()),
            FmtSep::comma(tys.iter(), |w, ty| FmtType(ty).fmt(w))
        )?;
    }

    for ctx in &class.ctx_constants {
        print_ctx_context(w, ctx)?;
    }

    for tc in &class.type_constants {
        print_type_constant(w, tc)?;
    }

    for use_ in &class.uses {
        writeln!(w, "  uses {}", FmtIdentifierId(use_.id, strings))?;
    }

    for (name, req_kind) in &class.requirements {
        let req = match req_kind {
            TraitReqKind::MustExtend => "extends",
            TraitReqKind::MustImplement => "implements",
            TraitReqKind::MustBeClass => "must_be_class",
        };
        writeln!(w, "  require {} {}", req, FmtIdentifierId(name.id, strings))?;
    }

    for prop in &class.properties {
        print_property(w, prop)?;
    }

    for attr in &class.attributes {
        writeln!(w, "  attribute {}", FmtAttribute(attr))?;
    }

    for c in &class.constants {
        write!(w, "  ")?;
        print_hack_constant(w, c)?;
    }

    for method in &class.methods {
        writeln!(w, "  method {}", FmtIdentifier(method.name.as_bytes()))?;
    }

    writeln!(w, "}}")?;
    writeln!(w)
}

pub(crate) fn print_coeffects(w: &mut dyn Write, coeffects: &Coeffects<'_>) -> Result {
    if !coeffects.static_coeffects.is_empty() || coeffects.unenforced_static_coeffects.is_empty() {
        write!(w, "  .coeffects_static")?;
        for co in &coeffects.static_coeffects {
            write!(w, " {}", co)?;
        }
        for co in &coeffects.unenforced_static_coeffects {
            write!(w, " {}", FmtIdentifier(co))?;
        }
        writeln!(w, ";")?;
    }

    if !coeffects.fun_param.is_empty() {
        write!(w, ".coeffects_fun_param")?;
        for fp in &coeffects.fun_param {
            write!(w, " {}", *fp)?;
        }
        writeln!(w, ";")?;
    }

    for (idx, name) in &coeffects.cc_param {
        writeln!(w, ".coeffects_cc_param {idx} {}", FmtQuotedStr(name))?;
    }

    for cc_this in &coeffects.cc_this {
        writeln!(
            w,
            ".coeffects_cc_this {}",
            FmtSep::comma(cc_this.iter(), |f, v| { FmtQuotedStr(v).fmt(f) })
        )?;
    }

    for (is_class, idx, qn) in &coeffects.cc_reified {
        writeln!(
            w,
            "  .coeffects_cc_reified {}{} {}",
            if *is_class { "isClass" } else { "" },
            idx,
            FmtSep::new("", "::", "", qn.iter(), |w, qn| FmtIdentifier(qn).fmt(w))
        )?;
    }

    if coeffects.closure_parent_scope {
        writeln!(w, "  .coeffects_closure_parent_scope;")?;
    }

    if coeffects.generator_this {
        writeln!(w, "  .coeffects_generator_this;")?;
    }

    if coeffects.caller {
        writeln!(w, "  .coeffects_caller;")?;
    }

    Ok(())
}

fn print_ctx_context(w: &mut dyn Write, ctx: &CtxConstant<'_>) -> Result {
    write!(
        w,
        "  ctx_context {} [{}] [{}]{}",
        FmtIdentifier(&ctx.name),
        FmtSep::comma(ctx.recognized.iter(), |w, i| FmtIdentifier(i).fmt(w)),
        FmtSep::comma(ctx.unrecognized.iter(), |w, i| FmtIdentifier(i).fmt(w)),
        if ctx.is_abstract { " abstract" } else { "" }
    )
}

pub(crate) fn print_fatal(w: &mut dyn Write, fatal: &FatalOp<'_>) -> Result {
    let (what, loc, msg) = match fatal {
        FatalOp::None => return Ok(()),
        FatalOp::Parse(loc, msg) => ("parse", loc, msg),
        FatalOp::Runtime(loc, msg) => ("runtime", loc, msg),
        FatalOp::RuntimeOmitFrame(loc, msg) => ("runtime_omit_frame", loc, msg),
    };

    writeln!(w, ".fatal {} {} {}\n", FmtLoc(loc), what, FmtQuotedStr(msg))
}

pub(crate) fn print_func_body(
    w: &mut dyn Write,
    func: &Func<'_>,
    verbose: bool,
    strings: &StringInterner<'_>,
) -> Result {
    if let Some(doc_comment) = func.doc_comment.as_ref() {
        writeln!(w, "  .doc {}", FmtQuotedStr(doc_comment))?;
    }
    for lid in func.literals.keys() {
        writeln!(
            w,
            "  .const {} = {}",
            FmtRawVid(ValueId::from_literal(lid)),
            FmtLiteralId(func, lid),
        )?;
    }

    for (id, func::ExFrame { parent, catch_bid }) in &func.ex_frames {
        write!(
            w,
            "  .ex-frame {}: catch={}",
            id.as_usize(),
            FmtBid(func, *catch_bid, verbose)
        )?;
        match parent {
            TryCatchId::None => {}
            TryCatchId::Try(id) => write!(w, ", parent=try({}), ", id.as_usize())?,
            TryCatchId::Catch(id) => write!(w, ", parent=catch({}), ", id.as_usize())?,
        }
        writeln!(w)?;
    }

    let live_instrs = crate::util::compute_live_instrs(func, verbose);

    let mut ctx = FuncContext {
        cur_loc_id: LocId::NONE,
        live_instrs,
        strings,
        verbose,
    };
    for bid in func.blocks.keys() {
        let block = &func.blocks[bid];
        write!(w, "{}", FmtBid(func, bid, false))?;
        if !block.params.is_empty() {
            write!(
                w,
                "({})",
                FmtSep::comma(&block.params, |w, iid| FmtVid(
                    func,
                    ValueId::from_instr(*iid),
                    verbose
                )
                .fmt(w))
            )?;
        }

        writeln!(w, ":")?;

        match block.tcid {
            TryCatchId::None => {}
            TryCatchId::Try(id) => writeln!(w, "  .try-id: {}", id)?,
            TryCatchId::Catch(id) => writeln!(w, "  .catch-id: {}", id)?,
        }

        for iid in block.iids() {
            let instr = func.instr(iid);
            if crate::print::print_instr(w, &mut ctx, func, iid, instr)? {
                writeln!(w)?;
            }
        }
    }

    Ok(())
}

fn print_function(
    w: &mut dyn Write,
    f: &Function<'_>,
    verbose: bool,
    strings: &StringInterner<'_>,
) -> Result {
    writeln!(
        w,
        "function {name}{tparams}{params}{shadowed_tparams} {{",
        name = FmtIdentifier(f.name.as_bytes()),
        tparams = FmtTParams(&f.func.tparams, strings),
        shadowed_tparams = FmtShadowedTParams(&f.func.shadowed_tparams, strings),
        params = FmtFuncParams(&f.func, strings)
    )?;
    print_coeffects(w, &f.coeffects)?;
    print_func_body(w, &f.func, verbose, strings)?;
    writeln!(w, "}}")?;
    writeln!(w)
}

fn print_hhbc(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    func: &Func<'_>,
    hhbc: &Hhbc,
) -> Result {
    let verbose = ctx.verbose;
    match *hhbc {
        Hhbc::Add(..)
        | Hhbc::BitAnd(..)
        | Hhbc::BitOr(..)
        | Hhbc::BitXor(..)
        | Hhbc::Cmp(..)
        | Hhbc::CmpOp(..)
        | Hhbc::Concat(..)
        | Hhbc::Div(..)
        | Hhbc::IsTypeStructC(..)
        | Hhbc::Modulo(..)
        | Hhbc::Mul(..)
        | Hhbc::Pow(..)
        | Hhbc::Shl(..)
        | Hhbc::Shr(..)
        | Hhbc::Sub(..) => print_binary_op(w, ctx, func, hhbc)?,

        Hhbc::AKExists(ops, _) => {
            write!(
                w,
                "ak_exists {}, {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose)
            )?;
        }
        Hhbc::AddElemC(ops, _) => {
            write!(
                w,
                "add_elem_c {}[{}] = {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose),
                FmtVid(func, ops[2], verbose),
            )?;
        }
        Hhbc::AddNewElemC(ops, _) => {
            write!(
                w,
                "add_new_elem_c {}[] = {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose),
            )?;
        }
        Hhbc::ArrayIdx(vids, _) => {
            write!(
                w,
                "array_idx {}[{}] or {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose),
                FmtVid(func, vids[2], verbose)
            )?;
        }
        Hhbc::ArrayMarkLegacy(vids, _) => {
            write!(
                w,
                "array_mark_legacy {}, {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose)
            )?;
        }
        Hhbc::ArrayUnmarkLegacy(vids, _) => {
            write!(
                w,
                "array_unmark_legacy {}, {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose)
            )?;
        }
        Hhbc::Await(vid, _) => {
            write!(w, "await {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::AwaitAll(ref range, _) => {
            write!(w, "await_all {}", FmtLids(range, ctx.strings),)?;
        }
        Hhbc::BareThis(op, _) => {
            write!(w, "bare_this {}", FmtBareThisOp(op))?;
        }
        Hhbc::BitNot(vid, _) => write!(w, "bit_not {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CGetG(vid, _) => write!(w, "get_global {}", FmtVid(func, vid, verbose))?,
        Hhbc::CGetL(lid, _) => write!(w, "get_local {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CGetQuietL(lid, _) => write!(w, "get_local quiet {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CGetS(vids, readonly, _) => write!(
            w,
            "get_static {}->{} {}",
            FmtVid(func, vids[1], verbose),
            FmtVid(func, vids[0], verbose),
            FmtReadonly(readonly)
        )?,
        Hhbc::CUGetL(lid, _) => write!(w, "get_local_or_uninit {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CastBool(vid, _) => write!(w, "cast_bool {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastDict(vid, _) => write!(w, "cast_dict {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastDouble(vid, _) => write!(w, "cast_double {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastInt(vid, _) => write!(w, "cast_int {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastKeyset(vid, _) => write!(w, "cast_keyset {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastString(vid, _) => write!(w, "cast_string {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::CastVec(vid, _) => write!(w, "cast_vec {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::ChainFaults(ops, _) => {
            write!(
                w,
                "chain_faults {}, {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose)
            )?;
        }
        Hhbc::CheckClsReifiedGenericMismatch(vid, _) => {
            write!(
                w,
                "check_cls_reified_generic_mismatch {}",
                FmtVid(func, vid, verbose)
            )?;
        }
        Hhbc::CheckClsRGSoft(vid, _) => {
            write!(w, "check_cls_rg_soft {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::CheckProp(prop, _) => {
            write!(w, "check_prop {}", FmtIdentifierId(prop.id, ctx.strings))?
        }
        Hhbc::CheckThis(_) => {
            write!(w, "check_this")?;
        }
        Hhbc::ClassGetC(vid, _) => write!(w, "class_get_c {}", FmtVid(func, vid, verbose))?,
        Hhbc::ClassGetTS(vid, _) => write!(w, "class_get_ts {}", FmtVid(func, vid, verbose))?,
        Hhbc::ClassHasReifiedGenerics(vid, _) => write!(
            w,
            "class_has_reified_generics {}",
            FmtVid(func, vid, verbose)
        )?,
        Hhbc::ClassName(vid, _) => {
            write!(w, "class_name {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::Clone(vid, _) => write!(w, "clone {}", FmtVid(func, vid, ctx.verbose),)?,
        Hhbc::ClsCns(clsid, id, _) => {
            write!(
                w,
                "cls_cns {}::{}",
                FmtVid(func, clsid, verbose),
                FmtIdentifierId(id.id, ctx.strings)
            )?;
        }
        Hhbc::ClsCnsD(id, clsid, _) => {
            write!(
                w,
                "cls_cns_d {}::{}",
                FmtIdentifierId(clsid.id, ctx.strings),
                FmtIdentifierId(id.id, ctx.strings)
            )?;
        }
        Hhbc::ClsCnsL(vid, lid, _) => {
            write!(
                w,
                "cls_cns_l {}::{}",
                FmtVid(func, vid, verbose),
                FmtLid(lid, ctx.strings)
            )?;
        }
        Hhbc::ColFromArray(vid, kind, _) => {
            write!(
                w,
                "col_from_array {} {}",
                FmtCollectionType(kind),
                FmtVid(func, vid, verbose)
            )?;
        }
        Hhbc::CombineAndResolveTypeStruct(ref vids, _) => {
            write!(
                w,
                "combine_and_resolve_type_struct {}",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose).fmt(w))
            )?;
        }
        Hhbc::ConcatN(ref vids, _) => {
            write!(
                w,
                "concatn {}",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose).fmt(w))
            )?;
        }
        Hhbc::ConsumeL(lid, _) => {
            write!(w, "consume_local {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::ContCheck(kind, _) => {
            let kind = match kind {
                ContCheckOp::IgnoreStarted => "ignore",
                ContCheckOp::CheckStarted => "check",
                _ => panic!("bad ContCheckOp value"),
            };
            write!(w, "cont_check {}", kind)?;
        }
        Hhbc::ContCurrent(_) => {
            write!(w, "cont_current")?;
        }
        Hhbc::ContEnter(vid, _) => {
            write!(w, "cont_enter {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::ContGetReturn(_) => {
            write!(w, "cont_get_return")?;
        }
        Hhbc::ContKey(_) => {
            write!(w, "cont_key")?;
        }
        Hhbc::ContRaise(vid, _) => {
            write!(w, "cont_raise {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::ContValid(_) => {
            write!(w, "cont_valid")?;
        }
        Hhbc::CreateCl {
            ref operands,
            clsid,
            loc: _,
        } => {
            write!(
                w,
                "create_class {}({})",
                FmtIdentifierId(clsid.id, ctx.strings),
                FmtSep::comma(operands.iter(), |w, arg| write!(
                    w,
                    "{}",
                    FmtVid(func, *arg, verbose)
                ))
            )?;
        }
        Hhbc::CreateCont(_) => write!(w, "create_cont")?,
        Hhbc::GetClsRGProp(vid, _) => {
            write!(w, "get_class_rg_prop {}", FmtVid(func, vid, verbose))?
        }
        Hhbc::GetMemoKeyL(lid, _) => {
            write!(w, "get_memo_key {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::HasReifiedParent(vid, _) => {
            write!(w, "has_reified_parent {}", FmtVid(func, vid, verbose))?
        }
        Hhbc::Idx(vids, _) => {
            write!(
                w,
                "idx {}[{}] or {}",
                FmtVid(func, vids[2], verbose),
                FmtVid(func, vids[1], verbose),
                FmtVid(func, vids[0], verbose),
            )?;
        }
        Hhbc::IncDecL(lid, op, _) => {
            let flag = incdec_flag(op);
            let (pre, post) = incdec_what(op);
            let lid = FmtLid(lid, ctx.strings);
            write!(w, "incdec_local {} {}{}{}", flag, pre, lid, post)?;
        }
        Hhbc::IncDecS([cls, prop], op, _) => {
            let flag = incdec_flag(op);
            let (pre, post) = incdec_what(op);
            write!(
                w,
                "incdec_static_prop {} {}{}::{}{}",
                flag,
                pre,
                FmtVid(func, cls, verbose),
                FmtVid(func, prop, verbose),
                post
            )?;
        }
        Hhbc::IncludeEval(ref ie) => print_include_eval(w, ctx, func, ie)?,
        Hhbc::InitProp(vid, prop, op, _) => {
            write!(
                w,
                "init_prop {}, {}, {}",
                FmtQuotedStringId(prop.id, ctx.strings),
                FmtVid(func, vid, verbose),
                FmtInitPropOp(op)
            )?;
        }
        Hhbc::InstanceOfD(vid, clsid, _) => write!(
            w,
            "instance_of_d {}, {}",
            FmtVid(func, vid, ctx.verbose),
            FmtIdentifierId(clsid.id, ctx.strings)
        )?,
        Hhbc::IsLateBoundCls(vid, _) => {
            write!(w, "is_late_bound_cls {}", FmtVid(func, vid, ctx.verbose))?;
        }
        Hhbc::IsTypeC(vid, op, _) => {
            write!(
                w,
                "is_type_c {}, {}",
                FmtVid(func, vid, ctx.verbose),
                FmtIsTypeOp(op)
            )?;
        }
        Hhbc::IsTypeL(lid, op, _) => {
            write!(
                w,
                "is_type_l {}, {}",
                FmtLid(lid, ctx.strings),
                FmtIsTypeOp(op)
            )?;
        }
        Hhbc::IssetG(vid, _) => {
            write!(w, "isset_g {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::IssetL(lid, _) => {
            write!(w, "isset_l {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::IssetS([cls, prop], _) => {
            write!(
                w,
                "isset_s {}::{}",
                FmtVid(func, cls, verbose),
                FmtVid(func, prop, verbose)
            )?;
        }
        Hhbc::IterFree(iter_id, _loc) => {
            write!(w, "iterator ^{} free", iter_id.idx)?;
        }
        Hhbc::LateBoundCls(_) => {
            write!(w, "late_bound_cls")?;
        }
        Hhbc::LockObj(vid, _) => {
            write!(w, "lock_obj {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::MemoSet(vid, ref locals, _) => {
            write!(w, "memo_set ")?;
            if !locals.is_empty() {
                write!(w, "{} ", FmtLids(locals, ctx.strings))?;
            }
            write!(w, "{}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::MemoSetEager(vid, ref locals, _) => {
            write!(w, "memo_set_eager ")?;
            if !locals.is_empty() {
                write!(w, "{} ", FmtLids(locals, ctx.strings))?;
            }
            write!(w, "{}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::NewDictArray(hint, _) => {
            write!(w, "new_dict_array {}", hint)?;
        }
        Hhbc::NewKeysetArray(ref operands, _) => {
            write!(
                w,
                "new_keyset_array [{}]",
                FmtSep::comma(operands.iter(), |w, arg| write!(
                    w,
                    "{}",
                    FmtVid(func, *arg, verbose)
                ))
            )?;
        }
        Hhbc::NewObj(vid, _) => {
            write!(w, "new_obj {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::NewObjD(clsid, _) => {
            write!(
                w,
                "new_obj direct {}",
                FmtIdentifierId(clsid.id, ctx.strings)
            )?;
        }
        Hhbc::NewObjR([cls, rp], _) => {
            write!(
                w,
                "new_obj_r {}, {}",
                FmtVid(func, cls, verbose),
                FmtVid(func, rp, verbose)
            )?;
        }
        Hhbc::NewObjRD(_, clsid, _) => {
            write!(
                w,
                "new_obj reified direct {}",
                FmtIdentifierId(clsid.id, ctx.strings)
            )?;
        }
        Hhbc::NewObjS(clsref, _) => {
            write!(w, "new_obj static {}", FmtSpecialClsRef(clsref))?;
        }
        Hhbc::NewPair(vids, _) => {
            write!(
                w,
                "new_pair {}, {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose)
            )?;
        }
        Hhbc::NewStructDict(ref keys, ref values, _) => {
            write!(
                w,
                "new_struct_dict [{}]",
                FmtSep::comma(keys.iter().zip(values.iter()), |w, (k, v)| {
                    write!(
                        w,
                        "{} => {}",
                        FmtQuotedStringId(*k, ctx.strings),
                        FmtVid(func, *v, verbose)
                    )
                })
            )?;
        }
        Hhbc::NewVec(ref vids, _) => {
            write!(
                w,
                "new_vec [{}]",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose).fmt(w))
            )?;
        }
        Hhbc::Not(vid, _) => write!(w, "not {}", FmtVid(func, vid, ctx.verbose))?,
        Hhbc::OODeclExists(vids, op, _) => {
            let kind = match op {
                OODeclExistsOp::Class => "class",
                OODeclExistsOp::Interface => "interface",
                OODeclExistsOp::Trait => "trait",
                _ => unreachable!(),
            };
            write!(
                w,
                "oo_decl_exists {}, {} {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose),
                kind
            )?;
        }
        Hhbc::ParentCls(_) => {
            write!(w, "parent")?;
        }
        Hhbc::Print(vid, _) => write!(w, "print {}", FmtVid(func, vid, ctx.verbose))?,
        Hhbc::RecordReifiedGeneric(vid, _) => write!(
            w,
            "record_reified_generic {}",
            FmtVid(func, vid, ctx.verbose)
        )?,
        Hhbc::ResolveClass(clsid, _) => write!(
            w,
            "resolve_class {}",
            FmtIdentifierId(clsid.id, ctx.strings)
        )?,
        Hhbc::ResolveClsMethod(vid, method, _) => {
            write!(
                w,
                "resolve_cls_method {}::{}",
                FmtVid(func, vid, ctx.verbose),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        Hhbc::ResolveClsMethodD(clsid, method, _) => {
            write!(
                w,
                "resolve_cls_method_d {}::{}",
                FmtIdentifierId(clsid.id, ctx.strings),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        Hhbc::ResolveClsMethodS(clsref, method, _) => {
            write!(
                w,
                "resolve_cls_method_s {}::{}",
                FmtSpecialClsRef(clsref),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        Hhbc::ResolveRClsMethod([clsid, vid], method, _) => {
            write!(
                w,
                "resolve_cls_method {}::{}, {}",
                FmtVid(func, clsid, verbose),
                FmtIdentifierId(method.id, ctx.strings),
                FmtVid(func, vid, verbose),
            )?;
        }
        Hhbc::ResolveRClsMethodS(vid, clsref, method, _) => {
            write!(
                w,
                "resolve_cls_method_s {}::{}, {}",
                FmtSpecialClsRef(clsref),
                FmtIdentifierId(method.id, ctx.strings),
                FmtVid(func, vid, verbose),
            )?;
        }
        Hhbc::ResolveFunc(func, _) => {
            write!(w, "resolve_func {}", FmtIdentifierId(func.id, ctx.strings))?;
        }
        Hhbc::ResolveRClsMethodD(vid, clsid, method, _) => {
            write!(
                w,
                "resolve_r_cls_method_d {}::{}, {}",
                FmtIdentifierId(clsid.id, ctx.strings),
                FmtIdentifierId(method.id, ctx.strings),
                FmtVid(func, vid, verbose),
            )?;
        }
        Hhbc::ResolveRFunc(rid, fid, _) => {
            write!(
                w,
                "resolve_r_func {}, {}",
                FmtIdentifierId(fid.id, ctx.strings),
                FmtVid(func, rid, verbose)
            )?;
        }
        Hhbc::ResolveMethCaller(func, _) => {
            write!(
                w,
                "resolve_meth_caller {}",
                FmtIdentifierId(func.id, ctx.strings)
            )?;
        }
        Hhbc::SelfCls(_) => {
            write!(w, "self")?;
        }
        Hhbc::SetG([target, value], _) => {
            write!(
                w,
                "set_global {}, {}",
                FmtVid(func, target, verbose),
                FmtVid(func, value, verbose),
            )?;
        }
        Hhbc::SetImplicitContextByValue(vid, _) => {
            write!(
                w,
                "set_implicit_context_by_value {}",
                FmtVid(func, vid, verbose)
            )?;
        }
        Hhbc::SetL(vid, lid, _) => {
            write!(
                w,
                "set_local {}, {}",
                FmtLid(lid, ctx.strings),
                FmtVid(func, vid, verbose),
            )?;
        }
        Hhbc::SetOpL(vid, lid, op, _) => {
            write!(
                w,
                "set_op_local {}, {}, {}",
                FmtLid(lid, ctx.strings),
                FmtSetOpOp(op),
                FmtVid(func, vid, verbose)
            )?;
        }
        Hhbc::SetOpG([x, y], op, _) => {
            write!(
                w,
                "set_op_global {}, {}, {}",
                FmtVid(func, x, verbose),
                FmtSetOpOp(op),
                FmtVid(func, y, verbose)
            )?;
        }
        Hhbc::SetOpS(vids, op, _) => {
            write!(
                w,
                "set_op_static_property {}, {}, {}, {}",
                FmtVid(func, vids[0], verbose),
                FmtVid(func, vids[1], verbose),
                FmtVid(func, vids[2], verbose),
                FmtSetOpOp(op),
            )?;
        }
        Hhbc::SetS(vids, readonly, _) => {
            write!(
                w,
                "set_s {}->{} {} = {}",
                FmtVid(func, vids[1], verbose),
                FmtVid(func, vids[0], verbose),
                FmtReadonly(readonly),
                FmtVid(func, vids[2], verbose)
            )?;
        }
        Hhbc::Silence(lid, op, _) => {
            let lid = FmtLid(lid, ctx.strings);
            write!(w, "silence {}, {:?}", lid, op)?;
        }
        Hhbc::This(_) => {
            write!(w, "this")?;
        }
        Hhbc::ThrowNonExhaustiveSwitch(_) => {
            write!(w, "throw_nonexhaustive_switch")?;
        }
        Hhbc::UnsetG(vid, _) => {
            write!(w, "unset {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::UnsetL(lid, _) => {
            write!(w, "unset {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::VerifyImplicitContextState(_) => {
            write!(w, "verify_implicit_context_state")?;
        }
        Hhbc::VerifyOutType(vid, lid, _) => {
            write!(
                w,
                "verify_out_type {}, {}",
                FmtVid(func, vid, verbose),
                FmtLid(lid, ctx.strings),
            )?;
        }
        Hhbc::VerifyParamType(lid, _) => {
            write!(w, "verify_param_type {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::VerifyParamTypeTS(vid, lid, _) => {
            write!(
                w,
                "verify_param_type_ts {}, {}",
                FmtVid(func, vid, verbose),
                FmtLid(lid, ctx.strings),
            )?;
        }
        Hhbc::VerifyRetTypeC(op, _) => {
            write!(w, "verify_ret_type_c {}", FmtVid(func, op, verbose))?;
        }
        Hhbc::VerifyRetTypeTS(ops, _) => {
            write!(
                w,
                "verify_ret_type_ts {}, {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose)
            )?;
        }
        Hhbc::WHResult(vid, _) => {
            write!(w, "wh_result {}", FmtVid(func, vid, verbose))?;
        }
        Hhbc::Yield(vid, _) => write!(w, "yield {}", FmtVid(func, vid, verbose))?,
        Hhbc::YieldK(ops, _) => write!(
            w,
            "yield {} => {}",
            FmtVid(func, ops[0], verbose),
            FmtVid(func, ops[1], verbose)
        )?,
    }
    Ok(())
}

fn print_hack_constant(w: &mut dyn Write, c: &HackConstant<'_>) -> Result {
    write!(w, "constant ")?;

    if c.is_abstract {
        write!(w, "abstract ")?;
    }

    write!(w, "{}", FmtIdentifier(c.name.as_bytes()))?;

    if let Some(value) = &c.value {
        write!(w, " = {}", FmtTypedValue(value))?;
    }

    writeln!(w)?;

    Ok(())
}

fn print_include_eval(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    func: &Func<'_>,
    ie: &instr::IncludeEval,
) -> Result {
    let vid = FmtVid(func, ie.vid, ctx.verbose);
    let kind = match ie.kind {
        IncludeKind::Eval => "eval",
        IncludeKind::Include => "include",
        IncludeKind::IncludeOnce => "include_once",
        IncludeKind::Require => "require",
        IncludeKind::RequireOnce => "require_once",
        IncludeKind::RequireOnceDoc => "require_once_doc",
    };
    write!(w, "{} {}", kind, vid)
}

pub(crate) fn print_instr(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    iid: InstrId,
    instr: &Instr,
) -> std::result::Result<bool, Error> {
    // Special cases
    if !ctx.verbose && matches!(instr, Instr::Special(Special::Param)) {
        return Ok(false);
    }

    print_loc(w, ctx, func, instr.loc_id())?;

    write!(w, "  ")?;

    if ctx.live_instrs.contains(&iid) {
        write!(w, "{} = ", FmtRawVid(ValueId::from_instr(iid)))?;
    }

    match instr {
        Instr::Call(call) => print_call(w, ctx, func, call)?,
        Instr::Hhbc(hhbc) => print_hhbc(w, ctx, func, hhbc)?,
        Instr::MemberOp(op) => print_member_op(w, ctx, func, op)?,
        Instr::Special(Special::Copy(vid)) => {
            write!(w, "copy {}", FmtVid(func, *vid, ctx.verbose))?;
        }
        Instr::Special(Special::IrToBc(ir_to_bc)) => {
            print_ir_to_bc(w, ctx, func, ir_to_bc)?;
        }
        Instr::Special(Special::Tmp(Tmp::GetVar(var))) => {
            write!(w, "get_var &{}", var.as_usize())?;
        }
        Instr::Special(Special::Tmp(Tmp::SetVar(var, value))) => {
            write!(
                w,
                "set_var &{}, {}",
                var.as_usize(),
                FmtVid(func, *value, ctx.verbose)
            )?;
        }
        Instr::Special(Special::Param) => write!(w, "param")?,
        Instr::Special(Special::Select(vid, index)) => write!(
            w,
            "select {} from {}",
            index,
            FmtVid(func, *vid, ctx.verbose)
        )?,
        Instr::Special(Special::Tombstone) => write!(w, "tombstone")?,
        Instr::Terminator(t) => print_terminator(w, ctx, func, iid, t)?,
    }
    Ok(true)
}

pub(crate) fn print_ir_to_bc(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    ir_to_bc: &IrToBc,
) -> std::result::Result<(), Error> {
    match ir_to_bc {
        IrToBc::PopC => write!(w, "popc")?,
        IrToBc::PopL(lid) => write!(w, "pop_local {}", FmtLid(*lid, ctx.strings),)?,
        IrToBc::PushL(lid) => {
            write!(w, "push {}", FmtLid(*lid, ctx.strings))?;
        }
        IrToBc::PushLiteral(vid) => write!(w, "push {}", FmtVid(func, *vid, ctx.verbose))?,
        IrToBc::PushUninit => write!(w, "push_uninit")?,
        IrToBc::UnsetL(lid) => {
            write!(w, "unset_local {}", FmtLid(*lid, ctx.strings))?;
        }
    }
    Ok(())
}

fn print_inner_loc(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    loc_id: LocId,
) -> Result {
    if ctx.cur_loc_id != loc_id {
        ctx.cur_loc_id = loc_id;
        if let Some(loc) = func.get_loc(loc_id) {
            write!(w, "<srcloc {}> ", FmtLoc(loc))?;
        }
    }
    Ok(())
}

fn print_loc(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    loc_id: LocId,
) -> Result {
    if ctx.cur_loc_id != loc_id {
        ctx.cur_loc_id = loc_id;
        if let Some(loc) = func.get_loc(loc_id) {
            writeln!(w, "  .srcloc {}", FmtLoc(loc))?;
        }
    }
    Ok(())
}

fn print_member_op(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    op: &instr::MemberOp,
) -> Result {
    let final_op_str = match op.final_op {
        FinalOp::IncDecM { .. } => "incdecm",
        FinalOp::QueryM { .. } => "querym",
        FinalOp::SetM { .. } => "setm",
        FinalOp::SetOpM { .. } => "setopm",
        FinalOp::SetRangeM { .. } => "setrangem",
        FinalOp::UnsetM { .. } => "unsetm",
    };
    write!(w, "{} ", final_op_str)?;

    let verbose = ctx.verbose;
    let mut operands = op.operands.iter().copied();
    let mut locals = op.locals.iter().copied();

    match op.final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => match inc_dec_op {
            IncDecOp::PreInc | IncDecOp::PreIncO => write!(w, "++")?,
            IncDecOp::PreDec | IncDecOp::PreDecO => write!(w, "--")?,
            IncDecOp::PostInc | IncDecOp::PostDec | IncDecOp::PostIncO | IncDecOp::PostDecO => {}
            _ => unreachable!(),
        },
        FinalOp::QueryM { .. }
        | FinalOp::SetM { .. }
        | FinalOp::SetOpM { .. }
        | FinalOp::SetRangeM { .. }
        | FinalOp::UnsetM { .. } => {}
    }

    match op.base_op {
        BaseOp::BaseC { mode, .. } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            let vid = operands.next().unwrap();
            write!(w, "{}", FmtVid(func, vid, verbose))?;
        }
        BaseOp::BaseGC { mode, .. } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            let vid = operands.next().unwrap();
            write!(w, "global {}", FmtVid(func, vid, verbose))?;
        }
        BaseOp::BaseH { .. } => {
            write!(w, "$this")?;
        }
        BaseOp::BaseL { mode, readonly, .. } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            if readonly != ReadonlyOp::Any {
                write!(w, "{} ", FmtReadonly(readonly))?;
            }
            let lid = locals.next().unwrap();
            write!(w, "{}", FmtLid(lid, ctx.strings))?;
        }
        BaseOp::BaseSC { mode, readonly, .. } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            if readonly != ReadonlyOp::Any {
                write!(w, "{} ", FmtReadonly(readonly))?;
            }
            let cls = operands.next().unwrap();
            let prop = operands.next().unwrap();
            write!(
                w,
                "{}::{}",
                FmtVid(func, cls, verbose),
                FmtVid(func, prop, verbose)
            )?;
        }
    }

    for op in op.intermediate_ops.iter() {
        print_inner_loc(w, ctx, func, op.loc)?;

        if op.mode != MOpMode::None {
            write!(w, " {} ", FmtMOpMode(op.mode))?;
        }
        if op.readonly != ReadonlyOp::Any {
            write!(w, " {} ", FmtReadonly(op.readonly))?;
        }
        print_member_key(w, ctx, &mut operands, &mut locals, func, &op.key)?;
    }

    match op.final_op {
        FinalOp::IncDecM {
            ref key,
            readonly,
            loc,
            ..
        }
        | FinalOp::QueryM {
            ref key,
            readonly,
            loc,
            ..
        }
        | FinalOp::SetM {
            ref key,
            readonly,
            loc,
            ..
        }
        | FinalOp::SetOpM {
            ref key,
            readonly,
            loc,
            ..
        }
        | FinalOp::UnsetM {
            ref key,
            readonly,
            loc,
            ..
        } => {
            print_inner_loc(w, ctx, func, loc)?;
            print_member_key(w, ctx, &mut operands, &mut locals, func, key)?;
            if readonly != ReadonlyOp::Any {
                write!(w, " {}", FmtReadonly(readonly))?;
            }
        }
        FinalOp::SetRangeM {
            sz, set_range_op, ..
        } => {
            let s1 = operands.next().unwrap();
            let s2 = operands.next().unwrap();
            let s3 = operands.next().unwrap();
            let set_range_op = match set_range_op {
                SetRangeOp::Forward => "forward",
                SetRangeOp::Reverse => "reverse",
                _ => unreachable!(),
            };
            write!(
                w,
                " set {}, {}, {} size {} {}",
                FmtVid(func, s1, verbose),
                FmtVid(func, s2, verbose),
                FmtVid(func, s3, verbose),
                sz,
                set_range_op
            )?;
        }
    }

    match op.final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => match inc_dec_op {
            IncDecOp::PreInc | IncDecOp::PreDec => {}
            IncDecOp::PreIncO | IncDecOp::PreDecO => write!(w, " overflow")?,
            IncDecOp::PostInc => write!(w, "++")?,
            IncDecOp::PostDec => write!(w, "--")?,
            IncDecOp::PostIncO => write!(w, "++ overflow")?,
            IncDecOp::PostDecO => write!(w, "++ overflow")?,
            _ => unreachable!(),
        },
        FinalOp::QueryM { query_m_op, .. } => match query_m_op {
            QueryMOp::CGet => {}
            QueryMOp::CGetQuiet => write!(w, "quiet ")?,
            QueryMOp::Isset => write!(w, "isset ")?,
            QueryMOp::InOut => write!(w, "inout ")?,
            _ => unreachable!(),
        },
        FinalOp::SetM { .. } => {
            let vid = operands.next().unwrap();
            write!(w, " = {}", FmtVid(func, vid, verbose))?;
        }
        FinalOp::SetOpM { set_op_op, .. } => {
            let vid = operands.next().unwrap();
            write!(
                w,
                " {} {}",
                FmtSetOpOp(set_op_op),
                FmtVid(func, vid, verbose)
            )?;
        }
        FinalOp::SetRangeM { .. } | FinalOp::UnsetM { .. } => {}
    }

    assert_eq!(operands.next(), None);
    Ok(())
}

fn print_member_key(
    w: &mut dyn Write,
    ctx: &FuncContext<'_, '_>,
    operands: &mut impl Iterator<Item = ValueId>,
    locals: &mut impl Iterator<Item = LocalId>,
    func: &Func<'_>,
    key: &MemberKey,
) -> Result {
    match *key {
        MemberKey::EC => {
            let vid = operands.next().unwrap();
            write!(w, "[{}]", FmtVid(func, vid, ctx.verbose))?;
        }
        MemberKey::EI(i) => write!(w, "[{}]", i)?,
        MemberKey::EL => {
            let lid = locals.next().unwrap();
            write!(w, "[{}]", FmtLid(lid, ctx.strings))?
        }
        MemberKey::ET(sid) => write!(w, "[{}]", FmtQuotedStringId(sid, ctx.strings))?,
        MemberKey::PC => {
            let vid = operands.next().unwrap();
            write!(w, "->{}", FmtVid(func, vid, ctx.verbose))?;
        }
        MemberKey::PL => {
            let lid = locals.next().unwrap();
            write!(w, "->{}", FmtLid(lid, ctx.strings))?
        }
        MemberKey::PT(pid) => write!(w, "->{}", FmtQuotedStringId(pid.id, ctx.strings))?,
        MemberKey::QT(pid) => write!(w, "?->{}", FmtQuotedStringId(pid.id, ctx.strings))?,
        MemberKey::W => write!(w, "[]")?,
    }
    Ok(())
}

fn print_method(
    w: &mut dyn Write,
    clsid: ClassId,
    method: &Method<'_>,
    verbose: bool,
    strings: &StringInterner<'_>,
) -> Result {
    writeln!(
        w,
        "method {clsid}::{method}{tparams}{params}{shadowed_tparams} {vis} {{",
        clsid = FmtIdentifierId(clsid.id, strings),
        method = FmtIdentifier(method.name.as_bytes()),
        tparams = FmtTParams(&method.func.tparams, strings),
        shadowed_tparams = FmtShadowedTParams(&method.func.shadowed_tparams, strings),
        params = FmtFuncParams(&method.func, strings),
        vis = FmtVisibility(method.visibility),
    )?;
    print_coeffects(w, &method.coeffects)?;
    print_func_body(w, &method.func, verbose, strings)?;
    writeln!(w, "}}")?;
    writeln!(w)
}

fn incdec_flag(op: IncDecOp) -> &'static str {
    match op {
        IncDecOp::PreInc | IncDecOp::PostInc | IncDecOp::PreDec | IncDecOp::PostDec => "",
        IncDecOp::PreIncO | IncDecOp::PostIncO | IncDecOp::PreDecO | IncDecOp::PostDecO => {
            "overflow"
        }
        _ => panic!("bad IncDecOp value"),
    }
}

fn incdec_what(op: IncDecOp) -> (&'static str, &'static str) {
    let what = match op {
        IncDecOp::PreInc | IncDecOp::PreIncO | IncDecOp::PostInc | IncDecOp::PostIncO => "++",
        IncDecOp::PreDec | IncDecOp::PreDecO | IncDecOp::PostDec | IncDecOp::PostDecO => "--",
        _ => panic!("bad IncDecOp value"),
    };

    match op {
        IncDecOp::PreInc | IncDecOp::PreDec | IncDecOp::PreIncO | IncDecOp::PreDecO => (what, ""),
        IncDecOp::PostInc | IncDecOp::PostDec | IncDecOp::PostIncO | IncDecOp::PostDecO => {
            ("", what)
        }
        _ => panic!("bad IncDecOp value"),
    }
}

pub(crate) fn print_param(
    w: &mut dyn Write,
    strings: &StringInterner<'_>,
    func: &Func<'_>,
    param: &Param<'_>,
) -> Result {
    if param.is_inout {
        write!(w, "inout ")?;
    }
    if param.is_readonly {
        write!(w, "readonly ")?;
    }
    let ellipsis_for_variadic = if param.is_variadic { "..." } else { "" };
    write!(
        w,
        "{} {}{}",
        FmtType(&param.ty),
        ellipsis_for_variadic,
        FmtIdentifierId(param.name, strings)
    )?;
    if let Some((bid, value_str)) = param.default_value {
        write!(
            w,
            " @ {} ({})",
            FmtBid(func, bid, false),
            FmtQuotedStr(&value_str)
        )?;
    }
    Ok(())
}

fn print_property(w: &mut dyn Write, property: &Property<'_>) -> Result {
    writeln!(
        w,
        "  {} {}{}",
        FmtIdentifier(property.name.as_bytes()),
        FmtAttr(property.flags),
        FmtSep::new(" <", ", ", ">", property.attributes.as_ref(), |w, attr| {
            write!(
                w,
                "{}({})",
                FmtIdentifier(attr.name.as_ref()),
                FmtSep::comma(attr.arguments.as_ref(), |w, arg| {
                    FmtTypedValue(arg).fmt(w)
                })
            )
        })
    )
}

fn print_terminator(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_, '_>,
    func: &Func<'_>,
    _iid: InstrId,
    terminator: &Terminator,
) -> Result {
    let verbose = ctx.verbose;
    match terminator {
        Terminator::CallAsync(call, targets) => print_call_async(w, ctx, func, call, targets)?,
        Terminator::Enter(bid, _) => write!(w, "enter to {}", FmtBid(func, *bid, verbose),)?,
        Terminator::Exit(vid, _) => {
            write!(w, "exit {}", FmtVid(func, *vid, verbose))?;
        }
        Terminator::Fatal(vid, op, _) => {
            let op = match *op {
                instr::FatalOp::Parse => "parse",
                instr::FatalOp::Runtime => "runtime",
                instr::FatalOp::RuntimeOmitFrame => "runtime_omit_frame",
                _ => panic!("bad FatalOp value"),
            };
            write!(w, "fatal {}, {}", op, FmtVid(func, *vid, verbose))?
        }
        Terminator::IterInit(args, vid) => {
            write!(
                w,
                "iterator ^{} init from {} jmp to {} else {} with {}",
                args.iter_id.idx,
                FmtVid(func, *vid, verbose),
                FmtBid(func, args.targets[0], verbose),
                FmtBid(func, args.targets[1], verbose),
                FmtOptKeyValue(args.key_lid(), args.value_lid(), ctx.strings)
            )?;
        }
        Terminator::IterNext(args) => {
            write!(
                w,
                "iterator ^{} next jmp to {} else {} with {}",
                args.iter_id.idx,
                FmtBid(func, args.targets[0], verbose),
                FmtBid(func, args.targets[1], verbose),
                FmtOptKeyValue(args.key_lid(), args.value_lid(), ctx.strings)
            )?;
        }
        Terminator::Jmp(bid, _) => write!(w, "jmp to {}", FmtBid(func, *bid, verbose),)?,
        Terminator::JmpArgs(bid, vids, _) => write!(
            w,
            "jmp to {} with ({})",
            FmtBid(func, *bid, verbose),
            FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose).fmt(w))
        )?,
        Terminator::JmpOp {
            cond,
            pred,
            targets,
            loc: _,
        } => {
            let pred = match pred {
                Predicate::NonZero => "jmp if nonzero",
                Predicate::Zero => "jmp if zero",
            };
            write!(
                w,
                "{} {} to {} else {}",
                pred,
                FmtVid(func, *cond, verbose),
                FmtBid(func, targets[0], verbose),
                FmtBid(func, targets[1], verbose),
            )?;
        }
        Terminator::MemoGet(get) => {
            write!(w, "memo_get ")?;
            if !get.locals.is_empty() {
                write!(w, "{} ", FmtLids(&get.locals, ctx.strings))?;
            }
            write!(
                w,
                " to {} else {}",
                FmtBid(func, get.value_edge(), verbose),
                FmtBid(func, get.no_value_edge(), verbose)
            )?;
        }
        Terminator::MemoGetEager(get) => {
            write!(w, "memo_get_eager ")?;
            if !get.locals.is_empty() {
                write!(w, "{} ", FmtLids(&get.locals, ctx.strings))?;
            }
            write!(
                w,
                " to {} eager {} else {}",
                FmtBid(func, get.suspended_edge(), verbose),
                FmtBid(func, get.eager_edge(), verbose),
                FmtBid(func, get.no_value_edge(), verbose)
            )?;
        }
        Terminator::NativeImpl(_) => {
            write!(w, "native_impl")?;
        }
        Terminator::Ret(vid, _) => {
            write!(w, "ret {}", FmtVid(func, *vid, verbose))?;
        }
        Terminator::RetCSuspended(vid, _) => {
            write!(w, "ret_c_suspended {}", FmtVid(func, *vid, verbose))?;
        }
        Terminator::RetM(vids, _) => {
            write!(
                w,
                "ret [{}]",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose).fmt(w))
            )?;
        }
        Terminator::Switch {
            cond,
            bounded,
            targets,
            ..
        } => {
            let bounded = match *bounded {
                SwitchKind::Bounded => "bounded",
                SwitchKind::Unbounded => "unbounded",
                _ => unreachable!(),
            };
            write!(
                w,
                "switch {} {} [{}]",
                bounded,
                FmtVid(func, *cond, verbose),
                FmtSep::comma(targets.iter(), |w, target| {
                    write!(w, "{}", FmtBid(func, *target, verbose),)
                })
            )?;
        }
        Terminator::SSwitch {
            cond,
            cases,
            targets,
            ..
        } => {
            write!(
                w,
                "sswitch {} [{}]",
                FmtVid(func, *cond, verbose),
                FmtSep::comma(cases.iter().zip(targets.iter()), |w, (case, target)| {
                    write!(
                        w,
                        "{} => {}",
                        FmtQuotedStringId(*case, ctx.strings),
                        FmtBid(func, *target, verbose),
                    )
                })
            )?;
        }
        Terminator::Throw(vid, _) => {
            write!(w, "throw {}", FmtVid(func, *vid, verbose))?;
        }
        Terminator::ThrowAsTypeStructException(ops, _) => {
            write!(
                w,
                "throw_as_type_struct_exception {}, {}",
                FmtVid(func, ops[0], verbose),
                FmtVid(func, ops[1], verbose)
            )?;
        }
        Terminator::Unreachable => write!(w, "unreachable")?,
    }
    Ok(())
}

fn print_type_constant(w: &mut dyn Write, tc: &TypeConstant<'_>) -> Result {
    write!(w, "type_constant ")?;
    if tc.is_abstract {
        write!(w, "abstract ")?;
    }
    write!(w, "{}", FmtIdentifier(&tc.name))?;
    if let Some(init) = &tc.initializer {
        write!(w, " = {}", FmtTypedValue(init))?;
    }
    writeln!(w)
}

pub fn print_unit(w: &mut dyn Write, unit: &Unit<'_>, verbose: bool) -> Result {
    for attr in &unit.file_attributes {
        writeln!(w, ".attr {}", FmtAttribute(attr))?;
    }

    for (k, v) in unit.adata.iter().sorted_by(|(k0, _), (k1, _)| k0.cmp(k1)) {
        writeln!(w, ".adata {} = {};", FmtQuotedStr(k), FmtTypedValue(v))?;
    }

    for v in &unit.symbol_refs.constants {
        writeln!(w, ".const_ref {};", FmtIdentifier(v.as_bytes()))?;
    }

    for c in &unit.constants {
        write!(w, ".")?;
        print_hack_constant(w, c)?;
    }

    for typedef in &unit.typedefs {
        // TODO: Fix this.
        writeln!(
            w,
            ".typedef {} = (TBD) {:?}",
            FmtIdentifier(typedef.name.as_bytes()),
            typedef
        )?;
    }

    for c in &unit.classes {
        print_class(w, c, &unit.strings)?;
    }

    for f in &unit.functions {
        print_function(w, f, verbose, &unit.strings)?;
    }

    for c in &unit.classes {
        for m in &c.methods {
            print_method(w, c.name, m, verbose, &unit.strings)?;
        }
    }

    print_fatal(w, &unit.fatal)?;

    Ok(())
}
