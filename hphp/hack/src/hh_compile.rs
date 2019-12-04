// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate itertools;
extern crate structopt;
extern crate walkdir;

use itertools::Either::{Left, Right};
use structopt::StructOpt;
use walkdir::WalkDir;

use compile_rust as compile;

use compile::{Env, EnvFlags, Output};
use options::Options;
use oxidized::{
    namespace_env::Env as NamespaceEnv,
    relative_path::{self, RelativePath},
    s_map::SMap,
};

use std::{
    io::{self, Read, Write},
    iter::Iterator,
    path::{Path, PathBuf},
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
    #[structopt(short = "o")]
    output_file: Option<PathBuf>,

    /// Dump configuration settings
    #[structopt(long)]
    dump_config: bool,

    /// Runs very quietly, and ignore any result if invoked without -o
    /// (lower priority than the debug-time option)
    #[structopt(long)]
    quiet_mode: bool,

    /// The level of verbosity (can be set multiple times)
    #[structopt(long = "verbose", parse(from_occurrences))]
    verbosity: isize,

    #[structopt(name = "FILENAME")]
    filename: PathBuf,
}

type OutputHandler = dyn Fn(&Path, Output);

fn process_single_file(opts: &Opts, filepath: &Path, handle_output: &OutputHandler) {
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
    std::fs::File::open(filepath)
        .expect(&format!("cannot open input file: {}", filepath.display()))
        .read_to_end(&mut text)
        .expect("TODO(hrust) error handling");

    let output = compile::from_text(&text, env);
    handle_output(filepath, output);
}

fn write_bytecode(path: &Option<impl AsRef<Path>>, segments: Vec<String>) {
    let mut out = path.as_ref().map_or_else(
        || Left(io::stdout()),
        |path| {
            Right(
                std::fs::OpenOptions::new()
        .write(true)
        .create(true)
        .truncate(false) // see sys_utils.write_strings_to_file in OCaml
        .open(path)
        .expect(&format!("cannot open file for writing: {}", path.as_ref().display())),
            )
        },
    );
    let failed_write_msg = path
        .as_ref()
        .map_or(String::from("failed to write to (stdout)"), |p| {
            format!("failed to write to file: {}", p.as_ref().display())
        });

    for s in segments {
        out.write_all(s.as_bytes()).expect(&failed_write_msg);
    }
}

fn expand_files(file_or_dir: &Path) -> impl Iterator<Item = PathBuf> {
    if !file_or_dir.is_dir() {
        return Left(std::iter::once(file_or_dir.to_owned()));
    }

    // Recursively expand the directory
    Right(
        WalkDir::new(file_or_dir)
            .into_iter()
            .filter_map(|e| e.ok().map(|e| e.into_path()))
            .filter(|e| !e.is_dir()),
    )
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
            std::fs::File::open(config_path)
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

fn main() {
    // use Rc to avoid needing to clone in clone+move in handle_output
    use std::rc::Rc;
    let opts = Rc::new(Opts::from_args());
    if opts.verbosity > 1 {
        eprintln!("hh_compile options/flags: {:#?}", opts);
    }

    // let opts0: &Opts = &opts;
    let config = Rc::new(Config::new(&opts));

    // TODO(hrust) implement handlers for daemon (HHVM) mode
    let handle_output: Box<OutputHandler> = {
        let config = Rc::clone(&config);
        let opts = Rc::clone(&opts);
        if opts.filename.is_dir() {
            Box::new(move |input_path: &Path, output: Output| {
                if let Ok(mut filepath_buf) = input_path.canonicalize() {
                    let extension = filepath_buf.extension().and_then(|os| os.to_str());
                    if let Some("php") = extension {
                    } else {
                        return;
                    }
                    filepath_buf.set_extension("hhas");
                    let output_path = &filepath_buf;
                    if output_path.exists() {
                        if !opts.quiet_mode {
                            eprintln!("Output file {} already exists", output_path.display());
                        }
                    } else {
                        write_bytecode(&Some(output_path), output.bytecode_segments);
                    }
                } else if opts.verbosity > 0 {
                    // Rust-only (guard via the new --verbose flag)
                    eprintln!("Failed to canonicalize path: {}", input_path.display());
                }
            })
        } else {
            // single file mode
            Box::new(move |_: &Path, output: Output| {
                if let None = opts.output_file {
                    config.dump_if_needed(&opts);
                    if opts.quiet_mode {
                        return;
                    }
                };
                write_bytecode(&opts.output_file, output.bytecode_segments);
            })
        }
    };

    // Actually execute the compilation(s)
    for filepath in expand_files(&opts.filename) {
        process_single_file(&opts, &filepath, &*handle_output);
    }
}
