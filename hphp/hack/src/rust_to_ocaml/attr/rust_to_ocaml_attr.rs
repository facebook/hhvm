// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro::TokenStream;

/// This attribute macro is intended to be consumed by the rust_to_ocaml codegen
/// tool, so this proc macro doesn't need to do anything other than return the
/// item (with the rust_to_ocaml attribute stripped by rustc).
///
/// Use of the rust_to_ocaml attribute in positions other than items (like field
/// definitions) are stripped by ocamlrep_derive macros (which is simpler than
/// filtering them from the `item` in this crate).
///
/// We may want to add validation later so that incorrect use of the attribute
/// emits errors at compile time, but stripping is good enough for now.
#[proc_macro_attribute]
pub fn rust_to_ocaml(_attr: TokenStream, item: TokenStream) -> TokenStream {
    item
}
