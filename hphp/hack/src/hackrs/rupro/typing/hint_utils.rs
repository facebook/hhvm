// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::TypeName;
use ty::decl;
use ty::reason::Reason;

/// Expose some utility functions to work with AST type hints.
///
/// In particular these utility function convert those AST type hints
/// into intermediate `Ty`s.
pub struct HintUtils;

impl HintUtils {
    pub fn fun_param<R: Reason>(
        fun_param: &oxidized::aast::FunParam<(), ()>,
    ) -> Option<decl::Ty<R>> {
        Self::type_hint(&fun_param.type_hint)
    }

    pub fn type_hint<R: Reason>(type_hint: &oxidized::aast::TypeHint<()>) -> Option<decl::Ty<R>> {
        type_hint.1.as_ref().map(Self::hint)
    }

    pub fn hint<R: Reason>(hint: &oxidized::aast_defs::Hint) -> decl::Ty<R> {
        use oxidized::aast_defs::Hint_::*;
        let r = R::hint(R::Pos::from(&hint.0));
        match &*hint.1 {
            Happly(id, argl) => {
                decl::Ty::apply(r, id.into(), argl.iter().map(Self::hint).collect())
            }
            Hprim(p) => decl::Ty::prim(r, *p),
            Habstr(tparam, hl) => {
                rupro_todo_assert!(hl.is_empty(), HKD);
                decl::Ty::generic(r, TypeName::new(tparam), vec![].into())
            }
            h => rupro_todo!(AST, "{:?}", h),
        }
    }
}
