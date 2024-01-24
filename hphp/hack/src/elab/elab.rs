// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]
#![cfg_attr(not(rust_lib_feature = "let_chains"), feature(let_chains))]

mod elab_utils;
mod env;
mod lambda_captures;
mod lift_await;
mod pass;
mod passes;
mod transform;
mod typed_local;

/// Private convenience module for simplifying imports in pass implementations.
mod prelude {
    pub use std::ops::ControlFlow;
    pub use std::ops::ControlFlow::Break;
    pub use std::ops::ControlFlow::Continue;
    pub use std::rc::Rc;

    pub use naming_special_names_rust as sn;
    pub use oxidized::naming_error::NamingError;
    pub use oxidized::naming_error::UnsupportedFeature;
    pub use oxidized::naming_phase_error::ExperimentalFeature;
    pub use oxidized::naming_phase_error::NamingPhaseError;
    pub use oxidized::nast;
    pub use oxidized::nast_check_error::NastCheckError;
    pub use oxidized::parsing_error::ParsingError;

    pub(crate) use crate::elab_utils;
    pub use crate::env::Env;
    pub use crate::pass::Pass;
    #[allow(unused_imports)]
    pub use crate::transform::Transform;
}

use std::sync::Arc;

use env::Env;
use env::ProgramSpecificOptions;
use oxidized::namespace_env;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast;
use oxidized::typechecker_options::TypecheckerOptions;
use pass::Pass;
use relative_path::RelativePath;
use transform::Transform;
use vec1::Vec1;

/// Provided for use in hackc as a simple collection of knobs outside of namespace env.
#[derive(Clone, Debug, Default)]
pub struct CodegenOpts {
    pub textual_remove_memoize: bool,
}

/// Provided for use in hackc, where we have an `ns_env` in hand already.
/// Expected to behave the same as `elaborate_program` when `po_codegen` is
/// `true`.
pub fn elaborate_program_for_codegen(
    ns_env: Arc<namespace_env::Env>,
    path: &RelativePath,
    program: &mut nast::Program,
    opts: &CodegenOpts,
) -> Result<(), Vec1<NamingPhaseError>> {
    assert!(ns_env.is_codegen);
    let tco = TypecheckerOptions {
        po_codegen: true,
        po_disable_xhp_element_mangling: ns_env.disable_xhp_element_mangling,
        // Do not copy the auto_ns_map; it's not read in this crate except via
        // elaborate_namespaces_visitor, which uses the one in `ns_env` here
        ..Default::default()
    };
    elaborate_namespaces_visitor::elaborate_program(ns_env, program);
    let mut env = make_env(&tco, path);
    elaborate_common(&env, program);
    elaborate_package_expr(&env, program);
    elaborate_for_codegen(&env, program, opts);
    // Passes below here can emit errors
    typed_local::elaborate_program(&mut env, program, tco.po_codegen);
    lift_await::elaborate_program(&mut env, program, tco.po_codegen);
    let errs = env.into_errors();
    match Vec1::try_from_vec(errs) {
        Err(_) => Ok(()),
        Ok(v) => Err(v),
    }
}

pub fn elaborate_program(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    program: &mut nast::Program,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_program(ns_env(tco), program);
    let mut env = make_env(tco, path);
    elaborate_common(&env, program);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_program(&mut env, program);
    typed_local::elaborate_program(&mut env, program, false);
    lift_await::elaborate_program(&mut env, program, false);
    elaborate_for_typechecking(env, program)
}

pub fn elaborate_fun_def(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    f: &mut nast::FunDef,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_fun_def(ns_env(tco), f);
    let mut env = make_env(tco, path);
    elaborate_common(&env, f);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_fun_def(&mut env, f);
    typed_local::elaborate_fun_def(&mut env, f, false);
    lift_await::elaborate_fun_def(&mut env, f, false);
    elaborate_for_typechecking(env, f)
}

pub fn elaborate_class_(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    c: &mut nast::Class_,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_class_(ns_env(tco), c);
    let mut env = make_env(tco, path);
    elaborate_common(&env, c);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_class_(&mut env, c);
    typed_local::elaborate_class_(&mut env, c, false);
    lift_await::elaborate_class_(&mut env, c, false);
    elaborate_for_typechecking(env, c)
}

pub fn elaborate_module_def(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    m: &mut nast::ModuleDef,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_module_def(ns_env(tco), m);
    let mut env = make_env(tco, path);
    elaborate_common(&env, m);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_module_def(&mut env, m);
    elaborate_for_typechecking(env, m)
}

pub fn elaborate_gconst(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    c: &mut nast::Gconst,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_gconst(ns_env(tco), c);
    let mut env = make_env(tco, path);
    elaborate_common(&env, c);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_gconst(&mut env, c);
    elaborate_for_typechecking(env, c)
}

pub fn elaborate_typedef(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    t: &mut nast::Typedef,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_typedef(ns_env(tco), t);
    let mut env = make_env(tco, path);
    elaborate_common(&env, t);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_typedef(&mut env, t);
    elaborate_for_typechecking(env, t)
}

pub fn elaborate_stmt(
    tco: &TypecheckerOptions,
    path: &RelativePath,
    t: &mut nast::Stmt,
) -> Vec<NamingPhaseError> {
    elaborate_namespaces_visitor::elaborate_stmt(ns_env(tco), t);
    let mut env = make_env(tco, path);
    elaborate_common(&env, t);
    if tco.po_codegen {
        return env.into_errors();
    }
    lambda_captures::elaborate_stmt(&mut env, t);
    typed_local::elaborate_stmt(&mut env, t, false);
    lift_await::elaborate_stmt(&mut env, t, false);
    elaborate_for_typechecking(env, t)
}

fn ns_env(tco: &TypecheckerOptions) -> Arc<namespace_env::Env> {
    Arc::new(namespace_env::Env::empty(
        tco.po_auto_namespace_map.clone(),
        tco.po_codegen,
        tco.po_disable_xhp_element_mangling,
    ))
}

fn make_env(tco: &TypecheckerOptions, rel_path: &RelativePath) -> Env {
    let is_hhi = rel_path.is_hhi();
    let path = rel_path.path();

    let allow_module_declarations = tco.tco_allow_all_files_for_module_declarations
        || tco
            .tco_allowed_files_for_module_declarations
            .iter()
            .any(|spec| {
                !spec.is_empty()
                    && (spec.ends_with('*') && path.starts_with(&spec[..spec.len() - 1])
                        || path == std::path::Path::new(spec))
            });

    Env::new(
        tco,
        &ProgramSpecificOptions {
            is_hhi,
            allow_module_declarations,
        },
    )
}

/// Run the passes which are common to codegen and typechecking.
/// For now, these passes may not emit errors.
fn elaborate_common<T: Transform>(env: &Env, node: &mut T) {
    #[derive(Copy, Clone)]
    struct NoopPass;
    impl Pass for NoopPass {}
    #[rustfmt::skip]
    let mut passes = pass::Passes { passes:vec![
        Box::new(NoopPass)
    ]};

    node.transform(env, &mut passes);
    env.assert_no_errors();
}

fn elaborate_for_typechecking<T: Transform>(env: Env, node: &mut T) -> Vec<NamingPhaseError> {
    #[rustfmt::skip]
    let mut passes = pass::Passes { passes:vec![
        // Stop on `Invalid` expressions
        Box::<passes::guard_invalid::GuardInvalidPass>::default(),

        // -- Canonicalization passes -----------------------------------------

        // Remove top-level file attributes, noop and markup statements
        Box::<passes::elab_defs::ElabDefsPass>::default(),

        // Remove function bodies when in hhi mode
        Box::<passes::elab_func_body::ElabFuncBodyPass>::default(),

        // Flatten `Block` statements
        Box::<passes::elab_block::ElabBlockPass>::default(),

        // Strip `Hsoft` hints or replace with `Hlike`
        Box::<passes::elab_hint_hsoft::ElabHintHsoftPass>::default(),

        // Elaborate `Happly` to canonical representation, if any
        Box::<passes::elab_hint_happly::ElabHintHapplyPass>::default(),

        // Elaborate class identifier expressions (`CIexpr`) to canonical
        // representation: `CIparent`, `CIself`, `CIstatic`, `CI` _or_
        // `CIexpr (_,_, Lvar _ | This )`
        Box::<passes::elab_class_id::ElabClassIdPass>::default(),

        // Strip type parameters from type parameters when HKTs are not enabled
        Box::<passes::elab_hkt::ElabHktPass>::default(),

        // Elaborate `Collection` to `ValCollection` or `KeyValCollection`
        Box::<passes::elab_expr_collection::ElabExprCollectionPass>::default(),

        // Deduplicate user attributes
        Box::<passes::elab_user_attributes::ElabUserAttributesPass>::default(),

        // Replace import expressions with invalid expression marker
        Box::<passes::elab_expr_import::ElabExprImportPass>::default(),

        // Elaborate local variables to canonical representation
        Box::<passes::elab_expr_lvar::ElabExprLvarPass>::default(),

        // Warn of explicit use of builtin enum classes; make subtyping of
        // enum classes explicit
        Box::<passes::elab_enum_class::ElabEnumClassPass>::default(),

        // Elaborate class members & xhp attributes
        Box::<passes::elab_class_vars::ElabClassVarsPass>::default(),

        // Elaborate special function calls to canonical representation, if any
        Box::<passes::validate_expr_call_echo::ValidateExprCallEchoPass>::default(),
        Box::<passes::elab_expr_call_call_user_func::ElabExprCallCallUserFuncPass>::default(),
        Box::<passes::elab_expr_call_hh_meth_caller::ElabExprCallHhMethCallerPass>::default(),

        // Elaborate invariant calls to canonical representation
        Box::<passes::elab_expr_call_hh_invariant::ElabExprCallHhInvariantPass>::default(),

        // -- Mark invalid hints and expressions & miscellaneous validation ---

        // Replace invalid uses of `void` and `noreturn` with `Herr`
        Box::<passes::elab_hint_retonly::ElabHintRetonlyPass>::default(),

        // Replace invalid uses of wildcard hints with `Herr`
        Box::<passes::elab_hint_wildcard::ElabHintWildcardPass>::default(),

        // Replace uses to `self` in shape field names with referenced class
        Box::<passes::elab_shape_field_name::ElabShapeFieldNamePass>::default(),

        // Replace invalid uses of `this` hints with `Herr`
        Box::<passes::elab_hint_this::ElabHintThisPass>::default(),

        // Replace invalid `Haccess` root hints with `Herr`
        Box::<passes::elab_hint_haccess::ElabHintHaccessPass>::default(),

        // Replace empty `Tuple`s with invalid expression marker
        Box::<passes::elab_expr_tuple::ElabExprTuplePass>::default(),

        // Validate / replace invalid uses of dynamic classes in `New` and `Class_get`
        // expressions
        Box::<passes::elab_dynamic_class_name::ElabDynamicClassNamePass>::default(),

        // Replace non-constant class or global constant with invalid expression marker
        Box::<passes::elab_const_expr::ElabConstExprPass>::default(),

        // Replace malformed key / value bindings in as expressions with invalid
        // local var markers
        Box::<passes::elab_as_expr::ElabAsExprPass>::default(),

        // Validate hints used in `Cast` expressions
        Box::<passes::validate_expr_cast::ValidateExprCastPass>::default(),

        // Validate where `dynamic` can be used in a hint
        Box::<passes::validate_dynamic_hint::ValidateDynamicHintPass>::default(),

        // Check for duplicate function parameter names
        Box::<passes::validate_fun_params::ValidateFunParamsPass>::default(),

        // Validate use of `require implements`, `require extends` and
        // `require class` declarations for traits, interfaces and classes
        Box::<passes::validate_class_req::ValidateClassReqPass>::default(),

        // Validation dealing with common xhp naming errors
        Box::<passes::validate_xhp_name::ValidateXhpNamePass>::default(),

        // -- Elaboration & validation under typechecker options --------------

        // Add `supportdyn` and `Like` wrappers everywhere - under `everything-sdt`
        // typechecker option
        Box::<passes::elab_everything_sdt::ElabEverythingSdtPass>::default(),

        // Validate use of `Hlike` hints - depends on `enable-like-type-hints`
        // and `everything_sdt` typechecker options
        Box::<passes::validate_like_hint::ValidateLikeHintPass>::default(),

        // Validate constructors under
        // `consistent-explicit_consistent_constructors` typechecker option
        Box::<passes::validate_class_consistent_construct::ValidateClassConsistentConstructPass>::default(),

        // Validate  use of `SupportDyn` class - depends on `enable-supportdyn`
        // and `everything_sdt` typechecker options
        Box::<passes::validate_supportdyn::ValidateSupportDynPass>::default(),

        // Validate use of module definitions - depends on:
        // - `allow_all_files_for_module_declarations`
        // - `allowed_files_for_module_declarations`
        // typechecker options
        Box::<passes::validate_module::ValidateModulePass>::default(),

        // Validate that there are no top-level statements
        Box::<passes::validate_toplevel_statement::ValidateToplevelStatementPass>::default(),

        // // -- Old 'NAST checks' ------------------------------------------------

        // Validate use of the `__Const` attribute on classes - depends on
        // `const_attribute` typechecker option
        Box::<passes::validate_class_user_attribute_const::ValidateClassUserAttributeConstPass>::default(),

        // Validate use of the `__Const` attribute on static class vars - depends
        // on the `const_static_props` typechecker option
        Box::<passes::validate_class_var_user_attribute_const::ValidateClassVarUserAttributeConstPass>::default(),
        Box::<passes::validate_class_var_user_attribute_lsb::ValidateClassVarUserAttributeLsbPass>::default(),

        // Validate `inout` `FunParam`s ensuring they are not used in functions with
        // special semantics or in memoized functions
        Box::<passes::validate_fun_param_inout::ValidateFunParamInoutPass>::default(),

        // Validate use of `Await` in sync functions and return in generators.
        Box::<passes::validate_coroutine::ValidateCoroutinePass>::default(),

        // Checks for the presence of a function body in methods, use of traits
        // and instance and static member variables in an interface definition
        Box::<passes::validate_interface::ValidateInterfacePass>::default(),

        // Checks for use of reserved names in functions, methods, class identifiers
        // and class constants
        Box::<passes::validate_illegal_name::ValidateIllegalNamePass>::default(),

        Box::<passes::validate_control_context::ValidateControlContextPass>::default(),

        Box::<passes::validate_class_tparams::ValidateClassTparamsPass>::default(),

        Box::<passes::validate_user_attribute_dynamically_callable::ValidateUserAttributeDynamicallyCallable>::default(),

        Box::<passes::validate_hint_habstr::ValidateHintHabstrPass>::default(),

        Box::<passes::validate_class_methods::ValidateClassMethodsPass>::default(),

        Box::<passes::validate_global_const::ValidateGlobalConstPass>::default(),

        Box::<passes::validate_class_member::ValidateClassMemberPass>::default(),

        Box::<passes::validate_shape_name::ValidateShapeNamePass>::default(),

        Box::<passes::validate_php_lambda::ValidatePhpLambdaPass>::default(),

        Box::<passes::validate_xhp_attribute::ValidateXhpAttributePass>::default(),
        Box::<passes::validate_user_attribute_arity::ValidateUserAttributeArityPass>::default(),
        Box::<passes::validate_user_attribute_deprecated_static::ValidateUserAttributeDeprecatedStaticPass>::default(),
        Box::<passes::validate_user_attribute_autocomplete_sort_text_static::ValidateUserAttributeAutocompleteSortTextStaticPass>::default(),
        Box::<passes::validate_user_attribute_entry_point::ValidateUserAttributeEntryPointPass>::default(),
        Box::<passes::validate_user_attribute_no_auto_dynamic::ValidateUserAttributeNoAutoDynamic>::default(),
        Box::<passes::validate_user_attribute_memoize::ValidateUserAttributeMemoizePass>::default(),
        Box::<passes::validate_user_attribute_soft_internal::ValidateUserAttributeSoftInternalPass>::default(),

        Box::<passes::validate_method_private_final::ValidateMethodPrivateFinalPass>::default(),

        Box::<passes::validate_trait_internal::ValidateTraitInternalPass>::default(),

        Box::<passes::validate_hint_hrefinement::ValidateHintHrefinementPass>::default(),

        Box::<passes::validate_expr_function_pointer::ValidateExprFunctionPointerPass>::default(),

        Box::<passes::validate_expr_array_get::ValidateExprArrayGetPass>::default(),

        Box::<passes::validate_expr_list::ValidateExprListPass>::default(),

        Box::<passes::validate_like_hint::ValidateLikeHintPass>::default(),
    ]};

    node.transform(&env, &mut passes);
    env.into_errors()
}

fn elaborate_package_expr<T: Transform>(env: &Env, node: &mut T) {
    #[rustfmt::skip]
    let mut passes = pass::Passes { passes: vec![
            Box::<passes::elab_expr_package::ElabExprPackagePass>::default()
    ]};
    node.transform(env, &mut passes);
    env.assert_no_errors();
}

fn elaborate_for_codegen<T: Transform>(env: &Env, node: &mut T, opts: &CodegenOpts) {
    let mut passes: Vec<Box<dyn Pass>> = Vec::new();

    if opts.textual_remove_memoize {
        passes.push(Box::<passes::remove_memo_attr::RemoveMemoAttr>::default());
    }
    // Insert assertion that the package being cross-package accessed is loaded
    passes.push(Box::<passes::elab_cross_package::ElabCrossPackagePass>::default());

    let mut passes = pass::Passes { passes };
    node.transform(env, &mut passes);
    env.assert_no_errors();
}
