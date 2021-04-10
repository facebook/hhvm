//
// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{expr_to_string_lossy, parse_file, Env};
use itertools::Either::*;
use ocamlrep::rc::RcOc;
use options::Options;
use oxidized::pos::Pos;
use oxidized::{
    aast,
    aast_visitor::{AstParams, Node, Visitor},
    ast,
};
use parser_core_types::source_text::SourceText;
use stack_limit::{StackLimit, MI};
use std::fs;

struct ExprTreeLiteralExtractor {
    literals: Vec<(Pos, ast::ExpressionTree)>,
}

impl<'ast> Visitor<'ast> for ExprTreeLiteralExtractor {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &ast::Expr) -> Result<(), ()> {
        use aast::Expr_::*;
        match &e.1 {
            ExpressionTree(et) => {
                self.literals.push((e.0.clone(), (&**et).clone()));
            }
            _ => e.recurse(env, self)?,
        }
        Ok(())
    }
}

/// Extract the expression tree literals in `program` along with their
/// positions.
///
/// Given the code:
///
/// ````
/// function foo(): void {
///   $c = Code`bar()`;
/// }
/// ```
///
/// We want the `` Code`bar()` `` part.
fn find_et_literals(program: ast::Program) -> Vec<(Pos, ast::ExpressionTree)> {
    let mut visitor = ExprTreeLiteralExtractor { literals: vec![] };
    for def in program {
        visitor
            .visit_def(&mut (), &def)
            .expect("Failed to extract expression tree literals");
    }

    visitor.literals
}

fn sort_by_start_pos<T>(items: &mut Vec<(Pos, T)>) {
    items.sort_by(|(p1, _), (p2, _)| p1.start_cnum().cmp(&p2.start_cnum()));
}

/// The source code of `program` with expression tree literals
/// replaced with their desugared form.
fn desugar_and_replace_et_literals<S: AsRef<str>>(
    env: &Env<S>,
    program: ast::Program,
    src: &str,
) -> String {
    let mut literals = find_et_literals(program);
    sort_by_start_pos(&mut literals);

    // Start from the last literal in the source code, so earlier
    // positions stay valid after string replacements.
    literals.reverse();

    let mut src = src.to_string();
    for (pos, literal) in literals {
        let desugared_literal_src = expr_to_string_lossy(env, &literal.runtime_expr);
        let (pos_start, pos_end) = pos.info_raw();
        src.replace_range(pos_start..pos_end, &desugared_literal_src);
    }

    src
}

/// Parse the file in `env`, desugar expression tree literals, and
/// print the source code as if the user manually wrote the desugared
/// syntax.
pub fn desugar_and_print<S: AsRef<str>>(env: &Env<S>) {
    let filepath = env.filepath.clone();
    let content = fs::read(filepath.to_absolute()).unwrap();

    let source_text = SourceText::make(RcOc::new(filepath), &content);

    let limit = StackLimit::relative(13 * MI);
    let opts = Options::from_configs(&env.config_jsons, &env.config_list).expect("Invalid options");
    match parse_file(&opts, &limit, source_text, false) {
        Left((_, msg, _)) => panic!("Parsing failed: {}", msg),
        Right(ast) => {
            let old_src = String::from_utf8_lossy(&content);
            let new_src = desugar_and_replace_et_literals(&env, ast, &old_src);
            print!("{}", new_src);
        }
    }
}
