// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::ast_defs::Tprim;
use oxidized::naming_error::NamingError;
use oxidized::naming_error::ReturnOnlyHint;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabHintRetonlyPass {
    allow_retonly: bool,
}

impl Pass for ElabHintRetonlyPass {
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, cfg: &Config) -> ControlFlow<(), ()> {
        match elem {
            Hint(pos, box hint_ @ Hint_::Hprim(Tprim::Tvoid)) if !self.allow_retonly => {
                cfg.emit_error(NamingError::ReturnOnlyTypehint {
                    pos: pos.clone(),
                    kind: ReturnOnlyHint::Hvoid,
                });
                *hint_ = Hint_::Herr;
                ControlFlow::Break(())
            }
            Hint(pos, box hint_ @ Hint_::Hprim(Tprim::Tnoreturn)) if !self.allow_retonly => {
                cfg.emit_error(NamingError::ReturnOnlyTypehint {
                    pos: pos.clone(),
                    kind: ReturnOnlyHint::Hnoreturn,
                });
                *hint_ = Hint_::Herr;
                ControlFlow::Break(())
            }
            _ => ControlFlow::Continue(()),
        }
    }

    fn on_ty_hint__top_down(&mut self, elem: &mut Hint_, _cfg: &Config) -> ControlFlow<(), ()> {
        match elem {
            Hint_::Happly(..) | Hint_::Habstr(..) => self.allow_retonly = true,
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_targ_top_down<Ex>(
        &mut self,
        _elem: &mut oxidized::aast::Targ<Ex>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.allow_retonly = true;
        ControlFlow::Continue(())
    }

    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        _elem: &mut Hint,
        _cfg: &Config,
    ) -> ControlFlow<(), ()> {
        self.allow_retonly = true;
        ControlFlow::Continue(())
    }

    fn on_fld_fun__ret_top_down<Ex>(
        &mut self,
        _elem: &mut oxidized::aast::TypeHint<Ex>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.allow_retonly = true;
        ControlFlow::Continue(())
    }

    fn on_fld_method__ret_top_down<Ex>(
        &mut self,
        _elem: &mut oxidized::aast::TypeHint<Ex>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.allow_retonly = true;
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::Block;
    use oxidized::aast_defs::FunParam;
    use oxidized::aast_defs::Fun_;
    use oxidized::aast_defs::FuncBody;
    use oxidized::aast_defs::HintFun;
    use oxidized::aast_defs::Targ;
    use oxidized::aast_defs::TypeHint;
    use oxidized::ast_defs::Id;
    use oxidized::ast_defs::ParamKind;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;

    use super::*;
    use crate::Transform;

    #[test]
    fn test_fun_ret_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem: Fun_<(), ()> = Fun_ {
            span: Default::default(),
            readonly_this: Default::default(),
            annotation: Default::default(),
            readonly_ret: Default::default(),
            ret: TypeHint(
                (),
                Some(Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)))),
            ),
            tparams: Default::default(),
            where_constraints: Default::default(),
            params: Default::default(),
            ctxs: Default::default(),
            unsafe_ctxs: Default::default(),
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: oxidized::ast_defs::FunKind::FSync,
            user_attributes: Default::default(),
            external: Default::default(),
            doc_comment: Default::default(),
        };
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(
            elem.ret.1,
            Some(Hint(_, box Hint_::Hprim(Tprim::Tvoid)))
        ))
    }

    #[test]
    fn test_hint_fun_return_ty_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem: HintFun = HintFun {
            is_readonly: Default::default(),
            param_tys: Default::default(),
            param_info: Default::default(),
            variadic_ty: Default::default(),
            ctxs: Default::default(),
            return_ty: Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid))),
            is_readonly_return: Default::default(),
        };
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(
            elem.return_ty,
            Hint(_, box Hint_::Hprim(Tprim::Tvoid))
        ))
    }

    #[test]
    fn test_hint_in_happly_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        // Whatever<void>
        let mut elem: Hint = Hint(
            Pos::default(),
            Box::new(Hint_::Happly(
                Id::default(),
                vec![Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)))],
            )),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(match elem {
            Hint(_, box Hint_::Happly(_, hints)) =>
                matches!(hints.as_slice(), [Hint(_, box Hint_::Hprim(Tprim::Tvoid))]),

            _ => false,
        })
    }

    #[test]
    fn test_hint_in_targ_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem: Targ<()> = Targ(
            (),
            Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid))),
        );
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(matches!(elem.1, Hint(_, box Hint_::Hprim(Tprim::Tvoid))))
    }

    #[test]
    fn test_hint_top_level_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem: Hint = Hint(Pos::default(), Box::new(Hint_::Hprim(Tprim::Tvoid)));
        elem.transform(&cfg, &mut pass);

        let retonly_hint_err_opt = cfg.into_errors().pop();
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
        let cfg = Config::default();

        let mut pass = ElabHintRetonlyPass::default();
        let mut elem: Fun_<(), ()> = Fun_ {
            span: Default::default(),
            readonly_this: Default::default(),
            annotation: Default::default(),
            readonly_ret: Default::default(),
            ret: TypeHint((), None),
            tparams: Default::default(),
            where_constraints: Default::default(),
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
            fun_kind: oxidized::ast_defs::FunKind::FSync,
            user_attributes: Default::default(),
            external: Default::default(),
            doc_comment: Default::default(),
        };
        elem.transform(&cfg, &mut pass);

        let retonly_hint_err_opt = cfg.into_errors().pop();
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
