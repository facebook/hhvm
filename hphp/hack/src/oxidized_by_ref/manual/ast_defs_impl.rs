// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::BStr;

use crate::ast_defs::*;
use crate::pos::Pos;

impl<'a> ShapeFieldName<'a> {
    pub fn get_name(&self) -> &'a BStr {
        use ShapeFieldName::*;
        match self {
            SFlitInt((_, name)) | SFclassConst((_, (_, name))) => name.as_bytes().into(),
            SFlitStr((_, name)) => name,
        }
    }

    pub fn get_pos(&self) -> &'a Pos {
        use ShapeFieldName::*;
        match self {
            SFlitInt((p, _)) | SFlitStr((p, _)) | SFclassConst((_, (p, _))) => p,
        }
    }
}

impl<'a> Id<'a> {
    pub fn pos(&self) -> &'a Pos {
        self.0
    }

    pub fn name(&self) -> &'a str {
        self.1
    }
}

impl std::fmt::Debug for Id<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Id({:?}, {:?})", self.pos(), self.name())
    }
}

impl<'a> Bop<'a> {
    pub fn is_any_eq(&self) -> bool {
        match self {
            Self::Eq(_) => true,
            _ => false,
        }
    }
}
