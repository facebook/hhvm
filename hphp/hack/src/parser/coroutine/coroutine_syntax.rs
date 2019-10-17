// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::{
    syntax::{Syntax, SyntaxType, SyntaxTypeBase, SyntaxVariant},
    token_kind::TokenKind,
};
use rust_editable_positioned_syntax::{
    editable_positioned_token::EditablePositionedToken, EditablePositionedSyntax,
    EditablePositionedValue,
};

const SINGLE_SPACE: &str = " ";

pub struct CoroutineSyntax {}

impl<'a> CoroutineSyntax {
    pub fn has_coroutine_modifier(modifiers: &EditablePositionedSyntax<'a>) -> bool {
        Syntax::syntax_node_to_list(modifiers).any(|x| x.is_coroutine())
    }

    pub fn make_syntax(
        syntax: SyntaxVariant<EditablePositionedToken<'a>, EditablePositionedValue<'a>>,
    ) -> EditablePositionedSyntax<'a> {
        Syntax::make(syntax, EditablePositionedValue::Synthetic)
    }

    pub fn make_member_selection_expression_syntax(
        receiver_syntax: EditablePositionedSyntax<'a>,
        member_syntax: EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        let member_selection_syntax = Self::make_token_syntax(TokenKind::MinusGreaterThan, None);
        SyntaxType::make_member_selection_expression(
            &(),
            receiver_syntax,
            member_selection_syntax,
            member_syntax,
        )
    }

    pub fn make_list(
        syntax_list: Vec<EditablePositionedSyntax<'a>>,
    ) -> EditablePositionedSyntax<'a> {
        Syntax::make_list(&(), syntax_list, 0)
    }

    pub fn make_missing() -> EditablePositionedSyntax<'a> {
        Syntax::make_missing(&(), 0)
    }

    fn make_token_syntax(
        token_kind: TokenKind,
        text: Option<String>,
    ) -> EditablePositionedSyntax<'a> {
        let text = text.unwrap_or(TokenKind::to_string(&token_kind).to_string());
        let token = EditablePositionedToken::synthesize_new(
            token_kind,
            String::from(SINGLE_SPACE),
            String::from(SINGLE_SPACE),
            text,
        );
        Self::make_syntax(SyntaxVariant::Token(Box::new(token)))
    }

    pub fn make_coroutine_token_syntax() -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Coroutine, None)
    }

    pub fn make_suspend_token_syntax() -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Suspend, None)
    }

    fn make_comma_syntax() -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Comma, None)
    }

    fn make_backslash_syntax() -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Backslash, None)
    }

    fn make_variable_syntax(text: String) -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Variable, Some(text))
    }

    pub fn make_name_syntax(name: String) -> EditablePositionedSyntax<'a> {
        if name.starts_with("\\") {
            Self::make_qualified_name_syntax(&vec![name.trim_start_matches('\\')], true)
        } else {
            Self::make_simple_name_syntax(name)
        }
    }

    fn make_simple_name_syntax(name: String) -> EditablePositionedSyntax<'a> {
        Self::make_token_syntax(TokenKind::Name, Some(name))
    }

    pub fn prepend_to_comma_delimited_syntax_list(
        prepend_syntax: EditablePositionedSyntax<'a>,
        syntax_list_syntax: &EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        let list_item = SyntaxType::make_list_item(&(), prepend_syntax, Self::make_comma_syntax());
        let mut syntax_list = Self::vecref_to_vec(
            Syntax::syntax_node_to_list(syntax_list_syntax).collect::<Vec<_>>(),
        );
        syntax_list.insert(0, list_item);
        Self::make_list(syntax_list)
    }

    pub fn make_variable_expression_syntax(text: String) -> EditablePositionedSyntax<'a> {
        SyntaxType::make_variable_expression(&(), Self::make_variable_syntax(text))
    }

    pub fn make_qualified_name_syntax(
        parts: &Vec<&str>,
        has_leading: bool,
    ) -> EditablePositionedSyntax<'a> {
        let mut parts: Vec<EditablePositionedSyntax<'a>> = parts
            .into_iter()
            .enumerate()
            .map(|(i, x)| {
                if i == parts.len() - 1 {
                    Self::make_qualified_name_part(x.to_string(), false)
                } else {
                    Self::make_qualified_name_part(x.to_string(), true)
                }
            })
            .collect();
        if has_leading {
            let backslash = SyntaxType::make_list_item(
                &(),
                Self::make_missing(),
                Self::make_backslash_syntax(),
            );
            parts.insert(0, backslash);
        }
        SyntaxType::make_qualified_name(&(), Self::make_list(parts))
    }

    fn make_qualified_name_part(name: String, has_backslash: bool) -> EditablePositionedSyntax<'a> {
        SyntaxType::make_list_item(
            &(),
            Self::make_simple_name_syntax(name),
            if has_backslash {
                Self::make_backslash_syntax()
            } else {
                Self::make_missing()
            },
        )
    }

    pub fn make_parameter_declaration_syntax(
        visibility_syntax: EditablePositionedSyntax<'a>,
        parameter_type_syntax: EditablePositionedSyntax<'a>,
        parameter_variable: String,
    ) -> EditablePositionedSyntax<'a> {
        let parameter_variable_syntax = Self::make_variable_syntax(parameter_variable);
        SyntaxType::make_parameter_declaration(
            &(),
            Self::make_missing(),
            visibility_syntax,
            Self::make_missing(),
            parameter_type_syntax,
            parameter_variable_syntax,
            Self::make_missing(),
        )
    }

    fn vecref_to_vec<T: Clone>(r: Vec<&T>) -> Vec<T> {
        let mut v = vec![];
        for e in r {
            v.push(e.clone());
        }
        v
    }
}
