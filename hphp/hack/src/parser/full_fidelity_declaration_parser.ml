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
module Operator = Full_fidelity_operator
module SimpleParser = Full_fidelity_simple_parser.WithLexer(Full_fidelity_lexer)

open TokenKind
open Full_fidelity_minimal_syntax

module WithExpressionAndStatementAndTypeParser
  (ExpressionParser : Full_fidelity_expression_parser_type.ExpressionParserType)
  (StatementParser : Full_fidelity_statement_parser_type.StatementParserType)
  (TypeParser : Full_fidelity_type_parser_type.TypeParserType) :
  Full_fidelity_declaration_parser_type.DeclarationParserType = struct

  include SimpleParser
  include Full_fidelity_parser_helpers.WithParser(SimpleParser)

  (* Types *)

  let parse_in_type_parser parser type_parser_function =
    let type_parser = TypeParser.make parser.lexer
      parser.errors parser.context in
    let (type_parser, node) = type_parser_function type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_generic_parameter_list_opt parser =
    parse_in_type_parser parser TypeParser.parse_generic_parameter_list_opt

  let parse_possible_generic_specifier parser =
    parse_in_type_parser parser TypeParser.parse_possible_generic_specifier

  let parse_type_specifier ?(allow_var=false) parser =
    parse_in_type_parser parser (TypeParser.parse_type_specifier ~allow_var)

  let parse_return_type parser =
    parse_in_type_parser parser TypeParser.parse_return_type

  let parse_type_constraint_opt parser =
    parse_in_type_parser parser TypeParser.parse_type_constraint_opt

  (* Expressions *)
  let parse_in_expression_parser parser expression_parser_function =
    let expr_parser = ExpressionParser.make parser.lexer
      parser.errors parser.context in
    let (expr_parser, node) = expression_parser_function expr_parser in
    let lexer = ExpressionParser.lexer expr_parser in
    let errors = ExpressionParser.errors expr_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_expression parser =
    parse_in_expression_parser parser ExpressionParser.parse_expression

  (* Statements *)
  let parse_in_statement_parser parser statement_parser_function =
    let statement_parser = StatementParser.make parser.lexer
      parser.errors parser.context in
    let (statement_parser, node) = statement_parser_function
       statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_compound_statement parser =
    parse_in_statement_parser parser StatementParser.parse_compound_statement

  let parse_statement parser =
    parse_in_statement_parser parser StatementParser.parse_statement

  (* Declarations *)

  let rec parse_inclusion_directive parser =
  (* SPEC:
    inclusion-directive:
      require-multiple-directive
      require-once-directive

    require-multiple-directive:
      require  include-filename  ;

    include-filename:
      expression

    require-once-directive:
      require_once  include-filename  ;

    In non-strict mode we allow an inclusion directive (without semi) to be
    used as an expression. It is therefore easier to actually parse this as:

    inclusion-directive:
      inclusion-expression  ;

    inclusion-expression:
      require include-filename
      require_once include-filename
    *)
    let (parser, expr) = parse_expression parser in
    let (parser, semi) = require_semicolon parser in
    let result = make_inclusion_directive expr semi in
    (parser, result)

  and parse_alias_declaration parser attr =
    (* SPEC
      alias-declaration:
        attribute-spec-opt type  name
          generic-type-parameter-list-opt  =  type-specifier  ;
        attribute-spec-opt newtype  name
          generic-type-parameter-list-opt type-constraint-opt
            =  type-specifier  ;
    *)

    let (parser, token) = next_token parser in
    let token = make_token token in
    (* Not `require_name` but `require_name_allow_keywords`, because the parser
     * must allow keywords in the place of identifiers; at least to parse .hhi
     * files.
     *)
    let (parser, name) = require_name_allow_keywords parser in
    let (parser, generic) = parse_generic_parameter_list_opt parser in
    let (parser, constr) = parse_type_constraint_opt parser in
    let (parser, equal) = require_equal parser in
    let (parser, ty) = parse_type_specifier parser in
    let (parser, semi) = require_semicolon parser in
    let result = make_alias_declaration
      attr token name generic constr equal ty semi in
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

    (* TODO: We must allow TRUE to be a legal enum member name; here we allow
      any keyword.  Consider making this more strict. *)
    let (parser, name) = require_name_allow_keywords parser in
    let (parser, equal) = require_equal parser in
    let (parser, value) = parse_expression parser in
    let (parser, semicolon) = require_semicolon parser in
    let result = make_enumerator name equal value semicolon in
    (parser, result)

  and parse_enumerator_list_opt parser =
    (* SPEC
      enumerator-list:
        enumerator
        enumerator-list   enumerator
    *)
    parse_terminated_list parser parse_enumerator RightBrace

  and parse_enum_declaration parser attrs =
    (*
    enum-declaration:
      attribute-specification-opt enum  name  enum-base  type-constraint-opt /
        {  enumerator-list-opt  }
    enum-base:
      :  int
      :  string
    *)
    (* TODO: SPEC ERROR: The spec states that the only legal enum types
    are "int" and "string", but Hack allows any type, and apparently
    some of those are meaningful and desired.  Figure out what types
    are actually legal and illegal as enum base types; put them in the
    spec, and add an error pass that says when they are wrong. *)
    let (parser, enum) = assert_token parser Enum in
    let (parser, name) = require_name parser in
    let (parser, colon) = require_colon parser in
    let (parser, base) = parse_type_specifier parser in
    let (parser, enum_type) = parse_type_constraint_opt parser in
    let (parser, left_brace, enumerators, right_brace) = parse_braced_list
      parser parse_enumerator_list_opt in
    let result = make_enum_declaration
      attrs enum name colon base enum_type left_brace enumerators right_brace in
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
    let result = make_namespace_declaration namespace_token name body in
    (parser, result)

  and parse_namespace_body parser =
    match peek_token_kind parser with
    | Semicolon ->
      let (parser, token) = next_token parser in
      (parser, make_namespace_empty_body (make_token token))
    | LeftBrace ->
      let (parser, token) = next_token parser in
      let left = make_token token in
      let (parser, body) =
        parse_terminated_list parser parse_declaration RightBrace in
      let (parser, right) = require_right_brace parser in
      let result = make_namespace_body left body right in
      (parser, result)
    | _ ->
      (* ERROR RECOVERY: return an inert namespace (one with all of its
       * components 'missing'), and recover--without advancing the parser--
       * back to the level that the namespace was declared in. *)
      let parser = with_error parser SyntaxError.error1038 in
      let missing = make_missing() in
      let result = make_namespace_body missing missing missing in
      (parser, result)

  and parse_namespace_use_kind_opt parser =
    (* SPEC
      namespace-use-kind:
        namespace
        function
        const *)
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Type
    | Namespace
    | Function
    | Const -> (parser1, (make_token token))
    | _ -> (parser, (make_missing()))

  and parse_namespace_use_clause parser =
    (* SPEC
      namespace-use-clause:
        qualified-name  namespace-aliasing-clauseopt
      namespace-use-kind-clause:
        namespace-use-kind-opt qualified-name  namespace-aliasing-clauseopt
      namespace-aliasing-clause:
        as  name
    *)
    let (parser, use_kind) = parse_namespace_use_kind_opt parser in
    let (parser, name) = require_qualified_name parser in
    let (parser1, as_token) = next_token parser in
    let (parser, as_token, alias) =
      if Token.kind as_token = As then
        let as_token = make_token as_token in
        let (parser, alias) = require_name parser1 in
        (parser, as_token, alias)
      else
        (parser, (make_missing()), (make_missing())) in
    let result = make_namespace_use_clause use_kind name as_token alias in
    (parser, result)

  and is_group_use parser =
    (* We want a heuristic to determine whether to parse the use clause as
    a group use or normal use clause.  We distinguish the two by (1) whether
    there is a namespace prefix -- in this case it is definitely a group use
    clause -- or, if there is a name followed by a curly. That's illegal, but
    we should give an informative error message about that. *)
    let (parser, _) = assert_token parser Use in
    let (parser, _) = parse_namespace_use_kind_opt parser in
    let (parser, token) = next_token parser in
    match Token.kind token with
    | NamespacePrefix -> true
    | Name
    | QualifiedName ->
      peek_token_kind parser = LeftBrace
    | _ -> false

  and parse_group_use parser =
    (* See below for grammar. *)
    let (parser, use_token) = assert_token parser Use in
    let (parser, use_kind) = parse_namespace_use_kind_opt parser in
    (* We already know that this is a name, qualified name, or prefix. *)
    (* TODO: Give an error in a later pass if it is not a prefix. *)
    let (parser, prefix) = next_token parser in
    let prefix = make_token prefix in
    let (parser, left, clauses, right) =
      parse_braced_comma_list_opt_allow_trailing
      parser parse_namespace_use_clause in
    let (parser, semi) = require_semicolon parser in
    let result = make_namespace_group_use_declaration
      use_token use_kind prefix left clauses right semi in
    (parser, result)

  and parse_namespace_use_declaration parser =
    (* SPEC
    namespace-use-declaration:
      use namespace-use-kind-opt namespace-use-clauses  ;
      use namespace-use-kind namespace-name-as-a-prefix
        { namespace-use-clauses }  ;
      use namespace-name-as-a-prefix { namespace-use-kind-clauses  }  ;

      TODO: Add the grammar for the namespace-use-clauses; ensure that it
      indicates that trailing commas are allowed in the list.
    *)
    (* TODO: ERROR RECOVERY
    In the "simple" format, the kind may only be specified up front.
    In the "group" format, if the kind is specified up front then it may not
    be specified in each clause.
    We do not enforce this rule here. Rather, we allow the kind to be anywhere,
    and we'll add an error reporting pass later that deduces violations. *)
    if is_group_use parser then
      parse_group_use parser
    else
      let (parser, use_token) = assert_token parser Use in
      let (parser, use_kind) = parse_namespace_use_kind_opt parser in
      let (parser, clauses) = parse_comma_list
        parser Semicolon SyntaxError.error1004 parse_namespace_use_clause in
      let (parser, semi) = require_semicolon parser in
      let result = make_namespace_use_declaration
        use_token use_kind clauses semi in
      (parser, result)

  and parse_classish_declaration parser attribute_spec =
    let (parser, modifiers) =
      parse_classish_modifiers parser in
    let (parser, token) =
      parse_classish_token parser in
    let (parser, name) = require_class_name parser in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser in
    let (parser, classish_extends, classish_extends_list) =
      parse_classish_extends_opt parser in
    let (parser, classish_implements, classish_implements_list) =
      parse_classish_implements_opt parser in
    let (parser, body) = parse_classish_body parser in
    let syntax = make_classish_declaration
      attribute_spec modifiers token name generic_type_parameter_list
      classish_extends classish_extends_list classish_implements
      classish_implements_list
      body
    in
    (parser, syntax)

  and parse_classish_modifiers parser =
    let rec parse_classish_modifier_opt parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
        | Abstract
        | Final ->
          let acc = (make_token token)::acc in
          parse_classish_modifier_opt parser1 acc
        | _ -> (parser, make_list (List.rev acc))
    in
    parse_classish_modifier_opt parser []

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
      (parser, make_missing (), make_missing ())
    else
    let (parser, extends_list) = parse_special_type_list parser1 in
    (parser, make_token extends_token, extends_list)

  and parse_classish_implements_opt parser =
    let (parser1, implements_token) = next_token parser in
    if (Token.kind implements_token) <> Implements then
      (parser, make_missing (), make_missing ())
    else
    let (parser, implements_list) = parse_special_type_list parser1 in
    (parser, make_token implements_token, implements_list)

  and parse_special_type parser =
    let (parser1, token) = next_xhp_class_name_or_other parser in
    match (Token.kind token) with
    | Comma ->
      (* ERROR RECOVERY. We expected a type but we got a comma.
      Give the error that we expected a type, not a name, even though
      not every type is legal here. *)
      let parser = with_error parser1 SyntaxError.error1007 in
      let item = make_missing() in
      let comma = make_token token in
      let list_item = make_list_item item comma in
      (parser, list_item)
    | Name
    | XHPClassName
    | QualifiedName ->
      let (parser, item) = parse_type_specifier parser in
      let (parser, comma) = optional_token parser Comma in
      let list_item = make_list_item item comma in
      (parser, list_item)
    | _ ->
      (* ERROR RECOVERY: We are expecting a type; give an error as above.
      Don't eat the offending token.
      *)
      let parser = with_error parser SyntaxError.error1007 in
      let list_item = make_list_item (make_missing()) (make_missing()) in
      (parser, list_item)

  and parse_special_type_list parser =
    (*
      An extends / implements list is a comma-separated list of types, but
      very special types; we want the types to consist of a name and an
      optional generic type argument list.

      TODO: Can the type name be of the form "foo::bar"? Those do not
      necessarily start with names. Investigate this.

      Normally we'd use one of the separated list helpers, but there is no
      specific end token we could use to detect the end of the list, and we
      want to bail out if we get something that is not a type of the right form.
      So we have custom logic here.

      TODO: This is one of the rare cases in Hack where a comma-separated list
      may not have a trailing comma. Is that desirable, or was that an
      oversight when the trailing comma rules were added?  If possible we
      should keep the rule as-is, and disallow the trailing comma; it makes
      parsing and error recovery easier.
    *)
    parse_list_until_no_comma parser parse_special_type

  and parse_classish_body parser =
    let (parser, left_brace_token) = require_left_brace parser in
    let (parser, classish_element_list) =
      parse_classish_element_list_opt parser in
    let (parser, right_brace_token) = require_right_brace parser in
    let syntax = make_classish_body
      left_brace_token classish_element_list right_brace_token in
    (parser, syntax)

  and parse_classish_element parser =
  (*We need to identify an element of a class, trait, etc. Possibilities
    are:

     // constant-declaration:
     const T $x = v ;
     abstract const T $x ;

     // type-constant-declaration
     const type T = X;
     abstract const type T;

     // property-declaration:
     public/private/protected/static T $x;
     TODO: We may wish to parse "T $x" and give an error indicating
     TODO: that we were expecting either const or public.
     Note that a visibility modifier is required; static is optional;
     any order is allowed.
     TODO: The spec indicates that abstract is disallowed, but Hack allows
     TODO: it; resolve this disagreement.

     // method-declaration
     <<attr>> public/private/protected/abstract/final/static async function
     Note that a modifier is required, the attr and async are optional.
     TODO: Hack requires a visibility modifier, unless "static" is supplied,
     TODO: in which case the method is considered to be public.  Is this
     TODO: desired? Resolve this disagreement with the spec.

     // constructor-declaration
     <<attr>> public/private/protected/abstract/final function __construct
     TODO: Hack allows static constructors and requires a visibility modifier,
     TODO: as with regular methods. Resolve this disagreement with the spec.

     // destructor-declaration
     <<attr>> public/private/protected function __destruct
     TODO: Hack allows static, final and abstract destructors
     TODO: as with regular methods. Resolve this disagreement with the spec.

     // trait clauses
    require  extends  qualified-name
    require  implements  qualified-name

    // XHP class attribute declaration
    attribute ... ;

    // XHP category declaration
    category ... ;

    // XHP children declaration
    children ... ;

  *)
    let token = peek_token parser in
    match (Token.kind token) with
    | Children -> parse_xhp_children_declaration parser
    | Category -> parse_xhp_category_declaration parser
    | Use -> parse_trait_use parser
    | Const -> parse_const_or_type_const_declaration parser (make_missing ())
    | Abstract -> parse_methodish_or_const_or_type_const parser
    | Async
    | Static
    | Public
    | Protected
    | Private
    | Final ->
      (* Parse methods, constructors, destructors or properties.
      TODO: const can also start with these tokens *)
      parse_methodish_or_property parser (make_missing())
    | LessThanLessThan ->
      (* Parse "methodish" declarations: methods, ctors and dtors *)
      (* TODO: Consider whether properties ought to allow attributes. *)
      let (parser, attr) = parse_attribute_specification_opt parser in
      let (parser, modifiers) = parse_modifiers parser in
      parse_methodish parser attr modifiers
    | Require ->
      (* We give an error if these are found where they should not be,
         in a later pass. *)
      parse_require_clause parser
    | TokenKind.Attribute -> parse_xhp_class_attribute_declaration parser
    | Function ->
      (* ERROR RECOVERY
      Hack requires that a function inside a class be marked
      with a visibility modifier, but PHP does not have this requirement.
      TODO: Add an error in a later pass for Hack files. *)
      parse_methodish parser (make_missing()) (make_missing())
    | Var ->
      (* TODO: We allow "var" as a synonym for "public" in a property; this
      is a PHP-ism that we do not support in Hack, but we parse anyways
      so as to give an error later.  Write an error detection pass. *)
      let (parser, var) = assert_token parser Var in
      parse_property_declaration parser var
    | kind when SimpleParser.expects parser kind -> (parser, make_missing())
    | _ ->
        (* TODO ERROR RECOVERY could be improved here. *)
      let (parser, token) = next_token parser in
      let parser = with_error parser SyntaxError.error1033 in
      let result = make_error (make_token token) in
      (parser, result)

  and parse_classish_element_list_opt parser =
    (* TODO: ERROR RECOVERY: consider bailing if the token cannot possibly
             start a classish element. *)
    (* ERROR RECOVERY: we're in the body of a classish, so we add visibility
     * modifiers to our context. *)
    let expected_tokens = [Public; Protected; Private] in
    let parser = SimpleParser.expect_in_new_scope parser expected_tokens in
    let (parser, element_list) =
      parse_terminated_list parser parse_classish_element RightBrace in
    let parser = SimpleParser.pop_scope parser expected_tokens in
    (parser, element_list)

  and parse_xhp_children_paren parser =
    (* SPEC (Draft)
    ( xhp-children-expressions )

    xhp-children-expressions:
      xhp-children-expression
      xhp-children-expressions , xhp-children-expression

    TODO: The parenthesized list of children expressions is NOT allowed
    to be comma-terminated. Is this intentional? It is inconsistent with
    practice throughout the rest of Hack. There is no syntactic difficulty
    in allowing a comma before the close paren. Consider allowing it.
    *)
    let (parser, left, exprs, right) =
      parse_parenthesized_comma_list parser parse_xhp_children_expression in
    let result = make_xhp_children_parenthesized_list left exprs right in
    (parser, result)

  and parse_xhp_children_term parser =
    (* SPEC (Draft)
    xhp-children-term:
      ( xhp-children-expressions ) trailing-opt
      name trailing-opt
      xhp-class-name trailing-opt
      xhp-category-name trailing-opt
    trailing: * ? +

    Note that there may be only zero or one trailing unary operator.
    "foo*?" is not a legal xhp child expression.
    *)
    let (parser1, token) = next_xhp_children_name_or_other parser in
    let name = make_token token in
    match Token.kind token with
    | Name
    | XHPClassName
    | XHPCategoryName -> parse_xhp_children_trailing parser1 name
    | LeftParen ->
      let (parser, term) = parse_xhp_children_paren parser in
      parse_xhp_children_trailing parser term
    | _ ->
      (* ERROR RECOVERY: Eat the offending token, keep going. *)
      (with_error parser SyntaxError.error1053, name)

  and parse_xhp_children_trailing parser term =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Star
    | Plus
    | Question ->
      let result = make_postfix_unary_expression term (make_token token) in
      (parser1, result)
    | _ -> (parser, term)

  and parse_xhp_children_bar parser left =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Bar ->
      let (parser, right) = parse_xhp_children_term parser1 in
      let result = make_binary_expression left (make_token token) right in
      parse_xhp_children_bar parser result
    | _ -> (parser, left)

  and parse_xhp_children_expression parser =
    (* SPEC (Draft)
    xhp-children-expression:
      xhp-children-term
      xhp-children-expression | xhp-children-term

    Note that the bar operator is left-associative. (Not that it matters
    semantically. *)
    let (parser, term) = parse_xhp_children_term parser in
    parse_xhp_children_bar parser term

  and parse_xhp_children_declaration parser =
    (* SPEC (Draft)
    xhp-children-declaration:
      children empty ;
      children xhp-children-expression ;
    *)
    let (parser, children) = assert_token parser Children in
    let (parser, expr) = if peek_token_kind parser = Empty then
      assert_token parser Empty
    else
      parse_xhp_children_expression parser in
    let (parser, semi) = require_semicolon parser in
    let result = make_xhp_children_declaration children expr semi in
    (parser, result)

  and parse_xhp_category parser =
    let (parser, token) = next_xhp_category_name parser in
    let category = make_token token in
    match Token.kind token with
    | XHPCategoryName -> (parser, category)
    | _ -> (with_error parser SyntaxError.error1052, category)

  and parse_xhp_category_declaration parser =
    (* SPEC (Draft)
    xhp-category-declaration:
      category xhp-category-list ,-opt  ;

    xhp-category-list:
      xhp-category-name
      xhp-category-list  ,  xhp-category-name
    *)
    let (parser, category) = assert_token parser Category in
    let (parser, items) = parse_comma_list_allow_trailing parser Semicolon
      SyntaxError.error1052 parse_xhp_category in
    let (parser, semi) = require_semicolon parser in
    let result = make_xhp_category_declaration category items semi in
    (parser, result)

  and parse_xhp_type_specifier parser =
    (* SPEC (Draft)
      xhp-type-specifier:
        enum { xhp-attribute-enum-list  ,-opt  }
        type-specifier

      The list of enum values must have at least one value and can be
      comma-terminated.

      xhp-enum-list:
        xhp-attribute-enum-value
        xhp-enum-list , xhp-attribute-enum-value

      xhp-attribute-enum-value:
        any integer literal
        any single-quoted-string literal
        any double-quoted-string literal

      TODO: What are the semantics of encapsulated expressions in double-quoted
            string literals here?
      ERROR RECOVERY: We parse any expressions here;
      TODO: give an error in a later pass if the expressions are not literals.

      TODO: We allow an empty list of enums; add an error for that.
    *)
    if peek_token_kind parser = Enum then
      let (parser, enum_token) = assert_token parser Enum in
      let (parser, left_brace, values, right_brace) =
        parse_braced_comma_list_opt_allow_trailing
        parser parse_expression in
      let result =
        make_xhp_enum_type enum_token left_brace values right_brace in
      (parser, result)
    else
      parse_type_specifier ~allow_var:true parser

  and parse_xhp_required_opt parser =
    (* SPEC (Draft)
      xhp-required :
        @  required

      Note that these are two tokens. They can have whitespace between them. *)
    if peek_token_kind parser = At then
      let (parser, at) = assert_token parser At in
      let (parser, req) = require_required parser in
      let result = make_xhp_required at req in
      (parser, result)
    else
      (parser, (make_missing()))

  and parse_xhp_class_attribute_typed parser =
    (* xhp-type-specifier xhp-name initializer-opt xhp-required-opt *)
    let (parser, ty) = parse_xhp_type_specifier parser in
    let (parser, name) = require_xhp_name parser in
    let (parser, init) = parse_simple_initializer_opt parser in
    let (parser, req) = parse_xhp_required_opt parser in
    let result = make_xhp_class_attribute ty name init req in
    (parser, result)

  and parse_xhp_class_attribute parser =
    (* SPEC (Draft)
    xhp-attribute-declaration:
      xhp-class-name
      xhp-type-specifier xhp-name initializer-opt xhp-required-opt

    ERROR RECOVERY:
    The xhp type specifier could be an xhp class name. To disambiguate we peek
    ahead a token; if it's a comma or semi, we're done. If not, then we assume
    that we are in the more complex case.
    *)
    if is_next_xhp_class_name parser then
      let (parser1, class_name) = require_class_name parser in
      match peek_token_kind parser1 with
      | Comma
      | Semicolon ->
        let type_specifier = make_simple_type_specifier class_name in
        let result = make_xhp_simple_class_attribute type_specifier in
        (parser1, result)
      | _ -> parse_xhp_class_attribute_typed parser
    else
      parse_xhp_class_attribute_typed parser

  and parse_xhp_class_attribute_declaration parser =
    (* SPEC: (Draft)
    xhp-class-attribute-declaration :
      attribute xhp-attribute-declaration-list ;

    xhp-attribute-declaration-list:
      xhp-attribute-declaration
      xhp-attribute-declaration-list , xhp-attribute-declaration

    TODO: The list of attributes may NOT be terminated with a trailing comma
    before the semicolon. This is inconsistent with the rest of Hack.
    Allowing a comma before the semi does not introduce any syntactic
    difficulty; consider allowing it.
    *)
    let (parser, attr_token) = assert_token parser TokenKind.Attribute in
    (* TODO: Better error message. *)
    let (parser, attrs) = parse_comma_list parser Semicolon
      SyntaxError.error1004 parse_xhp_class_attribute in
    let (parser, semi) = require_semicolon parser in
    let result = make_xhp_class_attribute_declaration attr_token attrs semi in
    (parser, result)

  and parse_qualified_name_type parser =
    (* Here we're parsing a name followed by an optional generic type
       argument list; if we don't have a name, give an error. *)
    match peek_token_kind parser with
    | Name
    | QualifiedName -> parse_possible_generic_specifier parser
    | _ -> require_qualified_name parser

  and parse_qualified_name_type_opt parser =
    (* Here we're parsing a name followed by an optional generic type
       argument list; if we don't have a name, give an error. *)
    match peek_token_kind parser with
    | Name
    | QualifiedName -> parse_possible_generic_specifier parser
    | _ -> parser, make_missing ()

  and parse_require_clause parser =
    (* SPEC
        require-extends-clause:
          require  extends  qualified-name  ;

        require-implements-clause:
          require  implements  qualified-name  ;
    *)
    (* We must also parse "require extends :foo;" *)
    (* TODO: What about "require extends :foo<int>;" ? *)
    (* TODO: The spec is incomplete; we need to be able to parse
       require extends Foo<int>;
       Fix the spec.
       TODO: Check whether we also need to handle
         require extends foo::bar
       and so on.
       *)
    (* ERROR RECOVERY: Detect if the implements/extends, name and semi are
       missing. *)
    let (parser, req) = assert_token parser Require in
    let (parser1, req_kind_token) = next_token parser in
    let (parser, req_kind) = match Token.kind req_kind_token with
    | Implements
    | Extends -> (parser1, make_token req_kind_token)
    | _ -> (with_error parser SyntaxError.error1045, make_missing()) in
    let (parser, name) = if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      (parser, make_token token)
    else
      parse_qualified_name_type parser in
    let (parser, semi) = require_semicolon parser in
    let result = make_require_clause req req_kind name semi in
    (parser, result)

  and parse_methodish_or_property parser attribute_spec =
    (* If there is an attribute then it cannot be a property. *)
    (* TODO: ERROR RECOVERY: Consider whether a property with an attribute
       TODO: ought to be (1) parsed, with an error, or (2) perhaps
       TODO: simply make it legal? A property seems like something that could
       TODO: reasonably have an attribute. *)
    let (parser, modifiers) = parse_modifiers parser in
    if is_missing attribute_spec then
      (* ERROR RECOVERY: match against two tokens, because if one token is
       * in error but the next isn't, then it's likely that the user is
       * simply still typing. Throw an error on what's being typed, then eat
       * it and keep going. *)
      let current_token_kind = peek_token_kind parser in
      let next_token = peek_token ~lookahead:1 parser in
      let next_token_kind = Token.kind next_token in
      match current_token_kind, next_token_kind with
      (* Detected the usual start to a method, so continue parsing as method. *)
      | (Async | Coroutine | Function) , _ ->
        parse_methodish parser attribute_spec modifiers
      | LeftParen, _ ->
        parse_property_declaration parser modifiers
      (* We encountered one unexpected token, but the next still indicates that
       * we should be parsing a methodish. Throw an error, process the token
       * as an extra, and keep going. *)
      | _, (Async | Coroutine | Function)
        when not (Token.has_leading_end_of_line next_token) ->
        let parser = with_error parser SyntaxError.error1056
          ~on_whole_token:true in
        let parser = skip_and_log_unexpected_token parser
          ~generate_error:false in
        parse_methodish parser attribute_spec modifiers
      (* Otherwise, continue parsing as a property (which might be a lambda). *)
      | ( _ , _ ) -> parse_property_declaration parser modifiers
    else
      parse_methodish parser attribute_spec modifiers

  and parse_trait_use_precendence_item parser name =
    let (parser, keyword) = assert_token parser Insteadof in
    let (parser, removed_names) =
      parse_trait_name_list
        parser
        (function Semicolon -> true | _ -> false)
    in
    let trait_use_precendence_item =
      make_trait_use_precedence_item
        name
        keyword
        removed_names
    in
    (parser, trait_use_precendence_item)

  and parse_trait_use_alias_item parser aliasing_name =
    let (parser, keyword) = assert_token parser As in
    let (parser, visibility) = parse_visibility_modifier_opt parser in
    let (parser, aliased_name) = parse_qualified_name_type_opt parser in
    let trait_use_alias_item =
      make_trait_use_alias_item
        aliasing_name
        keyword
        visibility
        aliased_name
    in
    (parser, trait_use_alias_item)

  and parse_trait_use_conflict_resolution_item parser =
    let (parser, qualifier) = parse_qualified_name_type parser in
    let (parser, name) =
      if (peek_token_kind parser) = ColonColon then
        (* scope resolution expression case *)
        let (parser, cc_token) = require_coloncolon parser in
        let (parser, name) = require_name parser in
        (parser, make_scope_resolution_expression qualifier cc_token name)
      else
        (* plain qualified name case *)
        (parser, qualifier)
    in
    if (peek_token_kind parser) = As then
      parse_trait_use_alias_item parser name
    else
      parse_trait_use_precendence_item parser name

  (*  SPEC:
    trait-use-conflict-resolution:
      use trait-name-list  {  trait-use-conflict-resolution-list  }

    trait-use-conflict-resolution-list:
      trait-use-conflict-resolution-item
      trait-use-conflict-resolution-item  trait-use-conflict-resolution-list

    trait-use-conflict-resolution-item:
      trait-use-conflict-resolution-item-name  as  name;
      trait-use-conflict-resolution-item-name  as  visibility-modifier  name;
      trait-use-conflict-resolution-item-name  as  visibility-modifier;
      scope-resolution-expression  insteadof  trait-name-list

    trait-use-conflict-resolution-item-name:
      qualified-name
      scope-resolution-expression
  *)
  and parse_trait_use_conflict_resolution parser use_token trait_name_list =
    let (parser, left_brace) = assert_token parser LeftBrace in
    let (parser, clauses) =
      parse_separated_list_opt
        parser
        Semicolon
        TrailingAllowed
        RightBrace
        SyntaxError.error1004
        parse_trait_use_conflict_resolution_item
    in
    let (parser, right_brace) = assert_token parser RightBrace in
    let trait_use_conflict_resolution =
      make_trait_use_conflict_resolution
        use_token
        trait_name_list
        left_brace
        clauses
        right_brace
    in
    (parser, trait_use_conflict_resolution)

  (* SPEC:
    trait-use-clause:
      use  trait-name-list  ;

    trait-name-list:
      qualified-name  generic-type-parameter-listopt
      trait-name-list  ,  qualified-name  generic-type-parameter-listopt
  *)
  and parse_trait_name_list parser predicate =
    parse_separated_list_predicate
      parser
      Comma
      NoTrailing
      predicate
      SyntaxError.error1004
      parse_qualified_name_type

  and parse_trait_use parser =
    let (parser, use_token) = assert_token parser Use in
    let (parser, trait_name_list) =
      parse_trait_name_list
        parser
        (function Semicolon | LeftBrace -> true | _ -> false)
    in
    if (peek_token_kind parser) = LeftBrace then
      parse_trait_use_conflict_resolution parser use_token trait_name_list
    else
      let (parser, semi) = require_semicolon parser in
      (parser, make_trait_use use_token trait_name_list semi)

  and parse_const_or_type_const_declaration parser abstr =
    let (parser, const) = assert_token parser Const in
    if (peek_token_kind parser) = Type
       && (peek_token_kind ~lookahead:1 parser <> Equal) then
      parse_type_const_declaration parser abstr const
    else
      parse_const_declaration parser abstr const

  and parse_property_declaration parser modifiers =
    (* SPEC:
        property-declaration:
          property-modifier  type-specifier  property-declarator-list  ;

       property-declarator-list:
         property-declarator
         property-declarator-list  ,  property-declarator
     *)
     (* The type specifier is optional in non-strict mode and required in
        strict mode. We give an error in a later pass. *)
     let (parser, prop_type) = match peek_token_kind parser with
     | Variable -> (parser, make_missing())
     | _ -> parse_type_specifier parser in
     let (parser, decls) = parse_comma_list
       parser Semicolon SyntaxError.error1008 parse_property_declarator in
     let (parser, semi) = require_semicolon parser in
     let result = make_property_declaration modifiers prop_type decls semi in
     (parser, result)

  and parse_property_declarator parser =
    (* SPEC:
      property-declarator:
        variable-name  property-initializer-opt
      property-initializer:
        =  expression
    *)
    let (parser, name) = require_variable parser in
    let (parser, simple_init) = parse_simple_initializer_opt parser in
    let result = make_property_declarator name simple_init in
    (parser, result)

  (* SPEC:
    const-declaration:
      abstract_opt  const  type-specifier_opt  constant-declarator-list  ;
    constant-declarator-list:
      constant-declarator
      constant-declarator-list  ,  constant-declarator
    constant-declarator:
      name  constant-initializer_opt
    constant-initializer:
      =  const-expression
  *)
  and parse_const_declaration parser abstr const =
    let (parser, type_spec) = if is_type_in_const parser then
      parse_type_specifier parser
    else
      parser, make_missing ()
    in
    let (parser, const_list) = parse_comma_list
      parser Semicolon SyntaxError.error1004 parse_constant_declarator in
    let (parser, semi) = require_semicolon parser in
    (parser, make_const_declaration abstr const type_spec const_list semi)

  and is_type_in_const parser =
    (* TODO Use Eric's helper here to assert length of errors *)
    let before = List.length (errors parser) in
    let (parser1, _) = parse_type_specifier parser in
    let (parser1, _) = require_name_allow_keywords parser1 in
    List.length (errors parser1) = before

  and parse_constant_declarator parser =
    (* TODO: We allow const names to be keywords here; in particular we
       require that const string TRUE = "true"; be legal.  Likely this
       should be more strict. What are the rules for which keywords are
       legal constant names and which are not?
       Note that if this logic is changed, it should be changed in
       is_type_in_const above as well.
    *)
    let (parser, const_name) = require_name_allow_keywords parser in
    let (parser, initializer_) = parse_simple_initializer_opt parser in
    (parser, make_constant_declarator const_name initializer_)

  (* SPEC:
    type-constant-declaration:
      abstract-type-constant-declaration
      concrete-type-constant-declaration
    abstract-type-constant-declaration:
      abstract  const  type  name  type-constraintopt  ;
    concrete-type-constant-declaration:
      const  type  name  type-constraintopt  =  type-specifier  ;
  *)
  and parse_type_const_declaration parser abstr const =
    (* TODO: Error handle -
      abstract type consts only in interfaces or abstract classes
      interfaces cannot have concrete type consts with type constraints
    *)
    let (parser, type_token) = assert_token parser Type in
    let (parser, name) = require_name parser in
    let (parser, type_constraint) = parse_type_constraint_opt parser in
    let (parser, equal_token, type_specifier) = if is_missing abstr then
      let (parser, equal_token) = require_equal parser in
      let (parser, type_spec) = parse_type_specifier parser in
      (parser, equal_token, type_spec)
    else
      (parser, make_missing (), make_missing ())
    in
    let (parser, semicolon) = require_semicolon parser in
    let syntax = make_type_const_declaration
      abstr const type_token name type_constraint equal_token type_specifier
      semicolon
    in
    (parser, syntax)

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
   (* TODO: The list of attrs can have a trailing comma. Update the spec. *)
   (* TODO: The list of values can have a trailing comma. Update the spec. *)
  and parse_attribute_specification_opt parser =
    if peek_token_kind parser = LessThanLessThan then
      let (parser, left, items, right) =
        parse_double_angled_comma_list_allow_trailing parser parse_attribute in
      (parser, make_attribute_specification left items right)
    else
      (parser, make_missing())

  and parse_attribute parser =
    let (parser, name) = require_name parser in
    let (parser, left, items, right) =
      if peek_token_kind parser = LeftParen then
        parse_parenthesized_comma_list_opt_allow_trailing
          parser parse_expression
      else
        (parser, make_missing(), make_missing(), make_missing()) in
    (parser, make_attribute name left items right)

  and parse_generic_type_parameter_list_opt parser =
    let (parser1, open_angle) = next_token parser in
    if (Token.kind open_angle) = LessThan then
        let type_parser = TypeParser.make parser.lexer
          parser.errors parser.context in
        let (type_parser, node) =
          TypeParser.parse_generic_type_parameter_list type_parser in
        let lexer = TypeParser.lexer type_parser in
        let errors = TypeParser.errors type_parser in
        let parser = { parser with lexer; errors } in
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

  and parse_parameter_list_opt parser =
      (* SPEC

        TODO: Update the spec to match this.

        parameter-list:
          variadic-parameter
          parameter-declaration-list
          parameter-declaration-list  ,
          parameter-declaration-list  ,  variadic-parameter

        parameter-declaration-list:
          parameter-declaration
          parameter-declaration-list  ,  parameter-declaration

        variadic-parameter:
          ...
          attribute-specification-opt visiblity-modifier-opt type-specifier \
            ...  variable-name
     *)
     (* This function parses the parens as well. *)
     (* ERROR RECOVERY: We allow variadic parameters in all positions; a later
        pass gives an error if a variadic parameter is in an incorrect position
        or followed by a trailing comma.  *)
     (* TODO: Add an error checking pass that ensures that a variadic parameter
     does not have a default value. *)
      parse_parenthesized_comma_list_opt_allow_trailing parser parse_parameter

  and parse_parameter parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | DotDotDot ->
      let next_kind = peek_token_kind parser1 in
      if next_kind = Variable then parse_parameter_declaration parser
      else (parser1, make_variadic_parameter (make_token token))
    | _ -> parse_parameter_declaration parser

  and parse_parameter_declaration parser =
    (* SPEC
    TODO: The specification does not include modifiers. Fix the spec.
    parameter-declaration:
      attribute-specification-opt  type-specifier  variable-name \
      default-argument-specifier-opt
    *)
    (* In strict mode, we require a type specifier. This error is not caught
       at parse time but rather by a later pass. *)
    let (parser, attrs) = parse_attribute_specification_opt parser in
    let (parser, visibility) = parse_visibility_modifier_opt parser in
    let token = peek_token parser in
    let (parser, type_specifier) =
      match Token.kind token with
        | Variable | DotDotDot | Ampersand -> (parser, make_missing())
        | _ -> parse_type_specifier parser in
    let (parser, name) = parse_decorated_variable_opt parser in
    let (parser, default) = parse_simple_initializer_opt parser in
    let syntax =
      make_parameter_declaration attrs visibility type_specifier name default in
    (parser, syntax)

  and parse_decorated_variable_opt parser =
    match peek_token_kind parser with
    | DotDotDot
    | Ampersand -> parse_decorated_variable parser
    | _ -> require_variable parser

  (* TODO: This is wrong. The variable here is not an *expression* that has
  an optional decoration on it.  It's a declaration. We shouldn't be using the
  same data structure for a decorated expression as a declaration; one
  is a *use* and the other is a *definition*. *)
  and parse_decorated_variable parser =
    (* TODO: Error on
          ... ... variable
          & & variable
          ... & variable
       at a later pass
     *)
    let (parser, decorator) = next_token parser in
    let decorator = make_token decorator in
    let (parser, variable) =
      match peek_token_kind parser with
      | DotDotDot
      | Ampersand -> parse_decorated_variable parser
      | _ -> require_variable parser
    in
    parser, make_decorated_expression decorator variable

  and parse_visibility_modifier_opt parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Public | Protected | Private -> (parser1, make_token token)
    | _ -> (parser, make_missing())

  (* SPEC
    default-argument-specifier:
      =  const-expression

    constant-initializer:
      =  const-expression
  *)
  and parse_simple_initializer_opt parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Equal ->
      (* TODO: Detect if expression is not const *)
      let (parser, default_value) = parse_expression parser1 in
      (parser, make_simple_initializer (make_token token) default_value)
    | _ -> (parser, make_missing())

  and parse_function parser =
    parse_function_declaration parser (make_missing())

  and parse_function_declaration parser attribute_specification =
    let (parser, header) =
      parse_function_declaration_header parser in
    let (parser, body) = parse_compound_statement parser in
    let syntax = make_function_declaration
      attribute_specification header body in
    (parser, syntax)

  and parse_constraint_operator parser =
    (* TODO: Put this in the specification
      constraint-operator:
        =
        as
        super
    *)
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Equal
    | As
    | Super -> (parser1, (make_token token))
    | _ -> (* ERROR RECOVERY: don't eat the offending token. *)
      (* TODO: Give parse error *)
      (parser, (make_missing()))

  and parse_where_constraint parser =
    (* TODO: Put this in the specification
    constraint:
      type-specifier  constraint-operator  type-specifier
    *)
    let (parser, left) = parse_type_specifier parser in
    let (parser, op) = parse_constraint_operator parser in
    let (parser, right) = parse_type_specifier parser in
    let result = make_where_constraint left op right in
    (parser, result)

  and parse_where_constraint_list_item parser =
    let (parser, where_constraint) = parse_where_constraint parser in
    let (parser, comma) = optional_token parser Comma in
    let result = make_list_item where_constraint comma in
    (parser, result)

  and parse_where_clause parser =
    (* TODO: Add this to the specification
      where-clause:
        where   constraint-list

      constraint-list:
        constraint
        constraint-list , constraint

      Note that a trailing comma is not accepted in a constraint list
    *)
    let (parser, keyword) = assert_token parser Where in
    let (parser, constraints) = parse_list_until_no_comma
      parser parse_where_constraint_list_item in
    let result = make_where_clause keyword constraints in
    (parser, result)

  and parse_where_clause_opt parser =
    if peek_token_kind parser != Where then
      (parser, (make_missing()))
    else
      parse_where_clause parser

  and parse_function_declaration_header parser =
    (* SPEC
      function-definition-header:
        attribute-specification-opt  async-opt  coroutine-opt  function  name  /
        generic-type-parameter-list-opt  (  parameter-list-opt  ) :  /
        return-type   where-clause-opt

      TODO: The spec does not specify "where" clauses. Add them.

    *)
    (* In strict mode, we require a type specifier. This error is not caught
       at parse time but rather by a later pass. *)
    (* In non-strict mode we allow an & to appear before the name.
      TODO: Produce an error if this occurs in strict mode, or if it
      TODO: appears before a special name like __construct, and so on. *)
    let (parser, async_token) = optional_token parser Async in
    let (parser, coroutine_token) = optional_token parser Coroutine in
    let (parser, function_token) = require_function parser in
    let (parser, ampersand_token) = optional_token parser Ampersand in
    let (parser, label) =
      parse_function_label parser in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser in
    let (parser, left_paren_token, parameter_list, right_paren_token) =
      parse_parameter_list_opt parser in
    let (parser, colon_token, return_type) =
      parse_return_type_hint_opt parser in
    let (parser, where_clause) = parse_where_clause_opt parser in
    let syntax =
      make_function_declaration_header
        async_token
        coroutine_token
        function_token
        ampersand_token
        label
        generic_type_parameter_list
        left_paren_token
        parameter_list
        right_paren_token
        colon_token
        return_type
        where_clause in
    (parser, syntax)

  (* A function label is either a function name, a __construct label, or a
  __destruct label. *)
  and parse_function_label parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Name
    | Construct
    | Destruct -> (parser1, make_token token)
    | _ ->
      (* TODO: We might have a non-reserved keyword as the name here; "empty",
      for example, is a keyword but a legal function name. What we do here is
      accept any keyword; what we *should* do is figure out which keywords are
      reserved and which are not, and reject the reserved keywords. *)
      let (parser, token) = next_token_as_name parser in
      if Token.kind token = Name then
        (parser, make_token token)
      else
        (* ERROR RECOVERY: Eat the offending token. *)
        let parser = with_error parser SyntaxError.error1044 in
        let error = make_error (make_token token) in
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
  and parse_methodish_or_const_or_type_const parser =
    let (parser1, abstract) = assert_token parser Abstract in
    if peek_token_kind parser1 = Const then
      parse_const_or_type_const_declaration parser1 abstract
    else
      let (parser, modifiers) = parse_modifiers parser in
      parse_methodish parser (make_missing ()) modifiers

  and parse_methodish parser attribute_spec modifiers =
    let (parser, header) = parse_function_declaration_header parser in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | LeftBrace ->
      let (parser, body) = parse_compound_statement parser in
      let syntax =
        make_methodish_declaration
          attribute_spec modifiers header body (make_missing ())in
      (parser, syntax)
    | Semicolon ->
      let semicolon = make_token token in
      let syntax =
        make_methodish_declaration
          attribute_spec modifiers header (make_missing())
        semicolon in
      (parser1, syntax)
    | _ ->
      (* ERROR RECOVERY: We expected either a block or a semicolon; we got
      neither. Use the offending token as the body of the method.
      TODO: Is this the right error recovery? *)
      let error = make_error (make_token token) in
      let syntax = make_methodish_declaration
        attribute_spec modifiers header error (make_missing()) in
      let parser = with_error parser1 SyntaxError.error1041 in
      (parser, syntax)

  and parse_modifier parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Abstract
    | Static
    | Public
    | Protected
    | Private
    | Final -> (parser1, Some (make_token token))
    | _ -> (parser, None)

  and parse_modifiers parser =
    parse_list_until_none parser parse_modifier

  and parse_enum_or_classish_or_function_declaration parser =
    (* An enum, type alias, function, interface, trait or class may all
      begin with an attribute. *)
    let parser, attribute_specification =
      parse_attribute_specification_opt parser in
    let parser1, token = next_token parser in
    match Token.kind token with
    | Enum -> parse_enum_declaration parser attribute_specification
    | Type | Newtype ->
      parse_alias_declaration parser attribute_specification
    | Async | Coroutine | Function ->
      parse_function_declaration parser attribute_specification
    | Abstract
    | Final
    | Interface
    | Trait
    | Class -> parse_classish_declaration parser attribute_specification
    | _ ->
      (* ERROR RECOVERY TODO: Produce an error here. *)
      (* TODO: This is wrong; we have lost the attribute specification
      from the tree. *)
      (parser1, make_error (make_token token))

  and parse_declaration parser =
    let recovery_tokens = [ Class; Trait; Interface ] in
    let parser = SimpleParser.expect_in_new_scope parser recovery_tokens in
    let (parser1, token) = next_token parser in
    let (parser, result) =
      match (Token.kind token) with
      | TokenKind.EndOfFile -> (parser1, make_end_of_file (make_token token))
      | Include
      | Include_once
      | Require
      | Require_once -> parse_inclusion_directive parser
      | Type
      | Newtype -> parse_alias_declaration parser (make_missing())
      | Enum -> parse_enum_declaration parser (make_missing())
      | Namespace -> parse_namespace_declaration parser
      | Use -> parse_namespace_use_declaration parser
      | Trait
      | Interface
      | Abstract
      | Final
      | Class -> parse_classish_declaration parser (make_missing())
      | Async
      | Coroutine
      | Function -> parse_function_declaration parser (make_missing())
      | LessThanLessThan ->
        parse_enum_or_classish_or_function_declaration parser
        (* TODO figure out what global const differs from class const *)
      | Const -> parse_const_declaration parser1 (make_missing ())
              (make_token token)
      | _ ->
        parse_statement parser in
        (* TODO: What if it's not a legal statement? Do we still make progress
        here? *)
    let parser1 = SimpleParser.pop_scope parser recovery_tokens in
    (parser1, result)

  let parse_leading_markup_section parser =
    let parser1, markup_section = parse_in_statement_parser parser
      (StatementParser.parse_markup_section ~is_leading_section:true)
    in
    let valid =
      match markup_section.syntax with
      (* proceed successfully if we've consumed <?... *)
      (* TODO: Give an error if there is leading trivia on the < in an hh file *)
      (* TODO: Handle the case where the langauge is not a Name. *)
      | MarkupSection { markup_suffix; _ } -> not (is_missing markup_suffix)
      | _ -> false
    in
    if valid then
      parser1, markup_section
    else
      let parser = with_error parser SyntaxError.error1001 in
      let markup_section =
        make_markup_section (make_missing ()) (make_missing ())
          (make_missing ()) (make_missing ())
      in
      (* ERROR RECOVERY *)
      (* Make no progress; try parsing the file without a header *)
      parser, markup_section

  let parse_script parser =
    let rec aux parser acc =
      let (parser, declaration) = parse_declaration parser in
      (* TODO: Assert that we either made progress, or we're at the end of
      the file. *)
      if kind declaration = SyntaxKind.EndOfFile then
        (* The only time an end-of-file node is useful is when there is
           leading trivia in the end-of-file token. *)
        match leading_token declaration with
        | Some token ->
          if Token.leading token = [] then (parser, acc)
          else (parser, declaration :: acc)
        | _ -> (parser, acc)
      else aux parser (declaration :: acc) in
    (* parse leading markup section *)
    let (parser, header) = parse_leading_markup_section parser in
    let (parser, declarations) = aux parser [] in
    (* include leading markup section as a head of declaration list *)
    let declarations = make_list (header :: List.rev declarations) in
    let result = make_script declarations in
    (* If we are not at the end of the file, something is wrong. *)
    assert ((peek_token_kind parser) = TokenKind.EndOfFile);
    (parser, result)

end
