// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{DeclProvider, Error, Result};
use arena_deserializer::serde::Deserialize;
use libc::{c_char, c_int};
use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::{direct_decl_parser, shallow_decl_defs::Decl};
use std::ffi::c_void;

/**
Technically, the object layout of the values we are working with
(produced in C++) is this:

```rust
  struct Decls<'decl>(direct_decl_parser::Decls<'decl>);
  struct Bytes(ffi::Bytes);

  enum ExternalDeclProviderResult<'decl> {
      Missing,
      Decls(*const Decls<'decl>),
      Bytes(*const Bytes),
  }
```

(c.f. 'rust_ffi_compile.rs' and 'hh_single_compile.cpp' (for example)).
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

/// Function signature for external provider functions.
pub type ProviderFunc<'decl> = unsafe extern "C" fn(
    // Caller provided cookie
    *const c_void,
    // A proxy for `HPHP::AutoloadMap::KindOf`
    c_int,
    // The symbol & len
    *const c_char,
    usize,
) -> ExternalDeclProviderResult<'decl>;

#[derive(Debug)]
pub struct ExternalDeclProvider<'decl> {
    pub provider: ProviderFunc<'decl>,
    pub data: *const c_void,
    pub arena: &'decl bumpalo::Bump,
}

impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl> {
    fn get_decl(&self, kind: NameType, symbol: &str) -> Result<Decl<'decl>> {
        // Need to convert NameType into HPHP::AutoloadMap::KindOf.
        let code: i32 = match kind {
            NameType::Class => 0,   // HPHP::AutoloadMap::KindOf::Type
            NameType::Typedef => 3, // HPHP::AutoloadMap::KindOf::TypeAlias
            NameType::Fun => 1,     // HPHP::AutoloadMap::KindOf::Function
            NameType::Const => 2,   // HPHP::AutoloadMap::KindOf::Constant
        };
        let result = unsafe {
            // Invoke extern C/C++ provider implementation.
            (self.provider)(self.data, code, symbol.as_ptr() as _, symbol.len())
        };
        match result {
            ExternalDeclProviderResult::Missing => Err(Error::NotFound),
            ExternalDeclProviderResult::Decls(p) => {
                let decls: &direct_decl_parser::Decls<'decl> = unsafe { p.as_ref() }.unwrap();
                let decl = decls
                    .iter()
                    .find_map(|(sym, decl)| if sym == symbol { Some(decl) } else { None });
                match decl {
                    None => Err(Error::NotFound),
                    Some(decl) => Ok(decl),
                }
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

                let decl = decls
                    .iter()
                    .find_map(|(sym, decl)| if sym == symbol { Some(decl) } else { None });
                match decl {
                    None => Err(Error::NotFound),
                    Some(decl) => Ok(decl),
                }
            }
        }
    }
} //impl<'decl> DeclProvider<'decl> for ExternalDeclProvider<'decl>
