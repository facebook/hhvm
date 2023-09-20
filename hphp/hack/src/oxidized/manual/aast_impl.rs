// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use crate::aast::*;
use crate::ast_defs;
use crate::pos::Pos;

macro_rules! vec_wrapper {
    (<$($tparam:ident),* $(,)?> $ty:ty , $elem:ty) => {
        impl<$($tparam,)*> $ty {
            #[inline]
            pub fn as_slice(&self) -> &[$elem] {
                self.0.as_slice()
            }
            #[inline]
            pub fn as_mut_slice(&mut self) -> &mut [$elem] {
                self.0.as_mut_slice()
            }
            #[inline]
            pub fn len(&self) -> usize {
                self.0.len()
            }
            #[inline]
            pub fn is_empty(&self) -> bool {
                self.0.is_empty()
            }
            #[inline]
            pub fn push(&mut self, stmt: $elem) {
                self.0.push(stmt)
            }
            #[inline]
            pub fn insert(&mut self, index: usize, stmt: $elem) {
                self.0.insert(index, stmt)
            }
            #[inline]
            pub fn drain<R>(&mut self, range: R) -> std::vec::Drain<'_, $elem>
            where
                R: std::ops::RangeBounds<usize>,
            {
                self.0.drain(range)
            }
            #[inline]
            pub fn clear(&mut self) {
                self.0.clear()
            }
            #[inline]
            pub fn iter(&self) -> std::slice::Iter<'_, $elem> {
                self.0.iter()
            }
            #[inline]
            pub fn iter_mut(&mut self) -> std::slice::IterMut<'_, $elem> {
                self.0.iter_mut()
            }
        }
        impl<$($tparam,)*> IntoIterator for $ty {
            type Item = $elem;
            type IntoIter = std::vec::IntoIter<$elem>;
            #[inline]
            fn into_iter(self) -> Self::IntoIter {
                self.0.into_iter()
            }
        }
        impl<'a, $($tparam,)*> IntoIterator for &'a $ty {
            type Item = &'a $elem;
            type IntoIter = std::slice::Iter<'a, $elem>;
            #[inline]
            fn into_iter(self) -> Self::IntoIter {
                self.iter()
            }
        }
        impl<'a, $($tparam,)*> IntoIterator for &'a mut $ty {
            type Item = &'a mut $elem;
            type IntoIter = std::slice::IterMut<'a, $elem>;
            #[inline]
            fn into_iter(self) -> Self::IntoIter {
                self.iter_mut()
            }
        }
        impl<$($tparam,)*> AsRef<[$elem]> for $ty {
            #[inline]
            fn as_ref(&self) -> &[$elem] {
                self.as_slice()
            }
        }
        impl<$($tparam,)*> AsMut<[$elem]> for $ty {
            #[inline]
            fn as_mut(&mut self) -> &mut [$elem] {
                self.as_mut_slice()
            }
        }
        impl<$($tparam,)*> std::borrow::Borrow<[$elem]> for $ty {
            #[inline]
            fn borrow(&self) -> &[$elem] {
                self.as_slice()
            }
        }
        impl<$($tparam,)*> std::borrow::BorrowMut<[$elem]> for $ty {
            #[inline]
            fn borrow_mut(&mut self) -> &mut [$elem] {
                self.as_mut_slice()
            }
        }
        impl<$($tparam,)*> std::ops::Deref for $ty {
            type Target = [$elem];
            #[inline]
            fn deref(&self) -> &Self::Target {
                self.as_slice()
            }
        }
        impl<$($tparam,)*> std::ops::DerefMut for $ty {
            #[inline]
            fn deref_mut(&mut self) -> &mut Self::Target {
                self.as_mut_slice()
            }
        }
        impl<I, $($tparam,)*> std::ops::Index<I> for $ty
        where I: std::slice::SliceIndex<[$elem]>
        {
            type Output = <I as std::slice::SliceIndex<[$elem]>>::Output;
            #[inline]
            fn index(&self, index: I) -> &Self::Output {
                std::ops::Index::index(&self.0, index)
            }
        }
        impl<I, $($tparam,)*> std::ops::IndexMut<I> for $ty
        where I: std::slice::SliceIndex<[$elem]>
        {
            #[inline]
            fn index_mut(&mut self, index: I) -> &mut Self::Output {
                std::ops::IndexMut::index_mut(&mut self.0, index)
            }
        }
        impl<$($tparam,)*> Default for $ty {
            #[inline]
            fn default() -> Self {
                Self(Default::default())
            }
        }
        impl<$($tparam,)*> From<Vec<$elem>> for $ty {
            #[inline]
            fn from(vec: Vec<$elem>) -> Self {
                Self(vec)
            }
        }
        impl<$($tparam,)*> From<$ty> for Vec<$elem> {
            #[inline]
            fn from(x: $ty) -> Vec<$elem> {
                x.0
            }
        }
        impl<$($tparam,)*> FromIterator<$elem> for $ty {
            #[inline]
            fn from_iter<I>(iter: I) -> Self
            where I: IntoIterator<Item = $elem>
            {
                Self(Vec::from_iter(iter))
            }
        }
        impl<$($tparam,)*> Extend<$elem> for $ty {
            #[inline]
            fn extend<T>(&mut self, iter: T)
            where T: IntoIterator<Item = $elem>
            {
                self.0.extend(iter)
            }
        }
    }
}

vec_wrapper!(<Ex, En> Program<Ex, En>, Def<Ex, En>);
vec_wrapper!(<Ex, En> Block<Ex, En>, Stmt<Ex, En>);
vec_wrapper!(<Ex, En> FinallyBlock<Ex, En>, Stmt<Ex, En>);
vec_wrapper!(<Ex, En> UserAttributes<Ex, En>, UserAttribute<Ex, En>);

impl<Ex, En> Program<Ex, En> {
    pub fn defs(&self) -> DefsIterator<'_, Ex, En> {
        let iter = match self.0.as_slice() {
            [Def::Namespace(defs)] => defs.1.iter(),
            _ => self.iter(),
        };
        DefsIterator { stack: vec![iter] }
    }

    pub fn first_pos(&self) -> Option<&Pos> {
        self.iter().find_map(|def| match def {
            Def::Fun(fd) => Some(fd.name.pos()),
            Def::Class(class) => Some(class.name.pos()),
            Def::Stmt(stmt) => Some(&stmt.0),
            Def::Typedef(td) => Some(td.name.pos()),
            Def::Constant(gc) => Some(gc.name.pos()),
            Def::Namespace(ns) => Some(ns.0.pos()),
            Def::Module(md) => Some(md.name.pos()),
            Def::SetModule(sid) => Some(sid.pos()),
            Def::NamespaceUse(sids) => sids.first().map(|(_, sid, _)| sid.pos()),
            Def::SetNamespaceEnv(..) => None,
            Def::FileAttributes(fa) => fa.user_attributes.first().map(|ua| ua.name.pos()),
        })
    }
}

pub struct DefsIterator<'a, Ex, En> {
    stack: Vec<std::slice::Iter<'a, Def<Ex, En>>>,
}

impl<'a, Ex, En> Iterator for DefsIterator<'a, Ex, En> {
    type Item = &'a Def<Ex, En>;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            let iter = match self.stack.last_mut() {
                Some(iter) => iter,
                None => return None,
            };
            let def = match iter.next() {
                Some(def) => def,
                None => {
                    self.stack.pop();
                    continue;
                }
            };
            match def {
                def @ (Def::Fun(_) | Def::Class(_) | Def::Typedef(_) | Def::Constant(_)) => {
                    return Some(def);
                }
                Def::Namespace(defs) => self.stack.push(defs.1.iter()),
                Def::Stmt(_)
                | Def::Module(_)
                | Def::SetModule(_)
                | Def::NamespaceUse(_)
                | Def::SetNamespaceEnv(_)
                | Def::FileAttributes(_) => {}
            }
        }
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
        if let Some(Expr(_, _, Expr_::Binop(binop))) = &self.1.as_expr() {
            if let ast_defs::Bop::Eq(_) = binop.as_ref().bop {
                return true;
            }
        }
        false
    }

    pub fn is_null_expr(&self) -> bool {
        matches!(&self.1.as_expr(), Some(Expr(_, _, Expr_::Null)))
    }

    pub fn is_declare_local_stmt(&self) -> bool {
        if let Stmt(_, Stmt_::DeclareLocal(_)) = self {
            true
        } else {
            false
        }
    }
}

impl<Ex, En> Expr<Ex, En> {
    pub fn new(ex: Ex, pos: Pos, e: Expr_<Ex, En>) -> Self {
        Self(ex, pos, e)
    }

    pub fn pos(&self) -> &Pos {
        &self.1
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
    std::sync::Arc::new(env)
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
impl<Ex: Clone, En: Clone> From<Method_<Ex, En>> for Cow<'_, Method_<Ex, En>> {
    fn from(x: Method_<Ex, En>) -> Self {
        Cow::Owned(x)
    }
}

impl<'a, Ex: Clone, En: Clone> From<&'a Method_<Ex, En>> for Cow<'a, Method_<Ex, En>> {
    fn from(x: &'a Method_<Ex, En>) -> Self {
        Cow::Borrowed(x)
    }
}
