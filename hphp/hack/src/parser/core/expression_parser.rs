// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::marker::PhantomData;

use crate::declaration_parser::DeclarationParser;
use crate::lexer::{Lexer, StringLiteralKind};
use crate::operator::{Assoc, Operator};
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::{NodeType, SmartConstructors, Token};
use crate::statement_parser::StatementParser;
use crate::type_parser::TypeParser;
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::syntax_error::{self as Errors, SyntaxError};
use parser_core_types::token_factory::TokenFactory;
use parser_core_types::token_kind::TokenKind;

#[derive(PartialEq)]
pub enum BinaryExpressionPrefixKind<P> {
    PrefixAssignment,
    PrefixLessThan(P),
    PrefixNone,
}

impl<P> BinaryExpressionPrefixKind<P> {
    pub fn is_assignment(&self) -> bool {
        match self {
            BinaryExpressionPrefixKind::PrefixAssignment => true,
            BinaryExpressionPrefixKind::PrefixNone => false,
            BinaryExpressionPrefixKind::PrefixLessThan(_) => false,
        }
    }
}

pub struct ExpressionParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    lexer: Lexer<'a, S::TF>,
    env: ParserEnv,
    context: Context<'a, Token<S>>,
    errors: Vec<SyntaxError>,
    sc: S,
    precedence: usize,
    allow_as_expressions: bool,
    in_expression_tree: bool,
    _phantom: PhantomData<S>,
}

impl<'a, S> std::clone::Clone for ExpressionParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn clone(&self) -> Self {
        Self {
            lexer: self.lexer.clone(),
            context: self.context.clone(),
            env: self.env.clone(),
            errors: self.errors.clone(),
            sc: self.sc.clone(),
            precedence: self.precedence,
            _phantom: self._phantom,
            allow_as_expressions: self.allow_as_expressions,
            in_expression_tree: self.in_expression_tree,
        }
    }
}

impl<'a, S> ParserTrait<'a, S> for ExpressionParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn make(
        lexer: Lexer<'a, S::TF>,
        env: ParserEnv,
        context: Context<'a, Token<S>>,
        errors: Vec<SyntaxError>,
        sc: S,
    ) -> Self {
        Self {
            lexer,
            env,
            precedence: 0,
            context,
            errors,
            sc,
            allow_as_expressions: true,
            in_expression_tree: false,
            _phantom: PhantomData,
        }
    }

    fn into_parts(self) -> (Lexer<'a, S::TF>, Context<'a, Token<S>>, Vec<SyntaxError>, S) {
        (self.lexer, self.context, self.errors, self.sc)
    }

    fn lexer(&self) -> &Lexer<'a, S::TF> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::TF> {
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

    fn drain_skipped_tokens(&mut self) -> std::vec::Drain<Token<S>> {
        self.context.skipped_tokens.drain(..)
    }

    fn skipped_tokens(&self) -> &[Token<S>] {
        &self.context.skipped_tokens
    }

    fn context_mut(&mut self) -> &mut Context<'a, Token<S>> {
        &mut self.context
    }

    fn context(&self) -> &Context<'a, Token<S>> {
        &self.context
    }
}

impl<'a, S> ExpressionParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn allow_as_expressions(&self) -> bool {
        self.allow_as_expressions
    }

    fn in_expression_tree(&self) -> bool {
        self.in_expression_tree
    }

    pub fn with_as_expressions<F, U>(&mut self, enabled: bool, f: F) -> U
    where
        F: Fn(&mut Self) -> U,
    {
        let old_enabled = self.allow_as_expressions();
        self.allow_as_expressions = enabled;
        let r = f(self);
        self.allow_as_expressions = old_enabled;
        r
    }

    fn with_type_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut TypeParser<'a, S>) -> U,
    {
        let mut type_parser: TypeParser<S> = TypeParser::make(
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

    fn parse_remaining_type_specifier(&mut self, name: S::R) -> S::R {
        let mut type_parser: TypeParser<S> = TypeParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = type_parser.parse_remaining_type_specifier(name);
        self.continue_from(type_parser);
        res
    }

    fn parse_generic_type_arguments(&mut self) -> (S::R, bool) {
        self.with_type_parser(|x| x.parse_generic_type_argument_list())
    }

    fn with_decl_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut DeclarationParser<'a, S>) -> U,
    {
        let mut decl_parser: DeclarationParser<S> = DeclarationParser::make(
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

    fn with_statement_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut StatementParser<'a, S>) -> U,
    {
        let mut statement_parser: StatementParser<S> = StatementParser::make(
            self.lexer.clone(),
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut statement_parser);
        self.continue_from(statement_parser);
        res
    }

    fn parse_compound_statement(&mut self) -> S::R {
        self.with_statement_parser(|x| x.parse_compound_statement())
    }

    fn parse_parameter_list_opt(&mut self) -> (S::R, S::R, S::R) {
        self.with_decl_parser(|x| x.parse_parameter_list_opt())
    }

    pub fn parse_expression(&mut self) -> S::R {
        let term = self.parse_term();
        self.parse_remaining_expression(term)
    }

    fn with_reset_precedence<U, F: FnOnce(&mut Self) -> U>(&mut self, parse_function: F) -> U {
        self.with_numeric_precedence(0, parse_function)
    }

    fn parse_expression_with_reset_precedence(&mut self) -> S::R {
        self.with_reset_precedence(|x| x.parse_expression())
    }

    fn parse_expression_with_operator_precedence(&mut self, operator: Operator) -> S::R {
        self.with_operator_precedence(operator, |x| x.parse_expression())
    }

    fn with_precedence(&mut self, precedence: usize) {
        self.precedence = precedence
    }

    fn with_numeric_precedence<U, F: FnOnce(&mut Self) -> U>(
        &mut self,
        new_precedence: usize,
        parse_function: F,
    ) -> U {
        let old_precedence = self.precedence;
        self.with_precedence(new_precedence);
        let result = parse_function(self);
        self.with_precedence(old_precedence);
        result
    }

    fn with_operator_precedence<F: FnOnce(&mut Self) -> S::R>(
        &mut self,
        operator: Operator,
        parse_function: F,
    ) -> S::R {
        let new_precedence = operator.precedence(&self.env);
        self.with_numeric_precedence(new_precedence, parse_function)
    }

    fn parse_as_name_or_error(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_token_non_reserved_as_name();
        match token.kind() {
            TokenKind::Namespace | TokenKind::Name => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                let name = self.scan_remaining_qualified_name(token);
                self.parse_name_or_collection_literal_expression(name)
            }
            kind if self.expects_here(kind) => {
                // ERROR RECOVERY: If we're encountering a token that matches a kind in
                // the previous scope of the expected stack, don't eat it--just mark the
                // name missing and continue parsing, starting from the offending token.
                self.with_error(Errors::error1015);
                S!(make_missing, self, self.pos())
            }
            _ => {
                self.continue_from(parser1);
                // ERROR RECOVERY: If we're encountering anything other than a TokenKind::Name
                // or the next expected kind, eat the offending token.
                // TODO: Increase the coverage of PrecedenceParser.expects_next, so that
                // we wind up eating fewer of the tokens that'll be needed by the outer
                // statement / declaration parsers.
                self.with_error(Errors::error1015);
                S!(make_token, self, token)
            }
        }
    }

    fn parse_term(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_class_name_or_other_token();
        let allow_new_attr = self.env.allow_new_attribute_syntax;
        match token.kind() {
            TokenKind::DecimalLiteral
            | TokenKind::OctalLiteral
            | TokenKind::HexadecimalLiteral
            | TokenKind::BinaryLiteral
            | TokenKind::FloatingLiteral
            | TokenKind::SingleQuotedStringLiteral
            | TokenKind::NowdocStringLiteral
            | TokenKind::DoubleQuotedStringLiteral
            | TokenKind::BooleanLiteral
            | TokenKind::NullLiteral => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                S!(make_literal_expression, self, token)
            }
            TokenKind::HeredocStringLiteral => {
                // We have a heredoc string literal but it might contain embedded
                // expressions. Start over.
                let (token, name) = self.next_docstring_header();
                self.parse_heredoc_string(token, name)
            }
            TokenKind::HeredocStringLiteralHead | TokenKind::DoubleQuotedStringLiteralHead => {
                self.continue_from(parser1);
                self.parse_double_quoted_like_string(token, StringLiteralKind::LiteralDoubleQuoted)
            }
            TokenKind::Variable => self.parse_variable_or_lambda(),
            TokenKind::XHPClassName => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                self.parse_name_or_collection_literal_expression(token)
            }
            TokenKind::Name => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                let qualified_name = self.scan_remaining_qualified_name(token);
                let mut parser1 = self.clone();
                let str_maybe = parser1.next_token_no_trailing();
                match str_maybe.kind() {
                    TokenKind::NowdocStringLiteral | TokenKind::HeredocStringLiteral => {
                        // for now, try generic type argument list with attributes before resorting to bad prefix
                        match self.try_parse_specified_function_call(&qualified_name) {
                            Some((type_arguments, p)) => {
                                self.continue_from(p);
                                self.do_parse_specified_function_call(
                                    qualified_name,
                                    type_arguments,
                                )
                            }
                            _ => {
                                self.with_error(Errors::prefixed_invalid_string_kind);
                                self.parse_name_or_collection_literal_expression(qualified_name)
                            }
                        }
                    }
                    TokenKind::HeredocStringLiteralHead => {
                        // Treat as an attempt to prefix a non-double-quoted string
                        self.with_error(Errors::prefixed_invalid_string_kind);
                        self.parse_name_or_collection_literal_expression(qualified_name)
                    }
                    TokenKind::SingleQuotedStringLiteral | TokenKind::DoubleQuotedStringLiteral => {
                        // This name prefixes a double-quoted string or a single
                        // quoted string
                        self.continue_from(parser1);
                        let str_ = S!(make_token, self, str_maybe);
                        let str_ = S!(make_literal_expression, self, str_);
                        S!(make_prefixed_string_expression, self, qualified_name, str_)
                    }
                    TokenKind::DoubleQuotedStringLiteralHead => {
                        self.continue_from(parser1);
                        // This name prefixes a double-quoted string containing embedded expressions
                        let str_ = self.parse_double_quoted_like_string(
                            str_maybe,
                            StringLiteralKind::LiteralDoubleQuoted,
                        );
                        S!(make_prefixed_string_expression, self, qualified_name, str_)
                    }
                    _ => {
                        // Not a prefixed string or an attempt at one
                        self.parse_name_or_collection_literal_expression(qualified_name)
                    }
                }
            }
            TokenKind::Backslash => {
                self.continue_from(parser1);
                let missing = S!(make_missing, self, self.pos());
                let backslash = S!(make_token, self, token);

                let qualified_name = self.scan_qualified_name(missing, backslash);
                self.parse_name_or_collection_literal_expression(qualified_name)
            }
            TokenKind::SelfToken | TokenKind::Parent => self.parse_scope_resolution_or_name(),
            TokenKind::Static => self.parse_anon_or_awaitable_or_scope_resolution_or_name(),
            TokenKind::Yield => self.parse_yield_expression(),
            TokenKind::Dollar => self.parse_dollar_expression(true),
            TokenKind::Exclamation
            | TokenKind::PlusPlus
            | TokenKind::MinusMinus
            | TokenKind::Tilde
            | TokenKind::Minus
            | TokenKind::Plus
            | TokenKind::Await
            | TokenKind::Readonly
            | TokenKind::Clone
            | TokenKind::Print => self.parse_prefix_unary_expression(),
            // Allow error suppression prefix when not using new attributes
            TokenKind::At if !allow_new_attr => self.parse_prefix_unary_expression(),
            TokenKind::LeftParen => self.parse_cast_or_parenthesized_or_lambda_expression(),
            TokenKind::LessThan => {
                self.continue_from(parser1);
                self.parse_possible_xhp_expression(/*in_xhp_body:*/ false, token)
            }
            TokenKind::List => self.parse_list_expression(),
            TokenKind::New => self.parse_object_creation_expression(),
            TokenKind::Varray => self.parse_varray_intrinsic_expression(),
            TokenKind::Vec => self.parse_vector_intrinsic_expression(),
            TokenKind::Darray => self.parse_darray_intrinsic_expression(),
            TokenKind::Dict => self.parse_dictionary_intrinsic_expression(),
            TokenKind::Keyset => self.parse_keyset_intrinsic_expression(),
            TokenKind::Tuple => self.parse_tuple_expression(),
            TokenKind::Shape => self.parse_shape_expression(),
            TokenKind::Function => {
                let attribute_spec = S!(make_missing, self, self.pos());
                self.parse_anon(attribute_spec)
            }
            TokenKind::DollarDollar => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                S!(make_pipe_variable_expression, self, token)
            }
            // LessThanLessThan start attribute spec that is allowed on anonymous
            // functions or lambdas
            TokenKind::LessThanLessThan | TokenKind::Async => {
                self.parse_anon_or_lambda_or_awaitable()
            }
            TokenKind::At if allow_new_attr => self.parse_anon_or_lambda_or_awaitable(),
            TokenKind::Include
            | TokenKind::Include_once
            | TokenKind::Require
            | TokenKind::Require_once => self.parse_inclusion_expression(),
            TokenKind::Isset => self.parse_isset_expression(),
            TokenKind::Eval => self.parse_eval_expression(),
            TokenKind::Hash => {
                let qualifier = S!(make_missing, self, self.pos());
                self.parse_enum_class_label(qualifier)
            }
            TokenKind::Empty => {
                self.with_error(Errors::empty_expression_illegal);
                let token = self.next_token_non_reserved_as_name();
                S!(make_token, self, token)
            }
            kind if self.expects(kind) => {
                // ERROR RECOVERY: if we've prematurely found a token we're expecting
                // later, mark the expression missing, throw an error, and do not advance
                // the parser.
                self.with_error(Errors::error1015);
                S!(make_missing, self, self.pos())
            }
            TokenKind::EndOfFile | _ => self.parse_as_name_or_error(),
        }
    }

    fn parse_eval_expression(&mut self) -> S::R {
        // TODO: This is a PHP-ism. Open questions:
        // * Should we allow a trailing comma? it is not a function call and
        //   never has more than one argument. See D4273242 for discussion.
        // * Is there any restriction on the kind of expression this can be?
        // * Should this be an error in strict mode?
        // * Should this be in the specification?
        // * Eval is case-insensitive. Should use of non-lowercase be an error?
        //
        // TODO: The original Hack and HHVM parsers accept "eval" as an
        // identifier, so we do too; consider whether it should be reserved.
        let mut parser1 = self.clone();
        let keyword = parser1.assert_token(TokenKind::Eval);
        if parser1.peek_token_kind() == TokenKind::LeftParen {
            self.continue_from(parser1);
            let left = self.assert_token(TokenKind::LeftParen);
            let arg = self.parse_expression_with_reset_precedence();
            let right = self.require_right_paren();
            S!(make_eval_expression, self, keyword, left, arg, right)
        } else {
            self.parse_as_name_or_error()
        }
    }

    fn parse_isset_expression(&mut self) -> S::R {
        // TODO: This is a PHP-ism. Open questions:
        // * Should we allow a trailing comma? See D4273242 for discussion.
        // * Is there any restriction on the kind of expression the arguments can be?
        // * Should this be an error in strict mode?
        // * Should this be in the specification?
        // * PHP requires that there be at least one argument; should we require
        //   that? if so, should we give the error in the parser or a later pass?
        // * Isset is case-insensitive. Should use of non-lowercase be an error?
        //
        // TODO: The original Hack and HHVM parsers accept "isset" as an
        // identifier, so we do too; consider whether it should be reserved.
        let mut parser1 = self.clone();
        let keyword = parser1.assert_token(TokenKind::Isset);
        if parser1.peek_token_kind() == TokenKind::LeftParen {
            self.continue_from(parser1);
            let (left, args, right) = self.parse_expression_list_opt();
            S!(make_isset_expression, self, keyword, left, args, right)
        } else {
            self.parse_as_name_or_error()
        }
    }

    fn parse_double_quoted_like_string(
        &mut self,
        head: Token<S>,
        literal_kind: StringLiteralKind,
    ) -> S::R {
        self.parse_string_literal(head, literal_kind)
    }

    fn parse_heredoc_string(&mut self, head: Token<S>, name: &[u8]) -> S::R {
        self.parse_string_literal(
            head,
            StringLiteralKind::LiteralHeredoc {
                heredoc: name.to_vec(),
            },
        )
    }

    fn parse_braced_expression_in_string(
        &mut self,
        left_brace: Token<S>,
        dollar_inside_braces: bool,
    ) -> S::R {
        // We are parsing something like "abc{$x}def" or "abc${x}def", and we
        // are at the left brace.
        //
        // We know that the left brace will not be preceded by trivia. However in the
        // second of the two cases mentioned above it is legal for there to be trivia
        // following the left brace. If we are in the first case, we've already
        // verified that there is no trailing trivia after the left brace.
        //
        // The expression may be followed by arbitrary trivia, including
        // newlines and comments. That means that the closing brace may have
        // leading trivia. But under no circumstances does the closing brace have
        // trailing trivia.
        //
        // It's an error for the closing brace to be missing.
        //
        // Therefore we lex the left brace normally, parse the expression normally,
        // but require that there be a right brace. We do not lex the trailing trivia
        // on the right brace.
        //
        // ERROR RECOVERY: If the right brace is missing, treat the remainder as
        // string text.

        let is_assignment_op = |token| {
            Operator::is_trailing_operator_token(token)
                && Operator::trailing_from_token(token).is_assignment()
        };

        let left_brace_trailing_is_empty = left_brace.trailing_is_empty();
        let left_brace = S!(make_token, self, left_brace);
        let mut parser1 = self.clone();
        let name_or_keyword_as_name = parser1.next_token_as_name();
        let after_name = parser1.next_token_no_trailing();
        let (expr, right_brace) = match (name_or_keyword_as_name.kind(), after_name.kind()) {
            (TokenKind::Name, TokenKind::RightBrace) => {
                self.continue_from(parser1);
                let expr = S!(make_token, self, name_or_keyword_as_name);
                let right_brace = S!(make_token, self, after_name);
                (expr, right_brace)
            }
            (TokenKind::Name, TokenKind::LeftBracket)
                if !dollar_inside_braces
                    && left_brace_trailing_is_empty
                    && name_or_keyword_as_name.leading_is_empty()
                    && name_or_keyword_as_name.trailing_is_empty() =>
            {
                // The case of "${x}" should be treated as if we were interpolating $x
                // (rather than interpolating the constant `x`).
                //
                // But we can also put other expressions in between the braces, such as
                // "${foo()}". In that case, `foo()` is evaluated, and then the result is
                // used as the variable name to interpolate.
                //
                // Considering that both start with `${ident`, how does the parser tell the
                // difference? It appears that PHP special-cases two forms to be treated as
                // direct variable interpolation:
                //
                // 1) `${x}` is semantically the same as `{$x}`.
                //
                //    No whitespace may come between `{` and `x`, or else the `x` is
                //    treated as a constant.
                //
                // 2) `${x[expr()]}` should be treated as `{$x[expr()]}`. More than one
                //    subscript expression, such as `${x[expr1()][expr2()]}`, is illegal.
                //
                //    No whitespace may come between either the `{` and `x` or the `x` and
                //    the `[`, or else the `x` is treated as a constant, and therefore
                //    arbitrary expressions are allowed in the curly braces. (This amounts
                //    to a variable-variable.)
                //
                // This is very similar to the grammar detailed in the specification
                // discussed in `parse_string_literal` below, except that `${x=>y}` is not
                // valid; it appears to be treated the same as performing member access on
                // the constant `x` rather than the variable `$x`, which is not valid
                // syntax.
                //
                // The first case can already be parsed successfully because `x` is a valid
                // expression, so we special-case only the second case here.
                self.continue_from(parser1);
                let receiver = S!(make_token, self, name_or_keyword_as_name);
                let left_bracket = S!(make_token, self, after_name);
                let index = self.parse_expression_with_reset_precedence();
                let right_bracket = self.require_right_bracket();
                let expr = S!(
                    make_subscript_expression,
                    self,
                    receiver,
                    left_bracket,
                    index,
                    right_bracket
                );

                let mut parser1 = self.clone();
                let right_brace = parser1.next_token_no_trailing();
                let right_brace = if right_brace.kind() == TokenKind::RightBrace {
                    self.continue_from(parser1);
                    S!(make_token, self, right_brace)
                } else {
                    self.with_error(Errors::error1006);
                    S!(make_missing, self, self.pos())
                };
                (expr, right_brace)
            }
            (TokenKind::Name, maybe_assignment_op) if is_assignment_op(maybe_assignment_op) => {
                // PHP compatibility: expressions like `${x + 1}` are okay, but
                // expressions like `${x = 1}` are not okay, since `x` is parsed as if it
                // were a constant, and you can't use an assignment operator with a
                // constant. Flag the issue by reporting that a right brace is expected.
                self.continue_from(parser1);
                let expr = S!(make_token, self, name_or_keyword_as_name);
                let mut parser1 = self.clone();
                let right_brace = parser1.next_token_no_trailing();
                let right_brace = if right_brace.kind() == TokenKind::RightBrace {
                    self.continue_from(parser1);
                    S!(make_token, self, right_brace)
                } else {
                    self.with_error(Errors::error1006);
                    S!(make_missing, self, self.pos())
                };
                (expr, right_brace)
            }
            (_, _) => {
                let start_offset = self.lexer().start();
                let expr = self.parse_expression_with_reset_precedence();
                let end_offset = self.lexer().start();

                // PHP compatibility: only allow a handful of expression types in
                // {$...}-expressions.
                if dollar_inside_braces
                    && !(expr.is_function_call_expression()
                || expr.is_subscript_expression()
                || expr.is_member_selection_expression()
                || expr.is_safe_member_selection_expression()
                || expr.is_variable_expression()
                // This is actually checking to see if we have a
                // variable-variable, which is allowed here. Variable-variables are
                // parsed as prefix unary expressions with `$` as the operator. We
                // cannot directly check the operator in this prefix unary
                // expression, but we already know that `dollar_inside_braces` is
                // true, so that operator must have been `$`.
                ||  expr.is_prefix_unary_expression())
                {
                    let error = SyntaxError::make(
                        start_offset,
                        end_offset,
                        Errors::illegal_interpolated_brace_with_embedded_dollar_expression,
                    );
                    self.add_error(error);
                };

                let mut parser1 = self.clone();
                let token = parser1.next_token_no_trailing();
                let right_brace = if token.kind() == TokenKind::RightBrace {
                    self.continue_from(parser1);
                    S!(make_token, self, token)
                } else {
                    self.with_error(Errors::error1006);
                    S!(make_missing, self, self.pos())
                };
                (expr, right_brace)
            }
        };
        S!(
            make_embedded_braced_expression,
            self,
            left_brace,
            expr,
            right_brace
        )
    }

    fn parse_string_literal(&mut self, head: Token<S>, literal_kind: StringLiteralKind) -> S::R {
        // SPEC
        //
        // Double-quoted string literals and heredoc string literals use basically
        // the same rules; here we have just the grammar for double-quoted string
        // literals.
        //
        // string-variable::
        // variable-name   offset-or-property-opt
        //
        // offset-or-property::
        // offset-in-string
        // property-in-string
        //
        // offset-in-string::
        // [   name   ]
        // [   variable-name   ]
        // [   integer-literal   ]
        //
        // property-in-string::
        // ->   name
        //
        // TODO: What about ?->
        //
        // The actual situation is considerably more complex than indicated
        // in the specification.
        //
        // TODO: Consider updating the specification.
        //
        // The tokens in the grammar above have no leading or trailing trivia.
        //
        // An embedded variable expression may also be enclosed in curly braces;
        // however, the $ of the variable expression must follow immediately after
        // the left brace.
        //
        // An embedded variable expression inside braces allows trivia between
        // the tokens and before the right brace.
        //
        // An embedded variable expression inside braces can be a much more complex
        // expression than indicated by the grammar above.  For example,
        // {$c->x->y[0]} is good, and {$c[$x is foo ? 0 : 1]} is good,
        // but {$c is foo ? $x : $y} is not.  It is not clear to me what
        // the legal grammar here is; it seems best in this situation to simply
        // parse any expression and do an error pass later.
        //
        // Note that the braced expressions can include double-quoted strings.
        // {$c["abc"]} is good, for instance.
        //
        // ${ is illegal in strict mode. In non-strict mode, ${varname is treated
        // the same as {$varname, and may be an arbitrary expression.
        //
        // TODO: We need to produce errors if there are unbalanced brackets,
        // example: "$x[0" is illegal.
        //
        // TODO: Similarly for any non-valid thing following the left bracket,
        // including trivia. example: "$x[  0]" is illegal.
        //
        //

        let merge = |parser: &mut Self, token: Token<S>, head: Option<Token<S>>| {
            // TODO: Assert that new head has no leading trivia, old head has no
            // trailing trivia.
            // Invariant: A token inside a list of string fragments is always a head,
            // body or tail.
            // TODO: Is this invariant what we want? We could preserve the parse of
            // the string. That is, something like "a${b}c${d}e" is at present
            // represented as head, expr, body, expr, tail.  It could be instead
            // head, dollar, left brace, expr, right brace, body, dollar, left
            // brace, expr, right brace, tail. Is that better?
            //
            // TODO: Similarly we might want to preserve the structure of
            // heredoc strings in the parse: that there is a header consisting of
            // an identifier, and so on, and then body text, etc.
            match head {
                Some(head) => {
                    let k = match (head.kind(), token.kind()) {
                        (
                            TokenKind::DoubleQuotedStringLiteralHead,
                            TokenKind::DoubleQuotedStringLiteralTail,
                        ) => TokenKind::DoubleQuotedStringLiteral,
                        (
                            TokenKind::HeredocStringLiteralHead,
                            TokenKind::HeredocStringLiteralTail,
                        ) => TokenKind::HeredocStringLiteral,
                        (TokenKind::DoubleQuotedStringLiteralHead, _) => {
                            TokenKind::DoubleQuotedStringLiteralHead
                        }
                        (TokenKind::HeredocStringLiteralHead, _) => {
                            TokenKind::HeredocStringLiteralHead
                        }
                        (_, TokenKind::DoubleQuotedStringLiteralTail) => {
                            TokenKind::DoubleQuotedStringLiteralTail
                        }
                        (_, TokenKind::HeredocStringLiteralTail) => {
                            TokenKind::HeredocStringLiteralTail
                        }
                        _ => TokenKind::StringLiteralBody,
                    };
                    // this is incorrect for minimal tokens
                    let o = head.leading_start_offset().unwrap_or(0);
                    let w = head.width() + token.width();
                    let (l, _, _) = head.into_trivia_and_width();
                    let (_, _, t) = token.into_trivia_and_width();
                    // TODO: Make a "position" type that is a tuple of source and offset.
                    Some(parser.sc_mut().token_factory_mut().make(k, o, w, l, t))
                }
                None => {
                    let token = match token.kind() {
                        TokenKind::StringLiteralBody
                        | TokenKind::HeredocStringLiteralTail
                        | TokenKind::DoubleQuotedStringLiteralTail => token,
                        _ => parser
                            .sc_mut()
                            .token_factory_mut()
                            .with_kind(token, TokenKind::StringLiteralBody),
                    };
                    Some(token)
                }
            }
        };

        let put_opt = |parser: &mut Self, head: Option<Token<S>>, acc: &mut Vec<S::R>| {
            if let Some(h) = head {
                let token = S!(make_token, parser, h);
                acc.push(token)
            }
        };

        let parse_embedded_expression = |parser: &mut Self, token: Token<S>| {
            let token = S!(make_token, parser, token);
            let var_expr = S!(make_variable_expression, parser, token);
            let mut parser1 = parser.clone();
            let token1 = parser1.next_token_in_string(&literal_kind);
            let mut parser2 = parser1.clone();
            let token2 = parser2.next_token_in_string(&literal_kind);
            let mut parser3 = parser2.clone();
            let token3 = parser3.next_token_in_string(&literal_kind);
            match (token1.kind(), token2.kind(), token3.kind()) {
                (TokenKind::MinusGreaterThan, TokenKind::Name, _) => {
                    parser.continue_from(parser2);
                    let token1 = S!(make_token, parser, token1);
                    let token2 = S!(make_token, parser, token2);
                    S!(
                        make_embedded_member_selection_expression,
                        parser,
                        var_expr,
                        token1,
                        token2
                    )
                }
                (TokenKind::LeftBracket, TokenKind::Name, TokenKind::RightBracket) => {
                    parser.continue_from(parser3);
                    let token1 = S!(make_token, parser, token1);
                    let token2 = S!(make_token, parser, token2);
                    let token3 = S!(make_token, parser, token3);
                    S!(
                        make_embedded_subscript_expression,
                        parser,
                        var_expr,
                        token1,
                        token2,
                        token3
                    )
                }
                (TokenKind::LeftBracket, TokenKind::Variable, TokenKind::RightBracket) => {
                    parser.continue_from(parser3);
                    let token1 = S!(make_token, parser, token1);
                    let token2 = S!(make_token, parser, token2);
                    let expr = S!(make_variable_expression, parser, token2);
                    let token3 = S!(make_token, parser, token3);
                    S!(
                        make_embedded_subscript_expression,
                        parser,
                        var_expr,
                        token1,
                        expr,
                        token3
                    )
                }
                (TokenKind::LeftBracket, TokenKind::DecimalLiteral, TokenKind::RightBracket)
                | (TokenKind::LeftBracket, TokenKind::OctalLiteral, TokenKind::RightBracket)
                | (
                    TokenKind::LeftBracket,
                    TokenKind::HexadecimalLiteral,
                    TokenKind::RightBracket,
                )
                | (TokenKind::LeftBracket, TokenKind::BinaryLiteral, TokenKind::RightBracket) => {
                    parser.continue_from(parser3);
                    let token1 = S!(make_token, parser, token1);
                    let token2 = S!(make_token, parser, token2);
                    let expr = S!(make_literal_expression, parser, token2);
                    let token3 = S!(make_token, parser, token3);
                    S!(
                        make_embedded_subscript_expression,
                        parser,
                        var_expr,
                        token1,
                        expr,
                        token3
                    )
                }
                (TokenKind::LeftBracket, _, _) => {
                    // PHP compatibility: throw an error if we encounter an
                    // insufficiently-simple expression for a string like "$b[<expr>]", or if
                    // the expression or closing bracket are missing.
                    parser.continue_from(parser1);
                    let token1 = S!(make_token, parser, token1);
                    let token2 = S!(make_missing, parser, parser.pos());
                    let token3 = S!(make_missing, parser, parser.pos());
                    parser.with_error(Errors::expected_simple_offset_expression);
                    S!(
                        make_embedded_subscript_expression,
                        parser,
                        var_expr,
                        token1,
                        token2,
                        token3
                    )
                }
                _ => var_expr,
            }
        };

        let handle_left_brace = |
            parser: &mut Self,
            left_brace: Token<S>,
            head: Option<Token<S>>,
            acc: &mut Vec<S::R>,
        | {
            // Note that here we use next_token_in_string because we need to know
            // whether there is trivia between the left brace and the $x which follows.
            let mut parser1 = parser.clone();
            let token = parser1.next_token_in_string(&literal_kind);
            // TODO: What about "{$$}" ?
            match token.kind() {
                TokenKind::Dollar | TokenKind::Variable => {
                    put_opt(parser, head, acc); // TODO(leoo) check with kasper (was self)
                    let expr = parser.parse_braced_expression_in_string(
                        left_brace, /* dollar_inside_braces:*/ true,
                    );
                    acc.push(expr);
                    None
                }
                _ => {
                    // We do not support {$ inside a string unless the $ begins a
                    // variable name. Append the { and start again on the $.
                    // TODO: Is this right? Suppose we have "{${x}".  Is that the same
                    // as "{"."${x}" ? Double check this.
                    // TODO: Give an error.
                    // We got a { not followed by a $. Ignore it.
                    // TODO: Give a warning?
                    merge(parser, left_brace, head)
                }
            }
        };

        let handle_dollar =
            |parser: &mut Self, dollar, head: Option<Token<S>>, acc: &mut Vec<S::R>| {
                // We need to parse ${x} as though it was {$x}
                // TODO: This should be an error in strict mode.
                // We must not have trivia between the $ and the {, but we can have
                // trivia after the {. That's why we use next_token_in_string here.
                let mut parser1 = parser.clone();
                let token = parser1.next_token_in_string(&literal_kind);
                match token.kind() {
                    TokenKind::LeftBrace => {
                        // The thing in the braces has to be an expression that begins
                        // with a variable, and the variable does *not* begin with a $. It's
                        // just the word.
                        //
                        // Unlike the {$var} case, there *can* be trivia before the expression,
                        // which means that trivia is likely the trailing trivia of the brace,
                        // not leading trivia of the expression.
                        // TODO: Enforce these rules by producing an error if they are
                        // violated.
                        // TODO: Make the parse tree for the leading word in the expression
                        // a variable expression, not a qualified name expression.
                        parser.continue_from(parser1);
                        put_opt(parser, head, acc);
                        let dollar = S!(make_token, parser, dollar);
                        let expr = parser.parse_braced_expression_in_string(
                            token, /*dollar_inside_braces:*/ false,
                        );
                        acc.push(dollar);
                        acc.push(expr);
                        None
                    }
                    _ => {
                        // We got a $ not followed by a { or variable name. Ignore it.
                        // TODO: Give a warning?
                        merge(parser, dollar, head)
                    }
                }
            };

        let mut acc = vec![];
        let mut head = Some(head);

        loop {
            let token = self.next_token_in_string(&literal_kind);
            match token.kind() {
                TokenKind::HeredocStringLiteralTail | TokenKind::DoubleQuotedStringLiteralTail => {
                    let head = merge(self, token, head);
                    put_opt(self, head, &mut acc);
                    break;
                }
                TokenKind::LeftBrace => head = handle_left_brace(self, token, head, &mut acc),
                TokenKind::Variable => {
                    put_opt(self, head, &mut acc);
                    let expr = parse_embedded_expression(self, token);
                    head = None;
                    acc.push(expr)
                }
                TokenKind::Dollar => head = handle_dollar(self, token, head, &mut acc),
                _ => head = merge(self, token, head),
            }
        }

        // If we've ended up with a single string literal with no internal
        // structure, do not represent that as a list with one item.
        let results = if acc.len() == 1 {
            acc.pop().unwrap()
        } else {
            S!(make_list, self, acc, self.pos())
        };
        S!(make_literal_expression, self, results)
    }

    fn parse_inclusion_expression(&mut self) -> S::R {
        // SPEC:
        // inclusion-directive:
        //   require-multiple-directive
        //   require-once-directive
        //
        // require-multiple-directive:
        //   require  include-filename  ;
        //
        // include-filename:
        //   expression
        //
        // require-once-directive:
        //   require_once  include-filename  ;
        //
        // In non-strict mode we allow an inclusion directive (without semi) to be
        // used as an expression. It is therefore easier to actually parse this as:
        //
        // inclusion-directive:
        //   inclusion-expression  ;
        //
        // inclusion-expression:
        //   require include-filename
        //   require_once include-filename
        //
        // TODO: We allow "include" and "include_once" as well, which are PHP-isms
        // specified as not supported in Hack. Do we need to produce an error in
        // strict mode?
        //
        // TODO: Produce an error if this is used in an expression context
        // in strict mode.
        let require = self.next_token();
        let operator = Operator::prefix_unary_from_token(require.kind());
        let require = S!(make_token, self, require);
        let filename = self.parse_expression_with_operator_precedence(operator);
        S!(make_inclusion_expression, self, require, filename)
    }

    fn peek_next_kind_if_operator(&self) -> Option<TokenKind> {
        let kind = self.peek_token_kind();
        if Operator::is_trailing_operator_token(kind) {
            Some(kind)
        } else {
            None
        }
    }

    fn operator_has_lower_precedence(&self, operator_kind: TokenKind) -> bool {
        let operator = Operator::trailing_from_token(operator_kind);
        operator.precedence(&self.env) < self.precedence
    }

    fn next_is_lower_precedence(&self) -> bool {
        match self.peek_next_kind_if_operator() {
            None => true,
            Some(kind) => self.operator_has_lower_precedence(kind),
        }
    }

    fn try_parse_specified_function_call(&mut self, term: &S::R) -> Option<(S::R, Self)> {
        if !Self::can_term_take_type_args(term) {
            return None;
        }
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            return None;
        }
        let mut parser1 = self.clone();
        let (type_arguments, no_arg_is_missing) = parser1.parse_generic_type_arguments();
        if no_arg_is_missing && self.errors.len() == parser1.errors.len() {
            Some((type_arguments, parser1))
        } else {
            // Parse empty <> for function pointer without targ
            let mut parser2 = self.clone();
            let open_angle = parser2.fetch_token();
            if parser2.peek_token_kind() == TokenKind::GreaterThan {
                let missing = S!(make_missing, parser2, parser2.pos());
                let close_angle = parser2.assert_token(TokenKind::GreaterThan);
                let empty_targs = S!(make_type_arguments, self, open_angle, missing, close_angle);
                return Some((empty_targs, parser2));
            }
            None
        }
    }

    fn do_parse_specified_function_call(&mut self, term: S::R, type_arguments: S::R) -> S::R {
        let result = match self.peek_token_kind() {
            TokenKind::ColonColon => {
                // handle a<type-args>::... case
                let type_specifier = S!(make_generic_type_specifier, self, term, type_arguments);
                self.parse_scope_resolution_expression(type_specifier)
            }
            TokenKind::Hash | TokenKind::LeftParen => {
                let missing = S!(make_missing, self, self.pos());
                let enum_class_label = match self.peek_token_kind() {
                    TokenKind::Hash => self.parse_enum_class_label(missing),
                    _ => missing,
                };
                let (left, args, right) = self.parse_expression_list_opt();
                S!(
                    make_function_call_expression,
                    self,
                    term,
                    type_arguments,
                    enum_class_label,
                    left,
                    args,
                    right
                )
            }
            _ => S!(make_function_pointer_expression, self, term, type_arguments),
        };
        self.parse_remaining_expression(result)
    }

    fn can_be_used_as_lvalue(t: &S::R) -> bool {
        t.is_variable_expression()
            || t.is_subscript_expression()
            || t.is_member_selection_expression()
            || t.is_scope_resolution_expression()
    }

    // detects if left_term and operator can be treated as a beginning of
    // assignment (respecting the precedence of operator on the left of
    // left term). Returns
    // - PrefixNone - either operator is not one of assignment operators or
    // precedence of the operator on the left is higher than precedence of
    // assignment.
    // - PrefixAssignment - left_term  and operator can be interpreted as a
    // prefix of assignment
    // - Prefix:LessThan - is the start of a specified function call f<T>(...)
    fn check_if_should_override_normal_precedence(
        &mut self,
        left_term: &S::R,
        operator: TokenKind,
        left_precedence: usize,
    ) -> BinaryExpressionPrefixKind<(S::R, Self)> {
        // We need to override the precedence of the < operator in the case where it
        // is the start of a specified function call.
        let maybe_prefix =
            if self.peek_token_kind_with_possible_attributized_type_list() == TokenKind::LessThan {
                match self.try_parse_specified_function_call(left_term) {
                    Some(r) => Some(BinaryExpressionPrefixKind::PrefixLessThan(r)),
                    None => None,
                }
            } else {
                None
            };
        match maybe_prefix {
            Some(r) => r,
            None => {
                // in PHP precedence of assignment in expression is bumped up to
                // recognize cases like !$x = ... or $a == $b || $c = ...
                // which should be parsed as !($x = ...) and $a == $b || ($c = ...)
                if left_precedence >= Operator::precedence_for_assignment_in_expressions() {
                    BinaryExpressionPrefixKind::PrefixNone
                } else {
                    match operator {
                        TokenKind::Equal if left_term.is_list_expression() => {
                            BinaryExpressionPrefixKind::PrefixAssignment
                        }
                        TokenKind::Equal
                        | TokenKind::PlusEqual
                        | TokenKind::MinusEqual
                        | TokenKind::StarEqual
                        | TokenKind::SlashEqual
                        | TokenKind::StarStarEqual
                        | TokenKind::DotEqual
                        | TokenKind::PercentEqual
                        | TokenKind::AmpersandEqual
                        | TokenKind::BarEqual
                        | TokenKind::CaratEqual
                        | TokenKind::LessThanLessThanEqual
                        | TokenKind::GreaterThanGreaterThanEqual
                        | TokenKind::QuestionQuestionEqual
                            if Self::can_be_used_as_lvalue(&left_term) =>
                        {
                            BinaryExpressionPrefixKind::PrefixAssignment
                        }
                        _ => BinaryExpressionPrefixKind::PrefixNone,
                    }
                }
            }
        }
    }

    fn can_term_take_type_args(term: &S::R) -> bool {
        term.is_name()
            || term.is_qualified_name()
            || term.is_member_selection_expression()
            || term.is_safe_member_selection_expression()
            || term.is_scope_resolution_expression()
    }

    fn parse_remaining_expression(&mut self, term: S::R) -> S::R {
        match self.peek_next_kind_if_operator() {
            None => term,
            Some(token) => {
                let assignment_prefix_kind =
                    self.check_if_should_override_normal_precedence(&term, token, self.precedence);
                // stop parsing expression if:
                // - precedence of the operator is less than precedence of the operator
                // on the left
                // AND
                // - <term> <operator> does not look like a prefix of
                // some assignment expression

                match assignment_prefix_kind {
                    BinaryExpressionPrefixKind::PrefixLessThan((type_args, parser1)) => {
                        self.continue_from(parser1);
                        self.do_parse_specified_function_call(term, type_args)
                    }
                    BinaryExpressionPrefixKind::PrefixNone
                        if self.operator_has_lower_precedence(token) =>
                    {
                        term
                    }
                    _ => match token {
                        // Binary operators
                        // TODO Add an error if PHP style <> is used in Hack.
                        TokenKind::Plus
                        | TokenKind::Minus
                        | TokenKind::Star
                        | TokenKind::Slash
                        | TokenKind::StarStar
                        | TokenKind::Equal
                        | TokenKind::BarEqual
                        | TokenKind::PlusEqual
                        | TokenKind::StarEqual
                        | TokenKind::StarStarEqual
                        | TokenKind::SlashEqual
                        | TokenKind::DotEqual
                        | TokenKind::MinusEqual
                        | TokenKind::PercentEqual
                        | TokenKind::CaratEqual
                        | TokenKind::AmpersandEqual
                        | TokenKind::LessThanLessThanEqual
                        | TokenKind::GreaterThanGreaterThanEqual
                        | TokenKind::EqualEqualEqual
                        | TokenKind::LessThan
                        | TokenKind::GreaterThan
                        | TokenKind::Percent
                        | TokenKind::Dot
                        | TokenKind::EqualEqual
                        | TokenKind::AmpersandAmpersand
                        | TokenKind::BarBar
                        | TokenKind::ExclamationEqual
                        | TokenKind::ExclamationEqualEqual
                        | TokenKind::LessThanEqual
                        | TokenKind::LessThanEqualGreaterThan
                        | TokenKind::GreaterThanEqual
                        | TokenKind::Ampersand
                        | TokenKind::Bar
                        | TokenKind::LessThanLessThan
                        | TokenKind::GreaterThanGreaterThan
                        | TokenKind::Carat
                        | TokenKind::BarGreaterThan
                        | TokenKind::QuestionColon
                        | TokenKind::QuestionQuestion
                        | TokenKind::QuestionQuestionEqual => {
                            self.parse_remaining_binary_expression(term, assignment_prefix_kind)
                        }
                        TokenKind::Instanceof => {
                            self.with_error(Errors::instanceof_disabled);
                            let _ = self.assert_token(TokenKind::Instanceof);
                            term
                        }
                        TokenKind::Is => self.parse_is_expression(term),
                        TokenKind::As if self.allow_as_expressions() => {
                            self.parse_as_expression(term)
                        }
                        TokenKind::QuestionAs => self.parse_nullable_as_expression(term),
                        TokenKind::QuestionMinusGreaterThan | TokenKind::MinusGreaterThan => {
                            let result = self.parse_member_selection_expression(term);
                            self.parse_remaining_expression(result)
                        }
                        TokenKind::ColonColon => {
                            let result = self.parse_scope_resolution_expression(term);
                            self.parse_remaining_expression(result)
                        }
                        TokenKind::PlusPlus | TokenKind::MinusMinus => {
                            self.parse_postfix_unary(term)
                        }
                        TokenKind::Hash => {
                            self.parse_function_call_or_enum_class_label_expression(term)
                        }
                        TokenKind::LeftParen => {
                            let missing = S!(make_missing, self, self.pos());
                            self.parse_function_call(missing, term)
                        }
                        TokenKind::LeftBracket | TokenKind::LeftBrace => self.parse_subscript(term),
                        TokenKind::Question => {
                            let token = self.assert_token(TokenKind::Question);
                            let result = self.parse_conditional_expression(term, token);
                            self.parse_remaining_expression(result)
                        }
                        _ => term,
                    },
                }
            }
        }
    }

    fn parse_member_selection_expression(&mut self, term: S::R) -> S::R {
        // SPEC:
        // member-selection-expression:
        //   postfix-expression  =>  name
        //   postfix-expression  =>  variable-name
        //   postfix-expression  =>  xhp-class-name (DRAFT XHP SPEC)
        //
        // null-safe-member-selection-expression:
        //   postfix-expression  ?=>  name
        //   postfix-expression  ?=>  variable-name
        //   postfix-expression  ?=>  xhp-class-name (DRAFT XHP SPEC)
        //
        // PHP allows $a=>{$b}; to be more compatible with PHP, and give
        // good errors, we allow that here as well.
        //
        // TODO: Produce an error if the braced syntax is used in Hack.
        let token = self.next_token();
        let token_kind = token.kind();
        let op = S!(make_token, self, token);
        // TODO: We are putting the name / variable into the tree as a token
        // leaf, rather than as a name or variable expression. Is that right?
        let name = match self.peek_token_kind() {
            TokenKind::LeftBrace => self.parse_braced_expression(),
            TokenKind::Variable if self.env.php5_compat_mode => {
                self.parse_variable_in_php5_compat_mode()
            }
            TokenKind::Dollar => self.parse_dollar_expression(false),
            _ => self.require_xhp_class_name_or_name_or_variable(),
        };
        if token_kind == TokenKind::MinusGreaterThan {
            S!(make_member_selection_expression, self, term, op, name)
        } else {
            S!(make_safe_member_selection_expression, self, term, op, name)
        }
    }

    fn parse_variable_in_php5_compat_mode(&mut self) -> S::R {
        // PHP7 had a breaking change in parsing variables:
        // (https://wiki.php.net/rfc/uniform_variable_syntax).
        // Hack parser by default uses PHP7 compatible more which interprets
        // variables accesses left-to-right. It usually matches PHP5 behavior
        // except for cases with '$' operator, member accesses and scope resolution
        // operators:
        // $$a[1][2] => ($$a)[1][2]
        // $a=>$b[c] => ($a=>$b)[c]
        // X::$a[b]() => (X::$a)[b]()
        //
        // In order to preserve backward compatibility we can parse
        // variable/subscript expressions and treat them as if
        // braced expressions to enfore PHP5 semantics
        // $$a[1][2] => ${$a[1][2]}
        // $a=>$b[c] => $a=>{$b[c]}
        // X::$a[b]() => X::{$a[b]}()

        let old_precedence = self.precedence;
        let precedence = Operator::IndexingOperator.precedence(&self.env);
        self.with_precedence(precedence);
        let e = self.parse_expression();
        self.with_precedence(old_precedence);
        e
    }

    fn parse_subscript(&mut self, term: S::R) -> S::R {
        // SPEC
        // subscript-expression:
        //   postfix-expression  [  expression-opt  ]
        //   postfix-expression  {  expression-opt  }   [Deprecated form]
        //
        // TODO: Produce an error for brace case in a later pass
        let left = self.next_token();
        match (left.kind(), self.peek_token_kind()) {
            (TokenKind::LeftBracket, TokenKind::RightBracket)
            | (TokenKind::LeftBrace, TokenKind::RightBrace) => {
                let right = self.next_token();
                let left = S!(make_token, self, left);
                let missing = S!(make_missing, self, self.pos());
                let right = S!(make_token, self, right);
                let result = S!(make_subscript_expression, self, term, left, missing, right);
                self.parse_remaining_expression(result)
            }
            (left_kind, _) => {
                let left_token = S!(make_token, self, left);
                let index = self.with_as_expressions(/* enabled :*/ true, |x| {
                    x.with_reset_precedence(|x| x.parse_expression())
                });
                let right = match left_kind {
                    TokenKind::LeftBracket => self.require_right_bracket(),
                    _ => self.require_right_brace(),
                };
                let result = S!(
                    make_subscript_expression,
                    self,
                    term,
                    left_token,
                    index,
                    right
                );
                self.parse_remaining_expression(result)
            }
        }
    }

    fn parse_expression_list_opt(&mut self) -> (S::R, S::R, S::R) {
        // SPEC
        //
        // TODO: This business of allowing ... does not appear in the spec. Add it.
        //
        // TODO: Add call-convention-opt to the specification.
        // (This work is tracked by task T22582676.)
        //
        // TODO: Update grammar for inout parameters.
        // (This work is tracked by task T22582715.)
        //
        // ERROR RECOVERY: A ... expression can only appear at the end of a
        // formal parameter list. However, we parse it everywhere without error,
        // and detect the error in a later pass.
        //
        // Note that it *is* legal for a ... expression be followed by a trailing
        // comma, even though it is not legal for such in a formal parameter list.
        //
        // TODO: Can *any* expression appear after the ... ?
        //
        // argument-expression-list:
        //   argument-expressions   ,-opt
        // argument-expressions:
        //   expression
        //   ... expression
        //   call-convention-opt  expression
        //   argument-expressions  ,  expression
        //
        // This function parses the parens as well.
        self.parse_parenthesized_comma_list_opt_allow_trailing(|x| {
            x.with_reset_precedence(|x| x.parse_decorated_expression_opt())
        })
    }

    fn parse_decorated_expression_opt(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::DotDotDot | TokenKind::Inout => {
                let decorator = self.fetch_token();
                let expr = self.parse_expression();
                S!(make_decorated_expression, self, decorator, expr)
            }
            _ => self.parse_expression(),
        }
    }

    fn parse_start_of_type_specifier(&mut self, start_token: Token<S>) -> Option<S::R> {
        let name = if start_token.kind() == TokenKind::Backslash {
            let missing = S!(make_missing, self, self.pos());
            let backslash = S!(make_token, self, start_token);
            self.scan_qualified_name(missing, backslash)
        } else {
            let start_token = S!(make_token, self, start_token);
            self.scan_remaining_qualified_name(start_token)
        };
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LeftParen | TokenKind::LessThan => Some(name),
            _ => None,
        }
    }

    fn parse_designator(&mut self) -> S::R {
        // SPEC:
        // class-type-designator:
        //   parent
        //   self
        //   static
        //   member-selection-expression
        //   null-safe-member-selection-expression
        //   qualified-name
        //   scope-resolution-expression
        //   subscript-expression
        //   variable-name
        //
        // TODO: Update the spec to allow qualified-name < type arguments >
        // TODO: This will need to be fixed to allow situations where the qualified name
        // is also a non-reserved token.
        let default =
            |x: &mut Self| x.parse_expression_with_operator_precedence(Operator::NewOperator);
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::Parent | TokenKind::SelfToken => {
                match parser1.peek_token_kind_with_possible_attributized_type_list() {
                    TokenKind::LeftParen => {
                        self.continue_from(parser1);
                        S!(make_token, self, token)
                    }
                    TokenKind::LessThan => {
                        let (type_arguments, no_arg_is_missing) =
                            parser1.parse_generic_type_arguments();
                        if no_arg_is_missing && self.errors.len() == parser1.errors.len() {
                            self.continue_from(parser1);
                            let token = S!(make_token, self, token);
                            let type_specifier =
                                S!(make_generic_type_specifier, self, token, type_arguments);
                            type_specifier
                        } else {
                            default(self)
                        }
                    }
                    _ => default(self),
                }
            }
            TokenKind::Static if parser1.peek_token_kind() == TokenKind::LeftParen => {
                self.continue_from(parser1);
                S!(make_token, self, token)
            }
            TokenKind::Name | TokenKind::Backslash => {
                match parser1.parse_start_of_type_specifier(token) {
                    Some(name) => {
                        // We want to parse new C() and new C<int>() as types, but
                        // new C::$x() as an expression.
                        self.continue_from(parser1);
                        self.parse_remaining_type_specifier(name)
                    }
                    None => default(self),
                }
            }
            _ => {
                default(self)
                // TODO: We need to verify in a later pass that the expression is a
                // scope resolution (that does not end in class!), a member selection,
                // a name, a variable, a property, or an array subscript expression.
            }
        }
    }

    fn parse_object_creation_expression(&mut self) -> S::R {
        // SPEC
        // object-creation-expression:
        //   new object-creation-what
        let new_token = self.assert_token(TokenKind::New);
        let new_what = self.parse_constructor_call();
        S!(make_object_creation_expression, self, new_token, new_what)
    }

    pub fn parse_constructor_call(&mut self) -> S::R {
        // SPEC
        // constructor-call:
        //   class-type-designator  (  argument-expression-list-opt  )
        //
        // PHP allows the entire expression list to be omitted.
        // TODO: SPEC ERROR: PHP allows the entire expression list to be omitted,
        // but Hack disallows this behavior. (See SyntaxError.error2038) However,
        // the Hack spec still states that the argument expression list is optional.
        // Update the spec to say that the argument expression list is required.
        let designator = self.parse_designator();
        let (left, args, right) = if self.peek_token_kind() == TokenKind::LeftParen {
            self.parse_expression_list_opt()
        } else {
            let missing1 = S!(make_missing, self, self.pos());
            let missing2 = S!(make_missing, self, self.pos());
            let missing3 = S!(make_missing, self, self.pos());
            (missing1, missing2, missing3)
        };
        S!(make_constructor_call, self, designator, left, args, right)
    }

    fn parse_function_call_or_enum_class_label_expression(&mut self, term: S::R) -> S::R {
        // SPEC
        // fully-qualified-label:
        //   term '#' name
        // function-call-with-label:
        //   term '#' name '(' ... ')'
        let hash = self.assert_token(TokenKind::Hash);
        let label_name = self.require_name();
        if self.peek_token_kind() == TokenKind::LeftParen {
            let missing = S!(make_missing, self, self.pos());
            let enum_class_label = S!(
                make_enum_class_label_expression,
                self,
                missing,
                hash,
                label_name
            );
            self.parse_function_call(enum_class_label, term)
        } else {
            S!(
                make_enum_class_label_expression,
                self,
                term,
                hash,
                label_name
            )
        }
    }

    fn parse_function_call(&mut self, enum_class_label: S::R, receiver: S::R) -> S::R {
        // SPEC
        // function-call-expression:
        //   postfix-expression  (  argument-expression-list-opt  )
        let type_arguments = S!(make_missing, self, self.pos());
        let old_enabled = self.allow_as_expressions();
        self.allow_as_expressions = true;
        let (left, args, right) = self.parse_expression_list_opt();
        let result = S!(
            make_function_call_expression,
            self,
            receiver,
            type_arguments,
            enum_class_label,
            left,
            args,
            right
        );
        self.allow_as_expressions = old_enabled;
        self.parse_remaining_expression(result)
    }

    fn parse_variable_or_lambda(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let variable = parser1.assert_token(TokenKind::Variable);
        if parser1.peek_token_kind() == TokenKind::EqualEqualGreaterThan {
            let attribute_spec = S!(make_missing, self, self.pos());
            self.parse_lambda_expression(attribute_spec)
        } else {
            self.continue_from(parser1);
            S!(make_variable_expression, self, variable)
        }
    }

    fn parse_yield_expression(&mut self) -> S::R {
        // SPEC:
        // yield  array-element-initializer
        //
        let yield_kw = self.assert_token(TokenKind::Yield);
        match self.peek_token_kind() {
            TokenKind::Semicolon => {
                let missing = S!(make_missing, self, self.pos());
                S!(make_yield_expression, self, yield_kw, missing)
            }
            _ => {
                let operand = self.parse_array_element_init();
                S!(make_yield_expression, self, yield_kw, operand)
            }
        }
    }

    pub fn parse_cast_or_parenthesized_or_lambda_expression(&mut self) -> S::R {
        // We need to disambiguate between casts, lambdas and ordinary
        // parenthesized expressions.
        let mut parser1 = self.clone();
        match parser1.possible_cast_expression() {
            Some((left, cast_type, right)) => {
                self.continue_from(parser1);
                let operand =
                    self.parse_expression_with_operator_precedence(Operator::CastOperator);
                S!(make_cast_expression, self, left, cast_type, right, operand)
            }
            _ => {
                let mut parser1 = self.clone();
                match parser1.possible_lambda_expression() {
                    Some((attribute_spec, async_, signature)) => {
                        self.continue_from(parser1);
                        self.parse_lambda_expression_after_signature(
                            attribute_spec,
                            async_,
                            signature,
                        )
                    }
                    None => self.parse_parenthesized_expression(),
                }
            }
        }
    }

    fn possible_cast_expression(&mut self) -> Option<(S::R, S::R, S::R)> {
        // SPEC:
        // cast-expression:
        //   (  cast-type  ) unary-expression
        // cast-type:
        //   bool, double, float, real, int, integer, object, string, binary
        //
        // TODO: This implies that a cast "(name)" can only be a simple name, but
        // I would expect that (\Foo\Bar), (:foo), and the like
        // should also be legal casts. If we implement that then we will need
        // a sophisticated heuristic to determine whether this is a cast or a
        // parenthesized expression.
        //
        // The cast expression introduces an ambiguity: (x)-y could be a
        // subtraction or a cast on top of a unary minus. We resolve this
        // ambiguity as follows:
        //
        // * If the thing in parens is one of the keywords mentioned above, then
        //   it's a cast.
        // * Otherwise, it's a parenthesized expression.
        let left_paren = self.assert_token(TokenKind::LeftParen);
        let type_token = self.next_token();
        let type_token_kind = type_token.kind();
        let right_paren = self.next_token();
        let is_cast = right_paren.kind() == TokenKind::RightParen
            && match type_token_kind {
                TokenKind::Bool
                | TokenKind::Boolean
                | TokenKind::Double
                | TokenKind::Float
                | TokenKind::Real
                | TokenKind::Int
                | TokenKind::Integer
                | TokenKind::Object
                | TokenKind::String
                | TokenKind::Binary => true,
                _ => false,
            };
        if is_cast {
            let type_token = S!(make_token, self, type_token);
            let right_paren = S!(make_token, self, right_paren);
            Some((left_paren, type_token, right_paren))
        } else {
            None
        }
    }

    fn possible_lambda_expression(&mut self) -> Option<(S::R, S::R, S::R)> {
        // We have a left paren in hand and we already know we're not in a cast.
        // We need to know whether this is a parenthesized expression or the
        // signature of a lambda.
        //
        // There are a number of difficulties. For example, we cannot simply
        // check to see if a colon follows the expression:
        //
        // $a = $b ? ($x) : ($y)              ($x) is parenthesized expression
        // $a = $b ? ($x) : int ==> 1 : ($y)  ($x) is lambda signature
        //
        // ERROR RECOVERY:
        //
        // What we'll do here is simply attempt to parse a lambda formal parameter
        // list. If we manage to do so *without error*, and the thing which follows
        // is ==>, then this is definitely a lambda. If those conditions are not
        // met then we assume we have a parenthesized expression in hand.
        //
        // TODO: There could be situations where we have good evidence that a
        // lambda is intended but these conditions are not met. Consider
        // a more sophisticated recovery strategy.  For example, if we have
        // (x)==> then odds are pretty good that a lambda was intended and the
        // error should say that ($x)==> was expected.

        let old_errors = self.errors.len();

        let attribute_spec = S!(make_missing, self, self.pos());
        let (async_, signature) = self.parse_lambda_header();
        if old_errors == self.errors.len()
            && self.peek_token_kind() == TokenKind::EqualEqualGreaterThan
        {
            Some((attribute_spec, async_, signature))
        } else {
            None
        }
    }

    fn parse_lambda_expression(&mut self, attribute_spec: S::R) -> S::R {
        // SPEC
        // lambda-expression:
        //   async-opt  lambda-function-signature  ==>  lambda-body
        let (async_, signature) = self.parse_lambda_header();
        let arrow = self.require_lambda_arrow();
        let body = self.parse_lambda_body();
        S!(
            make_lambda_expression,
            self,
            attribute_spec,
            async_,
            signature,
            arrow,
            body
        )
    }

    fn parse_lambda_expression_after_signature(
        &mut self,
        attribute_spec: S::R,
        async_: S::R,
        signature: S::R,
    ) -> S::R {
        // We had a signature with no async, and we disambiguated it
        // from a cast.
        let arrow = self.require_lambda_arrow();
        let body = self.parse_lambda_body();
        S!(
            make_lambda_expression,
            self,
            attribute_spec,
            async_,
            signature,
            arrow,
            body
        )
    }

    fn parse_lambda_header(&mut self) -> (S::R, S::R) {
        let async_ = self.optional_token(TokenKind::Async);
        let signature = self.parse_lambda_signature();
        (async_, signature)
    }

    fn parse_lambda_signature(&mut self) -> S::R {
        // SPEC:
        // lambda-function-signature:
        //   variable-name
        //   (  anonymous-function-parameter-declaration-list-opt  ) /
        //      anonymous-function-return-opt
        if self.peek_token_kind() == TokenKind::Variable {
            let token = self.next_token();
            S!(make_token, self, token)
        } else {
            let (left, params, right) = self.parse_parameter_list_opt();
            let contexts = self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_contexts());
            let (colon, readonly_opt, return_type) = self.parse_optional_return();
            S!(
                make_lambda_signature,
                self,
                left,
                params,
                right,
                contexts,
                colon,
                readonly_opt,
                return_type
            )
        }
    }

    fn parse_lambda_body(&mut self) -> S::R {
        // SPEC:
        // lambda-body:
        //   expression
        //   compound-statement
        if self.peek_token_kind() == TokenKind::LeftBrace {
            self.parse_compound_statement()
        } else {
            self.with_reset_precedence(|x| x.parse_expression())
        }
    }

    fn parse_parenthesized_expression(&mut self) -> S::R {
        let left_paren = self.assert_token(TokenKind::LeftParen);
        let expression =
            self.with_as_expressions(
                /* enabled:*/ true,
                |p| p.with_reset_precedence(|p| p.parse_expression()),
            );
        let right_paren = self.require_right_paren();
        S!(
            make_parenthesized_expression,
            self,
            left_paren,
            expression,
            right_paren
        )
    }

    fn parse_postfix_unary(&mut self, term: S::R) -> S::R {
        let token = self.fetch_token();
        let term = S!(make_postfix_unary_expression, self, term, token);
        self.parse_remaining_expression(term)
    }

    fn parse_prefix_unary_expression(&mut self) -> S::R {
        // TODO: Operand to ++ and -- must be an lvalue.
        let token = self.next_token();
        let kind = token.kind();
        let operator = Operator::prefix_unary_from_token(kind);
        let token = S!(make_token, self, token);
        let operand = self.parse_expression_with_operator_precedence(operator);
        S!(make_prefix_unary_expression, self, token, operand)
    }

    pub fn parse_simple_variable(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::Variable => {
                let variable = self.next_token();
                S!(make_token, self, variable)
            }
            TokenKind::Dollar => self.parse_dollar_expression(false),
            _ => self.require_variable(),
        }
    }

    fn parse_dollar_expression(&mut self, is_term: bool) -> S::R {
        let dollar = self.assert_token(TokenKind::Dollar);
        let operand = match self.peek_token_kind() {
            TokenKind::LeftBrace if is_term => {
                return self.parse_et_splice_expression(dollar);
            }
            TokenKind::LeftBrace => self.parse_braced_expression(),
            TokenKind::Variable if self.env.php5_compat_mode => {
                self.parse_variable_in_php5_compat_mode()
            }
            _ => self.parse_expression_with_operator_precedence(Operator::prefix_unary_from_token(
                TokenKind::Dollar,
            )),
        };
        S!(make_prefix_unary_expression, self, dollar, operand)
    }

    fn parse_is_as_helper<F>(&mut self, f: F, left: S::R, kw: TokenKind) -> S::R
    where
        F: Fn(&mut Self, S::R, S::R, S::R) -> S::R,
    {
        let op = self.assert_token(kw);
        let right =
            self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_type_specifier(false, true));
        let result = f(self, left, op, right);
        self.parse_remaining_expression(result)
    }

    fn parse_is_expression(&mut self, left: S::R) -> S::R {
        // SPEC:
        // is-expression:
        //   is-subject  is  type-specifier
        //
        // is-subject:
        //   expression
        self.parse_is_as_helper(
            |p, x, y, z| S!(make_is_expression, p, x, y, z),
            left,
            TokenKind::Is,
        )
    }

    fn parse_as_expression(&mut self, left: S::R) -> S::R {
        // SPEC:
        // as-expression:
        //   as-subject  as  type-specifier
        //
        // as-subject:
        //   expression
        self.parse_is_as_helper(
            |p, x, y, z| S!(make_as_expression, p, x, y, z),
            left,
            TokenKind::As,
        )
    }

    fn parse_nullable_as_expression(&mut self, left: S::R) -> S::R {
        // SPEC:
        // nullable-as-expression:
        //   as-subject  ?as  type-specifier
        self.parse_is_as_helper(
            |p, x, y, z| S!(make_nullable_as_expression, p, x, y, z),
            left,
            TokenKind::QuestionAs,
        )
    }

    fn parse_remaining_binary_expression(
        &mut self,
        left_term: S::R,
        assignment_prefix_kind: BinaryExpressionPrefixKind<(S::R, Self)>,
    ) -> S::R {
        // We have a left term. If we get here then we know that
        // we have a binary operator to its right, and that furthermore,
        // the binary operator is of equal or higher precedence than the
        // whatever is going on in the left term.
        //
        // Here's how this works.  Suppose we have something like
        //
        //   A x B y C
        //
        // where A, B and C are terms, and x and y are operators.
        // We must determine whether this parses as
        //
        //   (A x B) y C
        //
        // or
        //
        //   A x (B y C)
        //
        // We have the former if either x is higher precedence than y,
        // or x and y are the same precedence and x is left associative.
        // Otherwise, if x is lower precedence than y, or x is right
        // associative, then we have the latter.
        //
        // How are we going to figure this out?
        //
        // We have the term A in hand; the precedence is low.
        // We see that x follows A.
        // We obtain the precedence of x. It is higher than the precedence of A,
        // so we obtain B, and then we call a helper method that
        // collects together everything to the right of B that is
        // of higher precedence than x. (Or equal, and right-associative.)
        //
        // So, if x is of lower precedence than y (or equal and right-assoc)
        // then the helper will construct (B y C) as the right term, and then
        // we'll make A x (B y C), and we're done.  Otherwise, the helper
        // will simply return B, we'll construct (A x B) and recurse with that
        // as the left term.
        let is_rhs_of_assignment = assignment_prefix_kind.is_assignment();
        assert!(!self.next_is_lower_precedence() || is_rhs_of_assignment);

        let token = self.next_token();
        let operator = Operator::trailing_from_token(token.kind());
        let precedence = operator.precedence(&self.env);
        let token = S!(make_token, self, token);
        let right_term = if is_rhs_of_assignment {
            // reset the current precedence to make sure that expression on
            // the right hand side of the assignment is fully consumed
            self.with_reset_precedence(|p| p.parse_term())
        } else {
            self.parse_term()
        };
        let right_term = self.parse_remaining_binary_expression_helper(right_term, precedence);
        let term = S!(make_binary_expression, self, left_term, token, right_term);
        self.parse_remaining_expression(term)
    }

    fn parse_remaining_binary_expression_helper(
        &mut self,
        right_term: S::R,
        left_precedence: usize,
    ) -> S::R {
        // This gathers up terms to the right of an operator that are
        // operands of operators of higher precedence than the
        // operator to the left. For instance, if we have
        // A + B * C / D + E and we just parsed A +, then we want to
        // gather up B * C / D into the right side of the +.
        // In this case "right term" would be B and "left precedence"
        // would be the precedence of +.
        // See comments above for more details.
        let kind = self.peek_token_kind();
        if Operator::is_trailing_operator_token(kind)
            && (kind != TokenKind::As || self.allow_as_expressions())
        {
            let right_operator = Operator::trailing_from_token(kind);
            let right_precedence = right_operator.precedence(&self.env);
            let associativity = right_operator.associativity(&self.env);

            // check if this is the case ... $a = ...
            // where
            //   'left_precedence' - precedence of the operation on the left of $a
            //   'right_term' - $a
            //   'kind' - operator that follows right_term
            //
            // in case if right_term is valid left hand side for the assignment
            // and token is assignment operator and left_precedence is less than
            // bumped priority for the assignment we reset precedence before parsing
            // right hand side of the assignment to make sure it is consumed.
            match self.check_if_should_override_normal_precedence(
                &right_term,
                kind,
                left_precedence,
            ) {
                BinaryExpressionPrefixKind::PrefixLessThan(_) => {
                    let old_precedence = self.precedence;
                    let right_term = {
                        self.with_precedence(left_precedence);
                        self.parse_remaining_expression(right_term)
                    };
                    self.with_precedence(old_precedence);
                    self.parse_remaining_binary_expression_helper(right_term, left_precedence)
                }
                BinaryExpressionPrefixKind::PrefixAssignment => {
                    let old_precedence = self.precedence;
                    let right_term =
                        self.with_reset_precedence(|p| p.parse_remaining_expression(right_term));
                    self.with_precedence(old_precedence);
                    self.parse_remaining_binary_expression_helper(right_term, left_precedence)
                }
                BinaryExpressionPrefixKind::PrefixNone
                    if right_precedence > left_precedence
                        || (associativity == Assoc::RightAssociative
                            && right_precedence == left_precedence) =>
                {
                    let old_precedence = self.precedence;
                    let right_term = {
                        self.with_precedence(right_precedence);
                        self.parse_remaining_expression(right_term)
                    };
                    self.with_precedence(old_precedence);
                    self.parse_remaining_binary_expression_helper(right_term, left_precedence)
                }
                BinaryExpressionPrefixKind::PrefixNone => right_term,
            }
        } else {
            right_term
        }
    }

    fn parse_conditional_expression(&mut self, test: S::R, question: S::R) -> S::R {
        // POSSIBLE SPEC PROBLEM
        // We allow any expression, including assignment expressions, to be in
        // the consequence and alternative of a conditional expression, even
        // though assignment is lower precedence than ?:.  This is legal:
        // $a ? $b = $c : $d = $e
        // Interestingly, this is illegal in C and Java, which require parens,
        // but legal in C
        let kind = self.peek_token_kind();
        // ERROR RECOVERY
        // e1 ?: e2 is legal and we parse it as a binary expression. However,
        // it is possible to treat it degenerately as a conditional with no
        // consequence. This introduces an ambiguity
        //   x ? :y::m : z
        // Is that
        //   x   ?:   y::m   :   z    [1]
        // or
        //   x   ?   :y::m   :   z    [2]
        //
        // First consider a similar expression
        //   x ? : y::m
        // If we assume XHP class names cannot have a space after the : , then
        // this only has one interpretation
        //   x   ?:   y::m
        //
        // The first example also resolves cleanly to [2]. To reduce confusion,
        // we report an error for the e1 ? : e2 construction in a later pass.
        //
        // TODO: Add this to the XHP draft specification.
        let missing_consequence = kind == TokenKind::Colon && !(self.is_next_xhp_class_name());
        let consequence = if missing_consequence {
            S!(make_missing, self, self.pos())
        } else {
            self.with_reset_precedence(|p| p.parse_expression())
        };
        let colon = self.require_colon();
        let term = self.parse_term();
        let precedence = Operator::ConditionalQuestionOperator.precedence(&self.env);
        let alternative = self.parse_remaining_binary_expression_helper(term, precedence);
        S!(
            make_conditional_expression,
            self,
            test,
            question,
            consequence,
            colon,
            alternative
        )
    }

    /// Parse a name, a collection literal like vec[1, 2] or an
    /// expression tree literal Code`1`;
    fn parse_name_or_collection_literal_expression(&mut self, name: S::R) -> S::R {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LeftBrace => {
                let name = S!(make_simple_type_specifier, self, name);
                self.parse_collection_literal_expression(name)
            }
            TokenKind::LessThan => {
                let mut parser1 = self.clone();
                let (type_arguments, no_arg_is_missing) = parser1.parse_generic_type_arguments();
                if no_arg_is_missing
                    && self.errors.len() == parser1.errors.len()
                    && parser1.peek_token_kind() == TokenKind::LeftBrace
                {
                    self.continue_from(parser1);
                    let name = S!(make_generic_type_specifier, self, name, type_arguments);
                    self.parse_collection_literal_expression(name)
                } else {
                    name
                }
            }
            TokenKind::LeftBracket => {
                if self.peek_token_kind_with_lookahead(2) == TokenKind::EqualGreaterThan {
                    self.parse_record_creation_expression(name)
                } else {
                    name
                }
            }
            TokenKind::Backtick => {
                if self.in_expression_tree() {
                    // If we see Foo` whilst parsing an expression tree
                    // literal, it's a constant followed by a closing
                    // backtick. For example: Bar`1 + Foo`
                    name
                } else {
                    // Opening backtick of an expression tree literal.
                    let prefix = S!(make_simple_type_specifier, self, name);
                    let left_backtick = self.require_token(TokenKind::Backtick, Errors::error1065);
                    self.in_expression_tree = true;
                    let expr = self.parse_expression_with_reset_precedence();
                    self.in_expression_tree = false;
                    let right_backtick = self.require_token(TokenKind::Backtick, Errors::error1065);
                    S!(
                        make_prefixed_code_expression,
                        self,
                        prefix,
                        left_backtick,
                        expr,
                        right_backtick
                    )
                }
            }
            _ => name,
        }
    }

    fn parse_record_creation_expression(&mut self, name: S::R) -> S::R {
        // SPEC
        // record-creation:
        //   record-name [ record-field-initializer-list ]
        // record-fileld-initilizer-list:
        //   record-field-initilizer
        //   record-field-initializer-list, record-field-initializer
        // record-field-initializer:
        //   field-name => expression
        let left_bracket = self.assert_token(TokenKind::LeftBracket);
        let members = self.parse_comma_list_opt_allow_trailing(
            TokenKind::RightBracket,
            Errors::error1015,
            |x| x.parse_keyed_element_initializer(),
        );
        let right_bracket = self.require_right_bracket();
        S!(
            make_record_creation_expression,
            self,
            name,
            left_bracket,
            members,
            right_bracket
        )
    }

    fn parse_collection_literal_expression(&mut self, name: S::R) -> S::R {
        // SPEC
        // collection-literal:
        //   key-collection-class-type  {  cl-initializer-list-with-keys-opt  }
        //   non-key-collection-class-type  {  cl-initializer-list-without-keys-opt  }
        //   pair-type  {  cl-element-value  ,  cl-element-value  }
        //
        // The types are grammatically qualified names; however the specification
        // states that they must be as follows:
        //  * keyed collection type can be Map or ImmMap
        //  * non-keyed collection type can be Vector, ImmVector, Set or ImmSet
        //  * pair type can be Pair
        //
        // We will not attempt to determine if the names give the name of an
        // appropriate type here. That's for the type checker.
        //
        // The argumment lists are:
        //
        //  * for keyed, an optional comma-separated list of
        //    expression => expression pairs
        //  * for non-keyed, an optional comma-separated list of expressions
        //  * for pairs, a comma-separated list of exactly two expressions
        //
        // In all three cases, the lists may be comma-terminated.
        // TODO: This fact is not represented in the specification; it should be.
        // This work item is tracked by spec issue #109.

        let (left_brace, initialization_list, right_brace) =
            self.parse_braced_comma_list_opt_allow_trailing(|p| p.parse_init_expression());
        // Validating the name is a collection type happens in a later phase
        S!(
            make_collection_literal_expression,
            self,
            name,
            left_brace,
            initialization_list,
            right_brace
        )
    }

    fn parse_init_expression(&mut self) -> S::R {
        // ERROR RECOVERY
        // We expect either a list of expr, expr, expr, ... or
        // expr => expr, expr => expr, expr => expr, ...
        // Rather than require at parse time that the list be all one or the other,
        // we allow both, and give an error in the type checker.
        let expr1 = self.parse_expression_with_reset_precedence();
        if self.peek_token_kind() == TokenKind::EqualGreaterThan {
            let token = self.next_token();
            let arrow = S!(make_token, self, token);
            let expr2 = self.parse_expression_with_reset_precedence();
            S!(make_element_initializer, self, expr1, arrow, expr2)
        } else {
            expr1
        }
    }

    fn parse_keyed_element_initializer(&mut self) -> S::R {
        let expr1 = self.parse_expression_with_reset_precedence();
        let arrow = self.require_arrow();
        let expr2 = self.parse_expression_with_reset_precedence();
        S!(make_element_initializer, self, expr1, arrow, expr2)
    }

    fn parse_list_expression(&mut self) -> S::R {
        // SPEC:
        // list-intrinsic:
        //   list  (  expression-list-opt  )
        // expression-list:
        //   expression-opt
        //   expression-list , expression-opt
        //
        // See https://github.com/hhvm/hack-langspec/issues/82
        //
        // list-intrinsic must be used as the left-hand operand in a
        // simple-assignment-expression of which the right-hand operand
        // must be an expression that designates a vector-like array or
        // an instance of the class types Vector, ImmVector, or Pair
        // (the "source").
        //
        // TODO: Produce an error later if the expressions in the list destructuring
        // are not lvalues.
        let keyword = self.assert_token(TokenKind::List);
        let (left, items, right) = self.parse_parenthesized_comma_list_opt_items_opt(|p| {
            p.parse_expression_with_reset_precedence()
        });
        S!(make_list_expression, self, keyword, left, items, right)
    }

    fn parse_bracketed_collection_intrinsic_expression<F, G>(
        &mut self,
        keyword_token: TokenKind,
        parse_element_function: F,
        make_intrinsic_function: G,
    ) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
        G: Fn(&mut Self, S::R, S::R, S::R, S::R, S::R) -> S::R,
    {
        let mut parser1 = self.clone();
        let keyword = parser1.assert_token(keyword_token);
        let explicit_type = match parser1.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => {
                let (type_arguments, _) = parser1.parse_generic_type_arguments();
                // skip no_arg_is_missing check since there must only be 1 or 2 type arguments
                type_arguments
            }
            _ => S!(make_missing, parser1, parser1.pos()),
        };
        let left_bracket = parser1.optional_token(TokenKind::LeftBracket);
        if left_bracket.is_missing() {
            // Fall back to dict being an ordinary name. Perhaps we're calling a
            // function whose name is indicated by the keyword_token, for example.
            self.parse_as_name_or_error()
        } else {
            self.continue_from(parser1);
            let members = self.parse_comma_list_opt_allow_trailing(
                TokenKind::RightBracket,
                Errors::error1015,
                parse_element_function,
            );
            let right_bracket = self.require_right_bracket();
            make_intrinsic_function(
                self,
                keyword,
                explicit_type,
                left_bracket,
                members,
                right_bracket,
            )
        }
    }

    fn parse_darray_intrinsic_expression(&mut self) -> S::R {
        // TODO: Create the grammar and add it to the spec.
        self.parse_bracketed_collection_intrinsic_expression(
            TokenKind::Darray,
            |p| p.parse_keyed_element_initializer(),
            |p, a, b, c, d, e| S!(make_darray_intrinsic_expression, p, a, b, c, d, e),
        )
    }

    fn parse_dictionary_intrinsic_expression(&mut self) -> S::R {
        // TODO: Create the grammar and add it to the spec.
        // TODO: Can the list have a trailing comma?
        self.parse_bracketed_collection_intrinsic_expression(
            TokenKind::Dict,
            |p| p.parse_keyed_element_initializer(),
            |p, a, b, c, d, e| S!(make_dictionary_intrinsic_expression, p, a, b, c, d, e),
        )
    }

    fn parse_keyset_intrinsic_expression(&mut self) -> S::R {
        self.parse_bracketed_collection_intrinsic_expression(
            TokenKind::Keyset,
            |p| p.parse_expression_with_reset_precedence(),
            |p, a, b, c, d, e| S!(make_keyset_intrinsic_expression, p, a, b, c, d, e),
        )
    }

    fn parse_varray_intrinsic_expression(&mut self) -> S::R {
        // TODO: Create the grammar and add it to the spec.
        self.parse_bracketed_collection_intrinsic_expression(
            TokenKind::Varray,
            |p| p.parse_expression_with_reset_precedence(),
            |p, a, b, c, d, e| S!(make_varray_intrinsic_expression, p, a, b, c, d, e),
        )
    }

    fn parse_vector_intrinsic_expression(&mut self) -> S::R {
        // TODO: Create the grammar and add it to the spec.
        // TODO: Can the list have a trailing comma?
        self.parse_bracketed_collection_intrinsic_expression(
            TokenKind::Vec,
            |p| p.parse_expression_with_reset_precedence(),
            |p, a, b, c, d, e| S!(make_vector_intrinsic_expression, p, a, b, c, d, e),
        )
    }

    // array-element-initializer :=
    //   expression
    //   expression => expression
    fn parse_array_element_init(&mut self) -> S::R {
        let expr1 = self.with_reset_precedence(|p| p.parse_expression());
        match self.peek_token_kind() {
            TokenKind::EqualGreaterThan => {
                let token = self.next_token();
                let arrow = S!(make_token, self, token);
                let expr2 = self.with_reset_precedence(|p| p.parse_expression());
                S!(make_element_initializer, self, expr1, arrow, expr2)
            }
            _ => expr1,
        }
    }

    fn parse_field_initializer(&mut self) -> S::R {
        // SPEC
        // field-initializer:
        //   single-quoted-string-literal  =>  expression
        //   double_quoted_string_literal  =>  expression
        //   qualified-name  =>  expression
        //   scope-resolution-expression  =>  expression
        //

        // Specification is wrong, and fixing it is being tracked by
        // https://github.com/hhvm/hack-langspec/issues/108
        //

        // ERROR RECOVERY: We allow any expression on the left-hand side,
        // even though only some expressions are legal;
        // we will give an error in a later pass
        let name = self.with_reset_precedence(|p| p.parse_expression());
        let arrow = self.require_arrow();
        let value = self.with_reset_precedence(|p| p.parse_expression());
        S!(make_field_initializer, self, name, arrow, value)
    }

    fn parse_shape_expression(&mut self) -> S::R {
        // SPEC
        // shape-literal:
        //   shape  (  field-initializer-list-opt  )
        //
        // field-initializer-list:
        //   field-initializers  ,-op
        //
        // field-initializers:
        //   field-initializer
        //   field-initializers  ,  field-initializer
        let shape = self.assert_token(TokenKind::Shape);
        let (left_paren, fields, right_paren) =
            self.parse_parenthesized_comma_list_opt_allow_trailing(|p| p.parse_field_initializer());
        S!(
            make_shape_expression,
            self,
            shape,
            left_paren,
            fields,
            right_paren
        )
    }

    fn parse_tuple_expression(&mut self) -> S::R {
        // SPEC
        // tuple-literal:
        //   tuple  (  expression-list-one-or-more  )
        //
        // expression-list-one-or-more:
        //   expression
        //   expression-list-one-or-more  ,  expression
        //
        // TODO: Can the list be comma-terminated? If so, update the spec.
        // TODO: We need to produce an error in a later pass if the list is empty.
        let keyword = self.assert_token(TokenKind::Tuple);
        let (left_paren, items, right_paren) = self
            .parse_parenthesized_comma_list_opt_allow_trailing(|p| {
                p.parse_expression_with_reset_precedence()
            });
        S!(
            make_tuple_expression,
            self,
            keyword,
            left_paren,
            items,
            right_paren
        )
    }

    fn parse_use_variable(&mut self) -> S::R {
        self.require_variable()
    }

    fn parse_anon_or_lambda_or_awaitable(&mut self) -> S::R {
        // TODO: The original Hack parser accepts "async" as an identifier, and
        // so we do too. We might consider making it reserved.
        // Skip any async declarations that may be present. When we
        // feed the original parser into the syntax parsers. they will take care of
        // them as appropriate.
        let parser1 = self.clone();
        let attribute_spec = self.with_decl_parser(|p| p.parse_attribute_specification_opt());
        let mut parser2 = self.clone();
        let _ = parser2.optional_token(TokenKind::Async);
        match parser2.peek_token_kind() {
            TokenKind::Function => self.parse_anon(attribute_spec),
            TokenKind::LeftBrace => self.parse_async_block(attribute_spec),
            TokenKind::Variable | TokenKind::LeftParen => {
                self.parse_lambda_expression(attribute_spec)
            }
            _ => {
                self.continue_from(parser1);
                let async_as_name = self.next_token_as_name();
                S!(make_token, self, async_as_name)
            }
        }
    }

    fn parse_async_block(&mut self, attribute_spec: S::R) -> S::R {
        // grammar:
        //   awaitable-creation-expression :
        //     async-opt compound-statement
        // TODO awaitable-creation-expression must not be used as the
        //      anonymous-function-body in a lambda-expression
        let async_ = self.optional_token(TokenKind::Async);
        let stmt = self.parse_compound_statement();
        S!(
            make_awaitable_creation_expression,
            self,
            attribute_spec,
            async_,
            stmt
        )
    }

    fn parse_anon_use_opt(&mut self) -> S::R {
        // SPEC:
        // anonymous-function-use-clause:
        //   use  (  use-variable-name-list  ,-opt  )
        //
        // use-variable-name-list:
        //   variable-name
        //   use-variable-name-list  ,  variable-name
        let use_token = self.optional_token(TokenKind::Use);
        if use_token.is_missing() {
            use_token
        } else {
            let (left, vars, right) =
                self.parse_parenthesized_comma_list_opt_allow_trailing(|p| p.parse_use_variable());
            S!(
                make_anonymous_function_use_clause,
                self,
                use_token,
                left,
                vars,
                right
            )
        }
    }

    fn parse_optional_readonly(&mut self) -> S::R {
        self.optional_token(TokenKind::Readonly)
    }

    fn parse_optional_return(&mut self) -> (S::R, S::R, S::R) {
        // Parse an optional "colon-folowed-by-return-type"
        let colon = self.optional_token(TokenKind::Colon);
        let (readonly_opt, return_type) = if colon.is_missing() {
            let missing1 = S!(make_missing, self, self.pos());
            let missing2 = S!(make_missing, self, self.pos());
            (missing1, missing2)
        } else {
            let readonly = self.parse_optional_readonly();
            let return_type = self.with_type_parser(|p| p.parse_return_type());
            (readonly, return_type)
        };
        (colon, readonly_opt, return_type)
    }

    fn parse_anon(&mut self, attribute_spec: S::R) -> S::R {
        // SPEC
        // anonymous-function-creation-expression:
        //   async-opt function
        //     ( anonymous-function-parameter-list-opt  )
        //     anonymous-function-return-opt
        //     anonymous-function-use-clauseopt
        //     compound-statement
        //
        // An anonymous function's formal parameter list is the same as a named
        // function's formal parameter list except that types are optional.
        // The "..." syntax and trailing commas are supported. We'll simply
        // parse an optional parameter list; it already takes care of making the
        // type annotations optional.
        let async_ = self.optional_token(TokenKind::Async);
        let fn_ = self.assert_token(TokenKind::Function);
        let (left_paren, params, right_paren) = self.parse_parameter_list_opt();
        let ctx_list = self.with_type_parser(|p| p.parse_contexts());
        let (colon, readonly_opt, return_type) = self.parse_optional_return();
        let use_clause = self.parse_anon_use_opt();
        // Detect if the user has the type in the wrong place
        // function() use(): T // wrong
        // function(): T use() // correct
        if !use_clause.is_missing() {
            let misplaced_colon = self.clone().optional_token(TokenKind::Colon);
            if !misplaced_colon.is_missing() {
                self.with_error(Cow::Borrowed(
                    "Bad signature: use(...) should occur after the type",
                ));
            }
        }
        let body = self.parse_compound_statement();
        S!(
            make_anonymous_function,
            self,
            attribute_spec,
            async_,
            fn_,
            left_paren,
            params,
            right_paren,
            ctx_list,
            colon,
            readonly_opt,
            return_type,
            use_clause,
            body,
        )
    }

    fn parse_et_splice_expression(&mut self, dollar: S::R) -> S::R {
        let left_brace = self.assert_token(TokenKind::LeftBrace);
        let expression = self.parse_expression_with_reset_precedence();
        let right_brace = self.require_right_brace();
        S!(
            make_et_splice_expression,
            self,
            dollar,
            left_brace,
            expression,
            right_brace
        )
    }

    fn parse_braced_expression(&mut self) -> S::R {
        let left_brace = self.assert_token(TokenKind::LeftBrace);
        let expression = self.parse_expression_with_reset_precedence();
        let right_brace = self.require_right_brace();
        S!(
            make_braced_expression,
            self,
            left_brace,
            expression,
            right_brace
        )
    }

    fn require_right_brace_xhp(&mut self) -> S::R {
        // do not consume trailing trivia for the right brace
        // it should be accounted as XHP text
        let mut parser1 = self.clone();
        let token = parser1.next_token_no_trailing();
        if token.kind() == TokenKind::RightBrace {
            self.continue_from(parser1);
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Create a missing token for the expected token,
            // and continue on from the current token. Don't skip it.
            self.with_error(Errors::error1006);
            S!(make_missing, self, self.pos())
        }
    }

    fn parse_xhp_body_braced_expression(&mut self) -> S::R {
        // The difference between a regular braced expression and an
        // XHP body braced expression is:
        // <foo bar={$x}/*this_is_a_comment*/>{$y}/*this_is_body_text!*/</foo>
        let left_brace = self.assert_token(TokenKind::LeftBrace);
        let expression = self.parse_expression_with_reset_precedence();
        let right_brace = self.require_right_brace_xhp();
        S!(
            make_braced_expression,
            self,
            left_brace,
            expression,
            right_brace
        )
    }

    fn next_xhp_element_token(&mut self, no_trailing: bool) -> (Token<S>, &[u8]) {
        self.lexer_mut().next_xhp_element_token(no_trailing)
    }

    fn next_xhp_body_token(&mut self) -> Token<S> {
        self.lexer_mut().next_xhp_body_token()
    }

    fn parse_xhp_attribute(&mut self) -> Option<S::R> {
        let mut parser1 = self.clone();
        let (token, _) = parser1.next_xhp_element_token(false);
        match token.kind() {
            TokenKind::LeftBrace => self.parse_xhp_spread_attribute(),
            TokenKind::XHPElementName => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                self.parse_xhp_simple_attribute(token)
            }
            _ => None,
        }
    }

    fn parse_xhp_spread_attribute(&mut self) -> Option<S::R> {
        let (left_brace, _) = self.next_xhp_element_token(false);
        let left_brace = S!(make_token, self, left_brace);
        let ellipsis = self.require_token(TokenKind::DotDotDot, Errors::expected_dotdotdot);
        let expression = self.parse_expression_with_reset_precedence();
        let right_brace = self.require_right_brace();
        let node = S!(
            make_xhp_spread_attribute,
            self,
            left_brace,
            ellipsis,
            expression,
            right_brace
        );
        Some(node)
    }

    fn parse_xhp_simple_attribute(&mut self, name: S::R) -> Option<S::R> {
        // Parse the attribute name and then defensively check for well-formed
        // attribute assignment
        let mut parser1 = self.clone();
        let (token, _) = parser1.next_xhp_element_token(false);
        if token.kind() != TokenKind::Equal {
            self.with_error(Errors::error1016);
            self.continue_from(parser1);
            let missing1 = S!(make_missing, self, self.pos());
            let missing2 = S!(make_missing, self, self.pos());
            let node = S!(make_xhp_simple_attribute, self, name, missing1, missing2);
            // ERROR RECOVERY: The = is missing; assume that the name belongs
            // to the attribute, but that the remainder is missing, and start
            // looking for the next attribute.
            Some(node)
        } else {
            let equal = S!(make_token, parser1, token);
            let mut parser2 = parser1.clone();
            let (token, _text) = parser2.next_xhp_element_token(false);
            match token.kind() {
                TokenKind::XHPStringLiteral => {
                    self.continue_from(parser2);
                    let token = S!(make_token, self, token);
                    let node = S!(make_xhp_simple_attribute, self, name, equal, token);
                    Some(node)
                }
                TokenKind::LeftBrace => {
                    self.continue_from(parser1);
                    let expr = self.parse_braced_expression();
                    let node = S!(make_xhp_simple_attribute, self, name, equal, expr);
                    Some(node)
                }
                _ => {
                    // ERROR RECOVERY: The expression is missing; assume that the "name ="
                    // belongs to the attribute and start looking for the next attribute.
                    self.continue_from(parser1);
                    self.with_error(Errors::error1017);
                    self.continue_from(parser2);
                    let missing = S!(make_missing, self, self.pos());
                    let node = S!(make_xhp_simple_attribute, self, name, equal, missing);
                    Some(node)
                }
            }
        }
    }

    fn parse_xhp_body_element(&mut self) -> Option<S::R> {
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_body_token();
        match token.kind() {
            TokenKind::XHPComment | TokenKind::XHPBody => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                Some(token)
            }
            TokenKind::LeftBrace => {
                let expr = self.parse_xhp_body_braced_expression();
                Some(expr)
            }
            TokenKind::RightBrace => {
                // If we find a free-floating right-brace in the middle of an XHP body
                // that's just fine. It's part of the text. However, it is also likely
                // to be a mis-edit, so we'll keep it as a right-brace token so that
                // tooling can flag it as suspicious.
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                Some(token)
            }
            TokenKind::LessThan => {
                self.continue_from(parser1);
                let expr = self.parse_possible_xhp_expression(/* ~in_xhp_body:*/ true, token);
                Some(expr)
            }
            _ => None,
        }
    }

    fn parse_xhp_close(&mut self, consume_trailing_trivia: bool) -> S::R {
        let (less_than_slash, _) = self.next_xhp_element_token(false);
        let less_than_slash_token_kind = less_than_slash.kind();
        let less_than_slash_token = S!(make_token, self, less_than_slash);
        if less_than_slash_token_kind == TokenKind::LessThanSlash {
            let mut parser1 = self.clone();
            let (name, _name_text) = parser1.next_xhp_element_token(false);
            if name.kind() == TokenKind::XHPElementName {
                let name_token = S!(make_token, parser1, name);
                // TODO: Check that the given and name_text are the same.
                let mut parser2 = parser1.clone();
                let (greater_than, _) = parser2.next_xhp_element_token(!consume_trailing_trivia);
                if greater_than.kind() == TokenKind::GreaterThan {
                    self.continue_from(parser2);
                    let greater_than_token = S!(make_token, self, greater_than);
                    S!(
                        make_xhp_close,
                        self,
                        less_than_slash_token,
                        name_token,
                        greater_than_token
                    )
                } else {
                    // ERROR RECOVERY:
                    self.continue_from(parser1);
                    self.with_error(Errors::error1039);
                    let missing = S!(make_missing, self, self.pos());
                    S!(
                        make_xhp_close,
                        self,
                        less_than_slash_token,
                        name_token,
                        missing
                    )
                }
            } else {
                // ERROR RECOVERY:
                self.with_error(Errors::error1039);
                let missing1 = S!(make_missing, self, self.pos());
                let missing2 = S!(make_missing, self, self.pos());
                S!(
                    make_xhp_close,
                    self,
                    less_than_slash_token,
                    missing1,
                    missing2
                )
            }
        } else {
            // ERROR RECOVERY: We probably got a < without a following / or name.
            // TODO: For now we'll just bail out. We could use a more
            // sophisticated strategy here.
            self.with_error(Errors::error1039);
            let missing1 = S!(make_missing, self, self.pos());
            let missing2 = S!(make_missing, self, self.pos());
            S!(
                make_xhp_close,
                self,
                less_than_slash_token,
                missing1,
                missing2
            )
        }
    }

    fn parse_xhp_expression(
        &mut self,
        consume_trailing_trivia: bool,
        left_angle: S::R,
        name: S::R,
    ) -> S::R {
        let attrs = self.parse_list_until_none(|p| p.parse_xhp_attribute());
        let mut parser1 = self.clone();
        let (token, _) = parser1.next_xhp_element_token(/*no_trailing:*/ true);
        match token.kind() {
            TokenKind::SlashGreaterThan => {
                let pos = self.pos();
                // We have determined that this is a self-closing XHP tag, so
                // `consume_trailing_trivia` needs to be propagated down.
                let (token, _) =
                    self.next_xhp_element_token(/* ~no_trailing:*/ !consume_trailing_trivia);
                let token = S!(make_token, self, token);
                let xhp_open = S!(make_xhp_open, self, left_angle, name, attrs, token);
                let missing1 = S!(make_missing, self, pos);
                let missing2 = S!(make_missing, self, pos);
                S!(make_xhp_expression, self, xhp_open, missing1, missing2)
            }
            TokenKind::GreaterThan => {
                // This is not a self-closing tag, so we are now in an XHP body context.
                // We can use the GreaterThan token as-is (i.e., lexed above with
                // ~no_trailing:true), since we don't want to lex trailing trivia inside
                // XHP bodies.
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                let xhp_open = S!(make_xhp_open, self, left_angle, name, attrs, token);
                let xhp_body = self.parse_list_until_none(|p| p.parse_xhp_body_element());
                let xhp_close = self.parse_xhp_close(consume_trailing_trivia);
                S!(make_xhp_expression, self, xhp_open, xhp_body, xhp_close)
            }
            _ => {
                // ERROR RECOVERY: Assume the unexpected token belongs to whatever
                // comes next.
                let missing = S!(make_missing, self, self.pos());
                let xhp_open = S!(make_xhp_open, self, left_angle, name, attrs, missing);
                let missing1 = S!(make_missing, parser1, self.pos());
                let missing2 = S!(make_missing, parser1, self.pos());
                self.continue_from(parser1);
                self.with_error(Errors::error1013);
                S!(make_xhp_expression, self, xhp_open, missing1, missing2)
            }
        }
    }

    fn parse_possible_xhp_expression(&mut self, in_xhp_body: bool, less_than: Token<S>) -> S::R {
        // We got a < token where an expression was expected.
        //println!("assert_xhp_body_token start {}|", self.lexer().offset_as_string());
        let less_than = S!(make_token, self, less_than);

        //println!("assert_xhp_body_token end {}|", self.lexer().offset_as_string());
        let mut parser1 = self.clone();

        let (name, _text) = parser1.next_xhp_element_token(false);
        if name.kind() == TokenKind::XHPElementName {
            self.continue_from(parser1);
            let token = S!(make_token, self, name);
            self.parse_xhp_expression(!in_xhp_body, less_than, token)
        } else {
            // ERROR RECOVERY
            // In an expression context, it's hard to say what to do here. We are
            // expecting an expression, so we could simply produce an error for the < and
            // call that the expression. Or we could assume the the left side of an
            // inequality is missing, give a missing node for the left side, and parse
            // the remainder as the right side. We'll go for the former for now.
            //
            // In an XHP body context, we certainly expect a name here, because the <
            // could only legally be the first token in another XHPExpression.
            let error = if in_xhp_body {
                Errors::error1004
            } else {
                Errors::error1015
            };
            self.with_error(error);
            less_than
        }
    }

    fn parse_anon_or_awaitable_or_scope_resolution_or_name(&mut self) -> S::R {
        // static is a legal identifier, if next token is scope resolution operatpr
        // - parse expresson as scope resolution operator, otherwise try to interpret
        // it as anonymous function (will fallback to name in case of failure)
        if self.peek_token_kind_with_lookahead(1) == TokenKind::ColonColon {
            self.parse_scope_resolution_or_name()
        } else {
            // allow_attribute_spec since we end up here after seeing static
            self.parse_anon_or_lambda_or_awaitable()
        }
    }

    fn parse_scope_resolution_or_name(&mut self) -> S::R {
        // parent, self and static are legal identifiers. If the next
        // thing that follows is a scope resolution operator, parse them as
        // ordinary tokens, and then we'll pick them up as the operand to the
        // scope resolution operator when we call parse_remaining_expression.
        // Otherwise, parse them as ordinary names.
        let mut parser1 = self.clone();
        let qualifier = parser1.next_token();
        if parser1.peek_token_kind() == TokenKind::ColonColon {
            self.continue_from(parser1);
            S!(make_token, self, qualifier)
        } else {
            let parent_or_self_or_static_as_name = self.next_token_as_name();
            S!(make_token, self, parent_or_self_or_static_as_name)
        }
    }

    fn parse_scope_resolution_expression(&mut self, qualifier: S::R) -> S::R {
        // SPEC
        // scope-resolution-expression:
        //   scope-resolution-qualifier  ::  name
        //   scope-resolution-qualifier  ::  class
        //
        // scope-resolution-qualifier:
        //   qualified-name
        //   variable-name
        //   self
        //   parent
        //   static
        //
        // TODO: The left hand side can in fact be any expression in this parser;
        // we need to add a later error pass to detect that the left hand side is
        // a valid qualifier.
        // TODO: The right hand side, if a name or a variable, is treated as a
        // name or variabletoken* and not a name or variable *expression*. Is
        // that the desired tree topology? Give this more thought; it might impact
        // rename refactoring semantics.
        let op = self.require_coloncolon();
        let name = {
            let mut parser1 = self.clone();
            let token = parser1.next_token();
            match token.kind() {
                TokenKind::Class => {
                    self.continue_from(parser1);
                    S!(make_token, self, token)
                }
                TokenKind::Dollar => self.parse_dollar_expression(false),
                TokenKind::LeftBrace => self.parse_braced_expression(),
                TokenKind::Variable if self.env.php5_compat_mode => {
                    let mut parser1 = self.clone();
                    let e = parser1.parse_variable_in_php5_compat_mode();
                    // for :: only do PHP5 transform for call expressions
                    // in other cases fall back to the regular parsing logic
                    if parser1.peek_token_kind() == TokenKind::LeftParen &&
                    // make sure the left parenthesis means a call
                    // for the expression we are currently parsing, and
                    // are not for example for a constructor call whose
                    // name would be the result of this expression.
                     !(self.operator_has_lower_precedence(TokenKind::LeftParen))
                    {
                        self.continue_from(parser1);
                        e
                    } else {
                        self.require_name_or_variable_or_error(Errors::error1048)
                    }
                }
                _ => self.require_name_or_variable_or_error(Errors::error1048),
            }
        };
        S!(make_scope_resolution_expression, self, qualifier, op, name)
    }

    fn parse_enum_class_label(&mut self, qualifier: S::R) -> S::R {
        let hash = self.assert_token(TokenKind::Hash);
        let label_name = self.require_name();
        S!(
            make_enum_class_label_expression,
            self,
            qualifier,
            hash,
            label_name
        )
    }
}
