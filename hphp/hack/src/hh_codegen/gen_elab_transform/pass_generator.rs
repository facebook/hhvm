// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;

use super::contains_ocaml_attr;
use super::Context;
use super::Direction;

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
        #![allow(unused_variables, non_snake_case)]

        use std::ops::ControlFlow;
        use std::ops::ControlFlow::Continue;

        use oxidized::ast_defs::*;
        use oxidized::aast_defs::*;

        use crate::env::Env;

        type Ex = ();
        type En = ();

        pub trait Pass : PassClone {
            #(#pass_methods)*
        }

        pub trait PassClone {
            fn clone_box(&self) -> Box<dyn Pass>;
        }
        impl<T> PassClone for T
        where
            T: 'static + Pass + Clone,
        {
            fn clone_box(&self) -> Box<dyn Pass> {
                Box::new(self.clone())
            }
        }
        impl Clone for Box<dyn Pass> {
            fn clone(&self) -> Box<dyn Pass> {
                self.clone_box()
            }
        }

        pub struct Passes {
            pub passes: Vec<Box<dyn Pass>>,
        }
        impl Clone for Passes {
            fn clone(&self) -> Self {
                Self { passes: self.passes.clone() }
            }
        }
        impl Pass for Passes {
            #(#passes_methods)*
        }
    }
}

fn gen_pass_methods(s: synstructure::Structure<'_>, body_type: Body) -> TokenStream {
    // If the type is marked opaque, generate no methods; they won't be called.
    if contains_ocaml_attr(&s.ast().attrs, "transform.opaque") {
        return quote!();
    }
    let ty = &s.ast().ident;
    let name_td = super::gen_pass_method_name(ty.to_string(), Direction::TopDown);
    let name_bu = super::gen_pass_method_name(ty.to_string(), Direction::BottomUp);
    let (_, ty_generics, _) = s.ast().generics.split_for_impl();
    let fld_methods = gen_fld_methods(&s, body_type);
    let body_td = body_type.gen(&name_td);
    let body_bu = body_type.gen(&name_bu);
    quote! {
        #[inline(always)]
        fn #name_td(
            &mut self,
            env: &Env,
            elem: &mut #ty #ty_generics,
        ) -> ControlFlow<()> {
            #body_td
        }

        #[inline(always)]
        fn #name_bu(
            &mut self,
            env: &Env,
            elem: &mut #ty #ty_generics,
        ) -> ControlFlow<()> {
            #body_bu
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
                for pass in &mut self.passes {
                    pass.#name(env, elem)?;
                }
                Continue(())
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
    let name_td = super::gen_pass_fld_method_name(
        ty_name,
        ast.ident.as_ref().unwrap().to_string(),
        Direction::TopDown,
    );
    let name_bu = super::gen_pass_fld_method_name(
        ty_name,
        ast.ident.as_ref().unwrap().to_string(),
        Direction::BottomUp,
    );
    let field_ty = &ast.ty;
    let body_td = body_type.gen(&name_td);
    let body_bu = body_type.gen(&name_bu);
    quote! {
        #[inline(always)]
        fn #name_td(
            &mut self,
            env: &Env,
            elem: &mut #field_ty,
        ) -> ControlFlow<()> {
            #body_td
        }

        #[inline(always)]
        fn #name_bu(
            &mut self,
            env: &Env,
            elem: &mut #field_ty,
        ) -> ControlFlow<()> {
            #body_bu
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
    let name_td =
        super::gen_pass_ctor_method_name(ty_name, ast.ident.to_string(), Direction::TopDown);
    let name_bu =
        super::gen_pass_ctor_method_name(ty_name, ast.ident.to_string(), Direction::BottomUp);
    let body_td = body_type.gen(&name_td);
    let body_bu = body_type.gen(&name_bu);
    quote! {
        #[inline(always)]
        fn #name_td(
            &mut self,
            env: &Env,
            elem: &mut #variant_ty,
        ) -> ControlFlow<()> {
            #body_td
        }

        #[inline(always)]
        fn #name_bu(
            &mut self,
            env: &Env,
            elem: &mut #variant_ty,
        ) -> ControlFlow<()> {
            #body_bu
        }
    }
}
