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
use ir::ClassIdMap;
use ir::Instr;
use ir::LocalId;
use log::trace;
use newtype::IdVec;

use crate::context::Context;
use crate::convert;
use crate::convert::UnitState;
use crate::types;

/// Convert a hhbc::Function to an ir::Function
pub(crate) fn convert_function<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    src: &Function<'a>,
    unit_state: &UnitState<'a>,
) {
    trace!("--- convert_function {}", src.name.unsafe_as_str());

    let span = ir::SrcLoc::from_span(filename, &src.span);
    let func = convert_body(unit, filename, &src.body, span, unit_state);
    ir::verify::verify_func(&func, &Default::default(), &unit.strings);

    let attributes = src
        .attributes
        .as_ref()
        .iter()
        .map(|a| convert::convert_attribute(a, &unit.strings))
        .collect();

    let function = ir::Function {
        attributes,
        attrs: src.attrs,
        coeffects: convert_coeffects(&src.coeffects),
        func,
        flags: src.flags,
        name: ir::FunctionId::from_hhbc(src.name, &unit.strings),
    };

    unit.functions.push(function);
}

/// Convert a hhbc::Method to an ir::Method
pub(crate) fn convert_method<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    clsidx: usize,
    src: &Method<'a>,
    unit_state: &UnitState<'a>,
) {
    trace!("--- convert_method {}", src.name.unsafe_as_str());

    let span = ir::SrcLoc::from_span(filename, &src.span);
    let func = convert_body(unit, filename, &src.body, span, unit_state);
    ir::verify::verify_func(&func, &Default::default(), &unit.strings);

    let attributes = src
        .attributes
        .as_ref()
        .iter()
        .map(|attr| crate::convert::convert_attribute(attr, &unit.strings))
        .collect();

    let method = ir::Method {
        attributes,
        attrs: src.attrs,
        coeffects: convert_coeffects(&src.coeffects),
        flags: src.flags,
        func,
        name: ir::MethodId::from_hhbc(src.name, &unit.strings),
        visibility: src.visibility,
    };

    unit.classes.get_mut(clsidx).unwrap().methods.push(method);
}

/// Convert a hhbc::Body to an ir::Func
fn convert_body<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    body: &Body<'a>,
    src_loc: ir::SrcLoc,
    unit_state: &UnitState<'a>,
) -> ir::Func<'a> {
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

    let tparams: ClassIdMap<_> = upper_bounds
        .iter()
        .map(|hhbc::UpperBound { name, bounds }| {
            let id = unit.strings.intern_bytes(name.as_ref());
            let name = ir::ClassId::new(id);
            let bounds = bounds
                .iter()
                .map(|ty| types::convert_type(ty, &unit.strings))
                .collect();
            (name, ir::TParamBounds { bounds })
        })
        .collect();

    let shadowed_tparams: Vec<ir::ClassId> = shadowed_tparams
        .iter()
        .map(|name| {
            let id = unit.strings.intern_bytes(name.as_ref());
            ir::ClassId::new(id)
        })
        .collect();

    let mut locs: IdVec<ir::LocId, ir::SrcLoc> = Default::default();
    locs.push(src_loc);

    let func = ir::Func {
        blocks: Default::default(),
        doc_comment: doc_comment.clone().into_option(),
        ex_frames: Default::default(),
        instrs: Default::default(),
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        constants: Default::default(),
        locs,
        num_iters,
        params: Default::default(),
        return_type: types::convert_maybe_type(return_type_info.as_ref(), &unit.strings),
        shadowed_tparams,
        loc_id: ir::LocId::from_usize(0),
        tparams,
    };

    let mut ctx = Context::new(unit, filename, func, body_instrs, unit_state);

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

    for decl in decl_vars.as_ref() {
        let id = ctx.strings.intern_bytes(decl.as_ref());
        ctx.named_local_lookup.push(LocalId::Named(id));
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
    ir::passes::ssa::run(&mut func, &unit.strings);
    ir::passes::control::run(&mut func);
    ir::passes::clean::run(&mut func);

    trace!(
        "FUNC:\n{}",
        ir::print::DisplayFunc::new(&func, true, &unit.strings)
    );

    ir::verify::verify_func(&func, &Default::default(), &unit.strings);

    func
}

fn convert_param<'a, 'b>(ctx: &mut Context<'a, 'b>, param: &Param<'a>) -> ir::Param<'a> {
    let default_value = match &param.default_value {
        Maybe::Just(dv) => {
            let init = ctx.target_from_label(dv.label, 0);
            Some(DefaultValue {
                init,
                expr: dv.expr,
            })
        }
        Maybe::Nothing => None,
    };

    let name = ctx.strings.intern_bytes(param.name.as_ref());
    ctx.named_local_lookup.push(LocalId::Named(name));

    let user_attributes = param
        .user_attributes
        .iter()
        .map(|a| convert::convert_attribute(a, ctx.strings))
        .collect();

    ir::Param {
        default_value,
        name,
        is_variadic: param.is_variadic,
        is_inout: param.is_inout,
        is_readonly: param.is_readonly,
        ty: types::convert_maybe_type(param.type_info.as_ref(), ctx.strings),
        user_attributes,
    }
}

fn convert_coeffects<'a>(coeffects: &hhbc::Coeffects<'a>) -> ir::Coeffects<'a> {
    ir::Coeffects {
        static_coeffects: coeffects.get_static_coeffects().to_vec(),
        unenforced_static_coeffects: coeffects.get_unenforced_static_coeffects().to_vec(),
        fun_param: coeffects.get_fun_param().to_vec(),
        cc_param: coeffects.get_cc_param().to_vec(),
        cc_this: coeffects
            .get_cc_this()
            .iter()
            .map(|inner| CcThis {
                types: inner.types.iter().copied().collect(),
            })
            .collect(),
        cc_reified: coeffects
            .get_cc_reified()
            .iter()
            .map(|inner| CcReified {
                is_class: inner.is_class,
                index: inner.index,
                types: inner.types.iter().copied().collect(),
            })
            .collect(),
        closure_parent_scope: coeffects.is_closure_parent_scope(),
        generator_this: coeffects.generator_this(),
        caller: coeffects.caller(),
    }
}
