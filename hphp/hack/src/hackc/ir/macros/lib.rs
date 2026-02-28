// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod has_loc;
mod has_locals;
mod has_operands;
mod simple_type;
mod util;

use proc_macro::TokenStream;

#[proc_macro_derive(HasLoc, attributes(has_loc))]
pub fn has_loc_macro(input: TokenStream) -> TokenStream {
    match has_loc::build_has_loc(input.into()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro_derive(HasLocals, attributes(has_locals))]
pub fn has_locals_macro(input: TokenStream) -> TokenStream {
    match has_locals::build_has_locals(input.into()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro_derive(HasOperands, attributes(has_operands))]
pub fn has_operands_macro(input: TokenStream) -> TokenStream {
    match has_operands::build_has_operands(input.into()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}
