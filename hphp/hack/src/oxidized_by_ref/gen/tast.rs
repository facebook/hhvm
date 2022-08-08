// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<10b1e4a0937cd11573e318f179a9f84b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use aast_defs::*;
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::PossiblyEnforcedTy;
pub use typing_defs::Ty;
pub use typing_defs::ValKind;

#[allow(unused_imports)]
use crate::*;

pub type DeclTy<'a> = typing_defs::Ty<'a>;

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
#[rust_to_ocaml(prefix = "has_")]
#[repr(C)]
pub struct FunTastInfo {
    pub implicit_return: bool,
    /// True if there are leaves of the function's imaginary CFG without a return statement
    pub readonly: bool,
}
impl TrivialDrop for FunTastInfo {}
arena_deserializer::impl_deserialize_in_arena!(FunTastInfo);

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
pub struct SavedEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tcopt: &'a typechecker_options::TypecheckerOptions<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub inference_env: &'a typing_inference_env::TypingInferenceEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub condition_types: s_map::SMap<'a, &'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun_tast_info: Option<&'a FunTastInfo>,
}
impl<'a> TrivialDrop for SavedEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SavedEnv<'arena>);

pub type Program<'a> = aast::Program<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Def<'a> = aast::Def<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Expr<'a> = aast::Expr<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Expr_<'a> = aast::Expr_<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Stmt<'a> = aast::Stmt<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Stmt_<'a> = aast::Stmt_<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Case<'a> = aast::Case<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Block<'a> = aast::Block<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Class_<'a> = aast::Class_<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type ClassId<'a> = aast::ClassId<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type TypeHint<'a> = aast::TypeHint<'a, &'a Ty<'a>>;

pub type Targ<'a> = aast::Targ<'a, &'a Ty<'a>>;

pub type ClassGetExpr<'a> = aast::ClassGetExpr<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type ClassTypeconstDef<'a> = aast::ClassTypeconstDef<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type UserAttribute<'a> = aast::UserAttribute<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Fun_<'a> = aast::Fun_<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type FileAttribute<'a> = aast::FileAttribute<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type FunDef<'a> = aast::FunDef<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type FunParam<'a> = aast::FunParam<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type FuncBody<'a> = aast::FuncBody<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Method_<'a> = aast::Method_<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type ClassVar<'a> = aast::ClassVar<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type ClassConst<'a> = aast::ClassConst<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Tparam<'a> = aast::Tparam<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Typedef<'a> = aast::Typedef<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type Gconst<'a> = aast::Gconst<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;

pub type ModuleDef<'a> = aast::ModuleDef<'a, &'a Ty<'a>, &'a SavedEnv<'a>>;
