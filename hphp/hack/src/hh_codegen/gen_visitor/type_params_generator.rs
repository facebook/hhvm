// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{context::Context, generator::Generator};
use crate::common::*;
use proc_macro2::TokenStream;
use quote::quote;

pub struct TypeParamGenerator;

impl Generator for TypeParamGenerator {
    fn filename(&self) -> String {
        "type_params.rs".into()
    }

    fn gen(&self, ctx: &Context) -> Result<TokenStream> {
        let ty_params = ctx.root_ty_params_with_context();
        Ok(quote! {
            pub trait Params {
                #(type #ty_params;)*
            }
        })
    }
}
