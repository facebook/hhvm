// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(test)]

extern crate test;

fn main() {
    let command = "FILE_TO_PARSE=<file> buck run @mode/opt //hphp/hack/src/parser/bench:bench-unittest -- --bench";
    println!("Run this bench suite with: {}", command);
    println!();
    println!("Running this bench suite without FILE_TO_PARSE set will cause the benchmarks to complete instantly.");
}

#[cfg(test)]
mod tests {
    use test::Bencher;

    use decl_rust::direct_decl_parser;
    use facts_rust::facts_parser;
    use oxidized::{
        global_options,
        relative_path::{Prefix, RelativePath},
        rust_aast_parser_types,
    };
    use parser_rust::{
        indexed_source_text::IndexedSourceText, parser_env::ParserEnv, source_text::SourceText,
    };
    use std::{env, fs, path::PathBuf};

    fn get_file() -> Option<(RelativePath, String)> {
        match env::var("FILE_TO_PARSE") {
            Ok(file) => Some((
                RelativePath::make(Prefix::Dummy, PathBuf::from(&file)),
                fs::read_to_string(&file).expect(&format!("Could not read file {}", file)),
            )),
            Err(_) => None,
        }
    }

    #[bench]
    fn bench_facts_parse(b: &mut Bencher) {
        get_file().map(|(filename, text)| {
            b.iter(|| {
                let text = SourceText::make(&filename, text.as_bytes());
                let mut parser = facts_parser::FactsParser::make(&text, ParserEnv::default());
                parser.parse_script(None)
            });
        });
    }

    #[bench]
    fn bench_direct_decl_parse(b: &mut Bencher) {
        get_file().map(|(filename, text)| {
            b.iter(|| {
                let text = SourceText::make(&filename, text.as_bytes());
                let mut parser =
                    direct_decl_parser::DirectDeclParser::make(&text, ParserEnv::default());
                parser.parse_script(None)
            });
        });
    }

    #[bench]
    fn bench_aast_full_parse(b: &mut Bencher) {
        get_file().map(|(filename, text)| {
            b.iter(|| {
                let text = SourceText::make(&filename, text.as_bytes());
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(
                    &rust_aast_parser_types::Env {
                        is_hh_file: true,
                        codegen: false,
                        php5_compat_mode: false,
                        elaborate_namespaces: false,
                        include_line_comments: false,
                        keep_errors: false,
                        quick_mode: false,
                        show_all_errors: false,
                        lower_coroutines: false,
                        fail_open: false,
                        parser_options: global_options::GlobalOptions::default(),
                        hacksperimental: false,
                    },
                    &indexed_source_text,
                )
            })
        });
    }

    #[bench]
    fn bench_aast_quick_parse(b: &mut Bencher) {
        get_file().map(|(filename, text)| {
            b.iter(|| {
                let text = SourceText::make(&filename, text.as_bytes());
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(
                    &rust_aast_parser_types::Env {
                        is_hh_file: true,
                        codegen: false,
                        php5_compat_mode: false,
                        elaborate_namespaces: false,
                        include_line_comments: false,
                        keep_errors: false,
                        quick_mode: true,
                        show_all_errors: false,
                        lower_coroutines: false,
                        fail_open: false,
                        parser_options: global_options::GlobalOptions::default(),
                        hacksperimental: false,
                    },
                    &indexed_source_text,
                )
            })
        });
    }
}
