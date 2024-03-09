// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::FunParam;
use nast::Fun_;
use nast::Method_;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateFunParamsPass;

impl Pass for ValidateFunParamsPass {
    fn on_ty_fun__top_down(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<()> {
        self.validate_fun_params(env, &elem.params)
    }

    fn on_ty_method__top_down(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        self.validate_fun_params(env, &elem.params)
    }
}

impl ValidateFunParamsPass {
    fn validate_fun_params(&self, env: &Env, params: &Vec<FunParam>) -> ControlFlow<()> {
        let mut seen = std::collections::BTreeSet::<&String>::new();
        for FunParam { name, pos, .. } in params {
            if name == sn::special_idents::PLACEHOLDER {
                continue;
            } else if seen.contains(name) {
                env.emit_error(NamingError::AlreadyBound {
                    pos: pos.clone(),
                    name: name.clone(),
                });
            } else if name == sn::special_idents::THIS {
                env.emit_error(NamingError::ThisAsLexicalVariable(pos.clone()));
            } else {
                seen.insert(name);
            }
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Block;
    use nast::FunKind;
    use nast::FuncBody;
    use nast::Id;
    use nast::ParamKind;
    use nast::Pos;
    use nast::TypeHint;
    use nast::UserAttributes;
    use nast::Visibility;

    use super::*;

    fn mk_fun(params: Vec<FunParam>) -> Fun_ {
        Fun_ {
            span: Pos::NONE,
            readonly_this: None,
            annotation: (),
            readonly_ret: None,
            ret: TypeHint((), None),
            params,
            ctxs: None,
            unsafe_ctxs: None,
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: FunKind::FSync,
            user_attributes: UserAttributes(vec![]),
            external: false,
            doc_comment: None,
        }
    }

    fn mk_method(name: String, params: Vec<FunParam>) -> Method_ {
        Method_ {
            span: Pos::NONE,
            annotation: (),
            final_: false,
            abstract_: false,
            static_: true,
            readonly_this: false,
            visibility: Visibility::Public,
            name: Id(Pos::NONE, name),
            tparams: vec![],
            where_constraints: vec![],
            params,
            ctxs: None,
            unsafe_ctxs: None,
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: FunKind::FSync,
            user_attributes: UserAttributes(vec![]),
            readonly_ret: None,
            ret: TypeHint((), None),
            external: false,
            doc_comment: None,
        }
    }

    fn mk_param(name: String) -> FunParam {
        FunParam {
            name,
            annotation: (),
            type_hint: TypeHint((), None),
            is_variadic: false,
            pos: Pos::NONE,
            expr: None,
            readonly: None,
            callconv: ParamKind::Pnormal,
            user_attributes: UserAttributes(Vec::default()),
            visibility: Some(Visibility::Public),
        }
    }

    #[test]
    fn test_fn_no_args() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let mut fun = mk_fun(vec![]);

        fun.transform(&env, &mut pass);
        assert!(env.into_errors().is_empty())
    }

    #[test]
    fn test_meth_no_args() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let mut meth = mk_method("foo".to_string(), vec![]);

        meth.transform(&env, &mut pass);
        assert!(env.into_errors().is_empty())
    }

    #[test]
    fn test_fn_good_args() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let x = mk_param("x".to_string());
        let y = mk_param("y".to_string());
        let mut fun = mk_fun(vec![x, y]);

        fun.transform(&env, &mut pass);
        assert!(env.into_errors().is_empty())
    }

    #[test]
    fn test_meth_good_args() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let x = mk_param("x".to_string());
        let y = mk_param("y".to_string());
        let mut meth = mk_method("foo".to_string(), vec![x, y]);

        meth.transform(&env, &mut pass);
        assert!(env.into_errors().is_empty())
    }

    #[test]
    fn test_fn_args_multiply_bound() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let x = mk_param("x".to_string());
        let mut fun = mk_fun(vec![x.clone(), x]);

        fun.transform(&env, &mut pass);
        assert!(matches!(
            env.into_errors().as_slice(),
            &[NamingPhaseError::Naming(NamingError::AlreadyBound { .. })]
        ))
    }

    #[test]
    fn test_meth_args_multiply_bound() {
        let env = Env::default();

        let mut pass = ValidateFunParamsPass;

        let x = mk_param("x".to_string());
        let mut meth = mk_method("foo".to_string(), vec![x.clone(), x]);

        meth.transform(&env, &mut pass);
        assert!(matches!(
            env.into_errors().as_slice(),
            &[NamingPhaseError::Naming(NamingError::AlreadyBound { .. })]
        ))
    }
}
