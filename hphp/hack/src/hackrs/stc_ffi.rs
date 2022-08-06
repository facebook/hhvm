// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::rc::Rc;
use std::sync::Arc;

use decl_parser::DeclParser;
use folded_decl_provider::LazyFoldedDeclProvider;
use hcons::Consable;
use oxidized::global_options::GlobalOptions;
use pos::Prefix;
use pos::RelativePath;
use pos::RelativePathCtx;
use rupro::ast_provider::AstProvider;
use rupro::errors::HackError;
use rupro::tast;
use rupro::tast::TastExpander;
use rupro::typing_check_utils::TypingCheckUtils;
use rupro::typing_ctx::TypingCtx;
use rupro::typing_decl_provider::FoldingTypingDeclProvider;
use shallow_decl_provider::EagerShallowDeclProvider;
use structopt::StructOpt;
use ty::reason::BReason;
use ty::reason::NReason;
use ty::reason::Reason;

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

    #[structopt(long)]
    profile_type_check_multi: Option<i64>,
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
        file_provider::DiskProvider::new(Arc::clone(&relative_path_ctx)),
    );
    let decl_parser = DeclParser::<R>::new(Arc::clone(&file_provider));
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
        .iter()
        .map(|fln| RelativePath::new(Prefix::Root, fln.clone()))
        .collect();

    for &filename in &filenames {
        let decls = decl_parser.parse(filename).unwrap();
        shallow_decl_store.add_decls(decls).unwrap();
    }

    // println!("{:#?}", shallow_decl_provider);
    let (tasts, errs) = check_files(&cli_options, &ast_provider, Rc::clone(&ctx), filenames);
    if !errs.is_empty() {
        for err in errs {
            println!("{:#?}", err);
        }
    }
    for tast in tasts {
        print_tast(&options, &tast);
        println!()
    }
}

fn check_files<R: Reason>(
    cli_options: &CliOptions,
    ast_provider: &AstProvider,
    ctx: Rc<TypingCtx<R>>,
    filenames: Vec<RelativePath>,
) -> (Vec<tast::Program<R>>, Vec<HackError<R>>) {
    let mut tasts = Vec::new();
    let mut all_errs = Vec::new();
    for fln in &filenames {
        let (ast, errs) = ast_provider.get_ast(fln.clone()).unwrap();
        let (mut tast, errs) =
            TypingCheckUtils::type_file::<R>(Rc::clone(&ctx), &ast, errs).unwrap();
        TastExpander::expand_program(&mut tast);
        tasts.push(tast);
        all_errs.extend(errs);
    }

    if let Some(n) = cli_options.profile_type_check_multi {
        let cpu_start = sys_time();
        for _ in 0..n {
            // Clear the hash conser, to simulate freeing types.
            ty::local::Ty_::<R, ty::local::Ty<R>>::conser().clear();
            ty::decl::Ty_::<R>::conser().clear();
            ty::prop::PropF::<R, ty::prop::Prop<R>>::conser().clear();

            // The decl cache is already warm just as in OCaml!
            for fln in &filenames {
                let (ast, errs) = ast_provider.get_ast(fln.clone()).unwrap();
                let (mut tast, _errs) =
                    TypingCheckUtils::type_file::<R>(Rc::clone(&ctx), &ast, errs).unwrap();
                TastExpander::expand_program(&mut tast);
            }
        }
        println!("total warm cpu time (s) - {}", sys_time() - cpu_start);
    }

    (tasts, all_errs)
}

/// Return the processor time. Same implementation as OCaml for optimal
/// comparison. See `caml_sys_time_include_children_unboxed` in
/// `ocaml/runtime/sys.c`.
fn sys_time() -> f64 {
    let mut acc: f64 = 0.0;
    let mut ru = libc::rusage {
        ru_utime: libc::timeval {
            tv_sec: 0,
            tv_usec: 0,
        },
        ru_stime: libc::timeval {
            tv_sec: 0,
            tv_usec: 0,
        },
        ru_maxrss: 0,
        ru_ixrss: 0,
        ru_idrss: 0,
        ru_isrss: 0,
        ru_minflt: 0,
        ru_majflt: 0,
        ru_nswap: 0,
        ru_inblock: 0,
        ru_oublock: 0,
        ru_msgsnd: 0,
        ru_msgrcv: 0,
        ru_nsignals: 0,
        ru_nvcsw: 0,
        ru_nivcsw: 0,
    };

    assert_eq!(unsafe { libc::getrusage(libc::RUSAGE_SELF, &mut ru) }, 0);
    acc += (ru.ru_utime.tv_sec as f64)
        + (ru.ru_utime.tv_usec as f64) / 1e6_f64
        + (ru.ru_stime.tv_sec as f64)
        + (ru.ru_stime.tv_usec as f64) / 1e6_f64;

    assert_eq!(
        unsafe { libc::getrusage(libc::RUSAGE_CHILDREN, &mut ru) },
        0
    );
    acc += (ru.ru_utime.tv_sec as f64)
        + (ru.ru_utime.tv_usec as f64) / 1e6_f64
        + (ru.ru_stime.tv_sec as f64)
        + (ru.ru_stime.tv_usec as f64) / 1e6_f64;

    acc
}
