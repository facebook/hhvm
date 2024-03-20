// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::file_info::NameType;
use crate::shallow_decl_defs::Decl;
use crate::typing_defs_core::UserAttributeParam;

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
    pub fn sort_text(&mut self) -> Option<String> {
        match self {
            Decl::Class(decl) => decl
                .user_attributes
                .iter()
                .find(|ua| ua.name.1 == "__AutocompleteSortText")
                .and_then(|ua| ua.params.first().cloned())
                .and_then(|param| match param {
                    UserAttributeParam::String(s) => Some(s),
                    _ => None,
                })
                .map(|s| s.to_string()),
            _ => None,
        }
    }
}
