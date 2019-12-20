// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
extern crate proc_macro;

mod ref_kind;

use crate::{common, common::*, quote_helper::*};

use clap::ArgMatches;
use proc_macro2::TokenStream;
use quote::{format_ident, quote};
use ref_kind::RefKind;
use std::{
    fmt::Write,
    fs::File,
    io::Read,
    path::{Path, PathBuf},
};
use syn::*;

pub fn run(m: &ArgMatches) -> common::Result<Vec<(PathBuf, String)>> {
    let inputs = m.values_of("input").ok_or("missing input files")?;
    let output_dir = Path::new(m.value_of("output").ok_or("missing output path")?);
    let mut result = vec![];
    let mut mods = vec![];
    for input in inputs {
        let (file, uses) = parse_input_arg(input);
        eprintln!("Process: {}", file);
        eprintln!("Uses: {:?}", uses);
        let mut output_filename = Path::new(file)
            .file_stem()
            .ok_or("Unable to get file stem")?
            .to_os_string();
        output_filename.push("_impl_gen");
        mods.push(output_filename.clone().into_string().unwrap());
        output_filename.push(".rs");
        let output_file = output_dir.join(Path::new(&output_filename));
        let mut file = File::open(file)?;
        let mut src = String::new();
        file.read_to_string(&mut src)?;
        let file = syn::parse_file(&src)?;
        let output = mk_file(&file, uses);
        let mut output_content = String::new();
        write!(&mut output_content, "{}", output)?;
        eprintln!("Output: {:?}", output_file);
        result.push((output_file, output_content));
    }

    let mut mod_filename = output_dir.to_path_buf();
    mod_filename.push("mod.rs");
    let mod_content = mk_mod_file(mods)?;
    result.push((mod_filename, mod_content));
    Ok(result)
}

fn mk_mod_file(mods: Vec<String>) -> common::Result<String> {
    let mods = mods.into_iter().map(|m| format_ident!("{}", m));
    let content = quote! {
      #(pub mod #mods;)*
    };
    let mut result = String::new();
    write!(&mut result, "{}", content)?;
    Ok(result)
}

fn parse_input_arg<'a>(file_with_uses: &'a str) -> (&'a str, Vec<&'a str>) {
    let mut data = file_with_uses.split("|");
    (data.next().unwrap(), data.collect::<Vec<_>>())
}

fn mk_file(file: &syn::File, uses: Vec<&str>) -> TokenStream {
    let uses = uses
        .into_iter()
        .map(|u| syn::parse_str::<UseTree>(u).unwrap());
    let enums = get_enums(&file);
    let content = enums.into_iter().map(mk_impl);
    quote! {
        #(use #uses;)*
        #(#content)*
    }
}

fn mk_impl(e: &ItemEnum) -> TokenStream {
    let name = &e.ident;
    let is_singleton = e.variants.len() < 2;
    let generics = &e.generics;
    let constrs = e.variants.iter().map(|v| mk_constr(name, v));
    let is_functions = e
        .variants
        .iter()
        .map(|v| mk_is_function(name, v, is_singleton));
    let as_ref_functions = e
        .variants
        .iter()
        .map(|v| mk_as_function("", name, v, RefKind::Ref, is_singleton));

    let as_mut_functions = e
        .variants
        .iter()
        .map(|v| mk_as_function("_mut", name, v, RefKind::RefMut, is_singleton));

    let as_into_functions = e
        .variants
        .iter()
        .map(|v| mk_as_function("_into", name, v, RefKind::Owned, is_singleton));

    quote! {
        impl#generics #name#generics {
            #(#constrs)*
            #(#is_functions)*
            #(#as_ref_functions)*
            #(#as_mut_functions)*
            #(#as_into_functions)*
        }
    }
}

fn mk_as_function(
    fn_name_suffix: &str,
    enum_name: &Ident,
    v: &Variant,
    ref_kind: RefKind,
    is_singleton: bool,
) -> TokenStream {
    let name = &v.ident;
    let fn_name = format_ident!("as_{}{}", &to_snake(&name.to_string()), fn_name_suffix);
    match &v.fields {
        Fields::Unit => quote! {},
        Fields::Unnamed(FieldsUnnamed { unnamed, .. }) => {
            let mut i = 0;
            let mut field_match: Vec<TokenStream> = vec![];
            let mut results: Vec<TokenStream> = vec![];
            let mut return_tys: Vec<TokenStream> = vec![];
            for field in unnamed {
                let matched = format_ident!("p{}", i.to_string());
                field_match.push(quote! { #matched, });
                let tys = unbox(&field.ty);
                if tys.is_empty() {
                    results.push(ref_kind.mk_value(&matched, false, None));
                    return_tys.push(ref_kind.mk_ty(&field.ty));
                } else if tys.len() == 1 {
                    results.push(ref_kind.mk_value(&matched, true, None));
                    return_tys.push(ref_kind.mk_ty(&tys[0]));
                } else {
                    let mut j = 0;
                    for ty in tys {
                        results.push(ref_kind.mk_value(&matched, true, Some(j)));
                        return_tys.push(ref_kind.mk_ty(ty));
                        j += 1;
                    }
                }
                i += 1;
            }
            let sep = <Token![,]>::default();
            let return_tys = if return_tys.len() > 1 {
                with_paren(join(return_tys.iter(), sep))
            } else {
                join(return_tys.iter(), sep)
            };
            let results = if results.len() > 1 {
                with_paren(join(results.iter(), sep))
            } else {
                join(results.iter(), sep)
            };
            let self_ = ref_kind.mk_ty(&format_ident!("self"));
            let else_ = if is_singleton {
                quote! {}
            } else {
                quote! { _ => None, }
            };
            quote! {
                pub fn #fn_name(#self_) -> Option<#return_tys> {
                    match self {
                        #enum_name::#name(#(#field_match)*) => Some(#results),
                        #else_
                    }
                }
            }
        }
        Fields::Named(_) => {
            eprintln!("Warning: not support named field: {:?}", &v.ident);
            quote! {}
        }
    }
}

fn mk_is_function(enum_name: &Ident, v: &Variant, is_singleton: bool) -> TokenStream {
    let name = &v.ident;
    let field_match = if let Fields::Unit = &v.fields {
        quote! {}
    } else {
        quote! { (..) }
    };
    let body = if is_singleton {
        quote! { true }
    } else {
        quote! {
            match self {
                #enum_name::#name#field_match => true,
                _ => false,
            }
        }
    };
    let fn_name = format_ident!("is_{}", &to_snake(&name.to_string()));
    quote! {
        pub fn #fn_name(&self) -> bool { #body }
    }
}

fn mk_constr(enum_name: &Ident, v: &Variant) -> TokenStream {
    let name = &v.ident;
    let fn_name = format_ident!("mk_{}", &to_snake(&name.to_string()));
    match &v.fields {
        Fields::Unit => quote! {
            pub fn #fn_name () -> Self {
                #enum_name::#name
            }
        },
        Fields::Unnamed(FieldsUnnamed { unnamed, .. }) => {
            let mut i = 0;
            let mut params: Vec<TokenStream> = vec![];
            let mut args: Vec<TokenStream> = vec![];
            for f in unnamed {
                let ty = &f.ty;
                let boxed_tys = unbox(ty);
                if boxed_tys.is_empty() {
                    let param = format_ident!("p{}", i.to_string());
                    params.push(quote! {#param : #ty, });
                    args.push(quote! {#param ,});
                    i += 1;
                } else if boxed_tys.len() == 1 {
                    let param = format_ident!("p{}", i.to_string());
                    let ty = boxed_tys[0];
                    params.push(quote! {#param : #ty, });
                    args.push(quote! {Box::new(#param) ,});
                    i += 1;
                } else {
                    let mut tuple_items: Vec<TokenStream> = vec![];
                    for ty in boxed_tys.iter() {
                        let param = format_ident!("p{}", i.to_string());
                        params.push(quote! {#param : #ty, });
                        tuple_items.push(quote! {#param, });
                        i += 1;
                    }
                    args.push(quote! {Box::new(( #(#tuple_items)* )) ,});
                }
            }
            quote! {
                pub fn #fn_name (#(#params)*) -> Self {
                    #enum_name::#name(#(#args)*)
                }
            }
        }
        Fields::Named(_) => {
            eprintln!("Warning: not support named field: {:?}", &v.ident);
            quote! {}
        }
    }
}

fn get_enums(file: &syn::File) -> Vec<&ItemEnum> {
    let mut r = vec![];
    for i in file.items.iter() {
        if let Item::Enum(e) = i {
            r.push(e);
        }
    }
    r
}

fn unbox(ty: &Type) -> Vec<&Type> {
    if let Type::Path(TypePath { path, .. }) = ty {
        if let Some(path_seg) = path.segments.first() {
            if path_seg.ident == "Box" {
                if let syn::PathArguments::AngleBracketed(args) = &path_seg.arguments {
                    match args.args.first() {
                        Some(GenericArgument::Type(Type::Tuple(syn::TypeTuple {
                            elems, ..
                        }))) => {
                            return elems.iter().collect::<Vec<_>>();
                        }
                        Some(GenericArgument::Type(ty)) => {
                            return vec![ty];
                        }
                        _ => {
                            eprintln!("Warnning: box missing type argument");
                            return vec![ty];
                        }
                    }
                }
            }
        }
    }
    return vec![];
}
