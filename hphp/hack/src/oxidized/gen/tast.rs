// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8213fc6d15e08d60fab83f2a2e972dfa>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use aast_defs::*;
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::Ty;
pub use typing_defs::ValKind;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving show")]
pub type DeclTy = typing_defs::Ty;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (hash, show)")]
#[rust_to_ocaml(prefix = "has_")]
#[repr(C)]
pub struct FunTastInfo {
    pub implicit_return: bool,
    /// True if there are leaves of the function's imaginary CFG without a return statement
    pub readonly: bool,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (hash, show)")]
#[repr(u8)]
pub enum CheckStatus {
    /// The definition is checked only once.
    COnce,
    /// The definition is checked twice and this is the check under normal
    /// assumptions that is using the parameter and return types that are
    /// written in the source code (but potentially implicitly pessimised).
    CUnderNormalAssumptions,
    /// The definition is checked twice and this is the check under dynamic
    /// assumptions that is using the dynamic type for parameters and return.
    CUnderDynamicAssumptions,
}
impl TrivialDrop for CheckStatus {}
arena_deserializer::impl_deserialize_in_arena!(CheckStatus);

#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (hash, show)")]
#[repr(C)]
pub struct SavedEnv {
    #[rust_to_ocaml(attr = "opaque")]
    #[rust_to_ocaml(attr = "hash.ignore")]
    pub tcopt: typechecker_options::TypecheckerOptions,
    #[rust_to_ocaml(attr = "opaque")]
    #[rust_to_ocaml(attr = "hash.ignore")]
    pub inference_env: typing_inference_env::TypingInferenceEnv,
    pub tpenv: type_parameter_env::TypeParameterEnv,
    pub fun_tast_info: Option<FunTastInfo>,
    /// Indicates how many types the callable was checked and under what
    /// assumptions.
    pub checked: CheckStatus,
}

#[rust_to_ocaml(attr = "deriving show")]
pub type Program = aast::Program<Ty, SavedEnv>;

#[rust_to_ocaml(attr = "deriving (hash, show)")]
pub type Def = aast::Def<Ty, SavedEnv>;

#[rust_to_ocaml(attr = "deriving (hash, show)")]
pub type DefWithDynamic = tast_with_dynamic::TastWithDynamic<Def>;

pub type Expr = aast::Expr<Ty, SavedEnv>;

pub type Expr_ = aast::Expr_<Ty, SavedEnv>;

pub type Stmt = aast::Stmt<Ty, SavedEnv>;

pub type Stmt_ = aast::Stmt_<Ty, SavedEnv>;

pub type Case = aast::Case<Ty, SavedEnv>;

pub type Block = aast::Block<Ty, SavedEnv>;

pub type Class_ = aast::Class_<Ty, SavedEnv>;

pub type ClassId = aast::ClassId<Ty, SavedEnv>;

pub type TypeHint = aast::TypeHint<Ty>;

pub type Targ = aast::Targ<Ty>;

pub type ClassGetExpr = aast::ClassGetExpr<Ty, SavedEnv>;

pub type ClassTypeconstDef = aast::ClassTypeconstDef<Ty, SavedEnv>;

pub type UserAttribute = aast::UserAttribute<Ty, SavedEnv>;

pub type CaptureLid = aast::CaptureLid<Ty>;

pub type Fun_ = aast::Fun_<Ty, SavedEnv>;

pub type Efun = aast::Efun<Ty, SavedEnv>;

pub type FileAttribute = aast::FileAttribute<Ty, SavedEnv>;

pub type FunDef = aast::FunDef<Ty, SavedEnv>;

pub type FunParam = aast::FunParam<Ty, SavedEnv>;

pub type FuncBody = aast::FuncBody<Ty, SavedEnv>;

pub type Method_ = aast::Method_<Ty, SavedEnv>;

pub type ClassVar = aast::ClassVar<Ty, SavedEnv>;

pub type ClassConst = aast::ClassConst<Ty, SavedEnv>;

pub type Tparam = aast::Tparam<Ty, SavedEnv>;

pub type Typedef = aast::Typedef<Ty, SavedEnv>;

pub type Gconst = aast::Gconst<Ty, SavedEnv>;

pub type ModuleDef = aast::ModuleDef<Ty, SavedEnv>;

pub type CallExpr = aast::CallExpr<Ty, SavedEnv>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct ByNames {
    pub fun_tasts: s_map::SMap<tast_with_dynamic::TastWithDynamic<Def>>,
    pub class_tasts: s_map::SMap<tast_with_dynamic::TastWithDynamic<Def>>,
    pub typedef_tasts: s_map::SMap<Def>,
    pub gconst_tasts: s_map::SMap<Def>,
    pub module_tasts: s_map::SMap<Def>,
}
