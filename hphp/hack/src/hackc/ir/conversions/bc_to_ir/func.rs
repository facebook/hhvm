// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::Body;
use hhbc::Function;
use hhbc::Method;
use hhbc::Param;
use ir::func::DefaultValue;
use ir::instr::Terminator;
use ir::CcReified;
use ir::CcThis;
use ir::ClassNameMap;
use ir::Instr;
use ir::LocalId;
use log::trace;
use newtype::IdVec;

use crate::context::Context;
use crate::convert::UnitState;
use crate::types;

/// Convert a hhbc::Function to an ir::Function
pub(crate) fn convert_function(unit: &mut ir::Unit, src: &Function, unit_state: &UnitState) {
    trace!("--- convert_function {}", src.name.as_str());

    let span = ir::SrcLoc::from_span(&src.span);
    let func = convert_body(
        &src.body,
        &src.attributes,
        src.attrs,
        &src.coeffects,
        span,
        unit_state,
    );
    ir::verify::verify_func(&func, &Default::default());

    let function = ir::Function {
        func,
        flags: src.flags,
        name: src.name,
    };

    unit.functions.push(function);
}

/// Convert a hhbc::Method to an ir::Method
pub(crate) fn convert_method<'a>(
    unit: &mut ir::Unit,
    clsidx: usize,
    src: &Method,
    unit_state: &UnitState,
) {
    trace!("--- convert_method {}", src.name.as_str());

    let span = ir::SrcLoc::from_span(&src.span);
    let func = convert_body(
        &src.body,
        &src.attributes,
        src.attrs,
        &src.coeffects,
        span,
        unit_state,
    );
    ir::verify::verify_func(&func, &Default::default());

    let method = ir::Method {
        flags: src.flags,
        func,
        name: src.name,
        visibility: src.visibility,
    };

    unit.classes.get_mut(clsidx).unwrap().methods.push(method);
}

/// Convert a hhbc::Body to an ir::Func
fn convert_body<'a>(
    body: &Body,
    attributes: &[hhbc::Attribute],
    attrs: ir::Attr,
    coeffects: &hhbc::Coeffects,
    src_loc: ir::SrcLoc,
    unit_state: &UnitState,
) -> ir::Func {
    let Body {
        ref body_instrs,
        ref decl_vars,
        ref doc_comment,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        num_iters,
        ref params,
        ref return_type_info,
        ref shadowed_tparams,
        ref upper_bounds,
        stack_depth: _,
    } = *body;

    let tparams: ClassNameMap<_> = upper_bounds
        .iter()
        .map(|hhbc::UpperBound { name, bounds }| {
            let bounds = bounds.iter().map(types::convert_type).collect();
            (ir::ClassName::new(*name), ir::TParamBounds { bounds })
        })
        .collect();

    let shadowed_tparams: Vec<ir::ClassName> = shadowed_tparams
        .iter()
        .map(|s| ir::ClassName::new(*s))
        .collect();

    let mut locs: IdVec<ir::LocId, ir::SrcLoc> = Default::default();
    locs.push(src_loc);

    let coeffects = convert_coeffects(coeffects);

    let func = ir::Func {
        attributes: attributes.to_vec(),
        attrs,
        blocks: Default::default(),
        coeffects,
        doc_comment: doc_comment.clone().map(|c| c.clone().into()).into(),
        ex_frames: Default::default(),
        instrs: Default::default(),
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        imms: Default::default(),
        locs,
        num_iters,
        params: Default::default(),
        return_type: types::convert_maybe_type(return_type_info.as_ref()),
        shadowed_tparams,
        loc_id: ir::LocId::from_usize(0),
        tparams,
    };

    let mut ctx = Context::new(func, body_instrs, unit_state);

    for param in params.as_ref() {
        let ir_param = convert_param(&mut ctx, param);
        ctx.builder.func.params.push(ir_param);
        if let ffi::Just(dv) = param.default_value.as_ref() {
            // This default value will jump to a different start than the
            // Func::ENTRY_BID.
            let addr = ctx.label_to_addr[&dv.label];
            ctx.add_work_addr(addr);
        }
    }

    for &decl in decl_vars {
        ctx.named_local_lookup.push(LocalId::Named(decl));
    }

    // Go through the work queue and convert each sequence.  We convert a
    // sequence at a time to ensure that we don't attempt to cross a
    // try/catch boundary without changing ir::Blocks.
    while let Some(next) = ctx.work_queue_pending.pop_front() {
        crate::instrs::convert_sequence(&mut ctx, next);
    }

    let cur_bid = ctx.builder.cur_bid();
    if !ctx.builder.func.is_terminated(cur_bid) {
        // This is not a valid input - but might as well do something
        // reasonable.
        ctx.emit(Instr::Terminator(Terminator::Unreachable));
    }

    // Mark any empty blocks with 'unreachable'. These will be cleaned up
    // later but for now need to be valid IR (so they must end with a
    // terminator).
    for bid in ctx.builder.func.block_ids() {
        let block = ctx.builder.func.block_mut(bid);
        if block.is_empty() {
            ctx.builder.func.alloc_instr_in(bid, Instr::unreachable());
        }
    }

    let mut func = ctx.builder.finish();

    ir::passes::rpo_sort(&mut func);
    ir::passes::split_critical_edges(&mut func, true);
    ir::passes::ssa::run(&mut func);
    ir::passes::control::run(&mut func);
    ir::passes::clean::run(&mut func);

    trace!("FUNC:\n{}", ir::print::DisplayFunc::new(&func, true,));

    ir::verify::verify_func(&func, &Default::default());

    func
}

fn convert_param(ctx: &mut Context<'_>, param: &Param) -> ir::Param {
    let default_value = match &param.default_value {
        Maybe::Just(dv) => {
            let init = ctx.target_from_label(dv.label, 0);
            Some(DefaultValue {
                init,
                expr: dv.expr.to_vec(),
            })
        }
        Maybe::Nothing => None,
    };

    let name = param.name;
    ctx.named_local_lookup.push(LocalId::Named(name));
    let user_attributes = param.user_attributes.clone().into();
    ir::Param {
        default_value,
        name,
        is_variadic: param.is_variadic,
        is_inout: param.is_inout,
        is_readonly: param.is_readonly,
        ty: types::convert_maybe_type(param.type_info.as_ref()),
        user_attributes,
    }
}

fn convert_coeffects(coeffects: &hhbc::Coeffects) -> ir::Coeffects {
    ir::Coeffects {
        static_coeffects: coeffects.static_coeffects.clone(),
        unenforced_static_coeffects: coeffects.unenforced_static_coeffects.clone(),
        fun_param: coeffects.fun_param.clone(),
        cc_param: coeffects.cc_param.clone(),
        cc_this: Vec::from_iter(coeffects.get_cc_this().iter().map(|inner| CcThis {
            types: Vec::from_iter(inner.types.iter().copied()).into(),
        }))
        .into(),
        cc_reified: Vec::from_iter(coeffects.get_cc_reified().iter().map(|inner| CcReified {
            is_class: inner.is_class,
            index: inner.index,
            types: Vec::from_iter(inner.types.iter().copied()).into(),
        }))
        .into(),
        closure_parent_scope: coeffects.is_closure_parent_scope(),
        generator_this: coeffects.generator_this(),
        caller: coeffects.caller(),
    }
}
