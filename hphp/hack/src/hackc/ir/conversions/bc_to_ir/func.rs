// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Body;
use hhbc::Function;
use hhbc::Method;
use hhbc::ParamEntry;
use ir::func::DefaultValue;
use ir::instr::Terminator;
use ir::Instr;
use ir::LocalId;
use log::trace;
use newtype::IdVec;

use crate::context::Context;

/// Convert a hhbc::Function to an ir::Function
pub(crate) fn convert_function(unit: &mut ir::Unit, src: &Function) {
    trace!("--- convert_function {}", src.name.as_str());

    let func = convert_body(&src.body);
    ir::verify::verify_func(&func, &Default::default());

    let function = ir::Function {
        func,
        flags: src.flags,
        name: src.name,
    };

    unit.functions.push(function);
}

/// Convert a hhbc::Method to an ir::Method
pub(crate) fn convert_method(unit: &mut ir::Unit, clsidx: usize, src: &Method) {
    trace!("--- convert_method {}", src.name.as_str());

    let func = convert_body(&src.body);
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
fn convert_body(body: &Body) -> ir::Func {
    let Body {
        ref attributes,
        attrs,
        ref instrs,
        ref coeffects,
        ref decl_vars,
        ref doc_comment,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        num_iters,
        ref params,
        ref return_type,
        ref shadowed_tparams,
        ref upper_bounds,
        stack_depth: _,
        span,
    } = *body;

    let mut locs: IdVec<ir::LocId, ir::SrcLoc> = Default::default();
    locs.push(ir::SrcLoc::from_span(&span));
    let func = ir::Func {
        attributes: attributes.to_vec(),
        attrs,
        blocks: Default::default(),
        coeffects: coeffects.clone(),
        doc_comment: doc_comment.clone().map(|c| c.clone().into()).into(),
        ex_frames: Default::default(),
        instrs: Default::default(),
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        imms: Default::default(),
        locs,
        num_iters,
        params: Default::default(),
        return_type: return_type.clone().into(),
        shadowed_tparams: shadowed_tparams.clone().into(),
        span,
        upper_bounds: upper_bounds.clone().into(),
    };

    let mut ctx = Context::new(func, instrs);

    for e in params.as_ref() {
        let ir_param = convert_param(&mut ctx, e);
        ctx.builder.func.params.push(ir_param);
        if let ffi::Just(dv) = e.dv.as_ref() {
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

fn convert_param(
    ctx: &mut Context<'_>,
    ParamEntry { param, dv }: &ParamEntry,
) -> (ir::Param, Option<ir::DefaultValue>) {
    let default_value = dv
        .as_ref()
        .map(|dv| DefaultValue {
            init: ctx.target_from_label(dv.label, 0),
            expr: dv.expr.to_vec(),
        })
        .into();

    ctx.named_local_lookup.push(LocalId::Named(param.name));
    (param.clone(), default_value)
}
