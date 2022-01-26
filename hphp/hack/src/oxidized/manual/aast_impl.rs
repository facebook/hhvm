// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast::*;
use crate::ast_defs;
use crate::pos::Pos;
use std::{borrow::Cow, boxed::Box};

impl<Ex, En> Program<Ex, En> {
    pub fn as_slice(&self) -> &[Def<Ex, En>] {
        self.0.as_slice()
    }

    pub fn iter(&self) -> std::slice::Iter<'_, Def<Ex, En>> {
        self.0.iter()
    }

    pub fn iter_mut(&mut self) -> std::slice::IterMut<'_, Def<Ex, En>> {
        self.0.iter_mut()
    }
}

impl<Ex, En> IntoIterator for Program<Ex, En> {
    type Item = Def<Ex, En>;
    type IntoIter = std::vec::IntoIter<Def<Ex, En>>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.into_iter()
    }
}

impl<'a, Ex, En> IntoIterator for &'a Program<Ex, En> {
    type Item = &'a Def<Ex, En>;
    type IntoIter = std::slice::Iter<'a, Def<Ex, En>>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

impl<'a, Ex, En> IntoIterator for &'a mut Program<Ex, En> {
    type Item = &'a mut Def<Ex, En>;
    type IntoIter = std::slice::IterMut<'a, Def<Ex, En>>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter_mut()
    }
}

impl<Ex, En> Stmt<Ex, En> {
    pub fn new(pos: Pos, s: Stmt_<Ex, En>) -> Self {
        Self(pos, s)
    }

    pub fn noop(pos: Pos) -> Self {
        Self::new(pos, Stmt_::Noop)
    }

    pub fn is_assign_expr(&self) -> bool {
        if let Some(Expr(_, _, Expr_::Binop(bop))) = &self.1.as_expr() {
            if let (ast_defs::Bop::Eq(_), _, _) = bop.as_ref() {
                return true;
            }
        }
        false
    }
}

impl<Ex, En> Expr<Ex, En> {
    pub fn new(ex: Ex, pos: Pos, e: Expr_<Ex, En>) -> Self {
        Self(ex, pos, e)
    }

    pub fn lvar_name(&self) -> Option<&str> {
        match &self.2 {
            Expr_::Lvar(lid) => Some(&(lid.1).1),
            _ => None,
        }
    }

    pub fn is_import(&self) -> bool {
        match &self.2 {
            Expr_::Import(_) => true,
            _ => false,
        }
    }
}

impl<En> Expr<(), En> {
    pub fn pos(&self) -> &Pos {
        &self.1
    }
    pub fn mk_lvar(p: &Pos, n: &str) -> Self {
        Self::new(
            (),
            p.clone(),
            Expr_::Lvar(Box::new(Lid(p.clone(), (0, String::from(n))))),
        )
    }

    pub fn as_class_get(&self) -> Option<(&ClassId<(), En>, &ClassGetExpr<(), En>, &PropOrMethod)> {
        self.2.as_class_get()
    }

    pub fn as_class_const(&self) -> Option<(&ClassId<(), En>, &Pstring)> {
        self.2.as_class_const()
    }

    pub fn as_id(&self) -> Option<&Sid> {
        self.2.as_id()
    }
}

impl<Ex, En> Expr_<Ex, En> {
    pub fn make_string(s: Vec<u8>) -> Self {
        Expr_::String(s.into())
    }
}

impl<Ex, En> ClassId<Ex, En> {
    pub fn annot(&self) -> &Ex {
        &self.0
    }
    pub fn get(&self) -> &ClassId_<Ex, En> {
        &self.2
    }
    pub fn as_ciexpr(&self) -> Option<&Expr<Ex, En>> {
        self.2.as_ciexpr()
    }
}

impl<Hi> Targ<Hi> {
    pub fn hint(&self) -> &Hint {
        &self.1
    }

    pub fn annot(&self) -> &Hi {
        &self.0
    }
}

impl<Hi> TypeHint<Hi> {
    pub fn get_hint(&self) -> &TypeHint_ {
        &self.1
    }
}

// This wrapper constructor can avoid other crates (HackNative, etc)
// depends on ocamlrep.
pub fn new_nsenv(env: crate::namespace_env::Env) -> Nsenv {
    ocamlrep::rc::RcOc::new(env)
}

impl<Ex, En> Afield<Ex, En> {
    pub fn value(&self) -> &Expr<Ex, En> {
        match self {
            Self::AFvalue(e) => e,
            Self::AFkvalue(_, e) => e,
        }
    }
}

// TODO(hrust): consider codegen the following
impl<'a, Ex, En> Into<Cow<'a, Method_<Ex, En>>> for Method_<Ex, En>
where
    Ex: Clone,
    En: Clone,
{
    fn into(self) -> Cow<'a, Self> {
        Cow::Owned(self)
    }
}

impl<'a, Ex, En> Into<Cow<'a, Method_<Ex, En>>> for &'a Method_<Ex, En>
where
    Ex: Clone,
    En: Clone,
{
    fn into(self) -> Cow<'a, Method_<Ex, En>> {
        Cow::Borrowed(self)
    }
}
