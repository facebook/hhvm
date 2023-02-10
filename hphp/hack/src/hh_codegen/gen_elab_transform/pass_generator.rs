// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;

use super::contains_ocaml_attr;
use super::Context;

pub fn gen(ctx: &Context) -> TokenStream {
    let pass_methods: Vec<_> = ctx
        .type_structures()
        .flat_map(|s| gen_pass_methods(s, Body::Default))
        .collect();
    let passes_methods: Vec<_> = ctx
        .type_structures()
        .flat_map(|s| gen_pass_methods(s, Body::Passes))
        .collect();

    quote! {
        #![allow(unused_variables)]

        use std::ops::ControlFlow;
        use std::ops::ControlFlow::Continue;

        use oxidized::ast_defs::*;
        use oxidized::aast_defs::*;

        pub trait Pass {
            type Ctx: Clone;
            type Err;

            #(#pass_methods)*
        }

        pub struct Passes<Ctx, Err, P, Q>
        where
            Ctx: Clone,
            P: Pass<Ctx = Ctx, Err = Err>,
            Q: Pass<Ctx = Ctx, Err = Err>,
        {
            fst: P,
            snd: Q,
        }

        impl<Ctx, Err, P, Q> Pass for Passes<Ctx, Err, P, Q>
        where
            Ctx: Clone,
            P: Pass<Ctx = Ctx, Err = Err>,
            Q: Pass<Ctx = Ctx, Err = Err>,
        {
            type Ctx = Ctx;
            type Err = Err;

            #(#passes_methods)*
        }

        /// Used to combine multiple types implementing `Pass` into nested `Passes` types
        /// without requiring them to hand write it so :
        /// `passes![p1, p2, p3]` => `Passes(p1, Passes(p2, p3))`
        #[macro_export]
        macro_rules! passes {
            ( $p:expr $(,$ps:expr)+ $(,)? ) => {
                $crate::transform::Passes($p, $crate::passes!($($ps),*))
            };
            ( $p:expr $(,)? ) => {
                $p
            };
        }
    }
}

fn gen_pass_methods(s: synstructure::Structure<'_>, body_type: Body) -> TokenStream {
    // If the type is marked opaque, generate no methods; they won't be called.
    if contains_ocaml_attr(&s.ast().attrs, "transform.opaque") {
        return quote!();
    }
    let ty = &s.ast().ident;
    let name = super::gen_pass_method_name(ty.to_string());
    let (ty_params, ty_generics, _) = s.ast().generics.split_for_impl();
    let fld_methods = gen_fld_methods(&s, body_type);
    let body = body_type.gen(&name);
    let where_clause = ex_where_clause(&s.referenced_ty_params());
    quote! {
        #[inline(always)]
        fn #name #ty_params(
            &self,
            elem: &mut #ty #ty_generics,
            ctx: &mut Self::Ctx,
            errs: &mut Vec<Self::Err>,
        ) -> ControlFlow<(), ()> #where_clause {
            #body
        }

        #(#fld_methods)*
    }
}

#[derive(Copy, Clone, Debug)]
enum Body {
    Default,
    Passes,
}

impl Body {
    fn gen(self, name: &syn::Ident) -> TokenStream {
        match self {
            Body::Default => quote!(Continue(())),
            Body::Passes => quote! {
                self.fst.#name(elem, ctx, errs)?;
                self.snd.#name(elem, ctx, errs)
            },
        }
    }
}

fn gen_fld_methods(s: &synstructure::Structure<'_>, body_type: Body) -> Vec<TokenStream> {
    let ty_name = s.ast().ident.to_string();
    match &s.ast().data {
        syn::Data::Struct(data) => match &data.fields {
            syn::Fields::Named(..) => (s.variants().iter().flat_map(|v| v.bindings()))
                .filter(|field| contains_ocaml_attr(&field.ast().attrs, "transform.explicit"))
                .map(|bi| gen_fld_method(&ty_name, bi, body_type))
                .collect(),
            _ => vec![],
        },
        syn::Data::Enum(..) => (s.variants().iter())
            .filter(|variant| contains_ocaml_attr(variant.ast().attrs, "transform.explicit"))
            .map(|variant| gen_ctor_method(&ty_name, variant, body_type))
            .collect(),
        _ => vec![],
    }
}

fn gen_fld_method(
    ty_name: &str,
    binding_info: &synstructure::BindingInfo<'_>,
    body_type: Body,
) -> TokenStream {
    let ast = binding_info.ast();
    let name = super::gen_pass_fld_method_name(ty_name, ast.ident.as_ref().unwrap().to_string());
    let ty_params = binding_info.referenced_ty_params();
    let field_ty = &ast.ty;
    let body = body_type.gen(&name);
    let where_clause = ex_where_clause(&ty_params);
    quote! {
        #[allow(non_snake_case)]
        #[inline(always)]
        fn #name <#(#ty_params,)*>(
            &self,
            elem: &mut #field_ty,
            ctx: &mut Self::Ctx,
            errs: &mut Vec<Self::Err>,
        ) -> ControlFlow<(), ()> #where_clause {
            #body
        }
    }
}

fn gen_ctor_method(
    ty_name: &str,
    variant_info: &synstructure::VariantInfo<'_>,
    body_type: Body,
) -> TokenStream {
    let ast = variant_info.ast();
    let variant_ty = match &ast.fields {
        syn::Fields::Unnamed(fields) if fields.unnamed.len() == 1 => &fields.unnamed[0].ty,
        _ => panic!("transform.explicit only supports tuple-like variants with 1 field"),
    };
    let name = super::gen_pass_ctor_method_name(ty_name, ast.ident.to_string());
    let ty_params = variant_info.referenced_ty_params();
    let body = body_type.gen(&name);
    let where_clause = ex_where_clause(&ty_params);
    quote! {
        #[allow(non_snake_case)]
        #[inline(always)]
        fn #name <#(#ty_params,)*>(
            &self,
            elem: &mut #variant_ty,
            ctx: &mut Self::Ctx,
            errs: &mut Vec<Self::Err>,
        ) -> ControlFlow<(), ()> #where_clause {
            #body
        }
    }
}

fn ex_where_clause(referenced_ty_params: &[&syn::Ident]) -> TokenStream {
    if referenced_ty_params.iter().any(|tp| *tp == "Ex") {
        quote!(where Ex: Default)
    } else {
        quote!()
    }
}
