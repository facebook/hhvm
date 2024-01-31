// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::lexable_token::LexableToken;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_error::{self as Errors};
use parser_core_types::token_kind::TokenKind;

use crate::declaration_parser::DeclarationParser;
use crate::expression_parser::ExpressionParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::Context;
use crate::parser_trait::ParserTrait;
use crate::parser_trait::SeparatedListKind;
use crate::smart_constructors::NodeType;
use crate::smart_constructors::SmartConstructors;
use crate::smart_constructors::Token;

#[derive(Clone)]
pub struct TypeParser<'a, S>
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

impl<'a, S> ParserTrait<'a, S> for TypeParser<'a, S>
where
    S: SmartConstructors,
    S::Output: NodeType,
{
    fn make(
        mut lexer: Lexer<'a, S::Factory>,
        env: ParserEnv,
        context: Context<Token<S>>,
        errors: Vec<SyntaxError>,
        sc: S,
    ) -> Self {
        lexer.set_in_type(true);
        Self {
            lexer,
            env,
            context,
            errors,
            sc,
        }
    }

    fn into_parts(
        mut self,
    ) -> (
        Lexer<'a, S::Factory>,
        Context<Token<S>>,
        Vec<SyntaxError>,
        S,
    ) {
        self.lexer.set_in_type(false);
        (self.lexer, self.context, self.errors, self.sc)
    }

    fn lexer(&self) -> &Lexer<'a, S::Factory> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Factory> {
        &mut self.lexer
    }

    fn continue_from<P: ParserTrait<'a, S>>(&mut self, other: P) {
        let (mut lexer, context, errors, sc) = other.into_parts();
        lexer.set_in_type(true);
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

impl<'a, S> TypeParser<'a, S>
where
    S: SmartConstructors,
    S::Output: NodeType,
{
    fn with_expression_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut ExpressionParser<'a, S>) -> U,
    {
        let mut lexer = self.lexer.clone();
        lexer.set_in_type(false);
        let mut expression_parser: ExpressionParser<'_, S> = ExpressionParser::make(
            lexer,
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

    fn with_decl_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut DeclarationParser<'a, S>) -> U,
    {
        let mut lexer = self.lexer.clone();
        lexer.set_in_type(false);

        let mut declaration_parser: DeclarationParser<'_, S> = DeclarationParser::make(
            lexer,
            self.env.clone(),
            self.context.clone(),
            self.errors.clone(),
            self.sc.clone(),
        );
        let res = f(&mut declaration_parser);
        self.continue_from(declaration_parser);
        res
    }

    // parse type specifier but return missing if you fail to parse
    pub fn parse_type_specifier_opt(&mut self, allow_var: bool, allow_attr: bool) -> S::Output {
        // Strictly speaking, "mixed" is a nullable type specifier. We parse it as
        // a simple type specifier here.
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_class_name_or_other_token();
        let type_spec = match token.kind() {
            | TokenKind::Var if allow_var => {
                self.continue_from(parser1);
                let token = self.sc_mut().make_token(token);
                self.sc_mut().make_simple_type_specifier(token)
            }
            | TokenKind::This => self.parse_simple_type_or_type_constant(),
            | TokenKind::SelfToken => self.parse_type_constant(),
            // Any keyword-type could be a non-keyword type, because PHP, so check whether
            // these have generics.
            | TokenKind::Double // TODO: Specification does not mention double; fix it.
            | TokenKind::Bool
            | TokenKind::Boolean
            | TokenKind::Binary
            | TokenKind::Int
            | TokenKind::Integer
            | TokenKind::Float
            | TokenKind::Real
            | TokenKind::Num
            | TokenKind::String
            | TokenKind::Arraykey
            | TokenKind::Void
            | TokenKind::Noreturn
            | TokenKind::Resource
            | TokenKind::Mixed
            | TokenKind::NullLiteral
            | TokenKind::Name => self.parse_simple_type_or_type_constant_or_generic(),
            | TokenKind::Namespace => {
                let name = self.scan_name_or_qualified_name();
                self.parse_remaining_simple_type_or_type_constant_or_generic(name)
            }
            | TokenKind::Backslash => {
                self.continue_from(parser1);
                let pos = self.pos(); let missing = self.sc_mut().make_missing(pos);
                let token = self.sc_mut().make_token(token);
                let name = self.scan_qualified_name(missing, token);
                self.parse_remaining_simple_type_or_type_constant_or_generic(name)
            }
            | TokenKind::Category
            | TokenKind::XHP
            | TokenKind::XHPClassName => self.parse_simple_type_or_type_constant_or_generic(),
            | TokenKind::Darray => self.parse_darray_type_specifier(),
            | TokenKind::Varray => self.parse_varray_type_specifier(),
            | TokenKind::Vec => self.parse_vec_type_specifier(),
            | TokenKind::Dict => self.parse_dictionary_type_specifier(),
            | TokenKind::Keyset => self.parse_keyset_type_specifier(),
            | TokenKind::Tuple => self.parse_tuple_type_explicit_specifier(),
            | TokenKind::LeftParen => self.parse_tuple_or_closure_type_specifier(),
            | TokenKind::Shape => self.parse_shape_specifier(),
            | TokenKind::Question => self.parse_nullable_type_specifier(),
            | TokenKind::Tilde => self.parse_like_type_specifier(),
            | TokenKind::At => self.parse_soft_type_specifier(),
            | TokenKind::LessThanLessThan if allow_attr => self.parse_attributized_specifier(),
            | TokenKind::Class => self.parse_class_args_type_specifier(),
            | TokenKind::Classname => self.parse_classname_type_specifier(),
            | _ => {
                let pos = self.pos(); self.sc_mut().make_missing(pos)
            }
        };
        match self.peek_token_kind() {
            TokenKind::With if self.peek_token_kind_with_lookahead(1) == TokenKind::LeftBrace => {
                self.parse_type_refinement(type_spec)
            }
            _ => type_spec,
        }
    }

    // TODO: What about something like for::for? Is that a legal type constant?
    pub fn parse_type_specifier(&mut self, allow_var: bool, allow_attr: bool) -> S::Output {
        let result = self.parse_type_specifier_opt(allow_var, allow_attr);
        if result.is_missing() {
            self.with_error_on_whole_token(Errors::error1007, Vec::new());
            let token = self.next_xhp_class_name_or_other_token();
            let token = self.sc_mut().make_token(token);
            self.sc_mut().make_error(token)
        } else {
            result
        }
    }

    fn parse_type_refinement(&mut self, type_spec: S::Output) -> S::Output {
        // SPEC
        // type-refinement:
        //   type-specifier  with  {  type-refinement-members_opt  ;opt  }
        //
        // type-refinement-members:
        //   type-refinement-member  ;  type-refinement-members
        let keyword = self.assert_token(TokenKind::With);
        let left_brace = self.require_left_brace();
        let members =
            self.with_decl_parser(
                |x: &mut DeclarationParser<'a, S>| match x.peek_token_kind() {
                    TokenKind::Type | TokenKind::Ctx => {
                        // Note: blindly calling this without matching on expected token first
                        // would result in confusing error "expected `}`" in error cases such as
                        // `... with { const ... }`
                        x.parse_separated_list(
                            TokenKind::Semicolon,
                            SeparatedListKind::TrailingAllowed,
                            TokenKind::RightBrace,
                            Errors::expected_refinement_member,
                            |x| x.parse_refinement_member(),
                        )
                        .0
                    }
                    tk => {
                        if tk != TokenKind::RightBrace {
                            x.with_error(Errors::expected_refinement_member, Vec::new());
                        }
                        let pos = x.pos();
                        x.sc_mut().make_missing(pos)
                    }
                },
            );
        let right_brace = self.require_right_brace();
        let refinement = self.sc_mut().make_type_refinement(
            type_spec,
            keyword,
            left_brace,
            members,
            right_brace,
        );
        if self.peek_token_kind() == TokenKind::With {
            self.with_error(Errors::cannot_chain_type_refinements, Vec::new());
            // ERROR RECOVERY: nest chained refinement
            return self.parse_type_refinement(refinement);
        }
        refinement
    }

    // SPEC
    // type-constant-type-name:
    //   name  ::  name
    //   self  ::  name
    //   this  ::  name
    //   parent  ::  name
    //   type-constant-type-name  ::  name
    fn parse_remaining_type_constant(&mut self, left: S::Output) -> S::Output {
        let separator = self.fetch_token();
        let right = self.next_token_as_name();
        if right.kind() == TokenKind::Name {
            let right = self.sc_mut().make_token(right);
            let syntax = self.sc_mut().make_type_constant(left, separator, right);
            let token = self.peek_token();
            if token.kind() == TokenKind::ColonColon {
                self.parse_remaining_type_constant(syntax)
            } else {
                syntax
            }
        } else {
            // ERROR RECOVERY: Assume that the thing following the ::
            // that is not a name belongs to the next thing to be
            // parsed; treat the name as missing.
            self.with_error(Errors::error1004, Vec::new());
            let pos = self.pos();
            let missing = self.sc_mut().make_missing(pos);
            self.sc_mut().make_type_constant(left, separator, missing)
        }
    }

    fn parse_remaining_generic(&mut self, name: S::Output) -> S::Output {
        let (arguments, _) = self.parse_generic_type_argument_list();
        self.sc_mut().make_generic_type_specifier(name, arguments)
    }

    pub fn parse_simple_type_or_type_constant(&mut self) -> S::Output {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_type_constant(name)
    }

    pub fn parse_simple_type_or_generic(&mut self) -> S::Output {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_generic(name)
    }

    fn parse_type_constant(&mut self) -> S::Output {
        let name = self.next_xhp_class_name_or_other();
        let token = self.peek_token();
        match token.kind() {
            TokenKind::ColonColon => self.parse_remaining_type_constant(name),
            _ => {
                self.with_error(Errors::error1047, Vec::new());
                self.sc_mut().make_error(name)
            }
        }
    }

    fn parse_remaining_simple_type_or_type_constant(&mut self, name: S::Output) -> S::Output {
        let token = self.peek_token();
        match token.kind() {
            TokenKind::ColonColon => self.parse_remaining_type_constant(name),
            _ => self.sc_mut().make_simple_type_specifier(name),
        }
    }

    fn parse_simple_type_or_type_constant_or_generic(&mut self) -> S::Output {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_type_constant_or_generic(name)
    }

    pub fn parse_remaining_type_specifier(&mut self, name: S::Output) -> S::Output {
        self.parse_remaining_simple_type_or_type_constant_or_generic(name)
    }

    fn parse_remaining_simple_type_or_type_constant_or_generic(
        &mut self,
        name: S::Output,
    ) -> S::Output {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_remaining_generic(name),
            _ => self.parse_remaining_simple_type_or_type_constant(name),
        }
    }

    fn parse_remaining_simple_type_or_generic(&mut self, name: S::Output) -> S::Output {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_remaining_generic(name),
            _ => self.sc_mut().make_simple_type_specifier(name),
        }
    }

    // SPEC
    // generic-type-constraint-list:
    //   generic-type-constraint
    //   generic-type-constraint generic-type-constraint-list
    //
    // generic-type-constraint:
    //   as type-specifier
    //   super type-specifier
    //
    // TODO: SPEC ISSUES:
    // https://github.com/hhvm/hack-langspec/issues/83
    //
    // TODO: Do we also need to allow "= type-specifier" here?
    fn parse_generic_type_constraint_opt(&mut self) -> Option<S::Output> {
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::As | TokenKind::Super => {
                self.continue_from(parser1);
                let constraint_token = self.sc_mut().make_token(token);
                let matched_type = self.parse_type_specifier(false, true);
                let type_constraint = self
                    .sc_mut()
                    .make_type_constraint(constraint_token, matched_type);
                Some(type_constraint)
            }
            _ => None,
        }
    }

    fn parse_variance_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::Plus | TokenKind::Minus => self.fetch_token(),
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC
    // generic-type-parameter:
    //   generic-type-parameter-reified-opt  generic-type-parameter-variance-opt
    //     name type-parameter-list? generic-type-constraint-list-opt
    //
    // generic-type-parameter-variance:
    //   +
    //   -
    //
    // TODO: SPEC ISSUE: We allow any number of type constraints, not just zero
    // or one as indicated in the spec.
    // https://github.com/hhvm/hack-langspec/issues/83
    // TODO: Update the spec with reified
    pub fn parse_type_parameter(&mut self) -> S::Output {
        let attributes = self.with_decl_parser(|x: &mut DeclarationParser<'a, S>| {
            x.parse_attribute_specification_opt()
        });
        let reified = self.optional_token(TokenKind::Reify);
        let variance = self.parse_variance_opt();
        let type_name = self.require_name_allow_all_keywords();
        let param_params = self.parse_generic_type_parameter_list_opt();
        let constraints =
            self.parse_list_until_none(|x: &mut Self| x.parse_generic_type_constraint_opt());
        self.sc_mut().make_type_parameter(
            attributes,
            reified,
            variance,
            type_name,
            param_params,
            constraints,
        )
    }

    pub fn parse_generic_type_parameter_list_opt(&mut self) -> S::Output {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_generic_type_parameter_list(),
            _ => {
                let pos = self.pos();
                self.sc_mut().make_missing(pos)
            }
        }
    }

    // SPEC
    // type-parameter-list:
    // < generic-type-parameters  ,-opt >
    //
    // generic-type-parameters:
    //   generic-type-parameter
    //   generic-type-parameter  ,  generic-type-parameter
    //

    pub fn parse_generic_type_parameter_list(&mut self) -> S::Output {
        let left = self.assert_left_angle_in_type_list_with_possible_attribute();
        let (params, _) = self.parse_comma_list_allow_trailing(
            TokenKind::GreaterThan,
            Errors::error1007,
            |x: &mut Self| x.parse_type_parameter(),
        );

        let right = self.require_right_angle();
        self.sc_mut().make_type_parameters(left, params, right)
    }

    fn parse_type_list(&mut self, close_kind: TokenKind) -> S::Output {
        // SPEC:
        // type-specifier-list:
        //   type-specifiers  ,opt
        //
        // type-specifiers:
        //   type-specifier
        //   type-specifiers  ,  type-specifier
        let (items, _) =
            self.parse_comma_list_allow_trailing(close_kind, Errors::error1007, |x: &mut Self| {
                x.parse_type_specifier(false, true)
            });
        items
    }

    // SPEC
    //
    // TODO: Add this to the specification.
    // (This work is tracked by task T22582676.)
    //
    // call-convention:
    //   inout

    fn parse_call_convention_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
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
    // (This work is tracked by task T85043839.)
    //
    // readonly:
    //   readonly

    fn parse_readonly_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
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

    fn parse_optional_opt(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::Optional => {
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
    // closure-param-type-specifier-list:
    //   closure-param-type-specifiers  ,opt
    //
    // closure-param-type-specifiers:
    //   closure-param-type-specifier
    //   closure-param-type-specifiers  ,  closure-param-type-specifier

    fn parse_closure_param_list(&mut self, close_kind: TokenKind) -> S::Output {
        let (items, _) =
            self.parse_comma_list_allow_trailing(close_kind, Errors::error1007, |x: &mut Self| {
                x.parse_closure_param_type_or_ellipsis()
            });
        items
    }

    // SPEC
    //
    // TODO: Add this to the specification.
    // (This work is tracked by task T22582676.)
    //
    // ERROR RECOVERY: Variadic params cannot be declared inout; this error is
    // caught in a later pass.
    //
    // closure-param-type-specifier:
    //   call-convention-opt  type-specifier
    //   type-specifier  ...
    //   ...

    fn parse_closure_param_type_or_ellipsis(&mut self) -> S::Output {
        match self.peek_token_kind() {
            TokenKind::DotDotDot => {
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                let token = self.next_token();
                let token = self.sc_mut().make_token(token);
                self.sc_mut()
                    .make_variadic_parameter(missing1, missing2, token)
            }
            _ => {
                let optional = self.parse_optional_opt();
                let callconv = self.parse_call_convention_opt();
                let readonly = self.parse_readonly_opt();
                let ts = self.parse_type_specifier(false, true);
                match self.peek_token_kind() {
                    TokenKind::DotDotDot => {
                        let token = self.next_token();
                        let token = self.sc_mut().make_token(token);
                        self.sc_mut().make_variadic_parameter(callconv, ts, token)
                    }
                    _ => self
                        .sc_mut()
                        .make_closure_parameter_type_specifier(optional, callconv, readonly, ts),
                }
            }
        }
    }

    fn parse_optionally_reified_type(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::Reify {
            let token = self.next_token();
            let reified_kw = self.sc_mut().make_token(token);
            let type_argument = self.parse_type_specifier(false, true);
            self.sc_mut()
                .make_reified_type_argument(reified_kw, type_argument)
        } else {
            self.parse_type_specifier(false, true)
        }
    }

    pub fn parse_generic_type_argument_list(&mut self) -> (S::Output, bool) {
        // SPEC:
        // generic-type-argument-list:
        //   <  generic-type-arguments  ,opt  >
        //
        // generic-type-arguments:
        //   generic-type-argument
        //   generic-type-arguments  ,  generic-type-argument
        //
        // TODO: SPEC ISSUE
        // https://github.com/hhvm/hack-langspec/issues/84
        // The specification indicates that "noreturn" is only syntactically valid
        // as a return type hint, but this is plainly wrong because
        // Awaitable<noreturn> is a legal type. Likely the correct rule will be to
        // allow noreturn as a type argument, and then a later semantic analysis
        // pass can determine when it is being used incorrectly.
        //
        // For now, we extend the specification to allow return types, not just
        // ordinary types.
        let open_angle = self.assert_left_angle_in_type_list_with_possible_attribute();
        let (args, no_arg_is_missing) = self.parse_comma_list_allow_trailing(
            TokenKind::GreaterThan,
            Errors::error1007,
            |x: &mut Self| x.parse_optionally_reified_type(),
        );
        match self.peek_token_kind() {
            TokenKind::GreaterThan => {
                let close_angle = self.assert_token(TokenKind::GreaterThan);
                let result = self
                    .sc_mut()
                    .make_type_arguments(open_angle, args, close_angle);
                (result, no_arg_is_missing)
            }
            _ => {
                // ERROR RECOVERY: Don't eat the token that is in the place of the
                // missing > or ,.  TokenKind::Assume that it is the > that is missing and
                // try to parse whatever is coming after the type.
                self.with_error(Errors::error1014, Vec::new());
                let pos = self.pos();
                let missing = self.sc_mut().make_missing(pos);
                let result = self.sc_mut().make_type_arguments(open_angle, args, missing);
                (result, no_arg_is_missing)
            }
        }
    }

    fn parse_darray_type_specifier(&mut self) -> S::Output {
        // darray<type, type>
        let array_token = self.assert_token(TokenKind::Darray);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            self.sc_mut().make_simple_type_specifier(array_token)
        } else {
            let left_angle = self.assert_left_angle_in_type_list_with_possible_attribute();
            let key_type = self.parse_type_specifier(false, true);
            let comma = self.require_comma();
            let value_type = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right_angle = self.require_right_angle();
            self.sc_mut().make_darray_type_specifier(
                array_token,
                left_angle,
                key_type,
                comma,
                value_type,
                optional_comma,
                right_angle,
            )
        }
    }

    fn parse_varray_type_specifier(&mut self) -> S::Output {
        // varray<type>
        let array_token = self.assert_token(TokenKind::Varray);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            self.sc_mut().make_simple_type_specifier(array_token)
        } else {
            let left_angle = self.assert_left_angle_in_type_list_with_possible_attribute();
            let value_type = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right_angle = self.require_right_angle();
            self.sc_mut().make_varray_type_specifier(
                array_token,
                left_angle,
                value_type,
                optional_comma,
                right_angle,
            )
        }
    }

    fn parse_vec_type_specifier(&mut self) -> S::Output {
        // vec < type-specifier >
        // TODO: Should we allow a trailing comma?
        // TODO: Add this to the specification
        // ERROR RECOVERY: If there is no type argument list then just make
        // this a simple type.  TODO: Should this be an error at parse time? what
        // about at type checking time?
        let keyword = self.assert_token(TokenKind::Vec);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            self.sc_mut().make_simple_type_specifier(keyword)
        } else {
            let left = self.assert_left_angle_in_type_list_with_possible_attribute();
            let t = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right = self.require_right_angle();
            self.sc_mut()
                .make_vector_type_specifier(keyword, left, t, optional_comma, right)
        }
    }

    fn parse_keyset_type_specifier(&mut self) -> S::Output {
        // keyset < type-specifier >
        // TODO: Should we allow a trailing comma?
        // TODO: Add this to the specification
        // ERROR RECOVERY: If there is no type argument list then just make
        // this a simple type.  TODO: Should this be an error at parse time? what
        // about at type checking time?
        let keyword = self.assert_token(TokenKind::Keyset);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            self.sc_mut().make_simple_type_specifier(keyword)
        } else {
            let left = self.assert_left_angle_in_type_list_with_possible_attribute();
            let t = self.parse_type_specifier(false, true);
            let comma = self.optional_token(TokenKind::Comma);
            let right = self.require_right_angle();
            self.sc_mut()
                .make_keyset_type_specifier(keyword, left, t, comma, right)
        }
    }

    fn parse_tuple_type_explicit_specifier(&mut self) -> S::Output {
        // tuple < type-specifier-list >
        // TODO: Add this to the specification
        let keyword = self.assert_token(TokenKind::Tuple);
        let left_angle = if self.peek_next_partial_token_is_triple_left_angle() {
            self.assert_left_angle_in_type_list_with_possible_attribute()
        } else {
            self.require_left_angle()
        };
        let args = self.parse_type_list(TokenKind::GreaterThan);
        let mut parser1 = self.clone();
        let right_angle = parser1.next_token();
        if right_angle.kind() == TokenKind::GreaterThan {
            self.continue_from(parser1);
            let token = self.sc_mut().make_token(right_angle);
            self.sc_mut()
                .make_tuple_type_explicit_specifier(keyword, left_angle, args, token)
        } else {
            // ERROR RECOVERY: Don't eat the token that is in the place of the
            // missing > or ,.  TokenKind::Assume that it is the > that is missing and
            // try to parse whatever is coming after the type.
            self.with_error(Errors::error1022, Vec::new());
            let pos = self.pos();
            let right_angle = self.sc_mut().make_missing(pos);
            self.sc_mut()
                .make_tuple_type_explicit_specifier(keyword, left_angle, args, right_angle)
        }
    }

    fn parse_dictionary_type_specifier(&mut self) -> S::Output {
        // dict < type-specifier , type-specifier >
        //
        // TODO: Add this to the specification
        //
        // Though we require there to be exactly two items, we actually parse
        // an arbitrary comma-separated list here.
        //
        // TODO: Give an error in a later pass if there are not exactly two members.
        //
        // ERROR RECOVERY: If there is no type argument list then just make this
        // a simple type.  TODO: Should this be an error at parse time?  what
        // about at type checking time?
        let keyword = self.assert_token(TokenKind::Dict);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            self.sc_mut().make_simple_type_specifier(keyword)
        } else {
            // TODO: This allows "noreturn" as a type argument. Should we
            // disallow that at parse time?
            let left = self.assert_left_angle_in_type_list_with_possible_attribute();
            let (arguments, _) = self.parse_comma_list_allow_trailing(
                TokenKind::GreaterThan,
                Errors::error1007,
                |x: &mut Self| x.parse_return_type(),
            );
            let right = self.require_right_angle();
            self.sc_mut()
                .make_dictionary_type_specifier(keyword, left, arguments, right)
        }
    }

    fn parse_tuple_or_closure_type_specifier(&mut self) -> S::Output {
        let mut parser1 = self.clone();
        let _ = parser1.assert_token(TokenKind::LeftParen);
        let token = parser1.peek_token();
        match token.kind() {
            TokenKind::Readonly | TokenKind::Function => self.parse_closure_type_specifier(),
            _ => self.parse_tuple_or_union_or_intersection_type_specifier(),
        }
    }

    pub fn parse_contexts(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::LeftBracket {
            let (left_bracket, types, right_bracket) = self
                .parse_bracketted_comma_list_opt_allow_trailing(|x: &mut Self| {
                    match x.peek_token_kind() {
                        TokenKind::Ctx => {
                            let ctx = x.assert_token(TokenKind::Ctx);
                            let var =
                                x.with_expression_parser(|p: &mut ExpressionParser<'a, S>| {
                                    p.parse_simple_variable()
                                });
                            x.sc_mut().make_function_ctx_type_specifier(ctx, var)
                        }
                        TokenKind::Variable => {
                            /* Keeping this isolated from the type constant parsing code for now */
                            let var = x.assert_token(TokenKind::Variable);
                            let colcol = x.require_coloncolon();
                            let name = x.require_name();
                            x.sc_mut().make_type_constant(var, colcol, name)
                        }
                        _ => x.parse_type_specifier(false, false),
                    }
                });
            self.sc_mut()
                .make_contexts(left_bracket, types, right_bracket)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        }
    }

    fn parse_closure_type_specifier(&mut self) -> S::Output {
        // SPEC
        //
        // TODO: Update the specification with closure-param-type-specifier-list-opt.
        // (This work is tracked by task T22582676.)
        //
        // TODO: Update grammar for inout parameters.
        // (This work is tracked by task T22582715.)
        //
        // TODO: Update grammar for readonly keyword
        // (This work is tracked by task T87253111.)
        // closure-type-specifier:
        //   (  readonly-opt
        //   function ( \
        //   closure-param-type-specifier-list-opt \
        //   ) : type-specifier )
        //
        // TODO: Error recovery is pretty weak here. We could be smarter.
        let olp = self.fetch_token();
        let ro = self.parse_readonly_opt();
        let fnc = self.fetch_token();
        let ilp = self.require_left_paren();
        let (pts, irp) = if self.peek_token_kind() == TokenKind::RightParen {
            let pos = self.pos();
            let missing = self.sc_mut().make_missing(pos);
            let token = self.next_token();
            let token = self.sc_mut().make_token(token);
            (missing, token)
        } else {
            // TODO add second pass checking to ensure ellipsis is the last arg
            let pts = self.parse_closure_param_list(TokenKind::RightParen);
            let irp = self.require_right_paren();
            (pts, irp)
        };
        let ctxs = self.parse_contexts();
        let col = self.require_colon();
        let readonly = self.parse_readonly_opt();
        let ret = self.parse_type_specifier(false, true);
        let orp = self.require_right_paren();
        self.sc_mut()
            .make_closure_type_specifier(olp, ro, fnc, ilp, pts, irp, ctxs, col, readonly, ret, orp)
    }

    fn parse_tuple_or_union_or_intersection_type_specifier(&mut self) -> S::Output {
        // SPEC
        // tuple-union-intersection-type-specifier:
        //   ( type-specifier  ,  type-specifier-list  )
        //   ( type-specifier  &  intersection-type-specifier-list )
        //   ( type-specifier  |  union-type-specifier-list )
        // type-specifier-list:
        //   type-specifiers  ,opt
        //   type-specifiers
        //   type-specifier
        //   type-specifiers , type-specifier
        // intersection-type-specifier-list:
        //   type-specifier
        //   intersection-type-specifier-list & type-specifier
        // union-type-specifier-list:
        //   type-specifier
        //   union-type-specifier-list | type-specifier

        // TODO: Here we parse a type list with one or more items, but the grammar
        // actually requires a type list with two or more items. Give an error in
        // a later pass if there is only one item here.

        let left_paren = self.assert_token(TokenKind::LeftParen);

        let (args, _, separator_kind) = self.parse_separated_list_predicate(
            |x| x == TokenKind::Bar || x == TokenKind::Ampersand || x == TokenKind::Comma,
            SeparatedListKind::TrailingAllowed,
            |x| x == TokenKind::RightParen,
            Errors::error1007,
            |x: &mut Self| x.parse_type_specifier(false, true),
        );

        if self.peek_token_kind() == TokenKind::RightParen {
            let right_paren = self.next_token();
            let token = self.sc_mut().make_token(right_paren);
            match separator_kind {
                TokenKind::Bar => self
                    .sc_mut()
                    .make_union_type_specifier(left_paren, args, token),
                TokenKind::Ampersand => self
                    .sc_mut()
                    .make_intersection_type_specifier(left_paren, args, token),
                _ => self
                    .sc_mut()
                    .make_tuple_type_specifier(left_paren, args, token),
            }
        } else {
            // ERROR RECOVERY: Don't eat the token that is in the place of the
            // missing ) or ,.  Assume that it is the ) that is missing and
            // try to parse whatever is coming after the type.
            self.with_error(Errors::error1022, Vec::new());
            let pos = self.pos();
            let missing = self.sc_mut().make_missing(pos);
            self.sc_mut()
                .make_tuple_type_specifier(left_paren, args, missing)
        }
    }

    fn parse_nullable_type_specifier(&mut self) -> S::Output {
        // SPEC:
        // nullable-type-specifier:
        //   ? type-specifier
        //   mixed
        //
        // Note that we parse "mixed" as a simple type specifier, even though
        // technically it is classified as a nullable type specifier by the grammar.
        // Note that it is perfectly legal to have trivia between the ? and the
        // underlying type.
        let question = self.assert_token(TokenKind::Question);
        let nullable_type = self.parse_type_specifier(false, true);
        self.sc_mut()
            .make_nullable_type_specifier(question, nullable_type)
    }

    fn parse_like_type_specifier(&mut self) -> S::Output {
        // SPEC:
        // like-type-specifier:
        //   ~ type-specifier
        //
        // Note that it is perfectly legal to have trivia between the ~ and the
        // underlying type.
        let tilde = self.assert_token(TokenKind::Tilde);
        let like_type = self.parse_type_specifier(false, true);
        self.sc_mut().make_like_type_specifier(tilde, like_type)
    }

    fn parse_soft_type_specifier(&mut self) -> S::Output {
        // SPEC (Draft)
        // soft-type-specifier:
        //   @ type-specifier
        //
        // TODO: The spec does not mention this type grammar.  Work out where and
        // when it is legal, and what the exact semantics are, and put it in the spec.
        // Add an error pass if necessary to identify illegal usages of this type.
        //
        // Note that it is legal for trivia to come between the @ and the type.
        let soft_at = self.assert_token(TokenKind::At);
        let soft_type = self.parse_type_specifier(false, true);
        self.sc_mut().make_soft_type_specifier(soft_at, soft_type)
    }

    fn parse_attributized_specifier(&mut self) -> S::Output {
        // SPEC
        // attributized-specifier:
        // attribute-specification-opt type-specifier
        let attribute_spec_opt = self.with_decl_parser(|x: &mut DeclarationParser<'a, S>| {
            x.parse_attribute_specification_opt()
        });
        let attributized_type = self.parse_type_specifier(false, true);
        self.sc_mut()
            .make_attributized_specifier(attribute_spec_opt, attributized_type)
    }

    fn parse_class_args_type_specifier(&mut self) -> S::Output {
        // SPEC
        // class-args-type-specifier:
        //   class  <  qualified-name generic-type-argument-list-opt >
        let class = self.fetch_token();
        let left_angle = self.require_left_angle();
        let class_type = self.parse_type_specifier(false, true);
        let optional_comma = self.optional_token(TokenKind::Comma);
        let right_angle = self.require_right_angle();
        self.sc_mut().make_class_args_type_specifier(
            class,
            left_angle,
            class_type,
            optional_comma,
            right_angle,
        )
    }

    fn parse_classname_type_specifier(&mut self) -> S::Output {
        // SPEC
        // classname-type-specifier:
        //   classname
        //   classname  <  qualified-name generic-type-argument-list-opt >
        //
        // TODO: We parse any type as the class name type; we should write an
        // error detection pass later that determines when this is a bad type.
        //
        // TODO: Is this grammar correct?  In particular, can the name have a
        // scope resolution operator (::) in it?  Find out and update the spec if
        // this is permitted.

        // TODO ERROR RECOVERY is unsophisticated here.
        let classname = self.fetch_token();
        match self.peek_token_kind() {
            TokenKind::LessThan => {
                let left_angle = self.require_left_angle();
                let classname_type = self.parse_type_specifier(false, true);
                let optional_comma = self.optional_token(TokenKind::Comma);
                let right_angle = self.require_right_angle();
                self.sc_mut().make_classname_type_specifier(
                    classname,
                    left_angle,
                    classname_type,
                    optional_comma,
                    right_angle,
                )
            }
            _ => {
                let pos = self.pos();
                let missing1 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing2 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing3 = self.sc_mut().make_missing(pos);
                let pos = self.pos();
                let missing4 = self.sc_mut().make_missing(pos);
                self.sc_mut().make_classname_type_specifier(
                    classname, missing1, missing2, missing3, missing4,
                )
            }
        }
    }

    fn parse_field_specifier(&mut self) -> S::Output {
        // SPEC
        // field-specifier:
        //   ?-opt present-field-specifier
        // present-field-specifier:
        //   single-quoted-string-literal  =>  type-specifier
        //   qualified-name  =>  type-specifier
        //   scope-resolution-expression  =>  type-specifier

        // TODO: We require that it be either all literals or no literals in the
        // set of specifiers; make an error reporting pass that detects this.

        // ERROR RECOVERY: We allow any expression for the left-hand side.
        // TODO: Make an error-detecting pass that gives an error if the left-hand
        // side is not a literal or name.
        let question = if self.peek_token_kind() == TokenKind::Question {
            self.assert_token(TokenKind::Question)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        };
        let name = self.parse_expression();
        let arrow = self.require_arrow();
        let field_type = self.parse_type_specifier(false, true);
        self.sc_mut()
            .make_field_specifier(question, name, arrow, field_type)
    }

    fn parse_shape_specifier(&mut self) -> S::Output {
        // SPEC
        // shape-specifier:
        //   shape ( field-specifier-list-opt )
        // field-specifier-list:
        //   field-specifiers  ,  ...
        //   field-specifiers  ,-opt
        // field-specifiers:
        //   field-specifier
        //   field-specifiers  ,  field-specifier
        //
        // TODO: ERROR RECOVERY is not very sophisticated here.
        let shape = self.fetch_token();
        let lparen = self.require_left_paren();
        let is_closing_token =
            |x: TokenKind| x == TokenKind::RightParen || x == TokenKind::DotDotDot;
        let fields = self.parse_comma_list_opt_allow_trailing_predicate(
            is_closing_token,
            Errors::error1025,
            |x: &mut Self| x.parse_field_specifier(),
        );
        let ellipsis = if self.peek_token_kind() == TokenKind::DotDotDot {
            self.assert_token(TokenKind::DotDotDot)
        } else {
            let pos = self.pos();
            self.sc_mut().make_missing(pos)
        };
        let rparen = self.require_right_paren();
        self.sc_mut()
            .make_shape_type_specifier(shape, lparen, fields, ellipsis, rparen)
    }

    pub(crate) fn parse_type_constraint_opt(&mut self, allow_super: bool) -> Option<S::Output> {
        // SPEC
        // type-constraint:
        //   as  type-specifier
        //   super  type-specifier
        // TODO: What about = ?
        let make = |x: &mut Self| {
            let constraint = x.next_token();
            let constraint = x.sc_mut().make_token(constraint);
            let constraint_type = x.parse_type_specifier(false, true);
            Some(x.sc_mut().make_type_constraint(constraint, constraint_type))
        };
        let token = self.peek_token_kind();
        match token {
            TokenKind::As => make(self),
            TokenKind::Super if allow_super => make(self),
            _ => None,
        }
    }

    pub fn parse_context_constraint_opt(&mut self) -> Option<S::Output> {
        // SPEC
        // context-constraint:
        //   as  context-list
        //   super  context-list
        match self.peek_token_kind() {
            TokenKind::As | TokenKind::Super => {
                let constraint_token = self.next_token();
                let constraint_token = self.sc_mut().make_token(constraint_token);
                let constraint_ctx = self.parse_contexts();
                Some(
                    self.sc_mut()
                        .make_context_constraint(constraint_token, constraint_ctx),
                )
            }
            _ => None,
        }
    }

    pub fn parse_return_type(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::Noreturn {
            let token = self.next_token();
            self.sc_mut().make_token(token)
        } else {
            self.parse_type_specifier(false, true)
        }
    }

    // Same as parse_return_type but can return missing
    pub fn parse_return_type_opt(&mut self) -> S::Output {
        if self.peek_token_kind() == TokenKind::Noreturn {
            let token = self.next_token();
            self.sc_mut().make_token(token)
        } else {
            self.parse_type_specifier_opt(false, true)
        }
    }
}
