// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Pair;
use ffi::Slice;
use ffi::Str;
use ffi::Triple;
use hash::HashMap;
use hash::HashSet;
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
use hhbc::Instruct;
use std::fmt;

#[derive(Debug, Hash, PartialEq, Eq)]
pub(crate) struct CmpError {
    what: String,
    loc: Option<String>,
}

impl CmpError {
    fn error(what: String) -> Self {
        CmpError { what, loc: None }
    }
}

impl fmt::Display for CmpError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let loc = self.loc.as_ref().map_or("", String::as_str);
        write!(f, "{}: {}", loc, self.what)
    }
}

macro_rules! bail {
    ($msg:literal $(,)?) => {
        return Err(CmpError::error(format!($msg)))
    };
    ($err:expr $(,)?) => {
        return Err(CmpError::error(format!($err)))
    };
    ($fmt:expr, $($arg:tt)*) => {
        return Err(CmpError::error(format!($fmt, $($arg)*)))
    };
}

trait CmpContext {
    fn with_indexed<F: FnOnce() -> String>(self, f: F) -> Self;
    fn indexed(self, idx: &str) -> Self;
    fn qualified(self, name: &str) -> Self;
    fn with_raw<F: FnOnce() -> String>(self, f: F) -> Self;
}

impl<T> CmpContext for Result<T, CmpError> {
    fn with_raw<F>(self, f: F) -> Self
    where
        F: FnOnce() -> String,
    {
        match self {
            Ok(_) => self,
            Err(CmpError { what, loc: None }) => Err(CmpError {
                what,
                loc: Some(f()),
            }),
            Err(CmpError {
                what,
                loc: Some(loc),
            }) => Err(CmpError {
                what,
                loc: Some(format!("{}{loc}", f())),
            }),
        }
    }

    fn with_indexed<F>(self, f: F) -> Self
    where
        F: FnOnce() -> String,
    {
        self.with_raw(|| format!("[\"{}\"]", f()))
    }

    fn indexed(self, idx: &str) -> Self {
        self.with_indexed(|| idx.to_string())
    }

    fn qualified(self, name: &str) -> Self {
        self.with_raw(|| format!(".{name}"))
    }
}

type Result<T, E = CmpError> = std::result::Result<T, E>;

trait MapName {
    fn get_name(&self) -> &str;
}

impl MapName for hhbc::hhas_adata::HhasAdata<'_> {
    fn get_name(&self) -> &str {
        self.id.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_class::HhasClass<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_constant::HhasConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_coeffects::HhasCtxConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_function::HhasFunction<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_method::HhasMethod<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_module::HhasModule<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_property::HhasProperty<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_typedef::HhasTypedef<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_type_const::HhasTypeConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for ffi::Pair<hhbc::ClassName<'_>, hhbc::hhas_class::TraitReqKind> {
    fn get_name(&self) -> &str {
        self.0.unsafe_as_str()
    }
}

impl MapName for ffi::Pair<Str<'_>, Slice<'_, hhbc::hhas_type::HhasTypeInfo<'_>>> {
    fn get_name(&self) -> &str {
        self.0.unsafe_as_str()
    }
}

fn cmp_eq<Ta, Tb>(a: &Ta, b: &Tb) -> Result<()>
where
    Ta: PartialEq<Tb> + fmt::Debug,
    Tb: fmt::Debug,
{
    if a != b {
        bail!("Mismatch {:?} vs {:?}", a, b);
    }
    Ok(())
}

fn cmp_map_t<'a, 'b, Ta: 'a, Tb: 'b, F>(a: &'a [Ta], b: &'b [Tb], f_eq: F) -> Result<()>
where
    Ta: MapName,
    Tb: MapName,
    F: Fn(&'a Ta, &'b Tb) -> Result<()>,
{
    let a_hash: HashMap<&str, &Ta> = a.iter().map(|t| (t.get_name(), t)).collect();
    let b_hash: HashMap<&str, &Tb> = b.iter().map(|t| (t.get_name(), t)).collect();
    let a_keys: HashSet<&str> = a_hash.keys().copied().collect();
    let b_keys: HashSet<&str> = b_hash.keys().copied().collect();
    for k in &a_keys & &b_keys {
        f_eq(a_hash[k], b_hash[k]).with_indexed(|| k.to_string())?;
    }

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("lhs has key {k} but rhs does not");
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("rhs has key {k} but lhs does not");
    }

    Ok(())
}

fn cmp_set_t<'a, T>(a: &'a [T], b: &'a [T]) -> Result<()>
where
    T: std::hash::Hash + Eq + std::fmt::Debug,
{
    let a_keys: HashSet<&T> = a.iter().collect();
    let b_keys: HashSet<&T> = b.iter().collect();

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("lhs has value {k:?} but rhs does not");
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("rhs has value {k:?} but lhs does not");
    }

    Ok(())
}

fn cmp_option<T, F>(a: Option<&T>, b: Option<&T>, f_eq: F) -> Result<()>
where
    T: fmt::Debug,
    F: FnOnce(&T, &T) -> Result<()>,
{
    match (a, b) {
        (None, None) => Ok(()),
        (Some(inner), None) => bail!("Some({inner:?})\nNone"),
        (None, Some(inner)) => bail!("None\nSome({inner:?})"),
        (Some(lhs), Some(rhs)) => f_eq(lhs, rhs),
    }
}

fn cmp_slice<'a, V, F>(a: &'a [V], b: &'a [V], f_eq: F) -> Result<()>
where
    F: Fn(&V, &V) -> Result<()>,
{
    if a.len() != b.len() {
        bail!("Length mismatch: {} vs {}", a.len(), b.len());
    }
    for (i, (av, bv)) in a.iter().zip(b.iter()).enumerate() {
        f_eq(av, bv).with_indexed(|| i.to_string())?;
    }
    Ok(())
}

fn cmp_attribute(a: &HhasAttribute<'_>, b: &HhasAttribute<'_>) -> Result<()> {
    let HhasAttribute {
        name: a_name,
        arguments: a_arguments,
    } = a;
    let HhasAttribute {
        name: b_name,
        arguments: b_arguments,
    } = b;
    cmp_eq(a_name, b_name)?;
    cmp_slice(a_arguments, b_arguments, cmp_eq).qualified("arguments")?;
    Ok(())
}

fn cmp_attributes(a: &[HhasAttribute<'_>], b: &[HhasAttribute<'_>]) -> Result<()> {
    cmp_slice(a, b, cmp_attribute)
}

fn cmp_body(a: &HhasBody<'_>, b: &HhasBody<'_>) -> Result<()> {
    let HhasBody {
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
    } = a;
    let HhasBody {
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
    } = b;

    cmp_eq(a_num_iters, b_num_iters).qualified("num_iters")?;
    cmp_slice(a_params, b_params, cmp_param).qualified("params")?;
    cmp_eq(a_is_memoize_wrapper, b_is_memoize_wrapper).qualified("is_memoize_wrapper")?;
    cmp_eq(a_is_memoize_wrapper_lsb, b_is_memoize_wrapper_lsb)
        .qualified("is_memoize_wrapper_lsb")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
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

fn cmp_instr(a: &Instruct<'_>, b: &Instruct<'_>) -> Result<()> {
    if a != b {
        bail!(
            "Instruct mismatch: {} vs {}",
            a.variant_name(),
            b.variant_name()
        );
    }

    Ok(())
}

fn cmp_param(a: &HhasParam<'_>, b: &HhasParam<'_>) -> Result<()> {
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

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_eq(a_is_variadic, b_is_variadic).qualified("is_variadic")?;
    cmp_eq(a_is_inout, b_is_inout).qualified("is_inout")?;
    cmp_eq(a_is_readonly, b_is_readonly).qualified("is_readonly")?;
    cmp_eq(a_user_attributes, b_user_attributes).qualified("user_attributes")?;
    cmp_eq(a_type_info, b_type_info).qualified("type_info")?;
    cmp_option(
        a_default_value.as_ref().into_option(),
        b_default_value.as_ref().into_option(),
        |Pair(_, a_text), Pair(_, b_text)| cmp_eq(a_text, b_text),
    )
    .qualified("default_value")?;

    Ok(())
}

fn cmp_class(a: &HhasClass<'_>, b: &HhasClass<'_>) -> Result<()> {
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
    cmp_map_t(a_properties, b_properties, cmp_eq).qualified("properties")?;
    cmp_map_t(a_constants, b_constants, cmp_constant).qualified("constants")?;
    cmp_map_t(a_type_constants, b_type_constants, cmp_eq).qualified("type_constants")?;
    cmp_map_t(a_ctx_constants, b_ctx_constants, cmp_eq).qualified("ctx_constants")?;
    cmp_map_t(a_requirements, b_requirements, |a, b| cmp_eq(&a.1, &b.1))
        .qualified("requirements")?;
    cmp_map_t(a_upper_bounds, b_upper_bounds, |a, b| {
        cmp_slice(&a.1, &b.1, cmp_eq)
    })
    .qualified("upper_bounds")?;
    cmp_eq(a_doc_comment, b_doc_comment).qualified("doc_comment")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;

    cmp_map_t(a_methods, b_methods, cmp_method).qualified("methods")?;

    Ok(())
}

fn cmp_constant(a: &HhasConstant<'_>, b: &HhasConstant<'_>) -> Result<()> {
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
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_option(
        a_value.as_ref().into_option(),
        b_value.as_ref().into_option(),
        cmp_eq,
    )
    .qualified("value")?;
    cmp_eq(a_is_abstract, b_is_abstract).qualified("is_abstract")?;
    Ok(())
}

fn cmp_fatal(
    a: &Triple<FatalOp, HhasPos, Str<'_>>,
    b: &Triple<FatalOp, HhasPos, Str<'_>>,
) -> Result<()> {
    cmp_eq(&a.0, &b.0).indexed("0")?;
    cmp_eq(&a.1, &b.1).indexed("1")?;
    cmp_eq(&a.2, &b.2).indexed("2")?;
    Ok(())
}

fn cmp_function(a: &HhasFunction<'_>, b: &HhasFunction<'_>) -> Result<()> {
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

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_body(a_body, b_body).qualified("body")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;

    Ok(())
}

fn cmp_method(a: &HhasMethod<'_>, b: &HhasMethod<'_>) -> Result<()> {
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
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_visibility, b_visibility).qualified("visibility")?;
    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_body(a_body, b_body).qualified("body")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_coeffects, b_coeffects).qualified("coeffects")?;
    cmp_eq(a_flags, b_flags).qualified("flags")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_module(a: &HhasModule<'_>, b: &HhasModule<'_>) -> Result<()> {
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

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    Ok(())
}

fn cmp_symbol_refs(a: &HhasSymbolRefs<'_>, b: &HhasSymbolRefs<'_>) -> Result<()> {
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

    cmp_slice(a_includes, b_includes, cmp_eq).qualified("includes")?;
    cmp_slice(a_constants, b_constants, cmp_eq).qualified("constants")?;
    cmp_slice(a_functions, b_functions, cmp_eq).qualified("functions")?;
    cmp_slice(a_classes, b_classes, cmp_eq).qualified("classes")?;
    Ok(())
}

fn cmp_typedef(a: &HhasTypedef<'_>, b: &HhasTypedef<'_>) -> Result<()> {
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

    cmp_eq(a_name, b_name).qualified("name")?;
    cmp_attributes(a_attributes, b_attributes).qualified("attributes")?;
    cmp_eq(a_type_info, b_type_info).qualified("type_info")?;
    cmp_eq(a_type_structure, b_type_structure).qualified("type_structure")?;
    cmp_eq(a_span, b_span).qualified("span")?;
    cmp_eq(a_attrs, b_attrs).qualified("attrs")?;
    Ok(())
}

fn cmp_unit(a_unit: &HackCUnit<'_>, b_unit: &HackCUnit<'_>) -> Result<()> {
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

    cmp_map_t(
        a_functions.as_arena_ref(),
        b_functions.as_arena_ref(),
        cmp_function,
    )
    .qualified("functions")?;

    cmp_map_t(
        a_classes.as_arena_ref(),
        b_classes.as_arena_ref(),
        cmp_class,
    )
    .qualified("classes")?;

    Ok(())
}

/// Fancy version of `PartialEq::eq(a, b)` which also tries to report exactly
/// where the mismatch occurred.
pub(crate) fn cmp_hack_c_unit(a: &HackCUnit<'_>, b: &HackCUnit<'_>) -> Result<()> {
    cmp_unit(a, b).with_raw(|| "unit".to_string())
}
