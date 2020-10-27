// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{positioned_token::PositionedToken, positioned_value::PositionedValue, syntax::Syntax};
use crate::{source_text::SourceText, syntax_trait::SyntaxTrait};

pub type PositionedSyntax<'a> = Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>;

impl<'a> SyntaxTrait for Syntax<'a, PositionedToken<'a>, PositionedValue<'a>> {
    fn offset(&self) -> Option<usize> {
        Some(self.start_offset())
    }

    fn width(&self) -> usize {
        self.value.width()
    }

    fn leading_width(&self) -> usize {
        self.value.leading_width()
    }

    fn trailing_width(&self) -> usize {
        self.value.trailing_width()
    }

    fn full_width(&self) -> usize {
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn leading_start_offset(&self) -> usize {
        self.value.start_offset()
    }

    fn extract_text<'src>(&self, source_text: &'src SourceText) -> Option<&'src str> {
        Some(self.text(source_text))
    }
}
