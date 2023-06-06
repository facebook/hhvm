// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::i_set::ISet;
use oxidized::pos::Pos;
use oxidized::prim_defs::Comment;
use oxidized::scoured_comments::ScouredComments;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::lexable_token::LexablePositionedToken;
use parser_core_types::lexable_trivia::LexableTrivium;
use parser_core_types::positioned_trivia::PositionedTrivium;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_by_ref::syntax::Syntax;
use parser_core_types::syntax_by_ref::syntax_variant_generated::SyntaxVariant::*;
use parser_core_types::syntax_trait::SyntaxTrait;
use parser_core_types::trivia_kind::TriviaKind;
use regex::bytes::Regex;
use rescan_trivia::RescanTrivia;

/** The full fidelity parser considers all comments "simply" trivia. Some
 * comments have meaning, though. This meaning can either be relevant for the
 * type checker (like HH_FIXME, etc.), but also for other uses, like
 * Codex, where comments are used for documentation generation.
 *
 * Inlining the scrape for comments in the lowering code would be prohibitively
 * complicated, but a separate pass is fine.
 */
pub struct ScourComment<'a, T, V> {
    pub indexed_source_text: &'a IndexedSourceText<'a>,
    pub include_line_comments: bool,
    pub allowed_decl_fixme_codes: &'a ISet,
    pub phantom: std::marker::PhantomData<(*const T, *const V)>,
    pub disable_hh_ignore_error: isize,
}

impl<'src, 'arena, T, V> ScourComment<'src, T, V>
where
    T: LexablePositionedToken + RescanTrivia<PositionedTrivium> + 'arena,
    V: 'arena,
    Syntax<'arena, T, V>: SyntaxTrait,
{
    pub fn scour_comments<'r>(&self, top_node: &'r Syntax<'arena, T, V>) -> ScouredComments
    where
        'r: 'arena,
    {
        let mut acc = ScouredComments::new();
        let mut stack: Vec<(&'r Syntax<'arena, T, V>, bool)> = vec![(top_node, false)];

        while let Some((node, mut in_block)) = stack.pop() {
            match &node.children {
                CompoundStatement(_) => in_block = true,
                Token(t) => {
                    if t.has_trivia_kind(TriviaKind::DelimitedComment)
                        || (self.include_line_comments
                            && t.has_trivia_kind(TriviaKind::SingleLineComment))
                        || (t.has_trivia_kind(TriviaKind::FixMe)
                            || (t.has_trivia_kind(TriviaKind::IgnoreError)
                                && self.disable_hh_ignore_error <= 1))
                    {
                        let leading = t.scan_leading(self.source_text());
                        let trailing = t.scan_trailing(self.source_text());
                        for tr in leading.iter().chain(trailing.iter()) {
                            self.on_trivia(in_block, node, tr, &mut acc);
                        }
                    }
                    continue;
                }
                _ => {}
            }

            stack.extend(node.iter_children().rev().map(|c| (c, in_block)));
        }

        acc.comments.reverse();
        acc
    }

    fn on_trivia(
        &self,
        in_block: bool,
        node: &Syntax<'arena, T, V>,
        t: &PositionedTrivium,
        acc: &mut ScouredComments,
    ) {
        use relative_path::Prefix;
        use TriviaKind::*;
        match t.kind() {
            WhiteSpace | EndOfLine | FallThrough | ExtraTokenError => {}
            DelimitedComment => {
                let start = t.start_offset() + 2;
                let end = t.end_offset();
                let len = end - start - 1;
                let p = self.pos_of_offset(end - 1, end);
                let text = self.source_text().sub_as_str(start, len).to_string();
                acc.comments.push((p, Comment::CmtBlock(text)));
            }
            SingleLineComment => {
                if self.include_line_comments {
                    let text = self.source_text().text();
                    let start = t.start_offset();
                    let start = start + if text[start] == b'#' { 1 } else { 2 };
                    let end = t.end_offset();
                    let len = end + 1 - start;
                    let p = self.pos_of_offset(start, end);
                    let mut text = self.source_text().sub_as_str(start, len).to_string();
                    text.push('\n');
                    acc.comments.push((p, Comment::CmtLine(text)));
                }
            }
            FixMe | IgnoreError => {
                lazy_static! {
                    static ref IGNORE_ERROR: Regex =
                        Regex::new(r#"HH_(?:FIXME|IGNORE_ERROR)[ \t\n]*\[([0-9]+)\]"#).unwrap();
                }

                let text = t.text_raw(self.source_text());
                let pos = self.p_pos(node);
                let line = pos.line() as isize;
                let p = self.pos_of_offset(t.start_offset(), t.end_offset() + 1);
                match IGNORE_ERROR
                    .captures(text)
                    .and_then(|c| c.get(1))
                    .map(|m| m.as_bytes())
                {
                    Some(code) => {
                        let code = std::str::from_utf8(code).unwrap();
                        let code: isize = std::str::FromStr::from_str(code).unwrap();
                        let in_hhi = pos.filename().prefix() == Prefix::Hhi;
                        if !(in_block || in_hhi || self.allowed_decl_fixme_codes.contains(&code)) {
                            acc.add_to_misuses(line, code, p);
                        } else if self.disable_hh_ignore_error == 1 && t.kind() == IgnoreError {
                            acc.add_disallowed_ignore(p);
                        } else {
                            acc.add_to_fixmes(line, code, p);
                        }
                    }
                    None => {
                        // Errors.fixme_format pos;
                        acc.add_format_error(pos);
                    }
                }
            }
        }
    }

    fn source_text(&self) -> &'src SourceText<'src> {
        self.indexed_source_text.source_text()
    }

    fn p_pos(&self, node: &Syntax<'arena, T, V>) -> Pos {
        node.position_exclusive(self.indexed_source_text)
            .map_or(Pos::NONE, Into::into)
    }

    fn pos_of_offset(&self, start: usize, end: usize) -> Pos {
        self.indexed_source_text.relative_pos(start, end).into()
    }
}
