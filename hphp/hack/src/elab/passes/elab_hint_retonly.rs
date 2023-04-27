// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Hint;
use nast::Hint_;
use nast::Tprim;
use oxidized::naming_error::ReturnOnlyHint;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabHintRetonlyPass {
    allow_retonly: bool,
}

impl Pass for ElabHintRetonlyPass {
    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        match elem {
            Hint(pos, box hint_ @ Hint_::Hprim(Tprim::Tvoid)) if !self.allow_retonly => {
                env.emit_error(NamingError::ReturnOnlyTypehint {
                    pos: pos.clone(),
                    kind: ReturnOnlyHint::Hvoid,
                });
                *hint_ = Hint_::Herr;
                Break(())
            }
            Hint(pos, box hint_ @ Hint_::Hprim(Tprim::Tnoreturn)) if !self.allow_retonly => {
                env.emit_error(NamingError::ReturnOnlyTypehint {
                    pos: pos.clone(),
                    kind: ReturnOnlyHint::Hnoreturn,
                });
                *hint_ = Hint_::Herr;
                Break(())
            }
            _ => Continue(()),
        }
    }

    fn on_ty_hint__top_down(&mut self, _: &Env, elem: &mut Hint_) -> ControlFlow<()> {
        match elem {
            Hint_::Happly(..) | Hint_::Habstr(..) => self.allow_retonly = true,
            _ => (),
        }
        Continue(())
    }

    fn on_ty_targ_top_down(&mut self, _: &Env, _: &mut nast::Targ) -> ControlFlow<()> {
        self.allow_retonly = true;
        Continue(())
    }

    fn on_fld_hint_fun_return_ty_top_down(&mut self, _: &Env, _: &mut Hint) -> ControlFlow<()> {
        self.allow_retonly = true;
        Continue(())
    }

    fn on_fld_fun__ret_top_down(&mut self, _: &Env, _: &mut nast::TypeHint) -> ControlFlow<()> {
        self.allow_retonly = true;
        Continue(())
    }

    fn on_fld_method__ret_top_down(&mut self, _: &Env, _: &mut nast::TypeHint) -> ControlFlow<()> {
        self.allow_retonly = true;
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Block;
    use nast::FunParam;
    use nast::Fun_;
    use nast::FuncBody;
    use nast::HintFun;
    use nast::Id;
    use nast::ParamKind;
    use nast::Pos;
    use nast::Targ;
    use nast::TypeHint;

    use super::*;

    #[test]
    fn test_fun_ret_valid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem = Fun_ {
            span: Default::default(),
            readonly_this: Default::default(),
            annotation: Default::default(),
            readonly_ret: Default::default(),
            ret: TypeHint(
                (),
                Some(Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)))),
            ),
            params: Default::default(),
            ctxs: Default::default(),
            unsafe_ctxs: Default::default(),
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: nast::FunKind::FSync,
            user_attributes: Default::default(),
            external: Default::default(),
            doc_comment: Default::default(),
        };
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(
            elem.ret.1,
            Some(Hint(_, box Hint_::Hprim(Tprim::Tvoid)))
        ))
    }

    #[test]
    fn test_hint_fun_return_ty_valid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem = HintFun {
            is_readonly: Default::default(),
            param_tys: Default::default(),
            param_info: Default::default(),
            variadic_ty: Default::default(),
            ctxs: Default::default(),
            return_ty: Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid))),
            is_readonly_return: Default::default(),
        };
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(
            elem.return_ty,
            Hint(_, box Hint_::Hprim(Tprim::Tvoid))
        ))
    }

    #[test]
    fn test_hint_in_happly_valid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        // Whatever<void>
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Happly(
                Id::default(),
                vec![Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)))],
            )),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(match elem {
            Hint(_, box Hint_::Happly(_, hints)) =>
                matches!(hints.as_slice(), [Hint(_, box Hint_::Hprim(Tprim::Tvoid))]),

            _ => false,
        })
    }

    #[test]
    fn test_hint_in_targ_valid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem = Targ(
            (),
            Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid))),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(elem.1, Hint(_, box Hint_::Hprim(Tprim::Tvoid))))
    }

    #[test]
    fn test_hint_top_level_invalid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem = Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)));
        elem.transform(&env, &mut pass);

        let retonly_hint_err_opt = env.into_errors().pop();
        assert!(matches!(
            retonly_hint_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::ReturnOnlyTypehint { .. }
            ))
        ));
        assert!(matches!(elem, Hint(_, box Hint_::Herr)))
    }

    #[test]
    fn test_fun_param_invalid() {
        let env = Env::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem = Fun_ {
            span: Default::default(),
            readonly_this: Default::default(),
            annotation: Default::default(),
            readonly_ret: Default::default(),
            ret: TypeHint((), None),
            params: vec![FunParam {
                annotation: (),
                type_hint: TypeHint(
                    (),
                    Some(Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)))),
                ),
                is_variadic: Default::default(),
                pos: Default::default(),
                name: Default::default(),
                expr: Default::default(),
                readonly: Default::default(),
                callconv: ParamKind::Pnormal,
                user_attributes: Default::default(),
                visibility: Default::default(),
            }],
            ctxs: Default::default(),
            unsafe_ctxs: Default::default(),
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: nast::FunKind::FSync,
            user_attributes: Default::default(),
            external: Default::default(),
            doc_comment: Default::default(),
        };
        elem.transform(&env, &mut pass);

        let retonly_hint_err_opt = env.into_errors().pop();
        assert!(matches!(
            retonly_hint_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::ReturnOnlyTypehint { .. }
            ))
        ));
        assert!(match elem.params.pop() {
            Some(fp) => matches!(fp.type_hint, TypeHint(_, Some(Hint(_, box Hint_::Herr)))),
            _ => false,
        })
    }
}
