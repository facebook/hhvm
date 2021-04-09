// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;

use crate::common::gen_helpers;

use super::visitor_trait_generator;
use super::Context;

pub fn gen(ctx: &Context) -> TokenStream {
    let uses = gen_helpers::gen_module_uses(ctx.modules());
    let impls = ctx.type_structures().map(gen_node_impl).collect::<Vec<_>>();

    quote! {
        #![allow(unused_variables)]
        #![allow(unused_braces)]

        use super::node::Node;
        use super::visitor::Visitor;
        #uses

        #(#impls)*
    }
}

fn gen_node_impl(s: synstructure::Structure<'_>) -> TokenStream {
    let ty_name = &s.ast().ident;
    let type_args = super::rewrite_type_args(&s.ast().generics);
    let visit_fn = visitor_trait_generator::gen_visit_fn_name(ty_name.to_string());
    let recurse_body = s.each(|bi| quote! { #bi.accept(v) });

    // Sanity check: ensure that all types have at most one lifetime with name `'a`.
    let lifetimes = s.ast().generics.lifetimes().collect::<Vec<_>>();
    assert!(lifetimes.len() <= 1);
    if let Some(lifetime) = lifetimes.first() {
        assert_eq!("a", lifetime.lifetime.ident.to_string());
    }

    quote! {
        impl<'a> Node<'a> for #ty_name #type_args {
            fn accept(&'a self, v: &mut dyn Visitor<'a>) {
                v.#visit_fn(self)
            }

            fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
                match self { #recurse_body }
            }
        }
    }
}
