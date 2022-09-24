// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeSet;
use std::path::PathBuf;
use std::sync::Arc;

use hackrs_provider_backend::Config;
use hackrs_provider_backend::FileType;
use hackrs_provider_backend::HhServerProviderBackend;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::rc::RcOc;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use ocamlrep_ocamlpool::Bump;
use oxidized::file_info;
use oxidized::naming_types;
use oxidized::parser_options::ParserOptions;
use oxidized_by_ref::direct_decl_parser;
use oxidized_by_ref::shallow_decl_defs;
use pos::RelativePath;
use pos::RelativePathCtx;
use rust_provider_backend_api::RustProviderBackend;
use ty::decl;
use ty::reason::BReason;
use ty::reason::NReason;

pub enum BackendWrapper {
    Positioned(Arc<dyn RustProviderBackend<BReason>>),
    PositionFree(Arc<dyn RustProviderBackend<NReason>>),
}

impl BackendWrapper {
    pub fn positioned(backend: Arc<dyn RustProviderBackend<BReason>>) -> Custom<Self> {
        Custom::from(Self::Positioned(backend))
    }

    pub fn position_free(backend: Arc<dyn RustProviderBackend<NReason>>) -> Custom<Self> {
        Custom::from(Self::PositionFree(backend))
    }

    fn as_hh_server_backend(&self) -> Option<&HhServerProviderBackend> {
        match self {
            Self::Positioned(backend) => backend.as_any().downcast_ref(),
            Self::PositionFree(..) => None,
        }
    }

    pub fn file_provider(&self) -> &dyn file_provider::FileProvider {
        match self {
            Self::Positioned(backend) => backend.file_provider(),
            Self::PositionFree(backend) => backend.file_provider(),
        }
    }

    pub fn naming_provider(&self) -> &dyn naming_provider::NamingProvider {
        match self {
            Self::Positioned(backend) => backend.naming_provider(),
            Self::PositionFree(backend) => backend.naming_provider(),
        }
    }
}

impl ocamlrep_custom::CamlSerialize for BackendWrapper {
    ocamlrep_custom::caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        let backend = self
            .as_hh_server_backend()
            .expect("only HhServerProviderBackend can be serialized");
        let config: Config = backend.config();
        bincode::serialize(&config).unwrap()
    }

    fn deserialize(data: &[u8]) -> Self {
        let config: Config = bincode::deserialize(data).unwrap();
        BackendWrapper::Positioned(Arc::new(HhServerProviderBackend::new(config).unwrap()))
    }
}

type Backend = Custom<BackendWrapper>;

ocaml_ffi! {
    fn hh_rust_provider_backend_register_custom_types() {
        use ocamlrep_custom::CamlSerialize;
        // Safety: The OCaml runtime is currently interrupted by a call into
        // this function, so it's safe to interact with it.
        unsafe {
            BackendWrapper::register();
        }
    }

    fn hh_rust_provider_backend_make(
        root: PathBuf,
        hhi_root: PathBuf,
        tmp: PathBuf,
        opts: ParserOptions,
    ) -> Backend {
        let path_ctx = RelativePathCtx {
            root,
            hhi: hhi_root,
            tmp,
            ..Default::default()
        };
        let backend = Arc::new(HhServerProviderBackend::new(Config {
            path_ctx,
            parser_options: opts,
            db_path: None,
        }).unwrap());
        BackendWrapper::positioned(backend)
    }
}

const UNIMPLEMENTED_MESSAGE: &str = "RustProviderBackend impls other than HhServerProviderBackend \
    only support the minimum functionality necessary for typechecking a file. \
    This API is not supported.";

ocaml_ffi! {
    fn hh_rust_provider_backend_push_local_changes(backend: Backend) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.push_local_changes();
        } else {
            unimplemented!("push_local_changes: {UNIMPLEMENTED_MESSAGE}");
        }
    }

    fn hh_rust_provider_backend_pop_local_changes(backend: Backend) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.pop_local_changes();
        } else {
            unimplemented!("pop_local_changes: {UNIMPLEMENTED_MESSAGE}");
        }
    }
}

// Decl_provider ////////////////////////////////////////////////////////////

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_direct_decl_parse_and_cache<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        path: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> direct_decl_parser::ParsedFileWithHashes<'a> {
        let backend = unsafe { get_backend(backend) };
        let backend = match backend.as_hh_server_backend() {
            Some(backend) => backend,
            None => unimplemented!("direct_decl_parse_and_cache: {UNIMPLEMENTED_MESSAGE}"),
        };
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = ocamlrep::bytes_from_ocamlrep(text_value).expect("expected string");
        backend.parse_and_cache_decls(path, text, arena).unwrap()
    }

    fn hh_rust_provider_backend_add_shallow_decls<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        decls: &[(&'a str, shallow_decl_defs::Decl<'a>)],
    ) {
        let backend = unsafe { get_backend(backend) };
        let backend = match backend.as_hh_server_backend() {
            Some(backend) => backend,
            None => unimplemented!("add_shallow_decls: {UNIMPLEMENTED_MESSAGE}"),
        };
        backend.add_decls(decls).unwrap();
    }
}

// UnsafeOcamlPtr is used because ocamlrep_custom::Custom cannot be used with
// ocaml_ffi_with_arena (it does not implement FromOcamlRepIn, and shouldn't,
// since arena-allocating a Custom would result in failing to decrement the
// inner Rc and leaking memory).
unsafe fn get_backend(ptr: UnsafeOcamlPtr) -> Backend {
    Backend::from_ocamlrep(ptr.as_value()).unwrap()
}

// NB: this function interacts with the OCaml runtime (but won't trigger a GC).
fn to_ocaml<T: ToOcamlRep + ?Sized>(value: &T) -> UnsafeOcamlPtr {
    // SAFETY: this module doesn't do any concurrent interaction with the OCaml
    // runtime while invoking this function
    unsafe { UnsafeOcamlPtr::new(ocamlrep_ocamlpool::to_ocaml(value)) }
}

ocaml_ffi! {
    fn hh_rust_provider_backend_get_fun(
        backend: Backend,
        name: pos::FunName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<Arc<decl::FunDecl<BReason>>> = backend.folded_decl_provider()
                    .get_fun(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<Arc<decl::FunDecl<NReason>>> = backend.folded_decl_provider()
                    .get_fun(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_shallow_class(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<Arc<decl::ShallowClass<BReason>>> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.shallow_decl_provider().get_class(name).unwrap()
        } else {
            unimplemented!("get_shallow_class: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_get_typedef(
        backend: Backend,
        name: pos::TypeName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<Arc<decl::TypedefDecl<BReason>>> = backend.folded_decl_provider()
                    .get_typedef(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<Arc<decl::TypedefDecl<NReason>>> = backend.folded_decl_provider()
                    .get_typedef(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_gconst(
        backend: Backend,
        name: pos::ConstName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<Arc<decl::ConstDecl<BReason>>> = backend.folded_decl_provider()
                    .get_const(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<Arc<decl::ConstDecl<NReason>>> = backend.folded_decl_provider()
                    .get_const(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_module(
        backend: Backend,
        name: pos::ModuleName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<Arc<decl::ModuleDecl<BReason>>> = backend.folded_decl_provider()
                    .get_module(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<Arc<decl::ModuleDecl<NReason>>> = backend.folded_decl_provider()
                    .get_module(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_prop(
        backend: Backend,
        name: (pos::TypeName, pos::PropName),
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<decl::Ty<BReason>> = backend.folded_decl_provider()
                    .get_shallow_property_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<decl::Ty<NReason>> = backend.folded_decl_provider()
                    .get_shallow_property_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_static_prop(
        backend: Backend,
        name: (pos::TypeName, pos::PropName),
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<decl::Ty<BReason>> = backend.folded_decl_provider()
                    .get_shallow_static_property_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<decl::Ty<NReason>> = backend.folded_decl_provider()
                    .get_shallow_static_property_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_method(
        backend: Backend,
        name: (pos::TypeName, pos::MethodName),
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<decl::Ty<BReason>> = backend.folded_decl_provider()
                    .get_shallow_method_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<decl::Ty<NReason>> = backend.folded_decl_provider()
                    .get_shallow_method_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_static_method(
        backend: Backend,
        name: (pos::TypeName, pos::MethodName),
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<decl::Ty<BReason>> = backend.folded_decl_provider()
                    .get_shallow_static_method_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<decl::Ty<NReason>> = backend.folded_decl_provider()
                    .get_shallow_static_method_type(name.0.into(), name.0, name.1)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_constructor(
        backend: Backend,
        name: pos::TypeName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<decl::Ty<BReason>> = backend.folded_decl_provider()
                    .get_shallow_constructor_type(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<decl::Ty<NReason>> = backend.folded_decl_provider()
                    .get_shallow_constructor_type(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_get_folded_class(
        backend: Backend,
        name: pos::TypeName,
    ) -> UnsafeOcamlPtr {
        match &*backend {
            BackendWrapper::Positioned(backend) => {
                let res: Option<Arc<decl::FoldedClass<BReason>>> = backend.folded_decl_provider()
                    .get_class(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
            BackendWrapper::PositionFree(backend) => {
                let res: Option<Arc<decl::FoldedClass<NReason>>> = backend.folded_decl_provider()
                    .get_class(name.into(), name)
                    .unwrap();
                to_ocaml(&res)
            }
        }
    }

    fn hh_rust_provider_backend_declare_folded_class(
        backend: Backend,
        name: pos::TypeName,
    ) {
        match &*backend {
            BackendWrapper::Positioned(backend) => { backend.folded_decl_provider().get_class(name.into(), name).unwrap(); }
            BackendWrapper::PositionFree(backend) => { backend.folded_decl_provider().get_class(name.into(), name).unwrap(); }
        }
    }
}

// File_provider ////////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_file_provider_get(
        backend: Backend,
        path: RelativePath,
    ) -> Option<FileType> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.file_store().get(path).unwrap()
        } else {
            // NB: This is semantically different than the above. We'll read
            // from disk instead of returning None for files that aren't already
            // present in our file store (i.e., the in-memory cache). We'll
            // return Some("") instead of None for files that aren't present on
            // disk.
            Some(FileType::Disk(backend.file_provider().get(path).unwrap()))
        }
    }

    fn hh_rust_provider_backend_file_provider_get_contents(
        backend: Backend,
        path: RelativePath,
    ) -> bstr::BString {
        backend.file_provider().get(path).unwrap()
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_tests(
        backend: Backend,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.file_store().insert(path, FileType::Disk(contents)).unwrap();
        } else {
            unimplemented!("provide_file_for_tests: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_ide(
        backend: Backend,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.file_store().insert(path, FileType::Ide(contents)).unwrap();
        } else {
            unimplemented!("provide_file_for_ide: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_file_provider_provide_file_hint(
        backend: Backend,
        path: RelativePath,
        file: FileType,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            if let FileType::Ide(_) = file {
                backend.file_store().insert(path, file).unwrap();
            }
        } else {
            unimplemented!("provide_file_hint: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_file_provider_remove_batch(
        backend: Backend,
        paths: BTreeSet<RelativePath>,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.file_store().remove_batch(&mut paths.into_iter()).unwrap();
        } else {
            unimplemented!("file_provider_remove_batch: {UNIMPLEMENTED_MESSAGE}")
        }
    }
}

// Naming_provider //////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_naming_types_add(
        backend: Backend,
        name: pos::TypeName,
        pos: (file_info::Pos, naming_types::KindOfType),
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().add_type(name, &pos).unwrap();
        } else {
            unimplemented!("naming_types_add: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_types_get_pos(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<(file_info::Pos, naming_types::KindOfType)> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_type_pos(name).unwrap()
        } else {
            backend.naming_provider()
                .get_type_path_and_kind(name).unwrap()
                .map(|(path, kind)| {
                    (
                        file_info::Pos::File(kind.into(), RcOc::new(path.into())),
                        kind,
                    )
                })
        }
    }

    fn hh_rust_provider_backend_naming_types_remove_batch(
        backend: Backend,
        names: Vec<pos::TypeName>,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().remove_type_batch(&names).unwrap();
        } else {
            unimplemented!("naming_types_remove_batch: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_types_get_canon_name(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<pos::TypeName> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_canon_type_name(name).unwrap()
        } else {
            // TODO: raise an exception or change NamingProvider to include
            // canon methods; this implementation is incorrect
            Some(name)
        }
    }

    fn hh_rust_provider_backend_naming_funs_add(
        backend: Backend,
        name: pos::FunName,
        pos: file_info::Pos,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().add_fun(name, &pos).unwrap();
        } else {
            unimplemented!("naming_funs_add: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_funs_get_pos(
        backend: Backend,
        name: pos::FunName,
    ) -> Option<file_info::Pos> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_fun_pos(name).unwrap()
        } else {
            backend.naming_provider()
                .get_fun_path(name).unwrap()
                .map(|path| file_info::Pos::File(file_info::NameType::Fun, RcOc::new(path.into())))
        }
    }

    fn hh_rust_provider_backend_naming_funs_remove_batch(
        backend: Backend,
        names: Vec<pos::FunName>,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().remove_fun_batch(&names).unwrap();
        } else {
            unimplemented!("naming_funs_remove_batch: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_funs_get_canon_name(
        backend: Backend,
        name: pos::FunName,
    ) -> Option<pos::FunName> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_canon_fun_name(name).unwrap()
        } else {
            // TODO: raise an exception or change NamingProvider to include
            // canon methods; this implementation is incorrect
            Some(name)
        }
    }

    fn hh_rust_provider_backend_naming_consts_add(
        backend: Backend,
        name: pos::ConstName,
        pos: file_info::Pos,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().add_const(name, &pos).unwrap();
        } else {
            unimplemented!("naming_consts_add: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_consts_get_pos(
        backend: Backend,
        name: pos::ConstName,
    ) -> Option<file_info::Pos> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_const_pos(name).unwrap()
        } else {
            backend.naming_provider()
                .get_const_path(name).unwrap()
                .map(|path| file_info::Pos::File(file_info::NameType::Const, RcOc::new(path.into())))
        }
    }

    fn hh_rust_provider_backend_naming_consts_remove_batch(
        backend: Backend,
        names: Vec<pos::ConstName>,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().remove_const_batch(&names).unwrap();
        } else {
            unimplemented!("naming_consts_remove_batch: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_modules_add(
        backend: Backend,
        name: pos::ModuleName,
        pos: file_info::Pos,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().add_module(name, &pos).unwrap();
        } else {
            unimplemented!("naming_modules_add: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_modules_get_pos(
        backend: Backend,
        name: pos::ModuleName,
    ) -> Option<file_info::Pos> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_module_pos(name).unwrap()
        } else {
            backend.naming_provider()
                .get_module_path(name).unwrap()
                .map(|path| file_info::Pos::File(file_info::NameType::Module, RcOc::new(path.into())))
        }
    }

    fn hh_rust_provider_backend_naming_modules_remove_batch(
        backend: Backend,
        names: Vec<pos::ModuleName>,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().remove_module_batch(&names).unwrap();
        } else {
            unimplemented!("naming_modules_remove_batch: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_get_db_path(
        backend: Backend,
    ) -> Option<PathBuf> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().db_path()
        } else {
            unimplemented!("naming_get_db_path: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_set_db_path(
        backend: Backend,
        db_path: PathBuf,
    ) {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().set_db_path(db_path).unwrap()
        } else {
            unimplemented!("naming_set_db_path: {UNIMPLEMENTED_MESSAGE}")
        }
    }

    fn hh_rust_provider_backend_naming_get_filenames_by_hash(
        backend: Backend,
        deps: Custom<deps_rust::DepSet>,
    ) -> std::collections::BTreeSet<RelativePath> {
        if let Some(backend) = backend.as_hh_server_backend() {
            backend.naming_table().get_filenames_by_hash(&deps).unwrap()
        } else {
            unimplemented!("naming_get_filenames_by_hash: {UNIMPLEMENTED_MESSAGE}")
        }
    }
}
