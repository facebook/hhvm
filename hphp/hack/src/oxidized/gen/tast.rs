// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<28602011905a4ea63f86636f48fdd5d9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use aast_defs::*;
pub use typing_defs::PossiblyEnforcedTy;
pub use typing_defs::Reactivity;
pub use typing_defs::Ty;
pub use typing_defs::ValKind;
pub use typing_mutability_env::MutabilityEnv;

pub type DeclTy = typing_defs::Ty;

pub type TypeParamMutability = typing_defs::ParamMutability;

#[derive(
    Clone,
    Debug,
    Default,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct SavedEnv {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub inference_env: typing_inference_env::TypingInferenceEnv,
    pub tpenv: type_parameter_env::TypeParameterEnv,
    pub reactivity: Reactivity,
    pub local_mutability: MutabilityEnv,
    pub fun_mutable: Option<TypeParamMutability>,
    pub condition_types: s_map::SMap<Ty>,
    pub pessimize: bool,
}

#[derive(
    Clone,
    Copy,
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
pub enum FuncBodyAnn {
    HasUnsafeBlocks,
    NoUnsafeBlocks,
}

pub type Program = aast::Program<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Def = aast::Def<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Expr = aast::Expr<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Expr_ = aast::Expr_<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Stmt = aast::Stmt<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Block = aast::Block<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Class_ = aast::Class_<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type ClassId = aast::ClassId<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type TypeHint = aast::TypeHint<Ty>;

pub type Targ = aast::Targ<Ty>;

pub type ClassGetExpr = aast::ClassGetExpr<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type ClassTypeconst = aast::ClassTypeconst<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type UserAttribute = aast::UserAttribute<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Fun_ = aast::Fun_<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type FileAttribute = aast::FileAttribute<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type FunDef = aast::FunDef<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type FunParam = aast::FunParam<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type FuncBody = aast::FuncBody<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Method_ = aast::Method_<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type MethodRedeclaration = aast::MethodRedeclaration<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type ClassVar = aast::ClassVar<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type ClassTparams = aast::ClassTparams<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type ClassConst = aast::ClassConst<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Tparam = aast::Tparam<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Typedef = aast::Typedef<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type RecordDef = aast::RecordDef<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type Gconst = aast::Gconst<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;

pub type PuEnum = aast::PuEnum<(pos::Pos, Ty), FuncBodyAnn, SavedEnv, Ty>;
