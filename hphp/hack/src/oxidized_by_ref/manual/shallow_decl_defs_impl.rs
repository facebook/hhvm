// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::file_info::NameType;

use crate::shallow_decl_defs::Decl;
use crate::typing_defs_core::UserAttributeParam;

impl<'a> Decl<'a> {
    pub fn kind(&self) -> NameType {
        match self {
            Decl::Class(..) => NameType::Class,
            Decl::Fun(..) => NameType::Fun,
            Decl::Typedef(..) => NameType::Typedef,
            Decl::Const(..) => NameType::Const,
            Decl::Module(..) => NameType::Module,
        }
    }

    pub fn sort_text(&self) -> Option<&str> {
        match self {
            Decl::Class(decl) => decl
                .user_attributes
                .iter()
                .find(|ua| ua.name.1 == "__AutocompleteSortText")
                .and_then(|ua| ua.params.first().cloned())
                .and_then(|param| match param {
                    UserAttributeParam::String(s) => {
                        // This is a bit unsafe - but we're in the process of
                        // enforcing utf8 at the parsing layer (currently
                        // enabled in www) at which point this conversion won't
                        // be necessary.
                        Some(std::str::from_utf8(s).unwrap())
                    }
                    _ => None,
                }),
            _ => None,
        }
    }
}
