// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ::anyhow::{self, anyhow, Context};

use bumpalo::Bump;
use structopt::StructOpt;
use typing_check_service_rust::{typing_check_utils::from_text, typing_check_utils::*};

use typing_defs_rust::typing_make_type;

use decl_provider_rust as decl_provider;
use oxidized::relative_path::{self, RelativePath};
use oxidized::typing_defs::{Ty, Ty_};
use stack_limit::{StackLimit, KI, MI};

use std::{
    fs::File,
    io::Read,
    path::{Path, PathBuf},
    sync::Arc,
};

// don't consult CARGO_PKG_VERSION (Buck doesn't set it)
#[structopt(no_version)]
#[derive(StructOpt, Clone, Debug)]
// Right now, hh_check accepts only a filename and a verbosity flag
struct Opts {
    /// The level of verbosity (can be set multiple times)
    #[structopt(long = "verbose", parse(from_occurrences))]
    verbosity: isize,

    /// The path to an input Hack file
    #[structopt(name = "FILENAME")]
    filename: PathBuf,
}

/// DeclProvider for tests - assumes that all the declarations come from a single file
struct TestDeclProvider {
    decls: oxidized::direct_decl_parser::Decls,
}

impl TestDeclProvider {
    fn new(path: &RelativePath, text: &[u8]) -> Self {
        let decls = match decl_rust::direct_decl_parser::parse_decls(
            path.clone(),
            &String::from_utf8_lossy(text),
        ) {
            Err(e) => panic!("{:?}", e),
            Ok(decls) => decls,
        };
        Self { decls }
    }
}

impl decl_provider::DeclProvider for TestDeclProvider {
    fn get_fun(&self, s: &str) -> Option<&decl_provider::FunDecl> {
        self.decls.funs.get(s)
    }
    fn get_class(&self, name: &str) -> Option<&decl_provider::ClassDecl> {
        // Unlike the existing OCaml decl provider, classes in the direct decl parser don't seem to
        // have namespace qualification
        // TODO: fix this
        let stripped = name.trim_start_matches("\\");
        self.decls.classes.get(stripped)
    }
    fn get_ancestor<'a>(&self, t: &'a decl_provider::ClassDecl, name: &str) -> Option<&'a Ty> {
        // TODO(hrust): transitively close the hierarchy.
        // Right now, this just gets the direct ancestor
        t.extends.iter().find_map(|ty| match ty.1.as_ref() {
            Ty_::Tapply(sid, _) => {
                if name == sid.1 {
                    Some(ty)
                } else {
                    None
                }
            }
            _ => None,
        })
    }
}

fn read_file(filepath: &Path) -> anyhow::Result<Vec<u8>> {
    let mut text: Vec<u8> = Vec::new();
    File::open(filepath)
        .with_context(|| format!("cannot open input file: {}", filepath.display()))?
        .read_to_end(&mut text)?;
    Ok(text)
}

fn process_single_file_impl(
    opts: &Opts,
    filepath: &Path,
    content: &[u8],
    stack_limit: &StackLimit,
) -> anyhow::Result<Profile> {
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }
    let rel_path = RelativePath::make(relative_path::Prefix::Dummy, filepath.to_owned());
    let arena = Bump::new();
    let builder = typing_make_type::TypeBuilder::new(&arena);
    let provider = TestDeclProvider::new(&rel_path, content);
    let profile = from_text(&builder, &provider, stack_limit, &rel_path, content)?;
    Ok(profile)
}

fn process_single_file_with_retry(
    opts: &Opts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<Profile> {
    let ctx = &Arc::new((opts.clone(), filepath, content));
    let job_builder = move || {
        let new_ctx = Arc::clone(ctx);
        Box::new(
            move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
                let (opts, filepath, content) = new_ctx.as_ref();
                process_single_file_impl(opts, filepath, content.as_slice(), stack_limit)
            },
        )
    };

    // Assume peak is 2.5x of stack.
    // This is initial estimation, need to be improved later.
    let stack_slack = |stack_size| stack_size * 6 / 10;

    let on_retry = &mut |stack_size_tried: usize| {
        // Not always printing warning here because this would fail some HHVM tests
        if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
            eprintln!(
                "[hrust] warning: hh_check exceeded stack of {} KiB on: {}",
                (stack_size_tried - stack_slack(stack_size_tried)) / KI,
                ctx.as_ref().1.display(),
            );
        }
    };

    let job = stack_limit::retry::Job {
        nonmain_stack_min: 13 * MI,
        nonmain_stack_max: None,
        ..Default::default()
    };
    job.with_elastic_stack(&job_builder, on_retry, stack_slack)?
}

// Parse and typecheck a single file, given the command-line options and the file's contents
fn process_single_file(
    opts: &Opts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<Profile> {
    match std::panic::catch_unwind(|| process_single_file_with_retry(opts, filepath, content)) {
        Ok(r) => r,
        Err(panic) => match panic.downcast::<String>() {
            Ok(msg) => Err(anyhow!("panic: {}", msg)),
            Err(_) => Err(anyhow!("panic: unknown")),
        },
    }
}

fn main() -> anyhow::Result<()> {
    // Parse command-line options
    let opts = Opts::from_args();
    if opts.verbosity > 1 {
        eprintln!("hh_check options/flags: {:#?}", opts);
    }
    let filename = opts.filename.clone();
    let content = read_file(&filename)?;
    let r = process_single_file(&opts, filename, content);
    if let Err(e) = r {
        eprintln!("Error in file: {}", e);
    }
    Ok(())
}

#[no_mangle]
pub extern "C" fn hh_check_main() {
    match main() {
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(1)
        }
        Ok(()) => (),
    }
}
