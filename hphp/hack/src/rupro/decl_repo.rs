// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use rayon::prelude::*;
use structopt::StructOpt;

use hackrs::{
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    reason::{BReason, NReason, Reason},
    shallow_decl_provider::{EagerShallowDeclProvider, ShallowDeclCache},
};
use hackrs_test_utils::cache::NonEvictingCache;
use hackrs_test_utils::serde_cache::{Compression, SerializingCache};
use names::FileSummary;
use pos::{Prefix, RelativePath, RelativePathCtx, TypeName};

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The repository root (where .hhconfig is), e.g., ~/www
    root: PathBuf,

    /// Set the number of threads to use for parsing and folding.
    ///
    /// If omitted or set to 0, uses the rayon default (the value of the
    /// `RAYON_NUM_THREADS` environment variable if set, or the number of
    /// logical CPUs otherwise).
    #[structopt(long)]
    num_threads: Option<usize>,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[structopt(long)]
    with_pos: bool,

    /// In addition to parsing shallow decls, compute folded class decls.
    #[structopt(long)]
    fold: bool,

    /// Store decls in a data store which serializes and compresses them.
    #[structopt(long)]
    serialize: bool,

    /// If `--serialize` was given, use the given compression algorithm.
    #[structopt(default_value, long)]
    compression: Compression,
}

fn main() {
    let opts = CliOptions::from_args();

    if let Some(num_threads) = opts.num_threads {
        rayon::ThreadPoolBuilder::new()
            .num_threads(num_threads)
            .build_global()
            .unwrap();
    }

    let hhi_root = tempdir::TempDir::new("rupro_decl_repo_hhi").unwrap();
    hhi::write_hhi_files(hhi_root.path()).unwrap();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone(),
        hhi: hhi_root.path().into(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let mut filenames: Vec<RelativePath> = find_hack_files(&path_ctx.hhi)
        .map(|path| RelativePath::new(Prefix::Hhi, path.strip_prefix(&path_ctx.hhi).unwrap()))
        .collect();

    let ((), time_taken) = time(|| {
        filenames.extend(find_hack_files(&opts.root).map(|path| {
            match path.strip_prefix(&path_ctx.root) {
                Ok(suffix) => RelativePath::new(Prefix::Root, suffix),
                Err(..) => RelativePath::new(Prefix::Dummy, &path),
            }
        }))
    });
    println!(
        "Collected {} filenames in {:?}",
        filenames.len(),
        time_taken
    );

    if opts.with_pos {
        decl_repo::<BReason>(&opts, path_ctx, filenames);
    } else {
        decl_repo::<NReason>(&opts, path_ctx, filenames);
    }
}

fn decl_repo<R: Reason>(
    opts: &CliOptions,
    ctx: Arc<RelativePathCtx>,
    filenames: Vec<RelativePath>,
) {
    let shallow_decl_cache = make_shallow_cache::<R>(opts);
    let (summaries, time_taken) = time(|| parse(ctx, &shallow_decl_cache, filenames));
    println!("Parsed repo in {:?}", time_taken);
    print_rss();

    let folded_decl_provider = make_folded_provider(opts, shallow_decl_cache);
    if opts.fold {
        let ((), time_taken) = time(|| fold(&folded_decl_provider, summaries));
        println!("Folded repo in {:?}", time_taken);
        print_rss();
    }

    // exit without running destructors (our cache is huge and full of Arcs)
    std::process::exit(0);
}

fn parse<R: Reason>(
    ctx: Arc<RelativePathCtx>,
    shallow_decl_cache: &ShallowDeclCache<R>,
    filenames: Vec<RelativePath>,
) -> BTreeMap<RelativePath, FileSummary> {
    let decl_parser = DeclParser::new(ctx);
    let len = filenames.len();
    filenames
        .into_par_iter()
        .progress_count(len as u64)
        .map(|path| {
            let (decls, summary) = decl_parser.parse_and_summarize(path).unwrap();
            shallow_decl_cache.add_decls(decls);
            (path, summary)
        })
        .collect()
}

fn fold<R: Reason>(
    provider: &impl FoldedDeclProvider<R>,
    summaries: BTreeMap<RelativePath, FileSummary>,
) {
    let len = summaries.len();
    summaries
        .into_par_iter()
        .progress_count(len as u64)
        .for_each(|(filename, summary)| {
            for (class, _hash) in summary.classes() {
                let class = TypeName::new(class);
                provider
                    .get_class(class.into(), class)
                    .unwrap_or_else(|e| {
                        panic!("failed to fold class {} in {:?}: {:?}", class, filename, e)
                    })
                    .unwrap_or_else(|| {
                        panic!("failed to look up class {} in {:?}", class, filename)
                    });
            }
        })
}

fn make_shallow_cache<R: Reason>(opts: &CliOptions) -> ShallowDeclCache<R> {
    if opts.serialize {
        ShallowDeclCache::new(
            Arc::new(SerializingCache::with_compression(opts.compression)), // types
            Box::new(SerializingCache::with_compression(opts.compression)), // funs
            Box::new(SerializingCache::with_compression(opts.compression)), // consts
            Box::new(SerializingCache::with_compression(opts.compression)), // modules
            Box::new(SerializingCache::with_compression(opts.compression)), // properties
            Box::new(SerializingCache::with_compression(opts.compression)), // static_properties
            Box::new(SerializingCache::with_compression(opts.compression)), // methods
            Box::new(SerializingCache::with_compression(opts.compression)), // static_methods
            Box::new(SerializingCache::with_compression(opts.compression)), // constructors
        )
    } else {
        ShallowDeclCache::with_no_member_caches(
            Arc::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        )
    }
}

fn make_folded_provider<R: Reason>(
    opts: &CliOptions,
    shallow_decl_cache: ShallowDeclCache<R>,
) -> impl FoldedDeclProvider<R> {
    LazyFoldedDeclProvider::new(
        Default::default(),
        if opts.serialize {
            Arc::new(SerializingCache::with_compression(opts.compression))
        } else {
            Arc::new(NonEvictingCache::default())
        },
        hackrs::special_names::SpecialNames::new(),
        Arc::new(EagerShallowDeclProvider::new(Arc::new(shallow_decl_cache))),
        Arc::new(hackrs_test_utils::registrar::DependencyGraph::new()),
    )
}

fn find_hack_files(path: impl AsRef<Path>) -> impl Iterator<Item = PathBuf> {
    WalkDir::new(path)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| e.path())
        .filter(|path| find_utils::is_hack(path))
}

fn print_rss() {
    let me = procfs::process::Process::myself().unwrap();
    let page_size = procfs::page_size().unwrap();
    println!(
        "RSS: {:.3}GiB",
        (me.stat.rss * page_size) as f64 / 1024.0 / 1024.0 / 1024.0
    );
}

fn time<T>(f: impl FnOnce() -> T) -> (T, std::time::Duration) {
    let start = std::time::Instant::now();
    let result = f();
    let time_taken = start.elapsed();
    (result, time_taken)
}
