// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Pair;
use ffi::Str;
use ffi::Triple;
use hhbc::Body;
use hhbc::Function;
use hhbc::Method;
use hhbc::Param;
use ir::instr::Terminator;
use ir::ClassIdMap;
use ir::Instr;
use ir::LocalId;
use log::trace;

use crate::context::Context;
use crate::convert;
use crate::types;

/// Convert a hhbc::Function to an ir::Function
pub(crate) fn convert_function<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    src: &Function<'a>,
) {
    trace!("--- convert_function {}", src.name.unsafe_as_str());

    let func = convert_body(unit, filename, &src.body);
    ir::verify::verify_func(&func, &Default::default(), &unit.strings).unwrap();

    let attributes = src
        .attributes
        .as_ref()
        .iter()
        .map(convert::convert_attribute)
        .collect();

    let function = ir::Function {
        attributes,
        attrs: src.attrs,
        coeffects: convert_coeffects(&src.coeffects),
        func,
        flags: src.flags,
        name: src.name,
        span: ir::SrcLoc::from_span(filename, &src.span),
    };

    unit.functions.push(function);
}

/// Convert a hhbc::Method to an ir::Method
pub(crate) fn convert_method<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    clsidx: usize,
    src: &Method<'a>,
) {
    trace!("--- convert_method {}", src.name.unsafe_as_str());

    let func = convert_body(unit, filename, &src.body);
    ir::verify::verify_func(&func, &Default::default(), &unit.strings).unwrap();

    let attributes = src
        .attributes
        .as_ref()
        .iter()
        .map(|attr| ir::Attribute {
            name: attr.name,
            arguments: attr.arguments.as_ref().to_vec(),
        })
        .collect();

    let method = ir::Method {
        attributes,
        attrs: src.attrs,
        coeffects: convert_coeffects(&src.coeffects),
        flags: src.flags,
        func,
        name: src.name,
        span: ir::SrcLoc::from_span(filename, &src.span),
        visibility: src.visibility,
    };

    unit.classes.get_mut(clsidx).unwrap().methods.push(method);
}

/// Convert a hhbc::Body to an ir::Func
fn convert_body<'a>(
    unit: &mut ir::Unit<'a>,
    filename: ir::Filename,
    body: &Body<'a>,
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
    } = *body;

    let tparams: ClassIdMap<_> = upper_bounds
        .iter()
        .map(|Pair(name, bounds)| {
            let id = unit.strings.intern_bytes(name.as_ref());
            let name = ir::ClassId::new(id);
            let bounds = bounds
                .iter()
                .map(|ty| types::convert_type(ty, &mut unit.strings))
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

    let func = ir::Func {
        blocks: Default::default(),
        doc_comment: doc_comment.clone().into_option(),
        ex_frames: Default::default(),
        instrs: Default::default(),
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        constants: Default::default(),
        locs: Default::default(),
        num_iters,
        params: Default::default(),
        return_type: types::convert_maybe_type(return_type_info.as_ref(), &mut unit.strings),
        shadowed_tparams,
        tparams,
    };

    let mut ctx = Context::new(unit, filename, func, body_instrs);

    for param in params.as_ref() {
        let ir_param = convert_param(&mut ctx, param);
        ctx.builder.func.params.push(ir_param);
        if let ffi::Just(Pair(label, _)) = param.default_value.as_ref() {
            // This default value will jump to a different start than the
            // Func::ENTRY_BID.
            let addr = ctx.label_to_addr[label];
            ctx.add_work_addr(addr);
        }
    }

    for decl in decl_vars.as_ref() {
        let id = ctx.unit.strings.intern_bytes(decl.as_ref());
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
        ir::print::DisplayFunc(&func, true, &unit.strings)
    );

    ir::verify::verify_func(&func, &Default::default(), &unit.strings).unwrap();

    func
}

fn convert_param<'a, 'b>(ctx: &mut Context<'a, 'b>, param: &Param<'a>) -> ir::Param<'a> {
    let default_value = match param.default_value {
        Maybe::Just(Pair(label, value)) => {
            let bid = ctx.target_from_label(label, 0);
            Some((bid, value))
        }
        Maybe::Nothing => None,
    };

    let name = ctx.unit.strings.intern_bytes(param.name.as_ref());
    ctx.named_local_lookup.push(LocalId::Named(name));

    let user_attributes = param
        .user_attributes
        .iter()
        .map(convert::convert_attribute)
        .collect();

    ir::Param {
        default_value,
        name,
        is_variadic: param.is_variadic,
        is_inout: param.is_inout,
        is_readonly: param.is_readonly,
        ty: types::convert_maybe_type(param.type_info.as_ref(), &mut ctx.unit.strings),
        user_attributes,
    }
}

fn convert_coeffects<'a>(coeffects: &hhbc::Coeffects<'a>) -> ir::Coeffects<'a> {
    ir::Coeffects {
        static_coeffects: coeffects.get_static_coeffects().to_vec(),
        unenforced_static_coeffects: coeffects.get_unenforced_static_coeffects().to_vec(),
        fun_param: coeffects.get_fun_param().to_vec(),
        cc_param: coeffects
            .get_cc_param()
            .iter()
            .copied()
            .map(|Pair(a, b)| (a, b))
            .collect(),
        cc_this: coeffects
            .get_cc_this()
            .iter()
            .map(|inner| inner.iter().copied().collect::<Vec<Str<'a>>>())
            .collect(),
        cc_reified: coeffects
            .get_cc_reified()
            .iter()
            .copied()
            .map(|Triple(a, b, c)| {
                let c: Vec<Str<'a>> = c.iter().copied().collect();
                (a, b, c)
            })
            .collect(),
        closure_parent_scope: coeffects.is_closure_parent_scope(),
        generator_this: coeffects.generator_this(),
        caller: coeffects.caller(),
    }
}
