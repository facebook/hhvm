// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro::TokenStream;

#[proc_macro_attribute]
pub fn rust_to_ocaml(_attr: TokenStream, item: TokenStream) -> TokenStream {
    item
}
