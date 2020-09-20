// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::BStr;

use crate::ast_defs::*;
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
            SFlitInt((p, _)) | SFlitStr((p, _)) | SFclassConst(_, (p, _)) => &p,
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

impl Bop {
    pub fn is_any_eq(&self) -> bool {
        match self {
            Self::Eq(_) => true,
            _ => false,
        }
    }
}
