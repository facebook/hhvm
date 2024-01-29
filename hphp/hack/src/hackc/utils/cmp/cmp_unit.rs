// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Str;
use hhbc::Attribute;
use hhbc::Body;
use hhbc::Class;
use hhbc::Constant;
use hhbc::Fatal;
use hhbc::Function;
use hhbc::Instruct;
use hhbc::Method;
use hhbc::Module;
use hhbc::Opcode;
use hhbc::Param;
use hhbc::Property;
use hhbc::Rule;
use hhbc::SymbolRefs;
use hhbc::TypeInfo;
use hhbc::TypedValue;
use hhbc::Typedef;
use hhbc::Unit;

use crate::cmp_eq;
use crate::cmp_map_t;
use crate::cmp_option;
use crate::cmp_set_t;
use crate::cmp_slice;
use crate::CmpContext;
use crate::CmpError;
use crate::MapName;
use crate::Result;

impl MapName for hhbc::Adata<'_> {
    fn get_name(&self) -> String {
        self.id.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Class<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Constant<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::CtxConstant<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Function<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Method<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Module<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Property<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Typedef<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::TypeConstant<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::Requirement<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

impl MapName for hhbc::UpperBound<'_> {
    fn get_name(&self) -> String {
        self.name.unsafe_as_str().to_string()
    }
}

/// Currently, some includes aren't printed out. So this is like the cmp_slice without a length check.
/// T126391106: BCP drops information
/// T126543346: Difficult to verify IncludeRootRelative
fn cmp_includes(a: &[hhbc::IncludePath<'_>], b: &[hhbc::IncludePath<'_>]) -> Result {
    for bv in b.iter() {
        if !a.iter().any(|av| cmp_include(av, bv).is_ok()) {
            bail!("{:?} has no matching includes", bv);
        }
    }
    Ok(())
}

fn cmp_attributes(a: &[Attribute<'_>], b: &[Attribute<'_>]) -> Result {
    cmp_set_t(a, b)
}

fn cmp_body(a: &Body<'_>, b: &Body<'_>) -> Result {
    let Body {
        body_instrs: a_body_instrs,
        decl_vars: a_decl_vars,
        num_iters: a_num_iters,
        is_memoize_wrapper: a_is_memoize_wrapper,
        is_memoize_wrapper_lsb: a_is_memoize_wrapper_lsb,
        upper_bounds: a_upper_bounds,
        shadowed_tparams: a_shadowed_tparams,
        params: a_params,
        return_type_info: a_return_type_info,
        doc_comment: a_doc_comment,
        stack_depth: a_stack_depth,
    } = a;
    let Body {
        body_instrs: b_body_instrs,
        decl_vars: b_decl_vars,
        num_iters: b_num_iters,
        is_memoize_wrapper: b_is_memoize_wrapper,
        is_memoize_wrapper_lsb: b_is_memoize_wrapper_lsb,
        upper_bounds: b_upper_bounds,
        shadowed_tparams: b_shadowed_tparams,
        params: b_params,
        return_type_info: b_return_type_info,
        doc_comment: b_doc_comment,
        stack_depth: b_stack_depth,
    } = b;

    cmp_eq(a_num_iters, b_num_iters).qualified("num_iters")?;
    cmp_slice(a_params, b_params, cmp_param).qualified("params")?;
    cmp_eq(a_is_memoize_wrapper, b_is_memoize_wrapper).qualified("is_memoize_wrapper")?;
    cmp_eq(a_is_memoize_wrapper_lsb, b_is_memoize_wrapper_lsb)
        .qualified("is_memoize_wrapper_lsb")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_eq(a_stack_depth, b_stack_depth).qualified("stack_depth")?;
    cmp_eq(a_return_type_info, b_return_type_info).qualified("return_type_info")?;
    cmp_eq(a_upper_bounds, b_upper_bounds).qualified("upper_bounds")?;
    cmp_eq(a_shadowed_tparams, b_shadowed_tparams).qualified("shadowed_tparams")?;

    cmp_set_t(a_decl_vars, b_decl_vars).qualified("decl_vars")?;

    if a_body_instrs.len() != b_body_instrs.len() {
        bail!(
            "Mismatch in instruct lengths: {} vs {}",
            a_body_instrs.len(),
            b_body_instrs.len()
        );
    }

    for (idx, (a_instr, b_instr)) in a_body_instrs.iter().zip(b_body_instrs.iter()).enumerate() {
        cmp_instr(a_instr, b_instr)
            .indexed(&idx.to_string())
            .qualified("body_instrs")?;
    }

    Ok(())
}

/// This is unique because only a few FCAFlags are printed -- those specified in
/// as-base-hhas.h.
/// T126391106 -- BCP drops information
fn cmp_fcallargflags(a: &hhbc::FCallArgsFlags, b: &hhbc::FCallArgsFlags) -> Result {
    use hhbc::FCallArgsFlags;
    let mut not_printed = FCallArgsFlags::SkipRepack;
    not_printed.add(FCallArgsFlags::SkipCoeffectsCheck);
    not_printed.add(FCallArgsFlags::ExplicitContext);
    not_printed.add(FCallArgsFlags::HasInOut);
    not_printed.add(FCallArgsFlags::EnforceInOut);
    not_printed.add(FCallArgsFlags::EnforceReadonly);
    not_printed.add(FCallArgsFlags::HasAsyncEagerOffset);
    not_printed.add(FCallArgsFlags::NumArgsStart);
    let mut a = a.clone();
    let mut b = b.clone();
    a.repr &= !(not_printed.repr);
    b.repr &= !(not_printed.repr);
    cmp_eq(&a, &b)?;
    Ok(())
}

fn cmp_fcallargs(a: &hhbc::FCallArgs<'_>, b: &hhbc::FCallArgs<'_>) -> Result {
    let hhbc::FCallArgs {
        flags: a_flags,
        async_eager_target: a_aet,
        num_args: a_num_args,
        num_rets: a_num_rets,
        inouts: a_inouts,
        readonly: a_readonly,
        context: a_context,
    } = a;
    let hhbc::FCallArgs {
        flags: b_flags,
        async_eager_target: b_aet,
        num_args: b_num_args,
        num_rets: b_num_rets,
        inouts: b_inouts,
        readonly: b_readonly,
        context: b_context,
    } = b;
    cmp_fcallargflags(a_flags, b_flags).qualified("fcallargflags")?;
    cmp_eq(a_aet, b_aet)?;
    cmp_eq(a_num_args, b_num_args)?;
    cmp_eq(a_num_rets, b_num_rets)?;
    cmp_eq(a_inouts, b_inouts)?;
    cmp_eq(a_readonly, b_readonly)?;
    cmp_eq(a_context, b_context)?;
    Ok(())
}

fn cmp_fcall_instr(a: &Opcode<'_>, b: &Opcode<'_>) -> Result {
    match (a, b) {
        (hhbc::Opcode::FCallClsMethod(fa, a1, a2), hhbc::Opcode::FCallClsMethod(fb, b1, b2)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
        }
        (hhbc::Opcode::FCallClsMethodD(fa, a1, a2), hhbc::Opcode::FCallClsMethodD(fb, b1, b2)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
        }
        (hhbc::Opcode::FCallClsMethodS(fa, a1, a2), hhbc::Opcode::FCallClsMethodS(fb, b1, b2)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
        }
        (hhbc::Opcode::FCallObjMethod(fa, a1, a2), hhbc::Opcode::FCallObjMethod(fb, b1, b2)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
        }
        (
            hhbc::Opcode::FCallClsMethodM(fa, a1, a2, a3),
            hhbc::Opcode::FCallClsMethodM(fb, b1, b2, b3),
        ) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
            cmp_eq(a3, b3)?;
        }
        (
            hhbc::Opcode::FCallClsMethodSD(fa, a1, a2, a3),
            hhbc::Opcode::FCallClsMethodSD(fb, b1, b2, b3),
        ) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
            cmp_eq(a3, b3)?;
        }
        (
            hhbc::Opcode::FCallObjMethodD(fa, a1, a2, a3),
            hhbc::Opcode::FCallObjMethodD(fb, b1, b2, b3),
        ) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
            cmp_eq(a2, b2)?;
            cmp_eq(a3, b3)?;
        }

        (hhbc::Opcode::FCallCtor(fa, a1), hhbc::Opcode::FCallCtor(fb, b1)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
        }
        (hhbc::Opcode::FCallFuncD(fa, a1), hhbc::Opcode::FCallFuncD(fb, b1)) => {
            cmp_fcallargs(fa, fb)?;
            cmp_eq(a1, b1)?;
        }
        (hhbc::Opcode::FCallFunc(fa), hhbc::Opcode::FCallFunc(fb)) => {
            cmp_fcallargs(fa, fb)?;
        }
        _ => bail!(
            "Instruct mismatch: {} vs {}",
            a.variant_name(),
            b.variant_name()
        ),
    };
    Ok(())
}

fn cmp_instr(a: &Instruct<'_>, b: &Instruct<'_>) -> Result {
    if a == b {
        return Ok(());
    }
    if let (Instruct::Opcode(a), Instruct::Opcode(b)) = (a, b) {
        match a {
            hhbc::Opcode::FCallClsMethod(..)
            | hhbc::Opcode::FCallClsMethodM(..)
            | hhbc::Opcode::FCallClsMethodD(..)
            | hhbc::Opcode::FCallClsMethodS(..)
            | hhbc::Opcode::FCallClsMethodSD(..)
            | hhbc::Opcode::FCallCtor(..)
            | hhbc::Opcode::FCallFunc(..)
            | hhbc::Opcode::FCallFuncD(..)
            | hhbc::Opcode::FCallObjMethod(..)
            | hhbc::Opcode::FCallObjMethodD(..) => {
                return cmp_fcall_instr(a, b);
            }
            _ => bail!(
                "Instruct mismatch: {} vs {}",
                a.variant_name(),
                b.variant_name()
            ),
        }
    }
    bail!(
        "Instruct mismatch: {} vs {}",
        a.variant_name(),
        b.variant_name()
    )
}

fn cmp_param(a: &Param<'_>, b: &Param<'_>) -> Result {
    let Param {
        name: a_name,
        is_variadic: a_is_variadic,
        is_inout: a_is_inout,
        is_readonly: a_is_readonly,
        user_attributes: a_user_attributes,
        type_info: a_type_info,
        default_value: a_default_value,
    } = a;
    let Param {
        name: b_name,
        is_variadic: b_is_variadic,
        is_inout: b_is_inout,
        is_readonly: b_is_readonly,
        user_attributes: b_user_attributes,
        type_info: b_type_info,
        default_value: b_default_value,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_is_variadic, b_is_variadic).qualified("is_variadic")?;
    cmp_eq(a_is_inout, b_is_inout).qualified("is_inout")?;
    cmp_eq(a_is_readonly, b_is_readonly).qualified("is_readonly")?;
    // T126391106 -- BCP sorts attributes of parameters before printing.
    cmp_attributes(a_user_attributes, b_user_attributes).qualified("user_attributes")?;
    cmp_eq(a_type_info, b_type_info).qualified("type_info")?;
    cmp_option(
        a_default_value.as_ref().into_option(),
        b_default_value.as_ref().into_option(),
        |a, b| cmp_eq(&a.expr, &b.expr),
    )
    .qualified("default_value")?;
    Ok(())
}

fn cmp_class(a: &Class<'_>, b: &Class<'_>) -> Result {
    let Class {
        attributes: a_attributes,
        base: a_base,
        implements: a_implements,
        enum_includes: a_enum_includes,
        name: a_name,
        span: a_span,
        uses: a_uses,
        enum_type: a_enum_type,
        methods: a_methods,
        properties: a_properties,
        constants: a_constants,
        type_constants: a_type_constants,
        ctx_constants: a_ctx_constants,
        requirements: a_requirements,
        upper_bounds: a_upper_bounds,
        doc_comment: a_doc_comment,
        flags: a_flags,
    } = a;
    let Class {
        attributes: b_attributes,
        base: b_base,
        implements: b_implements,
        enum_includes: b_enum_includes,
        name: b_name,
        span: b_span,
        uses: b_uses,
        enum_type: b_enum_type,
        methods: b_methods,
        properties: b_properties,
        constants: b_constants,
        type_constants: b_type_constants,
        ctx_constants: b_ctx_constants,
        requirements: b_requirements,
        upper_bounds: b_upper_bounds,
        doc_comment: b_doc_comment,
        flags: b_flags,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_base, b_base).qualified("base")?;

    cmp_eq(a_implements, b_implements).qualified("implements")?;
    cmp_eq(a_enum_includes, b_enum_includes).qualified("enum_includes")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_uses, b_uses).qualified("uses")?;
    cmp_option(
        a_enum_type.as_ref().into_option(),
        b_enum_type.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("enum_type")?;
    cmp_map_t(a_properties, b_properties, cmp_properties).qualified("properties")?;
    cmp_map_t(a_constants, b_constants, cmp_constant).qualified("constants")?;
    cmp_map_t(a_type_constants, b_type_constants, cmp_eq).qualified("type_constants")?;
    cmp_map_t(a_ctx_constants, b_ctx_constants, cmp_eq).qualified("ctx_constants")?;
    cmp_map_t(a_requirements, b_requirements, |a, b| {
        cmp_eq(&a.kind, &b.kind)
    })
    .qualified("requirements")?;
    cmp_map_t(a_upper_bounds, b_upper_bounds, |a, b| {
        cmp_slice(&a.bounds, &b.bounds, cmp_eq)
    })
    .qualified("upper_bounds")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;

    cmp_map_t(a_methods, b_methods, cmp_method).qualified("methods")?;

    Ok(())
}

fn cmp_properties(a: &Property<'_>, b: &Property<'_>) -> Result {
    let Property {
        name: a_name,
        flags: a_flags,
        attributes: a_attributes,
        visibility: a_visibility,
        initial_value: a_initial_value,
        type_info: a_type_info,
        doc_comment: a_doc_comment,
    } = a;
    let Property {
        name: b_name,
        flags: b_flags,
        attributes: b_attributes,
        visibility: b_visibility,
        initial_value: b_initial_value,
        type_info: b_type_info,
        doc_comment: b_doc_comment,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibilitiy")?;
    cmp_initial_value(a_initial_value, b_initial_value).qualified("initial value")?;
    cmp_eq(a_type_info, b_type_info).qualified("type info")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    Ok(())
}

// T126391106: BCP/HCU is not consistent -- if there is no initial value the underlying
// HCU may have Just(Null) or Nothing in that slot.
fn cmp_initial_value(a: &Maybe<TypedValue<'_>>, b: &Maybe<TypedValue<'_>>) -> Result {
    match (a, b) {
        (Maybe::Nothing, Maybe::Just(TypedValue::Null))
        | (Maybe::Just(TypedValue::Null), Maybe::Nothing) => Ok(()),
        _ => cmp_eq(a, b),
    }
}

fn cmp_constant(a: &Constant<'_>, b: &Constant<'_>) -> Result {
    let Constant {
        name: a_name,
        value: a_value,
        attrs: a_attrs,
    } = a;
    let Constant {
        name: b_name,
        value: b_value,
        attrs: b_attrs,
    } = b;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_option(
        a_value.as_ref().into_option(),
        b_value.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("value")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_fatal(a: &Fatal<'_>, b: &Fatal<'_>) -> Result {
    cmp_eq(&a.op, &b.op).qualified("op")?;
    cmp_eq(&a.loc, &b.loc).qualified("loc")?;
    cmp_eq(&a.message, &b.message).qualified("message")?;
    Ok(())
}

fn is_pure(sc: &[naming_special_names_rust::coeffects::Ctx], usc: &[Str<'_>]) -> bool {
    (sc.len() == 1
        && sc.contains(&naming_special_names_rust::coeffects::Ctx::Pure)
        && usc.is_empty())
        || (sc.is_empty() && usc.len() == 1 && usc.iter().all(|usc| usc.as_bstr() == "pure"))
}

fn cmp_static_coeffects(
    a_sc: &[naming_special_names_rust::coeffects::Ctx],
    a_usc: &[Str<'_>],
    b_sc: &[naming_special_names_rust::coeffects::Ctx],
    b_usc: &[Str<'_>],
) -> Result {
    // T126548142 -- odd "pure" behavior
    if is_pure(a_sc, a_usc) && is_pure(b_sc, b_usc) {
        Ok(())
    } else {
        cmp_eq(a_sc, b_sc).qualified("Static coeffecients")?;
        cmp_eq(a_usc, b_usc).qualified("Unenforced static coeffecients")?;
        Ok(())
    }
}

fn cmp_coeffects(a: &hhbc::Coeffects<'_>, b: &hhbc::Coeffects<'_>) -> Result {
    let hhbc::Coeffects {
        static_coeffects: a_sc,
        unenforced_static_coeffects: a_usc,
        fun_param: a_fp,
        cc_param: a_cp,
        cc_this: a_ct,
        cc_reified: a_cr,
        closure_parent_scope: a_cps,
        generator_this: a_gt,
        caller: a_c,
    } = a;

    let hhbc::Coeffects {
        static_coeffects: b_sc,
        unenforced_static_coeffects: b_usc,
        fun_param: b_fp,
        cc_param: b_cp,
        cc_this: b_ct,
        cc_reified: b_cr,
        closure_parent_scope: b_cps,
        generator_this: b_gt,
        caller: b_c,
    } = b;

    cmp_static_coeffects(a_sc, a_usc, b_sc, b_usc)?;
    cmp_eq(a_fp, b_fp)?;
    cmp_eq(a_cp, b_cp)?;
    cmp_eq(a_ct, b_ct)?;
    cmp_eq(a_cr, b_cr)?;
    cmp_eq(&a_cps, &b_cps)?;
    cmp_eq(&a_gt, &b_gt)?;
    cmp_eq(&a_c, &b_c)?;
    Ok(())
}

fn cmp_function(a: &Function<'_>, b: &Function<'_>) -> Result {
    let Function {
        attributes: a_attributes,
        name: a_name,
        body: a_body,
        span: a_span,
        coeffects: a_coeffects,
        flags: a_flags,
        attrs: a_attrs,
    } = a;
    let Function {
        attributes: b_attributes,
        name: b_name,
        body: b_body,
        span: b_span,
        coeffects: b_coeffects,
        flags: b_flags,
        attrs: b_attrs,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_body(a_body, b_body).qualified("body")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_coeffects(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;

    Ok(())
}

fn cmp_method(a: &Method<'_>, b: &Method<'_>) -> Result {
    let Method {
        attributes: a_attributes,
        visibility: a_visibility,
        name: a_name,
        body: a_body,
        span: a_span,
        coeffects: a_coeffects,
        flags: a_flags,
        attrs: a_attrs,
    } = a;
    let Method {
        attributes: b_attributes,
        visibility: b_visibility,
        name: b_name,
        body: b_body,
        span: b_span,
        coeffects: b_coeffects,
        flags: b_flags,
        attrs: b_attrs,
    } = b;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_body(a_body, b_body).qualified("body")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_coeffects(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_rule(a: &Rule<'_>, b: &Rule<'_>) -> Result {
    let Rule {
        kind: a_kind,
        name: a_name,
    } = a;
    let Rule {
        kind: b_kind,
        name: b_name,
    } = b;

    cmp_eq(a_kind, b_kind).qualified("kind")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    Ok(())
}

fn cmp_module(a: &Module<'_>, b: &Module<'_>) -> Result {
    let Module {
        attributes: a_attributes,
        name: a_name,
        span: a_span,
        doc_comment: a_doc_comment,
        exports: a_exports,
        imports: a_imports,
    } = a;
    let Module {
        attributes: b_attributes,
        name: b_name,
        span: b_span,
        doc_comment: b_doc_comment,
        exports: b_exports,
        imports: b_imports,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_option(
        a_exports.as_ref().into_option(),
        b_exports.as_ref().into_option(),
        |a, b| cmp_slice(a, b, cmp_rule),
    )
    .qualified("exports")?;
    cmp_option(
        a_imports.as_ref().into_option(),
        b_imports.as_ref().into_option(),
        |a, b| cmp_slice(a, b, cmp_rule),
    )
    .qualified("imports")?;
    Ok(())
}

/// Compares two include paths. a can be a relative path and b an aboslute path as long as
/// a is the end of b
fn cmp_include(a: &hhbc::IncludePath<'_>, b: &hhbc::IncludePath<'_>) -> Result {
    if a != b {
        match (a, b) {
            (hhbc::IncludePath::SearchPathRelative(a_bs), hhbc::IncludePath::Absolute(b_bs)) => {
                let a_bs = a_bs.as_bstr();
                let b_bs = b_bs.as_bstr();
                if b_bs.ends_with(a_bs) {
                    Ok(())
                } else {
                    bail!("Mismatch {:?} vs {:?}", a, b)
                }
            }
            (hhbc::IncludePath::IncludeRootRelative(_, _), hhbc::IncludePath::Absolute(_)) => {
                Ok(())
            }
            _ => bail!("Mismatch {:?} vs {:?}", a, b),
        }
    } else {
        Ok(())
    }
}

fn cmp_symbol_refs(a: &SymbolRefs<'_>, b: &SymbolRefs<'_>) -> Result {
    let SymbolRefs {
        includes: a_includes,
        constants: a_constants,
        functions: a_functions,
        classes: a_classes,
    } = a;
    let SymbolRefs {
        includes: b_includes,
        constants: b_constants,
        functions: b_functions,
        classes: b_classes,
    } = b;

    cmp_includes(a_includes, b_includes).qualified("includes")?;
    cmp_slice(a_constants, b_constants, cmp_eq).qualified("constants")?;
    cmp_slice(a_functions, b_functions, cmp_eq).qualified("functions")?;
    cmp_slice(a_classes, b_classes, cmp_eq).qualified("classes")?;
    Ok(())
}

/// T126391106: BCP drops information -- &s TCF with nullable before printing.
fn cmp_type_constraint_flags(
    a: &hhvm_types_ffi::ffi::TypeConstraintFlags,
    b: &hhvm_types_ffi::ffi::TypeConstraintFlags,
) -> Result {
    use hhvm_types_ffi::ffi::TypeConstraintFlags;
    let a_flags = *a & TypeConstraintFlags::Nullable;
    let b_flags = *b & TypeConstraintFlags::Nullable;
    cmp_eq(&a_flags, &b_flags).qualified("TypeConstraintFlags")?;
    Ok(())
}

// T126391106: BCP doesn't disambiguate a constraint name of Just("") and Nothing
fn cmp_type_constraint_name(a: &Maybe<Str<'_>>, b: &Maybe<Str<'_>>) -> Result {
    match (a, b) {
        (Maybe::Nothing, Maybe::Just(s)) | (Maybe::Just(s), Maybe::Nothing) => {
            if s.as_bstr() == "" {
                Ok(())
            } else {
                bail!("Constraint name mismatch: {:?} vs {:?}", a, b)
            }
        }
        (a, b) => cmp_eq(a, b),
    }
}

fn cmp_type_constraint(a: &hhbc::Constraint<'_>, b: &hhbc::Constraint<'_>) -> Result {
    let hhbc::Constraint {
        name: a_name,
        flags: a_flags,
    } = a;
    let hhbc::Constraint {
        name: b_name,
        flags: b_flags,
    } = b;
    cmp_type_constraint_name(a_name, b_name).qualified("constraints")?;
    cmp_type_constraint_flags(a_flags, b_flags)?;
    Ok(())
}

/// User_type isn't printed in typedef's typeinfo.
fn cmp_typedef_typeinfo(a: &TypeInfo<'_>, b: &TypeInfo<'_>) -> Result {
    let TypeInfo {
        user_type: _a_user_type,
        type_constraint: a_constraint,
    } = a;
    let TypeInfo {
        user_type: _b_user_type,
        type_constraint: b_constraint,
    } = b;
    cmp_type_constraint(a_constraint, b_constraint).qualified("constraints")?;
    Ok(())
}

fn cmp_typedef(a: &Typedef<'_>, b: &Typedef<'_>) -> Result {
    let Typedef {
        name: a_name,
        attributes: a_attributes,
        type_info_union: a_type_info_union,
        type_structure: a_type_structure,
        span: a_span,
        attrs: a_attrs,
        case_type: a_case_type,
    } = a;
    let Typedef {
        name: b_name,
        attributes: b_attributes,
        type_info_union: b_type_info_union,
        type_structure: b_type_structure,
        span: b_span,
        attrs: b_attrs,
        case_type: b_case_type,
    } = b;

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_slice(a_type_info_union, b_type_info_union, cmp_typedef_typeinfo)
        .qualified("type_info_union")?;
    cmp_eq(a_type_structure, b_type_structure).qualified("type_structure")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    cmp_eq(a_case_type, b_case_type).qualified("case_type")?;
    Ok(())
}

fn cmp_unit(a_unit: &Unit<'_>, b_unit: &Unit<'_>) -> Result {
    let Unit {
        adata: a_adata,
        functions: a_functions,
        classes: a_classes,
        modules: a_modules,
        typedefs: a_typedefs,
        file_attributes: a_file_attributes,
        module_use: a_module_use,
        symbol_refs: a_symbol_refs,
        constants: a_constants,
        fatal: a_fatal,
        missing_symbols: _,
        error_symbols: _,
        valid_utf8: _,
        invalid_utf8_offset: _,
    } = a_unit;
    let Unit {
        adata: b_adata,
        functions: b_functions,
        classes: b_classes,
        modules: b_modules,
        typedefs: b_typedefs,
        file_attributes: b_file_attributes,
        module_use: b_module_use,
        symbol_refs: b_symbol_refs,
        constants: b_constants,
        fatal: b_fatal,
        missing_symbols: _,
        error_symbols: _,
        valid_utf8: _,
        invalid_utf8_offset: _,
    } = b_unit;

    cmp_map_t(a_adata, b_adata, cmp_eq).qualified("adata")?;

    cmp_map_t(a_typedefs, b_typedefs, cmp_typedef).qualified("typedefs")?;

    cmp_attributes(a_file_attributes, b_file_attributes).qualified("file_attributes")?;

    cmp_option(
        a_fatal.as_ref().into_option(),
        b_fatal.as_ref().into_option(),
        cmp_fatal,
    )
    .qualified("fatal")?;

    cmp_map_t(a_constants, b_constants, cmp_constant).qualified("constants")?;

    cmp_symbol_refs(a_symbol_refs, b_symbol_refs).qualified("symbol_refs")?;

    cmp_map_t(a_modules, b_modules, cmp_module).qualified("modules")?;

    cmp_option(
        a_module_use.as_ref().into_option(),
        b_module_use.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("module_use")?;

    cmp_map_t(a_functions, b_functions, cmp_function).qualified("functions")?;
    cmp_map_t(a_classes, b_classes, cmp_class).qualified("classes")?;

    Ok(())
}

/// Fancy version of `PartialEq::eq(a, b)` which also tries to report exactly
/// where the mismatch occurred.
pub fn cmp_hack_c_unit(a: &Unit<'_>, b: &Unit<'_>) -> Result {
    cmp_unit(a, b).with_raw(|| "unit".to_string())
}
