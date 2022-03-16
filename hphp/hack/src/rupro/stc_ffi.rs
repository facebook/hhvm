// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::ast_provider::AstProvider;
use hackrs::decl_parser::DeclParser;
use hackrs::folded_decl_provider::LazyFoldedDeclProvider;
use hackrs::reason::{BReason, NReason, Reason};
use hackrs::shallow_decl_provider::EagerShallowDeclProvider;
use hackrs::special_names::SpecialNames;
use hackrs::tast;
use hackrs::typing_check_utils::TypingCheckUtils;
use hackrs::typing_ctx::TypingCtx;
use hackrs::typing_decl_provider::FoldingTypingDeclProvider;
use hackrs_test_utils::cache::{
    make_non_eviction_shallow_decl_cache, NonEvictingCache, NonEvictingLocalCache,
};
use oxidized::global_options::GlobalOptions;
use pos::{Prefix, RelativePath, RelativePathCtx};
use std::path::PathBuf;
use std::rc::Rc;
use std::sync::Arc;
use structopt::StructOpt;

// fn create_nast(path: PathBuf) -> oxidized::aast::Program<(), ()> {}

fn print_tast<R: Reason>(opts: &GlobalOptions, tast: &tast::Program<R>) {
    unsafe {
        let stc_ffi_print_tast = ocaml_runtime::named_value("stc_ffi_print_tast").unwrap();
        ocaml_runtime::callback_exn(stc_ffi_print_tast, to_ocaml(&(opts, tast))).unwrap();
    }
}

/// Like `ocamlrep_ocamlpool::to_ocaml`, but uses `ocamlrep::Allocator::add`
/// rather than `ocamlrep::Allocator::add_root`, allowing us to use `ToOxidized`
/// in our implementation of `ToOcamlRep` (see impl of `ToOcamlRep` for
/// `typing_defs::Ty`). This means no value-sharing is preserved on the OCaml
/// side.
///
/// # Safety
///
/// No other thread may interact with the OCaml runtime or ocamlpool library
/// during the execution of `to_ocaml`.
unsafe fn to_ocaml<T: ocamlrep::ToOcamlRep>(value: &T) -> usize {
    let mut pool = ocamlrep_ocamlpool::Pool::new();
    let result = pool.add(value);
    result.to_bits()
}

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// Enable positions
    #[structopt(long("--with-pos"))]
    with_pos: bool,

    /// Hack source files
    #[structopt(value_name("FILEPATH"))]
    filenames: Vec<PathBuf>,
}

#[no_mangle]
pub extern "C" fn stc_main() {
    let cli_options = CliOptions::from_args();

    if cli_options.with_pos {
        main_impl::<BReason>(cli_options);
    } else {
        main_impl::<NReason>(cli_options);
    };
}

fn main_impl<R: Reason>(cli_options: CliOptions) {
    let relative_path_ctx = Arc::new(RelativePathCtx {
        root: PathBuf::new(),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });
    let options = Arc::new(oxidized::global_options::GlobalOptions::default());

    let special_names = SpecialNames::new();
    let ast_provider = AstProvider::new(
        Arc::clone(&relative_path_ctx),
        special_names,
        Arc::clone(&options),
    );
    let decl_parser = DeclParser::new(relative_path_ctx);
    let shallow_decl_cache = Arc::new(make_non_eviction_shallow_decl_cache());
    let shallow_decl_provider = Arc::new(EagerShallowDeclProvider::new(Arc::clone(
        &shallow_decl_cache,
    )));
    let folded_decl_cache = Arc::new(NonEvictingCache::new());
    let dependency_registrar = Arc::new(hackrs_test_utils::registrar::DependencyGraph::new());
    let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
        Arc::clone(&options),
        folded_decl_cache,
        special_names,
        shallow_decl_provider,
        dependency_registrar,
    ));
    let typing_decl_cache = Box::new(NonEvictingLocalCache::new());
    let typing_decl_provider = Rc::new(FoldingTypingDeclProvider::new(
        typing_decl_cache,
        folded_decl_provider,
    ));
    let ctx = Rc::new(TypingCtx::new(typing_decl_provider, special_names));

    let filenames: Vec<RelativePath> = cli_options
        .filenames
        .into_iter()
        .map(|fln| RelativePath::new(Prefix::Root, &fln))
        .collect();

    for &filename in &filenames {
        let decls = decl_parser.parse(filename).unwrap();
        shallow_decl_cache.add_decls(decls);
    }

    // println!("{:#?}", shallow_decl_provider);

    for fln in filenames {
        let (ast, errs) = ast_provider.get_ast(fln).unwrap();
        let (tast, errs) = TypingCheckUtils::type_file::<R>(Rc::clone(&ctx), &ast, errs).unwrap();
        if !errs.is_empty() {
            unimplemented!()
        }
        print_tast(&options, &tast);
    }
}
