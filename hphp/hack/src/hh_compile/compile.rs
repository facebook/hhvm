// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::utils;
use ::anyhow::anyhow;
use compile::{Env, EnvFlags, Profile};
use multifile_rust as multifile;
use options::Options;
use oxidized::relative_path::{self, RelativePath};
use rayon::prelude::*;
use stack_limit::{StackLimit, KI, MI};
use structopt::StructOpt;

use std::{
    fs::File,
    io::{self, stdout, Read, Write},
    iter::Iterator,
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

    /// read a list of files or stdin (one per line) from the file `input-file-list'"
    #[structopt(long)]
    input_file_list: Option<Option<PathBuf>>,

    /// Dump configuration settings
    #[structopt(long)]
    dump_config: bool,

    /// The level of verbosity (can be set multiple times)
    #[structopt(long = "verbose", parse(from_occurrences))]
    verbosity: isize,

    /// The path to an input Hack file (omit if --daemon or --input-file-list)
    #[structopt(name = "FILENAME", required_unless_one = &["daemon", "input-file-list"])]
    filename: Option<PathBuf>,

    /// Disable toplevel definition elaboration
    #[structopt(long)]
    disable_toplevel_elaboration: bool,

    #[structopt(long)]
    use_hhbc_by_ref: bool,

    #[structopt(long)]
    thread_num: Option<usize>,
}

pub fn run(opts: Opts) -> anyhow::Result<()> {
    type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

    if opts.verbosity > 1 {
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
            Some(filename) => utils::read_file_list(&filename)?.collect(),
            None => vec![
                opts.filename
                    .as_ref()
                    .cloned()
                    .ok_or_else(|| anyhow! {"TODO(hrust) support stdin"})?,
            ],
        };

        let process_one_file = |f: &PathBuf| {
            let content = utils::read_file(f)?;
            let files = multifile::to_files(f, content)?;
            for (f, content) in files {
                let f = f.as_ref();
                match process_single_file(&opts, f.into(), content) {
                    Err(e) => write!(
                        writer.lock().unwrap(),
                        "Error in file {}: {}",
                        f.display(),
                        e
                    )?,
                    Ok((output, _profile)) => {
                        writer.lock().unwrap().write_all(output.as_bytes())?
                    }
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
    opts: &Opts,
    filepath: &Path,
    content: &[u8],
    stack_limit: &StackLimit,
) -> anyhow::Result<(String, Option<Profile>)> {
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }

    let rel_path = RelativePath::make(relative_path::Prefix::Dummy, filepath.to_owned());
    let mut output = String::new();
    if opts.use_hhbc_by_ref {
        let mut flags = hhbc_by_ref_compile::EnvFlags::empty();
        flags.set(
            hhbc_by_ref_compile::EnvFlags::DISABLE_TOPLEVEL_ELABORATION,
            opts.disable_toplevel_elaboration,
        );
        let env: hhbc_by_ref_compile::Env<String> = hhbc_by_ref_compile::Env {
            filepath: rel_path,
            config_jsons: vec![],
            config_list: vec![],
            flags,
        };
        hhbc_by_ref_compile::from_text(&env, stack_limit, &mut output, content)?;
        Ok((output, None))
    } else {
        let mut flags = EnvFlags::empty();
        flags.set(
            EnvFlags::DISABLE_TOPLEVEL_ELABORATION,
            opts.disable_toplevel_elaboration,
        );
        let env: Env<String> = Env {
            filepath: rel_path,
            config_jsons: vec![],
            config_list: vec![],
            flags,
        };
        let profile = compile::from_text(&env, stack_limit, &mut output, content)?;
        Ok((output, profile))
    }
}

fn process_single_file_with_retry(
    opts: &Opts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<(String, Option<Profile>)> {
    let ctx = &Arc::new((opts.clone(), filepath, content));
    let job_builder = move || {
        let new_ctx = Arc::clone(ctx);
        move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
            let (opts, filepath, content) = new_ctx.as_ref();
            process_single_file_impl(opts, filepath, content.as_slice(), stack_limit)
        }
    };

    // Assume peak is 2.5x of stack.
    // This is initial estimation, need to be improved later.
    let stack_slack = |stack_size| stack_size * 6 / 10;

    let on_retry = &mut |stack_size_tried: usize| {
        // Not always printing warning here because this would fail some HHVM tests
        if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
            eprintln!(
                "[hrust] warning: hh_compile exceeded stack of {} KiB on: {:?}",
                (stack_size_tried - stack_slack(stack_size_tried)) / KI,
                ctx.1,
            );
        }
    };

    let job = stack_limit::retry::Job {
        nonmain_stack_min: 13 * MI,
        nonmain_stack_max: None,
        ..Default::default()
    };
    job.with_elastic_stack(job_builder, on_retry, stack_slack)?
}

fn process_single_file(
    opts: &Opts,
    filepath: PathBuf,
    content: Vec<u8>,
) -> anyhow::Result<(String, Option<Profile>)> {
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
        cli_args: &Vec<String>,
        f: impl FnOnce(&Options) -> T,
    ) -> T {
        self.jsons.push(json);
        let hhbc_options = self.to_options(cli_args);
        let ret = f(&hhbc_options);
        self.jsons.pop();
        ret
    }

    fn to_options(&self, cli_args: &Vec<String>) -> Options {
        Options::from_configs(&self.jsons, cli_args).unwrap()
    }

    fn dump_if_needed(&self, opts: &Opts) {
        if opts.dump_config {
            let hhbc_options = self.to_options(&opts.config_args);
            print!("===CONFIG===\n{}\n\n", hhbc_options.to_string());
            io::stdout().flush().expect("flushing stdout failed");
        }
    }
}
