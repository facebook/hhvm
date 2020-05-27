// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8b17a71d2f52261a904259e429950673>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Id(pub std::rc::Rc<Pos>, pub std::rc::Rc<String>);

pub type Pstring = (std::rc::Rc<Pos>, std::rc::Rc<String>);

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
    Eq(Option<std::rc::Rc<Bop>>),
}

pub use oxidized::ast_defs::Uop;
