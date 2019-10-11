// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
extern crate proc_macro;

use crate::{common, common::*};
use clap::ArgMatches;
use proc_macro2::TokenStream;
use quote::{format_ident, quote};
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

fn mk_file(file: &syn::File, uses: Vec<&str>) -> proc_macro2::TokenStream {
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

fn mk_impl(e: &ItemEnum) -> proc_macro2::TokenStream {
    let name = &e.ident;
    let generics = &e.generics;
    let mut seen_field = false;
    let mut constrs = vec![];
    for v in e.variants.iter() {
        let (has_field, constr) = mk_constr(name, v);
        seen_field = seen_field || has_field;
        constrs.push(constr);
    }
    if seen_field {
        quote! {
            impl#generics #name#generics {
                #(#constrs)*
            }
        }
    } else {
        quote! {}
    }
}

// It returns seen_fields and constructor function
fn mk_constr(enum_name: &Ident, v: &Variant) -> (bool, proc_macro2::TokenStream) {
    let name = &v.ident;
    let fn_name = format_ident!("mk_{}", &to_snake(&v.ident.to_string()));
    match &v.fields {
        Fields::Unit => (
            false,
            quote! {
                pub fn #fn_name () -> Self {
                    #enum_name::#name
                }
            },
        ),
        Fields::Unnamed(FieldsUnnamed { unnamed, .. }) => {
            let fields = unnamed.iter();
            let mut i = 0;
            let mut params: Vec<TokenStream> = vec![];
            let mut args: Vec<TokenStream> = vec![];
            for f in fields {
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
            (
                true,
                quote! {
                    pub fn #fn_name (#(#params)*) -> Self {
                        #enum_name::#name(#(#args)*)
                    }
                },
            )
        }
        Fields::Named(_) => {
            eprintln!("Warning: not support named field: {:?}", &v.ident);
            (false, quote! {})
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
