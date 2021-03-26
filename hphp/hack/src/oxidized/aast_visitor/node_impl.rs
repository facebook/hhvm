// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use super::{
    node::Node, node_mut::NodeMut, type_params::Params, visitor::Visitor, visitor_mut::VisitorMut,
};
use itertools::Either;
use ocamlrep::rc::RcOc;
use std::collections::BTreeMap;

macro_rules! leaf_node {
    ($ty:ty) => {
        impl<P: Params> Node<P> for $ty {}
        impl<P: Params> NodeMut<P> for $ty {}
    };
}

leaf_node!(bool);
leaf_node!(isize);
leaf_node!(String);
leaf_node!(bstr::BString);
leaf_node!(crate::pos::Pos);
leaf_node!(crate::file_info::Mode);
leaf_node!(crate::namespace_env::Env);

impl<P: Params, T> Node<P> for &T
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        (*self).accept(c, v)
    }
}

impl<P: Params, T> NodeMut<P> for &mut T
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        (*self).accept(c, v)
    }
}

impl<P: Params, L, R> Node<P> for Either<L, R>
where
    L: Node<P>,
    R: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            Either::Left(i) => i.accept(c, v),
            Either::Right(i) => i.accept(c, v),
        }
    }
}

impl<P: Params, L, R> NodeMut<P> for Either<L, R>
where
    L: NodeMut<P>,
    R: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            Either::Left(i) => i.accept(c, v),
            Either::Right(i) => i.accept(c, v),
        }
    }
}

impl<P: Params, T> Node<P> for &[T]
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for i in *self {
            i.accept(c, v)?;
        })
    }
}

impl<P: Params, T> Node<P> for Vec<T>
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for i in self {
            i.accept(c, v)?;
        })
    }
}

impl<P: Params, T> NodeMut<P> for Vec<T>
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for i in self {
            i.accept(c, v)?;
        })
    }
}

impl<P: Params, T> Node<P> for Option<T>
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(match self {
            Some(t) => t.accept(c, v)?,
            _ => {}
        })
    }
}

impl<P: Params, T> NodeMut<P> for Option<T>
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(match self {
            Some(t) => t.accept(c, v)?,
            _ => {}
        })
    }
}

impl<P: Params, K, V> Node<P> for BTreeMap<K, V>
where
    V: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for value in self.values() {
            value.accept(c, v)?;
        })
    }
}

impl<P: Params, K, V> NodeMut<P> for BTreeMap<K, V>
where
    V: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for value in self.values_mut() {
            value.accept(c, v)?;
        })
    }
}

impl<P: Params, T> Node<P> for RcOc<T>
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.as_ref().accept(c, v)
    }
}

impl<P: Params, T> NodeMut<P> for RcOc<T>
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(if let Some(x) = RcOc::get_mut(self) {
            x.accept(c, v)?;
        })
    }
}

impl<P: Params, T> Node<P> for std::rc::Rc<T>
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.as_ref().accept(c, v)
    }
}

impl<P: Params, T> NodeMut<P> for std::rc::Rc<T>
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(if let Some(x) = std::rc::Rc::get_mut(self) {
            x.accept(c, v)?;
        })
    }
}

impl<P: Params, T> Node<P> for Box<T>
where
    T: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.as_ref().accept(c, v)
    }
}

impl<P: Params, T> NodeMut<P> for Box<T>
where
    T: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.as_mut().accept(c, v)
    }
}

impl<P: Params, T1, T2> Node<P> for (T1, T2)
where
    T1: Node<P>,
    T2: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}

impl<P: Params, T1, T2, T3> Node<P> for (T1, T2, T3)
where
    T1: Node<P>,
    T2: Node<P>,
    T3: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}

impl<P: Params, T1, T2, T3, T4> Node<P> for (T1, T2, T3, T4)
where
    T1: Node<P>,
    T2: Node<P>,
    T3: Node<P>,
    T4: Node<P>,
{
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)
    }
}

impl<P: Params, T1, T2> NodeMut<P> for (T1, T2)
where
    T1: NodeMut<P>,
    T2: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}

impl<P: Params, T1, T2, T3> NodeMut<P> for (T1, T2, T3)
where
    T1: NodeMut<P>,
    T2: NodeMut<P>,
    T3: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}

impl<P: Params, T1, T2, T3, T4> NodeMut<P> for (T1, T2, T3, T4)
where
    T1: NodeMut<P>,
    T2: NodeMut<P>,
    T3: NodeMut<P>,
    T4: NodeMut<P>,
{
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)
    }
}

impl<P: Params> Node<P> for crate::LocalIdMap<P::Ex> {
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for (key, value) in self.0.iter() {
            key.accept(c, v)?;
            v.visit_ex(c, value)?;
        })
    }
}

/// `NodeMut` implementation doesn't visit keys,
/// mutating key requires re-constructing the underlaying map.
/// There will be extra perf cost even keys are not mutated.
/// Overriding its parent visit method can mutate keys economically.
impl<P: Params> NodeMut<P> for crate::LocalIdMap<P::Ex> {
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(for value in self.0.values_mut() {
            v.visit_ex(c, value)?
        })
    }
}
