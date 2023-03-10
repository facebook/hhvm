// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use bitflags::bitflags;
use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::ClassHint;
use oxidized::nast::ClassReq;
use oxidized::nast::ClassVar;
use oxidized::nast::Class_;
use oxidized::nast::ClassishKind;
use oxidized::nast::Expr_;
use oxidized::nast::Hint;
use oxidized::nast::Hint_;
use oxidized::nast::RequireKind;
use oxidized::nast::ShapeFieldInfo;
use oxidized::nast::Targ;
use oxidized::nast::Tparam;
use oxidized::nast::TraitHint;
use oxidized::nast::TypeHint;
use oxidized::nast::Variance;
use oxidized::nast::XhpAttr;
use oxidized::nast::XhpAttrHint;

use crate::env::Env;
use crate::Pass;

#[derive(Copy, Clone, Default)]
pub struct ElabHintThisPass {
    context: Option<Context>,
    flags: Flags,
}

#[derive(Copy, Clone)]
enum Context {
    ReqExtends,
    Extends,
    StaticClassVar(bool),
}

bitflags! {
    #[derive(Default)]
    struct Flags: u8 {
        const FORBID_THIS= 1 << 0;
        const IS_TOP_LEVEL_HACCESS_ROOT= 1 << 1;
        const IN_INTERFACE = 1 << 2;
        const IN_INVARIANT_FINAL = 1 << 3;
    }
}

impl ElabHintThisPass {
    fn forbid_this(&self) -> bool {
        self.flags.contains(Flags::FORBID_THIS)
    }

    fn set_forbid_this(&mut self, value: bool) {
        self.flags.set(Flags::FORBID_THIS, value)
    }

    fn set_static_class_var(&mut self, value: bool) {
        self.context = Some(Context::StaticClassVar(value))
    }

    fn in_req_extends(&self) -> bool {
        matches!(self.context, Some(Context::ReqExtends))
    }

    fn set_in_req_extends(&mut self) {
        self.context = Some(Context::ReqExtends)
    }

    fn in_extends(&self) -> bool {
        matches!(self.context, Some(Context::Extends))
    }

    fn set_in_extends(&mut self) {
        self.context = Some(Context::Extends)
    }

    fn is_top_level_haccess_root(&self) -> bool {
        self.flags.contains(Flags::IS_TOP_LEVEL_HACCESS_ROOT)
    }

    fn set_is_top_level_haccess_root(&mut self, value: bool) {
        self.flags.set(Flags::IS_TOP_LEVEL_HACCESS_ROOT, value)
    }

    fn in_interface(&self) -> bool {
        self.flags.contains(Flags::IN_INTERFACE)
    }

    fn set_in_interface(&mut self, value: bool) {
        self.flags.set(Flags::IN_INTERFACE, value)
    }

    fn in_invariant_final(&self) -> bool {
        self.flags.contains(Flags::IN_INVARIANT_FINAL)
    }

    fn set_in_invariant_final(&mut self, value: bool) {
        self.flags.set(Flags::IN_INVARIANT_FINAL, value)
    }

    // We want to disallow `this` hints in:
    // - _class_ and _abstract class_ type parameters
    // - non-late static bound class_var
    // - `extends` and `require extends` clauses _unless_ it appears as the
    //   top-level root of a type access
    fn forbid_in_extends(&self) -> bool {
        (self.in_req_extends() || self.in_extends())
            && (!self.in_interface())
            && (!self.is_top_level_haccess_root())
            && (!self.in_invariant_final())
    }
}

impl Pass for ElabHintThisPass {
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        let Hint(pos, box hint_) = elem;
        match &hint_ {
            Hint_::Hthis if self.forbid_this() || self.forbid_in_extends() => {
                // We have a `this` hint in a forbidden position; raise and error,
                // leave the `Herr` and break
                *hint_ = Hint_::Herr;
                env.emit_error(NamingError::ThisTypeForbidden {
                    pos: pos.clone(),
                    in_extends: self.in_extends(),
                    in_req_extends: self.in_req_extends(),
                });
                return ControlFlow::Break(());
            }
            // Otherwise, just update our state to reflect whether we are
            // at the top-level `Hint` inside an `Haccess`
            Hint_::Haccess(..) => self.set_is_top_level_haccess_root(true),
            _ => self.set_is_top_level_haccess_root(false),
        }

        ControlFlow::Continue(())
    }

    fn on_ty_class__top_down(&mut self, elem: &mut Class_, _env: &Env) -> ControlFlow<()> {
        let in_interface = matches!(elem.kind, ClassishKind::Cinterface);
        let in_invariant_final = elem.final_
            && elem
                .tparams
                .iter()
                .all(|tp| matches!(tp.variance, Variance::Invariant));
        self.set_in_interface(in_interface);
        self.set_in_invariant_final(in_invariant_final);
        ControlFlow::Continue(())
    }

    fn on_fld_class__tparams_top_down(
        &mut self,
        _elem: &mut Vec<Tparam>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(true);
        ControlFlow::Continue(())
    }

    fn on_fld_class__extends_top_down(
        &mut self,
        _elem: &mut Vec<ClassHint>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_in_extends();
        ControlFlow::Continue(())
    }

    fn on_fld_class__uses_top_down(
        &mut self,
        _elem: &mut Vec<TraitHint>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_fld_class__xhp_attrs_top_down(
        &mut self,
        _elem: &mut Vec<XhpAttr>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_fld_class__xhp_attr_uses_top_down(
        &mut self,
        _elem: &mut Vec<XhpAttrHint>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_fld_class__reqs_top_down(
        &mut self,
        _elem: &mut Vec<ClassReq>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_ty_class_req_top_down(&mut self, elem: &mut ClassReq, _env: &Env) -> ControlFlow<()> {
        if elem.1 == RequireKind::RequireExtends {
            self.set_in_req_extends()
        }
        ControlFlow::Continue(())
    }

    fn on_fld_class__implements_top_down(
        &mut self,
        _elem: &mut Vec<ClassHint>,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_ty_class_var_top_down(&mut self, elem: &mut ClassVar, _env: &Env) -> ControlFlow<()> {
        self.set_static_class_var(
            elem.is_static
                && !elem
                    .user_attributes
                    .iter()
                    .any(|ua| ua.name.name() == sn::user_attributes::LSB),
        );
        ControlFlow::Continue(())
    }

    fn on_fld_class_var_type__top_down(
        &mut self,
        _elem: &mut TypeHint,
        _env: &Env,
    ) -> ControlFlow<()> {
        let forbid_this = match self.context {
            Some(Context::StaticClassVar(lsb)) => lsb,
            _ => panic!("impossible"),
        };
        self.context = None;
        self.set_forbid_this(forbid_this);
        ControlFlow::Continue(())
    }

    fn on_fld_fun__ret_top_down(&mut self, _elem: &mut TypeHint, _env: &Env) -> ControlFlow<()> {
        self.context = None;
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_ty_expr__top_down(&mut self, elem: &mut Expr_, _env: &Env) -> ControlFlow<()> {
        match elem {
            Expr_::Cast(..) | Expr_::Is(..) | Expr_::As(..) | Expr_::Upcast(..) => {
                self.context = None;
                self.set_forbid_this(false);
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_shape_field_info_top_down(
        &mut self,
        _elem: &mut ShapeFieldInfo,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.context = None;
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        _elem: &mut Hint,
        _env: &Env,
    ) -> ControlFlow<()> {
        self.context = None;
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }

    fn on_ty_targ_top_down(&mut self, _elem: &mut Targ, _env: &Env) -> ControlFlow<()> {
        self.context = None;
        self.set_forbid_this(false);
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use file_info::Mode;
    use ocamlrep::rc::RcOc;
    use oxidized::namespace_env;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::nast::Id;
    use oxidized::nast::Pos;
    use oxidized::nast::UserAttribute;
    use oxidized::nast::UserAttributes;
    use oxidized::s_map::SMap;

    use super::*;
    use crate::Transform;

    fn make_class(
        kind: ClassishKind,
        name: &str,
        final_: bool,
        tparams: Vec<Tparam>,
        extends: Vec<ClassHint>,
        reqs: Vec<ClassReq>,
    ) -> Class_ {
        Class_ {
            span: Default::default(),
            annotation: (),
            mode: Mode::Mstrict,
            final_,
            is_xhp: Default::default(),
            has_xhp_keyword: Default::default(),
            kind,
            name: Id(Pos::default(), name.to_string()),
            tparams,
            extends,
            uses: Default::default(),
            xhp_attr_uses: Default::default(),
            xhp_category: Default::default(),
            reqs,
            implements: Default::default(),
            where_constraints: Default::default(),
            consts: Default::default(),
            typeconsts: Default::default(),
            vars: Default::default(),
            methods: Default::default(),
            xhp_children: Default::default(),
            xhp_attrs: Default::default(),
            namespace: RcOc::new(namespace_env::Env {
                ns_uses: SMap::default(),
                class_uses: SMap::default(),
                fun_uses: SMap::default(),
                const_uses: SMap::default(),
                name: None,
                auto_ns_map: vec![],
                is_codegen: false,
                disable_xhp_element_mangling: false,
            }),
            user_attributes: Default::default(),
            file_attributes: Default::default(),
            docs_url: Default::default(),
            enum_: Default::default(),
            doc_comment: Default::default(),
            emit_id: Default::default(),
            internal: Default::default(),
            module: Default::default(),
        }
    }

    fn make_static_class_var(name: &str, type_: TypeHint, lsb: bool) -> ClassVar {
        let user_attributes = if lsb {
            UserAttributes(vec![UserAttribute {
                name: Id(Default::default(), sn::user_attributes::LSB.to_string()),
                params: vec![],
            }])
        } else {
            UserAttributes(vec![])
        };
        ClassVar {
            final_: Default::default(),
            xhp_attr: Default::default(),
            abstract_: Default::default(),
            readonly: Default::default(),
            visibility: oxidized::nast::Visibility::Public,
            type_,
            id: Id(Default::default(), name.to_string()),
            expr: Default::default(),
            user_attributes,
            doc_comment: Default::default(),
            is_promoted_variadic: Default::default(),
            is_static: true,
            span: Default::default(),
        }
    }
    // -- `this` in extends clause ---------------------------------------------

    #[test]
    // We allow `this` as a generic on a super type when the class is final
    // and not generic
    fn test_final_extends_non_generic_valid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        // final class C extends B<this> {}
        let mut elem: Class_ = make_class(
            ClassishKind::Cclass(oxidized::nast::Abstraction::Concrete),
            "C",
            true,
            vec![],
            vec![Hint(
                Pos::default(),
                Box::new(Hint_::Happly(
                    Id(Pos::default(), "B".to_string()),
                    vec![Hint(Pos::default(), Box::new(Hint_::Hthis))],
                )),
            )],
            vec![],
        );

        elem.transform(&env, &mut pass);

        // Expect no errors
        assert!(env.into_errors().is_empty());
        assert!(match elem.extends.pop() {
            Some(Hint(_, box Hint_::Happly(_, hints))) => match hints.as_slice() {
                [Hint(_, box Hint_::Hthis)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    #[test]
    // We allow `this` as a generic on a super type when the class is final
    // and invariant in all type parameters
    fn test_final_extends_invariant_valid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        // final class C<T> extends B<this> {}
        let mut elem: Class_ = make_class(
            ClassishKind::Cclass(oxidized::nast::Abstraction::Concrete),
            "C",
            true,
            vec![Tparam {
                variance: Variance::Invariant,
                name: Id(Pos::default(), "T".to_string()),
                parameters: Default::default(),
                constraints: Default::default(),
                reified: oxidized::nast::ReifyKind::Erased,
                user_attributes: Default::default(),
            }],
            vec![Hint(
                Pos::default(),
                Box::new(Hint_::Happly(
                    Id(Pos::default(), "B".to_string()),
                    vec![Hint(Pos::default(), Box::new(Hint_::Hthis))],
                )),
            )],
            vec![],
        );

        elem.transform(&env, &mut pass);

        // Expect no errors
        assert!(env.into_errors().is_empty());
        assert!(match elem.extends.pop() {
            Some(Hint(_, box Hint_::Happly(_, hints))) => match hints.as_slice() {
                [Hint(_, box Hint_::Hthis)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    #[test]
    // We disallow `this` as a generic on a super type when the class is final
    // and but invariant in some type parameter
    fn test_final_extends_covariant_invalid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        // final class C<+T> extends B<this> {}
        let mut elem: Class_ = make_class(
            ClassishKind::Cclass(oxidized::nast::Abstraction::Concrete),
            "C",
            true,
            vec![Tparam {
                variance: Variance::Covariant,
                name: Id(Pos::default(), "T".to_string()),
                parameters: Default::default(),
                constraints: Default::default(),
                reified: oxidized::nast::ReifyKind::Erased,
                user_attributes: Default::default(),
            }],
            vec![Hint(
                Pos::default(),
                Box::new(Hint_::Happly(
                    Id(Pos::default(), "B".to_string()),
                    vec![Hint(Pos::default(), Box::new(Hint_::Hthis))],
                )),
            )],
            vec![],
        );

        elem.transform(&env, &mut pass);

        let this_type_forbidden_err_opt = env.into_errors().pop();
        assert!(match this_type_forbidden_err_opt {
            Some(NamingPhaseError::Naming(NamingError::ThisTypeForbidden {
                in_extends,
                in_req_extends,
                ..
            })) => in_extends && !in_req_extends,
            _ => false,
        });

        assert!(match elem.extends.pop() {
            Some(Hint(_, box Hint_::Happly(_, hints))) => match hints.as_slice() {
                [Hint(_, box Hint_::Herr)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    // -- `this` hint in require extends ---------------------------------------

    #[test]
    // We disallow `this` as a generic in require extends clauses
    fn test_req_extends_generic_invalid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        // trait C<T> { require extends B<this>; }
        let mut elem: Class_ = make_class(
            ClassishKind::Ctrait,
            "C",
            false,
            vec![Tparam {
                variance: Variance::Invariant,
                name: Id(Pos::default(), "T".to_string()),
                parameters: Default::default(),
                constraints: Default::default(),
                reified: oxidized::nast::ReifyKind::Erased,
                user_attributes: Default::default(),
            }],
            vec![],
            vec![ClassReq(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Happly(
                        Id(Pos::default(), "B".to_string()),
                        vec![Hint(Pos::default(), Box::new(Hint_::Hthis))],
                    )),
                ),
                RequireKind::RequireExtends,
            )],
        );

        elem.transform(&env, &mut pass);

        let this_type_forbidden_err_opt = env.into_errors().pop();
        assert!(match this_type_forbidden_err_opt {
            Some(NamingPhaseError::Naming(NamingError::ThisTypeForbidden {
                in_extends,
                in_req_extends,
                ..
            })) => !in_extends && in_req_extends,
            _ => false,
        });

        assert!(match elem.reqs.pop() {
            Some(ClassReq(Hint(_, box Hint_::Happly(_, hints)), _)) => match hints.as_slice() {
                [Hint(_, box Hint_::Herr)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    #[test]
    // We disallow `this` as a top-level hint in require extends clauses
    fn test_req_extends_top_level_invalid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        // trait C<T> { require extends this; }
        let mut elem: Class_ = make_class(
            ClassishKind::Ctrait,
            "C",
            false,
            vec![Tparam {
                variance: Variance::Invariant,
                name: Id(Pos::default(), "T".to_string()),
                parameters: Default::default(),
                constraints: Default::default(),
                reified: oxidized::nast::ReifyKind::Erased,
                user_attributes: Default::default(),
            }],
            vec![],
            vec![ClassReq(
                Hint(Pos::default(), Box::new(Hint_::Hthis)),
                RequireKind::RequireExtends,
            )],
        );

        elem.transform(&env, &mut pass);

        let this_type_forbidden_err_opt = env.into_errors().pop();
        assert!(match this_type_forbidden_err_opt {
            Some(NamingPhaseError::Naming(NamingError::ThisTypeForbidden {
                in_extends,
                in_req_extends,
                ..
            })) => !in_extends && in_req_extends,
            _ => false,
        });

        assert!(matches!(
            elem.reqs.pop(),
            Some(ClassReq(Hint(_, box Hint_::Herr), _))
        ));
    }

    // -- `this` in static class var -------------------------------------------

    #[test]
    fn test_lsb_static_class_var_valid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        let mut elem: ClassVar = make_static_class_var(
            "x",
            TypeHint(
                (),
                Some(Hint(
                    Default::default(),
                    Box::new(Hint_::Hoption(Hint(
                        Default::default(),
                        Box::new(Hint_::Hthis),
                    ))),
                )),
            ),
            true,
        );

        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());

        let TypeHint(_, hint_opt) = elem.type_;
        assert!(matches!(
            hint_opt,
            Some(Hint(_, box Hint_::Hoption(Hint(_, box Hint_::Hthis))))
        ));
    }

    #[test]
    fn test_non_lsb_static_class_var_invalid() {
        let env = Env::default();

        let mut pass = ElabHintThisPass::default();
        let mut elem: ClassVar = make_static_class_var(
            "x",
            TypeHint(
                (),
                Some(Hint(
                    Default::default(),
                    Box::new(Hint_::Hoption(Hint(
                        Default::default(),
                        Box::new(Hint_::Hthis),
                    ))),
                )),
            ),
            false,
        );

        elem.transform(&env, &mut pass);

        let this_type_forbidden_err_opt = env.into_errors().pop();
        assert!(match this_type_forbidden_err_opt {
            Some(NamingPhaseError::Naming(NamingError::ThisTypeForbidden {
                in_extends,
                in_req_extends,
                ..
            })) => !in_extends && !in_req_extends,
            _ => false,
        });

        let TypeHint(_, hint_opt) = elem.type_;
        assert!(matches!(
            hint_opt,
            Some(Hint(_, box Hint_::Hoption(Hint(_, box Hint_::Herr))))
        ));
    }
}
