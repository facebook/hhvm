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
use ir_core::instr::BaseOp;
use ir_core::instr::FinalOp;
use ir_core::instr::HasLoc;
use ir_core::instr::Hhbc;
use ir_core::instr::IncludeKind;
use ir_core::instr::IrToBc;
use ir_core::instr::MemberKey;
use ir_core::instr::Special;
use ir_core::instr::Terminator;
use ir_core::instr::Tmp;
use ir_core::unit::SymbolRefs;
use ir_core::*;

use crate::formatters::*;
use crate::util::FmtSep;
use crate::FmtEscapedString;

pub(crate) struct FuncContext<'a> {
    pub(crate) cur_loc_id: LocId,
    pub(crate) live_instrs: InstrIdSet,
    pub(crate) strings: &'a StringInterner,
    pub(crate) verbose: bool,
}

fn print_binary_op(w: &mut dyn Write, ctx: &FuncContext<'_>, func: &Func<'_>, op: &Hhbc) -> Result {
    let (prefix, infix, lhs, rhs) = match *op {
        Hhbc::Add([lhs, rhs], _) => ("add", ",", lhs, rhs),
        Hhbc::BitAnd([lhs, rhs], _) => ("bit_and", ",", lhs, rhs),
        Hhbc::BitOr([lhs, rhs], _) => ("bit_or", ",", lhs, rhs),
        Hhbc::BitXor([lhs, rhs], _) => ("bit_xor", ",", lhs, rhs),
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
        Hhbc::Concat([lhs, rhs], _) => ("concat", ",", lhs, rhs),
        Hhbc::Div([lhs, rhs], _) => ("div", ",", lhs, rhs),
        Hhbc::IsTypeStructC([lhs, rhs], op, _) => {
            let op = match op {
                TypeStructResolveOp::Resolve => " resolve",
                TypeStructResolveOp::DontResolve => " dont_resolve",
                _ => panic!("bad TypeStructResolveOp value"),
            };
            ("is_type_struct_c", op, lhs, rhs)
        }
        Hhbc::Modulo([lhs, rhs], _) => ("mod", ",", lhs, rhs),
        Hhbc::Mul([lhs, rhs], _) => ("mul", ",", lhs, rhs),
        Hhbc::Pow([lhs, rhs], _) => ("pow", ",", lhs, rhs),
        Hhbc::Shl([lhs, rhs], _) => ("shl", ",", lhs, rhs),
        Hhbc::Shr([lhs, rhs], _) => ("shr", ",", lhs, rhs),
        Hhbc::Sub([lhs, rhs], _) => ("sub", ",", lhs, rhs),
        _ => unreachable!(),
    };
    write!(
        w,
        "{} {}{} {}",
        prefix,
        FmtVid(func, lhs, ctx.verbose, ctx.strings),
        infix,
        FmtVid(func, rhs, ctx.verbose, ctx.strings)
    )
}

fn print_call(w: &mut dyn Write, ctx: &FuncContext<'_>, func: &Func<'_>, call: &Call) -> Result {
    let verbose = ctx.verbose;
    let strings = ctx.strings;
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
                "call cls_method {}::{}{}",
                FmtVid(func, call.detail.class(&call.operands), verbose, strings),
                FmtVid(func, call.detail.method(&call.operands), verbose, strings),
                dc
            )?;
        }
        CallDetail::FCallClsMethodD { clsid, method } => {
            write!(
                w,
                "call cls_method {}::{}",
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
                "call cls_method {}::{}{}",
                FmtVid(func, call.detail.class(&call.operands), verbose, strings),
                FmtIdentifierId(method.id, ctx.strings),
                dc
            )?;
        }
        CallDetail::FCallClsMethodS { clsref } => {
            write!(
                w,
                "call cls_method {}::{}",
                FmtSpecialClsRef(*clsref),
                FmtVid(func, call.detail.method(&call.operands), verbose, strings),
            )?;
        }
        CallDetail::FCallClsMethodSD { clsref, method } => {
            write!(
                w,
                "call cls_method {}::{}",
                FmtSpecialClsRef(*clsref),
                FmtIdentifierId(method.id, ctx.strings),
            )?;
        }
        CallDetail::FCallCtor => {
            write!(
                w,
                "call ctor {}",
                FmtVid(func, call.detail.obj(&call.operands), verbose, strings)
            )?;
        }
        CallDetail::FCallFunc => {
            write!(
                w,
                "call func {}",
                FmtVid(func, call.detail.target(&call.operands), verbose, strings)
            )?;
        }
        CallDetail::FCallFuncD { func } => {
            write!(w, "call func {}", FmtIdentifierId(func.id, ctx.strings))?;
        }
        CallDetail::FCallObjMethod { flavor } => {
            let arrow = match *flavor {
                ObjMethodOp::NullThrows => "->",
                ObjMethodOp::NullSafe => "?->",
                _ => unreachable!(),
            };
            write!(
                w,
                "call obj_method {}{arrow}{}",
                FmtVid(func, call.detail.obj(&call.operands), verbose, strings),
                FmtVid(func, call.detail.method(&call.operands), verbose, strings)
            )?;
        }
        CallDetail::FCallObjMethodD { method, flavor } => {
            let arrow = match *flavor {
                ObjMethodOp::NullThrows => "->",
                ObjMethodOp::NullSafe => "?->",
                _ => unreachable!(),
            };
            write!(
                w,
                "call obj_method {}{arrow}{}",
                FmtVid(func, call.detail.obj(&call.operands), verbose, strings),
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
            write!(
                w,
                "{}{}{}",
                readonly,
                inout,
                FmtVid(func, *arg, verbose, strings)
            )
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
    ctx: &FuncContext<'_>,
    func: &Func<'_>,
    call: &Call,
    targets: &[BlockId; 2],
) -> Result {
    write!(w, "async_")?;
    print_call(w, ctx, func, call)?;
    write!(
        w,
        " to {} eager {}",
        FmtBid(func, targets[0], ctx.verbose),
        FmtBid(func, targets[1], ctx.verbose),
    )
}

fn print_class(w: &mut dyn Write, class: &Class<'_>, strings: &StringInterner) -> Result {
    print_top_level_loc(w, Some(&class.src_loc), strings)?;
    writeln!(
        w,
        "class {} {} {{",
        FmtIdentifierId(class.name.id, strings),
        FmtAttr(class.flags, AttrContext::Class)
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
        writeln!(w, "  enum_type {}", FmtTypeInfo(et, strings))?;
    }

    if !class.enum_includes.is_empty() {
        writeln!(
            w,
            "  enum_includes {}",
            FmtSep::comma(class.enum_includes.iter(), |w, ie| FmtIdentifierId(
                ie.id, strings
            )
            .fmt(w))
        )?;
    }

    for (name, tys) in &class.upper_bounds {
        writeln!(
            w,
            "  upper_bound {}: [{}]",
            FmtIdentifier(name.as_ref()),
            FmtSep::comma(tys.iter(), |w, ty| FmtTypeInfo(ty, strings).fmt(w))
        )?;
    }

    for ctx in &class.ctx_constants {
        print_ctx_constant(w, ctx)?;
    }

    for tc in &class.type_constants {
        print_type_constant(w, tc, strings)?;
    }

    for use_ in &class.uses {
        writeln!(w, "  uses {}", FmtIdentifierId(use_.id, strings))?;
    }

    for req in &class.requirements {
        let kind = match req.kind {
            TraitReqKind::MustExtend => "extends",
            TraitReqKind::MustImplement => "implements",
            TraitReqKind::MustBeClass => "must_be_class",
        };
        writeln!(
            w,
            "  require {} {}",
            kind,
            FmtIdentifierId(req.name.id, strings)
        )?;
    }

    for prop in &class.properties {
        print_property(w, prop, strings)?;
    }

    for attr in &class.attributes {
        writeln!(w, "  attribute {}", FmtAttribute(attr, strings))?;
    }

    for c in &class.constants {
        write!(w, "  ")?;
        print_hack_constant(w, c, strings)?;
    }

    for method in &class.methods {
        writeln!(w, "  method {}", FmtIdentifierId(method.name.id, strings))?;
    }

    writeln!(w, "}}")?;
    writeln!(w)
}

pub(crate) fn print_coeffects(w: &mut dyn Write, coeffects: &Coeffects<'_>) -> Result {
    if !coeffects.static_coeffects.is_empty() || !coeffects.unenforced_static_coeffects.is_empty() {
        write!(
            w,
            "  .coeffects_static enforced({})",
            FmtSep::comma(coeffects.static_coeffects.iter(), |f, v| v.fmt(f))
        )?;
        if !coeffects.unenforced_static_coeffects.is_empty() {
            write!(
                w,
                " unenforced({})",
                FmtSep::comma(coeffects.unenforced_static_coeffects.iter(), |f, v| {
                    FmtIdentifier(v).fmt(f)
                })
            )?;
        }
        writeln!(w)?;
    }

    if !coeffects.fun_param.is_empty() {
        writeln!(
            w,
            "  .coeffects_fun_param {}",
            FmtSep::comma(coeffects.fun_param.iter(), |w, v| write!(w, "{v}"))
        )?;
    }

    for CcParam { index, ctx_name } in &coeffects.cc_param {
        writeln!(w, ".coeffects_cc_param {index} {}", FmtQuotedStr(ctx_name))?;
    }

    for CcThis { types } in &coeffects.cc_this {
        writeln!(
            w,
            ".coeffects_cc_this {}",
            FmtSep::comma(types.iter(), |f, v| { FmtQuotedStr(v).fmt(f) })
        )?;
    }

    for CcReified {
        is_class,
        index,
        types,
    } in &coeffects.cc_reified
    {
        writeln!(
            w,
            "  .coeffects_cc_reified {}{} {}",
            if *is_class { "is_class " } else { "" },
            index,
            FmtSep::new("", "::", "", types.iter(), |w, qn| FmtIdentifier(qn).fmt(w))
        )?;
    }

    if coeffects.closure_parent_scope {
        writeln!(w, "  .coeffects_closure_parent_scope")?;
    }

    if coeffects.generator_this {
        writeln!(w, "  .coeffects_generator_this")?;
    }

    if coeffects.caller {
        writeln!(w, "  .coeffects_caller")?;
    }

    Ok(())
}

fn print_ctx_constant(w: &mut dyn Write, ctx: &CtxConstant<'_>) -> Result {
    writeln!(
        w,
        "  ctx_constant {} [{}] [{}]{}",
        FmtIdentifier(&ctx.name),
        FmtSep::comma(ctx.recognized.iter(), |w, i| FmtIdentifier(i).fmt(w)),
        FmtSep::comma(ctx.unrecognized.iter(), |w, i| FmtIdentifier(i).fmt(w)),
        if ctx.is_abstract { " abstract" } else { "" }
    )
}

pub(crate) fn print_fatal(
    w: &mut dyn Write,
    fatal: Option<&Fatal>,
    strings: &StringInterner,
) -> Result {
    if let Some(Fatal { op, loc, message }) = fatal {
        let what = match *op {
            ir_core::FatalOp::Parse => "parse",
            ir_core::FatalOp::Runtime => "runtime",
            ir_core::FatalOp::RuntimeOmitFrame => "runtime_omit_frame",
            _ => unreachable!(),
        };

        print_top_level_loc(w, Some(loc), strings)?;
        writeln!(
            w,
            ".fatal {} {}\n",
            what,
            FmtQuotedStr(&ffi::Str::new(message))
        )?;
    }
    Ok(())
}

pub(crate) fn print_func_body(
    w: &mut dyn Write,
    func: &Func<'_>,
    verbose: bool,
    strings: &StringInterner,
    f_pre_block: Option<&dyn Fn(&mut dyn Write, BlockId) -> Result>,
    f_pre_instr: Option<&dyn Fn(&mut dyn Write, InstrId) -> Result>,
) -> Result {
    if let Some(doc_comment) = func.doc_comment.as_ref() {
        writeln!(w, "  .doc {}", FmtQuotedStr(doc_comment))?;
    }
    if func.num_iters != 0 {
        writeln!(w, "  .num_iters {}", func.num_iters)?;
    }
    if func.is_memoize_wrapper {
        writeln!(w, "  .is_memoize_wrapper")?;
    }
    if func.is_memoize_wrapper_lsb {
        writeln!(w, "  .is_memoize_wrapper_lsb")?;
    }

    for cid in func.constants.keys() {
        writeln!(
            w,
            "  .const {} = {}",
            FmtRawVid(ValueId::from_constant(cid)),
            FmtConstantId(func, cid, strings),
        )?;
    }

    for (id, func::ExFrame { parent, catch_bid }) in &func.ex_frames {
        write!(
            w,
            "  .ex_frame {}: catch={}",
            id.as_usize(),
            FmtBid(func, *catch_bid, verbose)
        )?;
        match parent {
            TryCatchId::None => {}
            TryCatchId::Try(id) => write!(w, ", parent=try({})", id.as_usize())?,
            TryCatchId::Catch(id) => write!(w, ", parent=catch({})", id.as_usize())?,
        }
        writeln!(w)?;
    }

    let live_instrs = crate::util::compute_live_instrs(func, verbose);

    let mut ctx = FuncContext {
        cur_loc_id: func.loc_id,
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
                    verbose,
                    strings
                )
                .fmt(w))
            )?;
        }

        writeln!(w, ":")?;

        if let Some(f_pre_block) = f_pre_block.as_ref() {
            f_pre_block(w, bid)?;
        }

        match block.tcid {
            TryCatchId::None => {}
            TryCatchId::Try(id) => writeln!(w, "  .try_id {}", id.as_usize())?,
            TryCatchId::Catch(id) => writeln!(w, "  .catch_id {}", id.as_usize())?,
        }

        for iid in block.iids() {
            if let Some(f_pre_instr) = f_pre_instr.as_ref() {
                f_pre_instr(w, iid)?;
            }
            let instr = func.instr(iid);
            if crate::print::print_instr(w, &mut ctx, func, iid, instr)? {
                writeln!(w)?;
            }
        }
    }

    Ok(())
}

fn print_attributes(w: &mut dyn Write, attrs: &[Attribute], strings: &StringInterner) -> Result {
    for attr in attrs {
        writeln!(w, "  .attr {}", FmtAttribute(attr, strings))?;
    }
    Ok(())
}

fn print_top_level_loc(
    w: &mut dyn Write,
    src_loc: Option<&SrcLoc>,
    strings: &StringInterner,
) -> Result {
    if let Some(loc) = src_loc {
        writeln!(w, ".srcloc {}", FmtFullLoc(loc, strings))?;
    } else {
        writeln!(w, ".srcloc none")?;
    }
    Ok(())
}

fn print_function(
    w: &mut dyn Write,
    f: &Function<'_>,
    verbose: bool,
    strings: &StringInterner,
) -> Result {
    print_top_level_loc(w, f.func.get_loc(f.func.loc_id), strings)?;
    writeln!(
        w,
        "function {name}{tparams}{params}{shadowed_tparams}: {ret_type} {attr} {{",
        name = FmtIdentifierId(f.name.id, strings),
        tparams = FmtTParams(&f.func.tparams, strings),
        shadowed_tparams = FmtShadowedTParams(&f.func.shadowed_tparams, strings),
        params = FmtFuncParams(&f.func, strings),
        ret_type = FmtTypeInfo(&f.func.return_type, strings),
        attr = FmtAttr(f.attrs, AttrContext::Function),
    )?;
    print_function_flags(w, f.flags)?;
    print_attributes(w, &f.attributes, strings)?;
    print_coeffects(w, &f.coeffects)?;
    print_func_body(w, &f.func, verbose, strings, None, None)?;
    writeln!(w, "}}")?;
    writeln!(w)
}

fn print_function_flags(w: &mut dyn Write, mut flags: FunctionFlags) -> Result {
    [
        get_bit(&mut flags, FunctionFlags::ASYNC, ".async"),
        get_bit(&mut flags, FunctionFlags::GENERATOR, ".generator"),
        get_bit(&mut flags, FunctionFlags::PAIR_GENERATOR, ".pair_generator"),
        get_bit(&mut flags, FunctionFlags::MEMOIZE_IMPL, ".memoize_impl"),
    ]
    .into_iter()
    .flatten()
    .try_for_each(|f| writeln!(w, "  {f}"))
}

fn print_hhbc(w: &mut dyn Write, ctx: &FuncContext<'_>, func: &Func<'_>, hhbc: &Hhbc) -> Result {
    let verbose = ctx.verbose;
    let strings = ctx.strings;
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
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings)
            )?;
        }
        Hhbc::AddElemC(ops, _) => {
            write!(
                w,
                "add_elem_c {}[{}] = {}",
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings),
                FmtVid(func, ops[2], verbose, strings),
            )?;
        }
        Hhbc::AddNewElemC(ops, _) => {
            write!(
                w,
                "add_new_elem_c {}[] = {}",
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings),
            )?;
        }
        Hhbc::ArrayIdx(vids, _) => {
            write!(
                w,
                "array_idx {}[{}] or {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings),
                FmtVid(func, vids[2], verbose, strings)
            )?;
        }
        Hhbc::ArrayMarkLegacy(vids, _) => {
            write!(
                w,
                "array_mark_legacy {}, {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings)
            )?;
        }
        Hhbc::ArrayUnmarkLegacy(vids, _) => {
            write!(
                w,
                "array_unmark_legacy {}, {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings)
            )?;
        }
        Hhbc::Await(vid, _) => {
            write!(w, "await {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::AwaitAll(ref range, _) => {
            write!(w, "await_all {}", FmtLids(range, ctx.strings),)?;
        }
        Hhbc::BareThis(op, _) => {
            write!(w, "bare_this {}", FmtBareThisOp(op))?;
        }
        Hhbc::BitNot(vid, _) => write!(w, "bit_not {}", FmtVid(func, vid, ctx.verbose, strings),)?,
        Hhbc::CGetG(vid, _) => write!(w, "get_global {}", FmtVid(func, vid, verbose, strings))?,
        Hhbc::CGetL(lid, _) => write!(w, "get_local {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CGetQuietL(lid, _) => write!(w, "get_local_quiet {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CGetS(vids, readonly, _) => write!(
            w,
            "get_static {}->{} {}",
            FmtVid(func, vids[1], verbose, strings),
            FmtVid(func, vids[0], verbose, strings),
            FmtReadonly(readonly)
        )?,
        Hhbc::CUGetL(lid, _) => write!(w, "get_local_or_uninit {}", FmtLid(lid, ctx.strings))?,
        Hhbc::CastBool(vid, _) => {
            write!(w, "cast_bool {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastDict(vid, _) => {
            write!(w, "cast_dict {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastDouble(vid, _) => {
            write!(w, "cast_double {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastInt(vid, _) => {
            write!(w, "cast_int {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastKeyset(vid, _) => {
            write!(w, "cast_keyset {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastString(vid, _) => {
            write!(w, "cast_string {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::CastVec(vid, _) => {
            write!(w, "cast_vec {}", FmtVid(func, vid, ctx.verbose, strings),)?
        }
        Hhbc::ChainFaults(ops, _) => {
            write!(
                w,
                "chain_faults {}, {}",
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings)
            )?;
        }
        Hhbc::CheckClsReifiedGenericMismatch(vid, _) => {
            write!(
                w,
                "check_cls_reified_generic_mismatch {}",
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::CheckClsRGSoft(vid, _) => {
            write!(
                w,
                "check_cls_rg_soft {}",
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::CheckProp(prop, _) => {
            write!(w, "check_prop {}", FmtIdentifierId(prop.id, ctx.strings))?
        }
        Hhbc::CheckThis(_) => {
            write!(w, "check_this")?;
        }
        Hhbc::ClassGetC(vid, _) => {
            write!(w, "class_get_c {}", FmtVid(func, vid, verbose, strings))?
        }
        Hhbc::ClassGetTS(vid, _) => {
            write!(w, "class_get_ts {}", FmtVid(func, vid, verbose, strings))?
        }
        Hhbc::ClassHasReifiedGenerics(vid, _) => write!(
            w,
            "class_has_reified_generics {}",
            FmtVid(func, vid, verbose, strings)
        )?,
        Hhbc::ClassName(vid, _) => {
            write!(w, "class_name {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::Clone(vid, _) => write!(w, "clone {}", FmtVid(func, vid, ctx.verbose, strings),)?,
        Hhbc::ClsCns(clsid, id, _) => {
            write!(
                w,
                "cls_cns {}::{}",
                FmtVid(func, clsid, verbose, strings),
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
                FmtVid(func, vid, verbose, strings),
                FmtLid(lid, ctx.strings)
            )?;
        }
        Hhbc::ColFromArray(vid, kind, _) => {
            write!(
                w,
                "col_from_array {} {}",
                FmtCollectionType(kind),
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::CombineAndResolveTypeStruct(ref vids, _) => {
            write!(
                w,
                "combine_and_resolve_type_struct {}",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose, strings)
                    .fmt(w))
            )?;
        }
        Hhbc::ConcatN(ref vids, _) => {
            write!(
                w,
                "concatn {}",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose, strings)
                    .fmt(w))
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
            write!(w, "cont_enter {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::ContGetReturn(_) => {
            write!(w, "cont_get_return")?;
        }
        Hhbc::ContKey(_) => {
            write!(w, "cont_key")?;
        }
        Hhbc::ContRaise(vid, _) => {
            write!(w, "cont_raise {}", FmtVid(func, vid, verbose, strings))?;
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
                    FmtVid(func, *arg, verbose, strings)
                ))
            )?;
        }
        Hhbc::CreateCont(_) => write!(w, "create_cont")?,
        Hhbc::CreateSpecialImplicitContext(vids, _) => {
            write!(
                w,
                "create_special_implicit_context {}, {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings)
            )?;
        }
        Hhbc::EnumClassLabelName(vid, _) => {
            write!(
                w,
                "enum_class_label_name {}",
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::GetClsRGProp(vid, _) => write!(
            w,
            "get_class_rg_prop {}",
            FmtVid(func, vid, verbose, strings)
        )?,
        Hhbc::GetMemoKeyL(lid, _) => {
            write!(w, "get_memo_key {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::HasReifiedParent(vid, _) => write!(
            w,
            "has_reified_parent {}",
            FmtVid(func, vid, verbose, strings)
        )?,
        Hhbc::Idx(vids, _) => {
            write!(
                w,
                "idx {}[{}] or {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings),
                FmtVid(func, vids[2], verbose, strings),
            )?;
        }
        Hhbc::IncDecL(lid, op, _) => {
            let (pre, post) = incdec_what(op);
            let lid = FmtLid(lid, ctx.strings);
            write!(w, "incdec_local {}{}{}", pre, lid, post)?;
        }
        Hhbc::IncDecS([cls, prop], op, _) => {
            let (pre, post) = incdec_what(op);
            write!(
                w,
                "incdec_static_prop {}{}::{}{}",
                pre,
                FmtVid(func, cls, verbose, strings),
                FmtVid(func, prop, verbose, strings),
                post
            )?;
        }
        Hhbc::IncludeEval(ref ie) => print_include_eval(w, ctx, func, ie)?,
        Hhbc::InitProp(vid, prop, op, _) => {
            write!(
                w,
                "init_prop {}, {}, {}",
                FmtQuotedStringId(prop.id, ctx.strings),
                FmtVid(func, vid, verbose, strings),
                FmtInitPropOp(op)
            )?;
        }
        Hhbc::InstanceOfD(vid, clsid, _) => write!(
            w,
            "instance_of_d {}, {}",
            FmtVid(func, vid, ctx.verbose, strings),
            FmtIdentifierId(clsid.id, ctx.strings)
        )?,
        Hhbc::IsLateBoundCls(vid, _) => {
            write!(
                w,
                "is_late_bound_cls {}",
                FmtVid(func, vid, ctx.verbose, strings)
            )?;
        }
        Hhbc::IsTypeC(vid, op, _) => {
            write!(
                w,
                "is_type_c {}, {}",
                FmtVid(func, vid, ctx.verbose, strings),
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
            write!(w, "isset_g {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::IssetL(lid, _) => {
            write!(w, "isset_l {}", FmtLid(lid, ctx.strings))?;
        }
        Hhbc::IssetS([cls, prop], _) => {
            write!(
                w,
                "isset_s {}::{}",
                FmtVid(func, cls, verbose, strings),
                FmtVid(func, prop, verbose, strings)
            )?;
        }
        Hhbc::IterFree(iter_id, _loc) => {
            write!(w, "iterator ^{} free", iter_id.idx)?;
        }
        Hhbc::LateBoundCls(_) => {
            write!(w, "late_bound_cls")?;
        }
        Hhbc::LazyClassFromClass(vid, _) => {
            write!(
                w,
                "lazy_class_from_class {}",
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::LockObj(vid, _) => {
            write!(w, "lock_obj {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::MemoSet(vid, ref locals, _) => {
            write!(
                w,
                "memo_set {}, {}",
                FmtLids(locals, ctx.strings),
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::MemoSetEager(vid, ref locals, _) => {
            write!(
                w,
                "memo_set_eager {}, {}",
                FmtLids(locals, ctx.strings),
                FmtVid(func, vid, verbose, strings)
            )?;
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
                    FmtVid(func, *arg, verbose, strings)
                ))
            )?;
        }
        Hhbc::NewObj(vid, _) => {
            write!(w, "new_obj {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::NewObjD(clsid, _) => {
            write!(
                w,
                "new_obj direct {}",
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
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings)
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
                        FmtVid(func, *v, verbose, strings)
                    )
                })
            )?;
        }
        Hhbc::NewVec(ref vids, _) => {
            write!(
                w,
                "new_vec [{}]",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose, strings)
                    .fmt(w))
            )?;
        }
        Hhbc::Not(vid, _) => write!(w, "not {}", FmtVid(func, vid, ctx.verbose, strings))?,
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
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings),
                kind
            )?;
        }
        Hhbc::ParentCls(_) => {
            write!(w, "parent")?;
        }
        Hhbc::Print(vid, _) => write!(w, "print {}", FmtVid(func, vid, ctx.verbose, strings))?,
        Hhbc::RaiseClassStringConversionNotice(_) => {
            write!(w, "raise_class_string_conversion_notice")?
        }
        Hhbc::RecordReifiedGeneric(vid, _) => write!(
            w,
            "record_reified_generic {}",
            FmtVid(func, vid, ctx.verbose, strings)
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
                FmtVid(func, vid, ctx.verbose, strings),
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
                "resolve_r_cls_method {}::{}, {}",
                FmtVid(func, clsid, verbose, strings),
                FmtIdentifierId(method.id, ctx.strings),
                FmtVid(func, vid, verbose, strings),
            )?;
        }
        Hhbc::ResolveRClsMethodS(vid, clsref, method, _) => {
            write!(
                w,
                "resolve_r_cls_method_s {}::{}, {}",
                FmtSpecialClsRef(clsref),
                FmtIdentifierId(method.id, ctx.strings),
                FmtVid(func, vid, verbose, strings),
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
                FmtVid(func, vid, verbose, strings),
            )?;
        }
        Hhbc::ResolveRFunc(rid, fid, _) => {
            write!(
                w,
                "resolve_r_func {}, {}",
                FmtIdentifierId(fid.id, ctx.strings),
                FmtVid(func, rid, verbose, strings)
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
                FmtVid(func, target, verbose, strings),
                FmtVid(func, value, verbose, strings),
            )?;
        }
        Hhbc::SetImplicitContextByValue(vid, _) => {
            write!(
                w,
                "set_implicit_context_by_value {}",
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::SetL(vid, lid, _) => {
            write!(
                w,
                "set_local {}, {}",
                FmtLid(lid, ctx.strings),
                FmtVid(func, vid, verbose, strings),
            )?;
        }
        Hhbc::SetOpL(vid, lid, op, _) => {
            write!(
                w,
                "set_op_local {} {} {}",
                FmtLid(lid, ctx.strings),
                FmtSetOpOp(op),
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        Hhbc::SetOpG([x, y], op, _) => {
            write!(
                w,
                "set_op_global {} {} {}",
                FmtVid(func, x, verbose, strings),
                FmtSetOpOp(op),
                FmtVid(func, y, verbose, strings)
            )?;
        }
        Hhbc::SetOpS(vids, op, _) => {
            write!(
                w,
                "set_op_static_property {}, {}, {}, {}",
                FmtVid(func, vids[0], verbose, strings),
                FmtVid(func, vids[1], verbose, strings),
                FmtVid(func, vids[2], verbose, strings),
                FmtSetOpOp(op),
            )?;
        }
        Hhbc::SetS(vids, readonly, _) => {
            write!(
                w,
                "set_s {}->{} {} = {}",
                FmtVid(func, vids[1], verbose, strings),
                FmtVid(func, vids[0], verbose, strings),
                FmtReadonly(readonly),
                FmtVid(func, vids[2], verbose, strings)
            )?;
        }
        Hhbc::Silence(lid, op, _) => {
            let lid = FmtLid(lid, ctx.strings);
            let op = match op {
                SilenceOp::Start => "start",
                SilenceOp::End => "end",
                _ => unreachable!(),
            };
            write!(w, "silence {}, {}", lid, op)?;
        }
        Hhbc::This(_) => {
            write!(w, "this")?;
        }
        Hhbc::ThrowNonExhaustiveSwitch(_) => {
            write!(w, "throw_nonexhaustive_switch")?;
        }
        Hhbc::UnsetG(vid, _) => {
            write!(w, "unset {}", FmtVid(func, vid, verbose, strings))?;
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
                FmtVid(func, vid, verbose, strings),
                FmtLid(lid, ctx.strings),
            )?;
        }
        Hhbc::VerifyParamType(vid, lid, _) => {
            write!(
                w,
                "verify_param_type {}, {}",
                FmtVid(func, vid, verbose, strings),
                FmtLid(lid, ctx.strings),
            )?;
        }
        Hhbc::VerifyParamTypeTS(vid, lid, _) => {
            write!(
                w,
                "verify_param_type_ts {}, {}",
                FmtVid(func, vid, verbose, strings),
                FmtLid(lid, ctx.strings),
            )?;
        }
        Hhbc::VerifyRetTypeC(op, _) => {
            write!(
                w,
                "verify_ret_type_c {}",
                FmtVid(func, op, verbose, strings)
            )?;
        }
        Hhbc::VerifyRetTypeTS(ops, _) => {
            write!(
                w,
                "verify_ret_type_ts {}, {}",
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings)
            )?;
        }
        Hhbc::WHResult(vid, _) => {
            write!(w, "wh_result {}", FmtVid(func, vid, verbose, strings))?;
        }
        Hhbc::Yield(vid, _) => write!(w, "yield {}", FmtVid(func, vid, verbose, strings))?,
        Hhbc::YieldK(ops, _) => write!(
            w,
            "yield {} => {}",
            FmtVid(func, ops[0], verbose, strings),
            FmtVid(func, ops[1], verbose, strings)
        )?,
    }
    Ok(())
}

fn print_hack_constant(w: &mut dyn Write, c: &HackConstant, strings: &StringInterner) -> Result {
    let attr = FmtAttr(c.attrs, AttrContext::Constant);

    write!(
        w,
        "constant {attr} {name}",
        name = FmtIdentifierId(c.name.id, strings)
    )?;

    if let Some(value) = &c.value {
        write!(w, " = {}", FmtTypedValue(value, strings))?;
    }

    writeln!(w)?;

    Ok(())
}

fn print_include_eval(
    w: &mut dyn Write,
    ctx: &FuncContext<'_>,
    func: &Func<'_>,
    ie: &instr::IncludeEval,
) -> Result {
    let vid = FmtVid(func, ie.vid, ctx.verbose, ctx.strings);
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
    ctx: &mut FuncContext<'_>,
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
            write!(w, "copy {}", FmtVid(func, *vid, ctx.verbose, ctx.strings))?;
        }
        Instr::Special(Special::IrToBc(ir_to_bc)) => {
            print_ir_to_bc(w, ctx, func, ir_to_bc)?;
        }
        Instr::Special(Special::Textual(textual)) => print_textual(w, ctx, func, textual)?,
        Instr::Special(Special::Tmp(Tmp::GetVar(var))) => {
            write!(w, "get_var &{}", var.as_usize())?;
        }
        Instr::Special(Special::Tmp(Tmp::SetVar(var, value))) => {
            write!(
                w,
                "set_var &{}, {}",
                var.as_usize(),
                FmtVid(func, *value, ctx.verbose, ctx.strings)
            )?;
        }
        Instr::Special(Special::Param) => write!(w, "param")?,
        Instr::Special(Special::Select(vid, index)) => write!(
            w,
            "select {} from {}",
            index,
            FmtVid(func, *vid, ctx.verbose, ctx.strings)
        )?,
        Instr::Special(Special::Tombstone) => write!(w, "tombstone")?,
        Instr::Terminator(t) => print_terminator(w, ctx, func, iid, t)?,
    }
    Ok(true)
}

pub(crate) fn print_textual(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
    func: &Func<'_>,
    textual: &instr::Textual,
) -> std::result::Result<(), Error> {
    use instr::Textual;
    let verbose = ctx.verbose;
    let strings = ctx.strings;
    match textual {
        Textual::AssertFalse(vid, _) => {
            write!(
                w,
                "textual::assert_false {}",
                FmtVid(func, *vid, verbose, strings)
            )?;
        }
        Textual::AssertTrue(vid, _) => {
            write!(
                w,
                "textual::assert_true {}",
                FmtVid(func, *vid, verbose, strings)
            )?;
        }
        Textual::Deref(lid) => {
            write!(w, "textual::deref {}", FmtLid(*lid, ctx.strings),)?;
        }
        Textual::HackBuiltin {
            values,
            target,
            loc: _,
        } => {
            write!(w, "textual::hack_builtin({target}")?;
            for vid in values.iter() {
                write!(w, ", {}", FmtVid(func, *vid, verbose, strings))?
            }
            write!(w, ")")?;
        }
        Textual::LoadGlobal { id, is_const } => {
            write!(
                w,
                "textual::load_global({}, {is_const})",
                FmtIdentifierId(id.id, strings)
            )?;
        }
        Textual::String(s) => {
            write!(w, "textual::string({:?})", s)?;
        }
    }
    Ok(())
}

pub(crate) fn print_ir_to_bc(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
    func: &Func<'_>,
    ir_to_bc: &IrToBc,
) -> std::result::Result<(), Error> {
    match ir_to_bc {
        IrToBc::PopC => write!(w, "popc")?,
        IrToBc::PopL(lid) => write!(w, "pop_local {}", FmtLid(*lid, ctx.strings),)?,
        IrToBc::PushL(lid) => {
            write!(w, "push {}", FmtLid(*lid, ctx.strings))?;
        }
        IrToBc::PushConstant(vid) => {
            write!(w, "push {}", FmtVid(func, *vid, ctx.verbose, ctx.strings))?
        }
        IrToBc::PushUninit => write!(w, "push_uninit")?,
        IrToBc::UnsetL(lid) => {
            write!(w, "unset_local {}", FmtLid(*lid, ctx.strings))?;
        }
    }
    Ok(())
}

fn print_inner_loc(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
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
    ctx: &mut FuncContext<'_>,
    func: &Func<'_>,
    loc_id: LocId,
) -> Result {
    if ctx.cur_loc_id != loc_id {
        if let Some(loc) = func.get_loc(loc_id) {
            let old_filename = func
                .get_loc(ctx.cur_loc_id)
                .map_or(UnitBytesId::NONE, |loc| loc.filename.0);
            if old_filename != loc.filename.0 {
                writeln!(w, "  .srcloc {}", FmtFullLoc(loc, ctx.strings))?;
            } else {
                writeln!(w, "  .srcloc {}", FmtLoc(loc))?;
            }
        }
        ctx.cur_loc_id = loc_id;
    }
    Ok(())
}

fn print_member_op(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
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
    let strings = ctx.strings;
    let mut operands = op.operands.iter().copied();
    let mut locals = op.locals.iter().copied();

    match op.final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => match inc_dec_op {
            IncDecOp::PreInc => write!(w, "++")?,
            IncDecOp::PreDec => write!(w, "--")?,
            IncDecOp::PostInc | IncDecOp::PostDec => {}
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
            write!(w, "{}", FmtVid(func, vid, true, strings))?;
        }
        BaseOp::BaseGC { mode, .. } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            let vid = operands.next().unwrap();
            write!(w, "global {}", FmtVid(func, vid, verbose, strings))?;
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
            let prop = operands.next().unwrap();
            let cls = operands.next().unwrap();
            write!(
                w,
                "{}::{}",
                FmtVid(func, cls, true, strings),
                FmtVid(func, prop, verbose, strings)
            )?;
        }
        BaseOp::BaseST {
            mode,
            readonly,
            prop,
            ..
        } => {
            if mode != MOpMode::None {
                write!(w, "{} ", FmtMOpMode(mode))?;
            }
            if readonly != ReadonlyOp::Any {
                write!(w, "{} ", FmtReadonly(readonly))?;
            }
            let cls = operands.next().unwrap();
            write!(
                w,
                "{}::{}",
                FmtVid(func, cls, true, strings),
                FmtQuotedStringId(prop.id, strings)
            )?;
        }
    }

    for op in op.intermediate_ops.iter() {
        print_member_key(
            w,
            ctx,
            &mut operands,
            &mut locals,
            func,
            op.loc,
            op.mode,
            op.readonly,
            &op.key,
        )?;
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
            print_member_key(
                w,
                ctx,
                &mut operands,
                &mut locals,
                func,
                loc,
                MOpMode::None,
                readonly,
                key,
            )?;
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
                FmtVid(func, s1, verbose, strings),
                FmtVid(func, s2, verbose, strings),
                FmtVid(func, s3, verbose, strings),
                sz,
                set_range_op
            )?;
        }
    }

    match op.final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => match inc_dec_op {
            IncDecOp::PreInc | IncDecOp::PreDec => {}
            IncDecOp::PostInc => write!(w, " ++")?,
            IncDecOp::PostDec => write!(w, " --")?,
            _ => unreachable!(),
        },
        FinalOp::QueryM { query_m_op, .. } => match query_m_op {
            QueryMOp::CGet => {}
            QueryMOp::CGetQuiet => write!(w, " quiet")?,
            QueryMOp::Isset => write!(w, " isset")?,
            QueryMOp::InOut => write!(w, " inout")?,
            _ => unreachable!(),
        },
        FinalOp::SetM { .. } => {
            let vid = operands.next().unwrap();
            write!(w, " = {}", FmtVid(func, vid, verbose, strings))?;
        }
        FinalOp::SetOpM { set_op_op, .. } => {
            let vid = operands.next().unwrap();
            write!(
                w,
                " {} {}",
                FmtSetOpOp(set_op_op),
                FmtVid(func, vid, verbose, strings)
            )?;
        }
        FinalOp::SetRangeM { .. } | FinalOp::UnsetM { .. } => {}
    }

    assert_eq!(operands.next(), None);
    Ok(())
}

fn print_member_key(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
    operands: &mut impl Iterator<Item = ValueId>,
    locals: &mut impl Iterator<Item = LocalId>,
    func: &Func<'_>,
    loc: LocId,
    mode: MOpMode,
    readonly: ReadonlyOp,
    key: &MemberKey,
) -> Result {
    let verbose = ctx.verbose;
    let strings = ctx.strings;
    match *key {
        MemberKey::EC | MemberKey::EI(_) | MemberKey::EL | MemberKey::ET(_) | MemberKey::W => {
            w.write_str("[")?
        }
        MemberKey::PC | MemberKey::PL | MemberKey::PT(_) => w.write_str("->")?,
        MemberKey::QT(_) => w.write_str("?->")?,
    }

    print_inner_loc(w, ctx, func, loc)?;

    if mode != MOpMode::None {
        write!(w, " {} ", FmtMOpMode(mode))?;
    }
    if readonly != ReadonlyOp::Any {
        write!(w, " {} ", FmtReadonly(readonly))?;
    }

    match *key {
        MemberKey::EC => {
            let vid = operands.next().unwrap();
            write!(w, "{}]", FmtVid(func, vid, verbose, strings))?;
        }
        MemberKey::EI(i) => write!(w, "{}]", i)?,
        MemberKey::EL => {
            let lid = locals.next().unwrap();
            write!(w, "{}]", FmtLid(lid, strings))?
        }
        MemberKey::ET(sid) => write!(w, "{}]", FmtQuotedStringId(sid, strings))?,
        MemberKey::PC => {
            let vid = operands.next().unwrap();
            write!(w, "{}", FmtVid(func, vid, verbose, strings))?;
        }
        MemberKey::PL => {
            let lid = locals.next().unwrap();
            write!(w, "{}", FmtLid(lid, strings))?
        }
        MemberKey::PT(pid) => write!(w, "{}", FmtQuotedStringId(pid.id, strings))?,
        MemberKey::QT(pid) => write!(w, "{}", FmtQuotedStringId(pid.id, strings))?,
        MemberKey::W => write!(w, "]")?,
    }
    Ok(())
}

fn print_method(
    w: &mut dyn Write,
    clsid: ClassId,
    method: &Method<'_>,
    verbose: bool,
    strings: &StringInterner,
) -> Result {
    print_top_level_loc(w, method.func.get_loc(method.func.loc_id), strings)?;
    writeln!(
        w,
        "method {clsid}::{method}{tparams}{params}{shadowed_tparams}: {ret_type} {attr} {vis} {{",
        clsid = FmtIdentifierId(clsid.id, strings),
        method = FmtIdentifierId(method.name.id, strings),
        tparams = FmtTParams(&method.func.tparams, strings),
        shadowed_tparams = FmtShadowedTParams(&method.func.shadowed_tparams, strings),
        params = FmtFuncParams(&method.func, strings),
        ret_type = FmtTypeInfo(&method.func.return_type, strings),
        vis = FmtVisibility(method.visibility),
        attr = FmtAttr(method.attrs, AttrContext::Method),
    )?;
    print_method_flags(w, method.flags)?;
    print_attributes(w, &method.attributes, strings)?;
    print_coeffects(w, &method.coeffects)?;
    print_func_body(w, &method.func, verbose, strings, None, None)?;
    writeln!(w, "}}")?;
    writeln!(w)
}

fn print_method_flags(w: &mut dyn Write, mut flags: MethodFlags) -> Result {
    [
        get_bit(&mut flags, MethodFlags::IS_ASYNC, ".async"),
        get_bit(&mut flags, MethodFlags::IS_GENERATOR, ".generator"),
        get_bit(
            &mut flags,
            MethodFlags::IS_PAIR_GENERATOR,
            ".pair_generator",
        ),
        get_bit(&mut flags, MethodFlags::IS_CLOSURE_BODY, ".closure_body"),
    ]
    .into_iter()
    .flatten()
    .try_for_each(|f| writeln!(w, "  {f}"))
}

fn incdec_what(op: IncDecOp) -> (&'static str, &'static str) {
    let what = match op {
        IncDecOp::PreInc | IncDecOp::PostInc => "++",
        IncDecOp::PreDec | IncDecOp::PostDec => "--",
        _ => panic!("bad IncDecOp value"),
    };

    match op {
        IncDecOp::PreInc | IncDecOp::PreDec => (what, ""),
        IncDecOp::PostInc | IncDecOp::PostDec => ("", what),
        _ => panic!("bad IncDecOp value"),
    }
}

pub(crate) fn print_param(
    w: &mut dyn Write,
    strings: &StringInterner,
    func: &Func<'_>,
    param: &Param<'_>,
) -> Result {
    let Param {
        is_inout,
        is_readonly,
        is_variadic,
        ref ty,
        name,
        default_value,
        ref user_attributes,
    } = *param;

    if is_inout {
        write!(w, "inout ")?;
    }
    if is_readonly {
        write!(w, "readonly ")?;
    }

    if !user_attributes.is_empty() {
        write!(
            w,
            "[{}] ",
            FmtSep::comma(user_attributes.iter(), |w, a| FmtAttribute(a, strings)
                .fmt(w))
        )?;
    }

    let ellipsis_for_variadic = if is_variadic { "..." } else { "" };
    write!(
        w,
        "{} {}{}",
        FmtTypeInfo(ty, strings),
        ellipsis_for_variadic,
        FmtIdentifierId(name, strings)
    )?;
    if let Some(dv) = default_value {
        write!(
            w,
            " @ {} ({})",
            FmtBid(func, dv.init, false),
            FmtQuotedStr(&dv.expr)
        )?;
    }
    Ok(())
}

fn print_property(w: &mut dyn Write, property: &Property<'_>, strings: &StringInterner) -> Result {
    write!(
        w,
        "  property {name} {flags}{attributes} {vis} : {ty} {doc}",
        name = FmtIdentifierId(property.name.id, strings),
        flags = FmtAttr(property.flags, AttrContext::Property),
        attributes = FmtSep::new(" <", ", ", ">", property.attributes.iter(), |w, attr| {
            write!(
                w,
                "{}({})",
                FmtIdentifierId(attr.name.id, strings),
                FmtSep::comma(attr.arguments.iter(), |w, arg| {
                    FmtTypedValue(arg, strings).fmt(w)
                })
            )
        }),
        vis = FmtVisibility(property.visibility),
        ty = FmtTypeInfo(&property.type_info, strings),
        doc = FmtDocComment(property.doc_comment.as_ref().into()),
    )?;

    if let Some(iv) = property.initial_value.as_ref() {
        write!(w, " = {}", FmtTypedValue(iv, strings))?;
    }

    writeln!(w)
}

fn print_symbol_refs(w: &mut dyn Write, refs: &SymbolRefs<'_>) -> Result {
    let SymbolRefs {
        classes,
        constants,
        functions,
        includes,
    } = refs;

    for v in classes {
        writeln!(w, ".class_ref {}", FmtIdentifier(v.as_bytes()))?;
    }

    for v in constants {
        writeln!(w, ".const_ref {}", FmtIdentifier(v.as_bytes()))?;
    }

    for v in functions {
        writeln!(w, ".func_ref {}", FmtIdentifier(v.as_bytes()))?;
    }

    for v in includes {
        write!(w, ".include_ref ")?;
        match v {
            IncludePath::Absolute(path) => write!(w, "{}", FmtQuotedStr(path))?,
            IncludePath::SearchPathRelative(path) => write!(w, "relative {}", FmtQuotedStr(path))?,
            IncludePath::IncludeRootRelative(root, path) => {
                write!(w, "rooted {} {}", FmtQuotedStr(root), FmtQuotedStr(path))?
            }
            IncludePath::DocRootRelative(path) => write!(w, "doc {}", FmtQuotedStr(path))?,
        }
        writeln!(w)?;
    }

    Ok(())
}

fn print_terminator(
    w: &mut dyn Write,
    ctx: &mut FuncContext<'_>,
    func: &Func<'_>,
    _iid: InstrId,
    terminator: &Terminator,
) -> Result {
    let verbose = ctx.verbose;
    let strings = ctx.strings;
    match terminator {
        Terminator::CallAsync(call, targets) => print_call_async(w, ctx, func, call, targets)?,
        Terminator::Enter(bid, _) => write!(w, "enter to {}", FmtBid(func, *bid, verbose),)?,
        Terminator::Exit(vid, _) => {
            write!(w, "exit {}", FmtVid(func, *vid, verbose, strings))?;
        }
        Terminator::Fatal(vid, op, _) => {
            let op = match *op {
                ir_core::FatalOp::Parse => "parse",
                ir_core::FatalOp::Runtime => "runtime",
                ir_core::FatalOp::RuntimeOmitFrame => "runtime_omit_frame",
                _ => panic!("bad FatalOp value"),
            };
            write!(w, "fatal {}, {}", op, FmtVid(func, *vid, verbose, strings))?
        }
        Terminator::IterInit(args, vid) => {
            write!(
                w,
                "iterator ^{} init from {} jmp to {} else {} with {}",
                args.iter_id.idx,
                FmtVid(func, *vid, verbose, strings),
                FmtBid(func, args.targets[0], verbose),
                FmtBid(func, args.targets[1], verbose),
                FmtOptKeyValue(args.key_lid(), args.value_lid(), strings)
            )?;
        }
        Terminator::IterNext(args) => {
            write!(
                w,
                "iterator ^{} next jmp to {} else {} with {}",
                args.iter_id.idx,
                FmtBid(func, args.targets[0], verbose),
                FmtBid(func, args.targets[1], verbose),
                FmtOptKeyValue(args.key_lid(), args.value_lid(), strings)
            )?;
        }
        Terminator::Jmp(bid, _) => write!(w, "jmp to {}", FmtBid(func, *bid, verbose),)?,
        Terminator::JmpArgs(bid, vids, _) => write!(
            w,
            "jmp to {} with ({})",
            FmtBid(func, *bid, verbose),
            FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose, strings)
                .fmt(w))
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
                FmtVid(func, *cond, verbose, strings),
                FmtBid(func, targets[0], verbose),
                FmtBid(func, targets[1], verbose),
            )?;
        }
        Terminator::MemoGet(get) => {
            write!(
                w,
                "memo_get {} to {} else {}",
                FmtLids(&get.locals, strings),
                FmtBid(func, get.value_edge(), verbose),
                FmtBid(func, get.no_value_edge(), verbose)
            )?;
        }
        Terminator::MemoGetEager(get) => {
            write!(
                w,
                "memo_get_eager {} to {} eager {} else {}",
                FmtLids(&get.locals, strings),
                FmtBid(func, get.suspended_edge(), verbose),
                FmtBid(func, get.eager_edge(), verbose),
                FmtBid(func, get.no_value_edge(), verbose)
            )?;
        }
        Terminator::NativeImpl(_) => {
            write!(w, "native_impl")?;
        }
        Terminator::Ret(vid, _) => {
            write!(w, "ret {}", FmtVid(func, *vid, verbose, strings))?;
        }
        Terminator::RetCSuspended(vid, _) => {
            write!(
                w,
                "ret_c_suspended {}",
                FmtVid(func, *vid, verbose, strings)
            )?;
        }
        Terminator::RetM(vids, _) => {
            write!(
                w,
                "ret [{}]",
                FmtSep::comma(vids.iter(), |w, vid| FmtVid(func, *vid, verbose, strings)
                    .fmt(w))
            )?;
        }
        Terminator::Switch {
            cond,
            bounded,
            base,
            targets,
            loc: _,
        } => {
            let bounded = match *bounded {
                SwitchKind::Bounded => "bounded",
                SwitchKind::Unbounded => "unbounded",
                _ => unreachable!(),
            };
            write!(
                w,
                "switch {} {} {base} [{}]",
                bounded,
                FmtVid(func, *cond, verbose, strings),
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
                FmtVid(func, *cond, verbose, strings),
                FmtSep::comma(cases.iter().zip(targets.iter()), |w, (case, target)| {
                    write!(
                        w,
                        "{} => {}",
                        FmtQuotedStringId(*case, strings),
                        FmtBid(func, *target, verbose),
                    )
                })
            )?;
        }
        Terminator::Throw(vid, _) => {
            write!(w, "throw {}", FmtVid(func, *vid, verbose, strings))?;
        }
        Terminator::ThrowAsTypeStructException(ops, _) => {
            write!(
                w,
                "throw_as_type_struct_exception {}, {}",
                FmtVid(func, ops[0], verbose, strings),
                FmtVid(func, ops[1], verbose, strings)
            )?;
        }
        Terminator::Unreachable => write!(w, "unreachable")?,
    }
    Ok(())
}

fn print_type_constant(
    w: &mut dyn Write,
    tc: &TypeConstant<'_>,
    strings: &StringInterner,
) -> Result {
    write!(w, "  type_constant ")?;
    if tc.is_abstract {
        write!(w, "abstract ")?;
    }
    write!(w, "{}", FmtIdentifier(&tc.name))?;
    if let Some(init) = &tc.initializer {
        write!(w, " = {}", FmtTypedValue(init, strings))?;
    }
    writeln!(w)
}

fn print_typedef(w: &mut dyn Write, typedef: &Typedef, strings: &StringInterner) -> Result {
    let Typedef {
        attributes,
        attrs,
        loc,
        name,
        type_info_union,
        type_structure,
        case_type,
    } = typedef;

    print_top_level_loc(w, Some(loc), strings)?;

    writeln!(
        w,
        "typedef {vis} {name}: {ty} = {attributes}{ts} {attrs}",
        vis = if *case_type { "case_type" } else { "alias" },
        name = FmtIdentifierId(name.id, strings),
        ty = FmtSep::comma(type_info_union.iter(), |w, ti| FmtTypeInfo(ti, strings)
            .fmt(w)),
        attributes = FmtSep::new("<", ",", "> ", attributes, |w, attribute| FmtAttribute(
            attribute, strings
        )
        .fmt(w)),
        ts = FmtTypedValue(type_structure, strings),
        attrs = FmtAttr(*attrs, AttrContext::Typedef)
    )
}

pub fn print_unit(w: &mut dyn Write, unit: &Unit<'_>, verbose: bool) -> Result {
    let strings = &unit.strings;

    for attr in &unit.file_attributes {
        writeln!(w, "attribute {}", FmtAttribute(attr, strings))?;
    }

    if !unit.modules.is_empty() {
        for module in unit.modules.iter() {
            let Module {
                attributes,
                name,
                src_loc,
                doc_comment,
            } = module;
            print_top_level_loc(w, Some(src_loc), strings)?;
            write!(
                w,
                "module {name} [{attributes}] ",
                name = FmtIdentifierId(name.id, strings),
                attributes =
                    FmtSep::comma(attributes.iter(), |w, a| FmtAttribute(a, strings).fmt(w))
            )?;
            if let Some(doc_comment) = doc_comment {
                write!(w, "{}", FmtEscapedString(doc_comment))?;
            } else {
                write!(w, "N")?;
            }

            writeln!(w)?;
        }
        writeln!(w)?;
    }

    if let Some(module_use) = unit.module_use.as_ref() {
        writeln!(w, "module_use {}\n", FmtEscapedString(module_use))?;
    }

    for c in &unit.constants {
        print_hack_constant(w, c, strings)?;
    }

    if !unit.typedefs.is_empty() {
        for typedef in &unit.typedefs {
            print_typedef(w, typedef, strings)?;
        }
        writeln!(w)?;
    }

    for c in &unit.classes {
        print_class(w, c, strings)?;
    }

    for f in &unit.functions {
        print_function(w, f, verbose, strings)?;
    }

    for c in &unit.classes {
        for m in &c.methods {
            print_method(w, c.name, m, verbose, strings)?;
        }
    }

    print_fatal(w, unit.fatal.as_ref(), strings)?;

    print_symbol_refs(w, &unit.symbol_refs)?;

    Ok(())
}
