// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::pos::Pos;

use crate::indexed_source_text::IndexedSourceText;
use crate::source_text::SourceText;

/// SyntaxTrait defines basic functionality implemented by each Syntax.
/// It corresponds to Syntax_sig::Syntax_S in OCaml implementation. It is a trait
/// and not an inherent implementation, because different syntaxes have different
/// data to work with; for example full_width is already cached inside
/// PositionedSyntax, while MinimalSyntax will have to iterate through entire
/// subtree to compute it. Because of bugs in implementation and nothing ever
/// enforcing it, in practice the values often will be different depending on
/// the syntax, so be careful.
pub trait SyntaxTrait {
    fn offset(&self) -> Option<usize>;
    fn width(&self) -> usize;
    fn leading_width(&self) -> usize;
    fn trailing_width(&self) -> usize;
    fn full_width(&self) -> usize;
    fn leading_start_offset(&self) -> usize;
    fn extract_text<'a>(&self, source_text: &'a SourceText<'_>) -> Option<&'a str>;

    fn start_offset(&self) -> usize {
        self.leading_start_offset() + self.leading_width()
    }

    fn end_offset(&self) -> usize {
        self.start_offset() + self.width().saturating_sub(1)
    }

    fn position_exclusive(&self, source_text: &IndexedSourceText<'_>) -> Option<Pos> {
        let start_offset = self.start_offset();
        let end_offset = self.end_offset() + 1;
        Some(source_text.relative_pos(start_offset, end_offset))
    }

    fn position(&self, source_text: &IndexedSourceText<'_>) -> Option<Pos> {
        let start_offset = self.start_offset();
        let end_offset = self.end_offset();
        Some(source_text.relative_pos(start_offset, end_offset))
    }

    fn text<'a>(&self, source_text: &'a SourceText<'_>) -> &'a str {
        source_text.sub_as_str(self.start_offset(), self.width())
    }

    fn full_text<'a>(&self, source_text: &'a SourceText<'_>) -> &'a [u8] {
        source_text.sub(self.leading_start_offset(), self.full_width())
    }

    fn leading_text<'a>(&self, source_text: &'a SourceText<'_>) -> &'a str {
        source_text.sub_as_str(self.leading_start_offset(), self.leading_width())
    }
}
