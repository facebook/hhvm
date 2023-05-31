// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(box_patterns)]

use itertools::Itertools;
use proc_macro2::Literal;
use proc_macro2::TokenStream;
use proc_macro_error::abort;
use proc_macro_error::proc_macro_error;
use proc_macro_error::ResultExt;
use quote::quote;
use syn::parenthesized;
use syn::parse::Parse;
use syn::parse::ParseStream;
use syn::parse_macro_input;
use syn::punctuated::Punctuated;
use syn::token;
use syn::Attribute;
use syn::DeriveInput;
use syn::Ident;
use syn::Result;
use syn::Token;

/// This macro is used to derive the TextualDecl trait for builtins.
///
/// When applied to an enum expects that each variant has an attribute in the form:
///   #[decl(fn name(ty1, ty2) -> ty)]
///
/// and it generates an impl containing a write_decls() method:
///   impl ... {
///     fn write_decls(w: &mut TextualFile<'_>) -> Result<()> { ... }
///   }
///
/// It also implements the Display trait for the enum.
///
#[proc_macro_error]
#[proc_macro_derive(TextualDecl, attributes(decl, function))]
pub fn textual_decl_derive(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    let name = &input.ident;
    let vis = &input.vis;
    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let mut last = None;
    let mut decls: Vec<TokenStream> = Vec::new();
    let mut displays: Vec<TokenStream> = Vec::new();
    match input.data {
        syn::Data::Enum(e) => {
            for variant in e.variants {
                let variant_name = &variant.ident;
                let attr = extract_exactly_one_attr(&variant.ident, variant.attrs, "decl");
                let decl: Decl = syn::parse2(attr.tokens).unwrap_or_abort();

                // Enforce sorted variant order.
                let variant_name_str = format!("{variant_name}");
                if let Some(last_str) = last {
                    if last_str > variant_name_str {
                        abort!(
                            variant_name,
                            format!(
                                "Variants are out of order - '{last_str}' must be after '{variant_name_str}'"
                            )
                        );
                    }
                }
                last = Some(variant_name_str);

                match decl {
                    Decl::FnSignature(signature) => {
                        let builtin_name =
                            Literal::string(&format!("$builtins.{}", signature.ident));
                        let params = signature
                            .parameters
                            .iter()
                            .map(|p| {
                                let (ty, owned) = p.ty.tokenize();
                                if owned { quote!(#ty) } else { ty }
                            })
                            .collect_vec();
                        let ret = {
                            let (ty, owned) = signature.ret.tokenize();
                            if owned { quote!(#ty) } else { ty }
                        };

                        let fn_name = if name == "Hhbc" {
                            quote!(crate::mangle::FunctionName::Builtin(Builtin::Hhbc(Hhbc::#variant_name)))
                        } else {
                            quote!(crate::mangle::FunctionName::Builtin(#name::#variant_name))
                        };

                        let decl = quote! {
                            if subset.contains(&#name::#variant_name) {
                                txf.declare_function(&#fn_name, &textual::FuncAttributes::default(), &[#(#params),*], &#ret)?;
                            }
                        };
                        decls.push(decl);

                        let display = quote!(#name::#variant_name => #builtin_name,);
                        displays.push(display);
                    }
                    Decl::Skip => {
                        displays.push(quote!(#name::#variant_name(f) => f.as_str(),));
                    }
                }
            }
        }
        syn::Data::Struct(s) => abort!(s.struct_token, "TextualDecl does not support 'struct'"),
        syn::Data::Union(u) => abort!(u.union_token, "TextualDecl does not support 'union'"),
    }

    let output = quote! {
        impl #impl_generics #name #ty_generics #where_clause {
            #vis fn write_decls(txf: &mut TextualFile<'_>, subset: &HashSet<#name #ty_generics>) -> Result<()> {
                #(#decls)*
                Ok(())
            }

            #vis fn as_str(&self) -> &'static str {
                match self {
                    #(#displays)*
                }
            }
        }
    };

    output.into()
}

fn extract_exactly_one_attr(ident: &Ident, attrs: Vec<Attribute>, name: &str) -> Attribute {
    let mut attrs = attrs.into_iter().filter(|attr| attr.path.is_ident(name));

    let attr = if let Some(attr) = attrs.next() {
        attr
    } else {
        abort!(ident, "variant is missing 'decl' attribute");
    };

    if let Some(next) = attrs.next() {
        abort!(
            next.path,
            "'decl' attribute may not be specified multiple times"
        );
    }

    attr
}

enum Decl {
    FnSignature(DeclSig),
    Skip,
}

impl Parse for Decl {
    fn parse(wrapped_input: ParseStream<'_>) -> Result<Self> {
        let input;
        parenthesized!(input in wrapped_input);

        if input.peek(Token![fn]) {
            let args;
            Ok(Decl::FnSignature(DeclSig {
                fn_token: input.parse()?,
                ident: input.parse()?,
                paren_token: parenthesized!(args in input),
                parameters: args.parse_terminated(DeclArg::parse)?,
                arrow: input.parse()?,
                ret: input.parse()?,
            }))
        } else {
            let tag: Ident = input.parse()?;
            if tag == "skip" {
                if !input.is_empty() {
                    return Err(input.error("'skip' may not be followed by additional tokens"));
                }
                Ok(Decl::Skip)
            } else {
                abort!(tag, "Unknown 'decl' type");
            }
        }
    }
}

struct DeclSig {
    #[allow(dead_code)]
    fn_token: token::Fn,
    ident: Ident,
    #[allow(dead_code)]
    paren_token: token::Paren,
    parameters: Punctuated<DeclArg, token::Comma>,
    #[allow(dead_code)]
    arrow: token::RArrow,
    ret: DeclTy,
}

struct DeclArg {
    #[allow(dead_code)]
    name: Option<(Ident, token::Colon)>,
    ty: DeclTy,
}

impl Parse for DeclArg {
    fn parse(input: ParseStream<'_>) -> Result<Self> {
        let name = if input.peek2(Token![:]) {
            let name: Ident = input.parse()?;
            let colon: token::Colon = input.parse()?;
            Some((name, colon))
        } else {
            None
        };

        let ty = input.parse()?;

        Ok(DeclArg { name, ty })
    }
}

#[derive(Debug)]
enum DeclTy {
    Ellipsis(token::Dot3),
    Float(Ident),
    Int(Ident),
    Noreturn(Ident),
    Ptr(token::Star, Box<DeclTy>),
    String(Ident),
    Type(Ident),
    Void(Ident),
}

impl Parse for DeclTy {
    fn parse(input: ParseStream<'_>) -> Result<Self> {
        if input.peek(Token![*]) {
            Ok(DeclTy::Ptr(input.parse()?, Box::new(input.parse()?)))
        } else if input.peek(Token![...]) {
            Ok(DeclTy::Ellipsis(input.parse()?))
        } else if input.peek(Ident) {
            let id: Ident = input.parse()?;
            if id == "float" {
                Ok(DeclTy::Float(id))
            } else if id == "int" {
                Ok(DeclTy::Int(id))
            } else if id == "noreturn" {
                Ok(DeclTy::Noreturn(id))
            } else if id == "string" {
                Ok(DeclTy::String(id))
            } else if id == "void" {
                Ok(DeclTy::Void(id))
            } else {
                Ok(DeclTy::Type(id))
            }
        } else {
            use syn::ext::IdentExt;
            let id = Ident::parse_any(input)?;
            abort!(id, "Unexpected token");
        }
    }
}

impl DeclTy {
    fn tokenize(&self) -> (TokenStream, bool) {
        match self {
            // a couple special cases first
            DeclTy::Ptr(_, box DeclTy::Void(_)) => (quote!(textual::Ty::VoidPtr), false),
            DeclTy::Ptr(_, box DeclTy::Type(id)) if id == "HackMixed" => {
                (quote!(textual::Ty::mixed_ptr()), false)
            }

            DeclTy::Ellipsis(_) => (quote!(textual::Ty::Ellipsis), true),
            DeclTy::Float(_) => (quote!(textual::Ty::Float), true),
            DeclTy::Int(_) => (quote!(textual::Ty::Int), true),
            DeclTy::Noreturn(_) => (quote!(textual::Ty::Noreturn), true),
            DeclTy::Ptr(_, box sub) => {
                let (mut sub, sub_owned) = sub.tokenize();
                if !sub_owned {
                    sub = quote!(#sub.clone());
                }
                (quote!(textual::Ty::Ptr(Box::new(#sub))), true)
            }
            DeclTy::String(_) => (quote!(textual::Ty::String), true),
            DeclTy::Type(name) => {
                let name = format!("{name}");
                (
                    quote!(textual::Ty::Type(crate::mangle::TypeName::UnmangledRef(#name))),
                    true,
                )
            }
            DeclTy::Void(_) => (quote!(textual::Ty::Void), true),
        }
    }
}
