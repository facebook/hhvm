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
    trace!("{}", ir::print::DisplayFunc::new(&func, true));
    trace!("--------------------");

    // Start by inserting stack pushes and pops (which are normally not in the
    // IR) into the IR.
    trace!("-- compute spills");
    func = pusher::run(func);

    trace!(
        "-- after pushes:\n{}",
        ir::print::DisplayFunc::new(&func, true)
    );

    // Now emit the instructions.
    trace!("-- emit instrs");
    let mut labeler = emitter::Labeler::new(&func);
    let (instrs, decl_vars) = emitter::emit_func(&func, &mut labeler, adata);

    let params = Vec::from_iter(func.repr.params.into_iter().map(|(param, dv)| {
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

    let instrs = instrs.to_vec();
    let stack_depth = stack_depth::compute_stack_depth(&params, &instrs).unwrap();

    hhbc::Body {
        attributes: func.attributes,
        attrs: func.attrs,
        coeffects: func.coeffects,
        doc_comment: func.doc_comment,
        is_memoize_wrapper: func.is_memoize_wrapper,
        is_memoize_wrapper_lsb: func.is_memoize_wrapper_lsb,
        num_iters: func.num_iters,
        return_type: func.return_type,
        shadowed_tparams: func.shadowed_tparams,
        upper_bounds: func.upper_bounds,
        span: func.span,
        repr: hhbc::BcRepr {
            instrs: instrs.into(),
            decl_vars: decl_vars.into(),
            params: params.into(),
            stack_depth,
        },
    }
}

pub(crate) fn convert_function(function: ir::Function, adata: &mut AdataState) -> hhbc::Function {
    trace!("convert_function {}", function.name);
    hhbc::Function {
        flags: function.flags,
        name: function.name,
        body: convert_func(function.body, adata),
    }
}

pub(crate) fn convert_method(method: ir::Method, adata: &mut AdataState) -> Method {
    trace!("convert_method {}", method.name);
    hhbc::Method {
        name: method.name,
        flags: method.flags,
        visibility: method.visibility,
        body: convert_func(method.body, adata),
    }
}
