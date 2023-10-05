// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::lexable_token::LexableToken;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_error::{self as Errors};
use parser_core_types::token_kind::TokenKind;
use parser_core_types::trivia_kind::TriviaKind;

use crate::declaration_parser::DeclarationParser;
use crate::expression_parser::ExpressionParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::Context;
use crate::parser_trait::ExpectedTokens;
use crate::parser_trait::ParserTrait;
use crate::pattern_parser::PatternParser;
use crate::smart_constructors::NodeType;
use crate::smart_constructors::SmartConstructors;
use crate::smart_constructors::Token;
use crate::type_parser::TypeParser;

#[derive(Clone)]
pub struct StatementParser<'a, S>
where
    S: SmartConstructors,
    S::Output: NodeType,
{
    lexer: Lexer<'a, S::Factory>,
    env: ParserEnv,
    context: Context<Token<S>>,
    errors: Vec<SyntaxError>,
    sc: S,
}

impl<'a, S> ParserTrait<'a, S> for StatementParser<'a, S>
where
    S: SmartConstructors,
    S::Output: NodeType,
{
    fn make(
        lexer: Lexer<'a, S::Factory>,
        env: ParserEnv,
        context: Context<Token<S>>,
        errors: Vec<SyntaxError>,
        sc: S,
    ) -> Self {
        Self {
            lexer,
            env,
            context,
            errors,
            sc,
        }
    }

    fn into_parts(
        self,
    ) -> (
        Lexer<'a, S::Factory>,
        Context<Token<S>>,
        Vec<SyntaxError>,
        S,
    ) {
        (self.lexer, self.context, self.errors, self.sc)
    }

    fn lexer(&self) -> &Lexer<'a, S::Factory> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Factory> {
        &mut self.lexer
    }

    fn continue_from<P: ParserTrait<'a, S>>(&mut self, other: P) {
        let (lexer, context, errors, sc) = other.into_parts();
        self.lexer = lexer;
        self.context = context;
        self.errors = errors;
        self.sc = sc;
    }

    fn add_error(&mut self, error: SyntaxError) {
        self.errors.push(error)
    }

    fn env(&self) -> &ParserEnv {
        &self.env
    }

    fn sc_mut(&mut self) -> &mut S {
        &mut self.sc
    }

    fn drain_skipped_tokens(&mut self) -> std::vec::Drain<'_, Token<S>> {
        self.context.skipped_tokens.drain(..)
    }

    fn skipped_tokens(&self) -> &[Token<S>] {
        &self.context.skipped_tokens
    }

    fn context_mut(&mut self) -> &mut Context<Token<S>> {
        &mut self.context
    }

    fn context(&self) -> &Context<Token<S>> {
        &self.context
    }
}

impl<'a, S> StatementParser<'a, S>
where
    S: SmartConstructors,
    S::Output: NodeType,
{
    fn with_type_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut TypeParser<'a, S>) -> U,
    {
        let mut type_parser: TypeParser<'_, S> = TypeParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut type_parser);
        self.continue_from(type_parser);
        res
    }

    fn parse_type_specifier(&mut self) -> S::Output {
        self.with_type_parser(|x: &mut TypeParser<'a, S>| x.parse_type_specifier(false, true))
    }

    fn with_expression_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut ExpressionParser<'a, S>) -> U,
    {
        let mut expression_parser: ExpressionParser<'_, S> = ExpressionParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut expression_parser);
        self.continue_from(expression_parser);
        res
    }

    fn with_decl_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut DeclarationParser<'a, S>) -> U,
    {
        let mut decl_parser: DeclarationParser<'_, S> = DeclarationParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut decl_parser);
        self.continue_from(decl_parser);
        res
    }

    fn with_pattern_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut PatternParser<'a, S>) -> U,
    {
        let mut pattern_parser: PatternParser<'_, S> = PatternParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut pattern_parser);
        self.continue_from(pattern_parser);
        res
    }

    pub fn parse_statement(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::Async | TokenKind::Function => {
                self.parse_possible_php_function(/* toplevel:*/ false)
            }
            TokenKind::Abstract
            | TokenKind::Final
            | TokenKind::Interface
            | TokenKind::Trait
            | TokenKind::XHP
            | TokenKind::Class => {
                self.with_error(Errors::decl_outside_global_scope, Vec::new());
                self.with_decl_parser(|x| {
                    let pos = x.pos();
                    let missing = x.sc_mut().make_missing(pos);
                    x.parse_classish_declaration(missing)
                })
            }
            TokenKind::Fallthrough => self.parse_possible_erroneous_fallthrough(),
            TokenKind::For => self.parse_for_statement(),
            TokenKind::Foreach => self.parse_foreach_statement(),
            TokenKind::Do => self.parse_do_statement(),
            TokenKind::While => self.parse_while_statement(),
            TokenKind::Using => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_using_statement(missing)
            }
            TokenKind::Await if self.peek_token_kind_with_lookahead(1) == TokenKind::Using => {
                let await_kw = self.assert_token(TokenKind::Await);
                self.parse_using_statement(await_kw)
            }
            TokenKind::If => self.parse_if_statement(),
            TokenKind::Switch => self.parse_switch_statement(),
            TokenKind::Match => self.parse_match_statement(),
            TokenKind::Try => self.parse_try_statement(),
            TokenKind::Break => self.parse_break_statement(),
            TokenKind::Continue => self.parse_continue_statement(),
            TokenKind::Return => self.parse_return_statement(),
            TokenKind::Yield => {
                match self.peek_token_kind_with_lookahead(1) {
                    // yield break;
                    TokenKind::Break => self.parse_yield_break_statement(),
                    // yield;
                    // yield some_expression;
                    _ => self.parse_expression_statement(),
                }
            }
            TokenKind::Throw => self.parse_throw_statement(),
            TokenKind::LeftBrace => self.parse_compound_statement(),
            TokenKind::Static => self.parse_expression_statement(),
            TokenKind::Echo => self.parse_echo_statement(),
            TokenKind::Concurrent => self.parse_concurrent_statement(),
            TokenKind::Unset => self.parse_unset_statement(),
            TokenKind::Let => self.parse_declare_local_statement(),
            TokenKind::Case => {
                let result = self.parse_case_label();
                // TODO: This puts the error in the wrong place. We should highlight
                // the entire label, not the trailing colon.
                self.with_error(Errors::error2003, Vec::new());
                result
            }
            TokenKind::Default => {
                let result = self.parse_default_label();
                // TODO: This puts the error in the wrong place. We should highlight
                // the entire label, not the trailing colon.
                self.with_error(Errors::error2004, Vec::new());
                result
            }
            TokenKind::Semicolon => self.parse_expression_statement(),
            // ERROR RECOVERY: when encountering a token that's invalid now but the
            // context says is expected later, make the whole statement missing
            // and continue on, starting at the unexpected token.
            // TODO T20390825: Make sure this this won't cause premature recovery.
            kind if self.expects(kind) => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => self.parse_expression_statement(),
        }
    }

    pub fn parse_header(&mut self) -> (S::Output, bool) {
        let (markup, suffix_opt) = self.lexer.scan_header();
        let (suffix, has_suffix) = match suffix_opt {
            Some((less_than_question, language_opt)) => {
                let less_than_question_token = self.sc_mut().make_token(less_than_question);
                let language = match language_opt {
                    Some(language) => {
                        let token = self.sc_mut().make_token(language);
                        token
                    }
                    None => {
                        let pos = self.pos();
                        let missing = self.sc_mut().make_missing(pos);
                        missing
                    }
                };
                let suffix = self
                    .sc_mut()
                    .make_markup_suffix(less_than_question_token, language);
                (suffix, true)
            }
            None => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                (missing, false)
            }
        };
        let s = match (markup, has_suffix) {
            (Some(markup), _) => {
                let markup = self.sc_mut().make_token(markup);
                self.sc_mut().make_markup_section(markup, suffix)
            }
            (None, false) => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            (None, true) => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.sc_mut().make_markup_section(missing, suffix)
            }
        };
        (s, has_suffix)
    }

    pub fn parse_possible_php_function(&mut self, toplevel: bool) -> S::Output {
        // ERROR RECOVERY: PHP supports nested named functions, but Hack does not.
        // (Hack only supports anonymous nested functions as expressions.)
        //
        // If we have a statement beginning with function left-paren, then parse it
        // as a statement expression beginning with an anonymous function; it will
        // then have to end with a semicolon.
        //
        // If it starts with something else, parse it as a function.
        //
        // TODO: Give an error for nested nominal functions in a later pass.
        let kind0 = self.peek_token_kind_with_lookahead(0);
        let kind1 = self.peek_token_kind_with_lookahead(1);
        match (kind0, kind1) {
            | (TokenKind::Async, TokenKind::Function)
                if self.peek_token_kind_with_lookahead(2) == TokenKind::LeftParen =>
                self.parse_expression_statement(),
            | (TokenKind::Function, TokenKind::LeftParen) // Verbose-style lambda
            // Async, compact-style lambda
            | (TokenKind::Async, TokenKind::LeftParen)
            | (TokenKind::Async, TokenKind::LeftBrace) // Async block
                => self.parse_expression_statement(),
            | _ => {
                let missing = self.with_decl_parser(|x: &mut DeclarationParser<'a, S>| {
		    let pos = x.pos();
                    let missing = x.sc_mut().make_missing(pos);
                    x.parse_function_declaration(missing)
                });
                if !toplevel {
                    self.with_error(Errors::inline_function_def, Vec::new())
                };
                missing
            }
        }
    }

    // Helper: parses ( expr )
    fn parse_paren_expr(&mut self) -> (S::Output, S::Output, S::Output) {
        let left_paren = self.require_left_paren();
        let expr_syntax = self.parse_expression();
        let right_paren = self.require_right_paren();
        (left_paren, expr_syntax, right_paren)
    }

    fn parse_for_statement(&mut self) -> S::Output {
        // SPEC
        // for-statement:
        //   for   (   for-initializer-opt   ;   for-control-opt   ;    \
        //     for-end-of-loop-opt   )   statement
        //
        // The initialize and end-of-loop clauses are optional,
        // comma-separated lists of expressions. The control clause is
        // an optional single expression.
        //
        // Note that unlike most such lists in Hack, it may *not* have a trailing
        // comma.
        //
        // TODO: There is no compelling reason to not allow a trailing comma
        // from the grammatical point of view. Each clause unambiguously ends in
        // either a semi or a paren, so we can allow a trailing comma without
        // difficulty.
        let for_keyword_token = self.assert_token(TokenKind::For);
        let for_left_paren = self.require_left_paren();
        let for_initializer_expr =
            self.parse_comma_list_opt(TokenKind::Semicolon, Errors::error1015, |x| {
                x.parse_expression()
            });
        let for_first_semicolon = self.require_semicolon();
        let for_control_expr = match self.peek_token_kind() {
            TokenKind::Semicolon => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => self.parse_expression(),
        };
        let for_second_semicolon = self.require_semicolon();
        let for_end_of_loop_expr =
            self.parse_comma_list_opt(TokenKind::RightParen, Errors::error1015, |x| {
                x.parse_expression()
            });
        let for_right_paren = self.require_right_paren();
        let for_statement = self.parse_statement();
        self.sc_mut().make_for_statement(
            for_keyword_token,
            for_left_paren,
            for_initializer_expr,
            for_first_semicolon,
            for_control_expr,
            for_second_semicolon,
            for_end_of_loop_expr,
            for_right_paren,
            for_statement,
        )
    }

    fn parse_foreach_statement(&mut self) -> S::Output {
        let foreach_keyword_token = self.assert_token(TokenKind::Foreach);
        let foreach_left_paren = self.require_left_paren();
        self.expect_in_new_scope(ExpectedTokens::RightParen);
        let foreach_collection_name =
            self.with_expression_parser(|x: &mut ExpressionParser<'a, S>| {
                x.with_as_expressions(false, |x| x.parse_expression())
            });
        let await_token = self.optional_token(TokenKind::Await);
        let as_token = self.require_as();
        let mut parser1 = self.clone();
        let after_as = parser1.parse_expression();

        let (foreach_key, foreach_arrow, foreach_value) = match parser1.peek_token_kind() {
            TokenKind::RightParen => {
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                let value = self.parse_expression();
                (missing1, missing2, value)
            }
            TokenKind::EqualGreaterThan => {
                self.continue_from(parser1);
                let arrow = self.assert_token(TokenKind::EqualGreaterThan);
                let value = self.parse_expression();
                (after_as, arrow, value)
            }
            _ => {
                self.continue_from(parser1);
                self.with_error(Errors::invalid_foreach_element, Vec::new());
                let token = self.fetch_token();
                let error = self.sc_mut().make_error(token);
                let foreach_value = self.parse_expression();
                (after_as, error, foreach_value)
            }
        };
        let right_paren_token = self.require_right_paren();

        self.pop_scope(ExpectedTokens::RightParen);
        let foreach_statement = self.parse_statement();
        self.sc_mut().make_foreach_statement(
            foreach_keyword_token,
            foreach_left_paren,
            foreach_collection_name,
            await_token,
            as_token,
            foreach_key,
            foreach_arrow,
            foreach_value,
            right_paren_token,
            foreach_statement,
        )
    }

    fn parse_do_statement(&mut self) -> S::Output {
        let do_keyword_token = self.assert_token(TokenKind::Do);
        let statement_node = self.parse_statement();
        let do_while_keyword_token = self.require_while();
        let (left_paren_token, expr_node, right_paren_token) = self.parse_paren_expr();
        let do_semicolon_token = self.require_semicolon();
        self.sc_mut().make_do_statement(
            do_keyword_token,
            statement_node,
            do_while_keyword_token,
            left_paren_token,
            expr_node,
            right_paren_token,
            do_semicolon_token,
        )
    }

    fn parse_while_statement(&mut self) -> S::Output {
        let while_keyword_token = self.assert_token(TokenKind::While);
        let (left_paren_token, expr_node, right_paren_token) = self.parse_paren_expr();
        let statement_node = self.parse_statement();
        self.sc_mut().make_while_statement(
            while_keyword_token,
            left_paren_token,
            expr_node,
            right_paren_token,
            statement_node,
        )
    }

    // SPEC:
    // using-statement:
    //   await-opt   using   expression   ;
    //   await-opt   using   (   expression-list   )   compound-statement
    //
    // TODO: Update the specification of the grammar
    fn parse_using_statement(&mut self, await_kw: S::Output) -> S::Output {
        let using_kw = self.assert_token(TokenKind::Using);
        // Decision point - Are we at a function scope or a body scope
        let token_kind = self.peek_token_kind();
        // if next token is left paren it can be either
        // - parenthesized expression followed by semicolon for function scoped using
        // - comma separated list of expressions wrapped in parens for blocks.
        // To distinguish between then try parse parenthesized expression and then
        // check next token. NOTE: we should not use 'parse_expression' here
        // since it might parse (expr) { smth() } as subscript expression $expr{$index}
        //
        let mut parser1 = self.clone();
        let expr = if token_kind == TokenKind::LeftParen {
            parser1.with_expression_parser(|p: &mut ExpressionParser<'a, S>| {
                p.parse_cast_or_parenthesized_or_lambda_expression()
            })
        } else {
            parser1.parse_expression()
        };
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::Semicolon => {
                self.continue_from(parser1);
                let semi = self.sc_mut().make_token(token);
                self.sc_mut()
                    .make_using_statement_function_scoped(await_kw, using_kw, expr, semi)
            }
            _ => {
                let left_paren = self.require_left_paren();
                let expressions =
                    self.parse_comma_list(TokenKind::RightParen, Errors::error1015, |x| {
                        x.parse_expression()
                    });
                let right_paren = self.require_right_paren();
                let statements = self.parse_statement();
                self.sc_mut().make_using_statement_block_scoped(
                    await_kw,
                    using_kw,
                    left_paren,
                    expressions,
                    right_paren,
                    statements,
                )
            }
        }
    }

    fn parse_unset_statement(&mut self) -> S::Output {
        // TODO: This is listed as unsupported in Hack in the spec; is that true?
        // TODO: If it is formally supported in Hack then update the spec; if not
        // TODO: then should we make it illegal in strict mode?
        // TODO: Can the list be comma-terminated?
        // TODO: Can the list be empty?
        // TODO: The list has to be expressions which evaluate as variables;
        // add an error checking pass.
        // TODO: TokenKind::Unset is case-insentive. Should non-lowercase be an error?
        let keyword = self.assert_token(TokenKind::Unset);
        let (left_paren, variables, right_paren) =
            self.parse_parenthesized_comma_list_opt_allow_trailing(|x| x.parse_expression());
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_unset_statement(keyword, left_paren, variables, right_paren, semi)
    }

    // parses the "( expr ) statement" segment of If, Elseif or Else clauses.
    fn parse_if_body_helper(&mut self) -> (S::Output, S::Output, S::Output, S::Output) {
        let (left_paren_token, expr_node, right_paren_token) = self.parse_paren_expr();
        let statement_node = self.parse_statement();
        (
            left_paren_token,
            expr_node,
            right_paren_token,
            statement_node,
        )
    }

    // do not eat token and return Missing if first token is not Else
    fn parse_else_opt(&mut self) -> S::Output {
        let else_token = self.optional_token(TokenKind::Else);
        if else_token.is_missing() {
            else_token
        } else {
            let else_consequence = self.parse_statement();
            self.sc_mut().make_else_clause(else_token, else_consequence)
        }
    }

    fn parse_if_statement(&mut self) -> S::Output {
        // SPEC:
        // if-statement:
        //   if   (   expression   )   statement   else-clause-opt
        //
        // else-clause:
        //   else   statement
        let if_keyword_token = self.assert_token(TokenKind::If);
        let (if_left_paren, if_expr, if_right_paren, if_consequence) = self.parse_if_body_helper();
        let else_syntax = self.parse_else_opt();
        self.sc_mut().make_if_statement(
            if_keyword_token,
            if_left_paren,
            if_expr,
            if_right_paren,
            if_consequence,
            else_syntax,
        )
    }

    fn parse_switch_statement(&mut self) -> S::Output {
        // SPEC:
        //
        // The spec for switches is very simple:
        //
        // switch-statement:
        //   switch  (  expression  )  compound-statement
        // labeled-statement:
        //   case-label
        //   default-label
        // case-label:
        //   case   expression  :  statement
        // default-label:
        //   default  :  statement
        //
        // where the compound statement, if not empty, must consist of only labeled
        // statements.
        //
        // These rules give a nice simple parse but it has some unfortunate properties.
        // Consider:
        //
        // switch (foo)
        // {
        //   case 1:
        //   case 2:
        //     break;
        //   default:
        //     break;
        // }
        //
        // What's the parse of the compound statement contents based on that grammar?
        //
        // case 1:
        //   case 2:
        //     break;
        // default:
        //   break;
        //
        // That is, the second case is a child of the first. That makes it harder
        // to write analyzers, it makes it harder to write pretty printers, and so on.
        //
        // What do we really want here? We want a switch to be a collection of
        // *sections* where each section has one or more *labels* and zero or more
        // *statements*.
        //
        // switch-statement:
        //   switch  (  expression  )  { switch-sections-opt }
        //
        // switch-sections:
        //   switch-section
        //   switch-sections switch-section
        //
        // switch-section:
        //   section-labels
        //   section-statements-opt
        //   section-fallthrough-opt
        //
        // section-fallthrough:
        //   fallthrough  ;
        //
        // section-labels:
        //   section-label
        //   section-labels section-label
        //
        // section-statements:
        //   statement
        //   section-statements statement
        //
        // The parsing of course has to be greedy; we never want to say that there
        // are zero statements *between* two sections.
        //
        // TODO: Update the specification with these rules.

        let switch_keyword_token = self.assert_token(TokenKind::Switch);
        let (left_paren_token, expr_node, right_paren_token) = self.parse_paren_expr();
        let left_brace_token = self.require_left_brace();
        let section_list = {
            let list =
                self.parse_terminated_list(|x| x.parse_switch_section(), TokenKind::RightBrace);
            if list.is_missing() {
                self.with_error(Errors::empty_switch_cases, Vec::new());
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            } else {
                list
            }
        };
        let right_brace_token = self.require_right_brace();
        self.sc_mut().make_switch_statement(
            switch_keyword_token,
            left_paren_token,
            expr_node,
            right_paren_token,
            left_brace_token,
            section_list,
            right_brace_token,
        )
    }

    fn is_switch_fallthrough(&self) -> bool {
        self.peek_token_kind() == TokenKind::Fallthrough
            && self.peek_token_kind_with_lookahead(1) == TokenKind::Semicolon
    }

    fn parse_possible_erroneous_fallthrough(&mut self) -> S::Output {
        if self.is_switch_fallthrough() {
            self.with_error_on_whole_token(Errors::error1055, Vec::new());
            self.parse_switch_fallthrough()
        } else {
            self.parse_expression_statement()
        }
    }

    fn parse_switch_fallthrough(&mut self) -> S::Output {
        // We don't get here unless we have fallthrough ;
        let keyword = self.assert_token(TokenKind::Fallthrough);
        let semi = self.assert_token(TokenKind::Semicolon);
        self.sc_mut().make_switch_fallthrough(keyword, semi)
    }

    fn parse_switch_fallthrough_opt(&mut self) -> S::Output {
        if self.is_switch_fallthrough() {
            self.parse_switch_fallthrough()
        } else {
            // As long as we have FALLTHROUGH comments, insert a faux-statement as if
            // there was a fallthrough statement. For example, the code
            //
            // > case 22:
            // >   $x = 0;
            // >   // FALLTHROUGH because we want all the other functionality as well
            // > case 42:
            // >   foo($x);
            // >   break;
            //
            // Should be parsed as if it were
            //
            // > case 22:
            // >   $x = 0;
            // >   // FALLTHROUGH because we want all the other functionality as well
            // >   fallthrough;
            // > case 43:
            // >   foo($x);
            // >   break;
            //
            // But since we have no actual occurrence (i.e. no position, no string) of
            // that `fallthrough;` statement, we construct a `switch_fallthrough`, but
            // fill it with `missing`.
            let next = self.peek_token();
            let commented_fallthrough = next.has_leading_trivia_kind(TriviaKind::FallThrough);
            let pos = self.pos();
            let missing = self.sc_mut().make_missing(pos);
            if commented_fallthrough {
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                self.sc_mut().make_switch_fallthrough(missing, missing1)
            } else {
                missing
            }
        }
    }

    fn parse_switch_section(&mut self) -> S::Output {
        // See parse_switch_statement for grammar
        let labels = self.parse_list_until_none(|x| x.parse_switch_section_label());
        if labels.is_missing() {
            self.with_error(Errors::error2008, Vec::new());
        };
        let statements = self.parse_list_until_none(|x| x.parse_switch_section_statement());
        let fallthrough = self.parse_switch_fallthrough_opt();
        self.sc_mut()
            .make_switch_section(labels, statements, fallthrough)
    }

    fn parse_switch_section_statement(&mut self) -> Option<S::Output> {
        if self.is_switch_fallthrough() {
            None
        } else {
            match self.peek_token_kind() {
                TokenKind::Default
                | TokenKind::Case
                | TokenKind::RightBrace
                | TokenKind::EndOfFile => None,
                _ => {
                    let statement = self.parse_statement();
                    Some(statement)
                }
            }
        }
    }

    fn parse_switch_section_label(&mut self) -> Option<S::Output> {
        // See the grammar under parse_switch_statement
        match self.peek_token_kind() {
            TokenKind::Case => {
                let label = self.parse_case_label();
                Some(label)
            }
            TokenKind::Default => {
                let label = self.parse_default_label();
                Some(label)
            }
            _ => None,
        }
    }

    fn parse_match_statement(&mut self) -> S::Output {
        // SPEC:
        //
        // match-statement:
        //   match  (  expression  )  {  match-statement-arms-opt  }
        //
        // match-statement-arms:
        //   match-statement-arm
        //   match-statement-arms match-statement-arm

        let match_keyword_token = self.assert_token(TokenKind::Match);
        let (left_paren_token, expr_node, right_paren_token) = self.parse_paren_expr();
        let left_brace_token = self.require_left_brace();
        let match_arms =
            self.parse_terminated_list(|x| x.parse_match_statement_arm(), TokenKind::RightBrace);
        if match_arms.is_missing() {
            self.with_error(Errors::empty_match_statement, Vec::new());
        }
        let right_brace_token = self.require_right_brace();
        self.sc_mut().make_match_statement(
            match_keyword_token,
            left_paren_token,
            expr_node,
            right_paren_token,
            left_brace_token,
            match_arms,
            right_brace_token,
        )
    }

    fn parse_match_statement_arm(&mut self) -> S::Output {
        // SPEC:
        //
        // match-statement-arm:
        //   pattern  =>  statement
        //
        // We parse any statement here, but only compound statements are
        // currently permitted. We emit this error in a later pass.

        if self.peek_token_kind() == TokenKind::Case {
            self.skip_and_log_unexpected_token(/* generate_error = */ true);
        }
        let pattern = self.parse_pattern();
        let arrow_token = self.require_token(TokenKind::EqualGreaterThan, Errors::error1028);
        let body = self.parse_statement();
        self.sc_mut()
            .make_match_statement_arm(pattern, arrow_token, body)
    }

    fn parse_catch_clause_opt(&mut self) -> Option<S::Output> {
        // SPEC
        // catch  (  type-specification-opt variable-name  )  compound-statement
        if self.peek_token_kind() == TokenKind::Catch {
            let catch_token = self.assert_token(TokenKind::Catch);
            let left_paren = self.require_left_paren();
            let catch_type = match self.peek_token_kind() {
                TokenKind::Variable => {
                    self.with_error(Errors::error1007, Vec::new());
                    let pos = self.pos();
                    self.sc_mut().make_missing(pos)
                }
                _ => self.parse_type_specifier(),
            };
            let catch_var = self.require_variable();

            let right_paren = self.require_right_paren();
            let compound_stmt = self.parse_compound_statement();
            let catch_clause = self.sc_mut().make_catch_clause(
                catch_token,
                left_paren,
                catch_type,
                catch_var,
                right_paren,
                compound_stmt,
            );
            Some(catch_clause)
        } else {
            None
        }
    }

    fn parse_finally_clause_opt(&mut self) -> S::Output {
        // SPEC
        // finally-clause:
        //   finally   compound-statement
        if self.peek_token_kind() == TokenKind::Finally {
            let finally_token = self.assert_token(TokenKind::Finally);
            let compound_stmt = self.parse_compound_statement();
            self.sc_mut()
                .make_finally_clause(finally_token, compound_stmt)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_try_statement(&mut self) -> S::Output {
        // SPEC:
        // try-statement:
        //   try  compound-statement   catch-clauses
        //   try  compound-statement   finally-clause
        //   try  compound-statement   catch-clauses   finally-clause
        let try_keyword_token = self.assert_token(TokenKind::Try);
        let try_compound_stmt = self.parse_compound_statement();
        let catch_clauses = self.parse_list_until_none(|x| x.parse_catch_clause_opt());
        let finally_clause = self.parse_finally_clause_opt();
        // If the catch and finally are both missing then we give an error in
        // a later pass.
        self.sc_mut().make_try_statement(
            try_keyword_token,
            try_compound_stmt,
            catch_clauses,
            finally_clause,
        )
    }

    fn parse_break_statement(&mut self) -> S::Output {
        // SPEC
        // break-statement:
        //   break  ;

        // We detect if we are not inside a switch or loop in a later pass.
        let break_token = self.assert_token(TokenKind::Break);
        let semi_token = self.require_semicolon();
        self.sc_mut().make_break_statement(break_token, semi_token)
    }

    fn parse_continue_statement(&mut self) -> S::Output {
        // SPEC
        // continue-statement:
        //   continue  ;

        // We detect if we are not inside a loop in a later pass.
        let continue_token = self.assert_token(TokenKind::Continue);
        let semi_token = self.require_semicolon();
        self.sc_mut()
            .make_continue_statement(continue_token, semi_token)
    }

    fn parse_return_statement(&mut self) -> S::Output {
        let return_token = self.assert_token(TokenKind::Return);
        if self.peek_token_kind() == TokenKind::Semicolon {
            let pos = self.pos();
            let missing = self.sc_mut().make_missing(pos);
            let semi_token = self.next_token();
            let semi_token = self.sc_mut().make_token(semi_token);
            self.sc_mut()
                .make_return_statement(return_token, missing, semi_token)
        } else {
            let expr = self.parse_expression();
            let semi_token = self.require_semicolon();
            self.sc_mut()
                .make_return_statement(return_token, expr, semi_token)
        }
    }

    fn parse_throw_statement(&mut self) -> S::Output {
        let throw_token = self.assert_token(TokenKind::Throw);
        let expr = self.parse_expression();
        let semi_token = self.require_semicolon();
        self.sc_mut()
            .make_throw_statement(throw_token, expr, semi_token)
    }

    fn parse_yield_break_statement(&mut self) -> S::Output {
        let yield_token = self.assert_token(TokenKind::Yield);
        let break_token = self.assert_token(TokenKind::Break);
        let semi_token = self.require_semicolon();
        self.sc_mut()
            .make_yield_break_statement(yield_token, break_token, semi_token)
    }

    fn parse_simple_initializer_opt(&mut self) -> S::Output {
        if let TokenKind::Equal = self.peek_token_kind() {
            let token = self.assert_token(TokenKind::Equal);
            let init = self.parse_expression();
            self.sc_mut().make_simple_initializer(token, init)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_declare_local_statement(&mut self) -> S::Output {
        let let_token = self.assert_token(TokenKind::Let);
        let variable = self.parse_expression();
        let colon_token = self.require_colon();
        let hint =
            self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_type_specifier(true, true));
        let simple_init = self.parse_simple_initializer_opt();
        let semi_token = self.require_semicolon();
        self.sc_mut().make_declare_local_statement(
            let_token,
            variable,
            colon_token,
            hint,
            simple_init,
            semi_token,
        )
    }

    fn parse_default_label(&mut self) -> S::Output {
        //
        // See comments under parse_switch_statement for the grammar.
        // TODO: Update the spec.
        // TODO: The spec is wrong; it implies that a statement must always follow
        //       the default:, but in fact
        //       switch($x) { default: }
        //       is legal. Fix the spec.
        // TODO: PHP allows a default to end in a semi; Hack does not.  We allow a semi
        // here; add an error in a later pass.
        let default_token = self.assert_token(TokenKind::Default);
        let colon_token = {
            if self.peek_token_kind() == TokenKind::Semicolon {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            } else {
                self.require_colon()
            }
        };
        self.sc_mut().make_default_label(default_token, colon_token)
    }
    fn parse_case_label(&mut self) -> S::Output {
        // SPEC:
        // See comments under parse_switch_statement for the grammar.
        // TODO: The spec is wrong; it implies that a statement must always follow
        //       the case, but in fact
        //       switch($x) { case 10: }
        //       is legal. Fix the spec.
        // TODO: PHP allows a case to end in a semi; Hack does not.  We allow a semi
        // here; add an error in a later pass.

        let case_token = self.assert_token(TokenKind::Case);
        let expr = self.parse_expression();
        let colon_token = {
            if self.peek_token_kind() == TokenKind::Semicolon {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            } else {
                self.require_colon()
            }
        };
        self.sc_mut().make_case_label(case_token, expr, colon_token)
    }

    fn parse_concurrent_statement(&mut self) -> S::Output {
        let keyword = self.assert_token(TokenKind::Concurrent);
        let statement = self.parse_statement();
        self.sc_mut().make_concurrent_statement(keyword, statement)
    }

    // SPEC:
    // TODO: update the spec to reflect that echo and print must be a statement
    // echo-intrinsic:
    //   echo  expression
    //   echo  (  expression  )
    //   echo  expression-list-two-or-more
    //
    // expression-list-two-or-more:
    //   expression  ,  expression
    //   expression-list-two-or-more  ,  expression

    fn parse_echo_statement(&mut self) -> S::Output {
        let token = self.assert_token(TokenKind::Echo);
        let expression_list = self.parse_comma_list(TokenKind::Semicolon, Errors::error1015, |x| {
            x.parse_expression()
        });
        let semicolon = self.require_semicolon();
        self.sc_mut()
            .make_echo_statement(token, expression_list, semicolon)
    }

    fn parse_expression_statement(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::Semicolon => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                let token = self.next_token();
                let token = self.sc_mut().make_token(token);
                self.sc_mut().make_expression_statement(missing, token)
            }
            _ => {
                self.expect_in_new_scope(ExpectedTokens::Semicolon);

                // Detect common patterns of users trying to put type
                // annotations on local variables.
                let saw_type_name = match self.peek_token_kind() {
                    // User-defined types.
                    TokenKind::Name => true,
                    // Common reserved names that are types.
                    TokenKind::Darray => true,
                    TokenKind::Dict => true,
                    TokenKind::Int => true,
                    TokenKind::Keyset => true,
                    TokenKind::Shape => true,
                    TokenKind::String => true,
                    TokenKind::Varray => true,
                    TokenKind::Vec => true,
                    _ => false,
                };

                let expression = self.parse_expression();

                let token = match self.require_semicolon_token(saw_type_name) {
                    Some(t) => self.sc_mut().make_token(t),
                    None => {
                        let pos = self.pos();
                        self.sc_mut().make_missing(pos)
                    }
                };
                self.pop_scope(ExpectedTokens::Semicolon);
                self.sc_mut().make_expression_statement(expression, token)
            }
        }
    }

    pub fn parse_compound_statement(&mut self) -> S::Output {
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::Semicolon => {
                self.continue_from(parser1);
                self.sc_mut().make_token(token)
            }
            _ => {
                let left_brace_token = self.require_left_brace();
                let statement_list =
                    self.parse_terminated_list(|x| x.parse_statement(), TokenKind::RightBrace);
                let right_brace_token = self.require_right_brace();
                self.sc_mut().make_compound_statement(
                    left_brace_token,
                    statement_list,
                    right_brace_token,
                )
            }
        }
    }

    fn parse_expression(&mut self) -> S::Output {
        self.with_expression_parser(|p: &mut ExpressionParser<'a, S>| p.parse_expression())
    }

    fn parse_pattern(&mut self) -> S::Output {
        self.with_pattern_parser(|p: &mut PatternParser<'a, S>| p.parse_pattern())
    }
}
