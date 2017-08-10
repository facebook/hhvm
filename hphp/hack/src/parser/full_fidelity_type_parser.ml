(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Token = Full_fidelity_minimal_token
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module SimpleParser =
  Full_fidelity_simple_parser.WithLexer(Full_fidelity_type_lexer)

open TokenKind
open Full_fidelity_minimal_syntax

module WithExpressionParser (ExpressionParser :
    Full_fidelity_expression_parser_type.ExpressionParserType) :
  Full_fidelity_type_parser_type.TypeParserType = struct

include SimpleParser
include Full_fidelity_parser_helpers.WithParser(SimpleParser)

let parse_expression parser =
  let expr_parser = ExpressionParser.make parser.lexer
    parser.errors parser.context in
  let (expr_parser, node) = ExpressionParser.parse_expression expr_parser in
  let lexer = ExpressionParser.lexer expr_parser in
  let errors = ExpressionParser.errors expr_parser in
  let parser = { parser with lexer; errors } in
  (parser, node)

(* TODO: What about something like for::for? Is that a legal
  type constant?  *)

let rec parse_type_specifier parser =
  (* Strictly speaking, "mixed" is a nullable type specifier. We parse it as
     a simple type specifier here. *)
  let (parser1, token) = next_xhp_class_name_or_other parser in
  match Token.kind token with
  | Double (* TODO: Specification does not mention double; fix it. *)
  | Bool
  | Int
  | Float
  | Num
  | String
  | Arraykey
  | Void
  | Noreturn
  | Resource
  | Object
  | Mixed -> (parser1, make_simple_type_specifier (make_token token))
  | This -> parse_simple_type_or_type_constant parser
  | Name -> parse_simple_type_or_type_constant_or_generic parser
  | Self
  | Parent -> parse_simple_type_or_type_constant parser
  | XHPClassName
  | QualifiedName -> parse_possible_generic_specifier_or_type_const parser
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
    let parser = with_error parser1 SyntaxError.error1007 in
    (parser, make_error (make_token token))

(* SPEC
  type-constant-type-name:
    name  ::  name
    self  ::  name
    this  ::  name
    parent  ::  name
    type-constant-type-name  ::  name
*)

and parse_remaining_type_constant parser left =
  let (parser, separator) = next_token parser in
  let (parser1, right) = next_token_as_name parser in
  if (Token.kind right) = Name then
    begin
      let syntax =
        make_type_constant left (make_token separator) (make_token right) in
      let token = peek_token parser1 in
      if (Token.kind token) = ColonColon then
        parse_remaining_type_constant parser1 syntax
      else
        (parser1, syntax)
    end
  else
    (* ERROR RECOVERY: Assume that the thing following the ::
       that is not a name belongs to the next thing to be
       parsed; treat the name as missing. *)
    let parser = with_error parser1 SyntaxError.error1004 in
    let syntax = make_type_constant
      left (make_token separator) (make_missing()) in
    (parser, syntax)

and parse_simple_type_or_type_constant parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  let token = peek_token parser in
  match Token.kind token with
  | ColonColon -> parse_remaining_type_constant parser (make_token name)
  | Self | Parent ->
    begin
      match peek_token_kind ~lookahead:1 parser with
      | ColonColon -> parse_remaining_type_constant parser (make_token name)
      | _ ->
        (parser, make_type_constant
                   (make_token token)
                   (make_missing())
                   (make_missing()))
    end
  | _ -> (parser, make_simple_type_specifier (make_token name))

and parse_simple_type_or_type_constant_or_generic parser =
  let (parser0, _) = next_xhp_class_name_or_other parser in
  match peek_token_kind parser0 with
  | LessThan -> parse_possible_generic_specifier_or_type_const parser
  | _ -> parse_simple_type_or_type_constant parser

and parse_possible_generic_specifier_or_type_const parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  let (parser, arguments) = parse_generic_type_argument_list_opt parser in
  if (kind arguments) = SyntaxKind.Missing then
    let token = peek_token parser in
    match Token.kind token with
    | ColonColon -> parse_remaining_type_constant parser (make_token name)
    | _ -> (parser, make_simple_type_specifier (make_token name))
  else
    (parser, make_generic_type_specifier (make_token name) arguments)

(* SPEC
  class-interface-trait-specifier:
    qualified-name generic-type-argument-listopt
*)
and parse_possible_generic_specifier parser =
  let (parser, name) = next_xhp_class_name_or_other parser in
  let (parser, arguments) = parse_generic_type_argument_list_opt parser in
  if (kind arguments) = SyntaxKind.Missing then
    (parser, make_simple_type_specifier (make_token name))
  else
    (parser, make_generic_type_specifier (make_token name) arguments)

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
    let constraint_token = make_token token in
    let (parser, matched_type) = parse_type_specifier parser1 in
    let type_constraint = make_type_constraint constraint_token matched_type in
    (parser, Some type_constraint)
  | _ -> (parser, None)

and parse_variance_opt parser =
  match peek_token_kind parser with
  | Plus | Minus ->
    let (parser, token) = next_token parser in
    let variance = (make_token token) in
    (parser, variance)
  | _ -> (parser, make_missing())

(* SPEC
  generic-type-parameter:
    generic-type-parameter-variance-opt  name  generic-type-constraint-list-opt

  generic-type-parameter-variance:
    +
    -

  TODO: SPEC ISSUE: We allow any number of type constraints, not just zero
  or one as indicated in the spec.
  https://github.com/hhvm/hack-langspec/issues/83
*)
and parse_type_parameter parser =
  let parser, variance = parse_variance_opt parser in
  let (parser, type_name) = require_name_allow_keywords parser in
  let (parser, constraints) =
    parse_list_until_none parser parse_generic_type_constraint_opt in
  (parser, make_type_parameter variance type_name constraints)

(* SPEC
  type-parameter-list:
  < generic-type-parameters  ,-opt >

  generic-type-parameters:
    generic-type-parameter
    generic-type-parameter  ,  generic-type-parameter
*)
and parse_generic_type_parameter_list parser =
  let (parser, left) = assert_token parser LessThan in
  let (parser, params) =  parse_comma_list_allow_trailing parser GreaterThan
    SyntaxError.error1007 parse_type_parameter in
  let (parser, right) = require_right_angle parser in
  let result = make_type_parameters left params right in
  (parser, result)

and parse_generic_parameter_list_opt parser =
  match peek_token_kind parser with
  | LessThan -> parse_generic_type_parameter_list parser
  | _ -> (parser, make_missing ())

and parse_generic_type_argument_list_opt parser =
  let token = peek_token parser in
  if (Token.kind token) = LessThan then
    parse_generic_type_argument_list parser
  else
    (parser, make_missing())

and parse_type_list parser close_kind =
  (* SPEC:
    type-specifier-list:
      type-specifiers  ,opt

    type-specifiers:
      type-specifier
      type-specifiers  ,  type-specifier
  *)
  parse_comma_list_allow_trailing parser close_kind SyntaxError.error1007
    parse_type_specifier

and parse_type_or_ellipsis_list parser close_kind =
  parse_comma_list_allow_trailing parser close_kind SyntaxError.error1007
  parse_type_or_ellipsis

and parse_type_or_ellipsis parser =
  let (parser1, token) = next_token parser in
  match Token.kind token with
  | DotDotDot -> (parser1, make_variadic_parameter (make_token token))
  | _ -> parse_type_specifier parser

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
  let (parser, open_angle) = next_token parser in
  let open_angle = make_token open_angle in
  let (parser, args) = parse_comma_list_allow_trailing parser GreaterThan
    SyntaxError.error1007 parse_return_type in
  let (parser1, close_angle) = next_token parser in
  if (Token.kind close_angle) = GreaterThan then
    let result = make_type_arguments open_angle args (make_token close_angle) in
    (parser1, result)
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing > or ,.  Assume that it is the > that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1014 in
    let result = make_type_arguments open_angle args (make_missing()) in
    (parser, result)

and parse_array_type_specifier parser =
  (* We allow
     array
     array<type>
     array<type, type>
     TODO: Put a proper reference to the specification in here.
     TODO: Can we have a comma termination in either case?  This list
     never has more than two elements.
     TODO: Should we just parse a comma-separated list here and give an
     error in a later pass?
  *)
  let (parser, array_token) = assert_token parser Array in
  if peek_token_kind parser <> LessThan then
    (parser, make_simple_type_specifier array_token)
  else begin
    let (parser, left_angle) = assert_token parser LessThan in
    (* ERROR RECOVERY: We could improve error recovery by detecting
       array<,  and marking the key type as missing. *)
    let (parser, key_type) = parse_type_specifier parser in
    let kind = Token.kind (peek_token parser) in
    if kind = GreaterThan then
      let (parser, right_angle) = next_token parser in
      let right_angle = make_token right_angle in
      let result = make_vector_array_type_specifier array_token
        left_angle key_type right_angle in
      (parser, result)
    else if kind = Comma then
      let (parser, comma) = next_token parser in
      let comma = make_token comma in
      let (parser, value_type) = parse_type_specifier parser in
      let (parser, right_angle) = require_right_angle parser in
      let result = make_map_array_type_specifier array_token left_angle key_type
        comma value_type right_angle in
      (parser, result)
    else
      (* ERROR RECOVERY: Assume that the > is missing and keep going. *)
      let right_angle = make_missing() in
      let result = make_vector_array_type_specifier array_token
        left_angle key_type right_angle in
      (parser, result)
    end


  and parse_darray_type_specifier parser =
    (* darray<type, type> *)
    let parser, array_token = assert_token parser Darray in
    if peek_token_kind parser != LessThan then
      let result = make_simple_type_specifier array_token in
      (parser, result)
    else
      let parser, left_angle = assert_token parser LessThan in
      let parser, key_type = parse_type_specifier parser in
      let parser, comma = next_token parser in
      let comma = make_token comma in
      let parser, value_type = parse_type_specifier parser in
      let parser, optional_comma = optional_token parser Comma in
      let parser, right_angle = require_right_angle parser in
      let result =
        make_darray_type_specifier
          array_token
          left_angle
          key_type
          comma
          value_type
          optional_comma
          right_angle in
      parser, result

  and parse_varray_type_specifier parser =
    (* varray<type> *)
    let parser, array_token = assert_token parser Varray in
    if peek_token_kind parser != LessThan then
      let result = make_simple_type_specifier array_token in
      (parser, result)
    else
      let parser, left_angle = assert_token parser LessThan in
      let parser, value_type = parse_type_specifier parser in
      let parser, optional_comma = optional_token parser Comma in
      let parser, right_angle = next_token parser in
      let right_angle = make_token right_angle in
      let result =
        make_varray_type_specifier
          array_token
          left_angle
          value_type
          optional_comma
          right_angle in
      parser, result

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
      let result = make_simple_type_specifier keyword in
      (parser, result)
    else
      let (parser, left) = require_left_angle parser in
      let (parser, t) = parse_type_specifier parser in
      let (parser, optional_comma) = optional_token parser Comma in
      let (parser, right) = require_right_angle parser in
      let result =
        make_vector_type_specifier keyword left t optional_comma right in
      (parser, result)

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
      let result = make_simple_type_specifier keyword in
      (parser, result)
    else
      let (parser, left) = require_left_angle parser in
      let (parser, t) = parse_type_specifier parser in
      let (parser, comma) = optional_token parser Comma in
      let (parser, right) = require_right_angle parser in
      let result = make_keyset_type_specifier keyword left t comma right in
      (parser, result)

  and parse_tuple_type_explicit_specifier parser =
    (*
      tuple < type-specifier-list >
      TODO: Add this to the specification
    *)
    let (parser, keyword) = assert_token parser Tuple in
    let (parser, left_angle) = assert_token parser LessThan in
    let (parser, args) = parse_type_list parser GreaterThan in
    let (parser1, right_angle) = next_token parser in
    if (Token.kind right_angle) = GreaterThan then
      let result = make_tuple_type_explicit_specifier keyword left_angle args
        (make_token right_angle) in
      (parser1, result)
    else
      (* ERROR RECOVERY: Don't eat the token that is in the place of the
         missing > or ,.  Assume that it is the > that is missing and
         try to parse whatever is coming after the type.  *)
      let parser = with_error parser SyntaxError.error1022 in
      let right_angle = make_missing () in
      let result =
        make_tuple_type_explicit_specifier keyword left_angle args right_angle
      in
      (parser, result)

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
      let result = make_simple_type_specifier keyword in
      (parser, result)
    else
      (* TODO: This allows "noreturn" as a type argument. Should we
      disallow that at parse time? *)
      let (parser, left) = require_left_angle parser in
      let (parser, arguments) =
        parse_comma_list_allow_trailing parser GreaterThan
        SyntaxError.error1007 parse_return_type in
      let (parser, right) = require_right_angle parser in
      let result = make_dictionary_type_specifier
        keyword left arguments right in
      (parser, result)

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
      closure-type-specifier:
          ( coroutine-opt function ( type-specifier-listopt ) : type-specifier )
  *)

  (* TODO: Error recovery is pretty weak here. We could be smarter. *)
  let (parser, olp) = next_token parser in
  let olp = make_token olp in
  let (parser, coroutine) = optional_token parser Coroutine in
  let (parser, fnc) = next_token parser in
  let fnc = make_token fnc in
  let (parser, ilp) = require_left_paren parser in
  let (parser1, token) = next_token parser in
  let (parser, pts, irp) =
    if (Token.kind token) = RightParen then
      (parser1, (make_missing()), (make_token token))
    else
      (* TODO add second pass checking to ensure ellipsis is the last arg *)
      let (parser, pts) = parse_type_or_ellipsis_list parser RightParen in
      let (parser, irp) = require_right_paren parser in
      (parser, pts, irp) in
  let (parser, col) = require_colon parser in
  let (parser, ret) = parse_type_specifier parser in
  let (parser, orp) = require_right_paren parser in
  let result =
    make_closure_type_specifier olp coroutine fnc ilp pts irp col ret orp in
  (parser, result)

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
    let result = make_tuple_type_specifier left_paren args
      (make_token right_paren) in
    (parser1, result)
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing ) or ,.  Assume that it is the ) that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1022 in
    let result = make_tuple_type_specifier left_paren args (make_missing()) in
    (parser, result)

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
  let result = make_nullable_type_specifier question nullable_type in
  (parser, result)

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
  let result = make_soft_type_specifier soft_at soft_type in
  (parser, result)

and parse_classname_type_specifier parser =
  (* SPEC
    classname-type-specifier:
      classname  <  qualified-name generic-type-argument-list-opt >

      TODO: We parse any type as the class name type; we should write an
      error detection pass later that determines when this is a bad type.

      TODO: Is this grammar correct?  In particular, can the name have a
      scope resolution operator (::) in it?  Find out and update the spec if
      this is permitted.
  *)

  (* TODO ERROR RECOVERY is unsophisticated here. *)
  let (parser, classname) = next_token parser in
  let classname = make_token classname in
  let (parser, left_angle) = require_left_angle parser in
  let (parser, classname_type) = parse_type_specifier parser in
  let (parser, optional_comma) = optional_token parser Comma in
  let (parser, right_angle) = require_right_angle parser in
  let result = make_classname_type_specifier
    classname left_angle classname_type optional_comma right_angle in
  (parser, result)

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
    else (parser, (make_missing())) in
  let (parser, name) = parse_expression parser in
  let (parser, arrow) = require_arrow parser in
  let (parser, field_type) = parse_type_specifier parser in
  let result = make_field_specifier question name arrow field_type in
  (parser, result)

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
  let (parser, shape) = next_token parser in
  let shape = make_token shape in
  let (parser, lparen) = require_left_paren parser in
  let is_closing_token = function
    | RightParen | DotDotDot -> true
    | _ -> false in
  let (parser, fields) = parse_comma_list_opt_allow_trailing_predicate
    parser is_closing_token SyntaxError.error1025 parse_field_specifier in
  let (parser, ellipsis) =
    if peek_token_kind parser = DotDotDot
    then assert_token parser DotDotDot
    else (parser, (make_missing())) in
  let (parser, rparen) = require_right_paren parser in
  let result = make_shape_type_specifier shape lparen fields ellipsis rparen in
  (parser, result)

and parse_type_constraint_opt parser =
  (* SPEC
    type-constraint:
      as  type-specifier
    TODO: Is this correct? Or do we need to allow "super" as well?
    TODO: What about = ?
  *)
  let (parser1, constraint_as) = next_token parser in
  if (Token.kind constraint_as) = As then
    let constraint_as = make_token constraint_as in
    let (parser, constraint_type) = parse_type_specifier parser1 in
    let result = make_type_constraint constraint_as constraint_type in
    (parser, result)
  else
    (parser, (make_missing()))

and parse_return_type parser =
  let (parser1, token) = next_token parser in
  if (Token.kind token) = Noreturn then
    (parser1, make_token token)
  else
    parse_type_specifier parser

end
