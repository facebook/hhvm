// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::anyhow;
use anyhow::Result;
use proc_macro2::Ident;
use proc_macro2::TokenStream;
use quote::format_ident;
use quote::quote;

use super::context::Context;
use super::gen_helper::*;
use super::generator::Generator;
use super::syn_helper::*;
use super::visitor_trait_generator;
use crate::impl_generator;

pub trait NodeImpl {
    fn filename() -> String;
    fn node_trait_name() -> syn::Ident;
    fn self_ref_kind(lifetime: Option<&Ident>) -> TokenStream;
    fn visitor_trait_name() -> syn::Ident;
    fn use_node() -> TokenStream;
    fn use_visitor() -> TokenStream;

    fn gen(ctx: &Context<'_>) -> Result<TokenStream> {
        let impls = ctx
            .non_alias_types()
            .map(|ty| {
                let ty = ty.as_ref();
                let item = ctx
                    .defs
                    .get(ty)
                    .ok_or_else(|| anyhow!("Type {} not found", ty))?;
                Self::gen_node_impl(ctx, ty, item)
            })
            .collect::<Result<Vec<_>>>()?;
        let use_node = Self::use_node();
        let use_visitor = Self::use_visitor();
        let uses = gen_module_uses(ctx.modules());
        Ok(quote! {
            #![allow(unused_imports)]
            #![allow(unused_variables)]

            #use_node
            #use_visitor
            #uses
            use super::type_params::Params;

            #(#impls)*
        })
    }

    fn gen_node_impl(ctx: &Context<'_>, ty_name: &str, ty_def: &syn::Item) -> Result<TokenStream> {
        let recurse_body = Self::gen_recurse_body(ctx, ty_name, ty_def)?;
        let visit_fn = visitor_trait_generator::gen_visit_fn_name(ty_name);
        let ty_name = format_ident!("{}", ty_name);
        let ty_params = gen_ty_params(get_ty_param_idents(ty_def)?);
        let node_trait_name = Self::node_trait_name();
        let node_lifetime = ctx.node_lifetime_ident();
        let self_ref_kind = Self::self_ref_kind(Some(&node_lifetime));
        let node_lifetime = make_lifetime(&node_lifetime);
        let visitor_trait_name = Self::visitor_trait_name();
        let context = ctx.context_ident();
        let error = ctx.error_ident();

        Ok(quote! {
            impl<P: Params> #node_trait_name<P> for #ty_name #ty_params {
                fn accept<#node_lifetime>(
                    #self_ref_kind self,
                    c: &mut P::#context,
                    v: &mut dyn #visitor_trait_name<#node_lifetime, Params = P>
                ) -> Result<(), P::#error> {
                    v.#visit_fn(c, self)
                }

                fn recurse<#node_lifetime>(
                    #self_ref_kind self,
                    c: &mut P::#context,
                    v: &mut dyn #visitor_trait_name<#node_lifetime, Params = P>
                ) -> Result<(), P::#error> {
                    #recurse_body
                }
            }
        })
    }

    fn try_simple_ty_param(ctx: &Context<'_>, ty: &syn::Type) -> Option<String> {
        try_simple_type(ty).filter(|t| ctx.is_root_ty_param(t))
    }

    fn try_gen_simple_ty_param_visit_call(
        ctx: &Context<'_>,
        ty: &syn::Type,
        last: bool,
        accessor: TokenStream,
    ) -> Option<TokenStream> {
        let ref_kind = Self::self_ref_kind(None);
        try_simple_type(ty)
            .filter(|t| ctx.is_root_ty_param(t))
            .map(|ty| {
                let fn_name = visitor_trait_generator::gen_visit_fn_name(ty);
                if !last {
                    quote! {v.#fn_name( c, #ref_kind #accessor )?;}
                } else {
                    quote! {v.#fn_name( c, #ref_kind #accessor )}
                }
            })
    }

    fn gen_recurse_body(ctx: &Context<'_>, ty_name: &str, ty: &syn::Item) -> Result<TokenStream> {
        use syn::Item::*;
        use syn::*;
        match ty {
            Struct(ItemStruct { fields, .. }) => Self::gen_recurse_struct_body(ctx, fields),
            Enum(ItemEnum { variants, .. }) => Self::gen_recurse_enum_body(ctx, ty_name, variants),
            _ => Ok(quote! {}),
        }
    }

    fn gen_recurse_struct_body(ctx: &Context<'_>, fields: &syn::Fields) -> Result<TokenStream> {
        use syn::*;
        match fields {
            Fields::Named(fields) => {
                let last_field = fields.named.len() - 1;
                let fields = get_field_and_type_from_named(fields);
                let calls = fields.iter().enumerate().map(|(i, (name, ty))| {
                    let accessor = format_ident!("{}", name);
                    Self::try_gen_simple_ty_param_visit_call(
                        ctx,
                        ty,
                        i == last_field,
                        quote! { self.#accessor },
                    )
                    .unwrap_or_else(|| {
                        if i != last_field {
                            quote! { self.#accessor.accept(c, v)?; }
                        } else {
                            quote! { self.#accessor.accept(c, v) }
                        }
                    })
                });
                Ok(quote! { #(#calls)* })
            }
            Fields::Unnamed(fields) => {
                let last_field = fields.unnamed.len() - 1;
                let fields = get_field_and_type_from_unnamed(fields);
                let calls = fields.map(|(i, ty)| {
                    let accessor = Index::from(i);
                    Self::try_gen_simple_ty_param_visit_call(
                        ctx,
                        ty,
                        i == last_field,
                        quote! { self.#accessor },
                    )
                    .unwrap_or_else(|| {
                        if i != last_field {
                            quote! {self.#accessor.accept(c, v)?; }
                        } else {
                            quote! {self.#accessor.accept(c, v) }
                        }
                    })
                });
                Ok(quote! { #(#calls)* })
            }
            Fields::Unit => Ok(quote! {}),
        }
    }

    fn gen_recurse_enum_body(
        ctx: &Context<'_>,
        ty_name: &str,
        variants: &syn::punctuated::Punctuated<syn::Variant, syn::token::Comma>,
    ) -> Result<TokenStream> {
        use syn::*;
        let node_lifetime = ctx.node_lifetime_ident();
        let self_ref_kind = Self::self_ref_kind(Some(&node_lifetime));
        let node_lifetime = make_lifetime(&node_lifetime);
        let mut arms: Vec<TokenStream> = Vec::new();
        let mut helpers: Vec<TokenStream> = Vec::new();
        for Variant {
            ident: variant_name,
            fields,
            ..
        } in variants.iter()
        {
            let ty_name = format_ident!("{}", ty_name);
            match fields {
                Fields::Named(_fields) => {
                    return Err(anyhow!(
                        "Enum with named fields not supported yet. Enum {:?}",
                        ty_name
                    ));
                }
                Fields::Unnamed(fields) => {
                    let mut pattern = vec![];
                    let mut calls = vec![];
                    if let Some((len, tys)) = try_get_types_from_box_tuple(fields) {
                        let v = format_ident!("a");
                        let helper_name = format_ident!("helper{}", helpers.len());
                        pattern.push(quote! {#v});
                        let mut arm_calls: Vec<TokenStream> = Vec::new();
                        let mut arm_tys: Vec<&syn::Type> = Vec::new();
                        for (i, ty) in tys {
                            let accessor = &Index::from(i);
                            let call = Self::try_gen_simple_ty_param_visit_call(
                                ctx,
                                ty,
                                i == len - 1,
                                quote! { #v.#accessor },
                            )
                            .unwrap_or_else(|| {
                                if i != len - 1 {
                                    quote! { #v.#accessor.accept(c, v)?; }
                                } else {
                                    quote! { #v.#accessor.accept(c, v) }
                                }
                            });
                            arm_calls.push(call);
                            arm_tys.push(ty);
                        }
                        if len > 1 {
                            // Emit a helper since the match has multiple accept() calls.
                            // This [inline] attribute doesn't affect mode/dbg but
                            // reduces stack size and improves perf at higher opt levels.
                            let visitor_trait_name = Self::visitor_trait_name();
                            calls.push(quote! { #helper_name(#v, c, v) });
                            helpers.push(quote! {
                                #[inline]
                                fn #helper_name<
                                    #node_lifetime,
                                    P: Params + Params<Ex=Ex> + Params<En=En>,
                                    Ex,
                                    En
                                >(
                                    a: #self_ref_kind Box<(#(#arm_tys,)*)>,
                                    c: &mut P::Context,
                                    v: &mut dyn #visitor_trait_name<'node, Params = P>,
                                ) -> Result<(), P::Error> {
                                    #(#arm_calls)*
                                }
                            });
                        } else {
                            calls.extend(arm_calls);
                        }
                    } else {
                        let last_field = fields.unnamed.len() - 1;
                        for (i, ty) in get_field_and_type_from_unnamed(fields) {
                            let v = format_ident!("a{}", i);
                            pattern.push(quote! {#v,});
                            calls.push(
                                Self::try_gen_simple_ty_param_visit_call(
                                    ctx,
                                    ty,
                                    i == last_field,
                                    quote! { #v },
                                )
                                .unwrap_or_else(|| {
                                    if i != last_field {
                                        quote! { #v.accept(c, v)?; }
                                    } else {
                                        quote! { #v.accept(c, v) }
                                    }
                                }),
                            );
                        }
                    }
                    arms.push(quote! {
                        #ty_name::#variant_name(#(# pattern)*) => {
                            #(#calls)*
                        }
                    });
                }
                Fields::Unit => arms.push(quote! {
                    #ty_name::#variant_name => {Ok(())}
                }),
            }
        }
        match arms.as_slice() {
            [] => Ok(quote! {}),
            _ => Ok(quote! {
                #(#helpers)*
                match self {
                    #(#arms)*
                }
            }),
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
    fn self_ref_kind(lifetime: Option<&Ident>) -> TokenStream {
        match lifetime {
            Some(l) => {
                let l = make_lifetime(l);
                quote! {&#l}
            }
            None => quote! {&},
        }
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
    fn self_ref_kind(lifetime: Option<&Ident>) -> TokenStream {
        match lifetime {
            Some(l) => {
                let l = make_lifetime(l);
                quote! {&#l mut}
            }
            None => quote! {&mut},
        }
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
