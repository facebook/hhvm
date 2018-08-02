(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct

module Token = Syntax.Token
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module SimpleParserSyntax = Full_fidelity_simple_parser.WithSyntax(Syntax)
module SimpleParser = SimpleParserSyntax.WithLexer(
  Full_fidelity_type_lexer.WithToken(Syntax.Token))
module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S

module type ExpressionParser_S = Full_fidelity_expression_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .ExpressionParser_S

module type TypeParser_S = Full_fidelity_type_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_type_lexer.WithToken(Syntax.Token))
  .TypeParser_S

module ParserHelperSyntax = Full_fidelity_parser_helpers.WithSyntax(Syntax)
module ParserHelper = ParserHelperSyntax
  .WithLexer(Full_fidelity_type_lexer.WithToken(Syntax.Token))

open TokenKind
open Syntax

module WithSmartConstructors (SCI : SCWithKind_S with module Token = Syntax.Token)
= struct

module WithExpressionParser
  (ExpressionParser : ExpressionParser_S with module SC = SCI) :
  (TypeParser_S with module SC = SCI) = struct

module Parser = SimpleParser.WithSmartConstructors (SCI)
include Parser
include ParserHelper.WithParser(Parser)

  let parse_expression parser =
    let expr_parser =
      ExpressionParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state in
    let (expr_parser, expr) = ExpressionParser.parse_expression expr_parser in
    let env = ExpressionParser.env expr_parser in
    let lexer = ExpressionParser.lexer expr_parser in
    let errors = ExpressionParser.errors expr_parser in
    let context = ExpressionParser.context expr_parser in
    let sc_state = ExpressionParser.sc_state expr_parser in
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, expr)

(* TODO: What about something like for::for? Is that a legal
  type constant?  *)

let rec parse_type_specifier ?(allow_var=false) parser =
  (* Strictly speaking, "mixed" is a nullable type specifier. We parse it as
     a simple type specifier here. *)
  let (parser1, token) = next_xhp_class_name_or_other_token parser in
  match Token.kind token with
  | Var when allow_var ->
    let (parser, token) = Make.token parser1 token in
    Make.simple_type_specifier parser token
  | This -> parse_simple_type_or_type_constant parser
  (* Any keyword-type could be a non-keyword type, because PHP, so check whether
   * these have generics.
   *)
  | Double (* TODO: Specification does not mention double; fix it. *)
  | Bool
  | Boolean
  | Binary
  | Int
  | Integer
  | Float
  | Real
  | Num
  | String
  | Arraykey
  | Void
  | Noreturn
  | Resource
  | Object
  | Mixed
  | Name -> parse_simple_type_or_type_constant_or_generic parser
  | Namespace ->
    let (parser, name) = scan_name_or_qualified_name parser in
    parse_remaining_simple_type_or_type_constant_or_generic parser name
  | Backslash ->
    let (parser, missing) = Make.missing parser1 (pos parser1) in
    let (parser, token) = Make.token parser token in
    let (parser, name) = scan_qualified_name parser missing token in
    parse_remaining_simple_type_or_type_constant_or_generic parser name
  | Self
  | Parent -> parse_simple_type_or_type_constant parser
  | Category
  | XHPClassName -> parse_possible_generic_specifier_or_type_const parser
  | Array -> parse_array_type_specifier parser
  | Darray -> parse_darray_type_specifier parser
  | Varray -> parse_varray_type_specifier parser
  | Vec -> parse_vec_type_specifier parser
  | Dict -> parse_dictionary_type_specifier parser
  | Keyset -> parse_keyset_type_specifier parser
  | Tuple -> parse_tuple_type_explicit_specifier parser
  | LeftParen -> parse_tuple_or_closure_type_specifier parser
  | Shape -> parse_shape_specifier parser
  | Question -> parse_nullable_type_specifier parser
  | At -> parse_soft_type_specifier parser
  | Classname -> parse_classname_type_specifier parser
  | _ ->
    let parser = with_error parser ~on_whole_token:true SyntaxError.error1007 in
    let (parser1, token) = next_xhp_class_name_or_other_token parser in
    let (parser, token) = Make.token parser1 token in
    Make.error parser token

(* SPEC
  type-constant-type-name:
    name  ::  name
    self  ::  name
    this  ::  name
    parent  ::  name
    type-constant-type-name  ::  name
*)

and parse_remaining_type_constant parser left =
  let (parser, separator) = fetch_token parser in
  let (parser1, right) = next_token_as_name parser in
  if (Token.kind right) = Name then
    begin
      let (parser, right) = Make.token parser1 right in
      let (parser, syntax) = Make.type_constant parser left separator right in
      let token = peek_token parser in
      if (Token.kind token) = ColonColon then
        parse_remaining_type_constant parser syntax
      else
        (parser, syntax)
    end
  else
    (* ERROR RECOVERY: Assume that the thing following the ::
       that is not a name belongs to the next thing to be
       parsed; treat the name as missing. *)
    let parser = with_error parser1 SyntaxError.error1004 in
    let (parser, missing) = Make.missing parser (pos parser) in
    Make.type_constant parser left separator missing

and parse_simple_type_or_type_constant parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  parse_remaining_simple_type_or_type_constant parser name

and parse_remaining_simple_type_or_type_constant parser name =
  let token = peek_token parser in
  match Token.kind token with
  | ColonColon -> parse_remaining_type_constant parser name
  | _ -> Make.simple_type_specifier parser name

and parse_simple_type_or_type_constant_or_generic parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  parse_remaining_simple_type_or_type_constant_or_generic parser name

and parse_remaining_type_specifier name parser =
  parse_remaining_simple_type_or_type_constant_or_generic parser name

and parse_remaining_simple_type_or_type_constant_or_generic parser name =
  match peek_token_kind parser with
  | LessThan -> parse_remaining_possible_generic_specifier_or_type_const parser name
  | _ -> parse_remaining_simple_type_or_type_constant parser name

and parse_possible_generic_specifier_or_type_const parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  parse_remaining_possible_generic_specifier_or_type_const parser name

and parse_remaining_possible_generic_specifier_or_type_const parser name =
  match Token.kind (peek_token parser) with
  | LessThan ->
    let (parser, arguments, _) = parse_generic_type_argument_list parser in
    Make.generic_type_specifier parser name arguments
  | ColonColon -> parse_remaining_type_constant parser name
  | _ -> Make.simple_type_specifier parser name

(* SPEC
  class-interface-trait-specifier:
    qualified-name generic-type-argument-listopt
*)
and parse_possible_generic_specifier parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  match Token.kind (peek_token parser) with
  | LessThan ->
    let (parser, arguments, _) = parse_generic_type_argument_list parser in
    Make.generic_type_specifier parser name arguments
  | _ -> Make.simple_type_specifier parser name

(* SPEC
    generic-type-constraint-list:
      generic-type-constraint
      generic-type-constraint generic-type-constraint-list

    generic-type-constraint:
      as type-specifier
      super type-specifier

    TODO: SPEC ISSUES:
    https://github.com/hhvm/hack-langspec/issues/83

    TODO: Do we also need to allow "= type-specifier" here?
*)

and parse_generic_type_constraint_opt parser =
  let (parser1, token) = next_token parser in
  match (Token.kind token) with
  | As
  | Super ->
    let (parser, constraint_token) = Make.token parser1 token in
    let (parser, matched_type) = parse_type_specifier parser in
    let (parser, type_constraint) =
      Make.type_constraint parser constraint_token matched_type
    in
    (parser, Some type_constraint)
  | _ -> (parser, None)

and parse_variance_opt parser =
  match peek_token_kind parser with
  | Plus | Minus -> fetch_token parser
  | _ -> Make.missing parser (pos parser)

(* SPEC
  generic-type-parameter:
    generic-type-parameter-reified-opt  generic-type-parameter-variance-opt
      name  generic-type-constraint-list-opt

  generic-type-parameter-variance:
    +
    -

  TODO: SPEC ISSUE: We allow any number of type constraints, not just zero
  or one as indicated in the spec.
  https://github.com/hhvm/hack-langspec/issues/83
  TODO: Update the spec with reified
*)
and parse_type_parameter parser =
  let (parser, reified) = optional_token parser Reified in
  let (parser, variance) = parse_variance_opt parser in
  let (parser, type_name) = require_name_allow_keywords parser in
  let (parser, constraints) =
    parse_list_until_none parser parse_generic_type_constraint_opt
  in
  Make.type_parameter parser reified variance type_name constraints

(* SPEC
  type-parameter-list:
  < generic-type-parameters  ,-opt >

  generic-type-parameters:
    generic-type-parameter
    generic-type-parameter  ,  generic-type-parameter
*)
and parse_generic_type_parameter_list parser =
  let (parser, left) = assert_token parser LessThan in
  let (parser, params, _) =
    parse_comma_list_allow_trailing
      parser
      GreaterThan
      SyntaxError.error1007
      parse_type_parameter
  in
  let (parser, right) = require_right_angle parser in
  Make.type_parameters parser left params right

and parse_generic_parameter_list_opt parser =
  match peek_token_kind parser with
  | LessThan -> parse_generic_type_parameter_list parser
  | _ -> Make.missing parser (pos parser)

and parse_type_list parser close_kind =
  (* SPEC:
    type-specifier-list:
      type-specifiers  ,opt

    type-specifiers:
      type-specifier
      type-specifiers  ,  type-specifier
  *)
  let (parser, items, _) =
    parse_comma_list_allow_trailing
      parser
      close_kind
      SyntaxError.error1007
      parse_type_specifier
  in
  (parser, items)

(* SPEC

  TODO: Add this to the specification.
  (This work is tracked by task T22582676.)

  call-convention:
    inout
*)
and parse_call_convention_opt parser =
  let (parser1, token) = next_token parser in
  match Token.kind token with
  | Inout -> Make.token parser1 token
  | _ -> Make.missing parser (pos parser)

(* SPEC

  TODO: Add this to the specification.
  (This work is tracked by task T22582676.)

  closure-param-type-specifier-list:
    closure-param-type-specifiers  ,opt

  closure-param-type-specifiers:
    closure-param-type-specifier
    closure-param-type-specifiers  ,  closure-param-type-specifier
*)
and parse_closure_param_list parser close_kind =
  let (parser, items, _) =
    parse_comma_list_allow_trailing
      parser
      close_kind
      SyntaxError.error1007
      parse_closure_param_type_or_ellipsis
  in
  (parser, items)

(* SPEC

  TODO: Add this to the specification.
  (This work is tracked by task T22582676.)

  ERROR RECOVERY: Variadic params cannot be declared inout; this error is
  caught in a later pass.

  closure-param-type-specifier:
    call-convention-opt  type-specifier
    type-specifier  ...
    ...
*)
and parse_closure_param_type_or_ellipsis parser =
  let (parser1, token) = next_token parser in
  match Token.kind token with
  | DotDotDot ->
    let pos = pos parser in
    let (parser, missing1) = Make.missing parser1 pos in
    let (parser, missing2) = Make.missing parser pos in
    let (parser, token) = Make.token parser token in
    Make.variadic_parameter parser missing1 missing2 token
  | _ ->
    let (parser, callconv) = parse_call_convention_opt parser in
    let (parser, ts) = parse_type_specifier parser in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | DotDotDot ->
      let (parser, token) = Make.token parser1 token in
      Make.variadic_parameter parser callconv ts token
    | _ -> Make.closure_parameter_type_specifier parser callconv ts

and parse_optionally_reified_type parser =
  let (parser1, token) = next_token parser in
  if (Token.kind token) = Reified then
    let (parser, reified_kw) = Make.token parser1 token in
    let (parser, type_argument) = parse_type_specifier parser in
    Make.reified_type_argument parser reified_kw type_argument
  else
    parse_type_specifier parser

and parse_generic_type_argument_list parser =
  (* SPEC:
    generic-type-argument-list:
      <  generic-type-arguments  ,opt  >

    generic-type-arguments:
      generic-type-argument
      generic-type-arguments  ,  generic-type-argument
  *)
  (* TODO: SPEC ISSUE
    https://github.com/hhvm/hack-langspec/issues/84
    The specification indicates that "noreturn" is only syntactically valid
    as a return type hint, but this is plainly wrong because
    Awaitable<noreturn> is a legal type. Likely the correct rule will be to
    allow noreturn as a type argument, and then a later semantic analysis
    pass can determine when it is being used incorrectly.

    For now, we extend the specification to allow return types, not just
    ordinary types.
  *)
  let (parser, open_angle) = fetch_token parser in
  let (parser, args, no_arg_is_missing) =
    parse_comma_list_allow_trailing
      parser
      GreaterThan
      SyntaxError.error1007
      parse_optionally_reified_type
  in
  let (parser1, close_angle) = next_token parser in
  if (Token.kind close_angle) = GreaterThan then
    let (parser, close_angle) = Make.token parser1 close_angle in
    let (parser, result) =
      Make.type_arguments parser open_angle args close_angle
    in
    (parser, result, no_arg_is_missing)
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing > or ,.  Assume that it is the > that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1014 in
    let (parser, missing) = Make.missing parser (pos parser) in
    let (parser, result) =
      Make.type_arguments parser open_angle args missing
    in
    (parser, result, no_arg_is_missing)

and parse_array_type_specifier parser =
  (* We allow
     array
     array<type>
     array<type, type>
     TODO: Put a proper reference to the specification in here.
     TODO: in HHVM trailing comma is permitted only in the case with one
     type argument: array<type, >
     so now it is not really comma-separated list
  *)
  let (parser, array_token) = assert_token parser Array in
  if peek_token_kind parser <> LessThan then
    Make.simple_type_specifier parser array_token
  else begin
    let (parser, left_angle) = assert_token parser LessThan in
    (* ERROR RECOVERY: We could improve error recovery by detecting
       array<,  and marking the key type as missing. *)
    let (parser, key_type) = parse_type_specifier parser in
    let kind = Token.kind (peek_token parser) in
    if kind = GreaterThan then
      let (parser, right_angle) = fetch_token parser in
      Make.vector_array_type_specifier
        parser
        array_token
        left_angle
        key_type
        right_angle
    else if kind = Comma then
      let (parser, comma) = fetch_token parser in
      let next_token_kind = Token.kind (peek_token parser) in
      let (parser, value_type) =
        if next_token_kind = GreaterThan then
          Make.missing parser (pos parser)
        else parse_type_specifier parser
      in
      let (parser, right_angle) = require_right_angle parser in
      Make.map_array_type_specifier
        parser
        array_token
        left_angle
        key_type
        comma
        value_type
        right_angle
    else
      (* ERROR RECOVERY: Assume that the > is missing and keep going. *)
      let (parser, right_angle) = Make.missing parser (pos parser) in
      Make.vector_array_type_specifier
        parser
        array_token
        left_angle
        key_type
        right_angle
    end


  and parse_darray_type_specifier parser =
    (* darray<type, type> *)
    let parser, array_token = assert_token parser Darray in
    if peek_token_kind parser != LessThan then
      Make.simple_type_specifier parser array_token
    else
      let parser, left_angle = assert_token parser LessThan in
      let parser, key_type = parse_type_specifier parser in
      let parser, comma = require_comma parser in
      let parser, value_type = parse_type_specifier parser in
      let parser, optional_comma = optional_token parser Comma in
      let parser, right_angle = require_right_angle parser in
      Make.darray_type_specifier
        parser
        array_token
        left_angle
        key_type
        comma
        value_type
        optional_comma
        right_angle

  and parse_varray_type_specifier parser =
    (* varray<type> *)
    let (parser, array_token) = assert_token parser Varray in
    if peek_token_kind parser != LessThan then
      Make.simple_type_specifier parser array_token
    else
      let (parser, left_angle) = assert_token parser LessThan in
      let (parser, value_type) = parse_type_specifier parser in
      let (parser, optional_comma) = optional_token parser Comma in
      let (parser, right_angle) = fetch_token parser in
      Make.varray_type_specifier
        parser
        array_token
        left_angle
        value_type
        optional_comma
        right_angle

  and parse_vec_type_specifier parser =
    (*
      vec < type-specifier >
      TODO: Should we allow a trailing comma?
      TODO: Add this to the specification
      ERROR RECOVERY: If there is no type argument list then just make
      this a simple type.  TODO: Should this be an error at parse time? what
      about at type checking time?
    *)
    let (parser, keyword) = assert_token parser Vec in
    if peek_token_kind parser != LessThan then
      Make.simple_type_specifier parser keyword
    else
      let (parser, left) = require_left_angle parser in
      let (parser, t) = parse_type_specifier parser in
      let (parser, optional_comma) = optional_token parser Comma in
      let (parser, right) = require_right_angle parser in
      Make.vector_type_specifier parser keyword left t optional_comma right

  and parse_keyset_type_specifier parser =
    (*
      keyset < type-specifier >
      TODO: Should we allow a trailing comma?
      TODO: Add this to the specification
      ERROR RECOVERY: If there is no type argument list then just make
      this a simple type.  TODO: Should this be an error at parse time? what
      about at type checking time?
    *)
    let (parser, keyword) = assert_token parser Keyset in
    if peek_token_kind parser != LessThan then
      Make.simple_type_specifier parser keyword
    else
      let (parser, left) = require_left_angle parser in
      let (parser, t) = parse_type_specifier parser in
      let (parser, comma) = optional_token parser Comma in
      let (parser, right) = require_right_angle parser in
      Make.keyset_type_specifier parser keyword left t comma right

  and parse_tuple_type_explicit_specifier parser =
    (*
      tuple < type-specifier-list >
      TODO: Add this to the specification
    *)
    let (parser, keyword) = assert_token parser Tuple in
    let (parser, left_angle) = require_left_angle parser in
    let (parser, args) = parse_type_list parser GreaterThan in
    let (parser1, right_angle) = next_token parser in
    if (Token.kind right_angle) = GreaterThan then
      let (parser, token) = Make.token parser1 right_angle in
      Make.tuple_type_explicit_specifier parser keyword left_angle args token
    else
      (* ERROR RECOVERY: Don't eat the token that is in the place of the
         missing > or ,.  Assume that it is the > that is missing and
         try to parse whatever is coming after the type.  *)
      let parser = with_error parser SyntaxError.error1022 in
      let (parser, right_angle) = Make.missing parser (pos parser) in
      Make.tuple_type_explicit_specifier
        parser
        keyword
        left_angle
        args
        right_angle

  and parse_dictionary_type_specifier parser =
    (*
      dict < type-specifier , type-specifier >

      TODO: Add this to the specification

      Though we require there to be exactly two items, we actually parse
      an arbitrary comma-separated list here.

      TODO: Give an error in a later pass if there are not exactly two members.

      ERROR RECOVERY: If there is no type argument list then just make this
      a simple type.  TODO: Should this be an error at parse time?  what
      about at type checking time?
    *)
    let (parser, keyword) = assert_token parser Dict in
    if peek_token_kind parser != LessThan then
      Make.simple_type_specifier parser keyword
    else
      (* TODO: This allows "noreturn" as a type argument. Should we
      disallow that at parse time? *)
      let (parser, left) = require_left_angle parser in
      let (parser, arguments, _) =
        parse_comma_list_allow_trailing
          parser
          GreaterThan
          SyntaxError.error1007
          parse_return_type
      in
      let (parser, right) = require_right_angle parser in
      Make.dictionary_type_specifier parser keyword left arguments right

and parse_tuple_or_closure_type_specifier parser =
  let (parser1, _) = assert_token parser LeftParen in
  let token = peek_token parser1 in
  match Token.kind token with
  | Function
  | Coroutine ->
    parse_closure_type_specifier parser
  | _ ->
    parse_tuple_type_specifier parser

and parse_closure_type_specifier parser =
  (* SPEC

    TODO: Update the specification with closure-param-type-specifier-list-opt.
    (This work is tracked by task T22582676.)

    TODO: Update grammar for inout parameters.
    (This work is tracked by task T22582715.)

    closure-type-specifier:
      ( coroutine-opt function ( \
      closure-param-type-specifier-list-opt \
      ) : type-specifier )
  *)
  (* TODO: Error recovery is pretty weak here. We could be smarter. *)
  let (parser, olp) = fetch_token parser in
  let (parser, coroutine) = optional_token parser Coroutine in
  let (parser, fnc) = fetch_token parser in
  let (parser, ilp) = require_left_paren parser in
  let (parser1, token) = next_token parser in
  let (parser, pts, irp) =
    if (Token.kind token) = RightParen then
      let (parser, missing) = Make.missing parser1 (pos parser) in
      let (parser, token) = Make.token parser token in
      (parser, missing, token)
    else
      (* TODO add second pass checking to ensure ellipsis is the last arg *)
      let (parser, pts) = parse_closure_param_list parser RightParen in
      let (parser, irp) = require_right_paren parser in
      (parser, pts, irp)
  in
  let (parser, col) = require_colon parser in
  let (parser, ret) = parse_type_specifier parser in
  let (parser, orp) = require_right_paren parser in
  Make.closure_type_specifier parser olp coroutine fnc ilp pts irp col ret orp

and parse_tuple_type_specifier parser =
  (* SPEC
      tuple-type-specifier:
        ( type-specifier  ,  type-specifier-list  )
      type-specifier-list:
        type-specifiers  ,opt
      type-specifiers
        type-specifier
        type-specifiers , type-specifier
  *)

  (* TODO: Here we parse a type list with one or more items, but the grammar
     actually requires a type list with two or more items. Give an error in
     a later pass if there is only one item here. *)

  let (parser, left_paren) = assert_token parser LeftParen in
  let (parser, args) = parse_type_list parser RightParen in
  let (parser1, right_paren) = next_token parser in
  if (Token.kind right_paren) = RightParen then
    let (parser, token) = Make.token parser1 right_paren in
    Make.tuple_type_specifier parser left_paren args token
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing ) or ,.  Assume that it is the ) that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1022 in
    let (parser, missing) = Make.missing parser (pos parser) in
    Make.tuple_type_specifier parser left_paren args missing

and parse_nullable_type_specifier parser =
  (* SPEC:
    nullable-type-specifier:
      ? type-specifier
      mixed

  * Note that we parse "mixed" as a simple type specifier, even though
    technically it is classified as a nullable type specifier by the grammar.
  * Note that it is perfectly legal to have trivia between the ? and the
    underlying type. *)
  let (parser, question) = assert_token parser Question in
  let (parser, nullable_type) = parse_type_specifier parser in
  Make.nullable_type_specifier parser question nullable_type

and parse_soft_type_specifier parser =
  (* SPEC (Draft)
    soft-type-specifier:
      @ type-specifier

    TODO: The spec does not mention this type grammar.  Work out where and
    when it is legal, and what the exact semantics are, and put it in the spec.
    Add an error pass if necessary to identify illegal usages of this type.

    Note that it is legal for trivia to come between the @ and the type.
  *)
  let (parser, soft_at) = assert_token parser At in
  let (parser, soft_type) = parse_type_specifier parser in
  Make.soft_type_specifier parser soft_at soft_type

and parse_classname_type_specifier parser =
  (* SPEC
    classname-type-specifier:
      classname
      classname  <  qualified-name generic-type-argument-list-opt >

      TODO: We parse any type as the class name type; we should write an
      error detection pass later that determines when this is a bad type.

      TODO: Is this grammar correct?  In particular, can the name have a
      scope resolution operator (::) in it?  Find out and update the spec if
      this is permitted.
  *)

  (* TODO ERROR RECOVERY is unsophisticated here. *)
  let (parser, classname) = fetch_token parser in
  match peek_token_kind parser with
  | LessThan ->
    let (parser, left_angle) = require_left_angle parser in
    let (parser, classname_type) = parse_type_specifier parser in
    let (parser, optional_comma) = optional_token parser Comma in
    let (parser, right_angle) = require_right_angle parser in
    Make.classname_type_specifier
      parser
      classname
      left_angle
      classname_type
      optional_comma
      right_angle
  | _ ->
    let (parser, missing1) = Make.missing parser (pos parser) in
    let (parser, missing2) = Make.missing parser (pos parser) in
    let (parser, missing3) = Make.missing parser (pos parser) in
    let (parser, missing4) = Make.missing parser (pos parser) in
    Make.classname_type_specifier
      parser
      classname
      missing1
      missing2
      missing3
      missing4

and parse_field_specifier parser =
  (* SPEC
    field-specifier:
      ?-opt present-field-specifier
    present-field-specifier:
      single-quoted-string-literal  =>  type-specifier
      qualified-name  =>  type-specifier
      scope-resolution-expression  =>  type-specifier
  *)

  (* TODO: We require that it be either all literals or no literals in the
           set of specifiers; make an error reporting pass that detects this. *)

  (* ERROR RECOVERY: We allow any expression for the left-hand side.
     TODO: Make an error-detecting pass that gives an error if the left-hand
     side is not a literal or name. *)
  let (parser, question) =
    if peek_token_kind parser = Question
    then assert_token parser Question
    else Make.missing parser (pos parser)
  in
  let (parser, name) = parse_expression parser in
  let (parser, arrow) = require_arrow parser in
  let (parser, field_type) = parse_type_specifier parser in
  Make.field_specifier parser question name arrow field_type

and parse_shape_specifier parser =
  (* SPEC
    shape-specifier:
      shape ( field-specifier-list-opt )
    field-specifier-list:
      field-specifiers  ,  ...
      field-specifiers  ,-opt
    field-specifiers:
      field-specifier
      field-specifiers  ,  field-specifier
  *)
  (* TODO: ERROR RECOVERY is not very sophisticated here. *)
  let (parser, shape) = fetch_token parser in
  let (parser, lparen) = require_left_paren parser in
  let is_closing_token = function
    | RightParen | DotDotDot -> true
    | _ -> false in
  let (parser, fields) = parse_comma_list_opt_allow_trailing_predicate
    parser is_closing_token SyntaxError.error1025 parse_field_specifier in
  let (parser, ellipsis) =
    if peek_token_kind parser = DotDotDot
    then assert_token parser DotDotDot
    else Make.missing parser (pos parser)
  in
  let (parser, rparen) = require_right_paren parser in
  Make.shape_type_specifier parser shape lparen fields ellipsis rparen

and parse_type_constraint_opt parser =
  (* SPEC
    type-constraint:
      as  type-specifier
    TODO: Is this correct? Or do we need to allow "super" as well?
    TODO: What about = ?
  *)
  let (parser1, constraint_as) = next_token parser in
  let kind = Token.kind constraint_as in
  if kind = As then
    let (parser, constraint_as) = Make.token parser1 constraint_as in
    let (parser, constraint_type) = parse_type_specifier parser in
    Make.type_constraint parser constraint_as constraint_type
  else
    Make.missing parser (pos parser)

and parse_return_type parser =
  let (parser1, token) = next_token parser in
  if (Token.kind token) = Noreturn then
    Make.token parser1 token
  else
    parse_type_specifier parser

end
end (* WithSmartConstructors *)
end (* WithSyntax *)
