// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_deserializer::serde::Deserialize;
use bincode::Options;
use cxx::CxxString;
use no_pos_hash::position_insensitive_hash;
use oxidized::relative_path::{Prefix, RelativePath};
use oxidized_by_ref::{decl_parser_options, direct_decl_parser};

#[cxx::bridge]
mod decl_ffi {
    pub struct DeclResult<'a> {
        pub hash: u64,
        pub serialized: Box<Bytes>,
        pub decls: Box<Decls<'a>>,
    }

    extern "Rust" {
        type Bump;
        type Bytes;
        type DeclParserOptions<'a>;
        type Decls<'a>;

        fn hackc_create_arena() -> Box<Bump>;
        fn hackc_create_direct_decl_parse_options(
            disable_xhp_element_mangling: bool,
            interpret_soft_types_as_like_types: bool,
        ) -> Box<DeclParserOptions<'static>>;

        unsafe fn hackc_direct_decl_parse<'a>(
            options: &'a DeclParserOptions<'a>,
            filename: &CxxString,
            text: &CxxString,
            bump: &'a Bump,
        ) -> DeclResult<'a>;

        fn hackc_print_decls(decls: &Decls);
        fn hackc_print_serialized_size(bytes: &Bytes);
        unsafe fn hackc_verify_deserialization(serialized: &Bytes, expected: &Decls) -> bool;
    }
}

pub struct Bump(bumpalo::Bump);
pub struct Bytes(ffi::Bytes);
pub struct Decls<'a>(direct_decl_parser::Decls<'a>);
pub struct DeclParserOptions<'a>(decl_parser_options::DeclParserOptions<'a>);
use decl_ffi::DeclResult;

fn hackc_create_arena() -> Box<Bump> {
    Box::new(Bump(bumpalo::Bump::new()))
}

fn hackc_print_decls<'a>(decls: &Decls<'a>) {
    println!("{:#?}", decls.0)
}

fn hackc_print_serialized_size(serialized: &Bytes) {
    println!("Decl-serialized size: {:#?}", serialized.0.len);
}

fn hackc_create_direct_decl_parse_options<'a>(
    disable_xhp_element_mangling: bool,
    interpret_soft_types_as_like_types: bool,
) -> Box<DeclParserOptions<'a>> {
    Box::new(DeclParserOptions(decl_parser_options::DeclParserOptions {
        auto_namespace_map: &[],
        disable_xhp_element_mangling,
        interpret_soft_types_as_like_types,
        everything_sdt: false,
    }))
}

impl<'a> DeclResult<'a> {
    pub fn new(hash: u64, serialized: Bytes, decls: Decls<'a>) -> Self {
        Self {
            hash,
            serialized: Box::new(serialized),
            decls: Box::new(decls),
        }
    }
}

fn hackc_direct_decl_parse<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: &CxxString,
    text: &CxxString,
    bump: &'a Bump,
) -> DeclResult<'a> {
    use std::os::unix::ffi::OsStrExt;

    let text = text.as_bytes();
    let path = std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes()));
    let filename = RelativePath::make(Prefix::Root, path);
    let decls = decl_rust::direct_decl_parser::parse_decls_without_reference_text(
        &opts.0, filename, text, &bump.0, None,
    );

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let data = op
        .serialize(&decls)
        .map_err(|e| format!("failed to serialize, error: {}", e))
        .unwrap();

    DeclResult::new(
        position_insensitive_hash(&decls),
        Bytes(ffi::Bytes::from(data)),
        Decls(decls),
    )
}

unsafe fn hackc_verify_deserialization<'a>(serialized: &Bytes, expected: &Decls<'a>) -> bool {
    let arena = bumpalo::Bump::new();

    let data = std::slice::from_raw_parts(serialized.0.data, serialized.0.len);

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let mut de = bincode::de::Deserializer::from_slice(data, op);

    let de = arena_deserializer::ArenaDeserializer::new(&arena, &mut de);
    let decls = direct_decl_parser::Decls::deserialize(de)
        .map_err(|e| format!("failed to deserialize, error: {}", e))
        .unwrap();

    decls == expected.0
}
