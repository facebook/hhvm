// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Pair;
use ffi::Slice;
use ffi::Triple;
use hhbc::Method;
use log::trace;

use crate::convert;
use crate::convert::UnitBuilder;
use crate::emitter;
use crate::pusher;
use crate::strings::StringCache;

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
///
pub(crate) fn convert_func<'a>(
    alloc: &'a bumpalo::Bump,
    mut func: ir::Func<'a>,
    strings: &StringCache<'a, '_>,
) -> hhbc::Body<'a> {
    // Compute liveness and implicit block parameters.

    trace!("-------------------- IR");
    trace!("{}", ir::print::DisplayFunc(&func, true, strings.interner));
    trace!("--------------------");

    // Start by inserting stack pushes and pops (which are normally not in the
    // IR) into the IR.
    trace!("-- compute spills");
    func = pusher::run(func, strings);

    trace!(
        "-- after pushes:\n{}",
        ir::print::DisplayFunc(&func, true, strings.interner)
    );

    // Now emit the instructions.
    trace!("-- emit instrs");
    let mut labeler = emitter::Labeler::new(&func);
    let (body_instrs, decl_vars) = emitter::emit_func(alloc, &func, &mut labeler, strings);

    let return_type_info = crate::types::convert(alloc, &func.return_type, strings);

    let decl_vars = Slice::fill_iter(alloc, decl_vars.into_iter());

    let params = Slice::fill_iter(
        alloc,
        func.params.into_iter().map(|param| {
            let name = strings.lookup_ffi_str(param.name);
            let user_attributes = convert::convert_attributes(alloc, param.user_attributes);
            let dv = param.default_value.map(|(bid, value)| {
                let label = labeler.lookup_bid(bid);
                ffi::Pair(label, value)
            });
            hhbc::Param {
                name,
                is_variadic: param.is_variadic,
                is_inout: param.is_inout,
                is_readonly: param.is_readonly,
                user_attributes,
                type_info: crate::types::convert(alloc, &param.ty, strings),
                default_value: dv.into(),
            }
        }),
    );

    let doc_comment = func.doc_comment.into();

    let upper_bounds = Slice::fill_iter(
        alloc,
        func.tparams.iter().map(|(name, tparam)| {
            let name = strings.lookup_class_name(*name);
            let type_info = Slice::fill_iter(
                alloc,
                tparam
                    .bounds
                    .iter()
                    .map(|ty| crate::types::convert(alloc, ty, strings).unwrap()),
            );
            Pair(name.as_ffi_str(), type_info)
        }),
    );

    let shadowed_tparams = Slice::fill_iter(
        alloc,
        func.shadowed_tparams
            .iter()
            .map(|name| strings.lookup_class_name(*name).as_ffi_str()),
    );

    hhbc::Body {
        body_instrs: body_instrs.compact(alloc),
        decl_vars,
        doc_comment,
        is_memoize_wrapper: func.is_memoize_wrapper,
        is_memoize_wrapper_lsb: func.is_memoize_wrapper_lsb,
        num_iters: func.num_iters,
        params,
        return_type_info,
        shadowed_tparams,
        upper_bounds,
    }
}

pub(crate) fn convert_function<'a>(
    alloc: &'a bumpalo::Bump,
    unit: &mut UnitBuilder<'a>,
    function: ir::Function<'a>,
    strings: &StringCache<'a, '_>,
) {
    let name = function.name;
    trace!("convert_function {}", name.as_bstr());
    let body = convert_func(alloc, function.func, strings);
    let attributes = convert::convert_attributes(alloc, function.attributes);
    let hhas_func = hhbc::Function {
        attributes,
        body,
        coeffects: convert_coeffects(alloc, &function.coeffects),
        flags: function.flags,
        name,
        span: function.span.to_span(),
        attrs: function.attrs,
    };
    unit.functions.push(hhas_func);
}

pub(crate) fn convert_method<'a>(
    alloc: &'a bumpalo::Bump,
    method: ir::Method<'a>,
    strings: &StringCache<'a, '_>,
) -> Method<'a> {
    trace!("convert_method {}", method.name.as_bstr());
    let body = convert_func(alloc, method.func, strings);
    let attributes = convert::convert_attributes(alloc, method.attributes);
    hhbc::Method {
        attributes,
        name: method.name,
        body,
        span: method.span.to_span(),
        coeffects: convert_coeffects(alloc, &method.coeffects),
        flags: method.flags,
        visibility: method.visibility,
        attrs: method.attrs,
    }
}

fn convert_coeffects<'a>(
    alloc: &'a bumpalo::Bump,
    coeffects: &ir::Coeffects<'a>,
) -> hhbc::Coeffects<'a> {
    let static_coeffects = Slice::fill_iter(alloc, coeffects.static_coeffects.iter().copied());
    let unenforced_static_coeffects =
        Slice::fill_iter(alloc, coeffects.unenforced_static_coeffects.iter().copied());
    let fun_param = Slice::fill_iter(alloc, coeffects.fun_param.iter().copied());
    let cc_param = Slice::fill_iter(
        alloc,
        coeffects.cc_param.iter().copied().map(|(a, b)| Pair(a, b)),
    );
    let cc_this = Slice::fill_iter(
        alloc,
        coeffects
            .cc_this
            .iter()
            .map(|inner| Slice::fill_iter(alloc, inner.iter().copied())),
    );
    let cc_reified = Slice::fill_iter(
        alloc,
        coeffects.cc_reified.iter().map(|(a, b, c)| {
            let c = Slice::fill_iter(alloc, c.iter().copied());
            Triple(*a, *b, c)
        }),
    );
    let closure_parent_scope = coeffects.closure_parent_scope;
    let generator_this = coeffects.generator_this;
    let caller = coeffects.caller;

    hhbc::Coeffects::new(
        static_coeffects,
        unenforced_static_coeffects,
        fun_param,
        cc_param,
        cc_this,
        cc_reified,
        closure_parent_scope,
        generator_this,
        caller,
    )
}
