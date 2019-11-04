// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{context::Context, gen_helper::*, generator::Generator, syn_helper::*};
use crate::{common::*, impl_generator};
use proc_macro2::TokenStream;
use quote::{format_ident, quote};

pub trait VisitorTrait {
    fn filename() -> String;
    fn trait_name() -> syn::Ident;
    fn node_ref_kind() -> TokenStream;
    fn use_node() -> TokenStream;

    fn gen(ctx: &Context) -> Result<TokenStream> {
        let associate_ty_decls = ctx
            .root_ty_params_with_context()
            .map(|ty| quote! {type #ty;});
        let associate_ty_bindings = ctx
            .root_ty_params_with_context()
            .map(|ty| quote! {#ty = Self::#ty,});

        let use_node = Self::use_node();
        let trait_name = Self::trait_name();
        let visit_functions = Self::gen_visit_functions(ctx)?;
        let visit_ty_params = Self::gen_visit_ty_params(ctx)?;

        // TODO: not hardcode `use crate::{aast::*, aast_defs::*};`
        Ok(quote! {
            #![allow(unused_variables)]

            use crate::{aast::*, aast_defs::*, ast_defs::*};
            #use_node

            pub trait #trait_name {

                #(#associate_ty_decls)*

                fn object(&mut self) -> &mut dyn #trait_name<#(#associate_ty_bindings)*>;

                #(#visit_ty_params)*

                #(#visit_functions)*
            }
        })
    }

    fn gen_visit_ty_params(ctx: &Context) -> Result<Vec<TokenStream>> {
        let ref_kind = Self::node_ref_kind();
        let context = ctx.visitor_context();
        Ok(ctx
            .root_ty_params_()
            .map(|ty| {
                let name = gen_visit_fn_name(ty.to_string());
                quote! {
                    fn #name(&mut self, c: &mut Self::#context, p: #ref_kind Self::#ty) {}
                }
            })
            .collect())
    }

    fn gen_visit_functions(ctx: &Context) -> Result<Vec<TokenStream>> {
        let ref_kind = Self::node_ref_kind();
        let context = ctx.visitor_context();
        let mut visit_fn = vec![];
        for ty in ctx.types.iter() {
            let def = ctx
                .defs
                .get(ty)
                .ok_or_else(|| format!("Type {} not found", ty))?;
            let ty_params = get_ty_param_idents(def)?;
            let ty_args = gen_ty_params_with_self(ty_params);
            let name = gen_visit_fn_name(ty);
            let ty = format_ident!("{}", ty);
            visit_fn.push(quote! {
                fn #name(&mut self, c: &mut Self::#context, p: #ref_kind #ty#ty_args) {
                    p.recurse(c, self.object())
                }
            });
        }
        Ok(visit_fn)
    }
}

pub fn gen_visit_fn_name(ty: impl AsRef<str>) -> syn::Ident {
    format_ident!("visit_{}", to_snake(ty.as_ref()))
}

pub struct RefVisitorTrait;
impl VisitorTrait for RefVisitorTrait {
    fn filename() -> String {
        "visitor.rs".into()
    }

    fn trait_name() -> syn::Ident {
        format_ident!("Visitor")
    }

    fn node_ref_kind() -> TokenStream {
        quote! { & }
    }

    fn use_node() -> TokenStream {
        quote! { use super::node::Node; }
    }
}
impl_generator!(RefVisitorTrait, VisitorTrait);

pub struct MutVisitorTrait;
impl VisitorTrait for MutVisitorTrait {
    fn filename() -> String {
        "visitor_mut.rs".into()
    }

    fn trait_name() -> syn::Ident {
        format_ident!("VisitorMut")
    }

    fn node_ref_kind() -> TokenStream {
        quote! { &mut }
    }

    fn use_node() -> TokenStream {
        quote! { use super::node_mut::NodeMut; }
    }
}
impl_generator!(MutVisitorTrait, VisitorTrait);
