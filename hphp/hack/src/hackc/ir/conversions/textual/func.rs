// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;
use std::sync::Arc;

use anyhow::Error;
use ir::instr::HasLoc;
use ir::instr::HasLocals;
use ir::instr::Hhbc;
use ir::instr::Predicate;
use ir::instr::Special;
use ir::instr::Terminator;
use ir::instr::Textual;
use ir::BlockId;
use ir::ClassId;
use ir::Constant;
use ir::Func;
use ir::FunctionFlags;
use ir::FunctionId;
use ir::IncDecOp;
use ir::Instr;
use ir::InstrId;
use ir::LocId;
use ir::LocalId;
use ir::MethodFlags;
use ir::MethodId;
use ir::SpecialClsRef;
use ir::StringInterner;
use ir::ValueId;
use itertools::Itertools;
use log::trace;
use naming_special_names_rust::special_idents;

use crate::class;
use crate::class::IsStatic;
use crate::hack;
use crate::lower;
use crate::mangle::FunctionName;
use crate::mangle::GlobalName;
use crate::mangle::Intrinsic;
use crate::mangle::TypeName;
use crate::member_op;
use crate::state::UnitState;
use crate::textual;
use crate::textual::Expr;
use crate::textual::Sid;
use crate::textual::TextualFile;
use crate::typed_value;
use crate::types::convert_ty;
use crate::util;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Functions are defined as taking a param bundle.
///
/// f(params: HackParams): mixed;
pub(crate) fn write_function(
    txf: &mut TextualFile<'_>,
    state: &mut UnitState,
    function: ir::Function<'_>,
) -> Result {
    trace!("Convert Function {}", function.name.as_bstr(&state.strings));

    let func_info = FuncInfo::Function(FunctionInfo {
        name: function.name,
        attrs: function.attrs,
        flags: function.flags,
    });

    lower_and_write_func(txf, state, textual::Ty::VoidPtr, function.func, func_info)
}

pub(crate) fn compute_func_params<'a, 'b>(
    params: &Vec<ir::Param<'_>>,
    unit_state: &'a mut UnitState,
    this_ty: textual::Ty,
) -> Result<Vec<(textual::Param<'b>, LocalId)>> {
    let mut result = Vec::new();

    // Prepend the 'this' parameter.
    let this_param = textual::Param {
        name: special_idents::THIS.into(),
        attr: None,
        ty: this_ty.into(),
    };
    let this_lid = LocalId::Named(unit_state.strings.intern_str(special_idents::THIS));
    result.push((this_param, this_lid));

    for p in params {
        let name_bytes = unit_state.strings.lookup_bytes(p.name);
        let name = util::escaped_string(&name_bytes);

        let mut attr = Vec::new();
        if p.is_variadic {
            attr.push(textual::VARIADIC);
        }

        let param = textual::Param {
            name: name.to_string().into(),
            attr: Some(attr.into_boxed_slice()),
            ty: convert_ty(&p.ty.enforced, &unit_state.strings).into(),
        };
        let lid = LocalId::Named(p.name);
        result.push((param, lid));
    }

    Ok(result)
}

pub(crate) fn lower_and_write_func(
    txf: &mut TextualFile<'_>,
    unit_state: &mut UnitState,
    this_ty: textual::Ty,
    func: ir::Func<'_>,
    func_info: FuncInfo<'_>,
) -> Result {
    fn lower_and_write_func_(
        txf: &mut TextualFile<'_>,
        unit_state: &mut UnitState,
        this_ty: textual::Ty,
        func: ir::Func<'_>,
        mut func_info: FuncInfo<'_>,
    ) -> Result {
        let func = lower::lower_func(func, &mut func_info, Arc::clone(&unit_state.strings));
        ir::verify::verify_func(&func, &Default::default(), &unit_state.strings);

        write_func(txf, unit_state, this_ty, func, Arc::new(func_info))
    }

    let has_defaults = func.params.iter().any(|p| p.default_value.is_some());
    if has_defaults {
        // When a function has defaults we need to split it into multiple
        // functions which each take a different number of parameters,
        // initialize them and then forward on to the function that takes all
        // the parameters.
        let inits = split_default_func(&func, &func_info, &unit_state.strings);
        for init in inits {
            lower_and_write_func_(txf, unit_state, this_ty.clone(), init, func_info.clone())?;
        }
    }

    lower_and_write_func_(txf, unit_state, this_ty, func, func_info)
}

/// Given a Func that has default parameters make a version of the function with
/// those parameters stripped out (one version is returned with each successive
/// parameter removed).
fn split_default_func<'a>(
    orig_func: &Func<'a>,
    func_info: &FuncInfo<'_>,
    strings: &StringInterner,
) -> Vec<Func<'a>> {
    let mut result = Vec::new();
    let loc = orig_func.loc_id;

    let min_params = orig_func
        .params
        .iter()
        .take_while(|p| p.default_value.is_none())
        .count();
    let max_params = orig_func.params.len();
    for param_count in min_params..max_params {
        let mut func = orig_func.clone();
        let target_bid = func.params[param_count].default_value.unwrap().init;
        func.params.truncate(param_count);
        for i in min_params..param_count {
            func.params[i].default_value = None;
        }

        // replace the entrypoint with a jump to the initializer.
        let iid = func.alloc_instr(Instr::Terminator(ir::instr::Terminator::Jmp(
            target_bid, loc,
        )));
        func.block_mut(Func::ENTRY_BID).iids = vec![iid];

        // And turn the 'enter' calls into a tail call into the non-default
        // function.
        let exit_bid = {
            let mut block = ir::Block::default();
            let params = orig_func
                .params
                .iter()
                .map(|param| {
                    let instr = Instr::Hhbc(Hhbc::CGetL(LocalId::Named(param.name), loc));
                    let iid = func.alloc_instr(instr);
                    block.iids.push(iid);
                    ValueId::from(iid)
                })
                .collect_vec();
            let instr = match func_info {
                FuncInfo::Function(fi) => Instr::simple_call(fi.name, &params, loc),
                FuncInfo::Method(mi) => {
                    let this_str = strings.intern_str(special_idents::THIS);
                    let instr = Instr::Hhbc(Hhbc::CGetL(LocalId::Named(this_str), loc));
                    let receiver = func.alloc_instr(instr);
                    block.iids.push(receiver);
                    Instr::simple_method_call(mi.name, receiver.into(), &params, loc)
                }
            };
            let iid = func.alloc_instr(instr);
            block.iids.push(iid);
            let iid = func.alloc_instr(Instr::ret(iid.into(), loc));
            block.iids.push(iid);

            func.alloc_bid(block)
        };

        for instr in func.instrs.iter_mut() {
            match instr {
                Instr::Terminator(Terminator::Enter(bid, _)) => *bid = exit_bid,
                _ => {}
            }
        }

        result.push(func);
    }

    result
}

fn write_func(
    txf: &mut TextualFile<'_>,
    unit_state: &mut UnitState,
    this_ty: textual::Ty,
    mut func: ir::Func<'_>,
    func_info: Arc<FuncInfo<'_>>,
) -> Result {
    let strings = Arc::clone(&unit_state.strings);

    let params = std::mem::take(&mut func.params);
    let (tx_params, param_lids): (Vec<_>, Vec<_>) =
        compute_func_params(&params, unit_state, this_ty)?
            .into_iter()
            .unzip();

    let ret_ty = convert_ty(&func.return_type.enforced, &strings);

    let lids = func
        .body_instrs()
        .flat_map(HasLocals::locals)
        .cloned()
        .collect::<HashSet<_>>();
    // TODO(arr): figure out how to provide more precise types
    let local_ty = textual::Ty::VoidPtr;
    let locals = lids
        .into_iter()
        .filter(|lid| !param_lids.contains(lid))
        .sorted_by(|x, y| cmp_lid(&strings, x, y))
        .zip(std::iter::repeat(&local_ty))
        .collect::<Vec<_>>();

    let name = match *func_info {
        FuncInfo::Method(ref mi) => match mi.name {
            name if name.is_86cinit(&strings) => {
                FunctionName::Intrinsic(Intrinsic::ConstInit(mi.class.name))
            }
            name if name.is_86pinit(&strings) => {
                FunctionName::Intrinsic(Intrinsic::PropInit(mi.class.name))
            }
            name if name.is_86sinit(&strings) => {
                FunctionName::Intrinsic(Intrinsic::StaticInit(mi.class.name))
            }
            _ => FunctionName::method(mi.class.name, mi.is_static, mi.name),
        },
        FuncInfo::Function(ref fi) => FunctionName::Function(fi.name),
    };

    let span = func.loc(func.loc_id).clone();

    let attributes = textual::FuncAttributes {
        is_async: func_info.is_async(),
        is_curry: false,
        is_final: func_info.attrs().is_final(),
    };

    txf.define_function(
        &name,
        Some(&span),
        &attributes,
        &tx_params,
        &ret_ty,
        &locals,
        {
            let func_info = Arc::clone(&func_info);
            |fb| {
                let mut func = rewrite_jmp_ops(func);
                ir::passes::clean::run(&mut func);

                let mut state = FuncState::new(fb, Arc::clone(&strings), &func, func_info);

                for bid in func.block_ids() {
                    write_block(&mut state, bid)?;
                }

                Ok(())
            }
        },
    )?;

    // For a user static method also generate an instance stub which forwards to
    // the static method.

    match (&*func_info, name) {
        (
            FuncInfo::Method(
                mi @ MethodInfo {
                    is_static: IsStatic::Static,
                    ..
                },
            ),
            FunctionName::Method(..) | FunctionName::Unmangled(..),
        ) => {
            write_instance_stub(txf, unit_state, mi, &tx_params, &ret_ty, &span)?;
        }
        _ => {}
    }

    Ok(())
}

pub(crate) fn write_func_decl(
    txf: &mut TextualFile<'_>,
    unit_state: &mut UnitState,
    this_ty: textual::Ty,
    mut func: ir::Func<'_>,
    func_info: Arc<FuncInfo<'_>>,
) -> Result {
    let params = std::mem::take(&mut func.params);
    let param_tys = compute_func_params(&params, unit_state, this_ty)?
        .into_iter()
        .map(|(param, _)| textual::Ty::clone(&param.ty))
        .collect_vec();

    let ret_ty = convert_ty(&func.return_type.enforced, &unit_state.strings);

    let name = match *func_info {
        FuncInfo::Method(ref mi) => match mi.name {
            name if name.is_86cinit(&unit_state.strings) => {
                FunctionName::Intrinsic(Intrinsic::ConstInit(mi.class.name))
            }
            name if name.is_86pinit(&unit_state.strings) => {
                FunctionName::Intrinsic(Intrinsic::PropInit(mi.class.name))
            }
            name if name.is_86sinit(&unit_state.strings) => {
                FunctionName::Intrinsic(Intrinsic::StaticInit(mi.class.name))
            }
            _ => FunctionName::method(mi.class.name, mi.is_static, mi.name),
        },
        FuncInfo::Function(ref fi) => FunctionName::Function(fi.name),
    };

    let attributes = textual::FuncAttributes {
        is_async: func_info.is_async(),
        is_final: func_info.attrs().is_final(),
        is_curry: false,
    };

    txf.declare_function(&name, &attributes, &param_tys, &ret_ty)?;

    Ok(())
}

/// For each static method we also write a non-static version of the method so
/// that callers of 'self::foo()' don't have to know if foo is static or
/// non-static.
fn write_instance_stub(
    txf: &mut TextualFile<'_>,
    unit_state: &mut UnitState,
    method_info: &MethodInfo<'_>,
    tx_params: &[textual::Param<'_>],
    ret_ty: &textual::Ty,
    span: &ir::SrcLoc,
) -> Result {
    let strings = &unit_state.strings;
    let name_str = FunctionName::method(
        method_info.class.name,
        IsStatic::NonStatic,
        method_info.name,
    );
    let attributes = textual::FuncAttributes::default();

    let mut tx_params = tx_params.to_vec();
    let inst_ty = method_info.non_static_ty();
    tx_params[0].ty = (&inst_ty).into();

    let locals = Vec::default();
    txf.define_function(
        &name_str,
        Some(span),
        &attributes,
        &tx_params,
        ret_ty,
        &locals,
        |fb| {
            fb.comment("forward to the static method")?;
            let this_str = strings.intern_str(special_idents::THIS);
            let this_lid = LocalId::Named(this_str);
            let this = fb.load(&inst_ty, textual::Expr::deref(this_lid))?;
            let static_this = hack::call_builtin(fb, hack::Builtin::GetStaticClass, [this])?;
            let target =
                FunctionName::method(method_info.class.name, IsStatic::Static, method_info.name);

            let params: Vec<Sid> = std::iter::once(Ok(static_this))
                .chain(tx_params.iter().skip(1).map(|param| {
                    let lid = LocalId::Named(strings.intern_str(param.name.as_ref()));
                    fb.load(&param.ty, textual::Expr::deref(lid))
                }))
                .try_collect()?;

            let call = fb.call(&target, params)?;
            fb.ret(call)?;
            Ok(())
        },
    )?;

    Ok(())
}

fn write_block(state: &mut FuncState<'_, '_, '_>, bid: BlockId) -> Result {
    trace!("  Block {bid}");
    let block = state.func.block(bid);

    let params = block
        .params
        .iter()
        .map(|iid| state.alloc_sid_for_iid(*iid))
        .collect_vec();
    // The entry BID is always included for us.
    if bid != Func::ENTRY_BID {
        state.fb.write_label(bid, &params)?;
    }

    // All the non-terminators.
    let n_iids = block.iids.len() - 1;
    for iid in &block.iids[..n_iids] {
        write_instr(state, *iid)?;
    }

    // The terminator.
    write_terminator(state, block.terminator_iid())?;

    // Exception handler.
    let handler = state.func.catch_target(bid);
    if handler != BlockId::NONE {
        state.fb.write_exception_handler(handler)?;
    }

    Ok(())
}

fn write_instr(state: &mut FuncState<'_, '_, '_>, iid: InstrId) -> Result {
    let instr = state.func.instr(iid);
    trace!("    Instr {iid}: {instr:?}");

    state.update_loc(instr.loc_id())?;

    // In general don't write directly to `w` here - isolate the formatting to
    // the `textual` crate.

    match *instr {
        Instr::Call(ref call) => write_call(state, iid, call)?,
        Instr::Hhbc(Hhbc::CreateCl {
            ref operands,
            clsid,
            loc: _,
        }) => {
            let ty = class::non_static_ty(clsid).deref();
            let cons = FunctionName::Intrinsic(Intrinsic::Construct(clsid));
            let obj = state.fb.write_expr_stmt(textual::Expr::Alloc(ty))?;
            let operands = operands
                .iter()
                .map(|vid| state.lookup_vid(*vid))
                .collect_vec();
            state.fb.call_static(&cons, obj.into(), operands)?;
            state.set_iid(iid, obj);
        }
        Instr::Hhbc(
            Hhbc::CGetL(lid, _)
            | Hhbc::CGetQuietL(lid, _)
            | Hhbc::CUGetL(lid, _)
            | Hhbc::ConsumeL(lid, _),
        ) => write_load_var(state, iid, lid)?,
        Instr::Hhbc(Hhbc::CGetS([field, class], _, _)) => {
            let class_id = lookup_constant_string(state.func, class).map(ClassId::new);
            let field_str = lookup_constant_string(state.func, field);
            let output = if let Some(field_str) = field_str {
                let field = util::escaped_string(&state.strings.lookup_bstr(field_str));
                let this = match class_id {
                    None => {
                        // "C"::foo
                        state.lookup_vid(class)
                    }
                    Some(cid) => {
                        // C::foo
                        // This isn't created by HackC but can be created by infer
                        // lowering.
                        state.load_static_class(cid)?.into()
                    }
                };
                state.call_builtin(hack::Builtin::FieldGet, (this, field))?
            } else {
                // Although the rest of these are technically valid they're
                // basically impossible to produce with HackC.
                panic!("Unhandled CGetS");
            };
            state.set_iid(iid, output);
        }
        Instr::Hhbc(Hhbc::SetS([field, class, value], _, _)) => {
            // Note that "easy" SetS are lowered before this point.
            let class_str = lookup_constant_string(state.func, class);
            let field_str = lookup_constant_string(state.func, field);
            let value = state.lookup_vid(value);
            match (class_str, field_str) {
                (None, Some(f)) => {
                    // $x::foo
                    let obj = state.lookup_vid(class);
                    let field = util::escaped_string(&state.strings.lookup_bstr(f));
                    state.store_mixed(
                        Expr::field(obj, textual::Ty::unknown(), field),
                        value.clone(),
                    )?;
                }
                // Although the rest of these are technically valid they're
                // basically impossible to produce with HackC.
                _ => {
                    panic!("Unhandled SetS");
                }
            }

            state.set_iid(iid, value);
        }
        Instr::Hhbc(Hhbc::IncDecL(lid, op, _)) => write_inc_dec_l(state, iid, lid, op)?,
        Instr::Hhbc(Hhbc::NewObjD(clsid, _)) => {
            // NewObjD allocates a default initialized object; constructor invocation is a
            // *separate* instruction. Thus we can translate it directly as textual Alloc
            // instruction.
            let ty = class::non_static_ty(clsid).deref();
            let obj = state.fb.write_expr_stmt(Expr::Alloc(ty))?;
            // HHVM calls 86pinit via NewObjD (and friends) when necessary. We
            // can't be sure so just call it explicitly.
            state.fb.call_static(
                &FunctionName::Intrinsic(Intrinsic::PropInit(clsid)),
                obj.into(),
                (),
            )?;
            state.set_iid(iid, obj);
        }
        Instr::Hhbc(Hhbc::InstanceOfD(target, cid, _)) => {
            let ty = class::non_static_ty(cid).deref();
            let target = Box::new(state.lookup_vid(target));
            // The result of __sil_instanceof is unboxed int, but we need a boxed HackBool
            let output = state.call_builtin(hack::Builtin::Bool, [Expr::InstanceOf(target, ty)])?;
            state.set_iid(iid, output);
        }
        Instr::Hhbc(Hhbc::ResolveClass(cid, _)) => {
            let vid = state.load_static_class(cid)?;
            state.set_iid(iid, vid);
        }
        Instr::Hhbc(Hhbc::ResolveClsMethodD(cid, method, _)) => {
            let that = state.load_static_class(cid)?;
            let name = FunctionName::Method(TypeName::StaticClass(cid), method);
            let expr = state.fb.write_alloc_curry(name, Some(that.into()), ())?;
            state.set_iid(iid, expr);
        }
        Instr::Hhbc(Hhbc::ResolveFunc(func_id, _)) => {
            let name = FunctionName::Function(func_id);
            let expr = state.fb.write_alloc_curry(name, None, ())?;
            state.set_iid(iid, expr);
        }
        Instr::Hhbc(Hhbc::ResolveMethCaller(func_id, _)) => {
            // ResolveMethCaller is weird in that HackC builds a function which
            // calls the method.
            let name = FunctionName::Function(func_id);
            let that = textual::Expr::null();
            let expr = state.fb.write_alloc_curry(name, None, [that])?;
            state.set_iid(iid, expr);
        }
        Instr::Hhbc(Hhbc::SelfCls(_)) => {
            let method_info = state
                .func_info
                .expect_method("SelfCls used in non-method context");
            let cid = method_info.class.name;
            let vid = state.load_static_class(cid)?;
            state.set_iid(iid, vid);
        }
        Instr::Hhbc(Hhbc::SetL(vid, lid, _)) => {
            write_set_var(state, lid, vid)?;
            // SetL emits the input as the output.
            state.copy_iid(iid, vid);
        }
        Instr::Hhbc(Hhbc::This(_)) => write_load_this(state, iid)?,
        Instr::Hhbc(Hhbc::UnsetL(lid, _)) => {
            state.store_mixed(
                textual::Expr::deref(lid),
                textual::Expr::Const(textual::Const::Null),
            )?;
        }
        Instr::MemberOp(ref mop) => member_op::write(state, iid, mop)?,
        Instr::Special(Special::Textual(Textual::AssertFalse(vid, _))) => {
            // I think "prune_not" means "stop if this expression IS true"...
            let pred = hack::expr_builtin(hack::Builtin::IsTrue, [state.lookup_vid(vid)]);
            state.fb.prune_not(pred)?;
        }
        Instr::Special(Special::Textual(Textual::AssertTrue(vid, _))) => {
            // I think "prune" means "stop if this expression IS NOT true"...
            let pred = hack::expr_builtin(hack::Builtin::IsTrue, [state.lookup_vid(vid)]);
            state.fb.prune(pred)?;
        }
        Instr::Special(Special::Textual(Textual::Deref(..))) => {
            // Do nothing - the expectation is that this will be emitted as a
            // Expr inlined in the target instruction (from lookup_iid()).
        }
        Instr::Special(Special::Textual(Textual::HackBuiltin {
            ref target,
            ref values,
            loc: _,
        })) => write_builtin(state, iid, target, values)?,
        Instr::Special(Special::Textual(Textual::LoadGlobal { id, is_const })) => {
            let name = match is_const {
                false => GlobalName::Global(id),
                true => GlobalName::GlobalConst(id),
            };
            let var = textual::Var::global(name);
            let expr = state.load_mixed(textual::Expr::deref(var))?;
            state.set_iid(iid, expr);
        }
        Instr::Special(Special::Textual(Textual::String(s))) => {
            let expr = {
                let s = state.strings.lookup_bstr(s);
                let s = util::escaped_string(&s);
                let s = hack::expr_builtin(hack::Builtin::String, [s]);
                state.fb.write_expr_stmt(s)?
            };
            state.set_iid(iid, expr);
        }

        Instr::Special(Special::Copy(vid)) => {
            write_copy(state, iid, vid)?;
        }
        Instr::Special(Special::IrToBc(..)) => todo!(),
        Instr::Special(Special::Param) => todo!(),
        Instr::Special(Special::Select(vid, _idx)) => {
            textual_todo! {
                let vid = state.lookup_vid(vid);
                let expr = state.fb.write_expr_stmt(vid)?;
                state.set_iid(iid, expr);
            }
        }
        Instr::Special(Special::Tmp(..)) => todo!(),
        Instr::Special(Special::Tombstone) => todo!(),

        Instr::Hhbc(ref hhbc) => {
            // This should only handle instructions that can't be rewritten into
            // a simpler form (like control flow and generic calls). Everything
            // else should be handled in lower().
            textual_todo! {
                message = ("Non-lowered hhbc instr: {hhbc:?} (from {})",
                           ir::print::formatters::FmtFullLoc(state.func.loc(hhbc.loc_id()), &state.strings)),
                use ir::instr::HasOperands;
                let name = FunctionName::Unmangled(format!("TODO_hhbc_{}", hhbc));
                let operands = instr
                    .operands()
                    .iter()
                    .map(|vid| state.lookup_vid(*vid))
                    .collect_vec();
                let output = state.fb.call(&name,operands)?;
                state.set_iid(iid, output);
            }
        }

        Instr::Terminator(_) => unreachable!(),
    }

    Ok(())
}

fn write_copy(state: &mut FuncState<'_, '_, '_>, iid: InstrId, vid: ValueId) -> Result {
    use hack::Builtin;
    use textual::Const;
    use typed_value::typed_value_expr;

    match vid.full() {
        ir::FullInstrId::Constant(cid) => {
            let constant = state.func.constant(cid);
            let expr = match constant {
                Constant::Array(tv) => typed_value_expr(tv, &state.strings),
                Constant::Bool(false) => Expr::Const(Const::False),
                Constant::Bool(true) => Expr::Const(Const::True),
                Constant::Dir => todo!(),
                Constant::EnumClassLabel(..) => todo!(),
                Constant::File => todo!(),
                Constant::Float(f) => Expr::Const(Const::Float(*f)),
                Constant::FuncCred => todo!(),
                Constant::Int(i) => hack::expr_builtin(Builtin::Int, [Expr::Const(Const::Int(*i))]),
                Constant::LazyClass(_cid) => todo!(),
                Constant::Method => todo!(),
                Constant::Named(..) => todo!(),
                Constant::NewCol(..) => todo!(),
                Constant::Null => Expr::Const(Const::Null),
                Constant::String(s) => {
                    let s = util::escaped_string(&state.strings.lookup_bytes(*s));
                    hack::expr_builtin(Builtin::String, [Expr::Const(Const::String(s))])
                }
                Constant::Uninit => Expr::Const(Const::Null),
            };

            let expr = state.fb.write_expr_stmt(expr)?;
            state.set_iid(iid, expr);
        }
        ir::FullInstrId::Instr(instr) => state.copy_iid(iid, ValueId::from_instr(instr)),
        ir::FullInstrId::None => unreachable!(),
    }
    Ok(())
}

fn write_terminator(state: &mut FuncState<'_, '_, '_>, iid: InstrId) -> Result {
    let terminator = match state.func.instr(iid) {
        Instr::Terminator(terminator) => terminator,
        _ => unreachable!(),
    };
    trace!("    Instr {iid}: {terminator:?}");

    state.update_loc(terminator.loc_id())?;

    match *terminator {
        Terminator::Enter(bid, _) | Terminator::Jmp(bid, _) => {
            state.fb.jmp(&[bid], ())?;
        }
        Terminator::Exit(msg, _) => {
            let msg = state.lookup_vid(msg);
            state.call_builtin(hack::Builtin::Hhbc(hack::Hhbc::Exit), [msg])?;
            state.fb.unreachable()?;
        }
        Terminator::Fatal(msg, _, _) => {
            let msg = state.lookup_vid(msg);
            state.call_builtin(hack::Builtin::Hhbc(hack::Hhbc::Fatal), [msg])?;
            state.fb.unreachable()?;
        }
        Terminator::JmpArgs(bid, ref params, _) => {
            let params = params.iter().map(|v| state.lookup_vid(*v)).collect_vec();
            state.fb.jmp(&[bid], params)?;
        }
        Terminator::JmpOp {
            cond: _,
            pred: _,
            targets: [true_bid, false_bid],
            loc: _,
        } => {
            // We just need to emit the jmp - the rewrite_jmp_ops() pass should
            // have already inserted assert in place on the target bids.
            state.fb.jmp(&[true_bid, false_bid], ())?;
        }
        Terminator::MemoGet(..) | Terminator::MemoGetEager(..) => {
            // This should have been lowered.
            unreachable!();
        }
        Terminator::Ret(vid, _) => {
            let sid = state.lookup_vid(vid);
            state.fb.ret(sid)?;
        }
        Terminator::Unreachable => {
            state.fb.unreachable()?;
        }

        Terminator::CallAsync(..)
        | Terminator::IterInit(..)
        | Terminator::IterNext(..)
        | Terminator::NativeImpl(..)
        | Terminator::RetCSuspended(..)
        | Terminator::RetM(..)
        | Terminator::SSwitch { .. }
        | Terminator::Switch { .. }
        | Terminator::ThrowAsTypeStructException { .. } => {
            state.write_todo(&format!("{}", terminator))?;
            state.fb.unreachable()?;
        }

        Terminator::Throw(vid, _) => {
            textual_todo! {
                let expr = state.lookup_vid(vid);
                let target = FunctionName::Unmangled("TODO_throw".to_string());
                state.fb.call(&target, [expr])?;
                state.fb.unreachable()?;
            }
        }
    }

    Ok(())
}

fn write_builtin(
    state: &mut FuncState<'_, '_, '_>,
    iid: InstrId,
    target: &str,
    values: &[ValueId],
) -> Result {
    let params = values
        .iter()
        .map(|vid| state.lookup_vid(*vid))
        .collect_vec();
    let target = FunctionName::Unmangled(target.to_string());
    let output = state.fb.call(&target, params)?;
    state.set_iid(iid, output);
    Ok(())
}

fn write_load_this(state: &mut FuncState<'_, '_, '_>, iid: InstrId) -> Result {
    let sid = state.load_this()?;
    state.set_iid(iid, sid);
    Ok(())
}

fn write_load_var(state: &mut FuncState<'_, '_, '_>, iid: InstrId, lid: LocalId) -> Result {
    let sid = state.load_mixed(textual::Expr::deref(lid))?;
    state.set_iid(iid, sid);
    Ok(())
}

fn write_set_var(state: &mut FuncState<'_, '_, '_>, lid: LocalId, vid: ValueId) -> Result {
    let value = state.lookup_vid(vid);
    state.store_mixed(textual::Expr::deref(lid), value)
}

fn write_call(state: &mut FuncState<'_, '_, '_>, iid: InstrId, call: &ir::Call) -> Result {
    use ir::instr::CallDetail;
    use ir::FCallArgsFlags;

    let ir::Call {
        ref operands,
        context,
        ref detail,
        flags,
        num_rets,
        ref inouts,
        ref readonly,
        loc: _,
    } = *call;

    let in_trait = state.func_info.declared_in_trait();

    if !inouts.as_ref().map_or(true, |inouts| inouts.is_empty()) {
        textual_todo! {
            state.fb.comment("TODO: inouts")?;
        }
    }

    if let Some(readonly) = readonly.as_ref() {
        textual_todo! {
            let params = readonly.iter().map(|idx| idx.to_string()).join(", ");
            state.fb.comment(&format!("TODO: readonly call params: {params}"))?;
        }
    }

    if num_rets >= 2 {
        textual_todo! {
            state.fb.comment("TODO: num_rets >= 2")?;
        }
    }

    // flags &= FCallArgsFlags::LockWhileUnwinding - ignored
    let is_async = flags & FCallArgsFlags::HasAsyncEagerOffset != 0;

    let args = detail.args(operands);
    let mut args = args
        .iter()
        .copied()
        .map(|vid| state.lookup_vid(vid))
        .collect_vec();

    // A 'splat' is a call with an expanded array:
    //   foo(1, 2, ...$a)
    let mut splat = None;

    if flags & FCallArgsFlags::HasUnpack != 0 {
        // 'unpack' means that the last arg was a splat.
        splat = args.pop();
    }
    if flags & FCallArgsFlags::HasGenerics != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::HasGenerics")?;
        }
    }
    if flags & FCallArgsFlags::SkipRepack != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::SkipRepack")?;
        }
    }
    if flags & FCallArgsFlags::SkipCoeffectsCheck != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::SkipCoeffectsCheck")?;
        }
    }
    if flags & FCallArgsFlags::EnforceMutableReturn != 0 {
        // todo!();
    }
    if flags & FCallArgsFlags::EnforceReadonlyThis != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::EnforceReadonlyThis")?;
        }
    }
    if flags & FCallArgsFlags::ExplicitContext != 0 {
        if let Some(context) = state.strings.lookup_bytes_or_none(context) {
            // For analysis context shouldn't really matter. For now with a
            // calling context just report it as a comment.
            let context = util::escaped_string(&context);
            state.fb.comment(&format!("ExplicitContext: {context}"))?;
        }
    }
    if flags & FCallArgsFlags::HasInOut != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::HasInOut")?;
        }
    }
    if flags & FCallArgsFlags::EnforceInOut != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::EnforceInOut")?;
        }
    }
    if flags & FCallArgsFlags::EnforceReadonly != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::EnforceReadonly")?;
        }
    }
    if flags & FCallArgsFlags::NumArgsStart != 0 {
        textual_todo! {
            state.fb.comment("TODO: FCallArgsFlags::NumArgsStart")?;
        }
    }

    if let Some(splat) = splat {
        // For a splat we'll pass the splat through a function that the model
        // can recognize so it understands that it needs some extra analysis.
        let splat = hack::call_builtin(state.fb, hack::Builtin::SilSplat, [splat])?;
        args.push(splat.into());
    }

    let mut output = match *detail {
        CallDetail::FCallClsMethod { .. } => write_todo(state.fb, "FCallClsMethod")?,
        CallDetail::FCallClsMethodD { clsid, method } => {
            // C::foo()
            let target = FunctionName::method(clsid, IsStatic::Static, method);
            let this = state.load_static_class(clsid)?;
            state.fb.call_static(&target, this.into(), args)?
        }
        CallDetail::FCallClsMethodM { method, log: _ } => {
            // $c::foo()

            // TODO: figure out better typing. If this is coming from a
            // `classname` parameter we at least have a lower-bound.
            let target = FunctionName::untyped_method(method);
            let obj = state.lookup_vid(detail.class(operands));
            state.fb.call_virtual(&target, obj, args)?
        }
        CallDetail::FCallClsMethodS { .. } => state.write_todo("TODO_FCallClsMethodS")?,
        CallDetail::FCallClsMethodSD { clsref, method } => match clsref {
            SpecialClsRef::SelfCls => {
                // self::foo() - Static call to the method in the current class.
                let mi = state.expect_method_info();
                let is_static = mi.is_static;
                let target = if in_trait {
                    let base = ClassId::from_str("__self__", &state.strings);
                    FunctionName::method(base, is_static, method)
                } else {
                    FunctionName::method(mi.class.name, is_static, method)
                };
                let this = state.load_this()?;
                state.fb.call_static(&target, this.into(), args)?
            }
            SpecialClsRef::LateBoundCls => {
                // static::foo() - Virtual call to the method in the current class.

                // Note: Use an untyped method call instead of typed to make
                // textual happier.
                //
                //   let mi = state.expect_method_info();
                //   let target = FunctionName::method(mi.class.name, mi.is_static, method);
                let target = FunctionName::untyped_method(method);

                let this = state.load_this()?;
                state.fb.call_virtual(&target, this.into(), args)?
            }
            SpecialClsRef::ParentCls => {
                // parent::foo() - Static call to the method in the parent class.
                let mi = state.expect_method_info();
                let is_static = mi.is_static;
                let target = if in_trait {
                    let base = ClassId::from_str("__parent__", &state.strings);
                    FunctionName::method(base, is_static, method)
                } else {
                    let base = if let Some(base) = mi.class.base {
                        base
                    } else {
                        // Uh oh. We're asking to call parent::foo() when we don't
                        // have a known parent. This can happen in a trait...
                        ClassId::from_str("__parent__", &state.strings)
                    };
                    FunctionName::method(base, is_static, method)
                };
                let this = state.load_this()?;
                state.fb.call_static(&target, this.into(), args)?
            }
            _ => unreachable!(),
        },
        CallDetail::FCallCtor => unreachable!(),
        CallDetail::FCallFunc => {
            // $foo()
            let target = detail.target(operands);
            let target = state.lookup_vid(target);
            let name = FunctionName::Intrinsic(Intrinsic::Invoke(TypeName::Unknown));
            state.fb.call_virtual(&name, target, args)?
        }
        CallDetail::FCallFuncD { func } => {
            // foo()
            let target = FunctionName::Function(func);
            // A top-level function is called like a class static in a special
            // top-level class. Its 'this' pointer is null.
            state.fb.call_static(&target, textual::Expr::null(), args)?
        }
        CallDetail::FCallObjMethod { .. } => state.write_todo("FCallObjMethod")?,
        CallDetail::FCallObjMethodD { flavor, method } => {
            // $x->y()

            // This should have been handled in lowering.
            assert!(flavor != ir::ObjMethodOp::NullSafe);

            // TODO: need to try to figure out the type.
            let target = FunctionName::untyped_method(method);
            let obj = state.lookup_vid(detail.obj(operands));
            state.fb.call_virtual(&target, obj, args)?
        }
    };

    if is_async {
        output = state.call_builtin(hack::Builtin::Hhbc(hack::Hhbc::Await), [output])?;
    }

    state.set_iid(iid, output);
    Ok(())
}

fn write_inc_dec_l(
    state: &mut FuncState<'_, '_, '_>,
    iid: InstrId,
    lid: LocalId,
    op: IncDecOp,
) -> Result {
    let builtin = match op {
        IncDecOp::PreInc => hack::Hhbc::Add,
        IncDecOp::PostInc => hack::Hhbc::Add,
        IncDecOp::PreDec => hack::Hhbc::Sub,
        IncDecOp::PostDec => hack::Hhbc::Sub,
        _ => unreachable!(),
    };

    let pre = state.load_mixed(textual::Expr::deref(lid))?;
    let one = state.call_builtin(hack::Builtin::Int, [1])?;
    let post = state.call_builtin(hack::Builtin::Hhbc(builtin), (pre, one))?;
    state.store_mixed(textual::Expr::deref(lid), post)?;

    let sid = match op {
        IncDecOp::PreInc | IncDecOp::PreDec => pre,
        IncDecOp::PostInc | IncDecOp::PostDec => post,
        _ => unreachable!(),
    };
    state.set_iid(iid, sid);

    Ok(())
}

pub(crate) struct FuncState<'a, 'b, 'c> {
    pub(crate) fb: &'a mut textual::FuncBuilder<'b, 'c>,
    pub(crate) func: &'a ir::Func<'a>,
    iid_mapping: ir::InstrIdMap<textual::Expr>,
    func_info: Arc<FuncInfo<'a>>,
    pub(crate) strings: Arc<StringInterner>,
}

impl<'a, 'b, 'c> FuncState<'a, 'b, 'c> {
    fn new(
        fb: &'a mut textual::FuncBuilder<'b, 'c>,
        strings: Arc<StringInterner>,
        func: &'a ir::Func<'a>,
        func_info: Arc<FuncInfo<'a>>,
    ) -> Self {
        Self {
            fb,
            func,
            iid_mapping: Default::default(),
            func_info,
            strings,
        }
    }

    pub fn alloc_sid_for_iid(&mut self, iid: InstrId) -> Sid {
        let sid = self.fb.alloc_sid();
        self.set_iid(iid, sid);
        sid
    }

    pub(crate) fn call_builtin(
        &mut self,
        target: hack::Builtin,
        params: impl textual::VarArgs,
    ) -> Result<Sid> {
        hack::call_builtin(self.fb, target, params)
    }

    pub(crate) fn copy_iid(&mut self, iid: InstrId, input: ValueId) {
        let expr = self.lookup_vid(input);
        self.set_iid(iid, expr);
    }

    fn expect_method_info(&self) -> &MethodInfo<'_> {
        self.func_info.expect_method("not in class context")
    }

    /// Loads the static singleton for a class.
    fn load_static_class(&mut self, cid: ClassId) -> Result<textual::Sid> {
        match *self.func_info {
            FuncInfo::Method(MethodInfo {
                class, is_static, ..
            }) if class.name == cid => {
                // If we're already in a member of the class then use '$this'.
                match is_static {
                    IsStatic::Static => self.load_this(),
                    IsStatic::NonStatic => {
                        let this = self.load_this()?;
                        hack::call_builtin(self.fb, hack::Builtin::GetStaticClass, [this])
                    }
                }
            }
            _ => {
                let cname = TypeName::Class(cid);
                let ty = textual::Ty::Type(cname);
                self.fb.lazy_class_initialize(&ty)
            }
        }
    }

    pub(crate) fn load_mixed(&mut self, src: impl Into<textual::Expr>) -> Result<Sid> {
        self.fb.load(&textual::Ty::mixed_ptr(), src)
    }

    fn load_this(&mut self) -> Result<textual::Sid> {
        let var = LocalId::Named(self.strings.intern_str(special_idents::THIS));
        let mi = self.expect_method_info();
        let ty = mi.class_ty();
        let this = self.fb.load(&ty, textual::Expr::deref(var))?;
        Ok(this)
    }

    pub(crate) fn lookup_iid(&self, iid: InstrId) -> textual::Expr {
        if let Some(expr) = self.iid_mapping.get(&iid) {
            return expr.clone();
        }

        // The iid wasn't found.  Maybe it's a "special" reference (like a
        // Deref()) - pessimistically look for that.
        match self.func.instr(iid) {
            Instr::Special(Special::Textual(Textual::Deref(lid))) => {
                return textual::Expr::deref(*lid);
            }
            _ => {}
        }

        panic!("failed to look up iid {iid}");
    }

    /// Look up a ValueId in the FuncState and return an Expr representing
    /// it. For InstrIds and complex ConstIds return an Expr containing the
    /// (already emitted) Sid. For simple ConstIds use an Expr representing the
    /// value directly.
    pub(crate) fn lookup_vid(&mut self, vid: ValueId) -> textual::Expr {
        match vid.full() {
            ir::FullInstrId::Instr(iid) => self.lookup_iid(iid),
            ir::FullInstrId::Constant(c) => {
                use hack::Builtin;
                use ir::CollectionType;
                let c = self.func.constant(c);
                match c {
                    Constant::Bool(false) => hack::expr_builtin(Builtin::Bool, [false]),
                    Constant::Bool(true) => hack::expr_builtin(Builtin::Bool, [true]),
                    Constant::Int(i) => hack::expr_builtin(Builtin::Int, [*i]),
                    Constant::LazyClass(cid) => {
                        let ty = TypeName::Class(*cid);
                        textual::Expr::Const(textual::Const::LazyClass(ty))
                    }
                    Constant::Null => textual::Expr::null(),
                    Constant::String(s) => {
                        let s = self.strings.lookup_bstr(*s);
                        let s = util::escaped_string(&s);
                        hack::expr_builtin(Builtin::String, [s])
                    }
                    Constant::EnumClassLabel(..) => textual_todo! { textual::Expr::null() },
                    Constant::Array(..) => textual_todo! { textual::Expr::null() },
                    Constant::Dir => textual_todo! { textual::Expr::null() },
                    Constant::Float(f) => hack::expr_builtin(Builtin::Float, [f.to_f64()]),
                    Constant::File => textual_todo! { textual::Expr::null() },
                    Constant::FuncCred => textual_todo! { textual::Expr::null() },
                    Constant::Method => textual_todo! { textual::Expr::null() },
                    Constant::NewCol(CollectionType::ImmMap) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColImmMap), ())
                    }
                    Constant::NewCol(CollectionType::ImmSet) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColImmSet), ())
                    }
                    Constant::NewCol(CollectionType::ImmVector) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColImmVector), ())
                    }
                    Constant::NewCol(CollectionType::Map) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColMap), ())
                    }
                    Constant::NewCol(CollectionType::Pair) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColPair), ())
                    }
                    Constant::NewCol(CollectionType::Set) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColSet), ())
                    }
                    Constant::NewCol(CollectionType::Vector) => {
                        hack::expr_builtin(Builtin::Hhbc(hack::Hhbc::NewColVector), ())
                    }
                    Constant::Uninit => textual::Expr::null(),
                    Constant::Named(..) | Constant::NewCol(_) => unreachable!(),
                }
            }
            ir::FullInstrId::None => unreachable!(),
        }
    }

    pub(crate) fn set_iid(&mut self, iid: InstrId, expr: impl Into<textual::Expr>) {
        let expr = expr.into();
        let old = self.iid_mapping.insert(iid, expr);
        assert!(old.is_none());
    }

    pub(crate) fn store_mixed(
        &mut self,
        dst: impl Into<textual::Expr>,
        src: impl Into<textual::Expr>,
    ) -> Result {
        self.fb.store(dst, src, &textual::Ty::mixed_ptr())
    }

    pub(crate) fn update_loc(&mut self, loc: LocId) -> Result {
        if loc != LocId::NONE {
            let new = &self.func.locs[loc];
            self.fb.write_loc(new)?;
        }
        Ok(())
    }

    pub(crate) fn write_todo(&mut self, msg: &str) -> Result<Sid> {
        textual_todo! {
            message = ("TODO: {msg}"),
            let target = FunctionName::Unmangled(format!("TODO_{msg}"));
            self.fb.call(&target, ())
        }
    }
}

/// Convert from a deterministic jump model to a non-deterministic model.
///
/// In Textual instead of "jump if" you say "jump to a, b" and then in 'a' and 'b'
/// you say "stop if my condition isn't met".
///
/// This inserts the needed 'assert_true' and 'assert_false' statements but
/// leaves the original JmpOp as a marker for where to jump to.
fn rewrite_jmp_ops<'a>(mut func: ir::Func<'a>) -> ir::Func<'a> {
    for bid in func.block_ids() {
        match *func.terminator(bid) {
            Terminator::JmpOp {
                cond,
                pred,
                targets: [mut true_bid, mut false_bid],
                loc,
            } => {
                // We need to rewrite this jump. Because we don't allow critical
                // edges we can just insert the 'assert' at the start of the
                // target block since we must be the only caller.
                trace!("    JmpOp at {bid} needs to be rewritten");
                match pred {
                    Predicate::Zero => {
                        std::mem::swap(&mut true_bid, &mut false_bid);
                    }
                    Predicate::NonZero => {}
                }

                let iid = func.alloc_instr(Instr::Special(Special::Textual(Textual::AssertTrue(
                    cond, loc,
                ))));
                func.block_mut(true_bid).iids.insert(0, iid);

                let iid = func.alloc_instr(Instr::Special(Special::Textual(Textual::AssertFalse(
                    cond, loc,
                ))));
                func.block_mut(false_bid).iids.insert(0, iid);
            }
            _ => {}
        }
    }

    func
}

#[derive(Clone)]
pub(crate) enum FuncInfo<'a> {
    Function(FunctionInfo),
    Method(MethodInfo<'a>),
}

impl<'a> FuncInfo<'a> {
    pub(crate) fn expect_method(&self, why: &str) -> &MethodInfo<'a> {
        match self {
            FuncInfo::Function(_) => panic!("{}", why),
            FuncInfo::Method(mi) => mi,
        }
    }

    pub(crate) fn name_id(&self) -> ir::UnitBytesId {
        match self {
            FuncInfo::Function(fi) => fi.name.id,
            FuncInfo::Method(mi) => mi.name.id,
        }
    }

    pub(crate) fn attrs(&self) -> &ir::Attr {
        match self {
            FuncInfo::Function(fi) => &fi.attrs,
            FuncInfo::Method(mi) => &mi.attrs,
        }
    }

    pub(crate) fn declared_in_trait(&self) -> bool {
        match self {
            FuncInfo::Function(_) => false,
            FuncInfo::Method(mi) => mi.declared_in_trait(),
        }
    }

    pub(crate) fn is_async(&self) -> bool {
        match self {
            FuncInfo::Function(fi) => fi.flags.contains(FunctionFlags::ASYNC),
            FuncInfo::Method(mi) => mi.flags.contains(MethodFlags::IS_ASYNC),
        }
    }
}

// Extra data associated with a (non-class) function that aren't stored on the
// Func.
#[derive(Clone)]
pub(crate) struct FunctionInfo {
    pub(crate) name: FunctionId,
    pub(crate) attrs: ir::Attr,
    pub(crate) flags: FunctionFlags,
}

// Extra data associated with class methods that aren't stored on the Func.
#[derive(Clone)]
pub(crate) struct MethodInfo<'a> {
    pub(crate) name: MethodId,
    pub(crate) attrs: ir::Attr,
    pub(crate) class: &'a ir::Class<'a>,
    pub(crate) is_static: IsStatic,
    pub(crate) flags: MethodFlags,
}

impl MethodInfo<'_> {
    pub(crate) fn non_static_ty(&self) -> textual::Ty {
        class::non_static_ty(self.class.name)
    }

    pub(crate) fn class_ty(&self) -> textual::Ty {
        class::class_ty(self.class.name, self.is_static)
    }

    pub(crate) fn declared_in_trait(&self) -> bool {
        self.class.is_trait()
    }
}

/// Compare locals such that named ones go first followed by unnamed ones.
/// Ordering for named locals is stable and is based on their source names.
/// Unnamed locals have only their id which may differ accross runs. In which
/// case the IR would be non-deterministic and hence unstable ordering would be
/// the least of our concerns.
fn cmp_lid(strings: &StringInterner, x: &LocalId, y: &LocalId) -> std::cmp::Ordering {
    match (x, y) {
        (LocalId::Named(x_bid), LocalId::Named(y_bid)) => {
            let x_name = strings.lookup_bytes(*x_bid);
            let y_name = strings.lookup_bytes(*y_bid);
            x_name.cmp(&y_name)
        }
        (LocalId::Named(_), LocalId::Unnamed(_)) => std::cmp::Ordering::Less,
        (LocalId::Unnamed(_), LocalId::Named(_)) => std::cmp::Ordering::Greater,
        (LocalId::Unnamed(x_id), LocalId::Unnamed(y_id)) => x_id.cmp(y_id),
    }
}

pub(crate) fn write_todo(fb: &mut textual::FuncBuilder<'_, '_>, msg: &str) -> Result<Sid> {
    trace!("TODO: {}", msg);
    textual_todo! {
        let target = FunctionName::Unmangled(format!("$todo.{msg}"));
        fb.call(&target, ())
    }
}

pub(crate) fn lookup_constant<'a, 'b>(
    func: &'a Func<'b>,
    mut vid: ValueId,
) -> Option<&'a ir::Constant<'b>> {
    use ir::FullInstrId;
    loop {
        match vid.full() {
            FullInstrId::Instr(iid) => {
                let instr = func.instr(iid);
                match instr {
                    // If the pointed-at instr is a copy then follow it and try
                    // again.
                    Instr::Special(Special::Copy(copy)) => {
                        vid = *copy;
                    }
                    // SetL's output is just its input, updating the local is a
                    // side-effect. Follow the input and try again.
                    Instr::Hhbc(Hhbc::SetL(input_vid, _, _)) => {
                        vid = *input_vid;
                    }
                    _ => return None,
                }
            }
            FullInstrId::Constant(cid) => {
                return Some(func.constant(cid));
            }
            FullInstrId::None => {
                return None;
            }
        }
    }
}

pub(crate) fn lookup_constant_string(func: &Func<'_>, vid: ValueId) -> Option<ir::UnitBytesId> {
    match lookup_constant(func, vid) {
        Some(Constant::String(id)) => Some(*id),
        _ => None,
    }
}
