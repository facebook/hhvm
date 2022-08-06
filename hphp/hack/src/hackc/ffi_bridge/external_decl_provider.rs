// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::c_void;

use arena_deserializer::serde::Deserialize;
use decl_provider::Error;
use decl_provider::Result;
use decl_provider::TypeDecl;
use direct_decl_parser::Decls;
use libc::c_char;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;

use crate::DeclProvider;
use crate::DeclsHolder;

/// Keep this in sync with struct ExternalDeclProviderResult in decl_provider.h
#[repr(C)]
pub enum ExternalDeclProviderResult {
    Missing,
    Decls(*const DeclsHolder), // XXX can we keep the <'decl> lifetime?
    Bytes(*const Vec<u8>),
}

// Bridge to C++ DeclProviders.
extern "C" {
    // Safety: direct_decl_parser::Decls is a list of tuples, which cannot be repr(C)
    // even if the contents are. But we never dereference Decls in C++.
    #[allow(improper_ctypes)]
    fn provide_type(
        provider: *const c_void,
        symbol: *const c_char,
        symbol_len: usize,
        depth: u64,
    ) -> ExternalDeclProviderResult;

    #[allow(improper_ctypes)]
    fn provide_func(
        provider: *const c_void,
        symbol: *const c_char,
        symbol_len: usize,
    ) -> ExternalDeclProviderResult;

    #[allow(improper_ctypes)]
    fn provide_const(
        provider: *const c_void,
        symbol: *const c_char,
        symbol_len: usize,
    ) -> ExternalDeclProviderResult;

    #[allow(improper_ctypes)]
    fn provide_module(
        provider: *const c_void,
        symbol: *const c_char,
        symbol_len: usize,
    ) -> ExternalDeclProviderResult;
}

#[derive(Debug)]
pub struct ExternalDeclProvider<'a> {
    pub provider: *const c_void,
    pub arena: &'a bumpalo::Bump,
}

impl<'a> DeclProvider<'a> for ExternalDeclProvider<'a> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'a>> {
        let result = unsafe {
            // Invoke extern C/C++ provider implementation.
            provide_type(self.provider, symbol.as_ptr() as _, symbol.len(), depth)
        };
        self.find_decl(result, |decls| decl_provider::find_type_decl(decls, symbol))
    }

    fn func_decl(&self, symbol: &str) -> Result<&'a FunDecl<'a>> {
        let result = unsafe { provide_func(self.provider, symbol.as_ptr() as _, symbol.len()) };
        self.find_decl(result, |decls| decl_provider::find_func_decl(decls, symbol))
    }

    fn const_decl(&self, symbol: &str) -> Result<&'a ConstDecl<'a>> {
        let result = unsafe { provide_const(self.provider, symbol.as_ptr() as _, symbol.len()) };
        self.find_decl(result, |decls| {
            decl_provider::find_const_decl(decls, symbol)
        })
    }

    fn module_decl(&self, symbol: &str) -> Result<&'a ModuleDecl<'a>> {
        let result = unsafe { provide_module(self.provider, symbol.as_ptr() as _, symbol.len()) };
        self.find_decl(result, |decls| {
            decl_provider::find_module_decl(decls, symbol)
        })
    }
}

impl<'a> ExternalDeclProvider<'a> {
    /// Search for the decl we asked for in the list of decls returned.
    /// This is O(N) for now.
    fn find_decl<T>(
        &self,
        result: ExternalDeclProviderResult,
        mut find: impl FnMut(&Decls<'a>) -> Result<T>,
    ) -> Result<T> {
        match result {
            ExternalDeclProviderResult::Missing => Err(Error::NotFound),
            ExternalDeclProviderResult::Decls(ptr) => {
                let holder = unsafe { ptr.as_ref() }.unwrap();
                find(&holder.decls)
            }
            ExternalDeclProviderResult::Bytes(p) => {
                let data = unsafe {
                    // turn raw pointer back into &Bytes, then &[u8]
                    p.as_ref().unwrap().as_slice()
                };
                let op = bincode::config::Options::with_native_endian(bincode::options());
                let mut de = bincode::de::Deserializer::from_slice(data, op);
                let de = arena_deserializer::ArenaDeserializer::new(self.arena, &mut de);
                let decls = direct_decl_parser::Decls::deserialize(de)
                    .map_err(|e| format!("failed to deserialize, error: {}", e))
                    .unwrap();
                find(&decls)
            }
        }
    }
}
