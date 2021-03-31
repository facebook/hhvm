// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::declaration_parser::DeclarationParser;
use crate::expression_parser::ExpressionParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::Context;
use crate::parser_trait::ParserTrait;
use crate::parser_trait::SeparatedListKind;
use crate::smart_constructors::{NodeType, SmartConstructors, Token};
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::syntax_error::{self as Errors, SyntaxError};
use parser_core_types::token_kind::TokenKind;

pub struct TypeParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    lexer: Lexer<'a, S::TF>,
    env: ParserEnv,
    context: Context<'a, Token<S>>,
    errors: Vec<SyntaxError>,
    sc: S,
}

impl<'a, S> std::clone::Clone for TypeParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn clone(&self) -> Self {
        Self {
            lexer: self.lexer.clone(),
            env: self.env.clone(),
            context: self.context.clone(),
            errors: self.errors.clone(),
            sc: self.sc.clone(),
        }
    }
}

impl<'a, S> ParserTrait<'a, S> for TypeParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn make(
        mut lexer: Lexer<'a, S::TF>,
        env: ParserEnv,
        context: Context<'a, Token<S>>,
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

    fn into_parts(mut self) -> (Lexer<'a, S::TF>, Context<'a, Token<S>>, Vec<SyntaxError>, S) {
        self.lexer.set_in_type(false);
        (self.lexer, self.context, self.errors, self.sc)
    }

    fn lexer(&self) -> &Lexer<'a, S::TF> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::TF> {
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

impl<'a, S> TypeParser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    fn with_expression_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut ExpressionParser<'a, S>) -> U,
    {
        let mut lexer = self.lexer.clone();
        lexer.set_in_type(false);
        let mut expression_parser: ExpressionParser<S> = ExpressionParser::make(
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

    fn parse_expression(&mut self) -> S::R {
        self.with_expression_parser(|p: &mut ExpressionParser<'a, S>| p.parse_expression())
    }

    fn with_decl_parser<F, U>(&mut self, f: F) -> U
    where
        F: Fn(&mut DeclarationParser<'a, S>) -> U,
    {
        let mut lexer = self.lexer.clone();
        lexer.set_in_type(false);

        let mut declaration_parser: DeclarationParser<S> = DeclarationParser::make(
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

    // TODO: What about something like for::for? Is that a legal type constant?
    pub fn parse_type_specifier(&mut self, allow_var: bool, allow_attr: bool) -> S::R {
        // Strictly speaking, "mixed" is a nullable type specifier. We parse it as
        // a simple type specifier here.
        let mut parser1 = self.clone();
        let token = parser1.next_xhp_class_name_or_other_token();
        let new_attr_syntax = self.env.allow_new_attribute_syntax;
        match token.kind() {
            | TokenKind::Var if allow_var => {
                self.continue_from(parser1);
                let token = S!(make_token, self, token);
                S!(make_simple_type_specifier, self, token)
            }
            | TokenKind::This => self.parse_simple_type_or_type_constant(),
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
            | TokenKind::Object
            | TokenKind::Mixed
            | TokenKind::NullLiteral
            | TokenKind::Name => self.parse_simple_type_or_type_constant_or_generic(),
            | TokenKind::Namespace => {
                let name = self.scan_name_or_qualified_name();
                self.parse_remaining_simple_type_or_type_constant_or_generic(name)
            }
            | TokenKind::Backslash => {
                self.continue_from(parser1);
                let missing = S!(make_missing, self, self.pos());
                let token = S!(make_token, self, token);
                let name = self.scan_qualified_name(missing, token);
                self.parse_remaining_simple_type_or_type_constant_or_generic(name)
            }
            | TokenKind::SelfToken
            | TokenKind::Parent => self.parse_simple_type_or_type_constant(),
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
            | TokenKind::At if !new_attr_syntax => self.parse_soft_type_specifier(),
            | TokenKind::At if new_attr_syntax => self.parse_attributized_specifier(),
            | TokenKind::LessThanLessThan if allow_attr => self.parse_attributized_specifier(),
            | TokenKind::Classname => self.parse_classname_type_specifier(),
            | _ => {
                self.with_error_on_whole_token(Errors::error1007);
                let token = self.next_xhp_class_name_or_other_token();
                let token = S!(make_token, self, token);
                S!(make_error, self, token)
            }
        }
    }

    // SPEC
    // type-constant-type-name:
    //   name  ::  name
    //   self  ::  name
    //   this  ::  name
    //   parent  ::  name
    //   type-constant-type-name  ::  name
    fn parse_remaining_type_constant(&mut self, left: S::R) -> S::R {
        let separator = self.fetch_token();
        let right = self.next_token_as_name();
        if right.kind() == TokenKind::Name {
            let right = S!(make_token, self, right);
            let syntax = S!(make_type_constant, self, left, separator, right);
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
            self.with_error(Errors::error1004);
            let missing = S!(make_missing, self, self.pos());
            S!(make_type_constant, self, left, separator, missing)
        }
    }

    fn parse_remaining_generic(&mut self, name: S::R) -> S::R {
        let (arguments, _) = self.parse_generic_type_argument_list();
        S!(make_generic_type_specifier, self, name, arguments)
    }

    pub fn parse_simple_type_or_type_constant(&mut self) -> S::R {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_type_constant(name)
    }

    pub fn parse_simple_type_or_generic(&mut self) -> S::R {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_generic(name)
    }

    fn parse_remaining_simple_type_or_type_constant(&mut self, name: S::R) -> S::R {
        let token = self.peek_token();
        match token.kind() {
            TokenKind::ColonColon => self.parse_remaining_type_constant(name),
            _ => S!(make_simple_type_specifier, self, name),
        }
    }

    fn parse_simple_type_or_type_constant_or_generic(&mut self) -> S::R {
        let name = self.next_xhp_class_name_or_other();
        self.parse_remaining_simple_type_or_type_constant_or_generic(name)
    }

    pub fn parse_remaining_type_specifier(&mut self, name: S::R) -> S::R {
        self.parse_remaining_simple_type_or_type_constant_or_generic(name)
    }

    fn parse_remaining_simple_type_or_type_constant_or_generic(&mut self, name: S::R) -> S::R {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_remaining_generic(name),
            _ => self.parse_remaining_simple_type_or_type_constant(name),
        }
    }

    fn parse_remaining_simple_type_or_generic(&mut self, name: S::R) -> S::R {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_remaining_generic(name),
            _ => S!(make_simple_type_specifier, self, name),
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
    fn parse_generic_type_constraint_opt(&mut self) -> Option<S::R> {
        let mut parser1 = self.clone();
        let token = parser1.next_token();
        match token.kind() {
            TokenKind::As | TokenKind::Super => {
                self.continue_from(parser1);
                let constraint_token = S!(make_token, self, token);
                let matched_type = self.parse_type_specifier(false, true);
                let type_constraint =
                    S!(make_type_constraint, self, constraint_token, matched_type);
                Some(type_constraint)
            }
            _ => None,
        }
    }

    fn parse_variance_opt(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::Plus | TokenKind::Minus => self.fetch_token(),
            _ => S!(make_missing, self, self.pos()),
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
    pub fn parse_type_parameter(&mut self) -> S::R {
        let attributes = self.with_decl_parser(|x: &mut DeclarationParser<'a, S>| {
            x.parse_attribute_specification_opt()
        });
        let reified = self.optional_token(TokenKind::Reify);
        let variance = self.parse_variance_opt();
        let type_name = self.require_name_allow_all_keywords();
        let param_params = self.parse_generic_type_parameter_list_opt();
        let constraints =
            self.parse_list_until_none(|x: &mut Self| x.parse_generic_type_constraint_opt());
        S!(
            make_type_parameter,
            self,
            attributes,
            reified,
            variance,
            type_name,
            param_params,
            constraints
        )
    }

    pub fn parse_generic_type_parameter_list_opt(&mut self) -> S::R {
        match self.peek_token_kind_with_possible_attributized_type_list() {
            TokenKind::LessThan => self.parse_generic_type_parameter_list(),
            _ => S!(make_missing, self, self.pos()),
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

    pub fn parse_generic_type_parameter_list(&mut self) -> S::R {
        let left = self.assert_left_angle_in_type_list_with_possible_attribute();
        let (params, _) = self.parse_comma_list_allow_trailing(
            TokenKind::GreaterThan,
            Errors::error1007,
            |x: &mut Self| x.parse_type_parameter(),
        );

        let right = self.require_right_angle();
        S!(make_type_parameters, self, left, params, right)
    }

    fn parse_type_list(&mut self, close_kind: TokenKind) -> S::R {
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

    fn parse_call_convention_opt(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::Inout => {
                let token = self.next_token();
                S!(make_token, self, token)
            }
            _ => S!(make_missing, self, self.pos()),
        }
    }

    // SPEC
    //
    // TODO: Add this to the specification.
    // (This work is tracked by task T85043839.)
    //
    // readonly:
    //   readonly

    fn parse_readonly_opt(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::Readonly => {
                let token = self.next_token();
                S!(make_token, self, token)
            }
            _ => S!(make_missing, self, self.pos()),
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

    fn parse_closure_param_list(&mut self, close_kind: TokenKind) -> S::R {
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

    fn parse_closure_param_type_or_ellipsis(&mut self) -> S::R {
        match self.peek_token_kind() {
            TokenKind::DotDotDot => {
                let missing1 = S!(make_missing, self, self.pos());
                let missing2 = S!(make_missing, self, self.pos());
                let token = self.next_token();
                let token = S!(make_token, self, token);
                S!(make_variadic_parameter, self, missing1, missing2, token)
            }
            _ => {
                let callconv = self.parse_call_convention_opt();
                let readonly = self.parse_readonly_opt();
                let ts = self.parse_type_specifier(false, true);
                match self.peek_token_kind() {
                    TokenKind::DotDotDot => {
                        let token = self.next_token();
                        let token = S!(make_token, self, token);
                        S!(make_variadic_parameter, self, callconv, ts, token)
                    }
                    _ => S!(
                        make_closure_parameter_type_specifier,
                        self,
                        callconv,
                        readonly,
                        ts
                    ),
                }
            }
        }
    }

    fn parse_optionally_reified_type(&mut self) -> S::R {
        if self.peek_token_kind() == TokenKind::Reify {
            let token = self.next_token();
            let reified_kw = S!(make_token, self, token);
            let type_argument = self.parse_type_specifier(false, true);
            S!(make_reified_type_argument, self, reified_kw, type_argument)
        } else {
            self.parse_type_specifier(false, true)
        }
    }

    pub fn parse_generic_type_argument_list(&mut self) -> (S::R, bool) {
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
                let result = S!(make_type_arguments, self, open_angle, args, close_angle);
                (result, no_arg_is_missing)
            }
            _ => {
                // ERROR RECOVERY: Don't eat the token that is in the place of the
                // missing > or ,.  TokenKind::Assume that it is the > that is missing and
                // try to parse whatever is coming after the type.
                self.with_error(Errors::error1014);
                let missing = S!(make_missing, self, self.pos());
                let result = S!(make_type_arguments, self, open_angle, args, missing);
                (result, no_arg_is_missing)
            }
        }
    }

    fn parse_darray_type_specifier(&mut self) -> S::R {
        // darray<type, type>
        let array_token = self.assert_token(TokenKind::Darray);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            S!(make_simple_type_specifier, self, array_token)
        } else {
            let left_angle = self.assert_left_angle_in_type_list_with_possible_attribute();
            let key_type = self.parse_type_specifier(false, true);
            let comma = self.require_comma();
            let value_type = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right_angle = self.require_right_angle();
            S!(
                make_darray_type_specifier,
                self,
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

    fn parse_varray_type_specifier(&mut self) -> S::R {
        // varray<type>
        let array_token = self.assert_token(TokenKind::Varray);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            S!(make_simple_type_specifier, self, array_token)
        } else {
            let left_angle = self.assert_left_angle_in_type_list_with_possible_attribute();
            let value_type = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right_angle = self.require_right_angle();
            S!(
                make_varray_type_specifier,
                self,
                array_token,
                left_angle,
                value_type,
                optional_comma,
                right_angle,
            )
        }
    }

    fn parse_vec_type_specifier(&mut self) -> S::R {
        // vec < type-specifier >
        // TODO: Should we allow a trailing comma?
        // TODO: Add this to the specification
        // ERROR RECOVERY: If there is no type argument list then just make
        // this a simple type.  TODO: Should this be an error at parse time? what
        // about at type checking time?
        let keyword = self.assert_token(TokenKind::Vec);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            S!(make_simple_type_specifier, self, keyword)
        } else {
            let left = self.assert_left_angle_in_type_list_with_possible_attribute();
            let t = self.parse_type_specifier(false, true);
            let optional_comma = self.optional_token(TokenKind::Comma);
            let right = self.require_right_angle();
            S!(
                make_vector_type_specifier,
                self,
                keyword,
                left,
                t,
                optional_comma,
                right
            )
        }
    }

    fn parse_keyset_type_specifier(&mut self) -> S::R {
        // keyset < type-specifier >
        // TODO: Should we allow a trailing comma?
        // TODO: Add this to the specification
        // ERROR RECOVERY: If there is no type argument list then just make
        // this a simple type.  TODO: Should this be an error at parse time? what
        // about at type checking time?
        let keyword = self.assert_token(TokenKind::Keyset);
        if self.peek_token_kind_with_possible_attributized_type_list() != TokenKind::LessThan {
            S!(make_simple_type_specifier, self, keyword)
        } else {
            let left = self.assert_left_angle_in_type_list_with_possible_attribute();
            let t = self.parse_type_specifier(false, true);
            let comma = self.optional_token(TokenKind::Comma);
            let right = self.require_right_angle();
            S!(
                make_keyset_type_specifier,
                self,
                keyword,
                left,
                t,
                comma,
                right
            )
        }
    }

    fn parse_tuple_type_explicit_specifier(&mut self) -> S::R {
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
            let token = S!(make_token, self, right_angle);
            S!(
                make_tuple_type_explicit_specifier,
                self,
                keyword,
                left_angle,
                args,
                token
            )
        } else {
            // ERROR RECOVERY: Don't eat the token that is in the place of the
            // missing > or ,.  TokenKind::Assume that it is the > that is missing and
            // try to parse whatever is coming after the type.
            self.with_error(Errors::error1022);
            let right_angle = S!(make_missing, self, self.pos());
            S!(
                make_tuple_type_explicit_specifier,
                self,
                keyword,
                left_angle,
                args,
                right_angle
            )
        }
    }

    fn parse_dictionary_type_specifier(&mut self) -> S::R {
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
            S!(make_simple_type_specifier, self, keyword)
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
            S!(
                make_dictionary_type_specifier,
                self,
                keyword,
                left,
                arguments,
                right
            )
        }
    }

    fn parse_tuple_or_closure_type_specifier(&mut self) -> S::R {
        let mut parser1 = self.clone();
        let _ = parser1.assert_token(TokenKind::LeftParen);
        let token = parser1.peek_token();
        match token.kind() {
            TokenKind::Readonly | TokenKind::Function => self.parse_closure_type_specifier(),
            _ => self.parse_tuple_or_union_or_intersection_type_specifier(),
        }
    }

    pub fn parse_contexts(&mut self) -> S::R {
        if self.peek_token_kind() == TokenKind::LeftBracket {
            let (left_bracket, types, right_bracket) = self
                .parse_bracketted_comma_list_opt_allow_trailing(|x: &mut Self| {
                    match x.peek_token_kind() {
                        TokenKind::Ctx => {
                            let ctx = x.assert_token(TokenKind::Ctx);
                            let var = x.with_expression_parser(|p: &mut ExpressionParser<'a, S>| {
                                p.parse_simple_variable()
                            });
                            S!(make_function_ctx_type_specifier, x, ctx, var)
                        }
                        TokenKind::Variable => {
                            /* Keeping this isolated from the type constant parsing code for now */
                            let var = x.assert_token(TokenKind::Variable);
                            let colcol = x.require_coloncolon();
                            let name = x.require_name();
                            S!(make_type_constant, x, var, colcol, name)
                        }
                        _ => x.parse_type_specifier(false, false),
                    }
                });
            S!(make_contexts, self, left_bracket, types, right_bracket)
        } else {
            S!(make_missing, self, self.pos())
        }
    }

    fn parse_closure_type_specifier(&mut self) -> S::R {
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
            let missing = S!(make_missing, self, self.pos());
            let token = self.next_token();
            let token = S!(make_token, self, token);
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
        S!(
            make_closure_type_specifier,
            self,
            olp,
            ro,
            fnc,
            ilp,
            pts,
            irp,
            ctxs,
            col,
            readonly,
            ret,
            orp
        )
    }

    fn parse_tuple_or_union_or_intersection_type_specifier(&mut self) -> S::R {
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
            let token = S!(make_token, self, right_paren);
            match separator_kind {
                TokenKind::Bar => S!(make_union_type_specifier, self, left_paren, args, token),
                TokenKind::Ampersand => S!(
                    make_intersection_type_specifier,
                    self,
                    left_paren,
                    args,
                    token
                ),
                _ => S!(make_tuple_type_specifier, self, left_paren, args, token),
            }
        } else {
            // ERROR RECOVERY: Don't eat the token that is in the place of the
            // missing ) or ,.  Assume that it is the ) that is missing and
            // try to parse whatever is coming after the type.
            self.with_error(Errors::error1022);
            let missing = S!(make_missing, self, self.pos());
            S!(make_tuple_type_specifier, self, left_paren, args, missing)
        }
    }

    fn parse_nullable_type_specifier(&mut self) -> S::R {
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
        S!(make_nullable_type_specifier, self, question, nullable_type)
    }

    fn parse_like_type_specifier(&mut self) -> S::R {
        // SPEC:
        // like-type-specifier:
        //   ~ type-specifier
        //
        // Note that it is perfectly legal to have trivia between the ~ and the
        // underlying type.
        let tilde = self.assert_token(TokenKind::Tilde);
        let like_type = self.parse_type_specifier(false, true);
        S!(make_like_type_specifier, self, tilde, like_type)
    }

    fn parse_soft_type_specifier(&mut self) -> S::R {
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
        S!(make_soft_type_specifier, self, soft_at, soft_type)
    }

    fn parse_attributized_specifier(&mut self) -> S::R {
        // SPEC
        // attributized-specifier:
        // attribute-specification-opt type-specifier
        let attribute_spec_opt = self.with_decl_parser(|x: &mut DeclarationParser<'a, S>| {
            x.parse_attribute_specification_opt()
        });
        let attributized_type = self.parse_type_specifier(false, true);
        S!(
            make_attributized_specifier,
            self,
            attribute_spec_opt,
            attributized_type
        )
    }

    fn parse_classname_type_specifier(&mut self) -> S::R {
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
                S!(
                    make_classname_type_specifier,
                    self,
                    classname,
                    left_angle,
                    classname_type,
                    optional_comma,
                    right_angle,
                )
            }
            _ => {
                let missing1 = S!(make_missing, self, self.pos());
                let missing2 = S!(make_missing, self, self.pos());
                let missing3 = S!(make_missing, self, self.pos());
                let missing4 = S!(make_missing, self, self.pos());
                S!(
                    make_classname_type_specifier,
                    self,
                    classname,
                    missing1,
                    missing2,
                    missing3,
                    missing4
                )
            }
        }
    }

    fn parse_field_specifier(&mut self) -> S::R {
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
            S!(make_missing, self, self.pos())
        };
        let name = self.parse_expression();
        let arrow = self.require_arrow();
        let field_type = self.parse_type_specifier(false, true);
        S!(
            make_field_specifier,
            self,
            question,
            name,
            arrow,
            field_type
        )
    }

    fn parse_shape_specifier(&mut self) -> S::R {
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
            &is_closing_token,
            Errors::error1025,
            |x: &mut Self| x.parse_field_specifier(),
        );
        let ellipsis = if self.peek_token_kind() == TokenKind::DotDotDot {
            self.assert_token(TokenKind::DotDotDot)
        } else {
            S!(make_missing, self, self.pos())
        };
        let rparen = self.require_right_paren();
        S!(
            make_shape_type_specifier,
            self,
            shape,
            lparen,
            fields,
            ellipsis,
            rparen
        )
    }

    pub fn parse_type_constraint_opt(&mut self) -> S::R {
        // SPEC
        // type-constraint:
        //   as  type-specifier
        // TODO: Is this correct? Or do we need to allow "super" as well?
        // TODO: What about = ?
        if self.peek_token_kind() == TokenKind::As {
            let constraint_as = self.next_token();
            let constraint_as = S!(make_token, self, constraint_as);
            let constraint_type = self.parse_type_specifier(false, true);
            S!(make_type_constraint, self, constraint_as, constraint_type)
        } else {
            S!(make_missing, self, self.pos())
        }
    }

    pub fn parse_context_constraint_opt(&mut self) -> Option<S::R> {
        // SPEC
        // context-constraint:
        //   as  context-list
        //   super  context-list
        match self.peek_token_kind() {
            TokenKind::As | TokenKind::Super => {
                let constraint_token = self.next_token();
                let constraint_token = S!(make_token, self, constraint_token);
                let constraint_ctx = self.parse_contexts();
                Some(S!(
                    make_context_constraint,
                    self,
                    constraint_token,
                    constraint_ctx
                ))
            }
            _ => None,
        }
    }

    pub fn parse_return_type(&mut self) -> S::R {
        if self.peek_token_kind() == TokenKind::Noreturn {
            let token = self.next_token();
            S!(make_token, self, token)
        } else {
            self.parse_type_specifier(false, true)
        }
    }
}
