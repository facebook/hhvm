// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::source_text::SourceText;

// SyntaxTrait defines basic functionality implemented by each Syntax. It corresponds to
// Syntax_sig::Syntax_S in OCaml implementation. It's a trait and not an  inherent implementation,
// because different syntaxes have different data to work with; for example full_width is already
// cached inside PositionedSyntax, while MinimalSyntax will have to iterate through entire subtree
// to compute it.
// Because of bugs in implementation and nothing ever enforcing it, in practice the values often
// will be different depending on the syntax, so be careful.
pub trait SyntaxTrait {
    fn offset(&self) -> Option<usize>;
    fn width(&self) -> usize;
    fn leading_width(&self) -> usize;
    fn trailing_width(&self) -> usize;
    fn full_width(&self) -> usize;
    fn leading_start_offset(&self) -> usize;
    fn extract_text<'a>(&self, source_text: &'a SourceText) -> Option<&'a str>;
}
