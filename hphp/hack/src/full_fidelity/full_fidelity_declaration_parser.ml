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
module Syntax = Full_fidelity_minimal_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Operator = Full_fidelity_operator
module TypeParser = Full_fidelity_type_parser
module SimpleParser = Full_fidelity_simple_parser.WithLexer(Full_fidelity_lexer)

open TokenKind
open Syntax

module WithExpressionAndStatementParser
  (ExpressionParser : Full_fidelity_expression_parser_type.ExpressionParserType)
  (StatementParser : Full_fidelity_statement_parser_type.StatementParserType) :
  Full_fidelity_declaration_parser_type.DeclarationParserType = struct

  include SimpleParser
  include Full_fidelity_parser_helpers.WithParser(SimpleParser)

  (* Types *)

  let parse_in_type_parser parser type_parser_function =
    let type_parser = TypeParser.make parser.lexer parser.errors in
    let (type_parser, node) = type_parser_function type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { lexer; errors } in
    (parser, node)

  let parse_possible_generic_specifier parser =
    parse_in_type_parser parser TypeParser.parse_possible_generic_specifier

  let parse_type_specifier parser =
    parse_in_type_parser parser TypeParser.parse_type_specifier

  let parse_return_type parser =
    parse_in_type_parser parser TypeParser.parse_return_type

  let parse_type_constraint_opt parser =
    parse_in_type_parser parser TypeParser.parse_type_constraint_opt

  (* Expressions *)

  let parse_expression parser =
    let expression_parser = ExpressionParser.make parser.lexer parser.errors in
    let (expression_parser, node) =
      ExpressionParser.parse_expression expression_parser in
    let lexer = ExpressionParser.lexer expression_parser in
    let errors = ExpressionParser.errors expression_parser in
    let parser = { lexer; errors } in
    (parser, node)


  (* Statements *)

  let parse_compound_statement parser =
    let statement_parser = StatementParser.make parser.lexer parser.errors in
    let (statement_parser, node) =
      StatementParser.parse_compound_statement statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let parser = { lexer; errors } in
    (parser, node)


  (* Declarations *)

  let rec parse_inclusion_directive parser =
  (* SPEC:
    inclusion-directive:
      require-multiple-directive
      require-once-directive

    require-multiple-directive:
      require  (  include-filename  )  ;
      require  include-filename  ;

    include-filename:
      expression

    require-once-directive:
      require_once  (  include-filename  )  ;
      require_once  include-filename  ;
    *)

    let (parser, require) = next_token parser in
    let require = make_token require in
    let (parser, left_paren) = optional_token parser LeftParen in
    let (parser, filename) = parse_expression parser in
    (* ERROR RECOVERY: TODO: We could detect if there is a right paren but
       no left paren and give an error saying the left paren is missing. *)
    let (parser, right_paren) =
      if is_missing left_paren then (parser, (make_missing()))
      else expect_right_paren parser in
    let (parser, semi) = expect_semicolon parser in
    let result = make_inclusion_directive
      require left_paren filename right_paren semi in
    (parser, result)

  and parse_alias_declaration parser =
    (* SPEC
      alias-declaration:
        type  name  =  type-to-be-aliased  ;
        newtype  name  type-constraintopt  =  type-to-be-aliased  ;

      type-to-be-aliased:
        type-specifier
        qualified-name
    *)

    (* TODO: Produce an error if the "type" version has a constraint. *)

    let (parser, token) = next_token parser in
    let token = make_token token in
    let (parser, name) = expect_name parser in
    let (parser, constr) = parse_type_constraint_opt parser in
    let (parser, equal) = expect_equal parser in
    let (parser, ty) = parse_type_specifier parser in
    let (parser, semi) = expect_semicolon parser in
    let result = make_alias token name constr equal ty semi in
    (parser, result)

  and parse_enumerator parser =
    (* SPEC
      enumerator:
        enumerator-constant  =  constant-expression ;
      enumerator-constant:
        name
      *)
    (* TODO: Add an error to a later pass that determines the value is
             a constant. *)
    let (parser, name) = expect_name parser in
    let (parser, equal) = expect_equal parser in
    let (parser, value) = parse_expression parser in
    let (parser, semicolon) = expect_semicolon parser  in
    let result = make_enumerator name equal value semicolon in
    (parser, result)

  and parse_enumerator_list_opt parser =
    (* SPEC
      enumerator-list:
        enumerator
        enumerator-list   enumerator
    *)
    let rec aux acc parser =
      let token = peek_token parser in
      match Token.kind token with
      | RightBrace -> (parser, make_list (List.rev acc))
      | EndOfFile ->
        (* ERROR RECOVERY: reach end of file, expect brace of enumerator *)
        let parser = with_error parser SyntaxError.error1040 in
        (parser, make_error [make_token token])
      | _ ->
        let (parser, enumerator) = parse_enumerator parser in
        aux (enumerator :: acc) parser
    in
    let token = peek_token parser in
    match Token.kind token with
    | RightBrace -> parser, make_missing ()
    | _ -> aux [] parser


  and parse_enum_declaration parser =
    (*
    enum-declaration:
      enum  name  enum-base  type-constraint-opt  {  enumerator-list-opt  }
    enum-base:
      :  int
      :  string
    *)
    let (parser, enum) = assert_token parser Enum in
    let (parser, name) = expect_name parser in
    let (parser, colon) = expect_colon parser in
    let (parser1, base) = next_token parser in
    let (parser, base) = match Token.kind base with
    | String
    | Int -> (parser1, make_token base)
    | LeftBrace ->
      (* ERROR RECOVERY *)
      (parser, make_missing())
    | _ ->
      (* ERROR RECOVERY *)
      (parser1, make_token base) in
    let (parser, enum_type) = parse_type_constraint_opt parser in
    let (parser, left_brace, enumerators, right_brace) = parse_delimited_list
      parser LeftBrace SyntaxError.error1037 RightBrace SyntaxError.error1006
      parse_enumerator_list_opt in
    let result = make_enum
      enum name colon base enum_type left_brace enumerators right_brace in
    (parser, result)

  and parse_namespace_declaration parser =
    (* SPEC
      namespace-definition:
        namespace  namespace-name  ;
        namespace  namespace-name-opt  { declaration-list }
    *)

    (* TODO: Some error cases not caught by the parser that should be caught
             in later passes:
             (1) You cannot mix the "semi" and "compound" flavours in one script
             (2) The declaration list may not contain a namespace decl.
             (3) Qualified names are a superset of legal namespace names.
    *)
    let (parser, namespace_token) = assert_token parser Namespace in
    let (parser1, token) = next_token parser in
    let (parser, name) = match Token.kind token with
    | Name
    | QualifiedName -> (parser1, make_token token)
    | LeftBrace -> (parser, (make_missing()))
    | Semicolon ->
      (* ERROR RECOVERY Plainly the name is missing. *)
      (with_error parser SyntaxError.error1004, (make_missing()))
    | _ ->
      (with_error parser1 SyntaxError.error1004, make_token token) in
    let (parser, body) = parse_namespace_body parser in
    let result = make_namespace namespace_token name body in
    (parser, result)

  and parse_namespace_body parser =
    let (parser, token) = next_token parser in
    match Token.kind token with
    | Semicolon -> (parser, make_token token)
    | LeftBrace ->
      let left = make_token token in
      let (parser, body) = parse_declarations parser true in
      let (parser, right) = expect_right_brace parser in
      let result = make_namespace_body left body right in
      (parser, result)
    | _ ->
      (* ERROR RECOVERY: Eat the offending token.
         TODO: Better would be to attempt to recover to the list of
         declarations? Suppose the offending token is "class" for instance? *)
      let parser = with_error parser SyntaxError.error1038 in
      let result = make_error [make_token token] in
      (parser, result)

  and parse_namespace_use_clause parser =
    (* SPEC
      namespace-use-clause:
        qualified-name  namespace-aliasing-clauseopt
      namespace-aliasing-clause:
        as  name
    *)
    let (parser, name) = expect_qualified_name parser in
    let (parser1, as_token) = next_token parser in
    let (parser, as_token, alias) =
      if Token.kind as_token = As then
        let as_token = make_token as_token in
        let (parser, alias) = expect_name parser1 in
        (parser, as_token, alias)
      else
        (parser, (make_missing()), (make_missing())) in
    let result = make_namespace_use_clause name as_token alias in
    (parser, result)

  and parse_namespace_use_declaration parser =
    (* SPEC
      namespace-use-declaration:
        use  namespace-use-clauses  ;
    *)
    let (parser, use_token) = assert_token parser Use in
    let (parser, clauses) = parse_comma_list
      parser Semicolon SyntaxError.error1004 parse_namespace_use_clause in
    let (parser, semi) = expect_semicolon parser in
    let result = make_namespace_use use_token clauses semi in
    (parser, result)

  and parse_classish_declaration parser attribute_spec =
    let (parser, abstract, final) =
      parse_classish_modifier_opt parser in
    let (parser, token) =
      parse_classish_token parser in
    let (parser, name) = expect_name parser in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser in
    let (parser, classish_extends, classish_extends_list) =
      parse_classish_extends_opt parser in
    let (parser, classish_implements, classish_implements_list) =
      parse_classish_implements_opt parser in
    let (parser, body) = parse_classish_body parser in
    let syntax = make_classish
      attribute_spec abstract final token name generic_type_parameter_list
      classish_extends classish_extends_list classish_implements
      classish_implements_list
      body
    in
    (parser, syntax)

  and parse_classish_modifier_opt parser =
    let (parser, abstract) = optional_token parser Abstract in
    let (parser, final) = optional_token parser Final in
    (parser, abstract, final)

  and parse_classish_token parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
      | Class
      | Trait
      | Interface -> (parser1, make_token token)
      | _ -> (with_error parser SyntaxError.error1035, (make_missing()))

  and parse_classish_extends_opt parser =
    let (parser1, extends_token) = next_token parser in
    if (Token.kind extends_token) <> Extends then
      (parser, make_missing (), Syntax.make_missing ())
    else
    let (parser, extends_list) = parse_qualified_name_list parser1 in
    (parser, make_token extends_token, extends_list)

  and parse_classish_implements_opt parser =
    let (parser1, implements_token) = next_token parser in
    if (Token.kind implements_token) <> Implements then
      (parser, make_missing (), Syntax.make_missing ())
    else
    let (parser, implements_list) = parse_qualified_name_list parser1 in
    (parser, make_token implements_token, implements_list)

  and parse_qualified_name_list parser =
    let rec aux parser acc =
      let token = peek_token parser in
      match (Token.kind token) with
        | Comma ->
            let (parser1, token) = next_token parser in
            aux parser1 ((make_token token) :: acc)
        | Name
        | QualifiedName ->
            let (parser, classish_reference) = parse_type_specifier parser in
            aux parser (classish_reference :: acc)
        | _ -> (parser, acc)
    in
    let (parser, qualified_name_list) = aux parser [] in
    let qualified_name_list = List.rev qualified_name_list in
    (parser, make_list qualified_name_list)

  and parse_classish_body parser =
    let (parser, left_brace_token) = expect_left_brace parser in
    let (parser, classish_element_list) =
      parse_classish_element_list_opt parser in
    let (parser, right_brace_token) = expect_right_brace parser in
    let syntax = make_classish_body
      left_brace_token classish_element_list right_brace_token in
    (parser, syntax)

  and parse_classish_element_list_opt parser =
    let rec aux parser acc =
      let token = peek_token parser in
      match (Token.kind token) with
      | RightBrace
      | EndOfFile -> (parser, acc)
      | Use ->
          let (parser, classish_use) = parse_trait_use parser in
          aux parser (classish_use :: acc)
      | Abstract
      | Static
      | Public
      | Protected
      | Private
      | Final ->
        (* Parse methods. TODO: not only methods can start with these tokens *)
        let (parser, syntax) = parse_methodish parser (make_missing ()) in
        aux parser (syntax :: acc)
      | LessThanLessThan ->
        (* Parse methods. TODO: not only methods can start with these tokens *)
        let (parser, attr) = parse_attribute_specification_opt parser in
        let (parser, syntax) = parse_methodish parser attr in
        aux parser (syntax :: acc)
      | _ ->
          (* TODO *)
        let (parser, token) = next_token parser in
        let parser = with_error parser SyntaxError.error1033 in
        aux parser (make_error [make_token token] :: acc)
    in
    let (parser, classish_elements) = aux parser [] in
    let classish_elements = List.rev classish_elements in
    (parser, make_list classish_elements)


  (* SPEC:
    trait-use-clause:
      use  trait-name-list  ;

    trait-name-list:
      qualified-name  generic-type-parameter-listopt
      trait-name-list  ,  qualified-name  generic-type-parameter-listopt
  *)
  and parse_trait_use parser =
    let (parser, use_token) = assert_token parser Use in
    let (parser, trait_name_list) = parse_comma_list
      parser Semicolon SyntaxError.error1004 parse_trait_name in
    let (parser, semi) = expect_semicolon parser in
    (parser, make_trait_use use_token trait_name_list semi)

  and parse_trait_name parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Name
    | QualifiedName -> parse_possible_generic_specifier parser
    | _ ->
       let parser = with_error parser1 SyntaxError.error1004 in
       (parser, make_error [make_token token])

  (* SPEC:
    attribute_specification := << attribute_list >>
    attribute_list :=
      attribute
      attribute_list , attribute
    attribute := attribute_name attribute_value_list_opt
    attribute_name := name
    attribute_value_list := ( attribute_values_opt )
    attribute_values :=
      attribute_value
      attribute_values , attribute_value
    attribute_value := expression
   *)
  and parse_attribute_specification_opt parser =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = LessThanLessThan then
      let (parser, attr_list) = parse_attribute_list_opt parser1 in
      let (parser, right) = expect_right_double_angle parser in
      (parser, make_attribute_specification (make_token token) attr_list right)
    else
      (parser, make_missing())

  and parse_attribute_list_opt parser =
    let token = peek_token parser in
    if (Token.kind token) = GreaterThanGreaterThan then
      let parser = with_error parser SyntaxError.error1034 in
      (parser, make_missing())
    else
      (* TODO use Eric's generic comma list parse once it lands *)
      let rec aux parser acc =
        let parser, attr = parse_attribute parser in
        let parser1, token = next_token parser in
        match Token.kind token with
        | Comma ->
          let comma = make_token token in
          let item = make_list_item attr comma in
          aux parser1 (item :: acc)
        | GreaterThanGreaterThan ->
          let comma = make_missing () in
          let item = make_list_item attr comma in
          parser, make_list (List.rev (item :: acc))
        | _ ->
          (* ERROR RECOVERY: assume closing bracket is missing. Caller will
           * report an error. Do not eat token.
           * TODO better ways to recover *)
          parser, make_list (List.rev acc)
      in
      aux parser []

  and parse_attribute parser =
    let (parser, name) = expect_name parser in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | LeftParen ->
      let left = make_token token in
      let parser, values = parse_attribute_values_opt parser1 in
      let parser, right = expect_right_paren parser in
      parser, make_attribute name left values right
    | _ ->
      let left = make_missing () in
      let values = make_missing () in
      let right = make_missing () in
      parser, make_attribute name left values right

  and parse_attribute_values_opt parser =
    let token = peek_token parser in
    if (Token.kind token) = RightParen then
      (parser, make_missing())
    else
      (* TODO replace with generic comma list parsing *)
      let rec aux parser acc =
        let parser, expr = parse_expression parser in
        let parser1, token = next_token parser in
        match Token.kind token with
        | Comma ->
          let comma = make_token token in
          let item = make_list_item expr comma in
          aux parser1 (item :: acc)
        | RightParen ->
          let comma = make_missing () in
          let item = make_list_item expr comma in
          parser, make_list (List.rev (item :: acc))
        | _ ->
          (* ERROR RECOVERY: assume right paren is missing. Caller will
           * report an error. Do not eat token.
           * TODO better ways to recover *)
          parser, make_list (List.rev acc)
      in
      aux parser []

  and parse_generic_type_parameter_list_opt parser =
    let (parser1, open_angle) = next_token parser in
    if (Token.kind open_angle) = LessThan then
        let type_parser = TypeParser.make parser.lexer parser.errors in
        let (type_parser, node) =
          TypeParser.parse_generic_type_parameter_list type_parser in
        let lexer = TypeParser.lexer type_parser in
        let errors = TypeParser.errors type_parser in
        let parser = { lexer; errors } in
        (parser, node)
    else
      (parser, make_missing())

  and parse_return_type_hint_opt parser =
    let (parser1, colon_token) = next_token parser in
    if (Token.kind colon_token) = Colon then
      let (parser2, return_type) = parse_return_type parser1 in
      (parser2, make_token colon_token, return_type)
    else
      (parser, make_missing(), make_missing())

   (* SPEC
      parameter-list:
          ...
          parameter-declaration-list
          parameter-declaration-list  ,  ...

        parameter-declaration-list:
          parameter-declaration
          parameter-declaration-list  ,  parameter-declaration
  *)
  and parse_parameter_list parser =
    (* TODO: Update this to produce a sequence of list_item elements. *)
    let rec aux parser parameters =
      let (parser, parameter) = parse_parameter parser in
      let parameters = parameter :: parameters in
      let (parser1, token) = next_token parser in
      match (Token.kind token) with
      | Comma ->
        aux parser1 ((make_token token) :: parameters )
      | RightParen ->
        (parser, parameters)
      | EndOfFile
      | _ ->
        (* ERROR RECOVERY TODO: How to recover? *)
        let parser = with_error parser SyntaxError.error1009 in
        (parser, parameters) in
    let (parser, parameters) = aux parser [] in
    (parser, make_list (List.rev parameters))

  and parse_parameter_list_opt parser =
    let token = peek_token parser in
    if (Token.kind token) = RightParen then (parser, make_missing())
    else parse_parameter_list parser

  and parse_parameter parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | DotDotDot ->
      (parser1, make_token token)
    | Comma ->
      (* ERROR RECOVERY *)
      (parser, make_missing())
    | _ ->
      parse_parameter_declaration parser

  (* SPEC
    parameter-declaration:
      attribute-specificationopt  type-specifier  variable-name \
      default-argument-specifieropt
  *)
  and parse_parameter_declaration parser =
    (* ERROR RECOVERY
       In strict mode, we require a type specifier. This error is not caught
       at parse time but rather by a later pass. *)
    let (parser, attrs) = parse_attribute_specification_opt parser in
    let (parser, visibility) = parse_visibility_modifier_opt parser in
    let token = peek_token parser in
    let (parser, type_specifier) =
      if (Token.kind token) = Variable then (parser, make_missing())
      else parse_type_specifier parser in
    let (parser, variable_name) = expect_variable parser in
    let (parser, default) = parse_default_argument_specifier_opt parser in
    let syntax =
      make_parameter_declaration attrs visibility type_specifier variable_name
      default in
    (parser, syntax)

  and parse_visibility_modifier_opt parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Public | Protected | Private -> (parser1, make_token token)
    | _ -> (parser, make_missing())

  (* SPEC
            default-argument-specifier:
              =  const-expression
  *)
  and parse_default_argument_specifier_opt parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Equal ->
      (* TODO: Detect if expression is not const *)
      let (parser, default_value) = parse_expression parser1 in
      (parser, make_simple_initializer (make_token token) default_value)
    | _ -> (parser, make_missing())

  and parse_function_declaration parser attribute_specification =
    let (parser, header) =
      parse_function_declaration_header parser in
    let (parser, body) = parse_compound_statement parser in
    let syntax = make_function attribute_specification header body in
    (parser, syntax)

  and parse_function_declaration_header parser =
    (* ERROR RECOVERY
       In strict mode, we require a type specifier. This error is not caught
       at parse time but rather by a later pass. *)
    let (parser, async_token) = optional_token parser Async in
    let (parser, function_token) = expect_function parser in
    let (parser, label) =
      parse_function_label parser in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser in
    let (parser, left_paren_token) = expect_left_paren parser in
    let (parser, parameter_list) = parse_parameter_list_opt parser in
    let (parser, right_paren_token) = expect_right_paren parser in
    let (parser, colon_token, return_type) =
      parse_return_type_hint_opt parser in
    let syntax = make_function_header async_token
      function_token label generic_type_parameter_list left_paren_token
      parameter_list right_paren_token colon_token return_type in
    (parser, syntax)

  (* a function label is either a function name, a __construct label, or a
   * __destruct label *)
  and parse_function_label parser =
    let parser, token = next_token parser in
    match Token.kind token with
    | Name | Construct | Destruct -> (parser, make_token token)
    | _ ->
      (* ERRPR RECOVERY *)
      let parser = with_error parser SyntaxError.error1044 in
      let error = make_error [make_token token] in
      (parser, error)
  (* SPEC
      method-declaration:
        attribute-spec-opt method-modifiers function-definition
        attribute-spec-opt method-modifiers function-definition-header ;
      method-modifiers:
        method-modifier
        method-modifiers method-modifier
      method-modifier:
        visibility-modifier (i.e. private, public, protected)
        static
        abstract
        final
   *)

  and parse_methodish parser attribute_spec =
    let (parser, modifiers) = parse_methodish_modifiers parser in
    let (parser, header) = parse_function_declaration_header parser in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | LeftBrace ->
      let (parser, body) = parse_compound_statement parser in
      let syntax =
        make_methodish attribute_spec modifiers header body (make_missing ())in
      (parser, syntax)
    | Semicolon ->
      let semicolon = make_token token in
      let syntax =
        make_methodish attribute_spec modifiers header (make_missing())
        semicolon in
      (parser1, syntax)
    | _ ->
      (* ERROR RECOVERY: skip to the next token *)
      let parser = with_error parser1 SyntaxError.error1041 in
      (parser, make_error [make_token token])

  (* TODO it is unclear what the modifier requirements are. Specifically, is it
   * required for constructors and destructors to have a visibility modifier?
   * Does the order of the modifiers matter? *)
  and parse_methodish_modifiers parser =
    let rec aux acc parser =
      (* In reality some modifiers cannot occur together, check this in a later
       * pass *)
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | EndOfFile ->
        (* ERROR RECOVERY it is likely that the function header is missing *)
        let parser = with_error parser SyntaxError.error1043 in
        (parser, make_list (List.rev acc))
      | Abstract
      | Static
      | Public
      | Protected
      | Private
      | Final ->
        let modifier = make_token token in
        aux (modifier :: acc) parser1
      | _ ->
        (* Not a modifier, end parsing modifiers *)
        (parser, make_list (List.rev acc))
    in
    aux [] parser

  and parse_classish_or_function_declaration parser =
    let parser, attribute_specification =
      parse_attribute_specification_opt parser in
    let parser1, token = next_token parser in
    match Token.kind token with
    | Async | Function ->
      parse_function_declaration parser attribute_specification
    | Class -> parse_classish_declaration parser attribute_specification
    | _ ->
      (* TODO *)
      (parser1, make_error [make_token token])

  and parse_declaration parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Require
    | Require_once -> parse_inclusion_directive parser
    | Type
    | Newtype -> parse_alias_declaration parser
    | Enum -> parse_enum_declaration parser
    | Namespace -> parse_namespace_declaration parser
    | Use -> parse_namespace_use_declaration parser
    | Trait
    | Interface
    | Abstract
    | Final
    | Class -> parse_classish_declaration parser(make_missing())
    | Async
    | Function -> parse_function_declaration parser (make_missing())
    | LessThanLessThan ->
      parse_classish_or_function_declaration parser
    | _ ->
      (* ERROR RECOVERY: Skip the token, try again. *)
      (* TODO: Better policy would be to skip ahead to
         the first token that makes a legal declaration. *)
      let parser = with_error parser1 SyntaxError.error1002 in
      (parser, make_error [make_token token])

  and parse_declarations parser expect_brace =
    let rec aux parser declarations =
      let token = peek_token parser in
      match (Token.kind token) with
      | EndOfFile -> (parser, declarations)
      | RightBrace when expect_brace ->
        (parser, declarations)
      (* TODO: ?> tokens *)
      | _ ->
        let (parser, declaration) = parse_declaration parser in
        aux parser (declaration :: declarations) in
    let (parser, declarations) = aux parser [] in
    let syntax = make_list (List.rev declarations) in
    (parser, syntax)

  let parse_script_header parser =
    (* TODO: Detect if there is trivia before or after any token. *)
    let (parser1, less_than) = next_token parser in
    let (parser2, question) = next_token parser1 in
    let (parser3, language) = next_token parser2 in
    let valid = (Token.kind less_than) == LessThan &&
                (Token.kind question) == Question &&
                (Token.kind language) == Name in
    if valid then
      let less_than = make_token less_than in
      let question = make_token question in
      let language = make_token language in
      let script_header = make_script_header less_than question language in
      (parser3, script_header)
    else
      (* TODO: Report an error *)
      (* ERROR RECOVERY *)
      (* Make no progress; try parsing the file without a header *)
      let parser = with_error parser SyntaxError.error1001 in
      let less_than = make_token (Token.make LessThan 0 [] []) in
      let question = make_token (Token.make Question 0 [] []) in
      let language = make_token (Token.make Name 0 [] []) in
      let script_header = make_script_header less_than question language in
      (parser, script_header )

  let parse_script parser =
    let (parser, script_header) = parse_script_header parser in
    let (parser, declarations) = parse_declarations parser false in
    (* TODO: ERROR_RECOVERY:
      If we are not at the end of the file, something is wrong. *)
    (parser, make_script script_header declarations)


end
