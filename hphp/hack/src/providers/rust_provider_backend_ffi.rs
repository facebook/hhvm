// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs_provider_backend::{FileType, HhServerProviderBackend};
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

struct BackendWrapper(HhServerProviderBackend);

impl std::ops::Deref for BackendWrapper {
    type Target = HhServerProviderBackend;
    fn deref(&self) -> &HhServerProviderBackend {
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
        let backend = HhServerProviderBackend::new(path_ctx, opts).unwrap();
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
        opts: &'a oxidized_by_ref::decl_parser_options::DeclParserOptions<'a>,
        path: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> direct_decl_parser::ParsedFileWithHashes<'a> {
        let backend = unsafe { get_backend(backend) };
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = ocamlrep::bytes_from_ocamlrep(text_value).expect("expected string");
        backend.parse_and_cache_decls(opts, path, text, arena).unwrap()
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
    ) -> Option<shallow_decl_defs::FunDecl<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_fun(name.into(), name)
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
        backend.folded_decl_provider().get_typedef(name.into(), name)
            .unwrap()
            .map(|_td| todo!("td.to_oxidized(arena)"))
    }

    fn hh_rust_provider_backend_get_gconst<'a>(
        arena: &'a Bump,
        backend: UnsafeOcamlPtr,
        name: pos::ConstName,
    ) -> Option<shallow_decl_defs::ConstDecl<'a>> {
        let backend = unsafe { get_backend(backend) };
        backend.folded_decl_provider().get_const(name.into(), name)
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
        backend.folded_decl_provider().get_class(name.into(), name)
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
