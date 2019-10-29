// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::core::mlvalues::Value;
use ocamlpool_rust::utils::{caml_set_field, reserve_block, u8_to_ocaml, usize_to_ocaml};
use ocamlrep_ocamlpool::add_to_ambient_pool;
use rust_to_ocaml::{to_list, SerializationContext, ToOcaml};

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
        let block = reserve_block(0.into(), 7);
        caml_set_field(block, 0, context.source_text);
        caml_set_field(block, 1, usize_to_ocaml(self.offset));
        caml_set_field(block, 2, usize_to_ocaml(self.leading_width));
        caml_set_field(block, 3, usize_to_ocaml(self.width));
        caml_set_field(block, 4, usize_to_ocaml(self.trailing_width));
        caml_set_field(block, 5, to_list(&self.leading, context));
        caml_set_field(block, 6, to_list(&self.trailing, context));
        block
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
                let block = reserve_block(0.into(), 1);
                caml_set_field(block, 0, source_data.to_ocaml(context));
                block
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
        let block = reserve_block(0.into(), 4);
        caml_set_field(block, 0, self.kind.to_ocaml(context));
        caml_set_field(block, 1, add_to_ambient_pool(&self.leading_text));
        caml_set_field(block, 2, add_to_ambient_pool(&self.trailing_text));
        caml_set_field(block, 3, self.token_data.to_ocaml(context));
        block
    }
}

impl ToOcaml for SyntheticTokenData {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        // From full_fidelity_editable_positioned_token.ml:
        // type synthetic_token_data = {
        //   text: string;
        // }
        let block = reserve_block(0.into(), 1);
        caml_set_field(block, 0, add_to_ambient_pool(&self.text));
        block
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
            TokenData::Original(source_data) => {
                let block = reserve_block(0.into(), 1);
                caml_set_field(block, 0, source_data.to_ocaml(context));
                block
            }
            TokenData::SynthesizedFromOriginal(synthetic_token_data, source_data) => {
                let block = reserve_block(1.into(), 2);
                caml_set_field(block, 0, synthetic_token_data.to_ocaml(context));
                caml_set_field(block, 1, source_data.to_ocaml(context));
                block
            }
            TokenData::Synthetic(synthetic_token_data) => {
                let block = reserve_block(2.into(), 1);
                caml_set_field(block, 0, synthetic_token_data.to_ocaml(context));
                block
            }
        }
    }
}
