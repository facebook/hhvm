// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;
use std::sync::Arc;

use depgraph_api::DeclName;
use oxidized::ast_defs::Variance;
use pos::FunName;
use pos::MethodName;
use pos::TypeName;
use ty::decl as DTy;
use ty::decl::FunDecl;
use ty::local::Exact;
use ty::local::Ty;
use ty::reason::Reason;

use crate::subtyping::oracle::Oracle;
use crate::typing::typing_error::Error;
use crate::typing::typing_error::Result;
use crate::typing_ctx::TypingCtx;
use crate::typing_decl_provider::Class;
use crate::typing_decl_provider::ClassElt;
use crate::typing_decl_provider::TypeDecl;

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

    /// Get a function type, return `None` if it can't be found.
    pub fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        self.ctx
            .typing_decl_provider
            .get_fun(self.dependent, name)
            .map_err(|e| e.into())
    }

    /// Get the member in a class.
    pub fn get_class_member(
        &self,
        is_method: bool,
        class: &dyn Class<R>,
        member_name: MethodName,
    ) -> Result<Option<Rc<ClassElt<R>>>> {
        if is_method {
            class
                .get_method(self.dependent, member_name)
                .map_err(|e| e.into())
        } else {
            rupro_todo!(MemberAccess);
        }
    }
}

/// A wrapper around `TEnvDecls` that implements the subtyping `Oracle` trait.
#[derive(Debug)]
pub struct TEnvDeclsOracle<R: Reason>(Rc<TEnvDecls<R>>);

impl<R: Reason> TEnvDeclsOracle<R> {
    pub fn new(decls: Rc<TEnvDecls<R>>) -> Self {
        Self(decls)
    }

    pub fn get_class(&self, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
        self.0.get_class(name)
    }
}

impl<R: Reason> Oracle<R> for TEnvDeclsOracle<R> {
    fn get_ancestor(
        &self,
        sub_name: TypeName,
        ty_params: &[Ty<R>],
        sup_name: TypeName,
    ) -> Result<Option<Ty<R>>> {
        let cls_opt = self.get_class(sub_name)?;
        if let Some(cls) = cls_opt {
            match cls.get_ancestor(&sup_name) {
                Some(dty) => match &**dty.node() {
                    DTy::Ty_::Tapply(box (cname, dtys)) => {
                        if dtys.len() == ty_params.len() {
                            let lty = Ty::class(
                                R::none(),
                                cname.clone(),
                                Exact::Nonexact,
                                ty_params.to_vec(),
                            );
                            Ok(Some(lty))
                        } else {
                            Ok(None)
                        }
                    }
                    _ =>
                    // not a class
                    {
                        Ok(None)
                    }
                },
                // no ancestor
                _ => Ok(None),
            }
        } else {
            // TODO[mjt] we're swallowing a failure in Naming here
            Ok(None)
        }
    }

    fn get_variance(&self, name: TypeName) -> Result<Option<Vec<Variance>>> {
        Ok(self
            .get_class(name)?
            .map(|cls| cls.get_tparams().iter().map(|p| p.variance).collect()))
    }

    fn is_final(&self, name: TypeName) -> Result<Option<bool>> {
        Ok(self.get_class(name)?.map(|x| x.is_final()))
    }
}
