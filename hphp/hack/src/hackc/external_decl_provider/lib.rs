// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_deserializer::serde::Deserialize;
use decl_provider::{self, DeclProvider};
use libc::{c_char, c_int};
use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::{direct_decl_parser, shallow_decl_defs::Decl};

/*
Technically, the object layout of the values we are working with
(produced in C++) is this (c.f. 'rust_ffi_compile.rs' and
'hh_single_compile.cpp' (for example)):
```rust
  struct Decls<'decl>(direct_decl_parser::Decls<'decl>);
  struct Bytes(ffi::Bytes);

  enum ExternalDeclProviderResult<'decl> {
      Missing,
      Decls(*const Decls<'decl>),
      Bytes(*const Bytes),
  }
```
That is, when C++ returns us results, this is formally the layout we
can expect in Rust.

With that definition for `ExternalDeclProvider<'decl>` then, in the
`ExternalDeclProviderResult::Decls(p)` case you can get at the actual
decls as:
```
  let decls: &Decls<'decl> = &(unsafe { p.as_ref() }.unwrap().0);
```

This definition for `ExternalDeclProvider<'decl>` and that access
style naturally works. There is a simplification we can make though.

We can treat:
    - a `*const Decls<'decl>` as a `*const direct_decl_parser::Decls<'decl>`
    - a `*const Bytes` as a `*const ffi::Bytes`
(both points follow from the fact that the address of a POD is equal
to the address of its first member).

Therefore, we arrive at the equivalent but more direct
```
  pub enum ExternalDeclProviderResult<'decl> {
      Missing,
      Decls(*const direct_decls::Decls<'decl>),
      Bytes(*const ffi::Bytes),
  }
```
and, given a value of case `ExternalDeclProviderResult::Decls(p)`, go
straight to the decls with:
```
let decls: &Decls<'decl> = unsafe { p.as_ref() }.unwrap();
```
*/
#[repr(C)]
pub enum ExternalDeclProviderResult<'decl> {
    Missing,
    Decls(*const direct_decl_parser::Decls<'decl>),
    Bytes(*const ffi::Bytes),
}

#[derive(Debug)]
pub struct ExternalDeclProvider<'decl>(
    pub  unsafe extern "C" fn(
        *const std::ffi::c_void, // Caller provided cookie
        c_int,                   // A proxy for `HPHP::AutoloadMap::KindOf`
        *const c_char,           // The symbol
    ) -> ExternalDeclProviderResult<'decl>, // Possible payload: `*const Decl<'decl>` or, `const* Bytes`
    pub *const std::ffi::c_void,
    &'decl bumpalo::Bump,
);

impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl> {
    fn get_decl(&self, kind: NameType, symbol: &str) -> Result<Decl<'decl>, decl_provider::Error> {
        // Need to convert NameType into HPHP::AutoloadMap::KindOf.
        let code: i32 = match kind {
            NameType::Class => 0,   // HPHP::AutoloadMap::KindOf::Type
            NameType::Typedef => 3, // HPHP::AutoloadMap::KindOf::TypeAlias
            NameType::Fun => 1,     // HPHP::AutoloadMap::KindOf::Function
            NameType::Const => 2,   // HPHP::AutoloadMap::KindOf::Constant
        };
        let symbol_ptr = std::ffi::CString::new(symbol).unwrap();
        match unsafe { self.0(self.1, code, symbol_ptr.as_c_str().as_ptr()) } {
            ExternalDeclProviderResult::Missing => Err(decl_provider::Error::NotFound),
            ExternalDeclProviderResult::Decls(p) => {
                let decls: &direct_decl_parser::Decls<'decl> = unsafe { p.as_ref() }.unwrap();
                let decl = decls
                    .iter()
                    .find_map(|(sym, decl)| if sym == symbol { Some(decl) } else { None });
                match decl {
                    None => Err(decl_provider::Error::NotFound),
                    Some(decl) => Ok(decl),
                }
            }
            ExternalDeclProviderResult::Bytes(p) => {
                let bytes: &ffi::Bytes = unsafe { p.as_ref() }.unwrap();
                let arena = self.2;
                let data = unsafe { std::slice::from_raw_parts(bytes.data, bytes.len) };
                let op = bincode::config::Options::with_native_endian(bincode::options());
                let mut de = bincode::de::Deserializer::from_slice(data, op);

                let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
                let decls = direct_decl_parser::Decls::deserialize(de)
                    .map_err(|e| format!("failed to deserialize, error: {}", e))
                    .unwrap();

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
} //impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl>

impl<'decl> ExternalDeclProvider<'decl> {
    pub fn new(
        decl_getter: unsafe extern "C" fn(
            *const std::ffi::c_void,
            c_int,
            *const c_char,
        ) -> ExternalDeclProviderResult<'decl>,
        decl_provider: *const std::ffi::c_void,
        decl_allocator: &'decl bumpalo::Bump,
    ) -> Self {
        Self(decl_getter, decl_provider, decl_allocator)
    }
}
