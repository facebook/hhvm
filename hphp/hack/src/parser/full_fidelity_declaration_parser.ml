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
module TriviaKind = Full_fidelity_trivia_kind
module SyntaxError = Full_fidelity_syntax_error
module Env = Full_fidelity_parser_env
module SimpleParserSyntax =
  Full_fidelity_simple_parser.WithSyntax(Syntax)
module SimpleParser = SimpleParserSyntax.WithLexer(
    Full_fidelity_lexer.WithToken(Syntax.Token))

module ParserHelperSyntax = Full_fidelity_parser_helpers.WithSyntax(Syntax)
module ParserHelper = ParserHelperSyntax
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))

module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S
module type ExpressionParser_S = Full_fidelity_expression_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .ExpressionParser_S

module type StatementParser_S = Full_fidelity_statement_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .StatementParser_S

module type TypeParser_S = Full_fidelity_type_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .TypeParser_S

module type DeclarationParser_S = Full_fidelity_declaration_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .DeclarationParser_S

open TokenKind
open Syntax

module WithSmartConstructors (SCI : SCWithKind_S with module Token = Syntax.Token)
= struct

module WithExpressionAndStatementAndTypeParser
  (ExpressionParser : ExpressionParser_S with module SC = SCI)
  (StatementParser : StatementParser_S with module SC = SCI)
  (TypeParser : TypeParser_S with module SC = SCI)
  : (DeclarationParser_S with module SC = SCI) = struct

  module Parser = SimpleParser.WithSmartConstructors(SCI)
  include Parser
  include ParserHelper.WithParser(Parser)

  (* Tokens *)

  let with_type_parser : 'a . t -> (TypeParser.t -> TypeParser.t * 'a) -> t * 'a
  = fun parser f ->
    let type_parser =
      TypeParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state in
    let (type_parser, node) = f type_parser in
    let env = TypeParser.env type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let context = TypeParser.context type_parser in
    let sc_state = TypeParser.sc_state type_parser in
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, node)

  let parse_possible_generic_specifier parser =
    with_type_parser parser TypeParser.parse_possible_generic_specifier

  let parse_type_specifier ?(allow_var=false) parser =
    with_type_parser parser (TypeParser.parse_type_specifier ~allow_var)

  let parse_simple_type_or_type_constant parser =
    with_type_parser parser (TypeParser.parse_simple_type_or_type_constant)

  let parse_type_constraint_opt parser =
    with_type_parser parser TypeParser.parse_type_constraint_opt

  let with_statement_parser
  : 'a . t -> (StatementParser.t -> StatementParser.t * 'a) -> t * 'a
  = fun parser f ->
    let statement_parser =
      StatementParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state in
    let (statement_parser, node) = f statement_parser in
    let env = StatementParser.env statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let context = StatementParser.context statement_parser in
    let sc_state = StatementParser.sc_state statement_parser in
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, node)

  let parse_compound_statement parser =
    with_statement_parser parser StatementParser.parse_compound_statement

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
    Make.inclusion_directive parser expr semi

  and parse_alias_declaration parser attr =
    (* SPEC
      alias-declaration:
        attribute-spec-opt type  name
          generic-type-parameter-list-opt  =  type-specifier  ;
        attribute-spec-opt newtype  name
          generic-type-parameter-list-opt type-constraint-opt
            =  type-specifier  ;
    *)

    let (parser, token) = fetch_token parser in
    (* Not `require_name` but `require_name_allow_keywords`, because the parser
     * must allow keywords in the place of identifiers; at least to parse .hhi
     * files.
     *)
    let (parser, name) = require_name_allow_keywords parser in
    let (parser, generic) =
      with_type_parser parser TypeParser.parse_generic_parameter_list_opt
    in
    let (parser, constr) = parse_type_constraint_opt parser in
    let (parser, equal) = require_equal parser in
    let (parser, ty) = parse_type_specifier parser in
    let (parser, semi) = require_semicolon parser in
    Make.alias_declaration parser attr token name generic constr equal ty semi

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
    Make.enumerator parser name equal value semicolon

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
    let (parser, left_brace, enumerators, right_brace) =
      parse_braced_list parser parse_enumerator_list_opt
    in
    Make.enum_declaration parser attrs enum name colon base enum_type left_brace
      enumerators right_brace

  and parse_namespace_declaration parser =
    (* SPEC
      namespace-definition:
        namespace  namespace-name  ;
        namespace  namespace-name-opt  { declaration-list }
    *)

    (* TODO: An error case not caught by the parser that should be caught
             in a later pass:
             Qualified names are a superset of legal namespace names.
    *)
    let (parser, namespace_token) = assert_token parser Namespace in
    let (parser1, token) = next_token parser in
    let (parser, name) = match Token.kind token with
    | Name ->
      let (parser, token) = Make.token parser1 token in
      scan_remaining_qualified_name parser token
    | LeftBrace -> Make.missing parser (pos parser)
    | Semicolon ->
      (* ERROR RECOVERY Plainly the name is missing. *)
      let parser = with_error parser SyntaxError.error1004 in
      Make.missing parser (pos parser)
    | _ ->
      (* TODO: Death to PHPisms; keywords as namespace names *)
      require_name_allow_keywords parser in
    let (parser, body) = parse_namespace_body parser in
    Make.namespace_declaration parser namespace_token name body

  and parse_namespace_body parser =
    match peek_token_kind parser with
    | Semicolon ->
      let (parser, token) = fetch_token parser in
      Make.namespace_empty_body parser token
    | LeftBrace ->
      let (parser, left) = fetch_token parser in
      let (parser, body) =
        parse_terminated_list parser parse_declaration RightBrace in
      let (parser, right) = require_right_brace parser in
      Make.namespace_body parser left body right
    | _ ->
      (* ERROR RECOVERY: return an inert namespace (one with all of its
       * components 'missing'), and recover--without advancing the parser--
       * back to the level that the namespace was declared in. *)
      let parser = with_error parser SyntaxError.error1038 in
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      let (parser, missing3) = Make.missing parser (pos parser) in
      Make.namespace_body parser missing1 missing2 missing3

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
    | Const -> Make.token parser1 token
    | _ -> Make.missing parser (pos parser)

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
        let (parser, as_token) = Make.token parser1 as_token in
        let (parser, alias) = require_name_allow_std_constants parser in
        (parser, as_token, alias)
      else
        let (parser, missing1) = Make.missing parser (pos parser) in
        let (parser, missing2) = Make.missing parser (pos parser) in
        (parser, missing1, missing2)
    in
    Make.namespace_use_clause parser use_kind name as_token alias

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
    | Backslash ->
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, backslash) = Make.token parser token in
      let (parser, _name, is_backslash) =
        scan_qualified_name_extended parser missing backslash
      in
      is_backslash || peek_token_kind parser = LeftBrace
    | Name ->
      let (parser, token) = Make.token parser token in
      let (parser, name, is_backslash) =
        scan_remaining_qualified_name_extended parser token
      in
      (* Here we rely on the implementation details of
      scan_remaining_qualified_name_extended. It's returning
      *exactly* token if there is nothing except it in the name. *)
      is_backslash && name <> token || peek_token_kind parser = LeftBrace
    | _ -> false

  and parse_group_use parser =
    (* See below for grammar. *)
    let (parser, use_token) = assert_token parser Use in
    let (parser, use_kind) = parse_namespace_use_kind_opt parser in
    (* We already know that this is a name, qualified name, or prefix. *)
    (* If this is not a prefix, it will be detected as an error in a later pass *)
    let (parser, prefix) = scan_name_or_qualified_name parser in
    let (parser, left, clauses, right) =
      parse_braced_comma_list_opt_allow_trailing
      parser parse_namespace_use_clause in
    let (parser, semi) = require_semicolon parser in
    Make.namespace_group_use_declaration parser use_token use_kind prefix left
      clauses right semi

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
    (* ERROR RECOVERY
    In the "simple" format, the kind may only be specified up front.

    The grammar in the specification says that in the "group"
    format, if the kind is specified up front then it may not
    be specified in each clause. However, HHVM's parser disallows
    the kind in each clause regardless of whether it is specified up front.
    We will fix the specification to match HHVM.

    The grammar in the specification also says that in the "simple" format,
    the kind may only be specified up front.  But HHVM allows the kind to
    be specified in each clause.  Again, we will fix the specification to match
    HHVM.

    TODO: Update the grammar comment above when the specification is fixed.
    (This work is being tracked by spec work items 102, 103 and 104.)

    We do not enforce these rules here. Rather, we allow the kind to be anywhere,
    and detect the errors in a later pass. *)
    if is_group_use parser then
      parse_group_use parser
    else
      let (parser, use_token) = assert_token parser Use in
      let (parser, use_kind) = parse_namespace_use_kind_opt parser in
      let (parser, clauses, _) =
        parse_comma_list_allow_trailing
          parser
          Semicolon
          SyntaxError.error1004
          parse_namespace_use_clause
      in
      let (parser, semi) = require_semicolon parser in
      Make.namespace_use_declaration parser use_token use_kind clauses semi

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
    Make.classish_declaration
      parser
      attribute_spec modifiers
      token
      name
      generic_type_parameter_list
      classish_extends
      classish_extends_list
      classish_implements
      classish_implements_list
      body

  and parse_classish_modifiers parser =
    let rec parse_classish_modifier_opt parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
        | Abstract
        | Final ->
          (* TODO(T25649779) *)
          let (parser, token) = Make.token parser1 token in
          let acc = token :: acc in
          parse_classish_modifier_opt parser acc
        | _ -> make_list parser (List.rev acc)
    in
    parse_classish_modifier_opt parser []

  and parse_classish_token parser =
    let spellcheck_tokens = [Class; Trait; Interface] in
    let token_str = current_token_text parser in
    let (parser1, token) = next_token parser in
    match Token.kind token with
      | Class
      | Trait
      | Interface -> Make.token parser1 token
       (* Spellcheck case *)
      | Name when is_misspelled_from spellcheck_tokens token_str ->
        (* Default won't be used, since we already checked is_misspelled_from *)
        let suggested_kind =
          Option.value (suggested_kind_from spellcheck_tokens token_str)
            ~default:Name in
        let parser = skip_and_log_misspelled_token parser suggested_kind in
        Make.missing parser (pos parser)
      | _ ->
        let parser = with_error parser SyntaxError.error1035 in
        Make.missing parser (pos parser)

  and parse_classish_extends_opt parser =
    let (parser1, extends_token) = next_token parser in
    if (Token.kind extends_token) <> Extends then
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      (parser, missing1, missing2)
    else
      let (parser, extends_token) = Make.token parser1 extends_token in
      let (parser, extends_list) = parse_special_type_list parser in
      (parser, extends_token, extends_list)

  and parse_classish_implements_opt parser =
    let (parser1, implements_token) = next_token parser in
    if (Token.kind implements_token) <> Implements then
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      (parser, missing1, missing2)
    else
    let (parser, implements_token) = Make.token parser1 implements_token in
    let (parser, implements_list) = parse_special_type_list parser in
    (parser, implements_token, implements_list)

  and parse_special_type parser =
    let (parser1, token) = next_xhp_class_name_or_other_token parser in
    match (Token.kind token) with
    | Comma ->
      (* ERROR RECOVERY. We expected a type but we got a comma.
      Give the error that we expected a type, not a name, even though
      not every type is legal here. *)
      let parser = with_error parser1 SyntaxError.error1007 in
      let (parser, comma) = Make.token parser token in
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, list_item) = Make.list_item parser missing comma in
      (parser, list_item, comma)
    | Backslash
    | Namespace
    | Name
    | XHPClassName ->
      let (parser, item) = parse_type_specifier parser in
      let (parser, comma) = optional_token parser Comma in
      let (parser, list_item) = Make.list_item parser item comma in
      (parser, list_item, comma)
    | Parent
    | Enum
    | Shape
    | Self when Env.hhvm_compat_mode (env parser) ->
      (* HHVM allows these keywords here for some reason *)
      let (parser, item) = parse_simple_type_or_type_constant parser in
      let (parser, comma) = optional_token parser Comma in
      let (parser, list_item) = Make.list_item parser item comma in
      (parser, list_item, comma)
    | _ ->
      (* ERROR RECOVERY: We are expecting a type; give an error as above.
      Don't eat the offending token.
      *)
      let parser = with_error parser SyntaxError.error1007 in
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      let (parser, list_item) = Make.list_item parser missing1 missing2 in
      (parser, list_item, missing2)

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
    let rec aux parser acc =
      let (parser, item, separator) = parse_special_type parser in
      if SC.is_missing separator then
        (parser, item :: acc)
      else
        aux parser (item :: acc)
    in
    let (parser, items) = aux parser [] in
    make_list parser (List.rev items)


  and parse_classish_body parser =
    let (parser, left_brace_token) = require_left_brace parser in
    let (parser, classish_element_list) =
      parse_classish_element_list_opt parser
    in
    let (parser, right_brace_token) = require_right_brace parser in
    Make.classish_body parser left_brace_token classish_element_list
      right_brace_token

  and parse_classish_element parser =
    (*We need to identify an element of a class, trait, etc. Possibilities
    are:

     // constant-declaration:
     const T $x = v ;
     abstract const T $x ;
     public const T $x = v ; // PHP7 only

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
     (This work is tracked by task T21622566)

     // method-declaration
     <<attr>> public/private/protected/abstract/final/static async function
     Note that a modifier is required, the attr and async are optional.
     TODO: Hack requires a visibility modifier, unless "static" is supplied,
     TODO: in which case the method is considered to be public.  Is this
     TODO: desired? Resolve this disagreement with the spec.

     // constructor-declaration
     <<attr>> public/private/protected/abstract/final function __construct
     Note that we allow static constructors in this parser; we produce an
     error in the post-parse error detection pass.

     // destructor-declaration
     <<attr>> public/private/protected function __destruct
     TODO: Hack and HHVM allow final and abstract destructors, but the
     TODO: spec says that these should not be legal; resolve this discrepancy.
     We do not give an error for incorrect destructor modifiers in this parser;
     we produce an error in the post-parse error detection pass.

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
    match peek_token_kind parser with
    | Children -> parse_xhp_children_declaration parser
    | Category -> parse_xhp_category_declaration parser
    | Use -> parse_trait_use parser
    | Const ->
      begin
        let (parser, missing) = Make.missing parser (pos parser) in
        let kind1 = peek_token_kind ~lookahead:1 parser in
        let kind2 = peek_token_kind ~lookahead:2 parser in
        match kind1, kind2 with
        | Type, Semicolon ->
          let (parser, missing') = Make.missing parser (pos parser) in
          let (parser, const) = assert_token parser Const in
          parse_const_declaration parser missing missing' const
        | Type, _ when kind2 <> Equal ->
          let (parser, const) = assert_token parser Const in
          parse_type_const_declaration parser missing const
        | _, _ ->
          let (parser, missing') = Make.missing parser (pos parser) in
          let (parser, const) = assert_token parser Const in
          parse_const_declaration parser missing missing' const
      end
    | Abstract -> parse_methodish_or_const_or_type_const parser
    | Public
    | Protected
    | Private ->
      let (parser1, visibility) = next_token parser in
      let next_kind = peek_token_kind parser1 in
      if next_kind = Const then
        let (parser, visibility) = Make.token parser1 visibility in
        let (parser, missing) = Make.missing parser (pos parser) in
        let (parser, const) = assert_token parser Const in
        parse_const_declaration parser visibility missing const
      else
        let (parser, missing) = Make.missing parser (pos parser) in
        parse_methodish_or_property parser missing
    | Async
    | Static
    | Final
    | LessThanLessThan ->
      (* Parse methods, constructors, destructors or properties. *)
      let (parser, attr) = parse_attribute_specification_opt parser in
      parse_methodish_or_property parser attr
    | Require ->
      (* We give an error if these are found where they should not be,
         in a later pass. *)
      parse_require_clause parser
    | TokenKind.Attribute -> parse_xhp_class_attribute_declaration parser
    | Function ->
      (* ERROR RECOVERY
      Hack requires that a function inside a class be marked
      with a visibility modifier, but PHP does not have this requirement.
      We accept the lack of a modifier here, and produce an error in
      a later pass. *)
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      parse_methodish parser missing1 missing2
    | Var ->
      (* We allow "var" as a synonym for "public" in a property; this
      is a PHP-ism that we do not support in Hack, but we parse anyways
      so as to give an error later. *)
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, var) = assert_token parser Var in
      parse_property_declaration parser missing var
    | kind when Parser.expects parser kind ->
      Make.missing parser (pos parser)
    | _ ->
        (* TODO ERROR RECOVERY could be improved here. *)
      let (parser, token) = fetch_token parser in
      let parser = with_error parser SyntaxError.error1033 in
      Make.error parser token
    (* Parser does not detect the error where non-static instance variables
      or methods are within abstract final classes in its first pass, but
      instead detects it in its second pass. *)

  and parse_classish_element_list_opt parser =
    (* TODO: ERROR RECOVERY: consider bailing if the token cannot possibly
             start a classish element. *)
    (* ERROR RECOVERY: we're in the body of a classish, so we add visibility
     * modifiers to our context. *)
    let recovery_tokens = [Public; Protected; Private] in
    let parser = Parser.expect_in_new_scope parser recovery_tokens in
    let (parser, element_list) =
      parse_terminated_list parser parse_classish_element RightBrace in
    let parser = Parser.pop_scope parser recovery_tokens in
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
      parse_parenthesized_comma_list parser parse_xhp_children_expression
    in
    Make.xhp_children_parenthesized_list parser left exprs right

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
    let (parser1, name) = Make.token parser1 token in
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
      let (parser, token) = Make.token parser1 token in
      Make.postfix_unary_expression parser term token
    | _ -> (parser, term)

  and parse_xhp_children_bar parser left =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Bar ->
      let (parser, token) = Make.token parser1 token in
      let (parser, right) = parse_xhp_children_term parser in
      let (parser, result) = Make.binary_expression parser left token right in
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
    let (parser1, token) = next_token parser in
    let (parser, expr) =
      match Token.kind token with
      | Empty -> Make.token parser1 token
      | _ -> parse_xhp_children_expression parser
    in
    let (parser, semi) = require_semicolon parser in
    Make.xhp_children_declaration parser children expr semi

  and parse_xhp_category parser =
    let (parser, token) = next_xhp_category_name parser in
    let (parser, category) = Make.token parser token in
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
    let (parser, items, _) =
      parse_comma_list_allow_trailing
        parser
        Semicolon
        SyntaxError.error1052
        parse_xhp_category
    in
    let (parser, semi) = require_semicolon parser in
    Make.xhp_category_declaration parser category items semi

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
      (This work is tracked by task T21175355)

      An empty list is illegal, but we allow it here and give an error in
      a later pass.
    *)
    let (parser', token) = next_token parser in
    let (parser', token, optional) =
      match Token.kind token with
      | Question ->
        let (parser, enum_token) = next_token parser' in
        let (parser, token) = Make.token parser token in
        (parser, enum_token, token)
      | _ ->
        let (parser, missing) = Make.missing parser' (pos parser) in
        (parser, token, missing)
    in
    match Token.kind token with
    | Enum ->
      let (parser, enum_token) = Make.token parser' token in
      let (parser, left_brace, values, right_brace) =
        parse_braced_comma_list_opt_allow_trailing parser parse_expression
      in
      Make.xhp_enum_type
        parser
        optional
        enum_token
        left_brace
        values
        right_brace
    | _kind -> parse_type_specifier ~allow_var:true parser

  and parse_xhp_required_opt parser =
    (* SPEC (Draft)
      xhp-required :
        @  required

      Note that these are two tokens. They can have whitespace between them. *)
    if peek_token_kind parser = At then
      let (parser, at) = assert_token parser At in
      let (parser, req) = require_required parser in
      Make.xhp_required parser at req
    else
      Make.missing parser (pos parser)

  and parse_xhp_class_attribute_typed parser =
    (* xhp-type-specifier xhp-name initializer-opt xhp-required-opt *)
    let (parser, ty) = parse_xhp_type_specifier parser in
    let (parser, name) = require_xhp_name parser in
    let (parser, init) = parse_simple_initializer_opt parser in
    let (parser, req) = parse_xhp_required_opt parser in
    Make.xhp_class_attribute parser ty name init req

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
        let (parser, type_specifier) =
          Make.simple_type_specifier parser1 class_name
        in
        Make.xhp_simple_class_attribute parser type_specifier
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
      SyntaxError.error1004 parse_xhp_class_attribute
    in
    let (parser, semi) = require_semicolon parser in
    Make.xhp_class_attribute_declaration parser attr_token attrs semi

  and parse_qualified_name_type parser =
    (* Here we're parsing a name followed by an optional generic type
       argument list; if we don't have a name, give an error. *)
    match peek_token_kind parser with
    | Backslash
    | Name ->
      parse_possible_generic_specifier parser
    | _ -> require_qualified_name parser

  and parse_qualified_name_type_opt parser =
    (* Here we're parsing a name followed by an optional generic type
       argument list; if we don't have a name, give an error. *)
    match peek_token_kind parser with
    | Backslash
    | Construct
    | Name -> parse_possible_generic_specifier parser
    | _ -> Make.missing parser (pos parser)

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
       (This work is being tracked by spec issue 105.)
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
    | Extends -> Make.token parser1 req_kind_token
    | _ ->
      let parser = with_error parser SyntaxError.error1045 in
      Make.missing parser (pos parser)
    in
    let (parser, name) =
      if is_next_xhp_class_name parser
      then parse_possible_generic_specifier parser
      else parse_qualified_name_type parser
    in
    let (parser, semi) = require_semicolon parser in
    Make.require_clause parser req req_kind name semi

  and parse_methodish_or_property parser attribute_spec =
    let (parser, modifiers, contains_abstract) = parse_modifiers parser in
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
      parse_property_declaration
        parser attribute_spec modifiers ~contains_abstract
    (* We encountered one unexpected token, but the next still indicates that
     * we should be parsing a methodish. Throw an error, process the token
     * as an extra, and keep going. *)
    | _, (Async | Coroutine | Function)
      when not (Syntax.has_leading_trivia TriviaKind.EndOfLine next_token) ->
      let parser = with_error parser SyntaxError.error1056
        ~on_whole_token:true in
      let parser = skip_and_log_unexpected_token parser
        ~generate_error:false in
      parse_methodish parser attribute_spec modifiers
    (* Otherwise, continue parsing as a property (which might be a lambda). *)
    | ( _ , _ ) ->
      parse_property_declaration
        parser attribute_spec modifiers ~contains_abstract

  and parse_trait_use_precedence_item parser name =
    let (parser, keyword) = assert_token parser Insteadof in
    let (parser, removed_names) =
      parse_trait_name_list
        parser
        (function Semicolon -> true | _ -> false)
    in
    Make.trait_use_precedence_item parser name keyword removed_names

  and parse_trait_use_alias_item parser aliasing_name =
    let (parser, keyword) =
      require_token parser As SyntaxError.expected_as_or_insteadof in
    let (parser, visibility, _) = parse_modifiers parser in
    let (parser, aliased_name) = parse_qualified_name_type_opt parser in
    Make.trait_use_alias_item parser aliasing_name keyword visibility
      aliased_name

  and parse_trait_use_conflict_resolution_item parser =
    let (parser, qualifier) = parse_qualified_name_type parser in
    let (parser, name) =
      if (peek_token_kind parser) = ColonColon then
        (* scope resolution expression case *)
        let (parser, cc_token) = require_coloncolon parser in
        let (parser, name) = require_token_one_of parser [Name; Construct] SyntaxError.error1004 in
        Make.scope_resolution_expression parser qualifier cc_token name
      else
        (* plain qualified name case *)
        (parser, qualifier) in
    match peek_token_kind parser with
    | Insteadof -> parse_trait_use_precedence_item parser name
    | As | _ -> parse_trait_use_alias_item parser name

  (*  SPEC:
    trait-use-conflict-resolution:
      use trait-name-list  {  trait-use-conflict-resolution-list  }

    trait-use-conflict-resolution-list:
      trait-use-conflict-resolution-item
      trait-use-conflict-resolution-item  trait-use-conflict-resolution-list

    trait-use-conflict-resolution-item:
      trait-use-alias-item
      trait-use-precedence-item

    trait-use-alias-item:
      trait-use-conflict-resolution-item-name  as  name;
      trait-use-conflict-resolution-item-name  as  visibility-modifier  name;
      trait-use-conflict-resolution-item-name  as  visibility-modifier;

    trait-use-precedence-item:
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
    let (parser, right_brace) =
     require_token parser RightBrace SyntaxError.error1006 in
    Make.trait_use_conflict_resolution
      parser
      use_token
      trait_name_list
      left_brace
      clauses
      right_brace

  (* SPEC:
    trait-use-clause:
      use  trait-name-list  ;

    trait-name-list:
      qualified-name  generic-type-parameter-listopt
      trait-name-list  ,  qualified-name  generic-type-parameter-listopt
  *)
  and parse_trait_name_list parser predicate =
    let (parser, items, _) =
      parse_separated_list_predicate
        parser
        Comma
        NoTrailing
        predicate
        SyntaxError.error1004
        parse_qualified_name_type
    in
    (parser, items)

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
      Make.trait_use parser use_token trait_name_list semi

  and parse_property_declaration
    ?(contains_abstract=false) parser attribute_spec modifiers =
    (* SPEC:
        property-declaration:
          attribute-spec-opt  property-modifier  type-specifier
            property-declarator-list  ;

       property-declarator-list:
         property-declarator
         property-declarator-list  ,  property-declarator
    *)
    (* The type specifier is optional in non-strict mode and required in
      strict mode. We give an error in a later pass. *)
    let (parser, prop_type) = match peek_token_kind parser with
      | Variable -> Make.missing parser (pos parser)
      | _ -> parse_type_specifier parser
    in
    let (parser, decls) =
      parse_comma_list parser Semicolon SyntaxError.error1008
        parse_property_declarator
    in
    let (parser, semi) = require_semicolon parser in
    let (parser, result) =
      Make.property_declaration
        parser attribute_spec modifiers prop_type decls semi
    in
    (* TODO: Move this to Full_fidelity_parser_errors. *)
    let parser =
      if contains_abstract then with_error parser SyntaxError.error2058
      else parser
    in
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
    Make.property_declarator parser name simple_init

  (* SPEC:
    const-declaration:
      abstract_opt  const  type-specifier_opt  constant-declarator-list  ;
      visibility  const  type-specifier_opt  constant-declarator-list  ;
    constant-declarator-list:
      constant-declarator
      constant-declarator-list  ,  constant-declarator
    constant-declarator:
      name  constant-initializer_opt
    constant-initializer:
      =  const-expression
  *)
  and parse_const_declaration parser visibility abstr const =
    let (parser, type_spec) =
      if is_type_in_const parser then
        parse_type_specifier parser
      else
        Make.missing parser (pos parser)
    in
    let (parser, const_list) = parse_comma_list
      parser Semicolon SyntaxError.error1004 parse_constant_declarator
    in
    let (parser, semi) = require_semicolon parser in
    Make.const_declaration
      parser
      visibility
      abstr
      const
      type_spec
      const_list
      semi

  and is_type_in_const parser =
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
    (* This permits abstract variables to have an initializer, and vice-versa.
       This is deliberate, and those errors will be detected after the syntax
       tree is created. *)
    let (parser, const_name) = require_name_allow_keywords parser in
    let (parser, initializer_) = parse_simple_initializer_opt parser in
    Make.constant_declarator parser const_name initializer_

  (* SPEC:
    type-constant-declaration:
      abstract-type-constant-declaration
      concrete-type-constant-declaration
    abstract-type-constant-declaration:
      abstract  const  type  name  type-constraintopt  ;
    concrete-type-constant-declaration:
      const  type  name  type-constraintopt  =  type-specifier  ;

    ERROR RECOVERY:

    An abstract type constant may only occur in an interface or an abstract
    class. We allow that to be parsed here, and the type checker detects the
    error.
    CONSIDER: We could detect this error in a post-parse pass; it is entirely
    syntactic.  Consider moving the error detection out of the type checker.

    An interface may not contain a non-abstract type constant that has a
    type constraint.  We allow that to be parsed here, and the type checker
    detects the error.
    CONSIDER: We could detect this error in a post-parse pass; it is entirely
    syntactic.  Consider moving the error detection out of the type checker.
  *)
  and parse_type_const_declaration parser abstr const =
    let (parser, type_token) = assert_token parser Type in
    let (parser, name) = require_name_allow_keywords parser in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser
    in
    let (parser, type_constraint) = parse_type_constraint_opt parser in
    let (parser, equal_token, type_specifier) = if SC.is_missing abstr then
      let (parser, equal_token) = require_equal parser in
      let (parser, type_spec) = parse_type_specifier parser in
      (parser, equal_token, type_spec)
    else
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      (parser, missing1, missing2)
    in
    let (parser, semicolon) = require_semicolon parser in
    Make.type_const_declaration
      parser
      abstr
      const
      type_token
      name
      generic_type_parameter_list
      type_constraint
      equal_token
      type_specifier
      semicolon

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
   (*
   TODO: The list of attrs can have a trailing comma. Update the spec.
   TODO: The list of values can have a trailing comma. Update the spec.
   (Both these work items are tracked by spec issue 106.) *)

  and parse_attribute_specification_opt parser =
    if peek_token_kind parser = LessThanLessThan then
      let (parser, left, items, right) =
        parse_double_angled_comma_list_allow_trailing parser parse_attribute
      in
      Make.attribute_specification parser left items right
    else
      Make.missing parser (pos parser)

  and parse_attribute parser =
    let (parser, name) = require_name parser in
    let (parser, left, items, right) =
      if peek_token_kind parser = LeftParen then
        parse_parenthesized_comma_list_opt_allow_trailing
          parser parse_expression
      else
        let (parser, missing1) = Make.missing parser (pos parser) in
        let (parser, missing2) = Make.missing parser (pos parser) in
        let (parser, missing3) = Make.missing parser (pos parser) in
        (parser, missing1, missing2, missing3)
    in
    Make.attribute parser name left items right

  and parse_generic_type_parameter_list_opt parser =
    let (_parser1, open_angle) = next_token parser in
    let kind = Token.kind open_angle in
    if kind = LessThan then
      with_type_parser parser TypeParser.parse_generic_type_parameter_list
    else
      Make.missing parser (pos parser)

  and parse_return_type_hint_opt parser =
    let (parser1, colon_token) = next_token parser in
    if (Token.kind colon_token) = Colon then
      let (parser, colon_token) = Make.token parser1 colon_token in
      let (parser, return_type) =
        with_type_parser parser TypeParser.parse_return_type
      in
      (parser, colon_token, return_type)
    else
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      (parser, missing1, missing2)

  and parse_parameter_list_opt parser =
      (* SPEC
        TODO: The specification is wrong in several respects concerning
        variadic parameters. Variadic parameters are permitted to have a
        type and name but this is not mentioned in the spec. And variadic
        parameters are not mentioned at all in the grammar for constructor
        parameter lists.  (This is tracked by spec issue 107.)

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
        or followed by a trailing comma, or if the parameter has a
        default value.  *)
      parse_parenthesized_comma_list_opt_allow_trailing parser parse_parameter

  and parse_parameter parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | DotDotDot ->
      let next_kind = peek_token_kind parser1 in
      if next_kind = Variable then parse_parameter_declaration parser
      else
        let pos = pos parser in
        let (parser, missing1) = Make.missing parser1 pos in
        let (parser, missing2) = Make.missing parser pos in
        let (parser, token) = Make.token parser token in
        Make.variadic_parameter parser missing1 missing2 token
    | _ -> parse_parameter_declaration parser

  and parse_parameter_declaration parser =
    (* SPEC

      TODO: Add call-convention-opt to the specification.
      (This work is tracked by task T22582676.)

      TODO: Update grammar for inout parameters.
      (This work is tracked by task T22582715.)

      parameter-declaration:
        attribute-specification-opt \
        call-convention-opt \
        type-specifier  variable-name \
        default-argument-specifier-opt
    *)
    (* ERROR RECOVERY
      * In strict mode, we require a type specifier. This error is not caught
        at parse time but rather by a later pass.
      * Visibility modifiers are only legal in constructor parameter
        lists; we give an error in a later pass.
      * Variadic params cannot be declared inout; we permit that here but
        give an error in a later pass.
      * Variadic params and inout params cannot have default values; these
        errors are also reported in a later pass.
    *)
    let (parser, attrs) = parse_attribute_specification_opt parser in
    let (parser, visibility) = parse_visibility_modifier_opt parser in
    let (parser, callconv) = parse_call_convention_opt parser in
    let token = peek_token parser in
    let (parser, type_specifier) =
      match Token.kind token with
        | Variable | DotDotDot | Ampersand -> Make.missing parser (pos parser)
        | _ -> parse_type_specifier parser
    in
    let (parser, name) = parse_decorated_variable_opt parser in
    let (parser, default) = parse_simple_initializer_opt parser in
    Make.parameter_declaration
      parser
      attrs
      visibility
      callconv
      type_specifier
      name
      default

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
    (* ERROR RECOVERY
       Detection of (variadic, byRef) inout params happens in post-parsing.
       Although a parameter can have at most one variadic/reference decorator,
       we deliberately allow multiple decorators in the initial parse and produce
       an error in a later pass.
     *)

    let (parser, decorator) = fetch_token parser in
    let (parser, variable) =
      match peek_token_kind parser with
      | DotDotDot
      | Ampersand -> parse_decorated_variable parser
      | _ -> require_variable parser
    in
    Make.decorated_expression parser decorator variable

  and parse_visibility_modifier_opt parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Public | Protected | Private -> Make.token parser1 token
    | _ -> Make.missing parser (pos parser)

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
      let (parser, token) = Make.token parser1 token in
      let (parser, default_value) = parse_expression parser in
      Make.simple_initializer parser token default_value
    | _ -> Make.missing parser (pos parser)

  and parse_function_declaration parser attribute_specification =
    let (parser, modifiers, _) = parse_modifiers parser in
    let (parser, header) =
      parse_function_declaration_header parser ~is_methodish:false modifiers in
    let (parser, body) = parse_compound_statement parser in
    Make.function_declaration parser attribute_specification header body

  and parse_constraint_operator parser =
    (* TODO: Put this in the specification
    (This work is tracked by spec issue #100.)
      constraint-operator:
        =
        as
        super
    *)
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Equal
    | As
    | Super -> Make.token parser1 token
    | _ -> (* ERROR RECOVERY: don't eat the offending token. *)
      (* TODO: Give parse error *)
      Make.missing parser (pos parser)

  and parse_where_constraint parser =
    (* TODO: Put this in the specification
    (This work is tracked by spec issue #100.)
    constraint:
      type-specifier  constraint-operator  type-specifier

    *)
    let (parser, left) = parse_type_specifier parser in
    let (parser, op) = parse_constraint_operator parser in
    let (parser, right) = parse_type_specifier parser in
    Make.where_constraint parser left op right

  and parse_where_constraint_list_item parser =
    match peek_token_kind parser with
    | Semicolon | LeftBrace ->
      (parser, None)
    | _ ->
      let (parser, where_constraint) = parse_where_constraint parser in
      let (parser, comma) = optional_token parser Comma in
      let (parser, result) = Make.list_item parser where_constraint comma in
      (parser, Some result)

  and parse_where_clause parser =
    (* TODO: Add this to the specification
    (This work is tracked by spec issue #100.)
      where-clause:
        where   constraint-list

      constraint-list:
        constraint
        constraint-list , constraint
    *)
    let (parser, keyword) = assert_token parser Where in
    let (parser, constraints) = parse_list_until_none
      parser parse_where_constraint_list_item in
    Make.where_clause parser keyword constraints

  and parse_where_clause_opt parser =
    if peek_token_kind parser != Where then
      Make.missing parser (pos parser)
    else
      parse_where_clause parser

  and parse_function_declaration_header parser modifiers ~is_methodish =
    (* SPEC
      function-definition-header:
        attribute-specification-opt  async-opt  coroutine-opt  function  name  /
        generic-type-parameter-list-opt  (  parameter-list-opt  ) :  /
        return-type   where-clause-opt
      TODO: The spec does not specify "where" clauses. Add them.
      (This work is tracked by spec issue #100.)
    *)
    (* In strict mode, we require a type specifier. This error is not caught
       at parse time but rather by a later pass. *)
    (* In non-strict mode we allow an & to appear before the name.
       In strict mode this produces an error during post-parsing. *)
    let (parser, function_token) = require_function parser in
    let (parser, ampersand_token) = optional_token parser Ampersand in
    let (parser, label) =
      parse_function_label_opt parser ~is_methodish in
    let (parser, generic_type_parameter_list) =
      parse_generic_type_parameter_list_opt parser in
    let (parser, left_paren_token, parameter_list, right_paren_token) =
      parse_parameter_list_opt parser in
    let (parser, colon_token, return_type) =
      parse_return_type_hint_opt parser in
    let (parser, where_clause) = parse_where_clause_opt parser in
    Make.function_declaration_header
      parser
      modifiers
      function_token
      ampersand_token
      label
      generic_type_parameter_list
      left_paren_token
      parameter_list
      right_paren_token
      colon_token
      return_type
      where_clause

  (* A function label is either a function name, a __construct label, or a
  __destruct label. *)
  and parse_function_label_opt parser ~is_methodish =
    let report_error parser token =
      let parser = with_error parser SyntaxError.error1044 in
      let (parser, token) = Make.token parser token in
      Make.error parser token
    in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Name
    | Construct
    | Destruct -> Make.token parser1 token
    | LeftParen ->
      (* It turns out, it was just a verbose lambda; YOLO PHP *)
      Make.missing parser (pos parser)
    | Trait | Interface | Class | Static | Using | Inout
    | Instanceof | Array | Throw | Print | As | And
    | Or | Xor | New | Const | Eval
      when not is_methodish ->
      (* these are illegal for function names *)
      (* ERROR RECOVERY: Eat the offending token. *)
      report_error parser1 token
    | _ ->
      (* TODO: We might have a non-reserved keyword as the name here
      What we do here is accept any keyword;
      what we *should* do is figure out which other keywords are
      reserved and which are not, and reject the reserved keywords. *)
      let (parser, token) = next_token_as_name parser in
      if Token.kind token = Name then
        Make.token parser token
      else
        (* ERROR RECOVERY: Eat the offending token. *)
        report_error parser token

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
    if peek_token_kind ~lookahead:1 parser = Const then
      let kind1 = peek_token_kind ~lookahead:2 parser in
      let kind2 = peek_token_kind ~lookahead:3 parser in
      match kind1, kind2 with
      | Type, Semicolon ->
        let (parser, missing) = Make.missing parser (pos parser) in
        let (parser, abstr) = assert_token parser Abstract in
        let (parser, const) = assert_token parser Const in
        parse_const_declaration parser missing abstr const
      | Type, _ when kind2 <> Equal ->
        let (parser, abstr) = assert_token parser Abstract in
        let (parser, const) = assert_token parser Const in
        parse_type_const_declaration parser abstr const
      | _, _ ->
        let (parser, missing) = Make.missing parser (pos parser) in
        let (parser, abstr) = assert_token parser Abstract in
        let (parser, const) = assert_token parser Const in
        parse_const_declaration parser missing abstr const
    else
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, modifiers, _) = parse_modifiers parser in
      parse_methodish parser missing modifiers

  and parse_methodish parser attribute_spec modifiers =
    let (parser, header) =
      parse_function_declaration_header parser modifiers ~is_methodish:true in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | LeftBrace ->
      let (parser, body) = parse_compound_statement parser in
      let (parser, missing) = Make.missing parser (pos parser) in
      Make.methodish_declaration parser attribute_spec header body missing
    | Semicolon ->
      let (parser, missing) = Make.missing parser1 (pos parser) in
      let (parser, semicolon) = Make.token parser token in
      Make.methodish_declaration parser attribute_spec header missing semicolon
    | _ ->
      (* ERROR RECOVERY: We expected either a block or a semicolon; we got
      neither. Use the offending token as the body of the method.
      TODO: Is this the right error recovery? *)
      let pos = pos parser in
      let parser = with_error parser1 SyntaxError.error1041 in
      let (parser, token) = Make.token parser token in
      let (parser, error) = Make.error parser token in
      let (parser, missing) = Make.missing parser pos in
      Make.methodish_declaration parser attribute_spec header error missing

  and parse_modifiers parser =
    let rec aux parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | Abstract
      | Static
      | Public
      | Protected
      | Private
      | Async
      | Coroutine
      | Final ->
        let (parser, item) = Make.token parser1 token in
        aux parser (item :: acc)
      | _ -> (parser, List.rev acc)
    in
    let (parser, items) = aux parser [] in
    let contains_abstract = List.exists SC.is_abstract items in
    let (parser, items_list) = make_list parser items in
    (parser, items_list, contains_abstract)

  and parse_enum_or_classish_or_function_declaration parser =
    (* An enum, type alias, function, interface, trait or class may all
      begin with an attribute. *)
    let parser1, attribute_specification =
      parse_attribute_specification_opt parser in
    let parser2, token = next_token parser1 in
    match Token.kind token with
    | Enum -> parse_enum_declaration parser1 attribute_specification
    | Type | Newtype -> parse_alias_declaration parser1 attribute_specification
    | Async | Coroutine | Function ->
      if SC.is_missing attribute_specification then
        (* if attribute section is missing - it might be either
           function declaration or expression statement containing
           anonymous function - use statement parser to determine in which case
           we are currently in *)
        with_statement_parser
          parser
          StatementParser.parse_possible_php_function
      else
        parse_function_declaration parser1 attribute_specification
    | Abstract
    | Final
    | Interface
    | Trait
    | Class -> parse_classish_declaration parser1 attribute_specification
    | _ ->
      (* ERROR RECOVERY TODO: Produce an error here. *)
      (* TODO: This is wrong; we have lost the attribute specification
      from the tree. *)
      let (parser, token) = Make.token parser2 token in
      Make.error parser token

  and parse_declaration parser =
    let recovery_tokens = [ Class; Trait; Interface ] in
    let parser = Parser.expect_in_new_scope parser recovery_tokens in
    let (parser1, token) = next_token parser in
    let (parser, result) =
      match (Token.kind token) with
      | Include
      | Include_once
      | Require
      | Require_once -> parse_inclusion_directive parser
      | Type
      | Newtype when
         match peek_token_kind parser1 with
         | Name | Classname -> true
         | _ -> false
      ->
        let (parser, missing) = Make.missing parser (pos parser) in
        parse_alias_declaration parser missing
      | Enum ->
        let (parser, missing) = Make.missing parser (pos parser) in
        parse_enum_declaration parser missing
      (* The keyword namespace before a name should be parsed as
        "the current namespace we are in", essentially a no op.
        example:
        namespace\f1(); should be parsed as a call to the function f1 in
        the current namespace.      *)
      | Namespace when peek_token_kind parser1 = Backslash ->
        with_statement_parser parser StatementParser.parse_statement
      | Namespace -> parse_namespace_declaration parser
      | Use -> parse_namespace_use_declaration parser
      | Trait
      | Interface
      | Abstract
      | Final
      | Class ->
        let (parser, missing) = Make.missing parser (pos parser) in
        parse_classish_declaration parser missing
      | Async
      | Coroutine
      | Function ->
        with_statement_parser
          parser
          StatementParser.parse_possible_php_function
      | LessThanLessThan ->
        parse_enum_or_classish_or_function_declaration parser
        (* TODO figure out what global const differs from class const *)
      | Const ->
        let pos = pos parser in
        let (parser, missing1) = Make.missing parser1 pos in
        let (parser, missing2) = Make.missing parser pos in
        let (parser, token) = Make.token parser token in
        parse_const_declaration parser missing1 missing2 token
      | _ ->
        with_statement_parser parser StatementParser.parse_statement
        (* TODO: What if it's not a legal statement? Do we still make progress
        here? *)
    in
    let parser1 = Parser.pop_scope parser recovery_tokens in
    (parser1, result)

  let parse_leading_markup_section parser =
    let (parser1, (markup_section, has_suffix)) =
      with_statement_parser parser
      (fun p ->
        let (p, s, has_suffix) =
          StatementParser.parse_markup_section ~is_leading_section:true p
        in
        p, (s, has_suffix)
      )
    in
      (* proceed successfully if we've consumed <?... *)
      (* We purposefully ignore leading trivia before the <?hh, and handle
      the error on a later pass *)
      (* TODO: Handle the case where the langauge is not a Name. *)
    (* Do not attempt to recover in HHVM compatibility mode *)
    if has_suffix || Env.hhvm_compat_mode (env parser) then
      parser1, markup_section
    else
      (* ERROR RECOVERY *)
      (* Make no progress; try parsing the file without a header *)
      let parser = with_error parser SyntaxError.error1001 in
      let (parser, missing1) = Make.missing parser (pos parser) in
      let (parser, missing2) = Make.missing parser (pos parser) in
      let (parser, missing3) = Make.missing parser (pos parser) in
      let (parser, missing4) = Make.missing parser (pos parser) in
      Make.markup_section parser missing1 missing2 missing3 missing4

  let parse_script parser =
    let rec aux parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | TokenKind.EndOfFile ->
        let (parser, token) = Make.token parser1 token in
        let (parser, end_of_file) = Make.end_of_file parser token in
        (parser, end_of_file :: acc)
      | _ ->
        let (parser, declaration) = parse_declaration parser in
        aux parser (declaration :: acc)
    in
    (* parse leading markup section *)
    let (parser, header) = parse_leading_markup_section parser in
    let (parser, declarations) = aux parser [] in
    (* include leading markup section as a head of declaration list *)
    let (parser, declarations) =
      make_list parser (header :: List.rev declarations)
    in
    let (parser, result) = Make.script parser declarations in
    (* If we are not at the end of the file, something is wrong. *)
    assert ((peek_token_kind parser) = TokenKind.EndOfFile);
    (parser, result)

end (* WithExpressionAndStatementAndTypeParser *)
end (* WithSmartConstructors *)
end (* WithSyntax *)
