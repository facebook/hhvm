// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::{
    ast,
    ast_defs::{FunKind, Id},
    pos::Pos,
    s_map,
};
use rx_rust as rx;
use std::borrow::Cow;

#[derive(Clone, Debug)]
pub struct LongLambda {
    pub is_static: bool,
    pub is_async: bool,
    pub rx_level: Option<rx::Level>,
}

#[derive(Clone, Debug)]
pub struct Lambda {
    pub is_async: bool,
    pub rx_level: Option<rx::Level>,
}

#[derive(Clone, Debug)]
pub enum ScopeItem<'a> {
    Class(Cow<'a, ast::Class_>),
    Function(Cow<'a, ast::Fun_>),
    Method(Cow<'a, ast::Method_>),
    LongLambda(Cow<'a, LongLambda>),
    Lambda(Cow<'a, Lambda>),
}

impl ScopeItem<'_> {
    pub fn is_in_lambda(&self) -> bool {
        match self {
            ScopeItem::Lambda(_) | ScopeItem::LongLambda(_) => true,
            _ => false,
        }
    }
}

#[derive(Clone, Default, Debug)]
pub struct Scope<'a> {
    pub items: Vec<ScopeItem<'a>>,
}

impl<'a> Scope<'a> {
    pub fn toplevel() -> Self {
        Scope { items: vec![] }
    }

    pub fn push_item(&mut self, s: ScopeItem<'a>) {
        self.items.push(s)
    }

    pub fn iter(&self) -> impl ExactSizeIterator<Item = &ScopeItem> {
        self.items.iter().rev()
    }

    pub fn iter_subscopes(&self) -> impl Iterator<Item = &[ScopeItem]> {
        (1..self.items.len()).rev().map(move |i| &self.items[..i])
    }

    pub fn get_subscope_class<'b>(sub_scope: &'b [ScopeItem<'b>]) -> Option<&'b ast::Class_> {
        for scope_item in sub_scope.iter().rev() {
            if let ScopeItem::Class(cd) = scope_item {
                return Some(cd.as_ref());
            }
        }
        None
    }

    pub fn get_class(&self) -> Option<&ast::Class_> {
        Self::get_subscope_class(&self.items[..])
    }

    pub fn get_span(&self) -> Pos {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(cd) => {
                    return cd.span.clone();
                }
                ScopeItem::Function(fd) => {
                    return fd.span.clone();
                }
                ScopeItem::Method(md) => {
                    return md.span.clone();
                }
                _ => (),
            }
        }
        Pos::make_none()
    }

    pub fn get_tparams(&self) -> Vec<&ast::Tparam> {
        let mut tparams = vec![];
        let extend_shallowly = &mut |tps| {
            for tparam in tps {
                tparams.push(tparam);
            }
        };
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(cd) => {
                    extend_shallowly(cd.tparams.list.as_slice());
                }
                ScopeItem::Function(fd) => {
                    extend_shallowly(fd.tparams.as_slice());
                }
                ScopeItem::Method(md) => {
                    extend_shallowly(md.tparams.as_slice());
                }
                _ => (),
            }
        }
        tparams
    }

    pub fn get_fun_tparams(&self) -> &[ast::Tparam] {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return &[];
                }
                ScopeItem::Function(fd) => {
                    return &fd.tparams[..];
                }
                ScopeItem::Method(md) => {
                    return &md.tparams[..];
                }
                _ => (),
            }
        }
        &[]
    }

    pub fn get_class_tparams(&self) -> Cow<ast::ClassTparams> {
        for scope_item in self.items.iter().rev() {
            if let ScopeItem::Class(cd) = scope_item {
                return Cow::Borrowed(&cd.tparams);
            }
        }
        Cow::Owned(ast::ClassTparams {
            list: vec![],
            constraints: s_map::SMap::new(),
        })
    }

    pub fn has_this(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) | ScopeItem::Function(_) => {
                    return false;
                }
                ScopeItem::Method(_) => {
                    return true;
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_in_async(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return false;
                }
                ScopeItem::Method(Cow::Borrowed(ast::Method_ { fun_kind, .. }))
                | ScopeItem::Method(Cow::Owned(ast::Method_ { fun_kind, .. }))
                | ScopeItem::Function(Cow::Borrowed(ast::Fun_ { fun_kind, .. }))
                | ScopeItem::Function(Cow::Owned(ast::Fun_ { fun_kind, .. })) => {
                    return *fun_kind == FunKind::FAsync || *fun_kind == FunKind::FAsyncGenerator;
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_toplevel(&self) -> bool {
        self.items.is_empty()
    }

    pub fn is_in_static_method(&self) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Method(md) => {
                    return md.static_;
                }
                ScopeItem::LongLambda(ll) => {
                    if ll.as_ref().is_static {
                        return false;
                    }
                }
                ScopeItem::Lambda(_) => (),
                _ => return false,
            }
        }
        false
    }

    pub fn is_in_lambda(&self) -> bool {
        self.items
            .last()
            .map(&ScopeItem::is_in_lambda)
            .unwrap_or(false)
    }

    pub fn rx_of_scope(&self) -> rx::Level {
        fn from_uas(user_attrs: &Vec<ast::UserAttribute>) -> rx::Level {
            rx::Level::from_ast(user_attrs).unwrap_or(rx::Level::NonRx)
        }
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Class(_) => {
                    return rx::Level::NonRx;
                }
                ScopeItem::Method(m) => return from_uas(&m.as_ref().user_attributes),
                ScopeItem::Function(f) => return from_uas(&f.as_ref().user_attributes),
                ScopeItem::Lambda(Cow::Borrowed(Lambda {
                    rx_level: Some(ref rl),
                    ..
                }))
                | ScopeItem::Lambda(Cow::Owned(Lambda {
                    rx_level: Some(ref rl),
                    ..
                }))
                | ScopeItem::LongLambda(Cow::Borrowed(LongLambda {
                    rx_level: Some(ref rl),
                    ..
                }))
                | ScopeItem::LongLambda(Cow::Owned(LongLambda {
                    rx_level: Some(ref rl),
                    ..
                })) => {
                    return *rl;
                }
                _ => (),
            }
        }
        rx::Level::NonRx
    }

    pub fn has_function_attribute(&self, attr_name: impl AsRef<str>) -> bool {
        for scope_item in self.items.iter().rev() {
            match scope_item {
                ScopeItem::Method(Cow::Borrowed(ast::Method_ {
                    user_attributes, ..
                }))
                | ScopeItem::Method(Cow::Owned(ast::Method_ {
                    user_attributes, ..
                }))
                | ScopeItem::Function(Cow::Borrowed(ast::Fun_ {
                    user_attributes, ..
                }))
                | ScopeItem::Function(Cow::Owned(ast::Fun_ {
                    user_attributes, ..
                })) => {
                    return user_attributes
                        .iter()
                        .any(|attr| attr.name.1 == attr_name.as_ref());
                }
                _ => (),
            }
        }
        false
    }

    pub fn is_static(&self) -> bool {
        for x in self.items.iter().rev() {
            match x {
                ScopeItem::LongLambda(x) => {
                    if x.is_static {
                        return true;
                    }
                }
                ScopeItem::Function(_) => return true,
                ScopeItem::Method(md) => return md.static_,
                ScopeItem::Lambda(_) => continue,
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
                .vars
                .iter()
                .map(|var| {
                    let Id(_, id) = &var.id;
                    format!("${}", id)
                })
                .collect::<Vec<_>>(),
            _ => panic!("closure scope should be lambda -> method -> class"),
        }
    }
}
