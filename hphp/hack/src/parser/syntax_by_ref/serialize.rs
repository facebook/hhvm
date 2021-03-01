// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::positioned_token::PositionedTokenFullTrivia;
use crate::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken,
    positioned_trivia::PositionedTrivium,
};
use serde::{
    ser::{SerializeSeq, SerializeStruct},
    Serialize, Serializer,
};

pub struct WithContext<'a, T: ?Sized>(pub &'a IndexedSourceText<'a>, pub &'a T);

impl<'a, T> WithContext<'a, T> {
    pub(crate) fn with<S: ?Sized>(&self, x: &'a S) -> WithContext<'a, S> {
        WithContext(self.0, x)
    }
}

impl<'a, T> Serialize for WithContext<'a, [T]>
where
    WithContext<'a, T>: Serialize,
{
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        let mut ss = s.serialize_seq(Some(self.1.len()))?;
        for i in self.1.iter() {
            ss.serialize_element(&WithContext(self.0, i))?;
        }
        ss.end()
    }
}

impl<'a> Serialize for WithContext<'a, PositionedTokenFullTrivia<'a>> {
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        let offset = self.1.offset();
        let width = self.1.width();
        let token_offset = offset + self.1.leading_width();
        let mut ss = s.serialize_struct("", 9)?;
        ss.serialize_field("kind", self.1.kind().to_string())?;
        ss.serialize_field("text", self.0.source_text().sub_as_str(token_offset, width))?;
        ss.serialize_field("offset", &offset)?;
        ss.serialize_field("leading_width", &self.1.leading_width())?;
        ss.serialize_field("width", &width)?;
        ss.serialize_field("trailing_width", &self.1.trailing_width())?;
        ss.serialize_field("leading", &self.with(self.1.clone_leading().as_slice()))?;
        ss.serialize_field("trailing", &self.with(self.1.clone_trailing().as_slice()))?;
        ss.serialize_field(
            "line_number",
            &self.0.offset_to_position(token_offset as isize).0,
        )?;
        ss.end()
    }
}

impl<'a> Serialize for WithContext<'a, PositionedTrivium> {
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        let mut ss = s.serialize_struct("", 4)?;
        ss.serialize_field("kind", self.1.kind.to_string())?;
        ss.serialize_field(
            "text",
            self.0.source_text().sub_as_str(self.1.offset, self.1.width),
        )?;
        ss.serialize_field("offset", &self.1.offset)?;
        ss.serialize_field("width", &self.1.width)?;
        ss.end()
    }
}
