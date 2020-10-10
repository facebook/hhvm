// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::utils;
use ::anyhow::{anyhow, Context, Result};
use ocamlrep::rc::RcOc;
use oxidized::relative_path::{self, RelativePath};
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};
use rayon::prelude::*;
use stack_limit::{StackLimit, KI, MI};
use std::{
    io::{stdin, BufRead, BufReader},
    iter::Iterator,
    path::PathBuf,
    sync::Arc,
};
use structopt::StructOpt;
use strum_macros::{Display, EnumString};

#[derive(StructOpt, Clone, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
pub struct Opts {
    #[structopt(long)]
    thread_num: Option<usize>,

    #[structopt(default_value, long)]
    parser: Parser,
}

#[derive(Clone, Copy, Debug, Display, EnumString)]
enum Parser {
    Positioned,
    PositionedByRef,
    Minimal,
}

impl std::default::Default for Parser {
    fn default() -> Self {
        Self::Positioned
    }
}

pub fn run(opts: Opts) -> Result<()> {
    let files = BufReader::new(stdin())
        .lines()
        .collect::<std::io::Result<Vec<_>>>()
        .context("could not read line from input file list")?
        .into_iter()
        .map(|l| PathBuf::from(l.trim()))
        .collect::<Vec<_>>();

    opts.thread_num.map_or((), |thread_num| {
        rayon::ThreadPoolBuilder::new()
            .num_threads(thread_num)
            .build_global()
            .unwrap();
    });

    files
        .par_iter()
        .try_for_each(|f| parse_file(opts.parser, f.clone()))
}

fn parse_file(parser: Parser, filepath: PathBuf) -> anyhow::Result<()> {
    let content = utils::read_file(&filepath)?;
    let ctx = &Arc::new((filepath.clone(), content));
    let make_retryable = move || {
        let new_ctx = Arc::clone(ctx);
        Box::new(
            move |
                stack_limit: &StackLimit,
                _nonmain_stack_size: Option<usize>,
            | -> anyhow::Result<()> {
                let env = ParserEnv::default();
                let (filepath, content) = new_ctx.as_ref();
                let path = RelativePath::make(relative_path::Prefix::Dummy, filepath.clone());
                let source_text = SourceText::make(RcOc::new(path), content.as_slice());
                match parser {
                    Parser::PositionedByRef => {
                        let arena = bumpalo::Bump::new();
                        let (_, _, _) = positioned_by_ref_parser::parse_script(
                            &arena,
                            &source_text,
                            env,
                            Some(stack_limit),
                        );
                    }
                    Parser::Positioned => {
                        let (_, _, _) =
                            positioned_parser::parse_script(&source_text, env, Some(stack_limit));
                    }
                    Parser::Minimal => {
                        let (_, _, _) =
                            minimal_parser::parse_script(&source_text, env, Some(stack_limit));
                    }
                }
                Ok(())
            },
        )
    };

    let on_retry = &mut |stack_size_tried: usize| {
        // Not always printing warning here because this would fail some HHVM tests
        if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
            eprintln!(
                "[hrust] warning: exceeded stack of {} KiB on {:?}",
                (stack_size_tried - utils::stack_slack_for_traversal_and_parsing(stack_size_tried))
                    / KI,
                filepath,
            );
        }
    };

    use stack_limit::retry;
    let job = retry::Job {
        nonmain_stack_min: 13 * MI, // assume we need much more if default stack size isn't enough
        nonmain_stack_max: Some(utils::MAX_STACK_SIZE),
        ..Default::default()
    };
    match job.with_elastic_stack(
        &make_retryable,
        on_retry,
        utils::stack_slack_for_traversal_and_parsing,
    ) {
        Ok(_) => Ok(()),
        Err(_) => Err(anyhow!("Stack limit exceed")),
    }
}
