// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Lid;
use oxidized::aast_defs::Sid;
use oxidized::ast_defs::Id;
use oxidized::local_id;
use oxidized::naming_error::NamingError;
use oxidized::tast::Pos;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabClassIdPass {
    in_class: bool,
}

impl Pass for ElabClassIdPass {
    /*
      The lowerer will give us CIexpr (Id  _ | Lvar _ ); here we:
      - convert CIexpr(_,_,Id _) to CIparent, CIself, CIstatic and CI.
      - convert CIexpr(_,_,Lvar $this) to CIexpr(_,_,This)

      If there is a CIexpr with anything other than an Lvar or This after this
      elaboration step, it is an error and will be raised in subsequent
      validation passes

     TODO[mjt] We're defining `on_ty_class_id` rather than `on_ty_class_id_`
     since the legacy code mangles positions by using the inner `class_id_`
     position in the output `class_id` tuple. This looks to be erroneous.

     TODO[mjt] The Lvar(..,this) => This rule is applied during lvar elaboration
     so we should probably drop it here

     TODO[mjt] Lowering gives us a very specific representation but we don't
     enforce this invariant at all here
    */
    fn on_ty_class_id_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut ClassId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()> {
        let ClassId(_annot, pos, class_id_) = elem;
        if let ClassId_::CIexpr(Expr(_, expr_pos, expr_)) = class_id_ as &mut ClassId_<_, _> {
            // [mjt] For some reason the legacy code modifies the position of
            // the surrounding [ClassId]. This seems wrong and causes a clone
            *pos = expr_pos.clone();
            match expr_ {
                Expr_::Id(sid) => {
                    // If the id is a special ref to a class, it is only
                    // valid if we are in a class
                    let Id(id_pos, cname) = sid as &mut Sid;
                    if cname == sn::classes::PARENT {
                        if !self.in_class {
                            let err_pos = std::mem::replace(id_pos, Pos::NONE);
                            env.emit_error(NamingError::ParentOutsideClass(err_pos));
                            let ci_pos = std::mem::replace(expr_pos, Pos::NONE);
                            *class_id_ = ClassId_::CI(Id(ci_pos, sn::classes::UNKNOWN.to_string()))
                        } else {
                            *class_id_ = ClassId_::CIparent
                        }
                    } else if cname == sn::classes::SELF {
                        if !self.in_class {
                            let err_pos = std::mem::replace(id_pos, Pos::NONE);
                            env.emit_error(NamingError::SelfOutsideClass(err_pos));
                            let ci_pos = std::mem::replace(expr_pos, Pos::NONE);
                            *class_id_ = ClassId_::CI(Id(ci_pos, sn::classes::UNKNOWN.to_string()))
                        } else {
                            *class_id_ = ClassId_::CIself
                        }
                    } else if cname == sn::classes::STATIC {
                        if !self.in_class {
                            let err_pos = std::mem::replace(id_pos, Pos::NONE);
                            env.emit_error(NamingError::StaticOutsideClass(err_pos));
                            let ci_pos = std::mem::replace(expr_pos, Pos::NONE);
                            *class_id_ = ClassId_::CI(Id(ci_pos, sn::classes::UNKNOWN.to_string()))
                        } else {
                            *class_id_ = ClassId_::CIstatic
                        }
                    } else {
                        // Otherwise, replace occurrences of CIexpr(_,_,Id(..))
                        // with CI(..)
                        let ci_pos = std::mem::replace(expr_pos, Pos::NONE);
                        let ci_name = std::mem::take(cname);
                        *class_id_ = ClassId_::CI(Id(ci_pos, ci_name))
                    }
                    ControlFlow::Continue(())
                }

                Expr_::Lvar(lid) => {
                    // Convert Lvar(this) => this; note that this overlaps
                    // with lvar elaboration
                    let Lid(_lid_pos, lcl_id) = &**lid;
                    if local_id::get_name(lcl_id) == sn::special_idents::THIS {
                        *expr_ = Expr_::This
                    }
                    ControlFlow::Continue(())
                }
                _ => ControlFlow::Continue(()),
            }
        } else {
            // We only ever expect a `CIexpr(..)` to come from lowering.
            // We should change the lowered AST repr to make this impossible.
            ControlFlow::Continue(())
        }
    }

    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        _elem: &mut oxidized::aast::Class_<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.in_class = true;
        ControlFlow::Continue(())
    }

    /* The attributes applied to a class exist outside the current class so
    references to `self` are invalid */
    fn on_fld_class__user_attributes_top_down<Ex: Default, En>(
        &mut self,
        _elem: &mut oxidized::tast::UserAttributes<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        self.in_class = true;
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Transform;
    // Elaboration of CIexpr(..,..,Id(..,..)) when the id refers to a class
    #[test]
    fn test_ciexpr_id_class_ref() {
        let env = Env::default();

        let mut pass = ElabClassIdPass::default();

        let cases: Vec<(&str, ClassId_<(), ()>)> = vec![
            (sn::classes::SELF, ClassId_::CIself),
            (sn::classes::PARENT, ClassId_::CIparent),
            (sn::classes::STATIC, ClassId_::CIstatic),
        ];
        for (cname, repr) in cases {
            let mut elem_outside = ClassId(
                (),
                Pos::NONE,
                ClassId_::CIexpr(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Id(Box::new(Id(Pos::NONE, cname.to_string()))),
                )),
            );
            let mut elem_inside = elem_outside.clone();

            // transforming when outside a class
            // expect CI(Id(.., UNKNOWN))
            pass.in_class = false;
            elem_outside.transform(&env, &mut pass);
            let ClassId(_, _, class_id_) = elem_outside;
            assert!(match class_id_ {
                ClassId_::CI(Id(_, nm)) => nm == sn::classes::UNKNOWN,
                _ => false,
            });

            // transforming when inside a class
            // expect
            pass.in_class = true;
            elem_inside.transform(&env, &mut pass);
            let ClassId(_, _, class_id_) = elem_inside;
            assert_eq!(class_id_, repr)
        }
    }

    // Elaboration of CIexpr(..,..,Id(..,..)) when the id does not refer
    // to a class
    #[test]
    fn test_ciexpr_id_non_class_ref() {
        let env = Env::default();

        let mut pass = ElabClassIdPass::default();
        let cname = "Classy";

        let mut elem_outside: ClassId<(), ()> = ClassId(
            (),
            Pos::NONE,
            ClassId_::CIexpr(Expr(
                (),
                Pos::NONE,
                Expr_::Id(Box::new(Id(Pos::NONE, cname.to_string()))),
            )),
        );
        let mut elem_inside = elem_outside.clone();

        // transforming when outside a class
        // expect CI(Id(.., cname))
        pass.in_class = false;
        elem_outside.transform(&env, &mut pass);
        let ClassId(_, _, class_id_) = elem_outside;
        assert!(match class_id_ {
            ClassId_::CI(Id(_, nm)) => nm == cname,
            _ => false,
        });

        // transforming when inside a class
        // expect CI(Id(.., cname))
        pass.in_class = true;
        elem_inside.transform(&env, &mut pass);
        let ClassId(_, _, class_id_) = elem_inside;
        assert!(match class_id_ {
            ClassId_::CI(Id(_, nm)) => nm == cname,
            _ => false,
        });
    }

    // Elaboration of CIexpr(..,..,Lvar(..,this)) => CIexpr(..,..,This)
    #[test]
    fn test_ciexpr_lvar_this() {
        let env = Env::default();

        let mut pass = ElabClassIdPass::default();

        let mut elem_outside: ClassId<(), ()> = ClassId(
            (),
            Pos::NONE,
            ClassId_::CIexpr(Expr(
                (),
                Pos::NONE,
                Expr_::Lvar(Box::new(Lid(
                    Pos::NONE,
                    local_id::make_unscoped(sn::special_idents::THIS),
                ))),
            )),
        );
        let mut elem_inside = elem_outside.clone();

        // transforming when outside a class
        // expect CIexpr(_,_,This)
        pass.in_class = false;
        elem_outside.transform(&env, &mut pass);
        let ClassId(_, _, class_id_) = elem_outside;
        assert!(matches!(
            class_id_,
            ClassId_::CIexpr(Expr(_, _, Expr_::This))
        ));

        // transforming when inside a class
        // expect
        pass.in_class = true;
        elem_inside.transform(&env, &mut pass);
        let ClassId(_, _, class_id_) = elem_inside;
        assert!(matches!(
            class_id_,
            ClassId_::CIexpr(Expr(_, _, Expr_::This))
        ));
    }

    // Elaboration of CIexpr(..,..,expr_)
    // for any expression other than `Id` or `Lvar(_,this)`, we expect the
    // elaborated ClassId_ to still have the same Expr_
    // Note[mjt]: in practice, I think we only ever see `Id` and `Lvar` expressions
    // in this position
    #[test]
    fn test_ciexpr_fallthrough() {
        let env = Env::default();

        let mut pass = ElabClassIdPass::default();

        let exprs_ = vec![
            Expr_::Lvar(Box::new(Lid(
                Pos::NONE,
                local_id::make_unscoped("wut".to_string()),
            ))),
            Expr_::Null,
        ];

        for expr_ in exprs_ {
            let mut elem_outside: ClassId<(), ()> = ClassId(
                (),
                Pos::NONE,
                ClassId_::CIexpr(Expr((), Pos::NONE, expr_.clone())),
            );
            let mut elem_inside = elem_outside.clone();

            pass.in_class = false;
            elem_outside.transform(&env, &mut pass);
            let ClassId(_, _, class_id_) = elem_outside;
            assert!(match class_id_ {
                ClassId_::CIexpr(Expr(_, _, ci_expr_)) => ci_expr_ == expr_,
                _ => false,
            });

            pass.in_class = true;
            elem_inside.transform(&env, &mut pass);
            let ClassId(_, _, class_id_) = elem_inside;
            assert!(match class_id_ {
                ClassId_::CIexpr(Expr(_, _, ci_expr_)) => ci_expr_ == expr_,
                _ => false,
            })
        }
    }
}
