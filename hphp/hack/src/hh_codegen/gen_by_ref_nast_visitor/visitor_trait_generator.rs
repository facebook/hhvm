// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::{format_ident, quote};

use crate::common::{gen_helpers, to_snake};

use super::Context;

pub fn gen(ctx: &Context) -> TokenStream {
    let uses = gen_helpers::gen_module_uses(ctx.modules());
    let visit_functions = ctx.types().map(gen_visit_function).collect::<Vec<_>>();

    quote! {
        #![allow(unused_variables)]

        #uses
        use super::node::Node;

        pub trait Visitor<'a> {
            fn object(&mut self) -> &mut dyn Visitor<'a>;

            #(#visit_functions)*
        }
    }
}

fn gen_visit_function(ast: &syn::DeriveInput) -> TokenStream {
    let ty = &ast.ident;
    let name = gen_visit_fn_name(ty.to_string());
    let type_args = super::rewrite_type_args(&ast.generics);
    quote! {
        fn #name(&mut self, p: &'a #ty #type_args) {
            p.recurse(self.object())
        }
    }
}

pub fn gen_visit_fn_name(ty: impl AsRef<str>) -> syn::Ident {
    format_ident!("visit_{}", to_snake(ty.as_ref()))
}
