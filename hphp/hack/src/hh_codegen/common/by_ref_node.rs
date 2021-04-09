// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;

pub fn node() -> TokenStream {
    quote! {
        use super::Visitor;

        pub trait Node<'a> {
            fn accept(&'a self, v: &mut dyn Visitor<'a>) {
                self.recurse(v)
            }
            fn recurse(&'a self, _v: &mut dyn Visitor<'a>) {}
        }
    }
}

pub fn node_impl() -> TokenStream {
    quote! {
        use super::{node::Node, visitor::Visitor};

        impl<'a> Node<'a> for () {}
        impl<'a> Node<'a> for bool {}
        impl<'a> Node<'a> for isize {}
        impl<'a> Node<'a> for str {}
        impl<'a> Node<'a> for bstr::BStr {}
        impl<'a> Node<'a> for crate::file_info::Mode {}
        impl<'a> Node<'a> for crate::local_id::LocalId<'a> {}
        impl<'a> Node<'a> for crate::method_flags::MethodFlags {}
        impl<'a> Node<'a> for crate::nast::FuncBodyAnn<'a> {}
        impl<'a> Node<'a> for crate::pos::Pos<'a> {}
        impl<'a> Node<'a> for crate::prop_flags::PropFlags {}
        impl<'a> Node<'a> for crate::tany_sentinel::TanySentinel {}
        impl<'a> Node<'a> for crate::typing_defs_flags::FunParamFlags {}
        impl<'a> Node<'a> for crate::typing_defs_flags::FunTypeFlags {}

        impl<'a, T: Node<'a> + ?Sized> Node<'a> for &'a T {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                (*self).accept(v)
            }
        }

        impl<'a, T: Node<'a>> Node<'a> for [T] {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                for x in self {
                    x.accept(v)
                }
            }
        }

        impl<'a, T: Node<'a>> Node<'a> for Option<T> {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                match self {
                    Some(t) => t.accept(v),
                    _ => {}
                }
            }
        }

        impl<'a, T: Node<'a>> Node<'a> for arena_collections::List<'a, T> {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                for elt in self.iter() {
                    elt.accept(v);
                }
            }
        }

        impl<'a, K: Node<'a>, V: Node<'a>> Node<'a> for arena_collections::map::Map<'a, K, V> {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                for (key, value) in self.iter() {
                    key.accept(v);
                    value.accept(v);
                }
            }
        }

        impl<'a, K: Node<'a>, V: Node<'a>> Node<'a> for arena_collections::SortedAssocList<'a, K, V> {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                for (key, value) in self.iter() {
                    key.accept(v);
                    value.accept(v);
                }
            }
        }

        impl<'a, T: Node<'a>> Node<'a> for arena_collections::SortedSet<'a, T> {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                for elt in self.iter() {
                    elt.accept(v);
                }
            }
        }

        impl<'a, T1, T2> Node<'a> for (T1, T2)
        where
            T1: Node<'a>,
            T2: Node<'a>,
        {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                self.0.accept(v);
                self.1.accept(v);
            }
        }

        impl<'a, T1, T2, T3> Node<'a> for (T1, T2, T3)
        where
            T1: Node<'a>,
            T2: Node<'a>,
            T3: Node<'a>,
        {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                self.0.accept(v);
                self.1.accept(v);
                self.2.accept(v);
            }
        }

        impl<'a, T1, T2, T3, T4> Node<'a> for (T1, T2, T3, T4)
        where
            T1: Node<'a>,
            T2: Node<'a>,
            T3: Node<'a>,
            T4: Node<'a>,
        {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                self.0.accept(v);
                self.1.accept(v);
                self.2.accept(v);
                self.3.accept(v);
            }
        }

        impl<'a, T1, T2, T3, T4, T5> Node<'a> for (T1, T2, T3, T4, T5)
        where
            T1: Node<'a>,
            T2: Node<'a>,
            T3: Node<'a>,
            T4: Node<'a>,
            T5: Node<'a>,
        {
            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                self.0.accept(v);
                self.1.accept(v);
                self.2.accept(v);
                self.3.accept(v);
                self.4.accept(v);
            }
        }
    }
}
