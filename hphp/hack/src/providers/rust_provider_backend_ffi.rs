// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use file_provider::FileType;
use hackrs_provider_backend::ProviderBackend;
use ocamlrep::{ptr::UnsafeOcamlPtr, FromOcamlRep};
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::{ocaml_ffi, ocaml_ffi_with_arena, Bump};
use oxidized::{file_info, naming_types};
use oxidized_by_ref::{
    decl_defs, direct_decl_parser, parser_options::ParserOptions, shallow_decl_defs,
};
use pos::{RelativePath, RelativePathCtx, ToOxidized};
use std::collections::BTreeSet;
use std::path::{Path, PathBuf};

struct BackendWrapper(ProviderBackend);

impl std::ops::Deref for BackendWrapper {
    type Target = ProviderBackend;
    fn deref(&self) -> &ProviderBackend {
        &self.0
    }
}

impl ocamlrep_custom::CamlSerialize for BackendWrapper {
    ocamlrep_custom::caml_serialize_default_impls!();
}

type Backend = Custom<BackendWrapper>;

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_make<'a>(
        arena: &'a Bump,
        root: &'a Path,
        hhi_root: &'a Path,
        tmp: &'a Path,
        opts: &'a ParserOptions<'a>,
    ) -> Backend {
        let path_ctx = RelativePathCtx {
            root: root.into(),
            hhi: hhi_root.into(),
            tmp: tmp.into(),
            ..Default::default()
        };
        let backend = ProviderBackend::new(path_ctx, opts).unwrap();
        Custom::from(BackendWrapper(backend))
    }
}

ocaml_ffi! {
    fn hh_rust_provider_backend_push_local_changes(backend: Backend) {
        backend.file_provider.push_local_changes();
    }

    fn hh_rust_provider_backend_pop_local_changes(backend: Backend) {
        backend.file_provider.pop_local_changes();
    }
}

// Decl_provider ////////////////////////////////////////////////////////////

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_direct_decl_parse_and_cache<'a>(
        _arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        _path: RelativePath,
    ) -> Option<direct_decl_parser::ParsedFileWithHashes<'a>> {
        let _backend = unsafe { get_backend(backend) };
        todo!()
    }

    fn hh_rust_provider_backend_get_fun<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::FunName,
    ) -> Option<shallow_decl_defs::FunDecl<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider.get_fun(name.into(), name)
            .unwrap()
            .map(|_f| todo!("f.to_oxidized(arena)"))
    }

    fn hh_rust_provider_backend_get_shallow_class<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        _name: pos::TypeName,
    ) -> Option<shallow_decl_defs::ClassDecl<'a>> {
        let _backend = unsafe { get_backend(backend) };
        todo!()
    }

    fn hh_rust_provider_backend_get_typedef<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<shallow_decl_defs::TypedefDecl<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider.get_typedef(name.into(), name)
            .unwrap()
            .map(|_td| todo!("td.to_oxidized(arena)"))
    }

    fn hh_rust_provider_backend_get_gconst<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::ConstName,
    ) -> Option<shallow_decl_defs::ConstDecl<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider.get_const(name.into(), name)
            .unwrap()
            .map(|_c| todo!("c.to_oxidized(arena)"))
    }

    fn hh_rust_provider_backend_get_module<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        _name: pos::ModuleName,
    ) -> Option<shallow_decl_defs::ModuleDecl<'a>> {
        let _backend = unsafe { get_backend(backend) };
        todo!()
    }

    fn hh_rust_provider_backend_get_folded_class<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::TypeName,
    ) -> Option<decl_defs::DeclClassType<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider.get_class(name.into(), name)
            .unwrap()
            .map(|c| c.to_oxidized(arena))
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
        backend.file_provider.get(path)
    }

    fn hh_rust_provider_backend_file_provider_get_contents(
        backend: Backend,
        path: RelativePath,
    ) -> Option<bstr::BString> {
        backend.file_provider.get_contents(path).ok().or_else(|| Some("".into()))
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_tests(
        backend: Backend,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        backend.file_provider.provide_file_for_tests(path, contents)
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_ide(
        backend: Backend,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        backend.file_provider.provide_file_for_ide(path, contents)
    }

    fn hh_rust_provider_backend_file_provider_provide_file_hint(
        backend: Backend,
        path: RelativePath,
        file: FileType,
    ) {
        backend.file_provider.provide_file_hint(path, file)
    }

    fn hh_rust_provider_backend_file_provider_remove_batch(
        backend: Backend,
        paths: BTreeSet<RelativePath>,
    ) {
        backend.file_provider.remove_batch(&paths)
    }
}

// Naming_provider //////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_naming_types_add(
        _backend: Backend,
        _name: pos::TypeName,
        _pos: (file_info::Pos, naming_types::KindOfType),
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_get_pos(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::TypeName,
    ) -> Option<(file_info::Pos, naming_types::KindOfType)> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_remove_batch(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::TypeName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_get_canon_name(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::TypeName,
    ) -> Option<pos::TypeName> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_add(
        _backend: Backend,
        _name: pos::FunName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_get_pos(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::FunName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_remove_batch(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::FunName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_get_canon_name(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::FunName,
    ) -> Option<pos::FunName> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_add(
        _backend: Backend,
        _name: pos::ConstName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_get_pos(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::ConstName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_remove_batch(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::ConstName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_add(
        _backend: Backend,
        _name: pos::ModuleName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_get_pos(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _name: pos::ModuleName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_remove_batch(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::ModuleName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_get_filenames_by_hash(
        _backend: Backend,
        _db_path: Option<PathBuf>,
        _deps: Custom<deps_rust::DepSet>,
    ) {
        todo!()
    }
}
