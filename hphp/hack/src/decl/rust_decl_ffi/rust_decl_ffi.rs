// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use direct_decl_parser;
use ocamlrep::{bytes_from_ocamlrep, ptr::UnsafeOcamlPtr};
use ocamlrep_caml_builtins::Int64;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{
    decl_parser_options::DeclParserOptions,
    direct_decl_parser::{Decl, Decls, ParsedFile},
    file_info,
};
use parser_core_types::indexed_source_text::IndexedSourceText;

// NB: Must keep in sync with OCaml type `Direct_decl_parser.parsed_file_with_hashes`
#[derive(ocamlrep_derive::ToOcamlRep)]
struct ParsedFileWithHashes<'a> {
    pub mode: Option<file_info::Mode>,
    pub hash: Int64,
    pub decls: Vec<(&'a str, Decl<'a>, Int64)>,
}

ocaml_ffi_with_arena! {
    fn hh_parse_decls_ffi<'a>(
        arena: &'a Bump,
        opts: &'a DeclParserOptions<'a>,
        filename: &'a oxidized_by_ref::relative_path::RelativePath<'a>,
        text: UnsafeOcamlPtr,
    ) -> ParsedFile<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        parse_decls(arena, opts, filename.to_oxidized(), text)
    }

    fn hh_parse_and_hash_decls_ffi<'a>(
        arena: &'a Bump,
        opts: &'a DeclParserOptions<'a>,
        filename: &'a oxidized_by_ref::relative_path::RelativePath<'a>,
        text: UnsafeOcamlPtr,
    ) -> ParsedFileWithHashes<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        let parsed_file = parse_decls(arena, opts, filename.to_oxidized(), text);
        hash_decls(parsed_file)
    }

    fn decls_hash<'a>(arena: &'a Bump, decls: Decls<'a>) -> Int64 {
        Int64(hh_hash::position_insensitive_hash(&decls) as i64)
    }
}

#[no_mangle]
unsafe extern "C" fn hh_parse_ast_and_decls_ffi(env: usize, source_text: usize) -> usize {
    fn inner(env: usize, source_text: usize) -> usize {
        use ast_and_decl_parser::Env;
        use ocamlrep::FromOcamlRep;
        use ocamlrep_ocamlpool::to_ocaml;
        use parser_core_types::source_text::SourceText;

        // SAFETY: We can't call into OCaml while these values created via
        // `from_ocaml` exist.
        let env = unsafe { Env::from_ocaml(env).unwrap() };
        let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
        let indexed_source_text = IndexedSourceText::new(source_text);

        let arena = &Bump::new();
        let (ast_result, decls) = parse_ast_and_decls(arena, &env, &indexed_source_text);
        let decls = hash_decls(decls);
        // SAFETY: Requires no concurrent interaction with the OCaml runtime
        unsafe { to_ocaml(&(ast_result, decls)) }
    }
    ocamlrep_ocamlpool::catch_unwind(|| inner(env, source_text))
}

fn parse_decls<'a>(
    arena: &'a Bump,
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
) -> ParsedFile<'a> {
    stack_limit::with_elastic_stack(|stack_limit| {
        direct_decl_parser::parse_decls(opts, filename.clone(), text, arena, Some(stack_limit))
    })
    .unwrap_or_else(|failure| {
        panic!(
            "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
            failure.max_stack_size_tried / stack_limit::KI
        );
    })
}

fn parse_ast_and_decls<'a>(
    arena: &'a Bump,
    env: &'a ast_and_decl_parser::Env,
    indexed_source_text: &'a IndexedSourceText<'a>,
) -> (
    ast_and_decl_parser::AastResult<ast_and_decl_parser::ParserResult>,
    ParsedFile<'a>,
) {
    stack_limit::with_elastic_stack(|stack_limit| {
        ast_and_decl_parser::from_text(&env, &indexed_source_text, arena, Some(stack_limit))
    })
    .expect("Expression recursion limit reached")
}

fn hash_decls<'a>(parsed_file: ParsedFile<'a>) -> ParsedFileWithHashes<'a> {
    let file_decls_hash = Int64(hh_hash::position_insensitive_hash(&parsed_file.decls) as i64);
    let decls = Vec::from_iter(
        parsed_file
            .decls
            .into_iter()
            .map(|(name, decl)| (name, decl, Int64(hh_hash::hash(&decl) as i64))),
    );
    ParsedFileWithHashes {
        mode: parsed_file.mode,
        hash: file_decls_hash,
        decls,
    }
}
