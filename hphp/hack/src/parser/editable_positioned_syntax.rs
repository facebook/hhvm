// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod editable_positioned_original_source_data;
pub mod editable_positioned_syntax_to_ocaml;
pub mod editable_positioned_token;

use std::rc::Rc;

use parser_core_types::positioned_syntax::PositionedSyntax;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax::SyntaxValueType;
use parser_core_types::syntax::{Syntax, SyntaxVariant};
use parser_core_types::syntax_kind::SyntaxKind;

use crate::editable_positioned_original_source_data::SourceData;
use crate::editable_positioned_token::EditablePositionedToken;

#[derive(Clone, Debug)]
pub enum EditablePositionedValue<'a> {
    Positioned(SourceData<'a>),
    Synthetic,
}

use EditablePositionedValue::*;

impl<'a> SyntaxValueType<EditablePositionedToken<'a>> for EditablePositionedValue<'a> {
    fn from_values(_nodes: &[&Self]) -> Self {
        panic!("TODO")
    }

    fn from_syntax(variant: &SyntaxVariant<EditablePositionedToken<'a>, Self>) -> Self {
        let folder = |node: EditablePositionedSyntax<'a>, acc: (Self, Self)| match acc {
            (Synthetic, Synthetic) => (node.value, Synthetic),
            (val, _) => (val, node.value),
        };
        let (first, last) =
            Syntax::fold_over_children_owned(&folder, (Synthetic, Synthetic), variant.clone());
        match (first, last) {
            (Synthetic, Synthetic) => Synthetic,
            (fst, Synthetic) => fst,
            (Positioned(fst), Positioned(lst)) => {
                Positioned(SourceData::spanning_between(&fst, &lst))
            }
            _ => Synthetic,
        }
    }

    fn from_children(_: SyntaxKind, _offset: usize, nodes: &[&Self]) -> Self {
        if let Some((&last, rest)) = nodes.split_last() {
            match (last, rest.split_first()) {
                (_, None) => last.clone(),
                (Positioned(lst), Some((&Positioned(fst), _))) => {
                    Positioned(SourceData::spanning_between(fst, lst))
                }
                _ => Synthetic,
            }
        } else {
            Synthetic
        }
    }

    fn from_token(_token: &EditablePositionedToken) -> Self {
        panic!("TODO")
    }

    fn text_range(&self) -> Option<(usize, usize)> {
        panic!("TODO")
    }
}

impl<'a> EditablePositionedValue<'a> {
    fn from_positioned_syntax(node: &PositionedSyntax, source_text: &Rc<SourceText<'a>>) -> Self {
        EditablePositionedValue::Positioned(SourceData::from_positioned_syntax(node, source_text))
    }
}

pub type EditablePositionedSyntax<'a> =
    Syntax<EditablePositionedToken<'a>, EditablePositionedValue<'a>>;

pub trait EditablePositionedSyntaxTrait<'a> {
    fn from_positioned_syntax(node: &PositionedSyntax, source_text: &Rc<SourceText<'a>>) -> Self;
    fn text(node: &EditablePositionedSyntax<'a>) -> String;
}

impl<'a> EditablePositionedSyntaxTrait<'a> for EditablePositionedSyntax<'a> {
    // Recursively reconstructs a PositionedSyntax as a hierarchy of
    // EditablePositionedSyntaxes, each of which is Positioned.
    fn from_positioned_syntax(node: &PositionedSyntax, source_text: &Rc<SourceText<'a>>) -> Self {
        let syntax = match &node.syntax {
            SyntaxVariant::Token(token) => SyntaxVariant::Token(Box::new(
                EditablePositionedToken::from_positioned_token(&token, source_text),
            )),
            _ => {
                let children = node
                    .iter_children()
                    .map(|x| Self::from_positioned_syntax(x, source_text))
                    .collect();
                Self::from_children(node.kind(), children)
            }
        };

        Self::make(
            syntax,
            EditablePositionedValue::from_positioned_syntax(&node, source_text),
        )
    }

    fn text(node: &EditablePositionedSyntax<'a>) -> String {
        let tokens = Syntax::all_tokens(node);
        match tokens.split_first() {
            None => String::new(),
            Some((&fst, tail)) => {
                let start_text = EditablePositionedToken::text(fst);
                match tail.split_last() {
                    None => start_text,
                    Some((&lst, middle)) => {
                        let end_text = EditablePositionedToken::text(lst);
                        let mid_text = middle
                            .into_iter()
                            .map(|t| EditablePositionedToken::full_text(t))
                            .collect::<Vec<_>>()
                            .join("");
                        [
                            start_text,
                            fst.trailing_text.to_string(),
                            mid_text,
                            lst.leading_text.to_string(),
                            end_text,
                        ]
                        .join("")
                    }
                }
            }
        }
    }
}
