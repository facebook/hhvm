use anyhow::Result;
use ffi::Pair;
use ffi::Str;
use ffi::Triple;
use hhbc::hackc_unit::HackCUnit;
use hhbc::hhas_attribute::HhasAttribute;
use hhbc::hhas_body::HhasBody;
use hhbc::hhas_class::HhasClass;
use hhbc::hhas_constant::HhasConstant;
use hhbc::hhas_function::HhasFunction;
use hhbc::hhas_method::HhasMethod;
use hhbc::hhas_module::HhasModule;
use hhbc::hhas_param::HhasParam;
use hhbc::hhas_pos::HhasPos;
use hhbc::hhas_symbol_refs::HhasSymbolRefs;
use hhbc::hhas_typedef::HhasTypedef;
use hhbc::FatalOp;

use crate::code_path::CodePath;
use crate::helpers::*;

/// Compare two HackCUnits semantically.
///
/// For a semantic comparison we don't care about any bytecode differences as
/// long as:
///
///   1. Instructs with side-effects exist and are in the same order.
///
///   2. Instructs that can mutate COW datatypes (dict, vec, keyset, string)
///   have the same COW behavior (if a datatype would have been mutated in-place
///   in a_unit it should be mutated in-place in b_unit).
///
///   3. An exception thrown from an instruction will be handled the same way
///
/// In general most of the HackCUnit is compared using Eq - although structs are
/// destructured so an error can report where the difference occurred.
///
/// The "interesting" bit happens in `body::compare_bodies()`.
///
pub fn sem_diff_unit<'arena>(a_unit: &HackCUnit<'arena>, b_unit: &HackCUnit<'arena>) -> Result<()> {
    let HackCUnit {
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
    } = a_unit;
    let HackCUnit {
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
    } = b_unit;

    let path = CodePath::name("HackCUnit");

    sem_diff_map_t(&path.qualified("adata"), a_adata, b_adata, sem_diff_eq)?;

    sem_diff_map_t(
        &path.qualified("typedefs"),
        a_typedefs,
        b_typedefs,
        sem_diff_typedef,
    )?;

    sem_diff_attributes(
        &path.qualified("file_attributes"),
        a_file_attributes,
        b_file_attributes,
    )?;

    sem_diff_option(
        &path.qualified("fatal"),
        a_fatal.as_ref().into_option(),
        b_fatal.as_ref().into_option(),
        sem_diff_fatal,
    )?;

    sem_diff_map_t(
        &path.qualified("constants"),
        a_constants,
        b_constants,
        sem_diff_constant,
    )?;

    sem_diff_symbol_refs(&path.qualified("symbol_refs"), a_symbol_refs, b_symbol_refs)?;

    sem_diff_map_t(
        &path.qualified("modules"),
        a_modules,
        b_modules,
        sem_diff_module,
    )?;

    sem_diff_option(
        &path.qualified("module_use"),
        a_module_use.as_ref().into_option(),
        b_module_use.as_ref().into_option(),
        sem_diff_eq,
    )?;

    sem_diff_map_t(
        &path.qualified("functions"),
        a_functions.as_arena_ref(),
        b_functions.as_arena_ref(),
        |path, a, b| sem_diff_function(path, a, a_unit, b, b_unit),
    )?;

    sem_diff_map_t(
        &path.qualified("classes"),
        a_classes.as_arena_ref(),
        b_classes.as_arena_ref(),
        |path, a, b| sem_diff_class(path, a, a_unit, b, b_unit),
    )?;

    Ok(())
}

fn sem_diff_attribute(
    path: &CodePath<'_>,
    a: &HhasAttribute<'_>,
    b: &HhasAttribute<'_>,
) -> Result<()> {
    let HhasAttribute {
        name: a_name,
        arguments: a_arguments,
    } = a;
    let HhasAttribute {
        name: b_name,
        arguments: b_arguments,
    } = b;
    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_slice(
        &path.qualified("arguments"),
        a_arguments,
        b_arguments,
        sem_diff_eq,
    )?;
    Ok(())
}

fn sem_diff_attributes(
    path: &CodePath<'_>,
    a: &[HhasAttribute<'_>],
    b: &[HhasAttribute<'_>],
) -> Result<()> {
    sem_diff_slice(path, a, b, sem_diff_attribute)
}

fn sem_diff_body<'arena>(
    path: &CodePath<'_>,
    a: &'arena HhasBody<'arena>,
    a_unit: &HackCUnit<'arena>,
    b: &'arena HhasBody<'arena>,
    b_unit: &HackCUnit<'arena>,
) -> Result<()> {
    let HhasBody {
        body_instrs: _,
        decl_vars: _,
        num_iters: a_num_iters,
        is_memoize_wrapper: a_is_memoize_wrapper,
        is_memoize_wrapper_lsb: a_is_memoize_wrapper_lsb,
        upper_bounds: a_upper_bounds,
        shadowed_tparams: a_shadowed_tparams,
        params: a_params,
        return_type_info: a_return_type_info,
        doc_comment: a_doc_comment,
    } = a;
    let HhasBody {
        body_instrs: _,
        decl_vars: _,
        num_iters: b_num_iters,
        is_memoize_wrapper: b_is_memoize_wrapper,
        is_memoize_wrapper_lsb: b_is_memoize_wrapper_lsb,
        upper_bounds: b_upper_bounds,
        shadowed_tparams: b_shadowed_tparams,
        params: b_params,
        return_type_info: b_return_type_info,
        doc_comment: b_doc_comment,
    } = b;

    sem_diff_eq(&path.qualified("num_iters"), a_num_iters, b_num_iters)?;
    sem_diff_slice(
        &path.qualified("params"),
        a_params,
        b_params,
        sem_diff_param,
    )?;
    sem_diff_eq(
        &path.qualified("is_memoize_wrapper"),
        a_is_memoize_wrapper,
        b_is_memoize_wrapper,
    )?;
    sem_diff_eq(
        &path.qualified("is_memoize_wrapper_lsb"),
        a_is_memoize_wrapper_lsb,
        b_is_memoize_wrapper_lsb,
    )?;
    sem_diff_eq(&path.qualified("doc_comment"), a_doc_comment, b_doc_comment)?;
    sem_diff_eq(
        &path.qualified("return_type_info"),
        a_return_type_info,
        b_return_type_info,
    )?;
    sem_diff_eq(
        &path.qualified("upper_bounds"),
        a_upper_bounds,
        b_upper_bounds,
    )?;
    sem_diff_eq(
        &path.qualified("shadowed_tparams"),
        a_shadowed_tparams,
        b_shadowed_tparams,
    )?;

    // Don't bother comparing decl_vars - when we use named vars later we'll
    // compare the names to ensure we're using the same ones.
    // sem_diff_set_t(&path.qualified("decl_vars"), a_decl_vars, b_decl_vars)?;

    // This compares the instrs themselves.
    crate::body::compare_bodies(path, a, a_unit, b, b_unit)
}

fn sem_diff_param<'arena>(
    path: &CodePath<'_>,
    a: &'arena HhasParam<'arena>,
    b: &'arena HhasParam<'arena>,
) -> Result<()> {
    let HhasParam {
        name: a_name,
        is_variadic: a_is_variadic,
        is_inout: a_is_inout,
        is_readonly: a_is_readonly,
        user_attributes: a_user_attributes,
        type_info: a_type_info,
        default_value: a_default_value,
    } = a;
    let HhasParam {
        name: b_name,
        is_variadic: b_is_variadic,
        is_inout: b_is_inout,
        is_readonly: b_is_readonly,
        user_attributes: b_user_attributes,
        type_info: b_type_info,
        default_value: b_default_value,
    } = b;

    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_eq(&path.qualified("is_variadic"), a_is_variadic, b_is_variadic)?;
    sem_diff_eq(&path.qualified("is_inout"), a_is_inout, b_is_inout)?;
    sem_diff_eq(&path.qualified("is_readonly"), a_is_readonly, b_is_readonly)?;
    sem_diff_eq(
        &path.qualified("user_attributes"),
        a_user_attributes,
        b_user_attributes,
    )?;
    sem_diff_eq(&path.qualified("type_info"), a_type_info, b_type_info)?;
    sem_diff_option(
        &path.qualified("default_value"),
        a_default_value.as_ref().into_option(),
        b_default_value.as_ref().into_option(),
        |path, Pair(_, a_text), Pair(_, b_text)| sem_diff_eq(path, a_text, b_text),
    )?;

    Ok(())
}

fn sem_diff_class<'arena>(
    path: &CodePath<'_>,
    a: &'arena HhasClass<'arena>,
    a_unit: &HackCUnit<'arena>,
    b: &'arena HhasClass<'arena>,
    b_unit: &HackCUnit<'arena>,
) -> Result<()> {
    let HhasClass {
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
    let HhasClass {
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

    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_attributes(&path.qualified("attributes"), a_attributes, b_attributes)?;
    sem_diff_eq(&path.qualified("base"), a_base, b_base)?;

    sem_diff_eq(&path.qualified("implements"), a_implements, b_implements)?;
    sem_diff_eq(
        &path.qualified("enum_includes"),
        a_enum_includes,
        b_enum_includes,
    )?;
    sem_diff_eq(&path.qualified("span"), a_span, b_span)?;
    sem_diff_eq(&path.qualified("uses"), a_uses, b_uses)?;
    sem_diff_option(
        &path.qualified("enum_type"),
        a_enum_type.as_ref().into_option(),
        b_enum_type.as_ref().into_option(),
        sem_diff_eq,
    )?;
    sem_diff_map_t(
        &path.qualified("properties"),
        a_properties,
        b_properties,
        sem_diff_eq,
    )?;
    sem_diff_map_t(
        &path.qualified("constants"),
        a_constants,
        b_constants,
        sem_diff_constant,
    )?;
    sem_diff_map_t(
        &path.qualified("type_constants"),
        a_type_constants,
        b_type_constants,
        sem_diff_eq,
    )?;
    sem_diff_map_t(
        &path.qualified("ctx_constants"),
        a_ctx_constants,
        b_ctx_constants,
        sem_diff_eq,
    )?;
    sem_diff_map_t(
        &path.qualified("requirements"),
        a_requirements,
        b_requirements,
        |path, a, b| sem_diff_eq(path, &a.1, &b.1),
    )?;
    sem_diff_map_t(
        &path.qualified("upper_bounds"),
        a_upper_bounds,
        b_upper_bounds,
        |path, a, b| sem_diff_slice(path, &a.1, &b.1, sem_diff_eq),
    )?;
    sem_diff_eq(&path.qualified("doc_comment"), a_doc_comment, b_doc_comment)?;
    sem_diff_eq(&path.qualified("flags"), a_flags, b_flags)?;

    sem_diff_map_t(
        &path.qualified("methods"),
        a_methods,
        b_methods,
        |path, a, b| sem_diff_method(path, a, a_unit, b, b_unit),
    )?;

    Ok(())
}

fn sem_diff_constant(
    path: &CodePath<'_>,
    a: &HhasConstant<'_>,
    b: &HhasConstant<'_>,
) -> Result<()> {
    let HhasConstant {
        name: a_name,
        value: a_value,
        is_abstract: a_is_abstract,
    } = a;
    let HhasConstant {
        name: b_name,
        value: b_value,
        is_abstract: b_is_abstract,
    } = b;
    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_option(
        &path.qualified("value"),
        a_value.as_ref().into_option(),
        b_value.as_ref().into_option(),
        sem_diff_eq,
    )?;
    sem_diff_eq(&path.qualified("is_abstract"), a_is_abstract, b_is_abstract)?;
    Ok(())
}

fn sem_diff_fatal(
    path: &CodePath<'_>,
    a: &Triple<FatalOp, HhasPos, Str<'_>>,
    b: &Triple<FatalOp, HhasPos, Str<'_>>,
) -> Result<()> {
    sem_diff_eq(&path.index(0), &a.0, &b.0)?;
    sem_diff_eq(&path.index(1), &a.1, &b.1)?;
    sem_diff_eq(&path.index(2), &a.2, &b.2)?;
    Ok(())
}

fn sem_diff_function<'arena>(
    path: &CodePath<'_>,
    a: &'arena HhasFunction<'arena>,
    a_unit: &HackCUnit<'arena>,
    b: &'arena HhasFunction<'arena>,
    b_unit: &HackCUnit<'arena>,
) -> Result<()> {
    let HhasFunction {
        attributes: a_attributes,
        name: a_name,
        body: a_body,
        span: a_span,
        coeffects: a_coeffects,
        flags: a_flags,
        attrs: a_attrs,
    } = a;
    let HhasFunction {
        attributes: b_attributes,
        name: b_name,
        body: b_body,
        span: b_span,
        coeffects: b_coeffects,
        flags: b_flags,
        attrs: b_attrs,
    } = b;

    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_attributes(&path.qualified("attributes"), a_attributes, b_attributes)?;
    sem_diff_body(&path.qualified("body"), a_body, a_unit, b_body, b_unit)?;
    sem_diff_eq(&path.qualified("span"), a_span, b_span)?;
    sem_diff_eq(&path.qualified("coeffects"), a_coeffects, b_coeffects)?;
    sem_diff_eq(&path.qualified("flags"), a_flags, b_flags)?;
    sem_diff_eq(&path.qualified("attrs"), a_attrs, b_attrs)?;

    Ok(())
}

fn sem_diff_method<'arena>(
    path: &CodePath<'_>,
    a: &'arena HhasMethod<'arena>,
    a_unit: &HackCUnit<'arena>,
    b: &'arena HhasMethod<'arena>,
    b_unit: &HackCUnit<'arena>,
) -> Result<()> {
    let HhasMethod {
        attributes: a_attributes,
        visibility: a_visibility,
        name: a_name,
        body: a_body,
        span: a_span,
        coeffects: a_coeffects,
        flags: a_flags,
        attrs: a_attrs,
    } = a;
    let HhasMethod {
        attributes: b_attributes,
        visibility: b_visibility,
        name: b_name,
        body: b_body,
        span: b_span,
        coeffects: b_coeffects,
        flags: b_flags,
        attrs: b_attrs,
    } = b;
    sem_diff_attributes(&path.qualified("attributes"), a_attributes, b_attributes)?;
    sem_diff_eq(&path.qualified("visibility"), a_visibility, b_visibility)?;
    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_body(&path.qualified("body"), a_body, a_unit, b_body, b_unit)?;
    sem_diff_eq(&path.qualified("span"), a_span, b_span)?;
    sem_diff_eq(&path.qualified("coeffects"), a_coeffects, b_coeffects)?;
    sem_diff_eq(&path.qualified("flags"), a_flags, b_flags)?;
    sem_diff_eq(&path.qualified("attrs"), a_attrs, b_attrs)?;
    Ok(())
}

fn sem_diff_module<'arena>(
    path: &CodePath<'_>,
    a: &HhasModule<'arena>,
    b: &HhasModule<'arena>,
) -> Result<()> {
    let HhasModule {
        attributes: a_attributes,
        name: a_name,
        span: a_span,
    } = a;
    let HhasModule {
        attributes: b_attributes,
        name: b_name,
        span: b_span,
    } = b;

    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_attributes(&path.qualified("attributes"), a_attributes, b_attributes)?;
    sem_diff_eq(&path.qualified("span"), a_span, b_span)?;
    Ok(())
}

fn sem_diff_symbol_refs<'arena>(
    path: &CodePath<'_>,
    a: &HhasSymbolRefs<'arena>,
    b: &HhasSymbolRefs<'arena>,
) -> Result<()> {
    let HhasSymbolRefs {
        includes: a_includes,
        constants: a_constants,
        functions: a_functions,
        classes: a_classes,
    } = a;
    let HhasSymbolRefs {
        includes: b_includes,
        constants: b_constants,
        functions: b_functions,
        classes: b_classes,
    } = b;

    sem_diff_slice(
        &path.qualified("includes"),
        a_includes,
        b_includes,
        sem_diff_eq,
    )?;
    sem_diff_slice(
        &path.qualified("constants"),
        a_constants,
        b_constants,
        sem_diff_eq,
    )?;
    sem_diff_slice(
        &path.qualified("functions"),
        a_functions,
        b_functions,
        sem_diff_eq,
    )?;
    sem_diff_slice(
        &path.qualified("classes"),
        a_classes,
        b_classes,
        sem_diff_eq,
    )?;
    Ok(())
}

fn sem_diff_typedef<'arena>(
    path: &CodePath<'_>,
    a: &HhasTypedef<'arena>,
    b: &HhasTypedef<'arena>,
) -> Result<()> {
    let HhasTypedef {
        name: a_name,
        attributes: a_attributes,
        type_info: a_type_info,
        type_structure: a_type_structure,
        span: a_span,
        attrs: a_attrs,
    } = a;
    let HhasTypedef {
        name: b_name,
        attributes: b_attributes,
        type_info: b_type_info,
        type_structure: b_type_structure,
        span: b_span,
        attrs: b_attrs,
    } = b;

    sem_diff_eq(&path.qualified("name"), a_name, b_name)?;
    sem_diff_attributes(&path.qualified("attributes"), a_attributes, b_attributes)?;
    sem_diff_eq(&path.qualified("type_info"), a_type_info, b_type_info)?;
    sem_diff_eq(
        &path.qualified("type_structure"),
        a_type_structure,
        b_type_structure,
    )?;
    sem_diff_eq(&path.qualified("span"), a_span, b_span)?;
    sem_diff_eq(&path.qualified("attrs"), a_attrs, b_attrs)?;
    Ok(())
}
