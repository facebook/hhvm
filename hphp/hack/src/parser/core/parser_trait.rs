// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::lexer::{self, Lexer};
use crate::parser_env::ParserEnv;
use crate::smart_constructors::{NodeType, SmartConstructors};
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::lexable_trivia::LexableTrivia;
use parser_core_types::syntax_error::{self as Errors, Error, SyntaxError};
use parser_core_types::token_kind::TokenKind;
use stack_limit::StackLimit;

#[derive(PartialEq)]
pub enum SeparatedListKind {
    NoTrailing,
    TrailingAllowed,
    ItemsOptional,
}

// This could be a set of token kinds, but it's part of parser envirnoment that is often cloned,
// so trying to keep it small.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ExpectedTokens {
    Classish = 0b0001,
    Semicolon = 0b0010,
    RightParen = 0b0100,
    Visibility = 0b1000,
}
const ET_COUNT: u32 = 4;
const ET_MASK: ETMask = (1 << ET_COUNT) - 1;
type ETMask = u16; // mask of bits in first ET_COUNT bits

impl ExpectedTokens {
    pub fn contains(mask: ETMask, token: TokenKind) -> bool {
        use ExpectedTokens::*;
        let bit: ETMask = match token {
            TokenKind::Class | TokenKind::Trait | TokenKind::Interface => Classish as ETMask,
            TokenKind::Semicolon => Semicolon as ETMask,
            TokenKind::RightParen => RightParen as ETMask,
            TokenKind::Public | TokenKind::Protected | TokenKind::Private => Visibility as ETMask,
            _ => 0 as ETMask,
        };
        (bit & mask) != 0
    }

    fn from(bit: ETMask) -> ExpectedTokens {
        // debug_assert!((bit & (!bit+1)) == bit, "unexpected multiple set bits in {:#b}");
        use ExpectedTokens::*;
        match bit {
            0b0001 => Classish,
            0b0010 => Semicolon,
            0b0100 => RightParen,
            _ => Visibility,
        }
    }
}

#[derive(Debug, Clone)]
pub struct ExpectedTokenVec(Vec<ETMask>);
impl ExpectedTokenVec {
    fn push(&mut self, et: ExpectedTokens) {
        let last_mask = *self.0.last().unwrap_or(&0) & ET_MASK;
        let bit = et as ETMask;
        self.0.push(bit | last_mask | (bit << ET_COUNT));
    }
    fn pop(&mut self) -> Option<ExpectedTokens> {
        self.0.pop().map(|x| ExpectedTokens::from(x >> ET_COUNT))
    }
    fn last_mask(&self) -> ETMask {
        self.0.last().map_or(0, |x| x >> ET_COUNT)
    }
    fn any_mask(&self) -> ETMask {
        self.0.last().map_or(0, |x| x & ET_MASK)
    }
}

#[derive(Debug, Clone)]
pub struct Context<'a, T> {
    pub expected: ExpectedTokenVec,
    pub skipped_tokens: Vec<T>,
    stack_limit: Option<&'a StackLimit>,
}

impl<'a, T> Context<'a, T> {
    pub fn empty(stack_limit: Option<&'a StackLimit>) -> Self {
        Self {
            expected: ExpectedTokenVec(vec![]),
            skipped_tokens: vec![],
            stack_limit,
        }
    }

    fn expect_in_new_scope(&mut self, expected: ExpectedTokens) {
        self.expected.push(expected);
    }

    fn pop_scope(&mut self, expected: ExpectedTokens) {
        let scope = self.expected.pop().unwrap();
        assert_eq!(expected, scope)
    }

    fn expects(&self, token_kind: TokenKind) -> bool {
        ExpectedTokens::contains(self.expected.any_mask(), token_kind)
    }

    fn expects_here(&self, token_kind: TokenKind) -> bool {
        ExpectedTokens::contains(self.expected.last_mask(), token_kind)
    }
}

pub trait ParserTrait<'a, S, T: Clone>: Clone
where
    S: SmartConstructors<T>,
    <S as SmartConstructors<T>>::R: NodeType,
{
    fn make(
        _: Lexer<'a, S::Token>,
        _: ParserEnv,
        _: Context<'a, S::Token>,
        _: Vec<SyntaxError>,
        _: S,
    ) -> Self;
    fn add_error(&mut self, _: SyntaxError);
    fn into_parts(
        self,
    ) -> (
        Lexer<'a, S::Token>,
        Context<'a, S::Token>,
        Vec<SyntaxError>,
        S,
    );
    fn lexer(&self) -> &Lexer<'a, S::Token>;
    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Token>;
    fn continue_from<P: ParserTrait<'a, S, T>>(&mut self, _: P)
    where
        T: Clone;

    fn env(&self) -> &ParserEnv;

    fn sc_mut(&mut self) -> &mut S;

    fn skipped_tokens(&self) -> &[S::Token];
    fn drain_skipped_tokens(&mut self) -> std::vec::Drain<S::Token>;

    fn context_mut(&mut self) -> &mut Context<'a, S::Token>;
    fn context(&self) -> &Context<'a, S::Token>;

    fn pos(&self) -> usize {
        self.lexer().offset()
    }

    fn add_skipped_token(&mut self, token: S::Token) {
        self.context_mut().skipped_tokens.push(token)
    }

    fn expects(&self, kind: TokenKind) -> bool {
        self.context().expects(kind)
    }

    fn expects_here(&self, kind: TokenKind) -> bool {
        self.context().expects_here(kind)
    }

    fn expect_in_new_scope(&mut self, expected: ExpectedTokens) {
        self.context_mut().expect_in_new_scope(expected)
    }

    fn pop_scope(&mut self, expected: ExpectedTokens) {
        self.context_mut().pop_scope(expected)
    }

    // This function reports an error starting at the current location of the
    // parser. Setting on_whole_token=false will report the error only on trivia,
    // which is useful in cases such as when "a semicolon is expected here" before
    // the current node. However, setting on_whole_token=true will report the error
    // only on the non-trivia text of the next token parsed, which is useful
    // in cases like "flagging an entire token as an extra".
    fn with_error_impl(&mut self, on_whole_token: bool, message: Error) {
        let (start_offset, end_offset) = self.error_offsets(on_whole_token);
        let error = SyntaxError::make(start_offset, end_offset, message);
        self.add_error(error)
    }

    fn with_error(&mut self, message: Error) {
        self.with_error_impl(false, message)
    }

    fn with_error_on_whole_token(&mut self, message: Error) {
        self.with_error_impl(true, message)
    }

    fn next_token_with_tokenizer<F>(&mut self, tokenizer: F) -> S::Token
    where
        F: Fn(&mut Lexer<'a, S::Token>) -> S::Token,
    {
        let token = tokenizer(self.lexer_mut());
        if !self.skipped_tokens().is_empty() {
            // SourceText is just an Rc, so this clone is just a refcount bump.
            let source = self.lexer().source().clone();
            let start = self.lexer().start();
            let mut leading = <S::Token as LexableToken>::Trivia::new();
            for t in self.drain_skipped_tokens() {
                let (t_leading, t_width, t_trailing) = t.into_trivia_and_width();
                leading.extend(t_leading);
                leading.push(<S::Token as LexableToken>::Trivia::make_extra_token_error(
                    &source, start, t_width,
                ));
                leading.extend(t_trailing);
            }
            leading.extend(token.clone_leading());
            token.with_leading(leading)
        } else {
            token
        }
    }

    fn next_token(&mut self) -> S::Token {
        self.next_token_with_tokenizer(|x| x.next_token())
    }

    fn next_token_no_trailing(&mut self) -> S::Token {
        self.lexer_mut().next_token_no_trailing()
    }

    fn next_docstring_header(&mut self) -> (S::Token, &'a [u8]) {
        self.lexer_mut().next_docstring_header()
    }

    fn next_token_in_string(&mut self, literal_kind: &lexer::StringLiteralKind) -> S::Token {
        self.lexer_mut().next_token_in_string(literal_kind)
    }

    fn next_xhp_class_name_or_other(&mut self) -> S::R {
        let token = self.next_xhp_class_name_or_other_token();
        match token.kind() {
            TokenKind::Namespace | TokenKind::Name => {
                let name_token = S!(make_token, self, token);
                self.scan_remaining_qualified_name(name_token)
            }
            TokenKind::Backslash => {
                let missing = S!(make_missing, self, self.pos());
                let backslash = S!(make_token, self, token);
                self.scan_qualified_name(missing, backslash)
            }
            _ => S!(make_token, self, token),
        }
    }

    fn next_xhp_children_name_or_other(&mut self) -> S::Token {
        if self.is_next_xhp_category_name() {
            self.next_xhp_category_name()
        } else if self.env().enable_xhp_class_modifier {
            self.next_xhp_modifier_class_name_or_other_token()
        } else {
            self.next_xhp_class_name_or_other_token()
        }
    }

    // Used in conjunction with the following function. If you call next_token
    // when the parser is at the <<<, it will scan the entire file looking for an
    // ending to the heredoc, which could quickly get bad if there are many such
    // declarations in a file.
    fn peek_next_partial_token_is_triple_left_angle(&self) -> bool {
        let mut lexer = self.lexer().clone();
        lexer.scan_leading_php_trivia();
        let tparam_open = lexer.peek_char(0);
        let attr1 = lexer.peek_char(1);
        let attr2 = lexer.peek_char(2);
        tparam_open == '<' && attr1 == '<' && attr2 == '<'
    }

    // Type parameter/argument lists begin with < and can have attributes immediately
    // afterwards, so this peeks a token kind at the beginning of such a list. *)
    fn peek_token_kind_with_possible_attributized_type_list(&self) -> TokenKind {
        if self.peek_next_partial_token_is_triple_left_angle() {
            TokenKind::LessThan
        } else {
            self.peek_token_kind()
        }
    }

    // In the case of attributes on generics, one could write
    // function f<<<__Attr>> reify T, ...> or Awaitable<<<__Soft>> int>
    // The triple left angle is currently lexed as a HeredocStringLiteral,
    // but we can get around this by manually advancing the lexer one token
    // and returning a LeftAngle. Then, the next token will be a LeftAngleLeftAngle
    fn assert_left_angle_in_type_list_with_possible_attribute(&mut self) -> S::R {
        let parser1 = self.clone();
        let lexer = self.lexer_mut();
        lexer.scan_leading_php_trivia();
        let tparam_open = lexer.peek_char(0);
        let attr1 = lexer.peek_char(1);
        let attr2 = lexer.peek_char(2);
        if tparam_open == '<' && attr1 == '<' && attr2 == '<' {
            lexer.advance(1);
            let token = S::Token::make(
                TokenKind::LessThan,
                lexer.start(),
                1,
                <S::Token as LexableToken>::Trivia::new(),
                <S::Token as LexableToken>::Trivia::new(),
            );
            S!(make_token, self, token)
        } else {
            self.continue_from(parser1);
            self.assert_token(TokenKind::LessThan)
        }
    }

    fn assert_xhp_body_token(&mut self, kind: TokenKind) -> S::R {
        self.assert_token_with_tokenizer(kind, |x: &mut Lexer<'a, S::Token>| {
            x.next_xhp_body_token()
        })
    }

    fn peek_token_with_lookahead(&self, lookahead: usize) -> S::Token {
        let mut lexer = self.lexer().clone();
        let mut i = 0;
        loop {
            if i == lookahead {
                // call peek_next_token instead of next_token for the last one to leverage
                // lexer caching
                return lexer.peek_next_token();
            }
            let _ = lexer.next_token();
            i += 1
        }
    }

    fn peek_token(&self) -> S::Token {
        self.lexer().peek_next_token()
    }

    fn peek_token_kind(&self) -> TokenKind {
        self.peek_token().kind()
    }

    fn peek_token_kind_with_lookahead(&self, lookahead: usize) -> TokenKind {
        self.peek_token_with_lookahead(lookahead).kind()
    }

    fn fetch_token(&mut self) -> S::R {
        let token = self.lexer_mut().next_token();
        S!(make_token, self, token)
    }

    fn assert_token_with_tokenizer<F>(&mut self, kind: TokenKind, tokenizer: F) -> S::R
    where
        F: Fn(&mut Lexer<'a, S::Token>) -> S::Token,
    {
        let token = self.next_token_with_tokenizer(tokenizer);
        if token.kind() != kind {
            panic!(
                "Expected {:?}, but got {:?}. This indicates a bug in the parser, regardless of how broken the input code is.",
                kind,
                token.kind()
            )
        }
        S!(make_token, self, token)
    }

    fn assert_token(&mut self, kind: TokenKind) -> S::R {
        self.assert_token_with_tokenizer(kind, |x: &mut Lexer<S::Token>| x.next_token())
    }

    fn token_text(&self, token: &S::Token) -> &'a str {
        match token.leading_start_offset() {
            None => "", // unavailable for minimal tokens
            Some(leading_start_offset) => unsafe {
                std::str::from_utf8_unchecked(
                    self.lexer()
                        .source()
                        .sub(leading_start_offset + token.leading_width(), token.width()),
                )
            },
        }
    }

    fn current_token_text(&self) -> &'a str {
        self.token_text(&self.peek_token())
    }
    // If the next token is a name or keyword, scan it as a name.
    fn next_token_as_name(&mut self) -> S::Token {
        // TODO: This isn't right.  Pass flags to the lexer.
        self.lexer_mut().next_token_as_name()
    }

    fn optional_token(&mut self, kind: TokenKind) -> S::R {
        if self.peek_token_kind() == kind {
            let token = self.next_token();
            S!(make_token, self, token)
        } else {
            S!(make_missing, self, self.pos())
        }
    }

    fn scan_qualified_name_worker(
        &mut self,
        mut name_opt: Option<S::R>,
        mut parts: Vec<S::R>,
        mut has_backslash: bool,
    ) -> (Vec<S::R>, Option<S::R>, bool) {
        loop {
            let mut parser1 = self.clone();
            let token = if parser1.is_next_xhp_class_name() {
                parser1.next_xhp_class_name()
            } else {
                parser1.next_token_as_name()
            };
            match (name_opt.is_some(), token.kind()) {
                (true, TokenKind::Backslash) => {
                    // found backslash, create item and recurse
                    self.continue_from(parser1);
                    let token = S!(make_token, self, token);
                    let part = S!(make_list_item, self, name_opt.unwrap(), token);
                    parts.push(part);
                    has_backslash = true;
                    name_opt = None;
                }
                (false, TokenKind::Name) => {
                    // found a name, recurse to look for backslash
                    self.continue_from(parser1);
                    let token = S!(make_token, self, token);
                    name_opt = Some(token);
                    has_backslash = false;
                }
                (true, _) if parts.is_empty() => {
                    // have not found anything - return [] to indicate failure
                    return (parts, name_opt, false);
                }
                (true, _) => {
                    // next token is not part of qualified name but we've consume some
                    // part of the input - create part for name with missing backslash
                    // and return accumulated result
                    let missing = S!(make_missing, self, self.pos());
                    let part = S!(make_list_item, self, name_opt.unwrap(), missing);
                    // TODO(T25649779)
                    parts.push(part);
                    return (parts, None, false);
                }
                _ => {
                    // next token is not part of qualified name - return accumulated result
                    return (parts, name_opt, has_backslash);
                }
            }
        }
    }

    fn scan_remaining_qualified_name_extended(&mut self, name_token: S::R) -> (S::R, bool) {
        let (parts, name_token_opt, is_backslash) =
            self.scan_qualified_name_worker(Some(name_token), vec![], false);
        if parts.is_empty() {
            (name_token_opt.unwrap(), is_backslash)
        } else {
            let list_node = S!(make_list, self, parts, self.pos());
            let name = S!(make_qualified_name, self, list_node);
            (name, is_backslash)
        }
    }

    fn scan_qualified_name_extended(&mut self, missing: S::R, backslash: S::R) -> (S::R, bool) {
        let head = S!(make_list_item, self, missing, backslash);
        let parts = vec![head];
        let (parts, _, is_backslash) = self.scan_qualified_name_worker(None, parts, false);
        let list_node = S!(make_list, self, parts, self.pos());
        let name = S!(make_qualified_name, self, list_node);
        (name, is_backslash)
    }

    fn scan_qualified_name(&mut self, missing: S::R, backslash: S::R) -> S::R {
        let (name, _) = self.scan_qualified_name_extended(missing, backslash);
        name
    }
    // If the next token is a name or an non-reserved keyword, scan it as
    // a name otherwise as a keyword.
    //
    // NB: A "reserved" keyword is in practice a keyword that cannot be used
    // as a class name or function name, for example, control flow keywords or
    // declaration keywords are reserved.
    fn next_token_non_reserved_as_name(&mut self) -> S::Token {
        // TODO: This isn't right.  Pass flags to the lexer.
        self.lexer_mut().next_token_non_reserved_as_name()
    }

    fn scan_header(&mut self) -> (S::Token, Option<(S::Token, Option<S::Token>)>) {
        self.lexer_mut().scan_header()
    }

    fn error_offsets(&mut self, on_whole_token: bool /* = false */) -> (usize, usize) {
        if on_whole_token {
            let token = self.peek_token();
            let start_offset = self.lexer().offset() + token.leading_width();
            let end_offset = start_offset + token.width();
            (start_offset, end_offset)
        } else {
            let start_offset = self.lexer().start();
            let end_offset = self.lexer().offset();
            (start_offset, end_offset)
        }
    }

    fn scan_name_or_qualified_name(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_token_non_reserved_as_name();
        match token.kind() {
            TokenKind::Namespace | TokenKind::Name => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                self.scan_remaining_qualified_name(token)
            }
            TokenKind::Backslash => {
                self.continue_from(parser1);

                let missing = S!(make_missing, self, self.pos());
                let token = S!(make_token, self, token);
                self.scan_qualified_name(missing, token)
            }
            _ => S!(make_missing, self, self.pos()),
        }
    }

    fn parse_alternate_if_block<F>(&mut self, parse_item: F) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        let mut parser1 = self.clone();
        let block = parser1.parse_list_while(parse_item, |x: &Self| match x.peek_token_kind() {
            TokenKind::Elseif | TokenKind::Else | TokenKind::Endif => false,
            _ => true,
        });
        if block.is_missing() {
            let empty1 = S!(make_missing, self, self.pos());
            let empty2 = S!(make_missing, self, self.pos());
            let es = S!(make_expression_statement, self, empty1, empty2);
            S!(make_list, self, vec![es], self.pos())
        } else {
            self.continue_from(parser1);
            block
        }
    }

    fn parse_separated_list<F>(
        &mut self,
        separator_kind: TokenKind,
        allow_trailing: SeparatedListKind,
        close_kind: TokenKind,
        error: Error,
        parse_item: F,
    ) -> (S::R, bool)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let (x, y, _) = self.parse_separated_list_predicate(
            |x| x == separator_kind,
            allow_trailing,
            |x| x == close_kind,
            error,
            parse_item,
        );
        (x, y)
    }

    fn require_qualified_name(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let name = if parser1.is_next_xhp_class_name() {
            parser1.next_xhp_class_name()
        } else {
            parser1.next_token_non_reserved_as_name()
        };

        match name.kind() {
            TokenKind::Namespace | TokenKind::Name | TokenKind::XHPClassName => {
                self.continue_from(parser1);
                let token = S!(make_token, self, name);
                self.scan_remaining_qualified_name(token)
            }
            TokenKind::Backslash => {
                self.continue_from(parser1);
                let missing = S!(make_missing, self, self.pos());
                let backslash = S!(make_token, self, name);
                self.scan_qualified_name(missing, backslash)
            }
            _ => {
                self.with_error(Errors::error1004);
                S!(make_missing, self, self.pos())
            }
        }
    }

    fn require_name(&mut self) -> S::R {
        self.require_token(TokenKind::Name, Errors::error1004)
    }

    fn require_xhp_class_name(&mut self) -> S::R {
        let token = self.next_xhp_modifier_class_name();
        S!(make_token, self, token)
    }

    fn require_xhp_class_name_or_name(&mut self) -> S::R {
        if self.is_next_xhp_class_name() {
            let token = self.next_xhp_class_name();
            S!(make_token, self, token)
        } else {
            self.require_token(TokenKind::Name, Errors::error1004)
        }
    }

    fn require_class_name(&mut self) -> S::R {
        if self.is_next_xhp_class_name() {
            let token = self.next_xhp_class_name();
            S!(make_token, self, token)
        } else {
            self.require_name_allow_non_reserved()
        }
    }

    fn require_function(&mut self) -> S::R {
        self.require_token(TokenKind::Function, Errors::error1003)
    }

    fn require_variable(&mut self) -> S::R {
        self.require_token(TokenKind::Variable, Errors::error1008)
    }

    fn require_colon(&mut self) -> S::R {
        self.require_token(TokenKind::Colon, Errors::error1020)
    }

    fn require_left_brace(&mut self) -> S::R {
        self.require_token(TokenKind::LeftBrace, Errors::error1034)
    }

    fn require_slashgt(&mut self) -> S::R {
        self.require_token(TokenKind::SlashGreaterThan, Errors::error1029)
    }

    fn require_right_brace(&mut self) -> S::R {
        self.require_token(TokenKind::RightBrace, Errors::error1006)
    }

    fn require_left_paren(&mut self) -> S::R {
        self.require_token(TokenKind::LeftParen, Errors::error1019)
    }

    fn require_left_angle(&mut self) -> S::R {
        self.require_token(TokenKind::LessThan, Errors::error1021)
    }

    fn require_right_angle(&mut self) -> S::R {
        self.require_token(TokenKind::GreaterThan, Errors::error1013)
    }

    fn require_comma(&mut self) -> S::R {
        self.require_token(TokenKind::Comma, Errors::error1054)
    }

    fn require_right_bracket(&mut self) -> S::R {
        self.require_token(TokenKind::RightBracket, Errors::error1032)
    }

    fn require_equal(&mut self) -> S::R {
        self.require_token(TokenKind::Equal, Errors::error1036)
    }

    fn require_arrow(&mut self) -> S::R {
        self.require_token(TokenKind::EqualGreaterThan, Errors::error1028)
    }

    fn require_lambda_arrow(&mut self) -> S::R {
        self.require_token(TokenKind::EqualEqualGreaterThan, Errors::error1046)
    }

    fn require_as(&mut self) -> S::R {
        self.require_token(TokenKind::As, Errors::error1023)
    }

    fn require_while(&mut self) -> S::R {
        self.require_token(TokenKind::While, Errors::error1018)
    }

    fn require_coloncolon(&mut self) -> S::R {
        self.require_token(TokenKind::ColonColon, Errors::error1047)
    }

    fn require_name_or_variable_or_error(&mut self, error: Error) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_token_as_name();
        match token.kind() {
            TokenKind::Namespace | TokenKind::Name => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                self.scan_remaining_qualified_name(token)
            }
            TokenKind::Variable => {
                self.continue_from(parser1);
                S!(make_token, self, token)
            }
            _ => {
                // ERROR RECOVERY: Create a missing token for the expected token,
                // and continue on from the current token. Don't skip it.
                self.with_error(error);
                S!(make_missing, self, self.pos())
            }
        }
    }

    fn require_name_or_variable(&mut self) -> S::R {
        self.require_name_or_variable_or_error(Errors::error1050)
    }

    fn require_xhp_class_name_or_name_or_variable(&mut self) -> S::R {
        if self.is_next_xhp_class_name() {
            let token = self.next_xhp_class_name();
            S!(make_token, self, token)
        } else {
            self.require_name_or_variable()
        }
    }

    fn require_colonat(&mut self) -> S::R {
        self.require_token(TokenKind::ColonAt, Errors::error1061)
    }

    fn require_name_allow_non_reserved(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_token_non_reserved_as_name();
        if token.kind() == TokenKind::Name {
            self.continue_from(parser1);
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Create a missing token for the expected token,
            // and continue on from the current token. Don't skip it.
            self.with_error(Errors::error1004);
            S!(make_missing, self, self.pos())
        }
    }

    fn next_xhp_category_name(&mut self) -> S::Token {
        self.lexer_mut().next_xhp_category_name()
    }

    // We have a number of issues involving xhp class names, which begin with
    // a colon and may contain internal colons and dashes.  These are some
    // helper methods to deal with them.
    fn is_next_name(&mut self) -> bool {
        self.lexer().is_next_name()
    }

    fn next_xhp_name(&mut self) -> S::Token {
        assert!(self.is_next_name());
        self.lexer_mut().next_xhp_name()
    }

    fn next_xhp_class_name(&mut self) -> S::Token {
        assert!(self.is_next_xhp_class_name());
        self.lexer_mut().next_xhp_class_name()
    }

    fn next_xhp_modifier_class_name(&mut self) -> S::Token {
        self.lexer_mut().next_xhp_modifier_class_name()
    }

    fn require_xhp_name(&mut self) -> S::R {
        if self.is_next_name() {
            let token = self.next_xhp_name();
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Create a missing token for the expected token,
            // and continue on from the current token. Don't skip it.
            // TODO: Different error?
            self.with_error(Errors::error1004);
            S!(make_missing, self, self.pos())
        }
    }

    fn is_next_xhp_category_name(&mut self) -> bool {
        self.lexer().is_next_xhp_category_name()
    }

    fn parse_comma_list_allow_trailing<F>(
        &mut self,
        close_predicate: TokenKind,
        error: Error,
        parse_item: F,
    ) -> (S::R, bool)
    where
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list(
            TokenKind::Comma,
            SeparatedListKind::TrailingAllowed,
            close_predicate,
            error,
            parse_item,
        )
    }

    fn parse_separated_list_predicate<P, SP, F>(
        &mut self,
        separator_predicate: SP,
        list_kind: SeparatedListKind,
        close_predicate: P,
        error: Error,
        parse_item: F,
    ) -> (S::R, bool, TokenKind)
    where
        P: Fn(TokenKind) -> bool,
        SP: Fn(TokenKind) -> bool,
        F: Fn(&mut Self) -> S::R,
    {
        let mut items = vec![];
        // Set this when we first see a separator
        let mut separator_kind = TokenKind::Empty;

        loop {
            // At this point we are expecting an item followed by a separator,
            // a close, or, if trailing separators are allowed, both
            let kind = self.peek_token_kind();
            if close_predicate(kind) || kind == TokenKind::EndOfFile {
                // ERROR RECOVERY: We expected an item but we found a close or
                // the end of the file. Make the item and separator both
                // "missing" and give an error.
                //
                // If items are optional and we found a close, the last item was
                // omitted and there was no error.

                if kind == TokenKind::EndOfFile || list_kind != SeparatedListKind::ItemsOptional {
                    self.with_error(error)
                };
                let missing1 = S!(make_missing, self, self.pos());
                let missing2 = S!(make_missing, self, self.pos());
                let list_item = S!(make_list_item, self, missing1, missing2);
                // TODO(T25649779)
                items.push(list_item);
                break;
            } else if separator_predicate(kind) {
                if separator_kind == TokenKind::Empty {
                    separator_kind = kind;
                } else if separator_kind != kind {
                    self.with_error(Errors::error1063);
                }

                // ERROR RECOVERY: We expected an item but we got a separator.
                // Assume the item was missing, eat the separator, and move on.
                //
                // If items are optional, there was no error, so eat the separator and
                // continue.
                //
                // TODO: This could be poor recovery. For example:
                //
                //     function bar (Foo< , int blah)
                //
                // Plainly the type arg is missing, but the comma is not associated with
                // the type argument list, it's associated with the formal
                // parameter list.

                let token = self.next_token();
                if list_kind != SeparatedListKind::ItemsOptional {
                    self.with_error(error.clone())
                }
                let item = S!(make_missing, self, self.pos());
                let separator = S!(make_token, self, token);
                let list_item = S!(make_list_item, self, item, separator);
                // TODO(T25649779)
                items.push(list_item)
            } else {
                // We got neither a close nor a separator; hopefully we're going
                // to parse an item followed by a close or separator.
                let item = parse_item(self);
                let kind = self.peek_token_kind();

                if close_predicate(kind) {
                    let missing = S!(make_missing, self, self.pos());
                    let list_item = S!(make_list_item, self, item, missing);
                    // TODO(T25649779)
                    items.push(list_item);
                    break;
                } else if separator_predicate(kind) {
                    if separator_kind == TokenKind::Empty {
                        separator_kind = kind;
                    } else {
                        if separator_kind != kind {
                            self.with_error(Errors::error1063);
                        }
                    }
                    let token = self.next_token();

                    let separator = S!(make_token, self, token);
                    let list_item = S!(make_list_item, self, item, separator);
                    // TODO(T25649779)
                    items.push(list_item);
                    let allow_trailing = list_kind != SeparatedListKind::NoTrailing;
                    // We got an item followed by a separator; what if the thing
                    // that comes next is a close?
                    if allow_trailing && close_predicate(self.peek_token_kind()) {
                        break;
                    }
                } else {
                    // ERROR RECOVERY: We were expecting a close or separator, but
                    // got neither. Bail out. Caller will give an error.
                    let missing = S!(make_missing, self, self.pos());
                    let list_item = S!(make_list_item, self, item, missing);
                    // TODO(T25649779)
                    items.push(list_item);
                    break;
                }
            }
        }
        let no_arg_is_missing = items.iter().all(|x| !x.is_missing());
        let item_list = S!(make_list, self, items, self.pos());
        (item_list, no_arg_is_missing, separator_kind)
    }

    fn parse_list_until_none<F>(&mut self, parse_item: F) -> S::R
    where
        F: Fn(&mut Self) -> Option<S::R>,
    {
        let mut acc = vec![];
        loop {
            let maybe_item = parse_item(self);
            match maybe_item {
                None => break,
                Some(item) => {
                    let is_missing = item.is_missing();
                    acc.push(item);
                    if self.peek_token_kind() == TokenKind::EndOfFile ||
                    // exit if parser did not make any progress
                        is_missing
                    {
                        break;
                    }
                }
            }
        }
        S!(make_list, self, acc, self.pos())
    }

    fn parse_separated_list_opt_predicate<P, F>(
        &mut self,
        separator_kind: TokenKind,
        allow_trailing: SeparatedListKind,
        close_predicate: P,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        P: Fn(TokenKind) -> bool,
        F: Fn(&mut Self) -> S::R,
    {
        let kind = self.peek_token_kind();
        if close_predicate(kind) {
            S!(make_missing, self, self.pos())
        } else {
            let (items, _, _) = self.parse_separated_list_predicate(
                |x| x == separator_kind,
                allow_trailing,
                close_predicate,
                error,
                parse_item,
            );
            items
        }
    }

    fn is_next_xhp_class_name(&self) -> bool {
        self.lexer().is_next_xhp_class_name()
    }

    fn next_xhp_modifier_class_name_or_other_token(&mut self) -> S::Token {
        if self.is_next_name() {
            self.next_xhp_modifier_class_name()
        } else {
            self.next_token()
        }
    }

    fn next_xhp_class_name_or_other_token(&mut self) -> S::Token {
        if self.is_next_xhp_class_name() {
            self.next_xhp_class_name()
        } else {
            self.next_token()
        }
    }

    fn parse_separated_list_opt<F>(
        &mut self,
        separator_kind: TokenKind,
        allow_trailing: SeparatedListKind,
        close_kind: TokenKind,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list_opt_predicate(
            separator_kind,
            allow_trailing,
            |x| x == close_kind,
            error,
            parse_item,
        )
    }

    fn parse_comma_list_opt_allow_trailing<F>(
        &mut self,
        close_kind: TokenKind,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list_opt(
            TokenKind::Comma,
            SeparatedListKind::TrailingAllowed,
            close_kind,
            error,
            parse_item,
        )
    }

    fn parse_comma_list_opt<F>(
        &mut self,
        close_kind: TokenKind,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list_opt(
            TokenKind::Comma,
            SeparatedListKind::NoTrailing,
            close_kind,
            error,
            parse_item,
        )
    }

    fn parse_comma_list_opt_items_opt<F>(
        &mut self,
        close_kind: TokenKind,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list_opt(
            TokenKind::Comma,
            SeparatedListKind::ItemsOptional,
            close_kind,
            error,
            parse_item,
        )
    }

    fn parse_comma_list_opt_allow_trailing_predicate<P, F>(
        &mut self,
        close_kind: P,
        error: Error,
        parse_item: F,
    ) -> S::R
    where
        P: Fn(TokenKind) -> bool,
        F: Fn(&mut Self) -> S::R,
    {
        self.parse_separated_list_opt_predicate(
            TokenKind::Comma,
            SeparatedListKind::TrailingAllowed,
            close_kind,
            error,
            parse_item,
        )
    }

    fn parse_comma_list<F>(&mut self, close_kind: TokenKind, error: Error, parse_item: F) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        let (items, _) = self.parse_separated_list(
            TokenKind::Comma,
            SeparatedListKind::NoTrailing,
            close_kind,
            error,
            parse_item,
        );
        items
    }

    fn parse_delimited_list<P>(
        &mut self,
        left_kind: TokenKind,
        left_error: Error,
        right_kind: TokenKind,
        right_error: Error,
        parse_items: P,
    ) -> (S::R, S::R, S::R)
    where
        P: FnOnce(&mut Self) -> S::R,
    {
        let left = self.require_token(left_kind, left_error);
        let items = parse_items(self);
        let right = self.require_token(right_kind, right_error);
        (left, items, right)
    }

    fn parse_braced_list<P>(&mut self, parse_items: P) -> (S::R, S::R, S::R)
    where
        P: FnOnce(&mut Self) -> S::R,
    {
        self.parse_delimited_list(
            TokenKind::LeftBrace,
            Errors::error1034,
            TokenKind::RightBrace,
            Errors::error1006,
            parse_items,
        )
    }

    fn parse_parenthesized_list<F>(&mut self, parse_items: F) -> (S::R, S::R, S::R)
    where
        F: FnOnce(&mut Self) -> S::R,
    {
        self.parse_delimited_list(
            TokenKind::LeftParen,
            Errors::error1019,
            TokenKind::RightParen,
            Errors::error1011,
            parse_items,
        )
    }

    fn parse_parenthesized_comma_list<F>(&mut self, parse_item: F) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items =
            |x: &mut Self| x.parse_comma_list(TokenKind::RightParen, Errors::error1011, parse_item);

        self.parse_parenthesized_list(parse_items)
    }

    fn parse_parenthesized_comma_list_opt_allow_trailing<F>(
        &mut self,
        parse_item: F,
    ) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items = |x: &mut Self| {
            x.parse_comma_list_opt_allow_trailing(
                TokenKind::RightParen,
                Errors::error1011,
                parse_item,
            )
        };

        self.parse_parenthesized_list(parse_items)
    }

    fn parse_parenthesized_comma_list_opt_items_opt<F>(
        &mut self,
        parse_item: F,
    ) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items = |x: &mut Self| {
            x.parse_comma_list_opt_items_opt(TokenKind::RightParen, Errors::error1011, parse_item)
        };

        self.parse_parenthesized_list(parse_items)
    }

    fn parse_braced_comma_list_opt_allow_trailing<F>(&mut self, parse_item: F) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items = |parser: &mut Self| {
            parser.parse_comma_list_opt_allow_trailing(
                TokenKind::RightBrace,
                Errors::error1006,
                parse_item,
            )
        };
        self.parse_braced_list(parse_items)
    }

    fn parse_bracketted_list<F>(&mut self, parse_items: F) -> (S::R, S::R, S::R)
    where
        F: FnOnce(&mut Self) -> S::R,
    {
        self.parse_delimited_list(
            TokenKind::LeftBracket,
            Errors::error1026,
            TokenKind::RightBracket,
            Errors::error1031,
            parse_items,
        )
    }

    fn parse_bracketted_comma_list_opt_allow_trailing<F>(
        &mut self,
        parse_item: F,
    ) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items = |x: &mut Self| {
            x.parse_comma_list_opt_allow_trailing(
                TokenKind::RightBracket,
                Errors::error1031,
                parse_item,
            )
        };
        self.parse_bracketted_list(parse_items)
    }

    fn parse_double_angled_list<F>(&mut self, parse_items: F) -> (S::R, S::R, S::R)
    where
        F: FnOnce(&mut Self) -> S::R,
    {
        self.parse_delimited_list(
            TokenKind::LessThanLessThan,
            Errors::error1029,
            TokenKind::GreaterThanGreaterThan,
            Errors::error1029,
            parse_items,
        )
    }

    fn parse_double_angled_comma_list_allow_trailing<F>(
        &mut self,
        parse_item: F,
    ) -> (S::R, S::R, S::R)
    where
        F: Fn(&mut Self) -> S::R,
    {
        let parse_items = |x: &mut Self| {
            let (items, _) = x.parse_comma_list_allow_trailing(
                TokenKind::GreaterThanGreaterThan,
                Errors::error1029,
                parse_item,
            );
            items
        };
        self.parse_double_angled_list(parse_items)
    }

    fn scan_remaining_qualified_name(&mut self, name_token: S::R) -> S::R {
        let (name, _) = self.scan_remaining_qualified_name_extended(name_token);
        name
    }

    // Parse with parse_item while a condition is met.
    fn parse_list_while<F, P>(&mut self, parse_item: F, predicate: P) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
        P: Fn(&Self) -> bool,
    {
        let mut items = vec![];
        loop {
            if self.peek_token_kind() == TokenKind::EndOfFile || !predicate(self) {
                break;
            };

            let lexer_before = self.lexer().clone();
            let result = parse_item(self);
            // ERROR RECOVERY: If the item is was parsed as 'missing', then it means
            // the parser bailed out of that scope. So, pass on whatever's been
            // accumulated so far, but with a 'Missing' SyntaxNode prepended.
            if result.is_missing() {
                items.push(result);
                break;
            } else if lexer_before.start() == self.lexer().start()
                && lexer_before.offset() == self.lexer().offset()
            {
                // INFINITE LOOP PREVENTION: If parse_item does not actually make
                // progress, just bail
                items.push(result);
                break;
            } else {
                // Or if nothing's wrong, continue.
                items.push(result)
            }
        }
        S!(make_list, self, items, self.pos())
    }

    fn parse_terminated_list<F>(&mut self, parse_item: F, terminator: TokenKind) -> S::R
    where
        F: Fn(&mut Self) -> S::R,
    {
        let predicate = |x: &Self| x.peek_token_kind() != terminator;
        self.parse_list_while(parse_item, predicate)
    }

    fn skip_and_log_unexpected_token(&mut self, generate_error: bool) {
        if generate_error {
            let extra_str = &self.current_token_text();
            self.with_error_on_whole_token(Errors::error1057(extra_str))
        };
        let token = self.next_token();
        self.add_skipped_token(token)
    }

    // Returns true if the strings underlying two tokens are of the same length
    // but with one character different.
    fn one_character_different<'b>(str1: &'b [u8], str2: &'b [u8]) -> bool {
        if str1.len() != str2.len() {
            false
        } else {
            // both strings have same length
            let str_len = str1.len();
            for i in 0..str_len {
                if str1[i] != str2[i] {
                    // Allow only one mistake
                    return str1[i + 1..] == str2[i + 1..];
                }
            }
            true
        }
    }

    // Compare the text of the token we have in hand to the text of the
    // anticipated kind. Note: this automatically returns false for any
    // TokenKinds of length 1.
    fn is_misspelled_kind(kind: TokenKind, token_str: &str) -> bool {
        let tokenkind_str = kind.to_string().as_bytes();
        let token_str = token_str.as_bytes();
        if tokenkind_str.len() <= 1 {
            false
        } else {
            Self::one_character_different(tokenkind_str, token_str)
        }
    }

    fn is_misspelled_from<'b>(kind_list: &[TokenKind], token_str: &'b str) -> bool {
        kind_list
            .iter()
            .any(|x| Self::is_misspelled_kind(*x, token_str))
    }

    // If token_str is a misspelling (by our narrow definition of misspelling)
    // of a TokenKind from kind_list, return the TokenKind that token_str is a
    // misspelling of. Otherwise, return None.
    fn suggested_kind_from(kind_list: &[TokenKind], token_str: &str) -> Option<TokenKind> {
        kind_list.iter().find_map(|x| {
            if Self::is_misspelled_kind(*x, token_str) {
                Some(*x)
            } else {
                None
            }
        })
    }

    fn skip_and_log_misspelled_token(&mut self, required_kind: TokenKind) {
        let received_str = &self.current_token_text();
        let required_str = required_kind.to_string();
        self.with_error_on_whole_token(Errors::error1058(received_str, required_str));
        self.skip_and_log_unexpected_token(/* generate_error:*/ false)
    }

    fn require_token_one_of(&mut self, kinds: &[TokenKind], error: Error) -> S::R {
        let token_kind = self.peek_token_kind();
        if kinds.iter().any(|x| *x == token_kind) {
            let token = self.next_token();
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Look at the next token after this. Is it the one we
            // require? If so, process the current token as extra and return the next
            // one. Otherwise, create a missing token for what we required,
            // and continue on from the current token (don't skip it).
            let next_kind = self.peek_token_kind_with_lookahead(1);
            if kinds.iter().any(|x| *x == next_kind) {
                self.skip_and_log_unexpected_token(true);
                let token = self.next_token();
                S!(make_token, self, token)
            } else {
                // ERROR RECOVERY: We know we didn't encounter an extra token.
                // So, as a second line of defense, check if the current token
                // is a misspelling, by our existing narrow definition of misspelling.
                let is_misspelling =
                    |k: &&TokenKind| Self::is_misspelled_kind(**k, &self.current_token_text());
                let kind = kinds.iter().find(is_misspelling);
                match kind {
                    Some(kind) => {
                        self.skip_and_log_misspelled_token(*kind);
                        S!(make_missing, self, self.pos())
                    }
                    None => {
                        self.with_error(error);
                        S!(make_missing, self, self.pos())
                    }
                }
            }
        }
    }

    fn require_token(&mut self, kind: TokenKind, error: Error) -> S::R {
        // Must behave as `require_token_one_of parser [kind] error`
        if self.peek_token_kind() == kind {
            let token = self.next_token();
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Look at the next token after this. Is it the one we
            // require? If so, process the current token as extra and return the next
            // one. Otherwise, create a missing token for what we required,
            // and continue on from the current token (don't skip it).
            let next_kind = self.peek_token_kind_with_lookahead(1);
            if next_kind == kind {
                self.skip_and_log_unexpected_token(true);
                let token = self.next_token();
                S!(make_token, self, token)
            } else {
                // ERROR RECOVERY: We know we didn't encounter an extra token.
                // So, as a second line of defense, check if the current token
                // is a misspelling, by our existing narrow definition of misspelling.
                if Self::is_misspelled_kind(kind, &self.current_token_text()) {
                    self.skip_and_log_misspelled_token(kind);
                    S!(make_missing, self, self.pos())
                } else {
                    self.with_error(error);
                    S!(make_missing, self, self.pos())
                }
            }
        }
    }

    fn require_and_return_token(&mut self, kind: TokenKind, error: Error) -> Option<S::Token> {
        if self.peek_token_kind() == kind {
            Some(self.next_token())
        } else {
            // ERROR RECOVERY: Look at the next token after this. Is it the one we
            // require? If so, process the current token as extra and return the next
            // one. Otherwise, create a missing token for what we required,
            // and continue on from the current token (don't skip it).
            let next_kind = self.peek_token_kind_with_lookahead(1);
            if next_kind == kind {
                self.skip_and_log_unexpected_token(true);
                Some(self.next_token())
            } else {
                // ERROR RECOVERY: We know we didn't encounter an extra token.
                // So, as a second line of defense, check if the current token
                // is a misspelling, by our existing narrow definition of misspelling.
                if Self::is_misspelled_kind(kind, &self.current_token_text()) {
                    self.skip_and_log_misspelled_token(kind);
                    None
                } else {
                    self.with_error(error);
                    None
                }
            }
        }
    }

    fn require_name_allow_all_keywords(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let token = parser1.next_token_as_name();

        if token.kind() == TokenKind::Name {
            self.continue_from(parser1);
            S!(make_token, self, token)
        } else {
            // ERROR RECOVERY: Create a missing token for the expected token,
            // and continue on from the current token. Don't skip it.
            self.with_error(Errors::error1004);
            S!(make_missing, self, self.pos())
        }
    }

    fn require_right_paren(&mut self) -> S::R {
        self.require_token(TokenKind::RightParen, Errors::error1011)
    }

    fn require_semicolon_token(&mut self, saw_type_name: bool) -> Option<S::Token> {
        match self.peek_token_kind() {
            TokenKind::Variable if saw_type_name => self
                .require_and_return_token(TokenKind::Semicolon, Errors::local_variable_with_type),
            _ => self.require_and_return_token(TokenKind::Semicolon, Errors::error1010),
        }
    }

    fn require_semicolon(&mut self) -> S::R {
        self.require_token(TokenKind::Semicolon, Errors::error1010)
    }

    fn check_stack_limit(&self) {
        self.context()
            .stack_limit
            .as_ref()
            .map(|limit| limit.panic_if_exceeded());
    }
}
