// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b106ee7f62db85290ff19c9a92f8a145>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct Id<'a>(pub Pos<'a>, pub &'a str);

pub type Pstring<'a> = (Pos<'a>, &'a str);

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum ShapeFieldName<'a> {
    SFlitInt(Pstring<'a>),
    SFlitStr(Pstring<'a>),
    SFclassConst(Id<'a>, Pstring<'a>),
}

pub use oxidized::ast_defs::Variance;

pub use oxidized::ast_defs::ConstraintKind;

pub type Reified<'a> = bool;

pub use oxidized::ast_defs::ClassKind;

pub use oxidized::ast_defs::ParamKind;

pub use oxidized::ast_defs::OgNullFlavor;

pub use oxidized::ast_defs::FunKind;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum Bop<'a> {
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
    Eq(Option<&'a Bop<'a>>),
}

pub use oxidized::ast_defs::Uop;
