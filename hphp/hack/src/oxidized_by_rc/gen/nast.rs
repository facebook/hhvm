// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<fc7ac3275fc61b6a9ab0425696785786>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use aast::ClassHint;
pub use aast::PuCaseValue;
pub use aast::RecordHint;
pub use aast::Sid;
pub use aast::TraitHint;
pub use aast::XhpAttrHint;
pub use ast_defs::ShapeFieldName;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum FuncBodyAnn {
    Named,
    NamedWithUnsafeBlocks,
    Unnamed(namespace_env::Env),
}

pub type Program = aast::Program<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Def = aast::Def<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Expr = aast::Expr<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Expr_ = aast::Expr_<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Stmt = aast::Stmt<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Block = aast::Block<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type UserAttribute = aast::UserAttribute<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type ClassId_ = aast::ClassId_<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Class_ = aast::Class_<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Method_ = aast::Method_<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type FileAttribute = aast::FileAttribute<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Fun_ = aast::Fun_<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type FunDef = aast::FunDef<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type FuncBody = aast::FuncBody<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type FunParam = aast::FunParam<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type FunVariadicity = aast::FunVariadicity<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Typedef = aast::Typedef<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type RecordDef = aast::RecordDef<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Tparam = aast::Tparam<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Gconst = aast::Gconst<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type ClassId = aast::ClassId<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Catch = aast::Catch<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Case = aast::Case<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Field = aast::Field<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Afield = aast::Afield<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type PuEnum = aast::PuEnum<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type PuMember = aast::PuMember<std::rc::Rc<pos::Pos>, FuncBodyAnn, (), ()>;

pub type Targ = aast::Targ<()>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct IgnoreAttributeEnv {
    pub ignored_attributes: Vec<std::rc::Rc<String>>,
}
