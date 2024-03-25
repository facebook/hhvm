// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::AdataState;
use hhbc::Method;
use hhbc::ParamEntry;
use log::trace;

use crate::emitter;
use crate::pusher;

/// Convert an ir::Func to an hhbc::Body.
///
/// The conversion has three general phases: push, optimize, and emit.
///
/// The push phase runs through the IR graph and inserts artificial pushes and
/// pops - before each instruction it pushes that instruction's operands onto
/// the stack and after each instruction pops that instruction's returns off the
/// stack into unnamed locals (or drops them for dead values).
///
/// The optimize phase (currently unimplemented) attempts to optimize the
/// instructions by moving common pushes into common locations and eliding
/// unnecessary use of locals.
///
/// The emit phase then converts the IR by emitting the HHBC sequence itself.
///
/// BlockArgs (phi-nodes) are handled like a split instruction - by pushing args
/// onto the stack before the jump and popping them off the stack in receiving
/// block.
pub(crate) fn convert_func(mut func: ir::Func, adata: &mut AdataState) -> hhbc::Body {
    // Compute liveness and implicit block parameters.

    trace!("-------------------- IR");
    trace!("{}", ir::print::DisplayFunc::new(&func, true,));
    trace!("--------------------");

    // Start by inserting stack pushes and pops (which are normally not in the
    // IR) into the IR.
    trace!("-- compute spills");
    func = pusher::run(func);

    trace!(
        "-- after pushes:\n{}",
        ir::print::DisplayFunc::new(&func, true,)
    );

    // Now emit the instructions.
    trace!("-- emit instrs");
    let mut labeler = emitter::Labeler::new(&func);
    let (body_instrs, decl_vars) = emitter::emit_func(&func, &mut labeler, adata);

    let return_type_info = func.return_type.into();

    let span = func.span;
    let params = Vec::from_iter(func.params.into_iter().map(|(param, dv)| {
        ParamEntry {
            param,
            dv: dv
                .map(|dv| hhbc::DefaultValue {
                    label: labeler.lookup_bid(dv.init),
                    expr: dv.expr.into(),
                })
                .into(),
        }
    }));

    let doc_comment = func.doc_comment.map(|c| c.into()).into();
    let upper_bounds = func.upper_bounds.into();

    let shadowed_tparams = func.shadowed_tparams;
    let body_instrs = body_instrs.to_vec();
    let stack_depth = stack_depth::compute_stack_depth(&params, &body_instrs).unwrap();

    hhbc::Body {
        attributes: func.attributes.into(),
        attrs: func.attrs,
        body_instrs: body_instrs.into(),
        coeffects: func.coeffects,
        decl_vars: decl_vars.into(),
        doc_comment,
        is_memoize_wrapper: func.is_memoize_wrapper,
        is_memoize_wrapper_lsb: func.is_memoize_wrapper_lsb,
        num_iters: func.num_iters,
        params: params.into(),
        return_type_info,
        shadowed_tparams: shadowed_tparams.into(),
        upper_bounds,
        stack_depth,
        span,
    }
}

pub(crate) fn convert_function(function: ir::Function, adata: &mut AdataState) -> hhbc::Function {
    trace!("convert_function {}", function.name);
    hhbc::Function {
        flags: function.flags,
        name: function.name,
        body: convert_func(function.func, adata),
    }
}

pub(crate) fn convert_method(method: ir::Method, adata: &mut AdataState) -> Method {
    trace!("convert_method {}", method.name);
    hhbc::Method {
        name: method.name,
        flags: method.flags,
        visibility: method.visibility,
        body: convert_func(method.func, adata),
    }
}
