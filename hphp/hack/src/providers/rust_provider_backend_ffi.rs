// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::PathBuf;
use std::sync::Arc;

use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use ocamlrep_ocamlpool::Bump;
use oxidized::naming_types;
use pos::RelativePath;
pub use rust_provider_backend_ffi_trait::ProviderBackendFfi;
use ty::decl;
use ty::reason::BReason;

pub struct BackendWrapper(Arc<dyn ProviderBackendFfi>);

impl BackendWrapper {
    pub fn new(provider: Arc<dyn ProviderBackendFfi>) -> Custom<Self> {
        Custom::from(Self(provider))
    }

    pub fn as_any(&self) -> &dyn std::any::Any {
        self.0.as_any()
    }
}

impl ocamlrep_custom::CamlSerialize for BackendWrapper {
    ocamlrep_custom::caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        self.0.serialize()
    }

    fn deserialize(data: &[u8]) -> Self {
        let deserialize = rust_provider_backend_ffi_trait::DESERIALIZE
            .get()
            .unwrap_or_else(|| {
                panic!(
                    "rust_provider_backend_ffi_trait::DESERIALIZE was not initialized in PID {}",
                    std::process::id()
                )
            });
        BackendWrapper(deserialize(data))
    }
}

impl std::ops::Deref for BackendWrapper {
    type Target = dyn ProviderBackendFfi;
    fn deref(&self) -> &Self::Target {
        &*self.0
    }
}

pub type Backend = Custom<BackendWrapper>;

ocaml_ffi! {
    fn hh_rust_provider_backend_register_custom_types() {
        use ocamlrep_custom::CamlSerialize;
        // Safety: The OCaml runtime is currently interrupted by a call into
        // this function, so it's safe to interact with it.
        unsafe {
            BackendWrapper::register();
        }
    }
}

ocaml_ffi! {
    fn hh_rust_provider_backend_push_local_changes(backend: Backend) {
        backend.push_local_changes();
    }

    fn hh_rust_provider_backend_pop_local_changes(backend: Backend) {
        backend.pop_local_changes();
    }

    fn hh_rust_provider_backend_set_ctx_empty(backend: Backend, is_empty: bool) {
        backend.set_ctx_empty(is_empty);
    }
}

// Decl_provider ////////////////////////////////////////////////////////////

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_direct_decl_parse_and_cache<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        path: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> rust_decl_ffi::OcamlParsedFileWithHashes<'a> {
        let backend = unsafe { get_backend(backend) };
        backend.direct_decl_parse_and_cache(path, text, arena).into()
    }

    fn hh_rust_provider_backend_add_shallow_decls<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        decls: &[&(&'a str, oxidized_by_ref::shallow_decl_defs::Decl<'a>)],
    ) {
        let backend = unsafe { get_backend(backend) };
        backend.add_shallow_decls(decls);
    }
}

// UnsafeOcamlPtr is used because ocamlrep_custom::Custom cannot be used with
// ocaml_ffi_with_arena (it does not implement FromOcamlRepIn, and shouldn't,
// since arena-allocating a Custom would result in failing to decrement the
// inner Rc and leaking memory).
unsafe fn get_backend(ptr: UnsafeOcamlPtr) -> Backend {
    Backend::from_ocamlrep(ptr.as_value()).unwrap()
}

// SAFETY: there must be no concurrent interaction with the OCaml runtime while
// this function runs (including the impl of `T::to_ocamlrep`). The present
// implementation of `ocamlrep_ocamlpool::to_ocaml` (as of Oct 2023) guarantees
// no triggering of the OCaml GC.
unsafe fn to_ocaml<T: ToOcamlRep + ?Sized>(value: &T) -> UnsafeOcamlPtr {
    UnsafeOcamlPtr::new(ocamlrep_ocamlpool::to_ocaml(value))
}

ocaml_ffi! {
    fn hh_rust_provider_backend_get_fun(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_fun(name)
    }
    fn hh_rust_provider_backend_get_shallow_class(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_shallow_class(name)
    }
    fn hh_rust_provider_backend_get_typedef(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_typedef(name)
    }
    fn hh_rust_provider_backend_get_gconst(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_gconst(name)
    }
    fn hh_rust_provider_backend_get_module(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_module(name)
    }
    fn hh_rust_provider_backend_get_folded_class(backend: Backend, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        backend.get_folded_class(name)
    }
    fn hh_rust_provider_backend_declare_folded_class(backend: Backend, name: pos::TypeName) {
        backend.declare_folded_class(name)
    }
    fn hh_rust_provider_backend_oldify_defs(backend: Backend, names: file_info::Names) {
        backend.oldify_defs(&names)
    }
    fn hh_rust_provider_backend_remove_old_defs(backend: Backend, names: file_info::Names) {
        backend.remove_old_defs(&names)
    }
    fn hh_rust_provider_backend_remove_defs(backend: Backend, names: file_info::Names) {
        backend.remove_defs(&names)
    }
    fn hh_rust_provider_backend_get_old_defs(
        backend: Backend,
        names: file_info::Names,
    ) -> (
        BTreeMap<pos::TypeName, Option<Arc<decl::ShallowClass<BReason>>>>,
        BTreeMap<pos::FunName, Option<Arc<decl::FunDecl<BReason>>>>,
        BTreeMap<pos::TypeName, Option<Arc<decl::TypedefDecl<BReason>>>>,
        BTreeMap<pos::ConstName, Option<Arc<decl::ConstDecl<BReason>>>>,
        BTreeMap<pos::ModuleName, Option<Arc<decl::ModuleDecl<BReason>>>>,
    ) {
        backend.get_old_defs(&names)
    }
}

// File_provider ////////////////////////////////////////////////////////////

ocaml_ffi! {
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
        backend.file_provider_provide_file_for_tests(path, contents)
    }

    fn hh_rust_provider_backend_file_provider_remove_batch(
        backend: Backend,
        paths: BTreeSet<RelativePath>,
    ) {
        backend.file_provider_remove_batch(paths)
    }
}

// Naming_provider //////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_naming_types_add(
        backend: Backend,
        name: pos::TypeName,
        pos: (file_info::Pos, naming_types::KindOfType),
    ) {
        backend.naming_types_add(name, &pos)
    }

    fn hh_rust_provider_backend_naming_types_get_pos(
        backend: Backend,
        name: UnsafeOcamlPtr,
    ) -> UnsafeOcamlPtr {
        if let Some(ocaml_pos_opt) = backend.get_type_pos(name) {
            return ocaml_pos_opt;
        }
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `backend.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<(file_info::Pos, naming_types::KindOfType)> = backend.naming_provider()
            .get_type_path_and_kind(name).unwrap()
            .map(|(path, kind)| {
                (
                    file_info::Pos::File(kind.into(), Arc::new(path.into())),
                    kind,
                )
            });
        unsafe { to_ocaml(&res) }
    }

    fn hh_rust_provider_backend_naming_types_remove_batch(
        backend: Backend,
        names: Vec<pos::TypeName>,
    ) {
        backend.naming_types_remove_batch(&names);
    }

    fn hh_rust_provider_backend_naming_types_get_canon_name(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<pos::TypeName> {
        backend.naming_provider().get_canon_type_name(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_funs_add(
        backend: Backend,
        name: pos::FunName,
        pos: file_info::Pos,
    ) {
        backend.naming_funs_add(name, &pos);
    }

    fn hh_rust_provider_backend_naming_funs_get_pos(
        backend: Backend,
        name: UnsafeOcamlPtr,
    ) -> UnsafeOcamlPtr {
        if let Some(ocaml_pos_opt) = backend.get_fun_pos(name) {
            return ocaml_pos_opt;
        }
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `backend.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::FunName::from(std::str::from_utf8(name).unwrap());
        let res: Option<file_info::Pos> = backend.naming_provider()
            .get_fun_path(name).unwrap()
            .map(|path| file_info::Pos::File(file_info::NameType::Fun, Arc::new(path.into())));
        unsafe { to_ocaml(&res) }
    }

    fn hh_rust_provider_backend_naming_funs_remove_batch(
        backend: Backend,
        names: Vec<pos::FunName>,
    ) {
        backend.naming_funs_remove_batch(&names);
    }

    fn hh_rust_provider_backend_naming_funs_get_canon_name(
        backend: Backend,
        name: pos::FunName,
    ) -> Option<pos::FunName> {
        backend.naming_provider().get_canon_fun_name(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_consts_add(
        backend: Backend,
        name: pos::ConstName,
        pos: file_info::Pos,
    ) {
        backend.naming_consts_add(name, &pos);
    }

    fn hh_rust_provider_backend_naming_consts_get_pos(
        backend: Backend,
        name: UnsafeOcamlPtr,
    ) -> UnsafeOcamlPtr {
        if let Some(ocaml_pos_opt) = backend.get_const_pos(name) {
            return ocaml_pos_opt;
        }
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `backend.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::ConstName::from(std::str::from_utf8(name).unwrap());
        let res: Option<file_info::Pos> = backend.naming_provider()
            .get_const_path(name).unwrap()
            .map(|path| file_info::Pos::File(file_info::NameType::Const, Arc::new(path.into())));
        unsafe { to_ocaml(&res) }
    }

    fn hh_rust_provider_backend_naming_consts_remove_batch(
        backend: Backend,
        names: Vec<pos::ConstName>,
    ) {
        backend.naming_consts_remove_batch(&names);
    }

    fn hh_rust_provider_backend_naming_modules_add(
        backend: Backend,
        name: pos::ModuleName,
        pos: file_info::Pos,
    ) {
        backend.naming_modules_add(name, &pos);
    }

    fn hh_rust_provider_backend_naming_modules_get_pos(
        backend: Backend,
        name: UnsafeOcamlPtr,
    ) -> UnsafeOcamlPtr {
        if let Some(ocaml_pos_opt) = backend.get_module_pos(name) {
            return ocaml_pos_opt;
        }
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `backend.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::ModuleName::from(std::str::from_utf8(name).unwrap());
        let res: Option<file_info::Pos> = backend.naming_provider()
            .get_module_path(name).unwrap()
            .map(|path| file_info::Pos::File(file_info::NameType::Module, Arc::new(path.into())));
        unsafe { to_ocaml(&res) }
    }

    fn hh_rust_provider_backend_naming_modules_remove_batch(
        backend: Backend,
        names: Vec<pos::ModuleName>,
    ) {
        backend.naming_modules_remove_batch(&names);
    }

    fn hh_rust_provider_backend_naming_get_db_path(
        backend: Backend,
    ) -> Option<PathBuf> {
        backend.naming_get_db_path()
    }

    fn hh_rust_provider_backend_naming_set_db_path(
        backend: Backend,
        db_path: PathBuf,
    ) {
        backend.naming_set_db_path(db_path);
    }

    fn hh_rust_provider_backend_naming_get_filenames_by_hash(
        backend: Backend,
        deps: Custom<deps_rust::DepSet>,
    ) -> std::collections::BTreeSet<RelativePath> {
        backend.naming_get_filenames_by_hash(&deps)
    }
}
