// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::DeclProvider;
use crate::DeclsHolder;
use arena_deserializer::serde::Deserialize;
use decl_provider::Error;
use decl_provider::Result;
use decl_provider::TypeDecl;
use libc::c_char;
use oxidized_by_ref::direct_decl_parser;
use std::ffi::c_void;

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
    fn provide_type_or_alias(
        provider: *const c_void,
        symbol: *const c_char,
        symbol_len: usize,
        depth: u64,
    ) -> ExternalDeclProviderResult;
}

#[derive(Debug)]
pub struct ExternalDeclProvider<'decl> {
    pub provider: *const c_void,
    pub arena: &'decl bumpalo::Bump,
}

impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'decl>> {
        // Invoke extern C/C++ provider implementation.
        let result = unsafe {
            provide_type_or_alias(self.provider, symbol.as_ptr() as _, symbol.len(), depth)
        };

        // Search for the decl we asked for in the list of decls returned.
        // This is O(N) for now.
        match result {
            ExternalDeclProviderResult::Missing => Err(Error::NotFound),
            ExternalDeclProviderResult::Decls(ptr) => {
                let holder = unsafe { ptr.as_ref() }.unwrap();
                decl_provider::find_type_decl(&holder.decls, symbol)
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
                decl_provider::find_type_decl(&decls, symbol)
            }
        }
    }
}
