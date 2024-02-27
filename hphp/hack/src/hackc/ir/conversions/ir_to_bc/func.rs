// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Method;
use log::trace;

use crate::adata::AdataCache;
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
pub(crate) fn convert_func<'a>(
    mut func: ir::Func<'a>,
    strings: &StringCache<'a>,
    adata: &mut AdataCache,
) -> hhbc::Body<'a> {
    // Compute liveness and implicit block parameters.

    trace!("-------------------- IR");
    trace!(
        "{}",
        ir::print::DisplayFunc::new(&func, true, &strings.interner)
    );
    trace!("--------------------");

    // Start by inserting stack pushes and pops (which are normally not in the
    // IR) into the IR.
    trace!("-- compute spills");
    func = pusher::run(func, strings);

    trace!(
        "-- after pushes:\n{}",
        ir::print::DisplayFunc::new(&func, true, &strings.interner)
    );

    // Now emit the instructions.
    trace!("-- emit instrs");
    let mut labeler = emitter::Labeler::new(&func);
    let (body_instrs, decl_vars) = emitter::emit_func(&func, &mut labeler, strings, adata);

    let return_type_info = crate::types::convert(&func.return_type, strings);

    let params = Vec::from_iter(func.params.into_iter().map(|param| {
        let name = strings.lookup_ffi_str(param.name);
        let user_attributes = convert::convert_attributes(param.user_attributes, strings);
        let default_value = param
            .default_value
            .map(|dv| {
                let label = labeler.lookup_bid(dv.init);
                hhbc::DefaultValue {
                    label,
                    expr: dv.expr.into(),
                }
            })
            .into();
        hhbc::Param {
            name,
            is_variadic: param.is_variadic,
            is_inout: param.is_inout,
            is_readonly: param.is_readonly,
            user_attributes: user_attributes.into(),
            type_info: crate::types::convert(&param.ty, strings),
            default_value,
        }
    }));

    let doc_comment = func.doc_comment.map(|c| c.into()).into();

    let upper_bounds = Vec::from_iter(func.tparams.iter().map(|(name, tparam)| {
        hhbc::UpperBound {
            name: strings.intern(name.id).expect("non-utf8 class name"),
            bounds: tparam
                .bounds
                .iter()
                .map(|ty| crate::types::convert(ty, strings).unwrap())
                .collect(),
        }
    }));

    let shadowed_tparams = Vec::from_iter(
        func.shadowed_tparams
            .iter()
            .map(|name| strings.lookup_class_name(*name).as_ffi_str()),
    );

    let body_instrs = body_instrs.to_vec();
    let stack_depth = stack_depth::compute_stack_depth(params.as_ref(), &body_instrs).unwrap();

    hhbc::Body {
        body_instrs: body_instrs.into(),
        decl_vars: decl_vars.into(),
        doc_comment,
        is_memoize_wrapper: func.is_memoize_wrapper,
        is_memoize_wrapper_lsb: func.is_memoize_wrapper_lsb,
        num_iters: func.num_iters,
        params: params.into(),
        return_type_info,
        shadowed_tparams: shadowed_tparams.into(),
        upper_bounds: upper_bounds.into(),
        stack_depth,
    }
}

pub(crate) fn convert_function<'a>(
    unit: &mut UnitBuilder<'a>,
    mut function: ir::Function<'a>,
    strings: &StringCache<'a>,
) {
    trace!(
        "convert_function {}",
        function.name.as_bstr(&strings.interner)
    );
    let span = function.func.loc(function.func.loc_id).to_span();
    let attributes =
        convert::convert_attributes(std::mem::take(&mut function.func.attributes), strings);
    let attrs = function.func.attrs;
    let coeffects = convert_coeffects(&function.func.coeffects);
    let body = convert_func(function.func, strings, &mut unit.adata_cache);
    let hhas_func = hhbc::Function {
        attributes: attributes.into(),
        body,
        coeffects,
        flags: function.flags,
        name: strings.lookup_function_name(function.name),
        span,
        attrs,
    };
    unit.functions.push(hhas_func);
}

pub(crate) fn convert_method<'a>(
    mut method: ir::Method<'a>,
    strings: &StringCache<'a>,
    adata: &mut AdataCache,
) -> Method<'a> {
    trace!("convert_method {}", method.name.as_bstr(&strings.interner));
    let span = method.func.loc(method.func.loc_id).to_span();
    let attrs = method.func.attrs;
    let coeffects = convert_coeffects(&method.func.coeffects);
    let attributes =
        convert::convert_attributes(std::mem::take(&mut method.func.attributes), strings);
    let body = convert_func(method.func, strings, adata);
    hhbc::Method {
        attributes: attributes.into(),
        name: strings.lookup_method_name(method.name),
        body,
        span,
        coeffects,
        flags: method.flags,
        visibility: method.visibility,
        attrs,
    }
}

fn convert_coeffects(coeffects: &ir::Coeffects) -> hhbc::Coeffects {
    let static_coeffects = coeffects.static_coeffects.clone();
    let unenforced_static_coeffects = coeffects.unenforced_static_coeffects.clone();
    let fun_param = coeffects.fun_param.clone();
    let cc_param = coeffects.cc_param.clone();
    let cc_this = Vec::from_iter(coeffects.cc_this.iter().map(|inner| hhbc::CcThis {
        types: inner.types.clone().into(),
    }));
    let cc_reified = Vec::from_iter(coeffects.cc_reified.iter().map(|inner| hhbc::CcReified {
        is_class: inner.is_class,
        index: inner.index,
        types: inner.types.clone().into(),
    }));
    let closure_parent_scope = coeffects.closure_parent_scope;
    let generator_this = coeffects.generator_this;
    let caller = coeffects.caller;

    hhbc::Coeffects::new(
        static_coeffects.into(),
        unenforced_static_coeffects.into(),
        fun_param.into(),
        cc_param.into(),
        cc_this,
        cc_reified,
        closure_parent_scope,
        generator_this,
        caller,
    )
}
