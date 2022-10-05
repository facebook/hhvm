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
use ocamlrep_ocamlpool::ocaml_ffi_arena_result;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use parser_core_types::indexed_source_text::IndexedSourceText;
use relative_path::RelativePath;

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
        direct_decl_parser::parse_decls(&opts, filename, text, arena)
    }

    fn hh_parse_and_hash_decls_ffi<'a>(
        arena: &'a Bump,
        opts: DeclParserOptions,
        filename: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> ParsedFileWithHashes<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        direct_decl_parser::parse_decls(&opts, filename, text, arena).into()
    }
}

ocaml_ffi_with_arena! {
    fn decls_hash<'a>(arena: &'a Bump, decls: Decls<'a>) -> Int64 {
        Int64(hh_hash::position_insensitive_hash(&decls) as i64)
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
        let decls = ParsedFileWithHashes::from(decls);
        // SAFETY: Requires no concurrent interaction with the OCaml runtime
        unsafe { to_ocaml(&(ast_result, decls)) }
    }
    ocamlrep_ocamlpool::catch_unwind(|| inner(env, source_text))
}
