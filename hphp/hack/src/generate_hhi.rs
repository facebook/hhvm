// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Write;
use std::ops::ControlFlow;
use std::path::PathBuf;

use ocamlrep::rc::RcOc;
use parser_core_types::positioned_token::PositionedToken as Token;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax::Syntax;
use parser_core_types::syntax::SyntaxTypeBase;
use parser_core_types::syntax::SyntaxVariant;
use parser_core_types::token_factory::SimpleTokenFactory;
use parser_core_types::token_kind::TokenKind;
use relative_path::Prefix;
use relative_path::RelativePath;

/// Convert a Hack source file to an HHI interface definition file by removing
/// all function and method bodies.
#[derive(clap::Parser, Debug)]
struct Opts {
    /// The Hack source file to generate an HHI file for.
    filename: PathBuf,
}

fn main() -> anyhow::Result<()> {
    let opts = <Opts as clap::Parser>::from_args();
    let text = std::fs::read(&opts.filename)?;
    let source_text = SourceText::make(
        RcOc::new(RelativePath::make(Prefix::Dummy, opts.filename)),
        &text,
    );
    let (mut root, _, state) = positioned_parser::parse_script(&source_text, Default::default());
    root.rewrite_pre_and_stop(|node| {
        match &mut node.syntax {
            // remove `<?hh` line if present
            SyntaxVariant::MarkupSuffix(..) => {
                *node = Syntax::make_missing(&state, 0);
                ControlFlow::Break(())
            }
            // remove function bodies
            SyntaxVariant::FunctionDeclaration(f) => {
                f.function_body =
                    Syntax::make_token(Token::make(TokenKind::Semicolon, 0, 0, vec![], vec![]));
                ControlFlow::Break(())
            }
            // remove method bodies
            SyntaxVariant::MethodishDeclaration(m) => {
                // no body
                m.methodish_function_body = Syntax::make_missing(&state, 0);
                // always have a semicolon, even if not abstract
                m.methodish_semicolon =
                    Syntax::make_token(Token::make(TokenKind::Semicolon, 0, 0, vec![], vec![]));
                ControlFlow::Break(())
            }
            _ => ControlFlow::Continue(()),
        }
    });

    let mut stdout = std::io::BufWriter::new(std::io::stdout().lock());
    stdout.write_all(b"<?hh\n// @")?;
    stdout.write_all(b"generated from implementation\n\n")?;
    root.write_text_from_edited_tree(&source_text, &mut stdout)?;
    stdout.write_all(b"\n")?;
    stdout.flush()?;

    Ok(())
}
