// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ::anyhow::{self, anyhow, Context};

use structopt::StructOpt;

use compile_rust as compile;

use compile::{Env, EnvFlags, Profile};
use hhbc_hhas_rust::{IoWrite, Write as HhWrite};
use itertools::Either::*;
use options::Options;
use oxidized::{
    namespace_env::Env as NamespaceEnv,
    relative_path::{self, RelativePath},
    s_map::SMap,
};

use std::{
    fs::File,
    io::{self, BufRead, Read, Write},
    iter::{once, Iterator, Map},
    path::{Path, PathBuf},
    vec::IntoIter,
};

#[derive(StructOpt, Clone, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
struct Opts {
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
}

fn process_single_file<W>(opts: &Opts, filepath: &Path, writer: &mut W) -> anyhow::Result<Profile>
where
    W: HhWrite,
    W::Error: Send + Sync + 'static,
{
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }

    let rel_path = RelativePath::make(relative_path::Prefix::Dummy, filepath.to_owned());
    let env = Env {
        filepath: rel_path,
        // TODO(hrust) port empty_from_popt
        empty_namespace: NamespaceEnv {
            is_codegen: true,
            ns_uses: SMap::new(),
            class_uses: SMap::new(),
            record_def_uses: SMap::new(),
            fun_uses: SMap::new(),
            const_uses: SMap::new(),
            name: None,
            auto_ns_map: vec![],
        },
        config_jsons: vec![],
        config_list: vec![],
        flags: EnvFlags::empty(),
    };
    let mut text: Vec<u8> = Vec::new();
    File::open(filepath)
        .with_context(|| format!("cannot open input file: {}", filepath.display()))?
        .read_to_end(&mut text)?;
    compile::from_text(&text, env, writer)
}

fn read_file_list(input_path: &Option<PathBuf>) -> anyhow::Result<impl Iterator<Item = PathBuf>> {
    fn read_lines(r: impl Read) -> anyhow::Result<Map<IntoIter<String>, fn(String) -> PathBuf>> {
        Ok(io::BufReader::new(r)
            .lines()
            .collect::<std::io::Result<Vec<_>>>()
            .context("could not read line from input file list")?
            .into_iter()
            .map(|l| PathBuf::from(l.trim())))
    }
    match input_path.as_ref() {
        None => read_lines(io::stdin()),
        Some(path) => read_lines(
            File::open(path)
                .with_context(|| format!("Could not open input file: {}", path.display()))?,
        ),
    }
}

fn assert_regular_file(filepath: impl AsRef<Path>) {
    let filepath = filepath.as_ref();
    if !filepath.is_file() {
        panic!(format!("{} not a valid file", filepath.display()));
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

fn main() -> anyhow::Result<()> {
    let opts = Opts::from_args();
    if opts.verbosity > 1 {
        eprintln!("hh_compile options/flags: {:#?}", opts);
    }
    let config = Config::new(&opts);

    if opts.daemon {
        unimplemented!("TODO(hrust) handlers for daemon (HHVM) mode");
    } else {
        config.dump_if_needed(&opts);
        let (files, mut writer) = match &opts.input_file_list {
            Some(filename) => {
                let files = read_file_list(&filename)?;
                let writer = IoWrite::new(std::io::stdout());
                (Left(files), writer)
            }
            None => {
                let writer = match &opts.output_file {
                    None => IoWrite::new(std::io::stdout()),
                    Some(output_file) => IoWrite::new(File::create(output_file)?),
                };
                let filename = opts
                    .filename
                    .as_ref()
                    .cloned()
                    .ok_or_else(|| anyhow! {"TODO(hrust) support stdin"})?;
                (Right(once(filename)), writer)
            }
        };
        for f in files {
            let r = process_single_file(&opts, &f, &mut writer);
            match r {
                Err(e) => eprintln!("Error in file {}: {}", f.display(), e),
                Ok(_) => writer.flush()?,
            }
        }
        Ok(())
    }
}
