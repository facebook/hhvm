// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use oxidized::ast_defs::Variance;
use pos::TypeName;
use ty::local::Ty;
use ty::reason::Reason;

use crate::typing::typing_error::Result;

pub trait Oracle<R: Reason>: Debug {
    /// Given a class name `C`, its type parameters and another class `D`
    /// return the least common ancestor instantiated at those type params
    fn get_ancestor(
        &self,
        name_sub: TypeName,
        ty_params: &[Ty<R>],
        name_sup: TypeName,
    ) -> Result<Option<Ty<R>>>;

    fn get_variance(&self, name: TypeName) -> Result<Option<Vec<Variance>>>;
    fn is_final(&self, name: TypeName) -> Result<Option<bool>>;
}

#[derive(Debug)]
pub struct NoClasses;

impl<R: Reason> Oracle<R> for NoClasses {
    fn get_ancestor(
        &self,
        _name_sub: TypeName,
        _ty_params: &[Ty<R>],
        _name_sup: TypeName,
    ) -> Result<Option<Ty<R>>> {
        Ok(None)
    }

    fn get_variance(&self, _name: TypeName) -> Result<Option<Vec<Variance>>> {
        Ok(None)
    }

    fn is_final(&self, _name: TypeName) -> Result<Option<bool>> {
        Ok(None)
    }
}
