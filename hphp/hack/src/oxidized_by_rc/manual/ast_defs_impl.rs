// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;
use std::rc::Rc;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Id(pub Rc<Pos>, pub Rc<String>);

pub type Pstring = (Rc<Pos>, Rc<String>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ShapeFieldName {
    SFlitInt(Pstring),
    SFlitStr(Pstring),
    SFclassConst(Id, Pstring),
}

pub use oxidized::ast_defs::Variance;

pub use oxidized::ast_defs::ConstraintKind;

pub use oxidized::ast_defs::Reified;

pub use oxidized::ast_defs::ClassKind;

pub use oxidized::ast_defs::ParamKind;

pub use oxidized::ast_defs::OgNullFlavor;

pub use oxidized::ast_defs::FunKind;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Bop {
    Plus,
    Minus,
    Star,
    Slash,
    Eqeq,
    Eqeqeq,
    Starstar,
    Diff,
    Diff2,
    Ampamp,
    Barbar,
    LogXor,
    Lt,
    Lte,
    Gt,
    Gte,
    Dot,
    Amp,
    Bar,
    Ltlt,
    Gtgt,
    Percent,
    Xor,
    Cmp,
    QuestionQuestion,
    Eq(Option<Rc<Bop>>),
}

pub use oxidized::ast_defs::Uop;
