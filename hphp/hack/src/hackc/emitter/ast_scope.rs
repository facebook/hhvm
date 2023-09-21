// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod ast_scope_item;
use std::borrow::Cow;

use hhbc::Coeffects;
use oxidized::ast;
use oxidized::ast_defs::FunKind;
use oxidized::ast_defs::Id;
use oxidized::pos::Pos;

pub use crate::ast_scope_item::Class;
pub use crate::ast_scope_item::Fun;
pub use crate::ast_scope_item::Lambda;
pub use crate::ast_scope_item::Method;
pub use crate::ast_scope_item::ScopeItem;

#[derive(Clone, Default, Debug, Eq, PartialEq)]
pub struct Scope<'a, 'arena> {
    items: Vec<ScopeItem<'a, 'arena>>,
    class_cache: Option<Class<'a>>,
}

impl<'a, 'arena> Scope<'a, 'arena> {
    pub fn with_item(item: ScopeItem<'a, 'arena>) -> Self {
        let mut scope = Self::default();
        scope.push_item(item);
        scope
    }

    pub fn push_item(&mut self, s: ScopeItem<'a, 'arena>) {
        if let ScopeItem::Class(cd) = &s {
            self.class_cache = Some(cd.clone());
        }
        self.items.push(s)
    }

    pub fn pop_item(&mut self) -> Option<ScopeItem<'a, 'arena>> {
        let s = self.items.pop();
        if let Some(ScopeItem::Class(_)) = &s {
            self.class_cache = None;
        }
        s
    }

    pub fn items(&self) -> &[ScopeItem<'a, 'arena>] {
        &self.items
    }

    pub fn iter(&self) -> impl ExactSizeIterator<Item = &ScopeItem<'a, 'arena>> {
        self.items.iter().rev()
    }

    pub fn iter_subscopes(&self) -> impl Iterator<Item = &[ScopeItem<'a, 'arena>]> {
        (0..self.items.len()).rev().map(move |i| &self.items[..i])
    }

    pub fn top(&self) -> Option<&ScopeItem<'a, 'arena>> {
        self.items.last()
    }

    pub fn get_class(&self) -> Option<&Class<'_>> {
        self.class_cache.as_ref()
    }

    pub fn get_span(&self) -> Option<&Pos> {
        self.top().map(ScopeItem::get_span)
    }

    pub fn get_span_or_none<'b>(&'b self) -> Cow<'b, Pos> {
        if let Some(pos) = self.get_span() {
            Cow::Borrowed(pos)
        } else {
            Cow::Owned(Pos::NONE)
        }
    }

    pub fn get_tparams(&self) -> Vec<&ast::Tparam> {
        let mut tparams = vec![];
        let extend_shallowly = &mut |tps| {
            for tparam in tps {
                tparams.push(tparam);
            }
        };
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Class(cd) => {
                    extend_shallowly(cd.get_tparams());
                }
                ScopeItem::Function(fd) => {
                    extend_shallowly(fd.get_tparams());
                }
                ScopeItem::Method(md) => {
                    extend_shallowly(md.get_tparams());
                }
                _ => {}
            }
        }
        tparams
    }

    pub fn get_fun_tparams(&self) -> &[ast::Tparam] {
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return &[];
                }
                ScopeItem::Function(fd) => {
                    return fd.get_tparams();
                }
                ScopeItem::Method(md) => {
                    return md.get_tparams();
                }
                _ => {}
            }
        }
        &[]
    }

    pub fn get_class_tparams(&self) -> &[ast::Tparam] {
        for scope_item in self.iter() {
            if let ScopeItem::Class(cd) = scope_item {
                return cd.get_tparams();
            }
        }
        &[]
    }

    pub fn has_this(&self) -> bool {
        if self.items.is_empty() {
            /* Assume top level has this */
            return true;
        }
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Class(_) | ScopeItem::Function(_) => {
                    return false;
                }
                ScopeItem::Method(_) => {
                    return true;
                }
                _ => {}
            }
        }
        false
    }

    pub fn is_in_async(&self) -> bool {
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return false;
                }
                ScopeItem::Method(m) => {
                    let fun_kind = m.get_fun_kind();
                    return fun_kind == FunKind::FAsync || fun_kind == FunKind::FAsyncGenerator;
                }
                ScopeItem::Function(f) => {
                    let fun_kind = f.get_fun_kind();
                    return fun_kind == FunKind::FAsync || fun_kind == FunKind::FAsyncGenerator;
                }
                _ => {}
            }
        }
        false
    }

    pub fn is_toplevel(&self) -> bool {
        self.items.is_empty()
    }

    pub fn is_in_static_method(&self) -> bool {
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Method(md) => {
                    return md.is_static();
                }
                ScopeItem::Lambda(_) => {}
                _ => return false,
            }
        }
        false
    }

    pub fn is_in_lambda(&self) -> bool {
        self.items.last().map_or(false, ScopeItem::is_in_lambda)
    }

    pub fn coeffects_of_scope(&self, alloc: &'arena bumpalo::Bump) -> Coeffects<'arena> {
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return Coeffects::default();
                }
                ScopeItem::Method(m) => {
                    return Coeffects::from_ast(
                        alloc,
                        m.get_ctxs(),
                        m.get_params(),
                        m.get_tparams(),
                        self.get_class_tparams(),
                    );
                }
                ScopeItem::Function(f) => {
                    return Coeffects::from_ast(
                        alloc,
                        f.get_ctxs(),
                        f.get_params(),
                        f.get_tparams(),
                        vec![],
                    );
                }
                ScopeItem::Lambda(Lambda { coeffects, .. })
                    if !coeffects.get_static_coeffects().is_empty() =>
                {
                    return coeffects.clone();
                }
                ScopeItem::Lambda(_) => {}
            }
        }
        Coeffects::default()
    }

    pub fn has_function_attribute(&self, attr_name: impl AsRef<str>) -> bool {
        let has = |ua: &[ast::UserAttribute]| ua.iter().any(|a| a.name.1 == attr_name.as_ref());
        for scope_item in self.iter() {
            match scope_item {
                ScopeItem::Method(m) => {
                    return has(m.get_user_attributes());
                }
                ScopeItem::Function(f) => {
                    return has(f.get_user_attributes());
                }
                _ => {}
            }
        }
        false
    }

    pub fn is_static(&self) -> bool {
        for x in self.iter() {
            match x {
                ScopeItem::Function(_) => return true,
                ScopeItem::Method(md) => return md.is_static(),
                ScopeItem::Lambda(_) => {}
                _ => return true,
            }
        }
        true
    }

    // get captured variables when in closure scope
    pub fn get_captured_vars(&self) -> Vec<String> {
        // closure scope: lambda -> method -> class
        match &self.items[..] {
            [.., ScopeItem::Class(ast_cls), _, _] => ast_cls
                .get_vars()
                .iter()
                .map(|var| {
                    let Id(_, id) = &var.id;
                    format!("${}", id)
                })
                .collect::<Vec<_>>(),
            _ => panic!("closure scope should be lambda -> method -> class"),
        }
    }

    pub fn is_in_debugger_eval_fun(&self) -> bool {
        for x in self.iter() {
            match x {
                ScopeItem::Lambda(_) => {}
                ScopeItem::Function(f) => return f.get_name().1 == "include",
                _ => return false,
            }
        }
        true
    }
}
