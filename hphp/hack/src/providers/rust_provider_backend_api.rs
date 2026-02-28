// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ty::reason::Reason;

/// A trait which includes only the ProviderBackend functionality necessary to
/// typecheck a file.
pub trait RustProviderBackend {
    type Reason: Reason;

    fn file_provider(&self) -> &dyn file_provider::FileProvider;

    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider;

    fn shallow_decl_provider(
        &self,
    ) -> &dyn shallow_decl_provider::ShallowDeclProvider<Self::Reason>;

    fn folded_decl_provider(&self) -> &dyn folded_decl_provider::FoldedDeclProvider<Self::Reason>;

    fn as_any(&self) -> &dyn std::any::Any;
}

impl<T> RustProviderBackend for std::sync::Arc<T>
where
    T: RustProviderBackend,
{
    type Reason = T::Reason;

    fn file_provider(&self) -> &dyn file_provider::FileProvider {
        (**self).file_provider()
    }
    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider {
        (**self).naming_provider()
    }
    fn shallow_decl_provider(
        &self,
    ) -> &dyn shallow_decl_provider::ShallowDeclProvider<Self::Reason> {
        (**self).shallow_decl_provider()
    }
    fn folded_decl_provider(&self) -> &dyn folded_decl_provider::FoldedDeclProvider<Self::Reason> {
        (**self).folded_decl_provider()
    }
    fn as_any(&self) -> &dyn std::any::Any {
        (**self).as_any()
    }
}
