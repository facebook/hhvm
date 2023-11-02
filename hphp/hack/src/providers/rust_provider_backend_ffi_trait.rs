// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::OnceLock;

use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::ToOcamlRep;
use oxidized::naming_types;
use pos::RelativePath;
use rust_provider_backend_api::RustProviderBackend;
use ty::decl;
use ty::reason::BReason;
use ty::reason::Reason;

/// The trait used to interface with OCaml. A default implementation is provided
/// for implementors of the `RustProviderBackend` trait, but especially
/// perf-sensitive use cases like hh_server may choose to provide their own
/// implementation of `ProviderBackendFfi`.
pub trait ProviderBackendFfi {
    fn file_provider(&self) -> &dyn file_provider::FileProvider;
    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider;

    /// FFI for `ShallowDeclProvider::get_fun`.
    /// OCaml signature: `string -> Shallow_decl_defs.fun_decl option`
    fn get_fun(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// FFI for `ShallowDeclProvider::get_class`.
    /// OCaml signature: `string -> Shallow_decl_defs.class_decl option`
    fn get_shallow_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// FFI for `ShallowDeclProvider::get_typedef`.
    /// OCaml signature: `string -> Shallow_decl_defs.typedef_decl option`
    fn get_typedef(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// FFI for `ShallowDeclProvider::get_const`.
    /// OCaml signature: `string -> Shallow_decl_defs.const_decl option`
    fn get_gconst(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// FFI for `ShallowDeclProvider::get_module`.
    /// OCaml signature: `string -> Shallow_decl_defs.module_decl option`
    fn get_module(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// FFI for `FoldedDeclProvider::get_class`.
    /// OCaml signature: `string -> Decl_defs.decl_class_type option`
    fn get_folded_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr;

    /// Fold the given class (writing to folded decl stores).
    fn declare_folded_class(&self, name: pos::TypeName);

    /// Optional fast path for `NamingProvider::get_type_path_and_kind`.
    ///
    /// If `Some` is returned, the `UnsafeOcamlPtr` is expected to have OCaml
    /// type `(FileInfo.pos * Naming_types.kind_of_type) option`. If `None` is
    /// returned, the FFI will fall back to `Self::naming_provider`.
    ///
    /// OCaml signature: `string -> (FileInfo.pos * Naming_types.kind_of_type) option`
    fn get_type_pos(&self, _name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        None
    }

    /// Optional fast path for `NamingProvider::get_fun_path`.
    ///
    /// If `Some` is returned, the `UnsafeOcamlPtr` is expected to have OCaml
    /// type `FileInfo.pos option`. If `None` is returned, the FFI will fall
    /// back to `Self::naming_provider`.
    ///
    /// OCaml signature: `string -> FileInfo.pos option`
    fn get_fun_pos(&self, _name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        None
    }

    /// Optional fast path for `NamingProvider::get_const_path`.
    ///
    /// If `Some` is returned, the `UnsafeOcamlPtr` is expected to have OCaml
    /// type `FileInfo.pos option`. If `None` is returned, the FFI will fall
    /// back to `Self::naming_provider`.
    ///
    /// OCaml signature: `string -> FileInfo.pos option`
    fn get_const_pos(&self, _name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        None
    }

    /// Optional fast path for `NamingProvider::get_module_path`.
    ///
    /// If `Some` is returned, the `UnsafeOcamlPtr` is expected to have OCaml
    /// type `FileInfo.pos option`. If `None` is returned, the FFI will fall
    /// back to `Self::naming_provider`.
    ///
    /// OCaml signature: `string -> FileInfo.pos option`
    fn get_module_pos(&self, _name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        None
    }

    /// Required for MultiWorker support. On process startup, a `deserialize`
    /// function must also be registered by setting
    /// `rust_provider_backend_ffi_trait::DESERIALIZE`.
    fn serialize(&self) -> Vec<u8> {
        unimplemented!("ProviderBackendFfi::serialize: {UNIMPLEMENTED_MESSAGE}")
    }

    // =========================================================================
    // APIs for driving hh_server follow. They are not invoked in typechecking,
    // so most implementors of `ProviderBackendFfi` should not need to implement
    // these.
    // =========================================================================

    /// Enter a "local changes" scope, where writes to file/naming/decl stores
    /// are ephemeral (and reverted upon the next invocation of
    /// `pop_local_changes`).
    fn push_local_changes(&self) {
        unimplemented!("ProviderBackendFfi::push_local_changes: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Revert the changes in the topmost "local changes" scope.
    fn pop_local_changes(&self) {
        unimplemented!("ProviderBackendFfi::pop_local_changes: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Notify providers about whether the IDE context is empty. When it is
    /// non-empty, it may be necessary to call into OCaml to examine the
    /// contents of IDE buffers when responding to naming/decl queries.
    fn set_ctx_empty(&self, _is_empty: bool) {}

    /// Decl-parse the given file and cache the resulting decls in shallow decl
    /// stores. Return the decls and their hashes.
    fn direct_decl_parse_and_cache<'a>(
        &self,
        _path: RelativePath,
        _text: UnsafeOcamlPtr,
        _arena: &'a bumpalo::Bump,
    ) -> oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes<'a> {
        unimplemented!("ProviderBackendFfi::direct_decl_parse_and_cache: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Write the given decls to shallow decl stores.
    fn add_shallow_decls(&self, _decls: &[&(&str, oxidized_by_ref::shallow_decl_defs::Decl<'_>)]) {
        unimplemented!("ProviderBackendFfi::add_shallow_decls: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Move decls from recently-invalidated files to "old heaps", so that they
    /// can be compared against new versions after reparsing.
    fn oldify_defs(&self, _names: &file_info::Names) {
        unimplemented!("ProviderBackendFfi::oldify_defs: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Remove "old heap" contents after reparsing and comparing.
    fn remove_old_defs(&self, _names: &file_info::Names) {
        unimplemented!("ProviderBackendFfi::remove_old_defs: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Remove the given decls from stores.
    fn remove_defs(&self, _names: &file_info::Names) {
        unimplemented!("ProviderBackendFfi::remove_defs: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Fetch decls from the "old heap" for comparing with new versions.
    fn get_old_defs(
        &self,
        _names: &file_info::Names,
    ) -> (
        BTreeMap<pos::TypeName, Option<Arc<decl::ShallowClass<BReason>>>>,
        BTreeMap<pos::FunName, Option<Arc<decl::FunDecl<BReason>>>>,
        BTreeMap<pos::TypeName, Option<Arc<decl::TypedefDecl<BReason>>>>,
        BTreeMap<pos::ConstName, Option<Arc<decl::ConstDecl<BReason>>>>,
        BTreeMap<pos::ModuleName, Option<Arc<decl::ModuleDecl<BReason>>>>,
    ) {
        unimplemented!("ProviderBackendFfi::get_old_defs: {UNIMPLEMENTED_MESSAGE}")
    }

    /// FFI for OCaml `File_provider.get`, which distinguishes between file
    /// contents on disk and file contents in an IDE buffer. Default impl uses
    /// `Self::file_provider` and assumes the contents are from disk.
    fn file_provider_get(&self, path: RelativePath) -> Option<bstr::BString> {
        Some(self.file_provider().get(path).unwrap())
    }

    /// Add the given contents to the file store. Intended to be used by test
    /// runners only--typically file providers do not need an in-memory cache
    /// for disk contents and can simply read from disk.
    fn file_provider_provide_file_for_tests(&self, _path: RelativePath, _contents: bstr::BString) {
        unimplemented!("ProviderBackendFfi::provide_file_for_tests: {UNIMPLEMENTED_MESSAGE}")
    }

    /// Remove the given paths from the file store.
    fn file_provider_remove_batch(&self, _paths: BTreeSet<RelativePath>) {
        unimplemented!("ProviderBackendFfi::file_provider_remove_batch: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_types_add(
        &self,
        _name: pos::TypeName,
        _pos: &(file_info::Pos, naming_types::KindOfType),
    ) {
        unimplemented!("ProviderBackendFfi::naming_types_add: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_types_remove_batch(&self, _names: &[pos::TypeName]) {
        unimplemented!("ProviderBackendFfi::naming_types_remove_batch: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_funs_add(&self, _name: pos::FunName, _pos: &file_info::Pos) {
        unimplemented!("ProviderBackendFfi::naming_funs_add: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_funs_remove_batch(&self, _names: &[pos::FunName]) {
        unimplemented!("ProviderBackendFfi::naming_funs_remove_batch: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_consts_add(&self, _name: pos::ConstName, _pos: &file_info::Pos) {
        unimplemented!("ProviderBackendFfi::naming_consts_add: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_consts_remove_batch(&self, _names: &[pos::ConstName]) {
        unimplemented!("ProviderBackendFfi::naming_consts_remove_batch: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_modules_add(&self, _name: pos::ModuleName, _pos: &file_info::Pos) {
        unimplemented!("ProviderBackendFfi::naming_modules_add: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_modules_remove_batch(&self, _names: &[pos::ModuleName]) {
        unimplemented!("ProviderBackendFfi::naming_modules_remove_batch: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_get_db_path(&self) -> Option<PathBuf> {
        unimplemented!("ProviderBackendFfi::naming_get_db_path: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_set_db_path(&self, _db_path: PathBuf) {
        unimplemented!("ProviderBackendFfi::naming_set_db_path: {UNIMPLEMENTED_MESSAGE}")
    }

    fn naming_get_filenames_by_hash(
        &self,
        _: &deps_rust::DepSet,
    ) -> std::collections::BTreeSet<RelativePath> {
        unimplemented!("ProviderBackendFfi::naming_get_filenames_by_hash: {UNIMPLEMENTED_MESSAGE}")
    }

    fn as_any(&self) -> &dyn std::any::Any;
}

pub static DESERIALIZE: OnceLock<fn(&[u8]) -> Arc<dyn ProviderBackendFfi>> = OnceLock::new();

const UNIMPLEMENTED_MESSAGE: &str = "The default ProviderBackendFfi impls \
    only support the minimum functionality necessary for typechecking a file. \
    This API is not supported.";

// SAFETY: there must be no concurrent interaction with the OCaml runtime while
// this function runs (including the impl of `T::to_ocamlrep`). The present
// implementation of `ocamlrep_ocamlpool::to_ocaml` (as of Oct 2023) guarantees
// no triggering of the OCaml GC.
unsafe fn to_ocaml<T: ToOcamlRep + ?Sized>(value: &T) -> UnsafeOcamlPtr {
    UnsafeOcamlPtr::new(ocamlrep_ocamlpool::to_ocaml(value))
}

impl<T, R> ProviderBackendFfi for T
where
    T: RustProviderBackend<Reason = R> + 'static,
    R: Reason,
{
    fn file_provider(&self) -> &dyn file_provider::FileProvider {
        RustProviderBackend::file_provider(self)
    }

    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider {
        RustProviderBackend::naming_provider(self)
    }

    fn get_fun(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::FunName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::FunDecl<R>>> =
            self.shallow_decl_provider().get_fun(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn get_shallow_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ShallowClass<R>>> =
            self.shallow_decl_provider().get_class(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn get_typedef(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::TypedefDecl<R>>> =
            self.folded_decl_provider().get_typedef(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn get_gconst(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::ConstName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ConstDecl<R>>> =
            self.shallow_decl_provider().get_const(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn get_module(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::ModuleName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ModuleDecl<R>>> =
            self.shallow_decl_provider().get_module(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn get_folded_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime, so immediately convert to an interned symbol.
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::FoldedClass<R>>> =
            self.folded_decl_provider().get_class(name).unwrap();
        unsafe { to_ocaml(&res) }
    }

    fn declare_folded_class(&self, name: pos::TypeName) {
        let _: Option<Arc<decl::FoldedClass<R>>> =
            self.folded_decl_provider().get_class(name).unwrap();
    }

    fn as_any(&self) -> &dyn std::any::Any {
        self
    }
}
