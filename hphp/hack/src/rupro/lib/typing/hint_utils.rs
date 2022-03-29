// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::DeclTy;
use crate::reason::Reason;

/// Expose some utility functions to work with AST type hints.
///
/// In particular these utility function convert those AST type hints
/// into intermediate `DeclTy`s.
pub struct HintUtils;

impl HintUtils {
    pub fn fun_param<R: Reason>(fun_param: &oxidized::aast::FunParam<(), ()>) -> Option<DeclTy<R>> {
        Self::type_hint(&fun_param.type_hint)
    }

    pub fn type_hint<R: Reason>(type_hint: &oxidized::aast::TypeHint<()>) -> Option<DeclTy<R>> {
        type_hint.1.as_ref().map(Self::hint)
    }

    pub fn hint<R: Reason>(hint: &oxidized::aast_defs::Hint) -> DeclTy<R> {
        use oxidized::aast_defs::Hint_::*;
        let r = R::hint(R::Pos::from(&hint.0));
        match &*hint.1 {
            Happly(id, argl) => DeclTy::apply(r, id.into(), argl.iter().map(Self::hint).collect()),
            Hprim(p) => DeclTy::prim(r, *p),
            h => rupro_todo!(AST, "{:?}", h),
        }
    }
}
