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
use hackrs::shallow_decl_provider::EagerShallowDeclProvider;
use hackrs::tast;
use hackrs::typing_check_utils::TypingCheckUtils;
use hackrs::typing_ctx::TypingCtx;
use hackrs::typing_decl_provider::FoldingTypingDeclProvider;
use oxidized::global_options::GlobalOptions;
use pos::{Prefix, RelativePath, RelativePathCtx};
use std::path::PathBuf;
use std::rc::Rc;
use std::sync::Arc;
use structopt::StructOpt;
use ty::reason::{BReason, NReason, Reason};

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
    let pool = ocamlrep_ocamlpool::Pool::new();
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

    let ast_provider = AstProvider::new(Arc::clone(&relative_path_ctx), Arc::clone(&options));
    let file_provider: Arc<dyn file_provider::FileProvider> = Arc::new(
        file_provider::PlainFileProvider::new(Arc::clone(&relative_path_ctx)),
    );
    let decl_parser = DeclParser::new(Arc::clone(&file_provider));
    let shallow_decl_store =
        Arc::new(hackrs_test_utils::store::make_non_evicting_shallow_decl_store());
    let shallow_decl_provider = Arc::new(EagerShallowDeclProvider::new(Arc::clone(
        &shallow_decl_store,
    )));
    let folded_decl_store = Arc::new(datastore::NonEvictingStore::new());
    let dependency_registrar = Arc::new(hackrs_test_utils::registrar::DependencyGraph::new());
    let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
        Arc::clone(&options),
        folded_decl_store,
        shallow_decl_provider,
        dependency_registrar,
    ));
    let typing_decl_store = Box::new(datastore::NonEvictingLocalStore::new());
    let typing_decl_provider = Rc::new(FoldingTypingDeclProvider::new(
        typing_decl_store,
        folded_decl_provider,
    ));
    let ctx = Rc::new(TypingCtx::new(typing_decl_provider));

    let filenames: Vec<RelativePath> = cli_options
        .filenames
        .into_iter()
        .map(|fln| RelativePath::new(Prefix::Root, &fln))
        .collect();

    for &filename in &filenames {
        let decls = decl_parser.parse(filename).unwrap();
        shallow_decl_store.add_decls(decls);
    }

    // println!("{:#?}", shallow_decl_provider);

    for fln in filenames {
        let (ast, errs) = ast_provider.get_ast(fln).unwrap();
        let (tast, errs) = TypingCheckUtils::type_file::<R>(Rc::clone(&ctx), &ast, errs).unwrap();
        if !errs.is_empty() {
            for err in errs {
                println!("{:#?}", err);
            }
        }
        print_tast(&options, &tast);
    }
}
