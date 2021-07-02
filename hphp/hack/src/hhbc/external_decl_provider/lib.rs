// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::{self, DeclProvider};
use libc::c_char;
use oxidized_by_ref::{direct_decl_parser::Decls, shallow_decl_defs::Decl};

pub struct ExternalDeclProvider<'decl>(
    pub unsafe extern "C" fn(*const std::ffi::c_void, *const c_char) -> *const std::ffi::c_void,
    pub *const std::ffi::c_void,
    pub std::marker::PhantomData<&'decl ()>,
);

impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl> {
    fn get_decl(&self, symbol: &str) -> Result<Decl<'decl>, decl_provider::Error> {
        let symbol_ptr = std::ffi::CString::new(symbol).unwrap();
        let r = unsafe { self.0(self.1, symbol_ptr.as_c_str().as_ptr()) };
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
