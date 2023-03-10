// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use bitflags::bitflags;
use naming_special_names_rust as sn;
use oxidized::aast_defs::ClassConstKind;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::Class_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::FunDef;
use oxidized::aast_defs::Gconst;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::KvcKind;
use oxidized::aast_defs::ModuleDef;
use oxidized::aast_defs::Typedef;
use oxidized::aast_defs::VcKind;
use oxidized::ast_defs::Bop;
use oxidized::ast_defs::ClassishKind;
use oxidized::ast_defs::Uop;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::elab_utils;
use crate::Pass;

#[derive(Copy, Clone)]
pub struct ElabConstExprPass {
    mode: file_info::Mode,
    flags: Flags,
}

bitflags! {
    #[derive(Default)]
    struct Flags: u8 {
        const IN_ENUM_CLASS = 1 << 0;
        const ENFORCE_CONST_EXPR = 1 << 1;
    }
}

impl ElabConstExprPass {
    pub fn in_enum_class(&self) -> bool {
        self.flags.contains(Flags::IN_ENUM_CLASS)
    }

    pub fn set_in_enum_class(&mut self, value: bool) {
        self.flags.set(Flags::IN_ENUM_CLASS, value)
    }

    pub fn enforce_const_expr(&self) -> bool {
        self.flags.contains(Flags::ENFORCE_CONST_EXPR)
    }

    pub fn set_enforce_const_expr(&mut self, value: bool) {
        self.flags.set(Flags::ENFORCE_CONST_EXPR, value)
    }
}

impl Default for ElabConstExprPass {
    fn default() -> Self {
        ElabConstExprPass {
            mode: file_info::Mode::Mstrict,
            flags: Flags::default(),
        }
    }
}

impl Pass for ElabConstExprPass {
    // We can determine that certain expressions are invalid based on one-level
    // pattern matching. We prefer to do this since we can stop the transformation
    // early in these cases. For cases where we need to pattern match on the
    // expression more deeply, we use the bottom-up pass
    fn on_ty_expr_top_down<Ex, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        if !self.enforce_const_expr() {
            ControlFlow::Continue(())
        } else {
            let Expr(_, pos, expr_) = elem;
            let invalid = |expr_: &mut Expr_<_, _>| {
                let inner_expr_ = std::mem::replace(expr_, Expr_::Null);
                let inner_expr = elab_utils::expr::from_expr_(inner_expr_);
                *expr_ = Expr_::Invalid(Box::new(Some(inner_expr)));
                ControlFlow::Break(())
            };

            match &expr_ {
                // -- always valid ---------------------------------------------
                Expr_::Id(..)
                | Expr_::Null
                | Expr_::True
                | Expr_::False
                | Expr_::Int(_)
                | Expr_::Float(_)
                | Expr_::String(_)
                | Expr_::FunctionPointer(..)
                | Expr_::Eif(..)
                | Expr_::Darray(..)
                | Expr_::Varray(..)
                | Expr_::Tuple(..)
                | Expr_::Shape(..)
                | Expr_::Upcast(..) => ControlFlow::Continue(()),

                // -- markers --------------------------------------------------
                Expr_::Invalid(..) | Expr_::Hole(..) => ControlFlow::Continue(()),

                // -- handled bottom-up ----------------------------------------
                Expr_::ClassConst(_) => ControlFlow::Continue(()),

                // -- conditionally valid --------------------------------------
                Expr_::As(box (_, Hint(_, box hint_), _)) => match hint_ {
                    // NB we can perform this top-down since the all valid hints
                    // are already in canonical
                    Hint_::Hlike(..) => ControlFlow::Continue(()),
                    Hint_::Happly(id, _) if id.name() == sn::fb::INCORRECT_TYPE => {
                        ControlFlow::Continue(())
                    }
                    // TODO[mjt] another example of inconsistency around error positions
                    Hint_::Happly(id, _) => {
                        cfg.emit_error(NamingError::IllegalConstant(id.0.clone()));
                        invalid(expr_)
                    }
                    _ => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                },
                Expr_::Unop(box (uop, _)) => match uop {
                    Uop::Uplus | Uop::Uminus | Uop::Utild | Uop::Unot => ControlFlow::Continue(()),
                    _ => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                },
                Expr_::Binop(box (bop, _, _)) => match bop {
                    Bop::Eq(_) => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                    _ => ControlFlow::Continue(()),
                },
                Expr_::ValCollection(box ((_, vc_kind), _, _)) => match vc_kind {
                    VcKind::Vec | VcKind::Keyset => ControlFlow::Continue(()),
                    _ => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                },
                Expr_::KeyValCollection(box ((_, kvc_kind), _, _)) => match kvc_kind {
                    KvcKind::Dict => ControlFlow::Continue(()),
                    _ => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                },
                Expr_::Call(box (Expr(_, _, call_expr_), _, _, _)) => match call_expr_ {
                    Expr_::Id(box id)
                        if id.name() == sn::std_lib_functions::ARRAY_MARK_LEGACY
                            || id.name() == sn::pseudo_functions::UNSAFE_CAST
                            || id.name() == sn::pseudo_functions::UNSAFE_NONNULL_CAST =>
                    {
                        ControlFlow::Continue(())
                    }
                    _ => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                },

                Expr_::Omitted => {
                    if matches!(self.mode, file_info::Mode::Mhhi) {
                        ControlFlow::Continue(())
                    } else {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }
                }

                // -- always invalid -------------------------------------------
                Expr_::This
                | Expr_::Lvar(..)
                | Expr_::Lplaceholder(..)
                | Expr_::ArrayGet(..)
                | Expr_::Await(..)
                | Expr_::Cast(..)
                | Expr_::ClassGet(..)
                | Expr_::Clone(..)
                | Expr_::Dollardollar(..)
                | Expr_::ETSplice(..)
                | Expr_::Efun(..)
                | Expr_::EnumClassLabel(..)
                | Expr_::ExpressionTree(..)
                | Expr_::Is(..)
                | Expr_::Lfun(..)
                | Expr_::List(..)
                | Expr_::MethodCaller(..)
                | Expr_::New(..)
                | Expr_::ObjGet(..)
                | Expr_::Pair(..)
                | Expr_::Pipe(..)
                | Expr_::PrefixedString(..)
                | Expr_::ReadonlyExpr(..)
                | Expr_::String2(..)
                | Expr_::Yield(..)
                | Expr_::Xml(..) => {
                    cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                    invalid(expr_)
                }

                // -- unexpected expressions -----------------------------------
                Expr_::Import(..) | Expr_::Collection(..) => panic!("Unexpected Expr"),
            }
        }
    }

    // Handle non-constant expressions which require pattern matching on some
    // element of the expression which is not yet transformed in the top-down pass
    fn on_ty_expr_bottom_up<Ex, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        if !self.enforce_const_expr() {
            ControlFlow::Continue(())
        } else {
            let Expr(_, pos, expr_) = elem;
            let invalid = |expr_: &mut Expr_<_, _>| {
                let inner_expr_ = std::mem::replace(expr_, Expr_::Null);
                let inner_expr = elab_utils::expr::from_expr_(inner_expr_);
                *expr_ = Expr_::Invalid(Box::new(Some(inner_expr)));
                ControlFlow::Break(())
            };

            match expr_ {
                Expr_::ClassConst(box (ClassId(_, _, class_id_), _)) => match class_id_ {
                    ClassId_::CIstatic => {
                        cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                        invalid(expr_)
                    }

                    ClassId_::CIparent | ClassId_::CIself | ClassId_::CI(..) => {
                        ControlFlow::Continue(())
                    }

                    ClassId_::CIexpr(Expr(_, _, expr_)) => match expr_ {
                        Expr_::This | Expr_::Id(..) => ControlFlow::Continue(()),
                        _ => {
                            cfg.emit_error(NamingError::IllegalConstant(pos.clone()));
                            invalid(expr_)
                        }
                    },
                },
                _ => ControlFlow::Continue(()),
            }
        }
    }

    fn on_ty_class__top_down<Ex, En>(
        &mut self,
        elem: &mut Class_<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.set_in_enum_class(match elem.kind {
            ClassishKind::CenumClass(_) => true,
            ClassishKind::Cclass(..)
            | ClassishKind::Cinterface
            | ClassishKind::Cenum
            | ClassishKind::Ctrait => false,
        });
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_class_const_kind_top_down<Ex, En>(
        &mut self,
        elem: &mut ClassConstKind<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.set_enforce_const_expr(
            !self.in_enum_class() && matches!(elem, ClassConstKind::CCConcrete(_)),
        );
        ControlFlow::Continue(())
    }

    fn on_ty_typedef_top_down<Ex, En>(
        &mut self,
        elem: &mut Typedef<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_gconst_top_down<Ex, En>(
        &mut self,
        elem: &mut Gconst<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.mode = elem.mode;
        self.set_enforce_const_expr(true);
        ControlFlow::Continue(())
    }

    fn on_ty_fun_def_top_down<Ex, En>(
        &mut self,
        elem: &mut FunDef<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }

    fn on_ty_module_def_top_down<Ex, En>(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.mode = elem.mode;
        ControlFlow::Continue(())
    }
}
