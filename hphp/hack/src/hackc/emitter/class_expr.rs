// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_scope::Scope;
use hhbc::ClassishKind;
use hhbc::SpecialClsRef;
use hhbc_string_utils as string_utils;
use instruction_sequence::InstrSeq;
use naming_special_names_rust::classes;
use naming_special_names_rust::user_attributes;
use oxidized::aast::*;
use oxidized::ast;
use oxidized::ast_defs;
use string_utils::reified::ReifiedTparam;

use crate::emitter::Emitter;

#[derive(Debug)]
pub enum ClassExpr<'arena> {
    Special(SpecialClsRef),
    Id(ast_defs::Id),
    Expr(ast::Expr),
    Reified(InstrSeq<'arena>),
}

impl<'arena> ClassExpr<'arena> {
    fn get_original_class_name<'decl>(
        emitter: &Emitter<'arena, 'decl>,
        check_traits: bool,
        resolve_self: bool,
        opt_class_info: Option<(ClassishKind, &str)>,
    ) -> Option<String> {
        if let Some((kind, class_name)) = opt_class_info {
            if (kind != ClassishKind::Trait || check_traits) && resolve_self {
                if string_utils::closures::unmangle_closure(class_name).is_none() {
                    return Some(class_name.to_string());
                } else if let Some(c) = emitter
                    .global_state()
                    .get_closure_enclosing_class(class_name)
                {
                    if ClassishKind::from(c.kind.clone()) != ClassishKind::Trait {
                        return Some(c.name.clone());
                    }
                }
            }
        }
        None
    }

    pub fn get_parent_class_name<'a>(class: &'a ast_scope::Class<'a>) -> Option<&'a str> {
        if let [Hint(_, hint)] = class.get_extends() {
            if let Hint_::Happly(ast_defs::Id(_, parent_cid), _) = &**hint {
                return Some(parent_cid);
            }
        }
        None
    }

    fn get_original_parent_class_name<'decl>(
        emitter: &Emitter<'arena, 'decl>,
        check_traits: bool,
        resolve_self: bool,
        opt_class_info: Option<(ClassishKind, &str)>,
        opt_parent_name: Option<String>,
    ) -> Option<String> {
        if let Some((kind, class_name)) = opt_class_info {
            if kind == ClassishKind::Interface {
                return Some(classes::PARENT.to_string());
            };
            if (kind != ClassishKind::Trait || check_traits) && resolve_self {
                if string_utils::closures::unmangle_closure(class_name).is_none() {
                    return opt_parent_name;
                } else if let Some(c) = emitter
                    .global_state()
                    .get_closure_enclosing_class(class_name)
                {
                    return c.parent_class_name.clone();
                }
            }
        }
        None
    }

    fn expr_to_class_expr<'a, 'decl>(
        emitter: &Emitter<'arena, 'decl>,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope<'a, 'arena>,
        expr: ast::Expr,
    ) -> Self {
        if let Some(cd) = scope.get_class() {
            Self::expr_to_class_expr_(
                emitter,
                check_traits,
                resolve_self,
                Some((ClassishKind::from(cd.get_kind()), cd.get_name_str())),
                Self::get_parent_class_name(cd).map(String::from),
                expr,
            )
        } else {
            Self::expr_to_class_expr_(emitter, check_traits, resolve_self, None, None, expr)
        }
    }

    pub fn expr_to_class_expr_<'decl>(
        emitter: &Emitter<'arena, 'decl>,
        check_traits: bool,
        resolve_self: bool,
        opt_class_info: Option<(ClassishKind, &str)>,
        opt_parent_name: Option<String>,
        expr: ast::Expr,
    ) -> Self {
        match expr.2 {
            Expr_::Id(x) => {
                let ast_defs::Id(pos, id) = *x;
                if string_utils::is_static(&id) {
                    Self::Special(SpecialClsRef::LateBoundCls)
                } else if string_utils::is_parent(&id) {
                    match Self::get_original_parent_class_name(
                        emitter,
                        check_traits,
                        resolve_self,
                        opt_class_info,
                        opt_parent_name,
                    ) {
                        Some(name) => Self::Id(ast_defs::Id(pos, name)),
                        None => Self::Special(SpecialClsRef::ParentCls),
                    }
                } else if string_utils::is_self(&id) {
                    match Self::get_original_class_name(
                        emitter,
                        check_traits,
                        resolve_self,
                        opt_class_info,
                    ) {
                        Some(name) => Self::Id(ast_defs::Id(pos, name)),
                        None => Self::Special(SpecialClsRef::SelfCls),
                    }
                } else {
                    Self::Id(ast_defs::Id(pos, id))
                }
            }
            _ => Self::Expr(expr),
        }
    }

    pub fn class_id_to_class_expr<'a, 'decl>(
        emitter: &Emitter<'arena, 'decl>,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope<'a, 'arena>,
        cid: &ast::ClassId,
    ) -> Self {
        let ClassId(_, annot, cid_) = cid;
        let expr = match cid_ {
            ClassId_::CIexpr(e) => e.clone(),
            ClassId_::CI(sid) => Expr((), annot.clone(), Expr_::mk_id(sid.clone())),
            ClassId_::CIparent => return Self::Special(SpecialClsRef::ParentCls),
            ClassId_::CIstatic => return Self::Special(SpecialClsRef::LateBoundCls),
            ClassId_::CIself => return Self::Special(SpecialClsRef::SelfCls),
        };
        Self::expr_to_class_expr(emitter, check_traits, resolve_self, scope, expr)
    }

    pub fn get_reified_tparam<'a>(
        scope: &Scope<'a, 'arena>,
        name: &str,
    ) -> Option<(ReifiedTparam, bool)> {
        fn soft(tp: &ast::Tparam) -> bool {
            tp.user_attributes
                .iter()
                .any(|ua| user_attributes::is_soft(&ua.name.1))
        }
        let reified = |(_, tp): &(usize, &ast::Tparam)| {
            matches!(tp.reified, ReifyKind::Reified | ReifyKind::SoftReified) && tp.name.1 == name
        };

        if let Some((i, tp)) = scope.get_fun_tparams().iter().enumerate().find(reified) {
            return Some((ReifiedTparam::Fun(i), soft(tp)));
        }
        if let Some((i, tp)) = scope.get_class_tparams().iter().enumerate().find(reified) {
            return Some((ReifiedTparam::Class(i), soft(tp)));
        }
        None
    }

    pub fn is_reified_tparam<'a>(scope: &Scope<'a, 'arena>, name: &str) -> bool {
        Self::get_reified_tparam(scope, name).is_some()
    }
}
