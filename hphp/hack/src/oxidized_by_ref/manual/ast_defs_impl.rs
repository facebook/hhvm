// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ast_defs::*;
use crate::pos::Pos;

impl<'a> ShapeFieldName<'a> {
    pub fn get_name(&self) -> &'a str {
        use ShapeFieldName::*;
        match self {
            SFlitInt((_, name)) | SFlitStr((_, name)) | SFclassConst(_, (_, name)) => name,
        }
    }

    pub fn get_pos(&self) -> &'a Pos {
        use ShapeFieldName::*;
        match self {
            SFlitInt((p, _)) | SFlitStr((p, _)) | SFclassConst(_, (p, _)) => p,
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

    pub fn to_oxidized(&self) -> oxidized::ast_defs::Id {
        oxidized::ast_defs::Id(self.0.to_oxidized(), self.1.to_string())
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
