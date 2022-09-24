// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ty::reason::Reason;

/// A trait which includes only the ProviderBackend functionality necessary to
/// typecheck a file.
pub trait RustProviderBackend<R: Reason> {
    fn file_provider(&self) -> &dyn file_provider::FileProvider;

    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider;

    fn folded_decl_provider(&self) -> &dyn folded_decl_provider::FoldedDeclProvider<R>;

    fn as_any(&self) -> &dyn std::any::Any;
}
