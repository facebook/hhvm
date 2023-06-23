// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs;
use std::path::PathBuf;
use std::sync::Arc;
use std::time::Duration;

use aast_parser::rust_aast_parser_types::Env as AastParserEnv;
use bumpalo::Bump;
use clap::Parser;
use criterion::Criterion;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use relative_path::Prefix;
use relative_path::RelativePath;

#[derive(Debug, Parser)]
struct Options {
    files: Vec<PathBuf>,

    // The total time in seconds to spend measuring each parser.
    #[clap(long, default_value = "10")]
    measurement_time: u64,

    // If true, only benchmark the direct decl parser.
    #[clap(long)]
    direct_decl_only: bool,
}

fn main() {
    let opts = Options::parse();

    let mut criterion = Criterion::default()
        .warm_up_time(Duration::from_secs(2))
        .sample_size(50)
        .measurement_time(Duration::from_secs(opts.measurement_time));

    let filenames_and_contents = get_contents(&opts.files);
    let files = filenames_and_contents
        .iter()
        .map(|(path, text)| (Arc::clone(path), text.as_bytes()))
        .collect::<Vec<_>>();

    if opts.direct_decl_only {
        bench_direct_decl_parse(&mut criterion, &files);
    } else {
        bench_aast_full_parse(&mut criterion, &files);
        bench_aast_quick_parse(&mut criterion, &files);
        bench_direct_decl_parse(&mut criterion, &files);
        bench_cst_and_decl_parse(&mut criterion, &files);
        bench_ast_and_decl_parse(&mut criterion, &files);
    }

    criterion.final_summary();
}

fn get_contents(filenames: &[PathBuf]) -> Vec<(Arc<RelativePath>, String)> {
    filenames
        .iter()
        .map(|file| {
            (
                Arc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(file))),
                fs::read_to_string(file)
                    .unwrap_or_else(|_| panic!("Could not read file {}", file.display())),
            )
        })
        .collect()
}

fn bench_direct_decl_parse(c: &mut Criterion, files: &[(Arc<RelativePath>, &[u8])]) {
    let mut arena = Bump::with_capacity(1024 * 1024); // 1 MB
    let opts = Default::default();
    c.bench_function("direct_decl_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let _ = direct_decl_parser::parse_decls_for_typechecking(
                    &opts,
                    (**filename).clone(),
                    text,
                    &arena,
                );
                arena.reset();
            }
        })
    });
}

fn bench_cst_and_decl_parse(c: &mut Criterion, files: &[(Arc<RelativePath>, &[u8])]) {
    let mut arena = Bump::with_capacity(1024 * 1024); // 1 MB
    let opts = Default::default();
    c.bench_function("cst_and_decl_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(Arc::clone(filename), text);
                let _ = cst_and_decl_parser::parse_script(
                    &opts,
                    Default::default(),
                    &text,
                    None,
                    &arena,
                );
                arena.reset();
            }
        })
    });
}

fn bench_ast_and_decl_parse(c: &mut Criterion, files: &[(Arc<RelativePath>, &[u8])]) {
    let mut arena = Bump::with_capacity(1024 * 1024); // 1 MB
    c.bench_function("ast_and_decl_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(Arc::clone(filename), text);
                let indexed_source_text = IndexedSourceText::new(text.clone());
                let _ = ast_and_decl_parser::from_text(
                    &Default::default(),
                    &indexed_source_text,
                    &arena,
                );
                arena.reset();
            }
        })
    });
}

fn bench_aast_full_parse(c: &mut Criterion, files: &[(Arc<RelativePath>, &[u8])]) {
    c.bench_function("aast_full_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(Arc::clone(filename), text);
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(&AastParserEnv::default(), &indexed_source_text)
                    .unwrap();
            }
        })
    });
}

fn bench_aast_quick_parse(c: &mut Criterion, files: &[(Arc<RelativePath>, &[u8])]) {
    c.bench_function("aast_quick_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(Arc::clone(filename), text);
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(
                    &AastParserEnv {
                        quick_mode: true,
                        ..AastParserEnv::default()
                    },
                    &indexed_source_text,
                )
                .unwrap();
            }
        })
    });
}
