// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_deserializer::serde::Deserialize;
use bincode::Options;
use no_pos_hash::position_insensitive_hash;
use oxidized::relative_path::{Prefix, RelativePath};
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, direct_decl_parser::Decls};

use libc::c_char;
use std::os::unix::ffi::OsStrExt;

mod internal {
    #[repr(C)]
    pub(crate) struct DeclResult<'a> {
        hash: u64,
        serialized: ffi::Bytes,
        decls: *mut oxidized_by_ref::direct_decl_parser::Decls<'a>,
    }

    impl<'a> DeclResult<'a> {
        pub fn new(
            hash: u64,
            serialized: ffi::Bytes,
            decls: oxidized_by_ref::direct_decl_parser::Decls<'a>,
        ) -> Self {
            Self {
                hash,
                serialized,
                decls: Box::into_raw(Box::new(decls)),
            }
        }

        pub unsafe fn into_parts(
            self,
        ) -> (
            u64,
            ffi::Bytes,
            Box<oxidized_by_ref::direct_decl_parser::Decls<'a>>,
        ) {
            (self.hash, self.serialized, Box::from_raw(self.decls))
        }
    }
}

use internal::DeclResult;

#[no_mangle]
unsafe extern "C" fn hackc_create_arena() -> *mut bumpalo::Bump {
    Box::into_raw(Box::new(bumpalo::Bump::new()))
}

#[no_mangle]
unsafe extern "C" fn hackc_free_arena(arena: *mut bumpalo::Bump) {
    let _ = Box::from_raw(arena);
}

#[no_mangle]
unsafe extern "C" fn hackc_create_direct_decl_parse_options(
    // TODO(Shayne): cxx doesn't support tuple,
    //auto_namespace_map: &'a [(&'a str, &'a str)],
    disable_xhp_element_mangling: bool,
    interpret_soft_types_as_like_types: bool,
) -> *mut DeclParserOptions<'static> {
    Box::into_raw(Box::new(DeclParserOptions {
        auto_namespace_map: &[],
        disable_xhp_element_mangling,
        interpret_soft_types_as_like_types,
    }))
}

// TODO(shiqicao): wrap catch_unwind
#[no_mangle]
unsafe extern "C" fn hackc_direct_decl_parse<'a>(
    opts: *const DeclParserOptions<'a>,
    filename: *const c_char,
    text: *const c_char,
    arena: *const bumpalo::Bump,
) -> DeclResult<'a> {
    let text: &'a [u8] = std::ffi::CStr::from_ptr(text).to_bytes();

    let path = std::path::PathBuf::from(std::ffi::OsStr::from_bytes(
        std::ffi::CStr::from_ptr(filename).to_bytes(),
    ));
    let filename = RelativePath::make(Prefix::Root, path);
    let decls = decl_rust::direct_decl_parser::parse_decls_without_reference_text(
        &(*opts),
        filename,
        text,
        &(*arena),
        None,
    );

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let data = op
        .serialize(&decls)
        .map_err(|e| format!("failed to serialize, error: {}", e))
        .unwrap();

    DeclResult::new(position_insensitive_hash(&decls), data.into(), decls)
}

#[no_mangle]
unsafe extern "C" fn hackc_print_decls(decls: *const Decls) {
    println!("{:#?}", *decls)
}

#[no_mangle]
unsafe extern "C" fn hackc_verify_deserialization(
    serialized: *const ffi::Bytes,
    expected: *const Decls,
) -> bool {
    let arena = bumpalo::Bump::new();

    let data = std::slice::from_raw_parts((*serialized).data, (*serialized).len);

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let mut de = bincode::de::Deserializer::from_slice(data, op);

    let de = arena_deserializer::ArenaDeserializer::new(&arena, &mut de);
    let decls = Decls::deserialize(de)
        .map_err(|e| format!("failed to deserialize, error: {}", e))
        .unwrap();

    decls == *expected
}

#[no_mangle]
unsafe extern "C" fn hackc_free_decl_result<'a>(decl_result: DeclResult<'a>) {
    let _ = decl_result.into_parts();
}
