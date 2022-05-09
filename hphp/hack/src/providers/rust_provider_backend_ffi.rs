// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{
    decl_parser::DeclParser,
    file_provider::{FileProvider, FileType, PlainFileProvider},
    folded_decl_provider::FoldedDeclProvider,
};
use ocamlrep::{ptr::UnsafeOcamlPtr, FromOcamlRep};
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::{ocaml_ffi, ocaml_ffi_with_arena, Bump};
use oxidized::{file_info, naming_types};
use oxidized_by_ref::{decl_defs, parser_options::ParserOptions, shallow_decl_defs};
use pos::{RelativePath, RelativePathCtx, ToOxidized};
use std::collections::BTreeSet;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use ty::reason::BReason;

struct ProviderBackend {
    #[allow(dead_code)]
    file_provider: Arc<dyn FileProvider>,
    #[allow(dead_code)]
    folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
}

impl ocamlrep_custom::CamlSerialize for ProviderBackend {
    ocamlrep_custom::caml_serialize_default_impls!();
}

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_make<'a>(
        arena: &'a Bump,
        root: &'a Path,
        hhi_root: &'a Path,
        tmp: &'a Path,
        opts: &'a ParserOptions<'a>,
    ) -> Custom<ProviderBackend> {
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.into(),
            hhi: hhi_root.into(),
            tmp: tmp.into(),
            ..Default::default()
        });
        let file_provider: Arc<dyn FileProvider> = Arc::new(PlainFileProvider::new(Arc::clone(&path_ctx)));
        let folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>> =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                None,
                &DeclParser::with_options(Arc::clone(&file_provider), opts),
                std::iter::empty(),
            );
        Custom::from(ProviderBackend {
            file_provider,
            folded_decl_provider,
        })
    }
}

// Decl_provider ////////////////////////////////////////////////////////////

ocaml_ffi_with_arena! {
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
unsafe fn get_backend(ptr: UnsafeOcamlPtr) -> Custom<ProviderBackend> {
    <Custom<ProviderBackend>>::from_ocamlrep(ptr.as_value()).unwrap()
}

ocaml_ffi! {
    fn hh_rust_provider_backend_decl_provider_push_local_changes(
        _backend: Custom<ProviderBackend>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_decl_provider_pop_local_changes(
        _backend: Custom<ProviderBackend>,
    ) {
        todo!()
    }
}

// File_provider ////////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_file_provider_get(
        backend: Custom<ProviderBackend>,
        path: RelativePath,
    ) -> Option<FileType> {
        backend.file_provider.get(path)
    }

    fn hh_rust_provider_backend_file_provider_get_contents(
        backend: Custom<ProviderBackend>,
        path: RelativePath,
    ) -> Option<bstr::BString> {
        backend.file_provider.get_contents(path).ok().or_else(|| Some("".into()))
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_tests(
        backend: Custom<ProviderBackend>,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        backend.file_provider.provide_file_for_tests(path, contents)
    }

    fn hh_rust_provider_backend_file_provider_provide_file_for_ide(
        backend: Custom<ProviderBackend>,
        path: RelativePath,
        contents: bstr::BString,
    ) {
        backend.file_provider.provide_file_for_ide(path, contents)
    }

    fn hh_rust_provider_backend_file_provider_provide_file_hint(
        backend: Custom<ProviderBackend>,
        path: RelativePath,
        file: FileType,
    ) {
        backend.file_provider.provide_file_hint(path, file)
    }

    fn hh_rust_provider_backend_file_provider_remove_batch(
        backend: Custom<ProviderBackend>,
        paths: BTreeSet<RelativePath>,
    ) {
        backend.file_provider.remove_batch(&paths)
    }

    fn hh_rust_provider_backend_file_provider_push_local_changes(
        backend: Custom<ProviderBackend>,
    ) {
        backend.file_provider.push_local_changes()
    }

    fn hh_rust_provider_backend_file_provider_pop_local_changes(
        backend: Custom<ProviderBackend>,
    ) {
        backend.file_provider.pop_local_changes()
    }
}

// Naming_provider //////////////////////////////////////////////////////////

ocaml_ffi! {
    fn hh_rust_provider_backend_naming_types_add(
        _backend: Custom<ProviderBackend>,
        _name: pos::TypeName,
        _pos: (file_info::Pos, naming_types::KindOfType),
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_get_pos(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::TypeName,
    ) -> Option<(file_info::Pos, naming_types::KindOfType)> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_remove_batch(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::TypeName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_types_get_canon_name(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::TypeName,
    ) -> Option<pos::TypeName> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_add(
        _backend: Custom<ProviderBackend>,
        _name: pos::FunName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_get_pos(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::FunName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_remove_batch(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::FunName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_funs_get_canon_name(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::FunName,
    ) -> Option<pos::FunName> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_add(
        _backend: Custom<ProviderBackend>,
        _name: pos::ConstName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_get_pos(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::ConstName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_consts_remove_batch(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::ConstName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_add(
        _backend: Custom<ProviderBackend>,
        _name: pos::ModuleName,
        _pos: file_info::Pos,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_get_pos(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _name: pos::ModuleName,
    ) -> Option<file_info::Pos> {
        todo!()
    }

    fn hh_rust_provider_backend_naming_modules_remove_batch(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _names: Vec<pos::ModuleName>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_get_filenames_by_hash(
        _backend: Custom<ProviderBackend>,
        _db_path: Option<PathBuf>,
        _deps: Custom<deps_rust::DepSet>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_push_local_changes(
        _backend: Custom<ProviderBackend>,
    ) {
        todo!()
    }

    fn hh_rust_provider_backend_naming_pop_local_changes(
        _backend: Custom<ProviderBackend>,
    ) {
        todo!()
    }
}
