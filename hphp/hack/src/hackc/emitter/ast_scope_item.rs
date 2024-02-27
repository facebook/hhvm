// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use hhbc::Coeffects;
use oxidized::ast;
use oxidized::file_info;
use oxidized::pos::Pos;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Lambda {
    pub is_long: bool,
    pub is_async: bool,
    pub coeffects: Coeffects,
    pub pos: Pos,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ScopeItem<'a> {
    Class(Class<'a>),
    Function(Fun<'a>),
    Method(Method<'a>),
    Lambda(Lambda),
}

impl<'a> ScopeItem<'a> {
    pub fn get_span(&self) -> &Pos {
        match self {
            ScopeItem::Class(cd) => cd.get_span(),
            ScopeItem::Function(fd) => fd.get_span(),
            ScopeItem::Method(md) => md.get_span(),
            ScopeItem::Lambda(lambda) => &lambda.pos,
        }
    }

    pub fn is_in_lambda(&self) -> bool {
        matches!(self, ScopeItem::Lambda(_))
    }

    pub fn is_in_long_lambda(&self) -> bool {
        match self {
            ScopeItem::Lambda(inner) => inner.is_long,
            _ => false,
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Class<'a> {
    Borrowed(&'a ast::Class_),
    Counted(Rc<Class_>),
}

impl<'a> Class<'a> {
    pub fn new_ref(ast: &'a ast::Class_) -> Self {
        Self::Borrowed(ast)
    }

    pub fn new_rc(x: &ast::Class_) -> Self {
        Self::Counted(Rc::new(Class_::new(x)))
    }

    pub fn get_tparams(&self) -> &[ast::Tparam] {
        match self {
            Self::Borrowed(x) => &x.tparams,
            Self::Counted(x) => &x.tparams,
        }
    }

    pub fn get_span(&self) -> &Pos {
        match self {
            Self::Borrowed(x) => &x.span,
            Self::Counted(x) => &x.span,
        }
    }

    pub fn get_name(&self) -> &ast::Id {
        match self {
            Self::Borrowed(x) => &x.name,
            Self::Counted(x) => &x.name,
        }
    }

    pub fn get_name_str(&self) -> &str {
        &self.get_name().1
    }

    pub fn get_mode(&self) -> file_info::Mode {
        match self {
            Self::Borrowed(x) => x.mode,
            Self::Counted(x) => x.mode,
        }
    }

    pub fn get_kind(&self) -> ast::ClassishKind {
        match self {
            Self::Borrowed(x) => x.kind,
            Self::Counted(x) => x.kind,
        }
    }

    pub fn get_extends(&self) -> &[ast::Hint] {
        match self {
            Self::Borrowed(x) => &x.extends,
            Self::Counted(x) => &x.extends,
        }
    }

    pub fn get_vars(&self) -> &[ast::ClassVar] {
        match self {
            Self::Borrowed(x) => &x.vars,
            Self::Counted(x) => &x.vars,
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Fun<'a> {
    Borrowed(&'a ast::FunDef),
    Counted(Rc<Fun_>),
}

impl<'a> Fun<'a> {
    pub fn new_ref(ast: &'a ast::FunDef) -> Self {
        Self::Borrowed(ast)
    }

    pub fn new_rc(x: &ast::FunDef) -> Self {
        Self::Counted(Rc::new(Fun_::new(x)))
    }

    pub fn get_tparams(&self) -> &[ast::Tparam] {
        match self {
            Self::Borrowed(x) => &x.tparams,
            Self::Counted(x) => &x.tparams,
        }
    }

    pub(crate) fn get_user_attributes(&self) -> &[ast::UserAttribute] {
        match self {
            Self::Borrowed(x) => &x.fun.user_attributes,
            Self::Counted(x) => &x.user_attributes,
        }
    }

    pub fn get_ctxs(&self) -> Option<&ast::Contexts> {
        match self {
            Self::Borrowed(x) => x.fun.ctxs.as_ref(),
            Self::Counted(x) => x.ctxs.as_ref(),
        }
    }

    pub fn get_params(&self) -> &[ast::FunParam] {
        match self {
            Self::Borrowed(x) => &x.fun.params,
            Self::Counted(x) => &x.params,
        }
    }

    pub fn get_span(&self) -> &Pos {
        match self {
            Self::Borrowed(x) => &x.fun.span,
            Self::Counted(x) => &x.span,
        }
    }

    pub fn get_name(&self) -> &ast::Id {
        match self {
            Self::Borrowed(x) => &x.name,
            Self::Counted(x) => &x.name,
        }
    }

    pub fn get_name_str(&self) -> &str {
        &self.get_name().1
    }

    pub fn get_mode(&self) -> file_info::Mode {
        match self {
            Self::Borrowed(x) => x.mode,
            Self::Counted(x) => x.mode,
        }
    }

    pub fn get_fun_kind(&self) -> ast::FunKind {
        match self {
            Self::Borrowed(x) => x.fun.fun_kind,
            Self::Counted(x) => x.fun_kind,
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Method<'a> {
    Borrowed(&'a ast::Method_),
    Counted(Rc<Method_>),
}

impl<'a> Method<'a> {
    pub fn new_ref(ast: &'a ast::Method_) -> Self {
        Self::Borrowed(ast)
    }

    pub fn new_rc(x: &ast::Method_) -> Self {
        Self::Counted(Rc::new(Method_::new(x)))
    }

    pub fn get_tparams(&self) -> &[ast::Tparam] {
        match self {
            Self::Borrowed(m) => &m.tparams,
            Self::Counted(m) => &m.tparams,
        }
    }

    pub fn is_static(&self) -> bool {
        match self {
            Self::Borrowed(m) => m.static_,
            Self::Counted(m) => m.static_,
        }
    }

    pub(crate) fn get_user_attributes(&self) -> &[ast::UserAttribute] {
        match self {
            Self::Borrowed(x) => &x.user_attributes,
            Self::Counted(x) => &x.user_attributes,
        }
    }

    pub fn get_ctxs(&self) -> Option<&ast::Contexts> {
        match self {
            Self::Borrowed(x) => x.ctxs.as_ref(),
            Self::Counted(x) => x.ctxs.as_ref(),
        }
    }

    pub fn get_params(&self) -> &[ast::FunParam] {
        match self {
            Self::Borrowed(x) => &x.params,
            Self::Counted(x) => &x.params,
        }
    }

    pub fn get_span(&self) -> &Pos {
        match self {
            Self::Borrowed(x) => &x.span,
            Self::Counted(x) => &x.span,
        }
    }

    pub fn get_name(&self) -> &ast::Id {
        match self {
            Self::Borrowed(x) => &x.name,
            Self::Counted(x) => &x.name,
        }
    }

    pub fn get_name_str(&self) -> &str {
        &self.get_name().1
    }

    pub fn get_fun_kind(&self) -> ast::FunKind {
        match self {
            Self::Borrowed(x) => x.fun_kind,
            Self::Counted(x) => x.fun_kind,
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct Class_ {
    name: ast::Id,
    span: Pos,
    tparams: Vec<ast::Tparam>,
    vars: Vec<ast::ClassVar>,
    mode: file_info::Mode,
    kind: ast::ClassishKind,
    extends: Vec<ast::Hint>,
}

impl Class_ {
    fn new(c: &ast::Class_) -> Self {
        Self {
            name: c.name.clone(),
            span: c.span.clone(),
            tparams: c.tparams.clone(),
            vars: c.vars.clone(),
            mode: c.mode,
            kind: c.kind.clone(),
            extends: c.extends.clone(),
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct Fun_ {
    name: ast::Id,
    span: Pos,
    tparams: Vec<ast::Tparam>,
    user_attributes: Vec<ast::UserAttribute>,
    mode: file_info::Mode,
    fun_kind: ast::FunKind,
    ctxs: Option<ast::Contexts>,
    params: Vec<ast::FunParam>,
}

impl Fun_ {
    fn new(fd: &ast::FunDef) -> Self {
        let f = &fd.fun;
        Self {
            name: fd.name.clone(),
            span: f.span.clone(),
            tparams: fd.tparams.clone(),
            user_attributes: f.user_attributes.clone().into(),
            mode: fd.mode,
            fun_kind: f.fun_kind,
            ctxs: f.ctxs.clone(),
            params: f.params.clone(),
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct Method_ {
    name: ast::Id,
    span: Pos,
    tparams: Vec<ast::Tparam>,
    user_attributes: Vec<ast::UserAttribute>,
    static_: bool,
    fun_kind: ast::FunKind,
    ctxs: Option<ast::Contexts>,
    params: Vec<ast::FunParam>,
}

impl Method_ {
    fn new(m: &ast::Method_) -> Self {
        Self {
            name: m.name.clone(),
            span: m.span.clone(),
            tparams: m.tparams.clone(),
            static_: m.static_,
            user_attributes: m.user_attributes.clone().into(),
            fun_kind: m.fun_kind,
            ctxs: m.ctxs.clone(),
            params: m.params.clone(),
        }
    }
}
