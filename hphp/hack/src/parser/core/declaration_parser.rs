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

use crate::expression_parser::ExpressionParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::Context;
use crate::parser_trait::ExpectedTokens;
use crate::parser_trait::ParserTrait;
use crate::parser_trait::SeparatedListKind;
use crate::smart_constructors::NodeType;
use crate::smart_constructors::SmartConstructors;
use crate::smart_constructors::Token;
use crate::statement_parser::StatementParser;
use crate::type_parser::TypeParser;

#[derive(Clone)]
pub struct DeclarationParser<'a, S>
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

impl<'a, S> ParserTrait<'a, S> for DeclarationParser<'a, S>
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

impl<'a, S> DeclarationParser<'a, S>
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

    fn parse_type_specifier(&mut self, allow_var: bool, allow_attr: bool) -> S::Output {
        self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_type_specifier(allow_var, allow_attr)
        })
    }

    fn with_statement_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut StatementParser<'a, S>) -> U,
    {
        let mut statement_parser: StatementParser<'_, S> = StatementParser::make(
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

    fn parse_simple_type_or_type_constant(&mut self) -> S::Output {
        self.with_type_parser(|x: &mut TypeParser<'a, S>| x.parse_simple_type_or_type_constant())
    }

    fn parse_simple_type_or_generic(&mut self) -> S::Output {
        self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_simple_type_or_generic())
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

    fn parse_expression(&mut self) -> S::Output {
        self.with_expression_parser(|p: &mut ExpressionParser<'a, S>| p.parse_expression())
    }

    fn parse_compound_statement(&mut self) -> S::Output {
        self.with_statement_parser(|p: &mut StatementParser<'a, S>| p.parse_compound_statement())
    }

    // SPEC:
    // enum-use:
    //   use  enum-name-list  ;
    //
    // enum-name-list:
    //   name
    //   enum-name-list  ,  name
    fn parse_enum_use(&mut self) -> Option<S::Output> {
        match self.peek_token_kind_with_lookahead(1) {
            TokenKind::Equal => None,
            _ => match self.peek_token_kind() {
                TokenKind::Use => {
                    let use_token = self.assert_token(TokenKind::Use);
                    let enum_name_list = self.parse_special_type_list();
                    let semi = self.require_semicolon();
                    Some(self.sc_mut().make_enum_use(use_token, enum_name_list, semi))
                }
                _ => None,
            },
        }
    }

    fn parse_enum_use_list_opt(&mut self) -> S::Output {
        self.parse_list_until_none(|x: &mut Self| x.parse_enum_use())
    }

    fn parse_enumerator_list_opt(&mut self) -> S::Output {
        // SPEC
        // enumerator-list:
        //   enumerator
        //   enumerator-list   enumerator
        //
        self.parse_terminated_list(|x: &mut Self| x.parse_enumerator(), TokenKind::RightBrace)
    }

    fn parse_enum_class_element(&mut self) -> S::Output {
        // We need to identify an element of an enum class. Possibilities
        // are:
        //
        // // type-constant-declaration:
        // const type name = type-specifier ;
        // abstract const type name ;
        //
        // // enum-class-enumerator:
        // type-specifier name = expression ;
        // abstract type-specifier name ;
        let attr = self.parse_attribute_specification_opt();

        let kind0 = self.peek_token_kind_with_lookahead(0);
        let kind1 = self.peek_token_kind_with_lookahead(1);
        let kind2 = self.peek_token_kind_with_lookahead(2);

        match (kind0, kind1, kind2) {
            (TokenKind::Abstract, TokenKind::Const, TokenKind::Type)
            | (TokenKind::Const, TokenKind::Type, _) => {
                let modifiers = self.parse_modifiers();
                let const_ = self.assert_token(TokenKind::Const);
                self.parse_type_const_declaration(attr, modifiers, const_)
            }
            _ => {
                if !attr.is_missing() {
                    self.with_error(Errors::no_attributes_on_enum_class_enumerator)
                }
                self.parse_enum_class_enumerator()
            }
        }
    }

    fn parse_enum_class_element_list_opt(&mut self) -> S::Output {
        self.parse_terminated_list(
            |x: &mut Self| x.parse_enum_class_element(),
            TokenKind::RightBrace,
        )
    }

    fn parse_enum_declaration(&mut self, attrs: S::Output, modifiers: S::Output) -> S::Output {
        // enum-declaration:
        //   attribute-specification-opt modifiers enum  name  enum-base  type-constraint-opt /
        //     {  enum-use-clause-list-opt; enumerator-list-opt  }
        // enum-base:
        //   :  int
        //   :  string
        //
        // TODO: SPEC ERROR: The spec states that the only legal enum types
        // are "int" and "string", but Hack allows any type, and apparently
        // some of those are meaningful and desired.  Figure out what types
        // are actually legal and illegal as enum base types; put them in the
        // spec, and add an error pass that says when they are wrong.
        let enum_ = self.assert_token(TokenKind::Enum);
        let name = self.require_name();
        let colon = self.require_colon();
        let base =
            self.parse_type_specifier(false /* allow_var */, true /* allow_attr */);
        let enum_type = self.parse_type_constraint_opt(false);
        let left_brace = self.require_left_brace();
        let use_clauses = self.parse_enum_use_list_opt();
        let enumerators = self.parse_enumerator_list_opt();
        let right_brace = self.require_right_brace();
        self.sc_mut().make_enum_declaration(
            attrs,
            modifiers,
            enum_,
            name,
            colon,
            base,
            enum_type,
            left_brace,
            use_clauses,
            enumerators,
            right_brace,
        )
    }

    fn parse_enum_class_declaration(
        &mut self,
        attrs: S::Output,
        modifiers: S::Output,
    ) -> S::Output {
        // enum-class-declaration:
        //   attribute-specification-opt modifiers-opt enum class name : base { enum-class-enumerator-list-opt }
        let enum_kw = self.assert_token(TokenKind::Enum);
        let class_kw = self.assert_token(TokenKind::Class);
        let name = self.require_name();
        let colon = self.require_colon();
        let base =
            self.parse_type_specifier(false /* allow_var */, false /* allow_attr */);
        let (classish_extends, classish_extends_list) = self.parse_extends_opt();
        let left_brace = self.require_left_brace();
        let elements = self.parse_enum_class_element_list_opt();
        let right_brace = self.require_right_brace();
        self.sc_mut().make_enum_class_declaration(
            attrs,
            modifiers,
            enum_kw,
            class_kw,
            name,
            colon,
            base,
            classish_extends,
            classish_extends_list,
            left_brace,
            elements,
            right_brace,
        )
    }

    fn parse_enum_or_enum_class_declaration(
        &mut self,
        attrs: S::Output,
        modifiers: S::Output,
    ) -> S::Output {
        match self.peek_token_kind_with_lookahead(1) {
            TokenKind::Class => self.parse_enum_class_declaration(attrs, modifiers),
            _ => self.parse_enum_declaration(attrs, modifiers),
        }
    }

    pub fn parse_leading_markup_section(&mut self) -> Option<S::Output> {
        let mut parser1 = self.clone();
        let (markup_section, has_suffix) =
            parser1.with_statement_parser(|p: &mut StatementParser<'a, S>| p.parse_header());
        // proceed successfully if we've consumed <?..., or dont need it
        // We purposefully ignore leading trivia before the <?hh, and handle
        // the error on a later pass
        let file_path = self.lexer().source().file_path();
        if has_suffix {
            self.continue_from(parser1);
            Some(markup_section)
        } else if self.lexer().source().length() > 0 {
            if file_path.has_extension("php") {
                self.with_error(Errors::error1001);
                None
            } else if !markup_section.is_missing() {
                // Anything without a `.php` or `.hackpartial` extension
                // should be treated like a `.hack` file (strict), which
                // can include a shebang (hashbang).
                //
                // parse the shebang correctly and continue
                //
                // Executables do not require an extension.
                self.continue_from(parser1);
                Some(markup_section)
            } else {
                None
            }
        } else {
            None
        }
    }

    fn parse_namespace_body(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::Semicolon => {
                let token = self.fetch_token();
                self.sc_mut().make_namespace_empty_body(token)
            }
            TokenKind::LeftBrace => {
                let left = self.fetch_token();
                let body = self.parse_terminated_list(
                    |x: &mut Self| x.parse_declaration(),
                    TokenKind::RightBrace,
                );
                let right = self.require_right_brace();
                self.sc_mut().make_namespace_body(left, body, right)
            }
            _ => {
                // ERROR RECOVERY: return an inert namespace (one with all of its
                // components 'missing'), and recover--without advancing the parser--
                // back to the level that the namespace was declared in.
                self.with_error(Errors::error1038);
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing3 = self.sc_mut().make_missing(pos);
                self.sc_mut()
                    .make_namespace_body(missing1, missing2, missing3)
            }
        }
    }

    fn is_group_use(&self) -> bool {
        let mut parser = self.clone();
        // We want a heuristic to determine whether to parse the use clause as
        // a group use or normal use clause.  We distinguish the two by (1) whether
        // there is a namespace prefix -- in this case it is definitely a group use
        // clause -- or, if there is a name followed by a curly. That's illegal, but
        // we should give an informative error message about that.
        parser.assert_token(TokenKind::Use);
        parser.parse_namespace_use_kind_opt();
        let token = parser.next_token();
        match token.kind() {
            TokenKind::Backslash => {
                let pos = parser.pos();
                let missing = parser.sc_mut().make_missing(pos);
                let backslash = parser.sc_mut().make_token(token);
                let (_name, is_backslash) = parser.scan_qualified_name_extended(missing, backslash);
                is_backslash || parser.peek_token_kind() == TokenKind::LeftBrace
            }
            TokenKind::Name => {
                let token = parser.sc_mut().make_token(token);
                let token_ref = &token as *const _;
                let (name, is_backslash) = parser.scan_remaining_qualified_name_extended(token);
                // Here we rely on the implementation details of
                // scan_remaining_qualified_name_extended. It's returning
                // *exactly* token if there is nothing except it in the name.
                is_backslash && (&name as *const _ == token_ref)
                    || parser.peek_token_kind() == TokenKind::LeftBrace
            }
            _ => false,
        }
    }

    fn parse_namespace_use_kind_opt(&mut self) -> S::Output {
        // SPEC
        // namespace-use-kind:
        //   namespace
        //   function
        //   const
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::Type | TokenKind::Namespace | TokenKind::Function | TokenKind::Const => {
                self.continue_from(parser1);
                self.sc_mut().make_token(token)
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    fn parse_group_use(&mut self) -> S::Output {
        // See below for grammar.
        let use_token = self.assert_token(TokenKind::Use);
        let use_kind = self.parse_namespace_use_kind_opt();
        // We already know that this is a name, qualified name, or prefix.
        // If this is not a prefix, it will be detected as an error in a later pass
        let prefix = self.scan_name_or_qualified_name();
        let (left, clauses, right) =
            self.parse_braced_comma_list_opt_allow_trailing(|x: &mut Self| {
                x.parse_namespace_use_clause()
            });
        let semi = self.require_semicolon();
        self.sc_mut().make_namespace_group_use_declaration(
            use_token, use_kind, prefix, left, clauses, right, semi,
        )
    }

    fn parse_namespace_use_clause(&mut self) -> S::Output {
        // SPEC
        // namespace-use-clause:
        //   qualified-name  namespace-aliasing-clauseopt
        // namespace-use-kind-clause:
        //   namespace-use-kind-opt qualified-name  namespace-aliasing-clauseopt
        // namespace-aliasing-clause:
        //   as  name
        //
        let use_kind = self.parse_namespace_use_kind_opt();
        let name = self.require_qualified_name();
        let (as_token, alias) = if self.peek_token_kind() == TokenKind::As {
            let as_token = self.next_token();
            let as_token = self.sc_mut().make_token(as_token);
            let alias = self.require_xhp_class_name_or_name();
            (as_token, alias)
        } else {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            (missing1, missing2)
        };
        self.sc_mut()
            .make_namespace_use_clause(use_kind, name, as_token, alias)
    }

    fn parse_namespace_use_declaration(&mut self) -> S::Output {
        // SPEC
        // namespace-use-declaration:
        //   use namespace-use-kind-opt namespace-use-clauses  ;
        //   use namespace-use-kind namespace-name-as-a-prefix
        //     { namespace-use-clauses }  ;
        // use namespace-name-as-a-prefix { namespace-use-kind-clauses  }  ;
        //
        // TODO: Add the grammar for the namespace-use-clauses; ensure that it
        // indicates that trailing commas are allowed in the list.
        //
        // ERROR RECOVERY
        // In the "simple" format, the kind may only be specified up front.
        //
        // The grammar in the specification says that in the "group"
        // format, if the kind is specified up front then it may not
        // be specified in each clause. However, HHVM's parser disallows
        // the kind in each clause regardless of whether it is specified up front.
        // We will fix the specification to match HHVM.
        //
        // The grammar in the specification also says that in the "simple" format,
        // the kind may only be specified up front.  But HHVM allows the kind to
        // be specified in each clause.  Again, we will fix the specification to match
        // HHVM.
        //
        // TODO: Update the grammar comment above when the specification is fixed.
        // (This work is being tracked by spec work items 102, 103 and 104.)
        //
        // We do not enforce these rules here. Rather, we allow the kind to be anywhere,
        // and detect the errors in a later pass.
        if self.is_group_use() {
            self.parse_group_use()
        } else {
            let use_token = self.assert_token(TokenKind::Use);
            let use_kind = self.parse_namespace_use_kind_opt();
            let (clauses, _) = self.parse_comma_list_allow_trailing(
                TokenKind::Semicolon,
                Errors::error1004,
                |x: &mut Self| x.parse_namespace_use_clause(),
            );
            let semi = self.require_semicolon();
            self.sc_mut()
                .make_namespace_use_declaration(use_token, use_kind, clauses, semi)
        }
    }

    fn parse_namespace_declaration(&mut self) -> S::Output {
        // SPEC
        // namespace-definition:
        //   namespace  namespace-name  ;
        //   namespace  namespace-name-opt  { declaration-list }
        //
        // TODO: An error case not caught by the parser that should be caught
        // in a later pass:
        // Qualified names are a superset of legal namespace names.
        let namespace_token = self.assert_token(TokenKind::Namespace);
        let name = match self.peek_token_kind() {
            TokenKind::Name => {
                let token = self.next_token();
                let token = self.sc_mut().make_token(token);
                self.scan_remaining_qualified_name(token)
            }
            TokenKind::LeftBrace => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            TokenKind::Semicolon => {
                // ERROR RECOVERY Plainly the name is missing.
                self.with_error(Errors::error1004);
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ =>
            // TODO: Death to PHPisms; keywords as namespace names
            {
                self.require_name_allow_non_reserved()
            }
        };
        let header = self
            .sc_mut()
            .make_namespace_declaration_header(namespace_token, name);
        let body = self.parse_namespace_body();
        self.sc_mut().make_namespace_declaration(header, body)
    }

    pub fn parse_classish_declaration(&mut self, attribute_spec: S::Output) -> S::Output {
        let modifiers = self.parse_classish_modifiers();

        if self.peek_token_kind() == TokenKind::Enum {
            return self.parse_enum_or_enum_class_declaration(attribute_spec, modifiers);
        }

        let (xhp, is_xhp_class) = self.parse_xhp_keyword();

        // Error on the XHP token unless it's been explicitly enabled
        if is_xhp_class && !self.env.enable_xhp_class_modifier {
            self.with_error(Errors::error1057("XHP"));
        }

        let token = self.parse_classish_token();
        let name = if is_xhp_class {
            self.require_xhp_class_name()
        } else {
            self.require_maybe_xhp_class_name()
        };
        let generic_type_parameter_list = self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_generic_type_parameter_list_opt()
        });
        let (classish_extends, classish_extends_list) = self.parse_extends_opt();
        let (classish_implements, classish_implements_list) = self.parse_classish_implements_opt();
        let classish_where_clause = self.parse_classish_where_clause_opt();
        let body = self.parse_classish_body();
        self.sc_mut().make_classish_declaration(
            attribute_spec,
            modifiers,
            xhp,
            token,
            name,
            generic_type_parameter_list,
            classish_extends,
            classish_extends_list,
            classish_implements,
            classish_implements_list,
            classish_where_clause,
            body,
        )
    }

    fn parse_classish_where_clause_opt(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::Where {
            self.parse_where_clause()
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_classish_implements_opt(&mut self) -> (S::Output, S::Output) {
        if self.peek_token_kind() != TokenKind::Implements {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            (missing1, missing2)
        } else {
            let implements_token = self.next_token();
            let implements_token = self.sc_mut().make_token(implements_token);
            let implements_list = self.parse_special_type_list();
            (implements_token, implements_list)
        }
    }

    fn parse_xhp_keyword(&mut self) -> (S::Output, bool) {
        let xhp = self.optional_token(TokenKind::XHP);
        let is_missing = xhp.is_missing();
        (xhp, !is_missing)
    }

    fn parse_classish_modifiers(&mut self) -> S::Output {
        let mut acc = vec![];
        loop {
            match self.peek_token_kind() {
                TokenKind::Abstract
                | TokenKind::Final
                | TokenKind::Internal
                | TokenKind::Public => {
                    // TODO(T25649779)
                    let token = self.next_token();
                    let token = self.sc_mut().make_token(token);
                    acc.push(token);
                }
                _ => {
                    let pos = self.pos();
                    return self.sc_mut().make_list(acc, pos);
                }
            }
        }
    }

    fn parse_classish_token(&mut self) -> S::Output {
        let spellcheck_tokens = vec![TokenKind::Class, TokenKind::Trait, TokenKind::Interface];
        let token_str = &self.current_token_text();
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Class | TokenKind::Trait | TokenKind::Interface => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            // Spellcheck case
            TokenKind::Name if Self::is_misspelled_from(&spellcheck_tokens, token_str) => {
                // Default won't be used, since we already checked is_misspelled_from
                let suggested_kind = Self::suggested_kind_from(&spellcheck_tokens, token_str)
                    .unwrap_or(TokenKind::Name);
                self.skip_and_log_misspelled_token(suggested_kind);
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => {
                self.with_error(Errors::error1035);
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    fn parse_special_type(&mut self) -> (S::Output, bool) {
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_class_name_or_other_token();
        match token.kind() {
            TokenKind::Comma => {
                // ERROR RECOVERY. We expected a type but we got a comma.
                // Give the error that we expected a type, not a name, even though
                // not every type is legal here.
                self.continue_from(parser1);
                self.with_error(Errors::error1007);
                let comma = self.sc_mut().make_token(token);
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                let list_item = self.sc_mut().make_list_item(missing, comma);
                (list_item, false)
            }
            TokenKind::Backslash
            | TokenKind::Namespace
            | TokenKind::Name
            | TokenKind::XHPClassName => {
                let item = self
                    .parse_type_specifier(false /* allow_var */, true /* allow_attr */);
                let comma = self.optional_token(TokenKind::Comma);
                let is_missing = comma.is_missing();
                let list_item = self.sc_mut().make_list_item(item, comma);
                (list_item, is_missing)
            }
            TokenKind::Enum | TokenKind::Shape if self.env.hhvm_compat_mode => {
                // HHVM allows these keywords here for some reason
                let item = self.parse_simple_type_or_type_constant();
                let comma = self.optional_token(TokenKind::Comma);
                let is_missing = comma.is_missing();
                let list_item = self.sc_mut().make_list_item(item, comma);
                (list_item, is_missing)
            }
            _ => {
                // ERROR RECOVERY: We are expecting a type; give an error as above.
                // Don't eat the offending token.
                self.with_error(Errors::error1007);
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                let list_item = self.sc_mut().make_list_item(missing1, missing2);
                (list_item, true)
            }
        }
    }

    fn parse_special_type_list(&mut self) -> S::Output {
        // An extends / implements / enum_use list is a comma-separated list of types,
        // but very special types; we want the types to consist of a name and an
        // optional generic type argument list.
        //
        // TODO: Can the type name be of the form "foo::bar"? Those do not
        // necessarily start with names. Investigate this.
        //
        // Normally we'd use one of the separated list helpers, but there is no
        // specific end token we could use to detect the end of the list, and we
        // want to bail out if we get something that is not a type of the right form.
        // So we have custom logic here.
        //
        // TODO: This is one of the rare cases in Hack where a comma-separated list
        // may not have a trailing comma. Is that desirable, or was that an
        // oversight when the trailing comma rules were added?  If possible we
        // should keep the rule as-is, and disallow the trailing comma; it makes
        // parsing and error recovery easier.
        let mut items = vec![];
        loop {
            let (item, is_missing) = self.parse_special_type();

            items.push(item);
            if is_missing {
                break;
            }
        }
        let pos = self.pos();
        self.sc_mut().make_list(items, pos)
    }

    fn parse_extends_opt(&mut self) -> (S::Output, S::Output) {
        let token_kind = self.peek_token_kind();
        if token_kind != TokenKind::Extends {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            (missing1, missing2)
        } else {
            let token = self.next_token();
            let extends_token = self.sc_mut().make_token(token);
            let extends_list = self.parse_special_type_list();
            (extends_token, extends_list)
        }
    }

    fn parse_classish_body(&mut self) -> S::Output {
        let left_brace_token = self.require_left_brace();
        let classish_element_list = self.parse_classish_element_list_opt();
        let right_brace_token = self.require_right_brace();
        self.sc_mut()
            .make_classish_body(left_brace_token, classish_element_list, right_brace_token)
    }

    fn parse_classish_element_list_opt(&mut self) -> S::Output {
        // TODO: ERROR RECOVERY: consider bailing if the token cannot possibly
        // start a classish element.
        // ERROR RECOVERY: we're in the body of a classish, so we add visibility
        // modifiers to our context.
        self.expect_in_new_scope(ExpectedTokens::Visibility);
        let element_list = self.parse_terminated_list(
            |x: &mut Self| x.parse_classish_element(),
            TokenKind::RightBrace,
        );
        self.pop_scope(ExpectedTokens::Visibility);
        element_list
    }

    fn parse_xhp_children_paren(&mut self) -> S::Output {
        // SPEC (Draft)
        // ( xhp-children-expressions )
        //
        // xhp-children-expressions:
        //   xhp-children-expression
        //   xhp-children-expressions , xhp-children-expression
        //
        // TODO: The parenthesized list of children expressions is NOT allowed
        // to be comma-terminated. Is this intentional? It is inconsistent with
        // practice throughout the rest of Hack. There is no syntactic difficulty
        // in allowing a comma before the close paren. Consider allowing it.
        let (left, exprs, right) =
            self.parse_parenthesized_comma_list(|x: &mut Self| x.parse_xhp_children_expression());
        self.sc_mut()
            .make_xhp_children_parenthesized_list(left, exprs, right)
    }

    fn parse_xhp_children_term(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-children-term:
        // ( xhp-children-expressions ) trailing-opt
        // name trailing-opt
        // xhp-class-name trailing-opt
        // xhp-category-name trailing-opt
        // trailing: * ? +
        //
        // name should be 'pcdata', 'any', or 'empty', however any name is
        // currently permitted
        //
        // Note that there may be only zero or one trailing unary operator.
        // "foo*?" is not a legal xhp child expression.
        //
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_children_name_or_other();
        let kind = token.kind();
        let name = parser1.sc_mut().make_token(token);
        match kind {
            TokenKind::Name => {
                self.continue_from(parser1);
                self.parse_xhp_children_trailing(name)
            }
            TokenKind::XHPClassName | TokenKind::XHPCategoryName => {
                self.continue_from(parser1);
                self.parse_xhp_children_trailing(name)
            }
            TokenKind::LeftParen => {
                let term = self.parse_xhp_children_paren();
                self.parse_xhp_children_trailing(term)
            }
            _ => {
                // ERROR RECOVERY: Eat the offending token, keep going.
                self.with_error(Errors::error1053);
                name
            }
        }
    }

    fn parse_xhp_children_trailing(&mut self, term: S::Output) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Star | TokenKind::Plus | TokenKind::Question => {
                let token = self.next_token();
                let token = self.sc_mut().make_token(token);
                self.sc_mut().make_postfix_unary_expression(term, token)
            }
            _ => term,
        }
    }

    fn parse_xhp_children_bar(&mut self, left: S::Output) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Bar => {
                let token = self.next_token();
                let token = self.sc_mut().make_token(token);
                let right = self.parse_xhp_children_term();
                let result = self.sc_mut().make_binary_expression(left, token, right);
                self.parse_xhp_children_bar(result)
            }
            _ => left,
        }
    }

    fn parse_xhp_children_expression(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-children-expression:
        //   xhp-children-term
        //   xhp-children-expression | xhp-children-term
        //
        // Note that the bar operator is left-associative. (Not that it matters
        // semantically.
        let term = self.parse_xhp_children_term();
        self.parse_xhp_children_bar(term)
    }

    fn parse_xhp_children_declaration(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-children-declaration:
        //   children empty ;
        //   children xhp-children-expression ;
        let children = self.assert_token(TokenKind::Children);
        if self.env.disable_xhp_children_declarations {
            self.with_error(Errors::error1064);
        }
        let token_kind = self.peek_token_kind();
        let expr = match token_kind {
            TokenKind::Empty => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            _ => self.parse_xhp_children_expression(),
        };
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_xhp_children_declaration(children, expr, semi)
    }

    fn parse_xhp_category(&mut self) -> S::Output {
        let token = self.next_xhp_category_name();
        let token_kind = token.kind();
        let category = self.sc_mut().make_token(token);
        match token_kind {
            TokenKind::XHPCategoryName => category,
            _ => {
                self.with_error(Errors::error1052);
                category
            }
        }
    }

    fn parse_xhp_attribute_enum(&mut self) -> S::Output {
        let like_token = if self.peek_token_kind() == TokenKind::Tilde {
            self.assert_token(TokenKind::Tilde)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        };
        let enum_token = self.assert_token(TokenKind::Enum);
        let (left_brace, values, right_brace) =
            self.parse_braced_comma_list_opt_allow_trailing(|x: &mut Self| x.parse_expression());
        self.sc_mut()
            .make_xhp_enum_type(like_token, enum_token, left_brace, values, right_brace)
    }

    fn parse_xhp_type_specifier(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-type-specifier:
        //   (enum | ~enum) { xhp-attribute-enum-list  ,-opt  }
        //   type-specifier
        //
        // The list of enum values must have at least one value and can be
        // comma-terminated.
        //
        // xhp-enum-list:
        //   xhp-attribute-enum-value
        //   xhp-enum-list , xhp-attribute-enum-value
        //
        // xhp-attribute-enum-value:
        //   any integer literal
        //   any single-quoted-string literal
        //   any double-quoted-string literal
        //
        // TODO: What are the semantics of encapsulated expressions in double-quoted
        // string literals here?
        //
        // An empty list is illegal, but we allow it here and give an error in
        // a later pass.
        match self.peek_token_kind() {
            TokenKind::Tilde if self.peek_token_kind_with_lookahead(1) == TokenKind::Enum => {
                self.parse_xhp_attribute_enum()
            }
            TokenKind::Enum => self.parse_xhp_attribute_enum(),
            _ => self.parse_type_specifier(true, true),
        }
    }

    fn parse_xhp_required_opt(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-required :
        //   @  (required | lateinit)
        //
        // Note that these are two tokens. They can have whitespace between them.
        if self.peek_token_kind() == TokenKind::At {
            let at = self.assert_token(TokenKind::At);
            let req_kind = self.next_token();
            let kind = req_kind.kind();
            let req = self.sc_mut().make_token(req_kind);
            match kind {
                TokenKind::Required => self.sc_mut().make_xhp_required(at, req),
                TokenKind::Lateinit => self.sc_mut().make_xhp_lateinit(at, req),
                _ => {
                    self.with_error(Errors::error1051);
                    let pos = self.pos();
                    self.sc_mut().make_missing(pos)
                }
            }
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_xhp_class_attribute_typed(&mut self) -> S::Output {
        // xhp-type-specifier xhp-name initializer-opt xhp-required-opt
        let ty = self.parse_xhp_type_specifier();
        let name = self.require_xhp_name();
        let init = self.parse_simple_initializer_opt();
        let req = self.parse_xhp_required_opt();
        self.sc_mut().make_xhp_class_attribute(ty, name, init, req)
    }

    fn parse_xhp_category_declaration(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-category-declaration:
        //   category xhp-category-list ,-opt  ;
        //
        // xhp-category-list:
        //   xhp-category-name
        //   xhp-category-list  ,  xhp-category-name
        let category = self.assert_token(TokenKind::Category);
        let (items, _) = self.parse_comma_list_allow_trailing(
            TokenKind::Semicolon,
            Errors::error1052,
            |x: &mut Self| x.parse_xhp_category(),
        );
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_xhp_category_declaration(category, items, semi)
    }

    fn parse_xhp_class_attribute(&mut self) -> S::Output {
        // SPEC (Draft)
        // xhp-attribute-declaration:
        //   xhp-class-name
        //   xhp-type-specifier xhp-name initializer-opt xhp-required-opt
        //
        // ERROR RECOVERY:
        // The xhp type specifier could be an xhp class name. To disambiguate we peek
        // ahead a token; if it's a comma or semi, we're done. If not, then we assume
        // that we are in the more complex case.
        if self.is_next_xhp_class_name() {
            let mut parser1 = self.clone();
            let class_name = parser1.require_maybe_xhp_class_name();
            match parser1.peek_token_kind() {
                TokenKind::Comma | TokenKind::Semicolon => {
                    self.continue_from(parser1);
                    let type_specifier = self.sc_mut().make_simple_type_specifier(class_name);
                    self.sc_mut()
                        .make_xhp_simple_class_attribute(type_specifier)
                }
                _ => self.parse_xhp_class_attribute_typed(),
            }
        } else {
            self.parse_xhp_class_attribute_typed()
        }
    }

    fn parse_xhp_class_attribute_declaration(&mut self) -> S::Output {
        // SPEC: (Draft)
        // xhp-class-attribute-declaration :
        //   attribute xhp-attribute-declaration-list ;
        //
        // xhp-attribute-declaration-list:
        //   xhp-attribute-declaration
        //   xhp-attribute-declaration-list , xhp-attribute-declaration
        //
        // TODO: The list of attributes may NOT be terminated with a trailing comma
        // before the semicolon. This is inconsistent with the rest of Hack.
        // Allowing a comma before the semi does not introduce any syntactic
        // difficulty; consider allowing it.
        let attr_token = self.assert_token(TokenKind::Attribute);
        // TODO: Better error message.
        let attrs =
            self.parse_comma_list(TokenKind::Semicolon, Errors::error1004, |x: &mut Self| {
                x.parse_xhp_class_attribute()
            });
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_xhp_class_attribute_declaration(attr_token, attrs, semi)
    }

    fn parse_qualified_name_type(&mut self) -> S::Output {
        // Here we're parsing a name followed by an optional generic type
        // argument list; if we don't have a name, give an error.
        match self.peek_token_kind() {
            TokenKind::Backslash | TokenKind::Name => self.parse_simple_type_or_generic(),
            _ => self.require_qualified_name(),
        }
    }

    fn parse_require_clause(&mut self) -> S::Output {
        // SPEC
        // require-extends-clause:
        //   require  extends  qualified-name  ;
        //
        // require-implements-clause:
        //   require  implements  qualified-name  ;
        //
        // require-class-clause:
        //   require  class  qualified-name  ;
        //
        // We must also parse "require extends :foo;"
        // TODO: What about "require extends :foo<int>;" ?
        // TODO: The spec is incomplete; we need to be able to parse
        // require extends Foo<int>;
        // (This work is being tracked by spec issue 105.)
        // TODO: Check whether we also need to handle
        // require extends foo::bar
        // and so on.
        //
        // ERROR RECOVERY: Detect if the implements/extends/class, name and semi
        // are missing.
        let req = self.assert_token(TokenKind::Require);
        let token_kind = self.peek_token_kind();
        let req_kind = match token_kind {
            TokenKind::Implements | TokenKind::Extends | TokenKind::Class => {
                let req_kind_token = self.next_token();
                self.sc_mut().make_token(req_kind_token)
            }
            _ => {
                self.with_error(Errors::error1045);
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        };
        let name = if self.is_next_xhp_class_name() {
            self.parse_simple_type_or_generic()
        } else {
            self.parse_qualified_name_type()
        };
        let semi = self.require_semicolon();
        self.sc_mut().make_require_clause(req, req_kind, name, semi)
    }

    fn parse_methodish_or_property(&mut self, attribute_spec: S::Output) -> S::Output {
        let modifiers = self.parse_modifiers();
        // ERROR RECOVERY: match against two tokens, because if one token is
        // in error but the next isn't, then it's likely that the user is
        // simply still typing. Throw an error on what's being typed, then eat
        // it and keep going.
        let current_token_kind = self.peek_token_kind();
        let next_token = self.peek_token_with_lookahead(1);
        let next_token_kind = next_token.kind();
        match (current_token_kind, next_token_kind) {
            // Detected the usual start to a method, so continue parsing as method.
            (TokenKind::Async, _) | (TokenKind::Function, _) => {
                self.parse_methodish(attribute_spec, modifiers)
            }
            (TokenKind::LeftParen, _) => self.parse_property_declaration(attribute_spec, modifiers),

            // We encountered one unexpected token, but the next still indicates that
            // we should be parsing a methodish. Throw an error, process the token
            // as an extra, and keep going.
            (_, TokenKind::Async) | (_, TokenKind::Function)
                if !(next_token.has_leading_trivia_kind(TriviaKind::EndOfLine)) =>
            {
                self.with_error_on_whole_token(Errors::error1056(
                    self.token_text(&self.peek_token()),
                ));
                self.skip_and_log_unexpected_token(false);
                self.parse_methodish(attribute_spec, modifiers)
            }
            // Otherwise, continue parsing as a property (which might be a lambda).
            _ => self.parse_property_declaration(attribute_spec, modifiers),
        }
    }

    // SPEC:
    // trait-use-clause:
    //   use  trait-name-list  ;
    //
    // trait-name-list:
    //   qualified-name  generic-type-parameter-listopt
    //   trait-name-list  ,  qualified-name  generic-type-parameter-listopt
    fn parse_trait_name_list<P>(&mut self, predicate: P) -> S::Output
    where
        P: Fn(TokenKind) -> bool,
    {
        let (items, _, _) = self.parse_separated_list_predicate(
            |x| x == TokenKind::Comma,
            SeparatedListKind::NoTrailing,
            predicate,
            Errors::error1004,
            |x: &mut Self| x.parse_qualified_name_type(),
        );
        items
    }

    fn parse_trait_use(&mut self) -> S::Output {
        let use_token = self.assert_token(TokenKind::Use);
        let trait_name_list =
            self.parse_trait_name_list(|x| x == TokenKind::Semicolon || x == TokenKind::LeftBrace);
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_trait_use(use_token, trait_name_list, semi)
    }

    fn parse_property_declaration(
        &mut self,
        attribute_spec: S::Output,
        modifiers: S::Output,
    ) -> S::Output {
        // SPEC:
        // property-declaration:
        //   attribute-spec-opt  property-modifier  type-specifier
        //   property-declarator-list  ;
        //
        // property-declarator-list:
        //   property-declarator
        //   property-declarator-list  ,  property-declarator
        //
        // The type specifier is optional in non-strict mode and required in
        // strict mode. We give an error in a later pass.
        let prop_type = match self.peek_token_kind() {
            TokenKind::Variable => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => self.parse_type_specifier(false /* allow_var */, false /* allow_attr */),
        };
        let decls =
            self.parse_comma_list(TokenKind::Semicolon, Errors::error1008, |x: &mut Self| {
                x.parse_property_declarator()
            });
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_property_declaration(attribute_spec, modifiers, prop_type, decls, semi)
    }

    fn parse_property_declarator(&mut self) -> S::Output {
        // SPEC:
        // property-declarator:
        //   variable-name  property-initializer-opt
        // property-initializer:
        //   =  expression
        let name = self.require_variable();
        let simple_init = self.parse_simple_initializer_opt();
        self.sc_mut().make_property_declarator(name, simple_init)
    }

    fn is_type_in_const(&self) -> bool {
        let mut parser1 = self.clone();
        let _ = parser1.parse_type_specifier(false, true);
        let _ = parser1.require_name_allow_all_keywords();
        self.errors.len() == parser1.errors.len()
    }

    // SPEC:
    // const-declaration:
    //   abstract_opt  const  type-specifier_opt  constant-declarator-list  ;
    //   visibility  const  type-specifier_opt  constant-declarator-list  ;
    // constant-declarator-list:
    //   constant-declarator
    //   constant-declarator-list  ,  constant-declarator
    // constant-declarator:
    //   name  constant-initializer_opt
    // constant-initializer:
    //   =  const-expression
    fn parse_const_declaration(
        &mut self,
        attributes: S::Output,
        modifiers: S::Output,
        const_: S::Output,
    ) -> S::Output {
        let type_spec = if self.is_type_in_const() {
            self.parse_type_specifier(/* allow_var = */ false, /* allow_attr = */ true)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        };

        let const_list =
            self.parse_comma_list(TokenKind::Semicolon, Errors::error1004, |x: &mut Self| {
                x.parse_constant_declarator()
            });
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_const_declaration(attributes, modifiers, const_, type_spec, const_list, semi)
    }

    fn parse_constant_declarator(&mut self) -> S::Output {
        // TODO: We allow const names to be keywords here; in particular we
        // require that const string TRUE = "true"; be legal.  Likely this
        // should be more strict. What are the rules for which keywords are
        // legal constant names and which are not?
        // Note that if this logic is changed, it should be changed in
        // is_type_in_const above as well.
        //
        // This permits abstract variables to have an initializer, and vice-versa.
        // This is deliberate, and those errors will be detected after the syntax
        // tree is created.
        self.sc_mut().begin_constant_declarator();
        let const_name = self.require_name_allow_all_keywords();
        let initializer_ = self.parse_simple_initializer_opt();
        self.sc_mut()
            .make_constant_declarator(const_name, initializer_)
    }

    // SPEC:
    // type-constant-declaration:
    //   abstract-type-constant-declaration
    //   concrete-type-constant-declaration
    // abstract-type-constant-declaration:
    //   abstract  const  type  name  type-constraint-list_opt  ;
    // concrete-type-constant-declaration:
    //   const  type  name  type-constraint-list_opt  =  type-specifier  ;
    // type-constraint-list:
    //   as  name
    //   super  name
    //   type-constraint-list  as  name
    //   type-constraint-list  super  name
    //
    // ERROR RECOVERY:
    //
    // An abstract type constant may only occur in an interface or an abstract
    // class. We allow that to be parsed here, and the type checker detects the
    // error.
    // CONSIDER: We could detect this error in a post-parse pass; it is entirely
    // syntactic.  Consider moving the error detection out of the type checker.
    //
    // An interface may not contain a non-abstract type constant that has a
    // type constraint.  We allow that to be parsed here, and the type checker
    // detects the error.
    // CONSIDER: We could detect this error in a post-parse pass; it is entirely
    // syntactic.  Consider moving the error detection out of the type checker.
    fn parse_type_const_declaration(
        &mut self,
        attributes: S::Output,
        modifiers: S::Output,
        const_: S::Output,
    ) -> S::Output {
        let type_token = self.assert_token(TokenKind::Type);
        let (name, type_parameters) = self.parse_nonreserved_name_maybe_parameterized();
        let type_constraints = self.parse_type_constraints(true);
        let (equal_token, type_specifier) = self.parse_equal_type();
        let semicolon = self.require_semicolon();
        self.sc_mut().make_type_const_declaration(
            attributes,
            modifiers,
            const_,
            type_token,
            name,
            type_parameters,
            type_constraints,
            equal_token,
            type_specifier,
            semicolon,
        )
    }

    /// Parses a non-reserved name optionally followed by type parameter list
    fn parse_nonreserved_name_maybe_parameterized(&mut self) -> (S::Output, S::Output) {
        let name = self.require_name_allow_non_reserved();
        let type_parameter_list = self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_generic_type_parameter_list_opt()
        });
        (name, type_parameter_list)
    }

    fn parse_equal_type(&mut self) -> (S::Output, S::Output) {
        if self.peek_token_kind() == TokenKind::Equal {
            let equal_token = self.assert_token(TokenKind::Equal);
            let type_spec = self
                .parse_type_specifier(/* allow_var = */ false, /* allow_attr = */ true);
            (equal_token, type_spec)
        } else {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            (missing1, missing2)
        }
    }

    fn parse_context_const_declaration(
        &mut self,
        modifiers: S::Output,
        const_: S::Output,
    ) -> S::Output {
        // SPEC
        // context-constant-declaration:
        //   abstract-context-constant-declaration
        //   concrete-context-constant-declaration
        // abstract-context-constant-declaration:
        //   abstract  const  ctx  name  context-constraint*  ;
        // concrete-context-constant-declaration:
        //   const  ctx  name  context-constraint*  =  context-list  ;
        let ctx_keyword = self.assert_token(TokenKind::Ctx);
        let name = self.require_name_allow_non_reserved();
        let (tparam_list, ctx_constraints) = self.with_type_parser(|p| {
            (
                p.parse_generic_type_parameter_list_opt(),
                p.parse_list_until_none(|p| p.parse_context_constraint_opt()),
            )
        });
        let (equal, ctx_list) = self.parse_equal_contexts();
        let semicolon = self.require_semicolon();
        self.sc_mut().make_context_const_declaration(
            modifiers,
            const_,
            ctx_keyword,
            name,
            tparam_list,
            ctx_constraints,
            equal,
            ctx_list,
            semicolon,
        )
    }

    fn parse_equal_contexts(&mut self) -> (S::Output, S::Output) {
        if self.peek_token_kind() == TokenKind::Equal {
            let equal = self.assert_token(TokenKind::Equal);
            let ctx_list = self.with_type_parser(|p| p.parse_contexts());
            (equal, ctx_list)
        } else {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            (missing1, missing2)
        }
    }

    pub(crate) fn parse_refinement_member(&mut self) -> S::Output {
        // SPEC:
        // type-refinement-member:
        //   type  name  type-constraints
        //   type  name  =  type-specifier
        //   type  name  type-constraints
        //   ctx  name  ctx-constraints
        //   ctx  name  =  context-list
        //
        // type-constraints:
        //   type-constraint
        //   type-constraint  type-constraints
        //
        // ctx-constraints:
        //   context-constraint
        //   context-constraint  ctx-constraints
        match self.peek_token_kind() {
            TokenKind::Type => {
                let keyword = self.assert_token(TokenKind::Type);
                let (name, type_params) = self.parse_nonreserved_name_maybe_parameterized();
                let constraints = self.with_type_parser(|p| {
                    p.parse_list_until_none(|p| p.parse_type_constraint_opt(true))
                });
                let (equal_token, type_specifier) = self.parse_equal_type();
                self.sc_mut().make_type_in_refinement(
                    keyword,
                    name,
                    type_params,
                    constraints,
                    equal_token,
                    type_specifier,
                )
            }
            TokenKind::Ctx => {
                let keyword = self.assert_token(TokenKind::Ctx);
                let (name, type_params) = self.parse_nonreserved_name_maybe_parameterized();
                let constraints = self.with_type_parser(|p| {
                    p.parse_list_until_none(|p| p.parse_context_constraint_opt())
                });
                let (equal_token, ctx_list) = self.parse_equal_contexts();
                self.sc_mut().make_ctx_in_refinement(
                    keyword,
                    name,
                    type_params,
                    constraints,
                    equal_token,
                    ctx_list,
                )
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC:
    // attribute_specification :=
    //   attribute_list
    //   old_attribute_specification
    // attribute_list :=
    //   attribute
    //   attribute_list attribute
    // attribute := @ attribute_name attribute_value_list_opt
    // old_attribute_specification := << old_attribute_list >>
    // old_attribute_list :=
    //   old_attribute
    //   old_attribute_list , old_attribute
    // old_attribute := attribute_name attribute_value_list_opt
    // attribute_name := name
    // attribute_value_list := ( attribute_values_opt )
    // attribute_values :=
    //   attribute_value
    //   attribute_values , attribute_value
    // attribute_value := expression
    //
    // TODO: The list of attrs can have a trailing comma. Update the spec.
    // TODO: The list of values can have a trailing comma. Update the spec.
    // (Both these work items are tracked by spec issue 106.)
    pub fn parse_old_attribute_specification_opt(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::LessThanLessThan {
            let (left, items, right) =
                self.parse_double_angled_comma_list_allow_trailing(|x: &mut Self| {
                    x.parse_old_attribute()
                });
            self.sc_mut()
                .make_old_attribute_specification(left, items, right)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_file_attribute_specification_opt(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::LessThanLessThan {
            let left = self.assert_token(TokenKind::LessThanLessThan);
            let keyword = self.assert_token(TokenKind::File);
            let colon = self.require_colon();
            let (items, _) = self.parse_comma_list_allow_trailing(
                TokenKind::GreaterThanGreaterThan,
                Errors::expected_user_attribute,
                |x: &mut Self| x.parse_old_attribute(),
            );
            let right = self.require_token(TokenKind::GreaterThanGreaterThan, Errors::error1029);
            self.sc_mut()
                .make_file_attribute_specification(left, keyword, colon, items, right)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_return_readonly_opt(&mut self) -> S::Output {
        let token_kind = self.peek_token_kind();
        if token_kind == TokenKind::Readonly {
            let token = self.next_token();
            return self.sc_mut().make_token(token);
        } else {
            let pos = self.pos();
            return self.sc_mut().make_missing(pos);
        }
    }

    fn parse_return_type_hint_opt(&mut self) -> (S::Output, S::Output, S::Output) {
        let token_kind = self.peek_token_kind();
        if token_kind == TokenKind::Colon {
            let token = self.next_token();
            let colon_token = self.sc_mut().make_token(token);
            let readonly_opt = self.parse_return_readonly_opt();
            // if no readonly keyword return type must exist
            // else return type is optional
            let return_type = if readonly_opt.is_missing() {
                self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_return_type())
            } else {
                self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_return_type_opt())
            };
            (colon_token, readonly_opt, return_type)
        } else {
            let pos = self.pos();
            let missing1 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing2 = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let missing3 = self.sc_mut().make_missing(pos);
            (missing1, missing2, missing3)
        }
    }

    pub fn parse_parameter_list_opt(&mut self) -> (S::Output, S::Output, S::Output) {
        // SPEC
        // TODO: The specification is wrong in several respects concerning
        // variadic parameters. Variadic parameters are permitted to have a
        // type and name but this is not mentioned in the spec. And variadic
        // parameters are not mentioned at all in the grammar for constructor
        // parameter lists.  (This is tracked by spec issue 107.)
        //
        // parameter-list:
        //   variadic-parameter
        //   parameter-declaration-list
        //   parameter-declaration-list  ,
        //   parameter-declaration-list  ,  variadic-parameter
        //
        // parameter-declaration-list:
        //   parameter-declaration
        //   parameter-declaration-list  ,  parameter-declaration
        //
        // variadic-parameter:
        //   ...
        //   attribute-specification-opt visiblity-modifier-opt type-specifier \
        //     ...  variable-name
        //
        // This function parses the parens as well.
        // ERROR RECOVERY: We allow variadic parameters in all positions; a later
        // pass gives an error if a variadic parameter is in an incorrect position
        // or followed by a trailing comma, or if the parameter has a
        // default value.
        self.parse_parenthesized_comma_list_opt_allow_trailing(|x: &mut Self| x.parse_parameter())
    }

    fn parse_parameter(&mut self) -> S::Output {
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::DotDotDot => {
                let next_kind = parser1.peek_token_kind();
                if next_kind == TokenKind::Variable {
                    self.parse_parameter_declaration()
                } else {
                    let pos = self.pos();
                    let missing1 = parser1.sc_mut().make_missing(pos);
                    let pos = self.pos();
                    let missing2 = parser1.sc_mut().make_missing(pos);
                    self.continue_from(parser1);
                    let token = self.sc_mut().make_token(token);
                    self.sc_mut()
                        .make_variadic_parameter(missing1, missing2, token)
                }
            }
            _ => self.parse_parameter_declaration(),
        }
    }

    fn parse_parameter_declaration(&mut self) -> S::Output {
        // SPEC
        //
        // TODO: Add call-convention-opt to the specification.
        // (This work is tracked by task T22582676.)
        //
        // TODO: Update grammar for inout parameters.
        // (This work is tracked by task T22582715.)
        //
        // parameter-declaration:
        //   attribute-specification-opt \
        //   call-convention-opt \
        //   mutability-opt \
        //   type-specifier  variable-name \
        //   default-argument-specifier-opt
        //
        // ERROR RECOVERY
        // In strict mode, we require a type specifier. This error is not caught
        // at parse time but rather by a later pass.
        // Visibility modifiers are only legal in constructor parameter
        // lists; we give an error in a later pass.
        // Variadic params cannot be declared inout; we permit that here but
        // give an error in a later pass.
        // Variadic params and inout params cannot have default values; these
        // errors are also reported in a later pass.
        let attrs = self.parse_attribute_specification_opt();
        let visibility = self.parse_visibility_modifier_opt();
        let callconv = self.parse_call_convention_opt();
        let readonly = self.parse_readonly_opt();
        let token = self.peek_token();
        let type_specifier = match token.kind() {
            TokenKind::Variable | TokenKind::DotDotDot => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => {
                self.parse_type_specifier(/* allow_var = */ false, /* allow_attr */ false)
            }
        };
        let name = self.parse_decorated_variable_opt();
        let default = self.parse_simple_initializer_opt();
        self.sc_mut().make_parameter_declaration(
            attrs,
            visibility,
            callconv,
            readonly,
            type_specifier,
            name,
            default,
        )
    }

    fn parse_decorated_variable_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::DotDotDot => self.parse_decorated_variable(),
            _ => self.require_variable(),
        }
    }

    // TODO: This is wrong. The variable here is not anexpression* that has
    // an optional decoration on it.  It's a declaration. We shouldn't be using the
    // same data structure for a decorated expression as a declaration; one
    // is ause* and the other is a *definition*.
    fn parse_decorated_variable(&mut self) -> S::Output {
        // ERROR RECOVERY
        // Detection of (variadic, byRef) inout params happens in post-parsing.
        // Although a parameter can have at most one variadic/reference decorator,
        // we deliberately allow multiple decorators in the initial parse and produce
        // an error in a later pass.
        let decorator = self.fetch_token();
        let variable = match self.peek_token_kind() {
            TokenKind::DotDotDot => self.parse_decorated_variable(),
            _ => self.require_variable(),
        };
        self.sc_mut().make_decorated_expression(decorator, variable)
    }

    fn parse_visibility_modifier_opt(&mut self) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Public | TokenKind::Protected | TokenKind::Private | TokenKind::Internal => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC
    //
    // TODO: Add this to the specification.
    // (This work is tracked by task T22582676.)
    //
    // call-convention:
    //   inout
    fn parse_call_convention_opt(&mut self) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Inout => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC
    //
    // TODO: Add this to the specification.
    // (This work is tracked by task T22582676.)
    //
    // readonly:
    //   readonly
    fn parse_readonly_opt(&mut self) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Readonly => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC
    // default-argument-specifier:
    //   =  const-expression
    //
    // constant-initializer:
    //   =  const-expression
    fn parse_simple_initializer_opt(&mut self) -> S::Output {
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Equal => {
                let token = self.next_token();
                // TODO: Detect if expression is not const
                let token = self.sc_mut().make_token(token);
                let default_value = self.parse_expression();
                self.sc_mut().make_simple_initializer(token, default_value)
            }
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    pub fn parse_function_declaration(&mut self, attribute_specification: S::Output) -> S::Output {
        let modifiers = self.parse_modifiers();
        let header =
            self.parse_function_declaration_header(modifiers, /* is_methodish =*/ false);
        let body = self.parse_compound_statement();
        self.sc_mut()
            .make_function_declaration(attribute_specification, header, body)
    }

    fn parse_constraint_operator(&mut self) -> S::Output {
        // TODO: Put this in the specification
        // (This work is tracked by spec issue 100.)
        // constraint-operator:
        //   =
        //   as
        //   super
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Equal | TokenKind::As | TokenKind::Super => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            _ =>
            // ERROR RECOVERY: don't eat the offending token.
            // TODO: Give parse error
            {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    fn parse_where_constraint(&mut self) -> S::Output {
        // TODO: Put this in the specification
        // (This work is tracked by spec issue 100.)
        // constraint:
        //   type-specifier  constraint-operator  type-specifier
        let left =
            self.parse_type_specifier(/* allow_var = */ false, /* allow_attr = */ true);
        let op = self.parse_constraint_operator();
        let right =
            self.parse_type_specifier(/* allow_var = */ false, /* allow_attr = */ true);
        self.sc_mut().make_where_constraint(left, op, right)
    }

    fn parse_where_constraint_list_item(&mut self) -> Option<S::Output> {
        match self.peek_token_kind() {
            TokenKind::Semicolon | TokenKind::LeftBrace => None,
            _ => {
                let where_constraint = self.parse_where_constraint();
                let comma = self.optional_token(TokenKind::Comma);
                let result = self.sc_mut().make_list_item(where_constraint, comma);
                Some(result)
            }
        }
    }

    fn parse_where_clause(&mut self) -> S::Output {
        // TODO: Add this to the specification
        // (This work is tracked by spec issue 100.)
        // where-clause:
        //   where   constraint-list
        //
        // constraint-list:
        //   constraint
        //   constraint-list , constraint
        let keyword = self.assert_token(TokenKind::Where);
        let constraints =
            self.parse_list_until_none(|x: &mut Self| x.parse_where_constraint_list_item());
        self.sc_mut().make_where_clause(keyword, constraints)
    }

    fn parse_where_clause_opt(&mut self) -> S::Output {
        if self.peek_token_kind() != TokenKind::Where {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        } else {
            self.parse_where_clause()
        }
    }

    fn parse_function_declaration_header(
        &mut self,
        modifiers: S::Output,
        is_methodish: bool,
    ) -> S::Output {
        // SPEC
        // function-definition-header:
        //   attribute-specification-opt  async-opt  function  name  /
        //   generic-type-parameter-list-opt  (  parameter-list-opt  ) :  /
        //   return-type   where-clause-opt
        // TODO: The spec does not specify "where" clauses. Add them.
        // (This work is tracked by spec issue 100.)
        //
        // In strict mode, we require a type specifier. This error is not caught
        // at parse time but rather by a later pass.
        let function_token = self.require_function();
        let label = self.parse_function_label_opt(is_methodish);
        let generic_type_parameter_list = self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_generic_type_parameter_list_opt()
        });
        let (left_paren_token, parameter_list, right_paren_token) = self.parse_parameter_list_opt();
        let contexts = self.with_type_parser(|p: &mut TypeParser<'a, S>| p.parse_contexts());
        let (colon_token, readonly_opt, return_type) = self.parse_return_type_hint_opt();
        let where_clause = self.parse_where_clause_opt();
        self.sc_mut().make_function_declaration_header(
            modifiers,
            function_token,
            label,
            generic_type_parameter_list,
            left_paren_token,
            parameter_list,
            right_paren_token,
            contexts,
            colon_token,
            readonly_opt,
            return_type,
            where_clause,
        )
    }

    // A function label is either a function name or a __construct label.
    fn parse_function_label_opt(&mut self, is_methodish: bool) -> S::Output {
        let report_error = |x: &mut Self, token: Token<S>| {
            x.with_error(Errors::error1044);
            let token = x.sc_mut().make_token(token);
            x.sc_mut().make_error(token)
        };
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::Name | TokenKind::Construct => {
                let token = self.next_token();
                self.sc_mut().make_token(token)
            }
            TokenKind::LeftParen => {
                // It turns out, it was just a verbose lambda; YOLO PHP
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            TokenKind::Isset | TokenKind::Unset | TokenKind::Empty => {
                // We need to parse those as names as they are defined in hhi
                let token = self.next_token_as_name();
                self.sc_mut().make_token(token)
            }
            _ => {
                let token = if is_methodish {
                    self.next_token_as_name()
                } else {
                    self.next_token_non_reserved_as_name()
                };
                if token.kind() == TokenKind::Name {
                    self.sc_mut().make_token(token)
                } else {
                    // ERROR RECOVERY: Eat the offending token.
                    report_error(self, token)
                }
            }
        }
    }

    fn parse_old_attribute(&mut self) -> S::Output {
        self.with_expression_parser(|p: &mut ExpressionParser<'a, S>| p.parse_constructor_call())
    }

    pub fn parse_attribute_specification_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::At if self.env.allow_new_attribute_syntax => {
                self.parse_new_attribute_specification_opt()
            }
            TokenKind::LessThanLessThan => self.parse_old_attribute_specification_opt(),
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    fn parse_new_attribute_specification_opt(&mut self) -> S::Output {
        let attributes = self.parse_list_while(
            |p: &mut Self| p.parse_new_attribute(),
            |p: &Self| p.peek_token_kind() == TokenKind::At,
        );
        self.sc_mut().make_attribute_specification(attributes)
    }

    fn parse_new_attribute(&mut self) -> S::Output {
        let at = self.assert_token(TokenKind::At);
        let token = self.peek_token();
        let constructor_call = match token.kind() {
            TokenKind::Name => self.with_expression_parser(|p: &mut ExpressionParser<'a, S>| {
                p.parse_constructor_call()
            }),
            _ => {
                self.with_error(Errors::expected_user_attribute);
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        };
        self.sc_mut().make_attribute(at, constructor_call)
    }

    // Parses modifiers and passes them into the parse methods for the
    // respective class body element.
    fn parse_methodish_or_property_or_const_or_type_const(
        &mut self,
        attribute_spec: S::Output,
    ) -> S::Output {
        let mut parser1 = self.clone();
        let modifiers = parser1.parse_modifiers();
        let kind0 = parser1.peek_token_kind_with_lookahead(0);
        let kind1 = parser1.peek_token_kind_with_lookahead(1);
        let kind2 = parser1.peek_token_kind_with_lookahead(2);
        match (kind0, kind1, kind2) {
            (TokenKind::Const, TokenKind::Type, TokenKind::Semicolon) => {
                self.continue_from(parser1);
                let const_ = self.assert_token(TokenKind::Const);
                self.parse_const_declaration(attribute_spec, modifiers, const_)
            }
            (TokenKind::Const, TokenKind::Type, _) if kind2 != TokenKind::Equal => {
                let modifiers = self.parse_modifiers();
                let const_ = self.assert_token(TokenKind::Const);
                self.parse_type_const_declaration(attribute_spec, modifiers, const_)
            }
            (TokenKind::Const, TokenKind::Ctx, _) if kind2 != TokenKind::Equal => {
                let modifiers = self.parse_modifiers();
                let const_ = self.assert_token(TokenKind::Const);
                self.parse_context_const_declaration(modifiers, const_)
            }
            (TokenKind::Const, _, _) => {
                self.continue_from(parser1);
                let const_ = self.assert_token(TokenKind::Const);
                self.parse_const_declaration(attribute_spec, modifiers, const_)
            }
            _ => self.parse_methodish_or_property(attribute_spec),
        }
    }

    // SPEC
    // method-declaration:
    //   attribute-spec-opt method-modifiers function-definition
    //   attribute-spec-opt method-modifiers function-definition-header ;
    // method-modifiers:
    //   method-modifier
    //   method-modifiers method-modifier
    // method-modifier:
    //   visibility-modifier (i.e. private, public, protected)
    //   static
    //   abstract
    //   final
    fn parse_methodish(&mut self, attribute_spec: S::Output, modifiers: S::Output) -> S::Output {
        let header =
            self.parse_function_declaration_header(modifiers, /* is_methodish:*/ true);
        let token_kind = self.peek_token_kind();
        match token_kind {
            TokenKind::LeftBrace => {
                let body = self.parse_compound_statement();
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.sc_mut()
                    .make_methodish_declaration(attribute_spec, header, body, missing)
            }
            TokenKind::Semicolon => {
                let pos = self.pos();
                let token = self.next_token();
                let missing = self.sc_mut().make_missing(pos);
                let semicolon = self.sc_mut().make_token(token);
                self.sc_mut()
                    .make_methodish_declaration(attribute_spec, header, missing, semicolon)
            }
            TokenKind::Equal => {
                let equal = self.assert_token(TokenKind::Equal);
                let qualifier = self.parse_qualified_name_type();
                let cc_token = self.require_coloncolon();
                let name = self.require_token_one_of(
                    &[TokenKind::Name, TokenKind::Construct],
                    Errors::error1004,
                );
                let name = self
                    .sc_mut()
                    .make_scope_resolution_expression(qualifier, cc_token, name);
                let semi = self.require_semicolon();
                self.sc_mut().make_methodish_trait_resolution(
                    attribute_spec,
                    header,
                    equal,
                    name,
                    semi,
                )
            }
            _ => {
                // ERROR RECOVERY: We expected either a block or a semicolon; we got
                // neither. Use the offending token as the body of the method.
                // TODO: Is this the right error recovery?
                let pos = self.pos();
                let token = self.next_token();
                self.with_error(Errors::error1041);
                let token = self.sc_mut().make_token(token);
                let error = self.sc_mut().make_error(token);
                let missing = self.sc_mut().make_missing(pos);
                self.sc_mut()
                    .make_methodish_declaration(attribute_spec, header, error, missing)
            }
        }
    }
    fn parse_modifiers(&mut self) -> S::Output {
        let mut items = vec![];
        loop {
            let token_kind = self.peek_token_kind();
            match token_kind {
                TokenKind::Abstract
                | TokenKind::Static
                | TokenKind::Public
                | TokenKind::Protected
                | TokenKind::Private
                | TokenKind::Async
                | TokenKind::Internal
                | TokenKind::Final
                | TokenKind::Readonly => {
                    let token = self.next_token();
                    let item = self.sc_mut().make_token(token);
                    items.push(item)
                }
                _ => break,
            }
        }
        let pos = self.pos();
        self.sc_mut().make_list(items, pos)
    }

    fn parse_toplevel_with_attributes_or_visibility(&mut self) -> S::Output {
        // An enum, type alias, function, interface, trait or class may all
        // begin with an attribute.
        let attribute_specification = match self.peek_token_kind() {
            TokenKind::At | TokenKind::LessThanLessThan => self.parse_attribute_specification_opt(),
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        };

        // Check for internal keyword; if internal, we want to track that info and look at the token after to determine
        // what we're parsing
        // For most toplevel entities we will parse internal as another modifier with no issues,
        // but we will need to lookahead at least 1 extra spot to see what we're actually parsing.
        let next_token_kind = self.peek_token_kind();
        let (token_maybe_after_internal, has_visibility) =
            if next_token_kind == TokenKind::Internal || next_token_kind == TokenKind::Public {
                (self.peek_token_kind_with_lookahead(1), true)
            } else {
                (next_token_kind, false)
            };

        match token_maybe_after_internal {
            TokenKind::Enum => {
                let modifiers = if has_visibility {
                    self.parse_modifiers()
                } else {
                    let pos = self.pos();
                    self.sc_mut().make_missing(pos)
                };
                self.parse_enum_or_enum_class_declaration(attribute_specification, modifiers)
            }
            TokenKind::Module
                if !has_visibility
                    && self.peek_token_kind_with_lookahead(1) == TokenKind::Newtype =>
            {
                self.parse_type_alias_declaration(attribute_specification, true)
            }
            TokenKind::Type | TokenKind::Newtype => {
                self.parse_type_alias_declaration(attribute_specification, false)
            }
            TokenKind::Newctx => self.parse_ctx_alias_declaration(attribute_specification),
            TokenKind::Async | TokenKind::Function => {
                if attribute_specification.is_missing() && !has_visibility {
                    // if attribute section is missing - it might be either
                    // function declaration or expression statement containing
                    // anonymous function - use statement parser to determine in which case
                    // we are currently in
                    self.with_statement_parser(|p: &mut StatementParser<'a, S>| {
                        p.parse_possible_php_function(/* toplevel=*/ true)
                    })
                } else {
                    self.parse_function_declaration(attribute_specification)
                }
            }
            TokenKind::Abstract
            | TokenKind::Final
            | TokenKind::Interface
            | TokenKind::Trait
            | TokenKind::XHP
            | TokenKind::Class => self.parse_classish_declaration(attribute_specification),
            // The only thing that can put an attribute on a new is a module declaration
            TokenKind::New
                if !has_visibility
                    && self.peek_token_kind_with_lookahead(1) == TokenKind::Module =>
            {
                self.parse_module_declaration(attribute_specification)
            }
            TokenKind::Package => self.parse_package_declaration(attribute_specification),
            _ => {
                // ERROR RECOVERY: we encountered an unexpected token, raise an error and continue
                // TODO: This is wrong; we have lost the attribute specification
                // from the tree.
                let token = self.next_token();
                self.with_error(Errors::error1057(self.token_text(&token)));
                let token = self.sc_mut().make_token(token);
                self.sc_mut().make_error(token)
            }
        }
    }

    fn parse_classish_element(&mut self) -> S::Output {
        // We need to identify an element of a class, trait, etc. Possibilities
        // are:
        //
        // // constant-declaration:
        // const T $x = v ;
        // abstract const T $x ;
        // public const T $x = v ; // PHP7 only
        //
        // // type-constant-declaration
        // const type T = X;
        // abstract const type T;
        //
        // // property-declaration:
        // public/private/protected/internal/static T $x;
        // TODO: We may wish to parse "T $x" and give an error indicating
        // TODO: that we were expecting either const or public.
        // Note that a visibility modifier is required; static is optional;
        // any order is allowed.
        //
        // // method-declaration
        // <<attr>> public/private/protected/abstract/final/static async function
        // Note that a modifier is required, the attr and async are optional.
        // TODO: Hack requires a visibility modifier, unless "static" is supplied,
        // TODO: in which case the method is considered to be public.  Is this
        // TODO: desired? Resolve this disagreement with the spec.
        //
        // // constructor-declaration
        // <<attr>> public/private/protected/abstract/final function __construct
        // Note that we allow static constructors in this parser; we produce an
        // error in the post-parse error detection pass.
        //
        // // trait clauses
        // require  extends  qualified-name
        // require  implements  qualified-name
        //
        // // XHP class attribute declaration
        // attribute ... ;
        //
        // // XHP category declaration
        // category ... ;
        //
        // // XHP children declaration
        // children ... ;
        match self.peek_token_kind() {
            TokenKind::Children => self.parse_xhp_children_declaration(),
            TokenKind::Category => self.parse_xhp_category_declaration(),
            TokenKind::Use => self.parse_trait_use(),
            TokenKind::Const
            | TokenKind::Abstract
            | TokenKind::Public
            | TokenKind::Protected
            | TokenKind::Private
            | TokenKind::Readonly
            | TokenKind::Internal
            | TokenKind::Static
            | TokenKind::Async
            | TokenKind::Final
            | TokenKind::LessThanLessThan => {
                let attr = self.parse_attribute_specification_opt();
                self.parse_methodish_or_property_or_const_or_type_const(attr)
            }
            TokenKind::At if self.env.allow_new_attribute_syntax => {
                let attr = self.parse_attribute_specification_opt();
                self.parse_methodish_or_property_or_const_or_type_const(attr)
            }
            TokenKind::Require => {
                // We give an error if these are found where they should not be,
                // in a later pass.
                self.parse_require_clause()
            }
            TokenKind::Attribute => self.parse_xhp_class_attribute_declaration(),
            TokenKind::Function => {
                // ERROR RECOVERY
                // Hack requires that a function inside a class be marked
                // with a visibility modifier, but PHP does not have this requirement.
                // We accept the lack of a modifier here, and produce an error in
                // a later pass.
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                self.parse_methodish(missing1, missing2)
            }
            kind if self.expects(kind) => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
            _ => {
                // If this is a property declaration which is missing its visibility
                // modifier, accept it here, but emit an error in a later pass.
                let mut parser1 = self.clone();
                let pos = self.pos();
                let missing1 = parser1.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = parser1.sc_mut().make_missing(pos);
                let property = parser1.parse_property_declaration(missing1, missing2);
                if self.errors.len() == parser1.errors.len() {
                    self.continue_from(parser1);
                    property
                } else {
                    // TODO ERROR RECOVERY could be improved here.
                    let token = self.fetch_token();
                    self.with_error(Errors::error1033);
                    self.sc_mut().make_error(token)
                    // Parser does not detect the error where non-static instance variables
                    // or methods are within abstract final classes in its first pass, but
                    // instead detects it in its second pass.
                }
            }
        }
    }

    fn parse_type_constraint_opt(&mut self, allow_super: bool) -> S::Output {
        self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_type_constraint_opt(allow_super).unwrap_or_else(|| {
                let pos = p.pos();
                p.sc_mut().make_missing(pos)
            })
        })
    }

    fn parse_type_constraints(&mut self, allow_super: bool) -> S::Output {
        self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_list_until_none(|p| p.parse_type_constraint_opt(allow_super))
        })
    }

    fn parse_type_alias_declaration(&mut self, attr: S::Output, module_newtype: bool) -> S::Output {
        // SPEC
        // alias-declaration:
        //   attribute-spec-opt modifiers type  name
        //     generic-type-parameter-list-opt  =  type-specifier  ;
        //   attribute-spec-opt modifiers newtype  name
        //     generic-type-parameter-list-opt type-constraint-opt
        //       =  type-specifier  ;
        let modifiers = self.parse_modifiers();
        let module_kw_opt = if module_newtype {
            self.assert_token(TokenKind::Module)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        };
        let token = self.fetch_token();
        // Not `require_name` but `require_name_allow_non_reserved`, because the parser
        // must allow keywords in the place of identifiers; at least to parse .hhi
        // files.
        let name = self.require_name_allow_non_reserved();
        let generic = self.with_type_parser(|p: &mut TypeParser<'a, S>| {
            p.parse_generic_type_parameter_list_opt()
        });
        let constraints = self.parse_type_constraints(true);
        let equal = self.require_equal();
        let ty = self.parse_type_specifier(false /* allow_var */, true /* allow_attr */);
        let semi = self.require_semicolon();
        self.sc_mut().make_alias_declaration(
            attr,
            modifiers,
            module_kw_opt,
            token,
            name,
            generic,
            constraints,
            equal,
            ty,
            semi,
        )
    }

    fn parse_ctx_alias_declaration(&mut self, attr: S::Output) -> S::Output {
        // SPEC
        // ctx-alias-declaration:
        //   newctx name type-constraint-opt = type-specifier  ;
        let token = self.fetch_token();
        let name = self.require_name();
        let pos = self.pos();
        let generic = self.sc_mut().make_missing(pos);
        let constr = self
            .with_type_parser(|p| p.parse_list_until_none(|p| p.parse_context_constraint_opt()));
        let (equal, ty) = if self.peek_token_kind() == TokenKind::Equal {
            let equal = self.require_equal();
            let ty = self.with_type_parser(|p| p.parse_contexts());
            (equal, ty)
        } else {
            let pos = self.pos();
            let equal = self.sc_mut().make_missing(pos);
            let pos = self.pos();
            let ty = self.sc_mut().make_missing(pos);
            (equal, ty)
        };
        let semi = self.require_semicolon();
        self.sc_mut()
            .make_context_alias_declaration(attr, token, name, generic, constr, equal, ty, semi)
    }

    fn parse_enumerator(&mut self) -> S::Output {
        // SPEC
        // enumerator:
        //   enumerator-constant  =  constant-expression ;
        // enumerator-constant:
        //   name
        //
        // TODO: Add an error to a later pass that determines the value is
        // a constant.

        // TODO: We must allow TRUE to be a legal enum member name; here we allow
        // any keyword.  Consider making this more strict.
        self.sc_mut().begin_enumerator();
        let name = self.require_name_allow_all_keywords();
        let equal = self.require_equal();
        let value = self.parse_expression();
        let semicolon = self.require_semicolon();
        self.sc_mut().make_enumerator(name, equal, value, semicolon)
    }

    fn parse_enum_class_enumerator(&mut self) -> S::Output {
        // SPEC
        // enum-class-enumerator:
        //   type-specifier name = expression ;
        //   abstract type-specifier name ;
        // Taken from parse_enumerator:
        // TODO: We must allow TRUE to be a legal enum member name; here we allow
        // any keyword.  Consider making this more strict.
        self.sc_mut().begin_enum_class_enumerator();
        let modifiers = self.parse_modifiers();
        let ty = self.parse_type_specifier(/*allow_var*/ false, /*allow_attr*/ false);
        let name = self.require_name_allow_all_keywords();
        let initializer_ = self.parse_simple_initializer_opt();
        let semicolon = self.require_semicolon();
        self.sc_mut()
            .make_enum_class_enumerator(modifiers, ty, name, initializer_, semicolon)
    }

    fn parse_inclusion_directive(&mut self) -> S::Output {
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
        let expr = self.parse_expression();
        let semi = self.require_semicolon();
        self.sc_mut().make_inclusion_directive(expr, semi)
    }

    fn parse_module_declaration(&mut self, attrs: S::Output) -> S::Output {
        let new_kw = self.assert_token(TokenKind::New);
        let module_kw = self.assert_token(TokenKind::Module);

        let name = self.require_qualified_module_name();
        let lb = self.require_left_brace();
        let pos = self.pos();
        let mut exports_block = self.sc_mut().make_missing(pos);
        let mut imports_block = self.sc_mut().make_missing(pos);
        loop {
            let kind = self.peek_token_kind();

            match kind {
                TokenKind::Exports => {
                    let exports = self.require_token(TokenKind::Exports, Errors::error1004);
                    let exports_lb = self.require_left_brace();
                    let clauses = self.parse_comma_list_allow_trailing_opt(
                        TokenKind::RightBrace,
                        Errors::error1004,
                        |x: &mut Self| x.require_qualified_referenced_module_name(),
                    );
                    let exports_rb = self.require_right_brace();

                    if !exports_block.is_missing() {
                        self.with_error(Errors::error1066);
                    }

                    exports_block = self
                        .sc_mut()
                        .make_module_exports(exports, exports_lb, clauses, exports_rb);
                }
                TokenKind::Imports => {
                    let imports = self.require_token(TokenKind::Imports, Errors::error1004);
                    let imports_lb = self.require_left_brace();
                    let clauses = self.parse_comma_list_allow_trailing_opt(
                        TokenKind::RightBrace,
                        Errors::error1004,
                        |x: &mut Self| x.require_qualified_referenced_module_name(),
                    );
                    let imports_rb = self.require_right_brace();

                    if !imports_block.is_missing() {
                        self.with_error(Errors::error1067);
                    }

                    imports_block = self
                        .sc_mut()
                        .make_module_imports(imports, imports_lb, clauses, imports_rb);
                }
                _ => {
                    let rb = self.require_right_brace();
                    return self.sc_mut().make_module_declaration(
                        attrs,
                        new_kw,
                        module_kw,
                        name,
                        lb,
                        exports_block,
                        imports_block,
                        rb,
                    );
                }
            }
        }
    }

    fn parse_module_membership_declaration(&mut self) -> S::Output {
        let module_kw = self.assert_token(TokenKind::Module);
        let name = self.require_qualified_module_name();
        let semicolon = self.require_semicolon();
        self.sc_mut()
            .make_module_membership_declaration(module_kw, name, semicolon)
    }

    fn parse_package_declaration(&mut self, attrs: S::Output) -> S::Output {
        let package_kw = self.assert_token(TokenKind::Package);

        let name = self.require_token(TokenKind::Name, Errors::error1004);
        let lb = self.require_left_brace();
        let pos = self.pos();
        let mut uses_block = self.sc_mut().make_missing(pos);
        let mut includes_block = self.sc_mut().make_missing(pos);
        loop {
            let kind = self.peek_token_kind();

            match kind {
                TokenKind::Use => {
                    let use_kw = self.require_token(TokenKind::Use, Errors::error1070);
                    let use_lb = self.require_left_brace();
                    let clauses = self.parse_comma_list_allow_trailing_opt(
                        TokenKind::RightBrace,
                        Errors::error1004,
                        |x: &mut Self| x.require_qualified_referenced_module_name(),
                    );
                    let use_rb = self.require_right_brace();

                    if !uses_block.is_missing() {
                        self.with_error(Errors::error1068);
                    }

                    uses_block = self
                        .sc_mut()
                        .make_package_uses(use_kw, use_lb, clauses, use_rb);
                }
                TokenKind::Include => {
                    let include_kw = self.require_token(TokenKind::Include, Errors::error1070);
                    let include_lb = self.require_left_brace();
                    let clauses = self.parse_comma_list_allow_trailing_opt(
                        TokenKind::RightBrace,
                        Errors::error1004,
                        |x: &mut Self| x.require_name(),
                    );
                    let include_rb = self.require_right_brace();

                    if !includes_block.is_missing() {
                        self.with_error(Errors::error1069);
                    }

                    includes_block = self
                        .sc_mut()
                        .make_package_includes(include_kw, include_lb, clauses, include_rb);
                }
                _ => {
                    let rb = self.require_right_brace();
                    return self.sc_mut().make_package_declaration(
                        attrs,
                        package_kw,
                        name,
                        lb,
                        uses_block,
                        includes_block,
                        rb,
                    );
                }
            }
        }
    }

    fn parse_declaration(&mut self) -> S::Output {
        self.expect_in_new_scope(ExpectedTokens::Classish);
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        let result = match token.kind() {
            TokenKind::Include
            | TokenKind::Include_once
            | TokenKind::Require
            | TokenKind::Require_once => self.parse_inclusion_directive(),
            TokenKind::Type | TokenKind::Newtype
                if {
                    let kind = parser1.peek_token_kind();
                    kind == TokenKind::Name || kind == TokenKind::Classname
                } =>
            {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_type_alias_declaration(missing, false)
            }
            TokenKind::Newctx => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_ctx_alias_declaration(missing)
            }
            TokenKind::Enum => {
                let pos = self.pos();
                let missing_attr = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing_mod = self.sc_mut().make_missing(pos);
                self.parse_enum_or_enum_class_declaration(missing_attr, missing_mod)
            }
            // The keyword namespace before a name should be parsed as
            // "the current namespace we are in", essentially a no op.
            // example:
            // namespace\f1(); should be parsed as a call to the function f1 in
            // the current namespace.
            TokenKind::Namespace if parser1.peek_token_kind() == TokenKind::Backslash => {
                self.with_statement_parser(|p: &mut StatementParser<'a, S>| p.parse_statement())
            }
            TokenKind::Namespace => self.parse_namespace_declaration(),
            TokenKind::Use => self.parse_namespace_use_declaration(),
            TokenKind::Trait | TokenKind::Interface | TokenKind::Class => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_classish_declaration(missing)
            }
            TokenKind::Abstract | TokenKind::Final | TokenKind::XHP => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_classish_declaration(missing)
            }
            TokenKind::Async | TokenKind::Function => {
                self.with_statement_parser(|p: &mut StatementParser<'a, S>| {
                    p.parse_possible_php_function(true)
                })
            }
            TokenKind::Internal => self.parse_toplevel_with_attributes_or_visibility(),
            TokenKind::Public => self.parse_toplevel_with_attributes_or_visibility(),
            TokenKind::At if self.env.allow_new_attribute_syntax => {
                self.parse_toplevel_with_attributes_or_visibility()
            }
            TokenKind::LessThanLessThan => match parser1.peek_token_kind() {
                TokenKind::File
                    if parser1.peek_token_kind_with_lookahead(1) == TokenKind::Colon =>
                {
                    self.parse_file_attribute_specification_opt()
                }
                _ => self.parse_toplevel_with_attributes_or_visibility(),
            },
            // TODO figure out what global const differs from class const
            TokenKind::Const => {
                let pos = self.pos();
                let missing1 = parser1.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = parser1.sc_mut().make_missing(pos);
                self.continue_from(parser1);
                let token = self.sc_mut().make_token(token);
                self.parse_const_declaration(missing1, missing2, token)
            }
            TokenKind::Module if parser1.peek_token_kind() == TokenKind::Newtype => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_type_alias_declaration(missing, true)
            }
            TokenKind::Module => self.parse_module_membership_declaration(),
            // If we see new as a token, it's a module definition
            TokenKind::New if parser1.peek_token_kind() == TokenKind::Module => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_module_declaration(missing)
            }
            TokenKind::Package => {
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                self.parse_package_declaration(missing)
            }
            // TODO: What if it's not a legal statement? Do we still make progress here?
            _ => self.with_statement_parser(|p: &mut StatementParser<'a, S>| p.parse_statement()),
        };

        self.pop_scope(ExpectedTokens::Classish);
        result
    }

    pub fn parse_script(&mut self) -> S::Output {
        let header = self.parse_leading_markup_section();
        let mut declarations = vec![];
        if let Some(x) = header {
            declarations.push(x)
        };
        loop {
            let token_kind = self.peek_token_kind();
            match token_kind {
                TokenKind::EndOfFile => {
                    let token = self.next_token();
                    let token = self.sc_mut().make_token(token);
                    let end_of_file = self.sc_mut().make_end_of_file(token);
                    declarations.push(end_of_file);
                    break;
                }
                _ => declarations.push(self.parse_declaration()),
            }
        }
        let pos = self.pos();
        let declarations = self.sc_mut().make_list(declarations, pos);
        let result = self.sc_mut().make_script(declarations);
        assert_eq!(self.peek_token_kind(), TokenKind::EndOfFile);
        result
    }
}
