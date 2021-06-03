// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8fb3de0de4be103be74032163ccbb84c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use aast_defs::*;
pub use typing_defs::PossiblyEnforcedTy;
pub use typing_defs::Ty;
pub use typing_defs::ValKind;

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
pub struct FunTastInfo {
    /// True if there are leaves of the function's imaginary CFG without a return statement
    pub has_implicit_return: bool,
    /// Result of {!Nast.named_body_is_unsafe}
    pub named_body_is_unsafe: bool,
}
impl TrivialDrop for FunTastInfo {}
arena_deserializer::impl_deserialize_in_arena!(FunTastInfo);

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
pub struct SavedEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tcopt: &'a typechecker_options::TypecheckerOptions<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub inference_env: &'a typing_inference_env::TypingInferenceEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub condition_types: s_map::SMap<'a, &'a Ty<'a>>,
    pub pessimize: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun_tast_info: Option<&'a FunTastInfo>,
}
impl<'a> TrivialDrop for SavedEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SavedEnv<'arena>);

pub type Program<'a> =
    aast::Program<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Def<'a> = aast::Def<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Expr<'a> =
    aast::Expr<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Expr_<'a> =
    aast::Expr_<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Stmt<'a> =
    aast::Stmt<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Block<'a> =
    aast::Block<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Class_<'a> =
    aast::Class_<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type ClassId<'a> =
    aast::ClassId<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type TypeHint<'a> = aast::TypeHint<'a, &'a Ty<'a>>;

pub type Targ<'a> = aast::Targ<'a, &'a Ty<'a>>;

pub type ClassGetExpr<'a> =
    aast::ClassGetExpr<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type ClassTypeconstDef<'a> =
    aast::ClassTypeconstDef<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type UserAttribute<'a> =
    aast::UserAttribute<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Fun_<'a> =
    aast::Fun_<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type FileAttribute<'a> =
    aast::FileAttribute<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type FunDef<'a> =
    aast::FunDef<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type FunParam<'a> =
    aast::FunParam<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type FunVariadicity<'a> =
    aast::FunVariadicity<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type FuncBody<'a> =
    aast::FuncBody<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Method_<'a> =
    aast::Method_<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type ClassVar<'a> =
    aast::ClassVar<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type ClassConst<'a> =
    aast::ClassConst<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Tparam<'a> =
    aast::Tparam<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Typedef<'a> =
    aast::Typedef<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type RecordDef<'a> =
    aast::RecordDef<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;

pub type Gconst<'a> =
    aast::Gconst<'a, (&'a pos::Pos<'a>, &'a Ty<'a>), (), &'a SavedEnv<'a>, &'a Ty<'a>>;
