// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast;
use crate::pos::Pos;

pub use crate::ast_defs::shape_map;

pub use aast::Sid;

pub type Afield = aast::Afield<Pos, (), (), ()>;
pub type Block = aast::Block<Pos, (), (), ()>;
pub type Case = aast::Case<Pos, (), (), ()>;
pub type Catch = aast::Catch<Pos, (), (), ()>;
pub type Class_ = aast::Class_<Pos, (), (), ()>;
pub type ClassId = aast::ClassId<Pos, (), (), ()>;
pub type ClassId_ = aast::ClassId_<Pos, (), (), ()>;
pub type Def = aast::Def<Pos, (), (), ()>;
pub type Expr = aast::Expr<Pos, (), (), ()>;
pub type Expr_ = aast::Expr_<Pos, (), (), ()>;
pub type Field = aast::Field<Pos, (), (), ()>;
pub type Fun_ = aast::Fun_<Pos, (), (), ()>;
pub type FuncBody = aast::FuncBody<Pos, (), (), ()>;
pub type FunDef = aast::FunDef<Pos, (), (), ()>;
pub type FunParam = aast::FunParam<Pos, (), (), ()>;
pub type FunVariadicity = aast::FunVariadicity<Pos, (), (), ()>;
pub type Gconst = aast::Gconst<Pos, (), (), ()>;
pub type Method_ = aast::Method_<Pos, (), (), ()>;
pub type MethodRedeclaration = aast::MethodRedeclaration<Pos, (), (), ()>;
pub type Program = aast::Program<Pos, (), (), ()>;
pub type RecordDef = aast::RecordDef<Pos, (), (), ()>;
pub type Stmt = aast::Stmt<Pos, (), (), ()>;
pub type Targ = aast::Targ<()>;
pub type Tparam = aast::Tparam<Pos, (), (), ()>;
pub type Typedef = aast::Typedef<Pos, (), (), ()>;
pub type UserAttribute = aast::UserAttribute<Pos, (), (), ()>;
