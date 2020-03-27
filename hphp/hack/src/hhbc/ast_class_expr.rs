// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_scope_rust::Scope;
use env::emitter::Emitter;
use global_state::LazyState;
use hhbc_ast_rust::SpecialClsRef;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::InstrSeq;
use naming_special_names_rust::classes;
use oxidized::{aast::*, ast, ast_defs};

#[derive(Debug)]
pub enum ClassExpr {
    Special(SpecialClsRef),
    Id(ast_defs::Id),
    Expr(ast::Expr),
    Reified(InstrSeq),
}

impl ClassExpr {
    fn get_original_class_name(
        emitter: &Emitter,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope,
    ) -> Option<String> {
        if let Some(cd) = scope.get_class() {
            if (cd.kind != ast_defs::ClassKind::Ctrait || check_traits) && resolve_self {
                let class_name = &cd.name.1;
                if string_utils::closures::unmangle_closure(class_name).is_none() {
                    return Some(class_name.to_string());
                } else if let Some(c) = emitter.emit_state().get_closure_enclosing_class(class_name)
                {
                    if c.kind != ast_defs::ClassKind::Ctrait {
                        return Some(c.name.clone());
                    }
                }
            }
        }
        None
    }

    fn get_parent_class_name(class: &ast::Class_) -> Option<String> {
        if let [Hint(_, hint)] = &class.extends[..] {
            if let Hint_::Happly(ast_defs::Id(_, parent_cid), _) = &**hint {
                return Some(parent_cid.to_string());
            }
        }
        None
    }

    fn get_original_parent_class_name(
        emitter: &Emitter,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope,
    ) -> Option<String> {
        if let Some(cd) = scope.get_class() {
            if cd.kind == ast_defs::ClassKind::Cinterface {
                return Some(classes::PARENT.to_string());
            };
            if (cd.kind != ast_defs::ClassKind::Ctrait || check_traits) && resolve_self {
                let class_name = &cd.name.1;
                if string_utils::closures::unmangle_closure(class_name).is_none() {
                    return Self::get_parent_class_name(cd);
                } else if let Some(c) = emitter.emit_state().get_closure_enclosing_class(class_name)
                {
                    return c.parent_class_name.clone();
                }
            }
        }
        None
    }

    pub fn expr_to_class_expr(
        emitter: &Emitter,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope,
        expr: ast::Expr,
    ) -> Self {
        match expr.1 {
            Expr_::Id(x) => {
                let ast_defs::Id(pos, id) = *x;
                if string_utils::is_static(&id) {
                    Self::Special(SpecialClsRef::Static)
                } else if string_utils::is_parent(&id) {
                    match Self::get_original_parent_class_name(
                        emitter,
                        check_traits,
                        resolve_self,
                        scope,
                    ) {
                        Some(name) => Self::Id(ast_defs::Id(pos, name)),
                        None => Self::Special(SpecialClsRef::Parent),
                    }
                } else if string_utils::is_self(&id) {
                    match Self::get_original_class_name(emitter, check_traits, resolve_self, scope)
                    {
                        Some(name) => Self::Id(ast_defs::Id(pos, name)),
                        None => Self::Special(SpecialClsRef::Self_),
                    }
                } else {
                    Self::Id(ast_defs::Id(pos, id))
                }
            }
            _ => Self::Expr(expr),
        }
    }

    pub fn class_id_to_class_expr(
        emitter: &Emitter,
        check_traits: bool,
        resolve_self: bool,
        scope: &Scope,
        cid: &ast::ClassId,
    ) -> Self {
        let ClassId(annot, cid_) = cid;
        let expr = match cid_ {
            ClassId_::CIexpr(e) => e.clone(),
            ClassId_::CI(sid) => Expr(annot.clone(), Expr_::mk_id(sid.clone())),
            ClassId_::CIparent => return Self::Special(SpecialClsRef::Parent),
            ClassId_::CIstatic => return Self::Special(SpecialClsRef::Static),
            ClassId_::CIself => return Self::Special(SpecialClsRef::Self_),
        };
        Self::expr_to_class_expr(emitter, check_traits, resolve_self, scope, expr)
    }
}
