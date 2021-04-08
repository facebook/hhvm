// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<dbb40af8d431faf469f17df4b13b470a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub type LoclTy<'a> = typing_defs::Ty<'a>;

pub type LocalIdSetT<'a> = local_id::set::Set<'a>;

#[derive(
    Clone,
    Debug,
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
pub struct LocalEnv<'a> {
    pub per_cont_env: &'a typing_per_cont_env::TypingPerContEnv<'a>,
    pub local_using_vars: &'a LocalIdSetT<'a>,
}
impl<'a> TrivialDrop for LocalEnv<'a> {}

#[derive(
    Clone,
    Debug,
    FromOcamlRepIn,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Env<'a> {
    pub function_pos: &'a pos::Pos<'a>,
    pub fresh_typarams: s_set::SSet<'a>,
    pub lenv: &'a LocalEnv<'a>,
    pub genv: &'a Genv<'a>,
    pub decl_env: &'a decl_env::Env<'a>,
    pub in_loop: bool,
    pub in_try: bool,
    pub in_case: bool,
    pub in_expr_tree: bool,
    pub inside_constructor: bool,
    pub tracing_info: Option<&'a decl_counters::TracingInfo<'a>>,
    pub global_tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
    pub log_levels: s_map::SMap<'a, isize>,
    pub inference_env: &'a typing_inference_env::TypingInferenceEnv<'a>,
    pub allow_wildcards: bool,
    pub big_envs: std::cell::Cell<&'a [(&'a pos::Pos<'a>, &'a Env<'a>)]>,
    pub pessimize: bool,
    pub fun_tast_info: Option<&'a tast::FunTastInfo>,
}
impl<'a> TrivialDrop for Env<'a> {}

#[derive(
    Clone,
    Debug,
    FromOcamlRepIn,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Genv<'a> {
    pub tcopt: &'a typechecker_options::TypecheckerOptions<'a>,
    pub return_: &'a typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: local_id::map::Map<'a, (&'a Ty<'a>, &'a pos::Pos<'a>, ParamMode)>,
    pub condition_types: s_map::SMap<'a, &'a Ty<'a>>,
    pub parent: Option<&'a (&'a str, &'a Ty<'a>)>,
    pub self_: Option<&'a (&'a str, &'a Ty<'a>)>,
    pub static_: bool,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub val_kind: typing_defs::ValKind,
    pub fun_is_ctor: bool,
    pub file: &'a relative_path::RelativePath<'a>,
}
impl<'a> TrivialDrop for Genv<'a> {}
