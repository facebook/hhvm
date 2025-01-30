// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use direct_decl_parser::parse_decls_for_bytecode_obr;
use direct_decl_parser::Decls;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;
use parser_core_types::source_text::SourceText;

use crate::DeclProvider;
use crate::Result;
use crate::TypeDecl;

/// A decl provider that also provides decls found in the given file.
///
/// Checks the current file first for decls. If not found,
/// checks the provided decl provider for the decls next.
///
/// Useful when the file under compilation is not indexed by the HHVM autoloader
/// or similar circumstances.
pub struct SelfProvider<'d> {
    fallback_decl_provider: Option<Arc<dyn DeclProvider<'d> + 'd>>,
    decls: Decls<'d>,
}

impl<'d> SelfProvider<'d> {
    pub fn new(
        fallback_decl_provider: Option<Arc<dyn DeclProvider<'d> + 'd>>,
        decl_opts: DeclParserOptions,
        source_text: SourceText<'_>,
        arena: &'d bumpalo::Bump,
    ) -> Self {
        let parsed_file = parse_decls_for_bytecode_obr(
            &decl_opts,
            source_text.file_path().clone(),
            source_text.text(),
            arena,
        );
        SelfProvider {
            fallback_decl_provider,
            decls: parsed_file.decls,
        }
    }

    /// Currently, because decls are not on by default everywhere
    /// only create a new SelfProvider when given a fallback provider,
    /// which indicates that we want to compile with decls.
    /// When decls are turned on everywhere by default and it is no longer optional
    /// this can simply return a nonoption decl provider
    pub fn wrap_existing_provider(
        fallback_decl_provider: Option<Arc<dyn DeclProvider<'d> + 'd>>,
        decl_opts: DeclParserOptions,
        source_text: SourceText<'_>,
        arena: &'d bumpalo::Bump,
    ) -> Option<Arc<dyn DeclProvider<'d> + 'd>> {
        if fallback_decl_provider.is_none() {
            None
        } else {
            Some(Arc::new(SelfProvider::new(
                fallback_decl_provider,
                decl_opts,
                source_text,
                arena,
            )) as Arc<dyn DeclProvider<'d> + 'd>)
        }
    }

    fn result_or_else<T>(
        &self,
        decl: Result<T>,
        get_fallback_decl: impl Fn(&Arc<dyn DeclProvider<'d> + 'd>) -> Result<T>,
    ) -> Result<T> {
        decl.or_else(|_| {
            self.fallback_decl_provider
                .as_ref()
                .map_or(Err(crate::Error::NotFound), get_fallback_decl)
        })
    }
}

impl<'d> DeclProvider<'d> for SelfProvider<'d> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'d>> {
        self.result_or_else(crate::find_type_decl(&self.decls, symbol), |provider| {
            provider.type_decl(symbol, depth)
        })
    }

    fn func_decl(&self, symbol: &str) -> Result<&'d FunDecl<'d>> {
        self.result_or_else(crate::find_func_decl(&self.decls, symbol), |provider| {
            provider.func_decl(symbol)
        })
    }

    fn const_decl(&self, symbol: &str) -> Result<&'d ConstDecl<'d>> {
        self.result_or_else(crate::find_const_decl(&self.decls, symbol), |provider| {
            provider.const_decl(symbol)
        })
    }

    fn module_decl(&self, symbol: &str) -> Result<&'d ModuleDecl<'d>> {
        self.result_or_else(crate::find_module_decl(&self.decls, symbol), |provider| {
            provider.module_decl(symbol)
        })
    }
}

impl<'d> std::fmt::Debug for SelfProvider<'d> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Some(p) = &self.fallback_decl_provider {
            write!(f, "SelfProvider({:?})", p)
        } else {
            write!(f, "SelfProvider")
        }
    }
}
