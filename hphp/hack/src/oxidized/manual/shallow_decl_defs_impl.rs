// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::file_info::NameType;
use crate::shallow_decl_defs::Decl;

impl Decl {
    pub fn kind(&self) -> NameType {
        match self {
            Decl::Class(..) => NameType::Class,
            Decl::Fun(..) => NameType::Fun,
            Decl::Typedef(..) => NameType::Typedef,
            Decl::Const(..) => NameType::Const,
            Decl::Module(..) => NameType::Module,
        }
    }
}
