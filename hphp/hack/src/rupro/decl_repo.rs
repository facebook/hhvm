// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;

use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use rayon::prelude::*;
use structopt::StructOpt;

use hackrs::shallow_decl_provider::{ShallowDeclGlobalCache, ShallowDeclProvider};
use hackrs::{alloc, reason::Reason};
use pos::{Prefix, RelativePath, RelativePathCtx};

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The repository root (where .hhconfig is), e.g., ~/www
    root: PathBuf,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[structopt(long)]
    with_pos: bool,
}

fn main() {
    let cli_options = CliOptions::from_args();

    let (_, alloc, pos_alloc) = alloc::get_allocators_for_main();

    let path_ctx = Arc::new(RelativePathCtx {
        root: cli_options.root.clone(),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let (filenames, time_taken) = time(|| {
        WalkDir::new(cli_options.root)
            .into_iter()
            .filter_map(|e| e.ok())
            .filter(|e| !e.file_type().is_dir() && find_utils::is_hack(&e.path()))
            .map(|e| alloc.relative_path(Prefix::Dummy, &e.path()))
            .collect::<Vec<RelativePath>>()
    });
    println!(
        "Collected {} filenames in {:?}",
        filenames.len(),
        time_taken
    );

    if cli_options.with_pos {
        parse_repo(pos_alloc, path_ctx, &filenames);
    } else {
        parse_repo(alloc, path_ctx, &filenames);
    }
}

fn parse_repo<R: Reason>(
    alloc: &'static alloc::Allocator<R>,
    ctx: Arc<RelativePathCtx>,
    filenames: &[RelativePath],
) {
    let shallow_decl_provider = Arc::new(ShallowDeclProvider::new(
        Arc::new(ShallowDeclGlobalCache::new()),
        alloc,
        ctx,
    ));
    let ((), time_taken) = time(|| {
        filenames
            .par_iter()
            .progress_count(filenames.len() as u64)
            .for_each(|&path| shallow_decl_provider.add_from_file(path).unwrap())
    });
    println!("Parsed {} files in {:?}", filenames.len(), time_taken);

    let me = procfs::process::Process::myself().unwrap();
    let page_size = procfs::page_size().unwrap();
    println!(
        "RSS: {:.3}GiB",
        (me.stat.rss * page_size) as f64 / 1024.0 / 1024.0 / 1024.0
    );

    // exit without running destructors (our cache is huge and full of Arcs)
    std::process::exit(0);
}

fn time<T>(f: impl FnOnce() -> T) -> (T, std::time::Duration) {
    let start = std::time::Instant::now();
    let result = f();
    let time_taken = start.elapsed();
    (result, time_taken)
}
