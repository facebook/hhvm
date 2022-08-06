// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::format_ident;
use quote::quote;

use super::Context;
use crate::common::gen_helpers;
use crate::common::to_snake;

pub fn gen(ctx: &Context) -> TokenStream {
    let uses = gen_helpers::gen_module_uses(ctx.modules());
    let visit_functions = ctx.types().map(gen_visit_function).collect::<Vec<_>>();

    quote! {
        #![allow(unused_imports)]
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
    let (_, ty_generics, _) = ast.generics.split_for_impl();
    quote! {
        fn #name(&mut self, p: &'a #ty #ty_generics) {
            p.recurse(self.object())
        }
    }
}

pub fn gen_visit_fn_name(ty: impl AsRef<str>) -> syn::Ident {
    format_ident!("visit_{}", to_snake(ty.as_ref()))
}
