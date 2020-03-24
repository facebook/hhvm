// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{
    context::Context, gen_helper::*, generator::Generator, syn_helper::*, visitor_trait_generator,
};
use crate::{common::*, impl_generator};
use proc_macro2::TokenStream;
use quote::{format_ident, quote};

pub trait NodeImpl {
    fn filename() -> String;
    fn node_trait_name() -> syn::Ident;
    fn self_ref_kind() -> TokenStream;
    fn visitor_trait_name() -> syn::Ident;
    fn use_node() -> TokenStream;
    fn use_visitor() -> TokenStream;

    fn gen(ctx: &Context) -> Result<TokenStream> {
        let impls = ctx
            .non_alias_types()
            .map(|ty| {
                let ty = ty.as_ref();
                let item = ctx
                    .defs
                    .get(ty)
                    .ok_or_else(|| format!("Type {} not found", ty))?;
                Self::gen_node_impl(ctx, ty, item)
            })
            .collect::<Result<Vec<_>>>()?;
        let use_node = Self::use_node();
        let use_visitor = Self::use_visitor();
        let uses = gen_module_uses(ctx.modules());
        Ok(quote! {
            #![allow(unused_variables)]

            #use_node
            #use_visitor
            #uses
            use super::type_params::Params;

            #(#impls)*
        })
    }

    fn gen_node_impl(ctx: &Context, ty_name: &str, ty_def: &syn::Item) -> Result<TokenStream> {
        let recurse_body = Self::gen_recurse_body(ctx, ty_name, ty_def)?;
        let visit_fn = visitor_trait_generator::gen_visit_fn_name(ty_name);
        let ty_name = format_ident!("{}", ty_name);
        let ty_params = gen_ty_params(get_ty_param_idents(ty_def)?);
        let node_trait_name = Self::node_trait_name();
        let self_ref_kind = Self::self_ref_kind();
        let visitor_trait_name = Self::visitor_trait_name();
        let context = ctx.context_ident();
        let error = ctx.error_ident();
        Ok(quote! {
            impl<P: Params> #node_trait_name<P> for #ty_name#ty_params {
                fn accept(
                    #self_ref_kind self,
                    c: &mut P::#context,
                    v: &mut dyn #visitor_trait_name<P = P>
                ) -> Result<(), P::#error> {
                    v.#visit_fn(c, self)
                }

                fn recurse(
                    #self_ref_kind self,
                    c: &mut P::#context,
                    v: &mut dyn #visitor_trait_name<P = P>
                ) -> Result<(), P::#error> {
                    #recurse_body
                }
            }
        })
    }

    fn try_simple_ty_param(ctx: &Context, ty: &syn::Type) -> Option<String> {
        try_simple_type(ty).filter(|t| ctx.is_root_ty_param(t))
    }

    fn try_gen_simple_ty_param_visit_call(
        ctx: &Context,
        ty: &syn::Type,
        accessor: TokenStream,
    ) -> Option<TokenStream> {
        let ref_kind = Self::self_ref_kind();
        try_simple_type(ty)
            .filter(|t| ctx.is_root_ty_param(t))
            .map(|ty| {
                let fn_name = visitor_trait_generator::gen_visit_fn_name(ty);
                quote! {v.#fn_name( c, #ref_kind #accessor )?;}
            })
    }

    fn gen_recurse_body(ctx: &Context, ty_name: &str, ty: &syn::Item) -> Result<TokenStream> {
        use syn::{Item::*, *};
        match ty {
            Struct(ItemStruct { fields, .. }) => match fields {
                Fields::Named(fields) => {
                    let fields = get_field_and_type_from_named(fields);
                    let calls = fields.iter().map(|(name, ty)| {
                        let accessor = format_ident!("{}", name);
                        Self::try_gen_simple_ty_param_visit_call(ctx, ty, quote! { self.#accessor})
                            .unwrap_or(quote! {self.#accessor.accept(c, v)?;})
                    });
                    Ok(quote! {
                        #(#calls)*
                        Ok(())
                    })
                }
                Fields::Unnamed(fields) => {
                    let fields = get_field_and_type_from_unnamed(fields);
                    let calls = fields.map(|(i, ty)| {
                        let accessor = Index::from(i);
                        Self::try_gen_simple_ty_param_visit_call(ctx, ty, quote! { self.#accessor})
                            .unwrap_or(quote! {self.#accessor.accept(c, v)?;})
                    });
                    Ok(quote! {
                        #(#calls)*
                        Ok(())
                    })
                }
                Fields::Unit => Ok(quote! {}),
            },
            Enum(ItemEnum { variants, .. }) => {
                let mut arms: Vec<TokenStream> = vec![];
                for Variant {
                    ident: variant_name,
                    fields,
                    ..
                } in variants.iter()
                {
                    let ty_name = format_ident!("{}", ty_name);
                    match fields {
                        Fields::Named(_fields) => {
                            return Err(format!(
                                "Enum with named fields not supported yet. Enum {:?}",
                                ty_name
                            )
                            .into());
                        }
                        Fields::Unnamed(fields) => {
                            let mut pattern = vec![];
                            let mut calls = vec![];
                            if let Some(tys) = try_get_types_from_box_tuple(fields) {
                                let v = format_ident!("a");
                                pattern.push(quote! {#v});
                                calls.extend(tys.map(|(i, ty)| {
                                    let accessor = &Index::from(i);
                                    Self::try_gen_simple_ty_param_visit_call(
                                        ctx,
                                        ty,
                                        quote! { #v.#accessor },
                                    )
                                    .unwrap_or(quote! {#v.#accessor.accept(c, v)?;})
                                }));
                            } else {
                                let fields =
                                    get_field_and_type_from_unnamed(fields).collect::<Vec<_>>();
                                for (i, ty) in fields.iter() {
                                    let v = format_ident!("a{}", *i);
                                    pattern.push(quote! {#v,});
                                    calls.push(
                                        Self::try_gen_simple_ty_param_visit_call(
                                            ctx,
                                            ty,
                                            quote! { #v },
                                        )
                                        .unwrap_or(quote! { #v.accept(c, v)?; }),
                                    );
                                }
                            }
                            arms.push(quote! {
                                #ty_name::#variant_name(#(# pattern)*) => {
                                    #(#calls)*
                                    Ok(())
                                }
                            });
                        }
                        Fields::Unit => arms.push(quote! {
                            #ty_name::#variant_name => { {Ok(())} }
                        }),
                    }
                }
                match arms.as_slice() {
                    [] => Ok(quote! {}),
                    _ => Ok(quote! {
                        match self {
                            #(#arms)*
                        }
                    }),
                }
            }
            _ => Ok(quote! {}),
        }
    }
}

pub struct RefNodeImpl;
impl NodeImpl for RefNodeImpl {
    fn filename() -> String {
        "node_impl_gen.rs".into()
    }
    fn node_trait_name() -> syn::Ident {
        format_ident!("Node")
    }
    fn self_ref_kind() -> TokenStream {
        quote! {&}
    }
    fn visitor_trait_name() -> syn::Ident {
        format_ident!("Visitor")
    }
    fn use_node() -> TokenStream {
        quote! { use super::node::Node; }
    }
    fn use_visitor() -> TokenStream {
        quote! { use super::visitor::Visitor; }
    }
}
impl_generator!(RefNodeImpl, NodeImpl);

pub struct MutNodeImpl;
impl NodeImpl for MutNodeImpl {
    fn filename() -> String {
        "node_mut_impl_gen.rs".into()
    }
    fn node_trait_name() -> syn::Ident {
        format_ident!("NodeMut")
    }
    fn self_ref_kind() -> TokenStream {
        quote! {&mut}
    }
    fn visitor_trait_name() -> syn::Ident {
        format_ident!("VisitorMut")
    }
    fn use_node() -> TokenStream {
        quote! { use super::node_mut::NodeMut; }
    }
    fn use_visitor() -> TokenStream {
        quote! { use super::visitor_mut::VisitorMut; }
    }
}
impl_generator!(MutNodeImpl, NodeImpl);
