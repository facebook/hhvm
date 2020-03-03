// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d97ff619d548baa814154c9a73e2556c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use crate::internal_type_set as i_ty_set;

pub type LoclTy = typing_defs::Ty;

pub type LocalIdSetT = local_id::set::Set;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct LocalEnv {
    pub per_cont_env: typing_per_cont_env::TypingPerContEnv,
    pub local_mutability: typing_mutability_env::MutabilityEnv,
    pub local_reactive: Reactivity,
    pub local_using_vars: LocalIdSetT,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, PartialEq, PartialOrd, Serialize)]
pub struct Env {
    pub function_pos: pos::Pos,
    pub fresh_typarams: s_set::SSet,
    pub lenv: LocalEnv,
    pub genv: Genv,
    pub decl_env: decl_env::Env,
    pub in_loop: bool,
    pub in_try: bool,
    pub in_case: bool,
    pub inside_constructor: bool,
    pub inside_ppl_class: bool,
    pub global_tpenv: type_parameter_env::TypeParameterEnv,
    pub log_levels: s_map::SMap<isize>,
    pub inference_env: typing_inference_env::TypingInferenceEnv,
    pub allow_wildcards: bool,
    pub big_envs: std::cell::RefCell<Vec<(pos::Pos, Box<Env>)>>,
    pub pessimize: bool,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, PartialEq, PartialOrd, Serialize)]
pub struct Genv {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_: typing_env_return_info::TypingEnvReturnInfo,
    pub params: local_id::map::Map<(Ty, ParamMode)>,
    pub condition_types: s_map::SMap<Ty>,
    pub parent: Option<(String, Ty)>,
    pub self_: Option<(String, Ty)>,
    pub static_: bool,
    pub fun_kind: ast_defs::FunKind,
    pub val_kind: typing_defs::ValKind,
    pub fun_mutable: Option<ParamMutability>,
    pub file: relative_path::RelativePath,
}
