// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hasher;
use std::io::BufRead;
use std::io::ErrorKind;
use std::io::Read;
use std::path::Path;
use std::path::PathBuf;
use std::process::Child;
use std::process::Command;
use std::process::Stdio;

use framing::LineFeedUnescaper;
use itertools::Itertools;
use rayon::iter::ParallelBridge;
use rayon::iter::ParallelIterator;
use structopt::clap::arg_enum;
use structopt::StructOpt;

arg_enum! {
    #[derive(Debug)]
    enum Mode {
        CheckOnly,
        TastOnly,
        Ser,
        SerDe,
    }
}

#[derive(StructOpt, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
#[allow(dead_code)]
struct Opts {
    #[structopt(name = "HH_SINGLE_TYPE_CHECK_BINARY")]
    hh_single_type_check_path: PathBuf,

    /// Do not do any type-related phase: check, serialization, deserialization
    /// (useful for measuring overhead of the benchmark itself)
    #[structopt(long, short = "n")]
    dry_run: bool,

    #[structopt(
        long,
        possible_values = &Mode::variants(),
        case_insensitive = true,
        default_value = "serde",
    )]
    mode: Mode,

    /// The number of threads and concurrent hh_single_type_check processes
    #[structopt(long, default_value = "1")]
    num_workers: u16,

    /// Increase output (-v to print (de)serialized types, -v -v to print Hack errors)
    #[structopt(short, parse(from_occurrences))]
    verbosity: u8,

    #[structopt(long)]
    naming_table: Option<PathBuf>,

    #[structopt(long)]
    www_root: Option<PathBuf>,

    #[structopt(long, default_value = "0")]
    batch_min_bytes: u64,

    /// The (partial) list of input Hack files or directories to process
    /// (the rest is read from the standard input, for practical reasons)
    #[structopt(name = "PATH")]
    paths: Vec<PathBuf>,
}

struct Context {
    proc: Child,
    filepaths: Vec<String>,
    hasher: fnv::FnvHasher,
    verbosity: u8,
}

impl Context {
    fn hash_and_maybe_println(&mut self, bs: &[u8], min_verbosity: u8) {
        self.hasher.write(bs);
        if self.verbosity >= min_verbosity {
            println!("{:02X?}", bs);
        }
    }
}

fn typecheck_only(ctx: &mut Context) {
    let err = || panic!("typechecker reported errors on: {:?}", ctx.filepaths);
    let expected_1st_line = "No errors";
    let mut buf: Vec<u8> = vec![0; expected_1st_line.as_bytes().len()];
    ctx.proc
        .stdout
        .take()
        .unwrap()
        .read_exact(&mut buf)
        .unwrap_or_else(|_| err());
    match std::str::from_utf8(&buf) {
        Ok(actual_1st_line) if actual_1st_line == expected_1st_line => {}
        _ => err(),
    }
}

fn serialize_only(ctx: &mut Context) {
    for bs in framing::read_lines_as_bytes(ctx.proc.stdout.take().unwrap()) {
        ctx.hash_and_maybe_println(&bs, 1);
    }
}

fn ser_de(ctx: &mut Context, arena: &bumpalo::Bump) {
    fn decode(bs: Vec<u8>) -> Vec<u8> {
        lazy_static::lazy_static! {
            static ref LF_UNESCAPER: LineFeedUnescaper = LineFeedUnescaper::new();
        }
        let bs = &LF_UNESCAPER.unescape(&bs);
        lz4::block::decompress(bs, None).unwrap()
    }
    let chunk_iter = framing::read_lines_as_bytes(ctx.proc.stdout.take().unwrap()).map(decode);
    for ty in de::into_types(arena, chunk_iter) {
        ctx.hash_and_maybe_println(format!("{:?}", ty).as_bytes(), 1);
    }
}

fn path_to_str(path: &Path) -> &str {
    path.to_str().expect("non-UTF8 input path")
}

fn batch(
    min_bytes: u64,
    path_iter: impl Iterator<Item = String>,
) -> impl Iterator<Item = Vec<String>> {
    path_iter
        .batching(move |path_it| {
            let mut paths = Vec::<String>::new();
            let mut byte_count: u64 = 0;
            while let Some(path) = path_it.next() {
                byte_count += std::fs::metadata(&path)
                    .unwrap_or_else(|_| panic!("failed to get file size of: {}", path))
                    .len();
                paths.push(path);
                if byte_count >= min_bytes {
                    break;
                }
            }
            if paths.is_empty() { None } else { Some(paths) }
        })
        .into_iter()
}

fn main() {
    let opts = Opts::from_args();
    let www_root = opts.www_root.as_ref().map(|p| path_to_str(p));
    let naming_table = opts.naming_table.as_ref().map(|p| path_to_str(p));
    let job = |filepaths: Vec<String>| {
        let proc = {
            let mut cmd = Command::new(&opts.hh_single_type_check_path);
            if let Mode::CheckOnly = &opts.mode {
                &mut cmd
            } else {
                cmd.arg(if let Mode::TastOnly = &opts.mode {
                    "--tast"
                } else {
                    "--type"
                })
            }
            .arg("--hh-log-level")
            .arg("show")
            .arg("-1")
            .stdin(Stdio::null())
            .stdout(Stdio::piped());
            if let Some(naming_table) = naming_table {
                cmd.arg("--naming-table").arg(naming_table);
                cmd.arg("--root").arg(www_root.unwrap());
            }
            for filepath in &filepaths {
                cmd.arg(filepath);
            }
            if opts.dry_run {
                eprintln!("DRY RUN {} files: {:?}", filepaths.len(), cmd);
                return (true, 0); // don't run a process
            }
            if opts.verbosity < 2 {
                cmd.stderr(Stdio::null())
            } else {
                &mut cmd
            }
            .spawn()
            .expect("failed to run typechecker")
        };
        // Rust recently no longer waits on spawned process in destructors,
        // so do that automatically when the handle goes out of scope, see:
        //   https://doc.rust-lang.org/std/process/struct.Child.html
        //   https://github.com/rust-lang/rust/issues/13854
        impl Drop for Context {
            fn drop(&mut self) {
                match self.proc.kill() {
                    Err(e) if e.kind() == ErrorKind::InvalidInput => { /* already finished */ }
                    Err(_) => eprintln!("failed to kill typechecker with PID: {}", self.proc.id()),
                    Ok(_) => {}
                }
            }
        }

        let mut ctx = Context {
            proc,
            filepaths,
            hasher: Default::default(),
            verbosity: opts.verbosity,
        };
        match &opts.mode {
            Mode::CheckOnly => typecheck_only(&mut ctx),
            Mode::SerDe => {
                let arena = bumpalo::Bump::new();
                ser_de(&mut ctx, &arena);
            }
            _ => serialize_only(&mut ctx),
        }
        let exit = ctx.proc.wait().expect("failed to wait on typechecker");
        (exit.success(), ctx.hasher.finish())
    };

    rayon::ThreadPoolBuilder::new()
        .num_threads(opts.num_workers.into())
        .build_global()
        .unwrap();

    // lazily compute list of files to process in parallel
    let filepaths = {
        let (tx, rx) = std::sync::mpsc::channel();
        // avoid eager eval of for-loop to avoid hitting channel capacity
        opts.paths
            .iter()
            .for_each(|path| tx.send(path_to_str(path).to_owned()).unwrap());
        // note: this is specially important here because of huge input
        std::io::stdin()
            .lock()
            .lines()
            .for_each(|line| tx.send(line.unwrap()).unwrap());
        rx.into_iter()
    };
    let filepaths = batch(opts.batch_min_bytes, filepaths);

    // combine checksums using a _commutative_ monoid (bitwise XOR, 0),
    // since the execution order is arbitrary
    let (ok, checksum) = filepaths
        .par_bridge()
        .map(job)
        .reduce(|| (true, 0u64), |(ok1, n1), (ok2, n2)| (ok1 & ok2, n1 ^ n2));
    if ok {
        println!("output-checksum={:#016x}", checksum);
    } else {
        panic!("some workers failed");
    }
}
