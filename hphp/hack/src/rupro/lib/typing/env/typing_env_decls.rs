// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::dependency_registrar::DeclName;
use crate::typing::typing_error::{Error, Result};
use crate::typing_ctx::TypingCtx;
use crate::typing_decl_provider::{Class, TypeDecl};
use pos::TypeName;
use std::rc::Rc;
use ty::reason::Reason;

/// Provides access to the decl provider, but enforcing dependency tracking.
#[derive(Debug)]
pub struct TEnvDecls<R: Reason> {
    ctx: Rc<TypingCtx<R>>,
    dependent: DeclName,
}

impl<R: Reason> TEnvDecls<R> {
    /// Construct a new decl provider context.
    ///
    /// `dependent` is the current toplevel definition being type checked,
    /// used for dependency tracking.
    pub fn new(ctx: Rc<TypingCtx<R>>, dependent: DeclName) -> Self {
        Self { ctx, dependent }
    }

    /// Get a class, return `None` if it can't be found.
    pub fn get_class(&self, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
        self.ctx
            .typing_decl_provider
            .get_class(self.dependent, name)
            .map_err(|e| e.into())
    }

    /// Get a class, return an error if it can't be found.
    pub fn get_class_or_error(&self, name: TypeName) -> Result<Rc<dyn Class<R>>> {
        self.get_class(name)
            .and_then(|cls| cls.ok_or_else(|| Error::DeclNotFound(name.into())))
    }

    /// Get a type, return `None` if it can't be found.
    pub fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        self.ctx
            .typing_decl_provider
            .get_type(self.dependent, name)
            .map_err(Into::into)
    }
}
