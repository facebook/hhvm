// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::context::Context;
use crate::common::*;
use proc_macro2::TokenStream;

#[macro_export]
macro_rules! impl_generator {
    ($ty:ty, $base:ident) => {
        impl Generator for $ty {
            fn filename(&self) -> String {
                <Self as $base>::filename()
            }

            fn gen(&self, ctx: &Context) -> Result<TokenStream> {
                <Self as $base>::gen(ctx)
            }
        }
    };
}

pub trait Generator {
    fn filename(&self) -> String;
    fn gen(&self, ctx: &Context) -> Result<TokenStream>;
}
