// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{compile::SingleFileOpts, utils};
use anyhow::Result;
use compile::Profile;
use log::info;
use multifile_rust as multifile;
use rayon::prelude::*;
use std::{
    ffi::OsStr,
    hash::{Hash, Hasher},
    io::Write,
    path::{Path, PathBuf},
    sync::{
        atomic::{AtomicBool, AtomicUsize, Ordering},
        Arc, Mutex,
    },
    time::{Duration, Instant},
};
use structopt::StructOpt;

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

#[derive(StructOpt, Clone, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
pub struct Opts {
    #[allow(dead_code)]
    #[structopt(flatten)]
    single_file_opts: SingleFileOpts,

    /// Number of parallel worker threads. By default, or if set to 0, use num-cpu threads.
    #[structopt(long, default_value = "0")]
    num_threads: usize,

    /// The input Hack files or directories to process.
    #[structopt(name = "PATH")]
    paths: Vec<PathBuf>,
}

fn process_one_file(writer: &SyncWrite, f: &Path) -> Result<Profile> {
    let content = utils::read_file(f)?;
    let files = multifile::to_files(f, content)?;
    let mut profile = Profile::default();
    for (f, content) in files {
        let f = f.as_ref();
        let compile_opts = crate::compile::SingleFileOpts {
            dump_symbol_refs: true,
            disable_toplevel_elaboration: false,
            verbosity: 0,
        };
        match crate::compile::process_single_file(&compile_opts, f.into(), content) {
            Err(e) => writeln!(writer.lock().unwrap(), "{}: error ({})", f.display(), e)?,
            Ok((output, prof)) => {
                profile += prof;
                let mut hasher = std::collections::hash_map::DefaultHasher::new();
                output.hash(&mut hasher);
                let crc = hasher.finish();
                writeln!(writer.lock().unwrap(), "{}: {:016x}", f.display(), crc)?;
            }
        }
    }
    Ok(profile)
}

fn to_hms(time: usize) -> String {
    if time >= 5400 {
        // > 90m
        format!(
            "{:02}h:{:02}m:{:02}s",
            time / 3600,
            (time % 3600) / 60,
            time % 60
        )
    } else if time > 90 {
        // > 90s
        format!("{:02}m:{:02}s", time / 60, time % 60)
    } else {
        format!("{}s", time)
    }
}

fn report(wall: Duration, count: usize, profile: Option<ProfileAcc>, total: usize) {
    let wall = wall.as_millis() as usize;
    let wall_per_sec = if wall > 0 { count * 1000 / wall } else { 0 };

    if let Some(profile) = profile {
        // Done, print final stats.
        eprintln!(
            "\rProcessed {} in {:.3}s ({}/s) cpu={:.3}s arenas={:.3}MiB  ",
            count,
            wall as f64 / 1000.0,
            wall_per_sec,
            profile.sum.total_sec(),
            profile.sum.codegen_bytes as f64 / (1024 * 1024) as f64,
        );
        eprintln!(
            "parser stack peak {} in {}",
            profile.max_parse.parse_peak,
            profile.max_parse_file.display()
        );
        eprintln!(
            "lowerer stack peak {} in {}",
            profile.max_lower.lower_peak,
            profile.max_lower_file.display()
        );
        eprintln!(
            "rewrite stack peak {} in {}",
            profile.max_rewrite.rewrite_peak,
            profile.max_rewrite_file.display()
        );
        eprint!(
            "emitter stack peak {} in {}",
            profile.max_emitter.emitter_peak,
            profile.max_emitter_file.display()
        );
    } else {
        let remaining = if wall_per_sec > 0 {
            let left = (total - count) / wall_per_sec;
            format!(", {} remaining", to_hms(left))
        } else {
            "".into()
        };
        eprint!(
            "\rProcessed {} / {} in {:.3}s ({}/s{})            ",
            count,
            total,
            wall as f64 / 1000.0,
            wall_per_sec,
            remaining,
        );
    }
}

#[derive(Default)]
struct ProfileAcc {
    sum: Profile,
    max_parse: Profile,
    max_parse_file: PathBuf,
    max_lower: Profile,
    max_lower_file: PathBuf,
    max_rewrite: Profile,
    max_rewrite_file: PathBuf,
    max_emitter: Profile,
    max_emitter_file: PathBuf,
}

impl ProfileAcc {
    fn fold(mut self, other: Self) -> Self {
        self.sum += other.sum;
        if other.max_parse.parse_peak > self.max_parse.parse_peak {
            self.max_parse = other.max_parse;
            self.max_parse_file = other.max_parse_file;
        }
        if other.max_lower.lower_peak > self.max_lower.lower_peak {
            self.max_lower = other.max_lower;
            self.max_lower_file = other.max_lower_file;
        }
        if other.max_rewrite.rewrite_peak > self.max_rewrite.rewrite_peak {
            self.max_rewrite = other.max_rewrite;
            self.max_rewrite_file = other.max_rewrite_file;
        }
        if other.max_emitter.emitter_peak > self.max_emitter.emitter_peak {
            self.max_emitter = other.max_emitter;
            self.max_emitter_file = other.max_emitter_file;
        }
        self
    }
}

fn crc_files(writer: &SyncWrite, files: &[PathBuf], num_threads: usize) -> Result<()> {
    let total = files.len();
    let count = Arc::new(AtomicUsize::new(0));
    let finished = Arc::new(AtomicBool::new(false));
    let start = Instant::now();

    let status_handle = {
        let count = count.clone();
        let finished = finished.clone();
        std::thread::spawn(move || {
            while !finished.load(Ordering::Acquire) {
                report(start.elapsed(), count.load(Ordering::Acquire), None, total);
                std::thread::sleep(Duration::from_millis(1000));
            }
        })
    };

    let count_one_file = |acc: ProfileAcc, f: &PathBuf| -> Result<ProfileAcc> {
        let profile = process_one_file(writer, f.as_path())?;
        count.fetch_add(1, Ordering::Release);
        Ok(acc.fold(ProfileAcc {
            sum: profile.clone(),
            max_parse: profile.clone(),
            max_parse_file: f.clone(),
            max_lower: profile.clone(),
            max_lower_file: f.clone(),
            max_rewrite: profile.clone(),
            max_rewrite_file: f.clone(),
            max_emitter: profile,
            max_emitter_file: f.clone(),
        }))
    };

    let profile = if num_threads == 1 {
        files
            .iter()
            .try_fold(ProfileAcc::default(), count_one_file)?
    } else {
        files
            .par_iter()
            .with_max_len(1)
            .try_fold(ProfileAcc::default, count_one_file)
            .try_reduce(ProfileAcc::default, |x, y| Ok(x.fold(y)))?
    };

    finished.store(true, Ordering::Release);
    let duration = start.elapsed();

    status_handle.join().unwrap();
    report(
        duration,
        count.load(Ordering::Acquire),
        Some(profile),
        total,
    );
    eprintln!();

    Ok(())
}

fn collect_files(
    paths: &[PathBuf],
    limit: Option<usize>,
    _num_threads: usize,
) -> Result<Vec<PathBuf>> {
    fn is_php_file_name(file: &OsStr) -> bool {
        use std::os::unix::ffi::OsStrExt;
        let file = file.as_bytes();
        file.ends_with(b".php") || file.ends_with(b".hack")
    }

    let mut files: Vec<(u64, PathBuf)> = paths
        .iter()
        .map(|path| {
            use jwalk::{DirEntry, Result, WalkDir};
            fn on_read_dir(
                _: Option<usize>,
                _: &Path,
                _: &mut (),
                children: &mut Vec<Result<DirEntry<((), ())>>>,
            ) {
                children.retain(|dir_entry_result| {
                    dir_entry_result.as_ref().map_or(false, |dir_entry| {
                        let file_type = &dir_entry.file_type;
                        if file_type.is_file() {
                            is_php_file_name(dir_entry.file_name())
                        } else {
                            true
                        }
                    })
                });
            }
            let walker = WalkDir::new(path).process_read_dir(on_read_dir);

            let mut files = Vec::new();
            for dir_entry in walker {
                let dir_entry = dir_entry?;
                if dir_entry.file_type.is_file() {
                    let len = dir_entry.metadata()?.len();
                    files.push((len, dir_entry.path()));
                }
            }
            Ok(files)
        })
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .flatten()
        .collect();

    // Sort largest first process outliers first, then by path.
    files.sort_unstable_by(|(len1, path1), (len2, path2)| {
        len1.cmp(len2).reverse().then(path1.cmp(path2))
    });
    let mut files: Vec<PathBuf> = files.into_iter().map(|(_, path)| path).collect();
    if let Some(limit) = limit {
        if files.len() > limit {
            files.resize_with(limit, || unreachable!());
        }
    }

    Ok(files)
}

pub fn run(opts: Opts) -> Result<()> {
    let writer: SyncWrite = Mutex::new(Box::new(std::io::stdout()));

    rayon::ThreadPoolBuilder::new()
        .num_threads(opts.num_threads)
        .stack_size(32 * 1024 * 1024)
        .build_global()
        .unwrap();

    info!("Collecting files");
    let files = collect_files(&opts.paths, None, opts.num_threads)?;
    info!("{} files found", files.len());

    crc_files(&writer, &files, opts.num_threads)?;

    Ok(())
}
