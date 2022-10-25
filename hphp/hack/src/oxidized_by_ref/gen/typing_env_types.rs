// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<bf380db8a4fb084540aa8141062aa388>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;

#[allow(unused_imports)]
use crate::*;

pub type LoclTy<'a> = typing_defs::Ty<'a>;

pub type LocalIdSetT<'a> = local_id::set::Set<'a>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct LocalEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub per_cont_env: &'a typing_per_cont_env::TypingPerContEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub local_using_vars: &'a LocalIdSetT<'a>,
}
impl<'a> TrivialDrop for LocalEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(LocalEnv<'arena>);

/// See the .mli file for the documentation of fields.
#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Env<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fresh_typarams: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lenv: &'a LocalEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub genv: &'a Genv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decl_env: &'a decl_env::Env<'a>,
    pub in_loop: bool,
    pub in_try: bool,
    pub in_expr_tree: bool,
    pub inside_constructor: bool,
    pub in_support_dynamic_type_method_check: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tracing_info: Option<&'a decl_counters::TracingInfo<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub log_levels: s_map::SMap<'a, isize>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub inference_env: &'a typing_inference_env::TypingInferenceEnv<'a>,
    pub allow_wildcards: bool,
    #[serde(skip)]
    pub big_envs: std::cell::Cell<&'a [(&'a pos::Pos<'a>, &'a Env<'a>)]>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun_tast_info: Option<&'a tast::FunTastInfo>,
}
impl<'a> TrivialDrop for Env<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Env<'arena>);

/// See the .mli file for the documentation of fields.
#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Genv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tcopt: &'a typechecker_options::TypecheckerOptions<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub callable_pos: &'a pos::Pos<'a>,
    pub readonly: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub return_: &'a typing_env_return_info::TypingEnvReturnInfo<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: local_id::map::Map<'a, (&'a Ty<'a>, &'a pos::Pos<'a>, Option<&'a Ty<'a>>)>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub condition_types: s_map::SMap<'a, &'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub parent: Option<&'a (&'a str, &'a Ty<'a>)>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub self_: Option<&'a (&'a str, &'a Ty<'a>)>,
    pub static_: bool,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub val_kind: typing_defs::ValKind,
    pub fun_is_ctor: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file: &'a relative_path::RelativePath<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub current_module: Option<ast_defs::Id<'a>>,
    pub this_internal: bool,
    pub this_support_dynamic_type: bool,
}
impl<'a> TrivialDrop for Genv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Genv<'arena>);
