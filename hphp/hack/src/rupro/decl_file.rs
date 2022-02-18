// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{
    alloc,
    cache::NonEvictingCache,
    decl_defs::shallow,
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    reason::Reason,
    shallow_decl_provider::{EagerShallowDeclProvider, ShallowDeclCache},
    special_names::SpecialNames,
    typing_decl_provider::{FoldingTypingDeclProvider, TypingDeclProvider},
};
use pos::{Prefix, RelativePath, RelativePathCtx};
use std::path::PathBuf;
use std::sync::Arc;
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The Hack source files to declare.
    #[structopt(value_name("FILEPATH"))]
    filenames: Vec<PathBuf>,

    /// The repository root (where .hhconfig is), e.g., ~/www
    #[structopt(long)]
    root: Option<PathBuf>,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[structopt(long)]
    with_pos: bool,

    /// Print the shallow decls in the file.
    #[structopt(long)]
    shallow: bool,

    /// Print the folded decls of the classes in the file.
    #[structopt(long)]
    folded: bool,

    /// Print the typing decls (i.e., folded decls with member types) of the classes in the file.
    #[structopt(long)]
    typing: bool,
}

fn main() {
    let mut opts = CliOptions::from_args();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone().unwrap_or_else(PathBuf::new),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let (global_alloc, alloc, pos_alloc) = alloc::get_allocators_for_main();
    let special_names = SpecialNames::new(global_alloc);

    let filenames: Vec<RelativePath> = opts
        .filenames
        .iter()
        .map(|path| {
            if let Some(root) = opts.root.as_ref() {
                if let Ok(suffix) = path.strip_prefix(root) {
                    return alloc.relative_path(Prefix::Root, suffix);
                }
            }
            alloc.relative_path(Prefix::Dummy, path)
        })
        .collect();

    // If no modes were specified, print the shallow decls at least.
    if !opts.shallow && !opts.folded && !opts.typing {
        opts.shallow = true;
    }

    if opts.with_pos {
        decl_files(&opts, pos_alloc, path_ctx, special_names, &filenames);
    } else {
        decl_files(&opts, alloc, path_ctx, special_names, &filenames);
    }
}

fn decl_files<R: Reason>(
    opts: &CliOptions,
    alloc: &'static alloc::Allocator<R>,
    ctx: Arc<RelativePathCtx>,
    special_names: &'static SpecialNames,
    filenames: &[RelativePath],
) {
    let decl_parser = DeclParser::new(alloc, ctx);
    let shallow_decl_cache = Arc::new(ShallowDeclCache::with_no_eviction());
    let shallow_decl_provider = Arc::new(EagerShallowDeclProvider::new(Arc::clone(
        &shallow_decl_cache,
    )));
    let folded_decl_cache = Arc::new(NonEvictingCache::new());
    let folded_decl_provider: Arc<dyn FoldedDeclProvider<R>> =
        Arc::new(LazyFoldedDeclProvider::new(
            folded_decl_cache,
            alloc,
            special_names,
            shallow_decl_provider,
        ));
    let typing_decl_cache = Arc::new(NonEvictingCache::new());
    let typing_decl_provider = Arc::new(FoldingTypingDeclProvider::new(
        typing_decl_cache,
        Arc::clone(&folded_decl_provider),
    ));

    for &path in filenames {
        let decls = decl_parser.parse(path).unwrap();
        shallow_decl_cache.add_decls(decls);
    }

    for &path in filenames {
        for decl in decl_parser.parse(path).unwrap() {
            match decl {
                shallow::Decl::Class(name, decl) => {
                    if opts.shallow {
                        println!("{:#?}", decl);
                    }
                    if opts.folded {
                        println!("{:#?}", folded_decl_provider.get_class(name).unwrap())
                    }
                    if opts.typing {
                        println!("{:#?}", typing_decl_provider.get_class(name).unwrap())
                    }
                }
                shallow::Decl::Fun(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Typedef(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Const(_, decl) => println!("{:#?}", decl),
            }
        }
    }
}
