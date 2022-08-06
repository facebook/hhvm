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
use ocamlrep::FromOcamlRep;
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
use ty::decl;
use ty::reason::BReason;

struct BackendWrapper(HhServerProviderBackend);

impl std::ops::Deref for BackendWrapper {
    type Target = HhServerProviderBackend;
    fn deref(&self) -> &HhServerProviderBackend {
        &self.0
    }
}

impl ocamlrep_custom::CamlSerialize for BackendWrapper {
    ocamlrep_custom::caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        let config: Config = self.0.config();
        bincode::serialize(&config).unwrap()
    }

    fn deserialize(data: &[u8]) -> Self {
        let config: Config = bincode::deserialize(data).unwrap();
        Self(HhServerProviderBackend::new(config).unwrap())
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
        let backend = HhServerProviderBackend::new(Config {
            path_ctx,
            parser_options: opts,
            db_path: None,
        }).unwrap();
        Custom::from(BackendWrapper(backend))
    }
}

ocaml_ffi! {
    fn hh_rust_provider_backend_push_local_changes(backend: Backend) {
        backend.push_local_changes();
    }

    fn hh_rust_provider_backend_pop_local_changes(backend: Backend) {
        backend.pop_local_changes();
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
        backend.add_decls(decls).unwrap();
    }

    fn hh_rust_provider_backend_get_fun<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::FunName,
    ) -> Option<Arc<decl::FunDecl<BReason>>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_fun(name.into(), name).unwrap()
    }

    fn hh_rust_provider_backend_get_shallow_class<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<Arc<decl::ShallowClass<BReason>>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_class(name).unwrap()
    }

    fn hh_rust_provider_backend_get_typedef<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<Arc<decl::TypedefDecl<BReason>>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_typedef(name.into(), name).unwrap()
    }

    fn hh_rust_provider_backend_get_gconst<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::ConstName,
    ) -> Option<Arc<decl::ConstDecl<BReason>>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_const(name.into(), name).unwrap()
    }

    fn hh_rust_provider_backend_get_module<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        _name: pos::ModuleName,
    ) -> Option<Arc<decl::ModuleDecl<BReason>>> {
        let _backend = unsafe { get_backend(backend) };
        todo!()
    }

    fn hh_rust_provider_backend_get_prop<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: (pos::TypeName, pos::PropName),
    ) -> Option<decl::Ty<BReason>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_property_type(name.0, name.1).unwrap()
    }

    fn hh_rust_provider_backend_get_static_prop<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: (pos::TypeName, pos::PropName),
    ) -> Option<decl::Ty<BReason>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_static_property_type(name.0, name.1).unwrap()
    }

    fn hh_rust_provider_backend_get_method<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: (pos::TypeName, pos::MethodName),
    ) -> Option<decl::Ty<BReason>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_method_type(name.0, name.1).unwrap()
    }

    fn hh_rust_provider_backend_get_static_method<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: (pos::TypeName, pos::MethodName),
    ) -> Option<decl::Ty<BReason>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_static_method_type(name.0, name.1).unwrap()
    }

    fn hh_rust_provider_backend_get_constructor<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<decl::Ty<BReason>> {
        let backend = unsafe { get_backend(backend) };
        backend.shallow_decl_provider().get_constructor_type(name).unwrap()
    }

    fn hh_rust_provider_backend_get_folded_class<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<Arc<decl::FoldedClass<BReason>>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_class(name.into(), name).unwrap()
    }

    fn hh_rust_provider_backend_declare_folded_class<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_class(name.into(), name).unwrap();
    }
}

// UnsafeOcamlPtr is used because ocamlrep_custom::Custom cannot be used with
// ocaml_ffi_with_arena (it does not implement FromOcamlRepIn, and shouldn't,
// since arena-allocating a Custom would result in failing to decrement the
// inner Rc and leaking memory).
unsafe fn get_backend(ptr: UnsafeOcamlPtr) -> Backend {
    Backend::from_ocamlrep(ptr.as_value()).unwrap()
}

// File_provider ////////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_file_provider_get(
        backend: Backend,
        path: RelativePath,
    ) -> Option<FileType> {
        backend.file_store().get(path).unwrap()
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
        backend.file_store().insert(path, FileType::Disk(contents)).unwrap();
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_ide(
        backend: Backend,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        backend.file_store().insert(path, FileType::Ide(contents)).unwrap();
    }

    fn hh_rust_provider_backend_file_provider_provide_file_hint(
        backend: Backend,
        path: RelativePath,
        file: FileType,
    ) {
        if let FileType::Ide(_) = file {
            backend.file_store().insert(path, file).unwrap();
        }
    }

    fn hh_rust_provider_backend_file_provider_remove_batch(
        backend: Backend,
        paths: BTreeSet<RelativePath>,
    ) {
        backend.file_store().remove_batch(&mut paths.into_iter()).unwrap();
    }
}

// Naming_provider //////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_naming_types_add(
        backend: Backend,
        name: pos::TypeName,
        pos: (file_info::Pos, naming_types::KindOfType),
    ) {
        backend.naming_table().add_type(name, &pos).unwrap();
    }

    fn hh_rust_provider_backend_naming_types_get_pos(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<(file_info::Pos, naming_types::KindOfType)> {
        backend.naming_table().get_type_pos(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_types_remove_batch(
        backend: Backend,
        names: Vec<pos::TypeName>,
    ) {
        backend.naming_table().remove_type_batch(&names).unwrap();
    }

    fn hh_rust_provider_backend_naming_types_get_canon_name(
        backend: Backend,
        name: pos::TypeName,
    ) -> Option<pos::TypeName> {
        backend.naming_table().get_canon_type_name(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_funs_add(
        backend: Backend,
        name: pos::FunName,
        pos: file_info::Pos,
    ) {
        backend.naming_table().add_fun(name, &pos).unwrap();
    }

    fn hh_rust_provider_backend_naming_funs_get_pos(
        backend: Backend,
        name: pos::FunName,
    ) -> Option<file_info::Pos> {
        backend.naming_table().get_fun_pos(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_funs_remove_batch(
        backend: Backend,
        names: Vec<pos::FunName>,
    ) {
        backend.naming_table().remove_fun_batch(&names).unwrap();
    }

    fn hh_rust_provider_backend_naming_funs_get_canon_name(
        backend: Backend,
        name: pos::FunName,
    ) -> Option<pos::FunName> {
        backend.naming_table().get_canon_fun_name(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_consts_add(
        backend: Backend,
        name: pos::ConstName,
        pos: file_info::Pos,
    ) {
        backend.naming_table().add_const(name, &pos).unwrap();
    }

    fn hh_rust_provider_backend_naming_consts_get_pos(
        backend: Backend,
        name: pos::ConstName,
    ) -> Option<file_info::Pos> {
        backend.naming_table().get_const_pos(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_consts_remove_batch(
        backend: Backend,
        names: Vec<pos::ConstName>,
    ) {
        backend.naming_table().remove_const_batch(&names).unwrap();
    }

    fn hh_rust_provider_backend_naming_modules_add(
        backend: Backend,
        name: pos::ModuleName,
        pos: file_info::Pos,
    ) {
        backend.naming_table().add_module(name, &pos).unwrap();
    }

    fn hh_rust_provider_backend_naming_modules_get_pos(
        backend: Backend,
        name: pos::ModuleName,
    ) -> Option<file_info::Pos> {
        backend.naming_table().get_module_pos(name).unwrap()
    }

    fn hh_rust_provider_backend_naming_modules_remove_batch(
        backend: Backend,
        names: Vec<pos::ModuleName>,
    ) {
        backend.naming_table().remove_module_batch(&names).unwrap();
    }

    fn hh_rust_provider_backend_naming_get_db_path(
        backend: Backend,
    ) -> Option<PathBuf> {
        backend.naming_table().db_path()
    }

    fn hh_rust_provider_backend_naming_set_db_path(
        backend: Backend,
        db_path: PathBuf,
    ) {
        backend.naming_table().set_db_path(db_path).unwrap()
    }

    fn hh_rust_provider_backend_naming_get_filenames_by_hash(
        backend: Backend,
        deps: Custom<deps_rust::DepSet>,
    ) -> std::collections::BTreeSet<RelativePath> {
        backend.naming_table().get_filenames_by_hash(&deps).unwrap()
    }
}
