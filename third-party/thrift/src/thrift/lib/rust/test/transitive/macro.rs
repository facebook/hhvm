/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use proc_macro::TokenStream;
use quote::quote;

#[proc_macro_derive(GetStructNameD)]
pub fn standalone_subcommand_derive(input: TokenStream) -> TokenStream {
    let ast: syn::DeriveInput = syn::parse(input).unwrap();

    match ast.data {
        syn::Data::Struct(s) => s,
        _ => panic!("`#[derive(GetStructNameD)]` only supports Thrift structs"),
    };

    let name = &ast.ident;

    let name_string = name.to_string();

    let gen = quote! {
        impl ::get_struct_name_types::GetStructNameT for #name {
            fn get_struct_name() -> &'static str {
                #name_string
            }
        }
    };

    gen.into()
}
