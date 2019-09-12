// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::core::mlvalues::Value;
use ocamlpool_rust::ocamlvalue::Ocamlvalue;
use ocamlpool_rust::utils::{caml_block, caml_tuple, u8_to_ocaml, usize_to_ocaml};
use rust_to_ocaml::rust_to_ocaml::{to_list, SerializationContext, ToOcaml};

use crate::editable_positioned_original_source_data::SourceData;
use crate::editable_positioned_token::{EditablePositionedToken, SyntheticTokenData, TokenData};
use crate::EditablePositionedValue;

impl ToOcaml for SourceData<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // from Full_fidelity_editable_positioned_original_source_data.ml:
        // type t = {
        //   source_text: SourceText.t;
        //   offset: int;
        //   leading_width: int;
        //   width: int;
        //   trailing_width: int;
        //   leading: Trivia.t list;
        //   trailing: Trivia.t list;
        // }
        caml_tuple(&[
            context.source_text,
            usize_to_ocaml(self.offset),
            usize_to_ocaml(self.leading_width),
            usize_to_ocaml(self.width),
            usize_to_ocaml(self.trailing_width),
            to_list(&self.leading, context),
            to_list(&self.trailing, context),
        ])
    }
}

impl ToOcaml for EditablePositionedValue<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_editable_positioned_syntax.ml:
        // type t =
        //   | Positioned of SourceData.t
        //   | Synthetic
        match self {
            EditablePositionedValue::Positioned(source_data) => {
                caml_block(0, &[source_data.to_ocaml(context)])
            }
            EditablePositionedValue::Synthetic => u8_to_ocaml(0),
        }
    }
}

impl ToOcaml for EditablePositionedToken<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_editable_positioned_token.ml:
        // type t = {
        //   kind: TokenKind.t;
        //   leading_text: string;
        //   trailing_text: string;
        //   token_data: token_data;
        // }
        caml_tuple(&[
            self.kind.to_ocaml(context),
            self.leading_text.ocamlvalue(),
            self.trailing_text.ocamlvalue(),
            self.token_data.to_ocaml(context),
        ])
    }
}

impl ToOcaml for SyntheticTokenData {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        // From full_fidelity_editable_positioned_token.ml:
        // type synthetic_token_data = {
        //   text: string;
        // }
        caml_tuple(&[self.text.ocamlvalue()])
    }
}

impl ToOcaml for TokenData<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_editable_positioned_token.ml:
        // type token_data =
        //   | Original of SourceData.t
        //   | SynthesizedFromOriginal of synthetic_token_data * SourceData.t
        //   | Synthetic of synthetic_token_data
        match self {
            TokenData::Original(source_data) => caml_block(0, &[source_data.to_ocaml(context)]),
            TokenData::SynthesizedFromOriginal(synthetic_token_data, source_data) => caml_block(
                1,
                &[
                    synthetic_token_data.to_ocaml(context),
                    source_data.to_ocaml(context),
                ],
            ),
            TokenData::Synthetic(synthetic_token_data) => {
                caml_block(2, &[synthetic_token_data.to_ocaml(context)])
            }
        }
    }
}
