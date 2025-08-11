// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<655afbef47fe06198a4abb4e5568520d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct ParserOptions {
    /// Flag to disable using lvals as expressions.
    pub disable_lval_as_an_expression: bool,
    /// Enable const static properties
    pub const_static_props: bool,
    /// Statically check default function arguments
    pub const_default_func_args: bool,
    /// Static properties can be abstract
    pub abstract_static_props: bool,
    /// Flag to disallow HH\fun and HH\class_meth in constants and constant initializers
    pub disallow_func_ptrs_in_constants: bool,
    /// Enable the new style xhp class.
    /// Old style: class :name {}
    /// New style: xhp class name {}
    pub enable_xhp_class_modifier: bool,
    /// Flag to disable the old stype xhp element mangling. `<something/>` would otherwise be resolved as `xhp_something`
    /// The new style `xhp class something {}` does not do this style of mangling, thus we need a way to disable it on the
    /// 'lookup side'.
    pub disable_xhp_element_mangling: bool,
    /// Allows enabling unstable features via the __EnableUnstableFeatures attribute
    pub allow_unstable_features: bool,
    pub hhvm_compat_mode: bool,
    pub hhi_mode: bool,
    /// Are we emitting bytecode?
    pub codegen: bool,
    /// Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute.
    pub disable_legacy_soft_typehints: bool,
    /// Disable <<...>> attribute syntax
    pub disable_legacy_attribute_syntax: bool,
    /// Disable `children (foo|bar+|pcdata)` declarations as they can be implemented without special syntax
    pub disable_xhp_children_declarations: bool,
    /// Statically check default lambda arguments. Subset of default_func_args
    pub const_default_lambda_args: bool,
    /// <<__Soft>> T -> ~T
    pub interpret_soft_types_as_like_types: bool,
    /// Enable features used to typecheck systemlib
    pub is_systemlib: bool,
    pub disallow_static_constants_in_default_func_args: bool,
    /// Namespace aliasing map
    pub auto_namespace_map: Vec<(String, String)>,
    /// All classes are implcitly marked <<__SupportDynamicType>>
    pub everything_sdt: bool,
    /// Parse all user attributes rather than only the ones needed for typing
    pub keep_user_attributes: bool,
    /// Stack size to use for parallel workers inside the parser.
    pub stack_size: isize,
    /// Flag for disabling functions in HHI files with the __PHPStdLib attribute
    pub deregister_php_stdlib: bool,
    /// Enables union and intersection type hints
    pub union_intersection_type_hints: bool,
    /// Replace concurrent blocks with their bodies in the AST
    pub unwrap_concurrent: bool,
    /// Flag to disable the error suppression operator
    pub disallow_silence: bool,
    /// Disable parser-based readonly checking
    pub no_parser_readonly_check: bool,
    /// Disable HH_IGNORE_ERROR comments, either raising an error if 1 or treating them as normal comments if 2.
    pub disable_hh_ignore_error: isize,
    /// Set of error codes disallowed in decl positions
    pub allowed_decl_fixme_codes: i_set::ISet,
    /// Ignore the experimental_features and consider_unspecified_experimental_features_released config
    /// options and use a hard coded function instead
    pub use_legacy_experimental_feature_config: bool,
    /// A mapping of names of experimental features to their status: Unstable/Preview/OngoingRelease
    pub experimental_features: s_map::SMap<experimental_features::FeatureStatus>,
    /// Any experimental features not specified in the experimental_features configuration field should
    /// default to OngoingRelease if this is true. Otherwise they will default to Unstable. This should be true for
    /// testing and tools that don't read .hhconfig (e.g., like hh_single_type_check and hh_parse). It should
    /// be false for hh_server.
    pub consider_unspecified_experimental_features_released: bool,
    /// Whether PackageV2 is enabled.
    pub package_v2: bool,
    /// Information used to determine which package a file belongs to during typechecking.
    pub package_info: package_info::PackageInfo,
    /// Option for the package v2 to strip the multifile filename mangling used in Hack tests.
    /// Should be set to true only by the unit tests in the Hack test suite
    pub package_v2_support_multifile_tests: bool,
    /// When false, type hint class<T> (Hclass_ptr) becomes decl ty classname<T> (Tnewtype).
    /// When true, it becomes decl ty class<T> (Tclass_ptr). This option is similar to the
    /// interpret_soft_types_as_like_types switch.
    pub enable_class_pointer_hint: bool,
    /// When true, plain <<__Memoize>> will not be allowed.
    pub disallow_non_annotated_memoize: bool,
    /// When true, plain <<__Memoize>> will be treated as <<__Memoize(#KeyedByIC)>>.
    pub treat_non_annotated_memoize_as_kbic: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct FfiT(
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub s_map::SMap<experimental_features::FeatureStatus>,
    pub bool,
    pub bool,
);
