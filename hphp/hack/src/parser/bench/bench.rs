// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::time::Duration;
use std::{fs, path::PathBuf};

use bumpalo::Bump;
use criterion::Criterion;
use structopt::StructOpt;

use aast_parser::rust_aast_parser_types::Env as AastParserEnv;
use direct_decl_parser;
use facts_parser;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::{Prefix, RelativePath};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, parser_env::ParserEnv, source_text::SourceText,
};

#[derive(Debug, StructOpt)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (buck doesn't set it)
struct Options {
    #[structopt(parse(from_os_str))]
    files: Vec<PathBuf>,

    // The total time in seconds to spend measuring each parser.
    #[structopt(long, default_value = "10")]
    measurement_time: u64,

    // If true, only benchmark the direct decl parser.
    #[structopt(long)]
    direct_decl_only: bool,
}

fn main() {
    let opts = Options::from_args();

    let mut criterion = Criterion::default()
        .warm_up_time(Duration::from_secs(2))
        .sample_size(50)
        .measurement_time(Duration::from_secs(opts.measurement_time));

    let filenames_and_contents = get_contents(&opts.files);
    let files = filenames_and_contents
        .iter()
        .map(|(path, text)| (RcOc::clone(path), text.as_bytes()))
        .collect::<Vec<_>>();

    if opts.direct_decl_only {
        bench_direct_decl_parse(&mut criterion, &files);
    } else {
        bench_aast_full_parse(&mut criterion, &files);
        bench_aast_quick_parse(&mut criterion, &files);
        bench_direct_decl_parse(&mut criterion, &files);
        bench_facts_parse(&mut criterion, &files);
    }

    criterion.final_summary();
}

fn get_contents(filenames: &[PathBuf]) -> Vec<(RcOc<RelativePath>, String)> {
    filenames
        .into_iter()
        .map(|file| {
            (
                RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(file))),
                fs::read_to_string(&file)
                    .expect(&format!("Could not read file {}", file.display())),
            )
        })
        .collect()
}

fn bench_facts_parse(c: &mut Criterion, files: &[(RcOc<RelativePath>, &[u8])]) {
    c.bench_function("facts_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(RcOc::clone(filename), text);
                facts_parser::parse_script(&text, ParserEnv::default(), None);
            }
        })
    });
}

fn bench_direct_decl_parse(c: &mut Criterion, files: &[(RcOc<RelativePath>, &[u8])]) {
    let mut arena = Bump::with_capacity(1024 * 1024); // 1 MB
    c.bench_function("direct_decl_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(RcOc::clone(filename), text);
                let _ = direct_decl_parser::parse_script(&text, ParserEnv::default(), &arena, None);
                arena.reset();
            }
        })
    });
}

fn bench_aast_full_parse(c: &mut Criterion, files: &[(RcOc<RelativePath>, &[u8])]) {
    c.bench_function("aast_full_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(RcOc::clone(filename), text);
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(
                    &AastParserEnv::default(),
                    &indexed_source_text,
                    None,
                )
                .unwrap();
            }
        })
    });
}

fn bench_aast_quick_parse(c: &mut Criterion, files: &[(RcOc<RelativePath>, &[u8])]) {
    c.bench_function("aast_quick_parse", |b| {
        b.iter(|| {
            for (filename, text) in files {
                let text = SourceText::make(RcOc::clone(filename), text);
                let indexed_source_text = IndexedSourceText::new(text.clone());
                aast_parser::AastParser::from_text(
                    &AastParserEnv {
                        quick_mode: true,
                        ..AastParserEnv::default()
                    },
                    &indexed_source_text,
                    None,
                )
                .unwrap();
            }
        })
    });
}
