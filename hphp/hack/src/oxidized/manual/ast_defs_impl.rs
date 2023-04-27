// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::BStr;

use crate::ast_defs::*;
use crate::naming_error;
use crate::pos::Pos;

impl ShapeFieldName {
    pub fn get_name(&self) -> &BStr {
        use ShapeFieldName::*;
        match self {
            SFlitInt((_, name)) | SFclassConst(_, (_, name)) => name.as_bytes().into(),
            SFlitStr((_, name)) => name.as_ref(),
        }
    }

    pub fn get_pos(&self) -> &Pos {
        use ShapeFieldName::*;
        match self {
            SFlitInt((p, _)) | SFlitStr((p, _)) | SFclassConst(_, (p, _)) => p,
        }
    }
}

impl FunKind {
    pub fn is_async(self) -> bool {
        self == FunKind::FAsync || self == FunKind::FAsyncGenerator
    }
}

impl Id {
    pub fn pos(&self) -> &Pos {
        &self.0
    }

    pub fn name(&self) -> &str {
        &self.1
    }
}

#[allow(clippy::derivable_impls)]
impl Default for Id {
    fn default() -> Self {
        Id(Default::default(), Default::default())
    }
}

impl Bop {
    pub fn is_any_eq(&self) -> bool {
        match self {
            Self::Eq(_) => true,
            _ => false,
        }
    }
}

impl Variance {
    pub fn negate(&self) -> Self {
        match self {
            Variance::Covariant => Variance::Contravariant,
            Variance::Contravariant => Variance::Covariant,
            Variance::Invariant => Variance::Invariant,
        }
    }
}

impl From<Visibility> for naming_error::Visibility {
    fn from(val: Visibility) -> Self {
        match val {
            Visibility::Internal => naming_error::Visibility::Vinternal,
            Visibility::Private => naming_error::Visibility::Vprivate,
            Visibility::Protected => naming_error::Visibility::Vprotected,
            Visibility::Public => naming_error::Visibility::Vprivate,
        }
    }
}
