// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::utils;
use ::anyhow::anyhow;
use compile::Profile;
use decl_provider::NoDeclProvider;
use multifile_rust as multifile;
use ocamlrep::rc::RcOc;
use options::Options;
use oxidized::relative_path::{self, RelativePath};
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;
use stack_limit::StackLimit;
use structopt::StructOpt;

use std::{
    fs::File,
    io::{self, stdout, Read, Write},
    path::{Path, PathBuf},
    sync::{Arc, Mutex},
};

#[derive(StructOpt, Clone, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
pub struct Opts {
    /// " Configuration: Server.Port=<value> "
    ///     Allows overriding config options passed on a file
    #[structopt(short = "v")]
    config_args: Vec<String>,

    /// Config file in JSON format
    #[structopt(short = "c")]
    config_file: Option<PathBuf>,

    /// Output file. Creates it if necessary
    #[structopt(short = "o", conflicts_with("input_file_list"))]
    output_file: Option<PathBuf>,

    /// Run a daemon which processes Hack source from standard input
    #[structopt(long)]
    daemon: bool,

    /// Read a list of files or stdin (one per line) from FILE
    #[structopt(long, name = "FILE")]
    input_file_list: Option<Option<PathBuf>>,

    /// Dump configuration settings
    #[structopt(long)]
    dump_config: bool,

    /// The path to an input Hack file (omit if --daemon or --input-file-list)
    #[structopt(name = "FILENAME", required_unless_one = &["daemon", "input-file-list"])]
    filename: Option<PathBuf>,

    #[structopt(flatten)]
    single_file_opts: SingleFileOpts,

    /// Number of parallel worker threads. By default, use num-cpu worker threads.
    /// If 0 or 1, uses the main (single) thread.
    #[structopt(long)]
    thread_num: Option<usize>,
}

#[derive(StructOpt, Clone, Debug)]
pub(crate) struct SingleFileOpts {
    /// Dump symbol ref sections of HHAS
    #[structopt(long)]
    pub(crate) dump_symbol_refs: bool,

    /// Disable toplevel definition elaboration
    #[structopt(long)]
    pub(crate) disable_toplevel_elaboration: bool,

    /// The level of verbosity (can be set multiple times)
    #[structopt(long = "verbose", parse(from_occurrences))]
    pub(crate) verbosity: isize,
}

pub fn run(opts: Opts) -> anyhow::Result<()> {
    type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

    if opts.single_file_opts.verbosity > 1 {
        eprintln!("hh_compile options/flags: {:#?}", opts);
    }
    let config = Config::new(&opts);

    if opts.daemon {
        unimplemented!("TODO(hrust) handlers for daemon (HHVM) mode");
    } else {
        config.dump_if_needed(&opts);

        let writer: SyncWrite = match &opts.output_file {
            None => Mutex::new(Box::new(stdout())),
            Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
        };

        let files: Vec<_> = match &opts.input_file_list {
            Some(filename) => utils::read_file_list(filename.as_ref())?.collect(),
            None => vec![
                opts.filename
                    .as_ref()
                    .cloned()
                    .ok_or_else(|| anyhow!("TODO(hrust) support stdin"))?,
            ],
        };

        let process_one_file = |f: &PathBuf| {
            let content = utils::read_file(f)?;
            let files = multifile::to_files(f, content)?;
            for (f, content) in files {
                let f = f.as_ref();
                match process_single_file(&opts.single_file_opts, f.into(), content) {
                    Err(e) => write!(
                        writer.lock().unwrap(),
                        "Error in file {}: {}",
                        f.display(),
                        e
                    )?,
                    Ok((output, _profile)) => writer.lock().unwrap().write_all(&output)?,
                }
            }
            Ok(())
        };

        if opts.thread_num.map_or(false, |n| n <= 1) {
            files.iter().try_for_each(process_one_file)
        } else {
            opts.thread_num.map_or((), |thread_num| {
                rayon::ThreadPoolBuilder::new()
                    .num_threads(thread_num)
                    .build_global()
                    .unwrap();
            });
            files.par_iter().try_for_each(process_one_file)
        }
    }
}

fn process_single_file_impl(
    opts: &SingleFileOpts,
    filepath: &Path,
    content: &[u8],
    stack_limit: &StackLimit,
) -> anyhow::Result<(Vec<u8>, Profile)> {
    use compile::{Env, EnvFlags, HHBCFlags, NativeEnv, ParserFlags};
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }

    let rel_path = RelativePath::make(relative_path::Prefix::Dummy, filepath.to_owned());
    let source_text = SourceText::make(RcOc::new(rel_path.clone()), content);
    let mut output = Vec::new();
    let mut flags = EnvFlags::empty();
    flags.set(
        EnvFlags::DISABLE_TOPLEVEL_ELABORATION,
        opts.disable_toplevel_elaboration,
    );
    flags.set(EnvFlags::DUMP_SYMBOL_REFS, opts.dump_symbol_refs);
    let hhbc_flags = HHBCFlags::EMIT_CLS_METH_POINTERS
        | HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS
        | HHBCFlags::FOLD_LAZY_CLASS_KEYS
        | HHBCFlags::LOG_EXTERN_COMPILER_PERF;
    let parser_flags = ParserFlags::ENABLE_ENUM_CLASSES;
    let native_env = NativeEnv {
        filepath: rel_path,
        aliased_namespaces: "",
        include_roots: "",
        emit_class_pointers: 0,
        check_int_overflow: 0,
        hhbc_flags,
        parser_flags,
        flags,
    };
    let env = Env {
        filepath: native_env.filepath.clone(),
        config_jsons: Default::default(),
        config_list: Default::default(),
        flags: native_env.flags,
    };
    let alloc = bumpalo::Bump::new();
    let profile = compile::from_text(
        &alloc,
        &env,
        stack_limit,
        &mut output,
        source_text,
        Some(&native_env),
        &NoDeclProvider,
    )?
    .expect("LOG_EXTERN_COMPILE_PERF was set");
    Ok((output, profile))
}

fn process_single_file_with_retry(
    opts: &SingleFileOpts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<(Vec<u8>, Profile)> {
    let ctx = &Arc::new((opts.clone(), filepath, content));
    stack_limit::with_elastic_stack(|stack_limit| {
        let new_ctx = Arc::clone(ctx);
        let (opts, filepath, content) = new_ctx.as_ref();
        process_single_file_impl(opts, filepath, content.as_slice(), stack_limit)
    })?
}

pub(crate) fn process_single_file(
    opts: &SingleFileOpts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<(Vec<u8>, Profile)> {
    match std::panic::catch_unwind(|| process_single_file_with_retry(opts, filepath, content)) {
        Ok(r) => r,
        Err(panic) => match panic.downcast::<String>() {
            Ok(msg) => Err(anyhow!("panic: {}", msg)),
            Err(_) => Err(anyhow!("panic: unknown")),
        },
    }
}

fn assert_regular_file(filepath: impl AsRef<Path>) {
    let filepath = filepath.as_ref();
    if !filepath.is_file() {
        panic!("{} not a valid file", filepath.display());
    }
}

struct Config {
    jsons: Vec<String>,
}
impl Config {
    fn new(opts: &Opts) -> Config {
        let mut ret = Config { jsons: vec![] };

        if let Some(config_path) = opts.config_file.as_ref() {
            assert_regular_file(config_path);
            let mut config_json = String::new();
            File::open(config_path)
                .map(|mut f| {
                    f.read_to_string(&mut config_json)
                        .expect("failed to read config file")
                })
                .expect("failed to open config file");
            ret.jsons.push(config_json);
        };
        ret
    }

    #[allow(dead_code)] // will be used if --daemon (by HHVM)
    fn with_merged<T>(
        &mut self,
        json: String,
        cli_args: &[String],
        f: impl FnOnce(&Options) -> T,
    ) -> T {
        self.jsons.push(json);
        let hhbc_options = self.to_options(cli_args);
        let ret = f(&hhbc_options);
        self.jsons.pop();
        ret
    }

    fn to_options(&self, cli_args: &[String]) -> Options {
        Options::from_configs(&self.jsons, cli_args).unwrap()
    }

    fn dump_if_needed(&self, opts: &Opts) {
        if opts.dump_config {
            let hhbc_options = self.to_options(&opts.config_args);
            print!("===CONFIG===\n{}\n\n", hhbc_options.to_json());
            io::stdout().flush().expect("flushing stdout failed");
        }
    }
}
