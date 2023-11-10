// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_and_decl_parser::Env;
use bumpalo::Bump;
use ocamlrep::bytes_from_ocamlrep;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep_caml_builtins::Int64;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::ocaml_ffi_arena_result;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use parser_core_types::indexed_source_text::IndexedSourceText;
use relative_path::RelativePath;

#[derive(Debug, Clone)]
pub struct OcamlParsedFileWithHashes<'a>(ParsedFileWithHashes<'a>);

impl<'a> From<ParsedFileWithHashes<'a>> for OcamlParsedFileWithHashes<'a> {
    fn from(file: ParsedFileWithHashes<'a>) -> Self {
        Self(file)
    }
}

// NB: Must keep in sync with OCaml type `Direct_decl_parser.parsed_file_with_hashes`.
// Written manually because the underlying type doesn't implement ToOcamlRep;
// even if it did, its self.0.decls structure stores hh24_types::DeclHash
// but we here need an Int64. Writing manually is slicker than constructing
// a temporary vec.
impl ocamlrep::ToOcamlRep for OcamlParsedFileWithHashes<'_> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size(3);
        alloc.set_field(&mut block, 0, alloc.add(&self.0.mode));
        alloc.set_field(
            &mut block,
            1,
            alloc.add_copy(Int64(self.0.file_decls_hash.as_u64() as i64)),
        );
        let mut hd = alloc.add(&());
        for (name, decl, hash) in self.0.iter() {
            let mut tuple = alloc.block_with_size(3);
            alloc.set_field(&mut tuple, 0, alloc.add(name));
            alloc.set_field(&mut tuple, 1, alloc.add(decl));
            alloc.set_field(&mut tuple, 2, alloc.add_copy(Int64(hash.as_u64() as i64)));

            let mut cons_cell = alloc.block_with_size(2);
            alloc.set_field(&mut cons_cell, 0, tuple.build());
            alloc.set_field(&mut cons_cell, 1, hd);
            hd = cons_cell.build();
        }
        alloc.set_field(&mut block, 2, hd);
        block.build()
    }
}

ocaml_ffi_arena_result! {
    fn hh_parse_decls_ffi<'a>(
        arena: &'a Bump,
        opts: DeclParserOptions,
        filename: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> ParsedFile<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        direct_decl_parser::parse_decls_for_typechecking(&opts, filename, text, arena)
    }

    fn hh_parse_and_hash_decls_ffi<'a>(
        arena: &'a Bump,
        opts: DeclParserOptions,
        deregister_php_stdlib_if_hhi: bool,
        filename: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> OcamlParsedFileWithHashes<'a> {
        let prefix = filename.prefix();
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        let parsed_file = direct_decl_parser::parse_decls_for_typechecking(&opts, filename, text, arena);
        let with_hashes = ParsedFileWithHashes::new(parsed_file, deregister_php_stdlib_if_hhi, prefix, arena);
        with_hashes.into()
    }
}

ocaml_ffi! {
    fn checksum_addremove_ffi(
        checksum: Int64,
        symbol: Int64,
        decl_hash: Int64,
        path: relative_path::RelativePath
    ) -> Int64 {
        // CARE! This implementation must be identical to the strongly-typed one in hh24_types.rs
        // I wrote it out as a separate copy because I didn't want hh_server to take a dependency
        // upon hh24_types
        let checksum = checksum.0 as u64;
        let checksum = checksum ^ hh_hash::hash(&(symbol, decl_hash, path));
        Int64(checksum as i64)
    }
}

#[no_mangle]
unsafe extern "C" fn hh_parse_ast_and_decls_ffi(env: usize, source_text: usize) -> usize {
    fn inner(env: usize, source_text: usize) -> usize {
        use ocamlrep::FromOcamlRep;
        use ocamlrep_ocamlpool::to_ocaml;
        use parser_core_types::source_text::SourceText;

        // SAFETY: We can't call into OCaml while these values created via
        // `from_ocaml` exist.
        let env = unsafe { Env::from_ocaml(env).unwrap() };
        let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
        let indexed_source_text = IndexedSourceText::new(source_text);

        let arena = &Bump::new();
        let (ast_result, decls) = ast_and_decl_parser::from_text(&env, &indexed_source_text, arena);
        // WARNING! this doesn't respect deregister_php_stdlib and is likely wrong.
        let decls = ParsedFileWithHashes::new_without_deregistering_do_not_use(decls);
        let decls = OcamlParsedFileWithHashes::from(decls);
        // SAFETY: Requires no concurrent interaction with the OCaml runtime
        unsafe { to_ocaml(&(ast_result, decls)) }
    }
    ocamlrep_ocamlpool::catch_unwind(|| inner(env, source_text))
}
