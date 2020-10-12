// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<081f9fc88882b536ae90d5d34857deb0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use crate::internal_type_set as i_ty_set;

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
    pub local_mutability: typing_mutability_env::MutabilityEnv<'a>,
    pub local_reactive: &'a Reactivity<'a>,
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
    pub inside_constructor: bool,
    pub et_spliced_types: Option<i_map::IMap<'a, (&'a tast::Expr<'a>, &'a typing_defs::Ty<'a>)>>,
    pub global_tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
    pub log_levels: s_map::SMap<'a, isize>,
    pub inference_env: &'a typing_inference_env::TypingInferenceEnv<'a>,
    pub allow_wildcards: bool,
    pub big_envs: std::cell::Cell<&'a [(&'a pos::Pos<'a>, &'a Env<'a>)]>,
    pub pessimize: bool,
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
    pub fun_mutable: Option<ParamMutability>,
    pub file: &'a relative_path::RelativePath<'a>,
}
impl<'a> TrivialDrop for Genv<'a> {}
