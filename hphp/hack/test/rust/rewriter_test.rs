// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::relative_path::{Prefix, RelativePath};

use parser_core_types::{
    lexable_token::LexableToken, positioned_syntax::PositionedSyntax,
    positioned_token::PositionedToken, syntax::Syntax, token_kind::TokenKind,
};
use parser_rust::{
    parser::Parser, parser_env::ParserEnv, rewriter::Rewriter, smart_constructors::NoState,
    smart_constructors_wrappers::WithKind, source_text::SourceText, syntax::SyntaxVariant,
};
use positioned_parser::positioned_smart_constructors::PositionedSmartConstructors;

use std::io::Read;

type PositionedSyntaxParser<'a> = Parser<'a, WithKind<PositionedSmartConstructors>, NoState>;

#[test]
fn test_bracket_braces_swap() {
    let cwd = std::env::current_dir().unwrap();
    let file = cwd.join("hphp/hack/test/rust/rewriter_test_target.php");
    let mut contents = vec![];
    std::fs::File::open(file.to_str().unwrap())
        .unwrap()
        .read_to_end(&mut contents)
        .unwrap();
    let path = RelativePath::make(Prefix::Dummy, file);
    let source = SourceText::make(&path, &contents[..]);
    let mut parser = PositionedSyntaxParser::make(&source, ParserEnv::default());
    let root = parser.parse_script(None);

    let flatten = |node: PositionedSyntax, acc: &mut Vec<PositionedSyntax>| {
        acc.push(node.clone());
        Some(node)
    };
    let swap_braces_brackets = |node: PositionedSyntax| {
        if let SyntaxVariant::Token(t) = &node.syntax {
            let replacement = match t.kind() {
                TokenKind::LeftBracket => TokenKind::LeftBrace,
                TokenKind::LeftBrace => TokenKind::LeftBracket,
                TokenKind::RightBracket => TokenKind::RightBrace,
                TokenKind::RightBrace => TokenKind::RightBracket,
                _ => return Some(node),
            };
            return Some(Syntax::make_token(PositionedToken::make(
                replacement,
                &source,
                t.offset(),
                t.width(),
                t.leading().iter().map(|x| x.clone()).collect(),
                t.trailing().iter().map(|x| x.clone()).collect(),
            )));
        }
        Some(node)
    };

    let (root, initial_state) = Rewriter::aggregating_rewrite_post(root, vec![], flatten);
    let root = Rewriter::rewrite_post(root, swap_braces_brackets);
    let (root, intermediate_state) = Rewriter::aggregating_rewrite_post(root, vec![], flatten);
    let root = Rewriter::rewrite_post(root, swap_braces_brackets);
    let (_, final_state) = Rewriter::aggregating_rewrite_post(root, vec![], flatten);

    let initial_string_rep = format!("{:?}", initial_state);
    let intermediate_string_rep = format!("{:?}", intermediate_state);
    let final_string_rep = format!("{:?}", final_state);

    assert_ne!(initial_string_rep, intermediate_string_rep);
    assert_ne!(intermediate_string_rep, final_string_rep);
    assert_eq!(initial_string_rep, final_string_rep);
}
