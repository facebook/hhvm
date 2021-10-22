// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::{self, DeclProvider};
use libc::{c_char, c_int};
use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::{direct_decl_parser::Decls, shallow_decl_defs::Decl};

#[derive(Debug)]
pub struct ExternalDeclProvider<'decl>(
    // The int proxies for HPHP::AutoloadMap::KindOf.
    pub  unsafe extern "C" fn(
        *const std::ffi::c_void,
        c_int,
        *const c_char,
    ) -> *const std::ffi::c_void,
    pub *const std::ffi::c_void,
    pub std::marker::PhantomData<&'decl ()>,
);

impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl> {
    fn get_decl(&self, kind: NameType, symbol: &str) -> Result<Decl<'decl>, decl_provider::Error> {
        /* Need to convert NameType into HPHP::AutoloadMap::KindOf */
        let code: i32 = match kind {
            NameType::Class => 0,   // HPHP::AutoloadMap::KindOf::Type
            NameType::Typedef => 3, // HPHP::AutoloadMap::KindOf::TypeAlias
            NameType::Fun => 1,     // HPHP::AutoloadMap::KindOf::Function
            NameType::Const => 2,   // HPHP::AutoloadMap::KindOf::Constant
            NameType::RecordDef => panic!("RecordDef is not a valid HPHP::AutoloadMap::KindOf"),
        };
        let symbol_ptr = std::ffi::CString::new(symbol).unwrap();
        let r = unsafe { self.0(self.1, code, symbol_ptr.as_c_str().as_ptr()) };
        if r.is_null() {
            Err(decl_provider::Error::NotFound)
        } else {
            let decls: &Decls<'decl> = unsafe { &(*(r as *const Decls<'decl>)) };
            let decl = decls
                .iter()
                .find_map(|(sym, decl)| if sym == symbol { Some(decl) } else { None });
            match decl {
                None => Err(decl_provider::Error::NotFound),
                Some(decl) => Ok(decl),
            }
        }
    }
}

impl<'decl> ExternalDeclProvider<'decl> {
    pub fn new(
        decl_getter: unsafe extern "C" fn(
            *const std::ffi::c_void,
            c_int,
            *const c_char,
        ) -> *const std::ffi::c_void,
        decl_provider: *const std::ffi::c_void,
    ) -> Self {
        Self(decl_getter, decl_provider, std::marker::PhantomData)
    }
}
