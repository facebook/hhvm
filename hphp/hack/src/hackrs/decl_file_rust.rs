// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;

use decl_parser::DeclParser;
use folded_decl_provider::FoldedDeclProvider;
use hackrs_test_utils::decl_provider::make_folded_decl_provider;
use hackrs_test_utils::serde_store::StoreOpts;
use hackrs_test_utils::store::make_shallow_decl_store;
use hackrs_test_utils::store::populate_shallow_decl_store;
use jwalk::WalkDir;
use pos::Prefix;
use pos::RelativePath;
use pos::RelativePathCtx;
use rupro::typing_decl_provider::FoldingTypingDeclProvider;
use rupro::typing_decl_provider::TypingDeclProvider;
use structopt::StructOpt;
use ty::decl::shallow;
use ty::reason::BReason;
use ty::reason::NReason;
use ty::reason::Reason;

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The Hack source files to declare.
    #[structopt(value_name("FILEPATH"))]
    filenames: Vec<PathBuf>,

    /// The repository root (where .hhconfig is), e.g., ~/www
    #[structopt(long)]
    root: Option<PathBuf>,

    /// Path to a SQLite naming table (allowing declaration of a single file in a large repo).
    #[structopt(long)]
    naming_table: Option<PathBuf>,

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

    let hhi_root = tempdir::TempDir::new("rupro_decl_file_hhi").unwrap();
    hhi::write_hhi_files(hhi_root.path()).unwrap();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone().unwrap_or_else(PathBuf::new),
        hhi: hhi_root.path().into(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let filenames: Vec<RelativePath> = opts
        .filenames
        .iter()
        .map(|path| {
            if let Some(root) = opts.root.as_ref() {
                if let Ok(suffix) = path.strip_prefix(root) {
                    return RelativePath::new(Prefix::Root, suffix);
                }
            }
            RelativePath::new(Prefix::Dummy, path)
        })
        .collect();

    // If no modes were specified, print the shallow decls at least.
    if !opts.shallow && !opts.folded && !opts.typing {
        opts.shallow = true;
    }

    if opts.with_pos {
        decl_files::<BReason>(&opts, path_ctx, &filenames);
    } else {
        decl_files::<NReason>(&opts, path_ctx, &filenames);
    }
}

fn decl_files<R: Reason>(opts: &CliOptions, ctx: Arc<RelativePathCtx>, filenames: &[RelativePath]) {
    // Add hhi files to the given list of filenames
    let mut all_filenames = WalkDir::new(&ctx.hhi)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| RelativePath::new(Prefix::Hhi, e.path().strip_prefix(&ctx.hhi).unwrap()))
        .collect::<Vec<_>>();
    let file_provider: Arc<dyn file_provider::FileProvider> =
        Arc::new(file_provider::DiskProvider::new(ctx));
    let decl_parser = DeclParser::new(Arc::clone(&file_provider));
    all_filenames.extend(filenames);

    let shallow_decl_store = make_shallow_decl_store(StoreOpts::Unserialized);
    populate_shallow_decl_store(&shallow_decl_store, decl_parser.clone(), &all_filenames);

    let folded_decl_provider = Arc::new(make_folded_decl_provider(
        StoreOpts::Unserialized,
        opts.naming_table.as_ref(),
        shallow_decl_store,
        decl_parser.clone(),
    ));

    let typing_decl_provider = Arc::new(FoldingTypingDeclProvider::new(
        Box::new(datastore::NonEvictingLocalStore::new()),
        Arc::clone(&folded_decl_provider) as Arc<dyn FoldedDeclProvider<R>>,
    ));

    let mut saw_err = false;

    for &path in filenames {
        for decl in decl_parser.parse(path).unwrap() {
            match decl {
                shallow::Decl::Class(name, decl) => {
                    if opts.shallow {
                        println!("{:#?}", decl);
                    }
                    if opts.folded {
                        match folded_decl_provider.get_class(name.into(), name) {
                            Ok(decl) => println!(
                                "{:#?}",
                                decl.expect("expected decl provider to return Some")
                            ),
                            Err(e) => {
                                saw_err = true;
                                eprintln!("Error: {}", e);
                            }
                        }
                    }
                    if opts.typing {
                        match typing_decl_provider.get_class(name.into(), name) {
                            Ok(decl) => println!(
                                "{:#?}",
                                decl.expect("expected decl provider to return Some")
                            ),
                            Err(e) => {
                                saw_err = true;
                                eprintln!("Error: {}", e);
                            }
                        }
                    }
                }
                shallow::Decl::Fun(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Typedef(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Const(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Module(_, decl) => println!("{:#?}", decl),
            }
        }
    }

    if saw_err {
        std::process::exit(1);
    }
}
