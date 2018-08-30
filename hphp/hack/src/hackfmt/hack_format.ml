(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Format_env
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_editable_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module Token = Full_fidelity_editable_token
module TokenKind = Full_fidelity_token_kind
module Trivia = Full_fidelity_editable_trivia
module TriviaKind = Full_fidelity_trivia_kind
module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)

open Hh_core
open Doc

let make_list = Syntax.make_list SourceText.empty 0
let make_missing () = Syntax.make_missing SourceText.empty 0

(* Main transform function, which takes a full-fidelity CST node and produces a
 * Doc.t node (the IR which is fed to Chunk_builder.build).
 *
 * Exported via the `transform` alias below. *)
let rec t (env: Env.t) (node: Syntax.t) : Doc.t =
  (* Leave this node as it was in the original source if it is preceded by a
     hackfmt-ignore comment. *)
  match transform_node_if_ignored node with
  | Some doc -> doc
  | None ->
  match Syntax.syntax node with
  | Syntax.Missing ->
    Nothing
  | Syntax.Token x ->
    let token_kind = Token.kind x in
    Concat [
      begin
        match token_kind with
        | TokenKind.EndOfFile ->
          let leading_trivia = Token.leading x in
          let trivia_without_trailing_invisibles =
            let reversed = List.rev leading_trivia in
            List.rev (List.drop_while reversed ~f:is_invisible)
          in
          transform_leading_trivia trivia_without_trailing_invisibles
        | _ -> transform_leading_trivia (Token.leading x)
      end;
      begin
        match token_kind with
        | TokenKind.EndOfFile -> Nothing
        | TokenKind.SingleQuotedStringLiteral
        | TokenKind.DoubleQuotedStringLiteral
        | TokenKind.DoubleQuotedStringLiteralHead
        | TokenKind.StringLiteralBody
        | TokenKind.DoubleQuotedStringLiteralTail
        | TokenKind.HeredocStringLiteral
        | TokenKind.HeredocStringLiteralHead
        | TokenKind.HeredocStringLiteralTail
        | TokenKind.NowdocStringLiteral ->
          make_string (Token.text x) (Token.width x)
        | _ -> Text (Token.text x, Token.width x)
      end;
      transform_trailing_trivia (Token.trailing x);
    ]
  | Syntax.SyntaxList _ ->
    failwith (Printf.sprintf
      "Error: SyntaxList should never be handled directly;
      offending text is '%s'." (Syntax.text node));
  | Syntax.EndOfFile x ->
    t env x.end_of_file_token
  | Syntax.Script x ->
    begin match Syntax.syntax x.script_declarations with
    | Syntax.SyntaxList (header::declarations)
      when Syntax.is_markup_section header ->
      Concat [
        t env header;
        Newline;
        handle_list env declarations;
      ]
    | _ ->
      Concat [
        handle_possible_list env x.script_declarations;
      ]
    end
  | Syntax.LiteralExpression { literal_expression } ->
    (* Double quoted string literals can create a list *)
    let wrap_with_literal_type token transformed =
      match Token.kind token with
      | TokenKind.HeredocStringLiteral
      | TokenKind.HeredocStringLiteralHead
      | TokenKind.HeredocStringLiteralTail
      | TokenKind.NowdocStringLiteral -> DocLiteral transformed
      | TokenKind.DecimalLiteral
      | TokenKind.OctalLiteral
      | TokenKind.HexadecimalLiteral
      | TokenKind.BinaryLiteral
      | TokenKind.FloatingLiteral -> NumericLiteral transformed
      | _ -> transformed
    in
    begin match Syntax.syntax literal_expression with
      | Syntax.Token tok ->
        wrap_with_literal_type tok (t env literal_expression)
      | Syntax.SyntaxList l ->
        let last = Syntax.trailing_token literal_expression in
        begin match last with
          | Some tok -> wrap_with_literal_type tok (Concat (List.map l (t env)))
          | _ -> failwith "Expected Token"
        end
      | _ -> failwith "Expected Token or SyntaxList"
    end
  | Syntax.PrefixedStringExpression {
      prefixed_string_name = name;
      prefixed_string_str = str } ->
      Concat [
        t env name;
        t env str;
      ]
  | Syntax.MarkupSection {
      markup_prefix = prefix;
      markup_text = text;
      markup_suffix = suffix;
      _ } ->
    if Syntax.is_missing prefix
    then
      (* leading markup section
         for hh files - strip leading whitespaces\newlines - they are not
         emitted and having them in Hack file is illegal anyways *)
      let is_hh_script = match Syntax.syntax suffix with
        | Syntax.MarkupSuffix { markup_suffix_name = Syntax.{
            syntax = Token t; _
          }; _ } ->
          (Token.text t) = "hh"
        | _ -> false
      in
      let rec all_whitespaces s i =
        i >= String.length s
        || (match String.get s i with
            | ' ' | '\t' | '\r' | '\n' -> all_whitespaces s (i + 1)
            | _ -> false)
      in
      let text_contains_only_whitespaces = match Syntax.syntax text with
        | Syntax.Token t -> all_whitespaces (Token.text t) 0
        | _ -> false
      in
      if is_hh_script && text_contains_only_whitespaces
      then t env suffix
      else transform_simple env node
    else transform_simple env node
  | Syntax.MarkupSuffix _
  | Syntax.SimpleTypeSpecifier _
  | Syntax.VariableExpression _
  | Syntax.PipeVariableExpression _
  | Syntax.PropertyDeclarator _
  | Syntax.ConstantDeclarator _
  | Syntax.StaticDeclarator _
  | Syntax.ScopeResolutionExpression _
  | Syntax.EmbeddedMemberSelectionExpression _
  | Syntax.EmbeddedSubscriptExpression _
  | Syntax.PostfixUnaryExpression _
  | Syntax.XHPRequired _
  | Syntax.XHPSimpleClassAttribute _
  | Syntax.XHPClose _
  | Syntax.TypeConstant _
  | Syntax.GenericTypeSpecifier _
  | Syntax.NullableTypeSpecifier _
  | Syntax.SoftTypeSpecifier _
  | Syntax.ListItem _ ->
    transform_simple env node
  | Syntax.ReifiedTypeArgument
    { reified_type_argument_reified; reified_type_argument_type } ->
    Concat [
      t env reified_type_argument_reified;
      Space;
      t env reified_type_argument_type;
    ]
  | Syntax.QualifiedName { qualified_name_parts; } ->
    handle_possible_list env qualified_name_parts
  | Syntax.ExpressionStatement _ ->
    transform_simple_statement env node
  | Syntax.EnumDeclaration {
      enum_attribute_spec = attr;
      enum_keyword = kw;
      enum_name = name;
      enum_colon = colon_kw;
      enum_base = base;
      enum_type = enum_type;
      enum_left_brace = left_b;
      enum_enumerators = enumerators;
      enum_right_brace = right_b } ->
    Concat [
      t env attr;
      when_present attr newline;
      t env kw;
      Space;
      t env name;
      t env colon_kw;
      Space;
      SplitWith Cost.Base;
      Nest [
        Space;
        t env base;
        Space;
        t env enum_type;
        Space;
      ];
      braced_block_nest env left_b right_b [
        handle_possible_list env enumerators
      ];
      Newline;
    ]
  | Syntax.Enumerator {
      enumerator_name = name;
      enumerator_equal = eq_kw;
      enumerator_value = value;
      enumerator_semicolon = semi } ->
    let value = t env value in
    Concat [
      t env name;
      Space;
      t env eq_kw;
      Space;
      if has_split value then SplitWith Cost.Base else Nothing;
      Nest [value];
      t env semi;
      Newline;
    ]
  | Syntax.AliasDeclaration {
      alias_attribute_spec = attr;
      alias_keyword = kw;
      alias_name = name;
      alias_generic_parameter = generic;
      alias_constraint = type_constraint;
      alias_equal = eq_kw;
      alias_type = alias_type;
      alias_semicolon = semi } ->
    (* TODO: revisit this for long names *)
    Concat [
      t env attr;
      when_present attr newline;
      t env kw;
      Space;
      t env name;
      t env generic;
      Space;
      t env type_constraint;
      Space;
      t env eq_kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env alias_type];
      t env semi;
      Newline;
    ]
  | Syntax.PropertyDeclaration {
      property_attribute_spec = attr;
      property_modifiers = modifiers;
      property_type = prop_type;
      property_declarators = declarators;
      property_semicolon = semi } ->
    Concat [
      t env attr;
      when_present attr newline;
      handle_possible_list env ~after_each:(fun _ -> Space) modifiers;
      t env prop_type;
      handle_declarator_list env declarators;
      t env semi;
      Newline;
    ]
  | Syntax.NamespaceDeclaration {
      namespace_keyword = kw;
      namespace_name = name;
      namespace_body = body } ->
    Concat [
      t env kw;
      Space;
      t env name;
      t env body;
      Newline;
    ]
  | Syntax.NamespaceBody {
      namespace_left_brace = left_b;
      namespace_declarations = decls;
      namespace_right_brace = right_b } ->
    Concat [
      Space;
      braced_block_nest env left_b right_b [handle_possible_list env decls];
    ]
  | Syntax.NamespaceEmptyBody {
      namespace_semicolon = semi } ->
    Concat [
      t env semi;
    ]
  | Syntax.NamespaceUseDeclaration {
      namespace_use_keyword = kw;
      namespace_use_kind = use_kind;
      namespace_use_clauses = clauses;
      namespace_use_semicolon = semi } ->
    Concat [
      t env kw;
      Space;
      t env use_kind;
      when_present use_kind space;
      WithRule (Rule.Parental, Nest [
        handle_possible_list env clauses ~after_each:after_each_argument;
      ]);
      t env semi;
      Newline;
    ]
  | Syntax.NamespaceGroupUseDeclaration {
      namespace_group_use_keyword = kw;
      namespace_group_use_kind = use_kind;
      namespace_group_use_prefix = prefix;
      namespace_group_use_left_brace = left_b;
      namespace_group_use_clauses = clauses;
      namespace_group_use_right_brace = right_b;
      namespace_group_use_semicolon = semi } ->
    Concat [
      t env kw;
      Space;
      t env use_kind;
      when_present use_kind space;
      t env prefix;
      transform_argish env left_b clauses right_b;
      t env semi;
      Newline;
    ]
  | Syntax.NamespaceUseClause {
      namespace_use_clause_kind = use_kind;
      namespace_use_name = name;
      namespace_use_as = as_kw;
      namespace_use_alias = alias } ->
    Concat [
      t env use_kind;
      when_present use_kind space;
      t env name;
      when_present as_kw space;
      t env as_kw;
      when_present alias space;
      t env alias;
    ]
  | Syntax.FunctionDeclaration {
      function_attribute_spec = attr;
      function_declaration_header = header;
      function_body = body } ->
    Concat [
      t env attr;
      when_present attr newline;
      t env header;
      handle_possible_compound_statement env ~allow_collapse:true body;
      Newline;
    ]
  | Syntax.FunctionDeclarationHeader {
      function_modifiers = modifiers;
      function_keyword = kw;
      function_ampersand = amp;
      function_name = name;
      function_type_parameter_list = type_params;
      function_left_paren = leftp;
      function_parameter_list = params;
      function_right_paren = rightp;
      function_colon = colon;
      function_type = ret_type;
      function_where_clause = where } ->
    Concat [
      Span (
        transform_fn_decl_name env modifiers kw amp name type_params leftp);
      transform_fn_decl_args env params rightp colon ret_type where;
    ]
  | Syntax.WhereClause {
      where_clause_keyword = where;
      where_clause_constraints = constraints } ->
    Concat [
      t env where;
      Space;
      handle_possible_list env constraints ~after_each:(fun _ -> Space);
    ]
  | Syntax.WhereConstraint {
      where_constraint_left_type = left;
      where_constraint_operator = op;
      where_constraint_right_type = right } ->
    Concat [
      t env left;
      Space;
      t env op;
      Space;
      t env right;
    ]
  | Syntax.MethodishDeclaration {
      methodish_attribute = attr;
      methodish_function_decl_header = func_decl;
      methodish_function_body = body;
      methodish_semicolon = semi } ->
    Concat [
      t env attr;
      when_present attr newline;
      (
        let fn_name, args_and_where = match Syntax.syntax func_decl with
          | Syntax.FunctionDeclarationHeader {
              function_modifiers = modifiers;
              function_keyword = kw;
              function_ampersand = amp;
              function_name = name;
              function_type_parameter_list = type_params;
              function_left_paren = leftp;
              function_parameter_list = params;
              function_right_paren = rightp;
              function_colon = colon;
              function_type = ret_type;
              function_where_clause = where } ->
            Concat (
              transform_fn_decl_name env
                modifiers
                kw
                amp
                name
                type_params
                leftp
            ),
            transform_fn_decl_args env params rightp colon ret_type where
          | _ -> failwith "Expected FunctionDeclarationHeader"
        in
        Concat [
          Span [fn_name];
          args_and_where;
        ]
      );
      when_present body (fun () ->
        handle_possible_compound_statement env ~allow_collapse:true body
      );
      t env semi;
      Newline;
    ]
  | Syntax.ClassishDeclaration {
      classish_attribute = attr;
      classish_modifiers = modifiers;
      classish_keyword = kw;
      classish_name = name;
      classish_type_parameters = type_params;
      classish_extends_keyword = extends_kw;
      classish_extends_list = extends;
      classish_implements_keyword = impl_kw;
      classish_implements_list = impls;
      classish_body = body } ->
    let after_each_ancestor is_last =
      if is_last then Nothing else space_split () in
    Concat [
      t env attr;
      when_present attr newline;
      Span [
        handle_possible_list env ~after_each:(fun _ -> Space) modifiers;
        t env kw;
        Space;
        SplitWith Cost.Base;
        Nest [
          t env name;
          t env type_params;
        ];
      ];

      WithRule (Rule.Parental, Concat [
        when_present extends_kw (fun () -> Nest [
          Space;
          Split;
          t env extends_kw;
          WithRule (Rule.Parental, Nest [ Span [
            Space;
            if list_length extends = 1
            then SplitWith Cost.Base
            else Split;
            Nest [
              handle_possible_list env ~after_each:after_each_ancestor extends
            ]
          ]])
        ]);

        when_present impl_kw (fun () -> Nest [
          Space;
          Split;
          t env impl_kw;
          WithRule (Rule.Parental, Nest [ Span [
            Space;
            if list_length impls = 1
            then SplitWith Cost.Base
            else Split;
            Nest [
              handle_possible_list env ~after_each:after_each_ancestor impls
            ]
          ]])
        ]);
      ]);
      t env body;
    ]
  | Syntax.ClassishBody {
      classish_body_left_brace = left_b;
      classish_body_elements = body;
      classish_body_right_brace = right_b } ->
    Concat [
      Space;
      braced_block_nest env left_b right_b [
        handle_possible_list env body
      ];
      Newline;
    ]
  | Syntax.TraitUsePrecedenceItem {
      trait_use_precedence_item_name = name;
      trait_use_precedence_item_keyword = kw;
      trait_use_precedence_item_removed_names = removed_names } ->
    Concat [
      t env name;
      Space;
      t env kw;
      Space;
      WithRule (Rule.Parental, Nest [
        handle_possible_list env ~before_each:space_split removed_names;
      ]);
    ]
  | Syntax.TraitUseAliasItem {
      trait_use_alias_item_aliasing_name = aliasing_name;
      trait_use_alias_item_keyword = kw;
      trait_use_alias_item_modifiers = visibility;
      trait_use_alias_item_aliased_name = aliased_name } ->
    Concat [
      t env aliasing_name;
      Space;
      t env kw;
      Space;
      t env visibility;
      Space;
      t env aliased_name;
    ]
  | Syntax.TraitUseConflictResolution {
      trait_use_conflict_resolution_keyword = kw;
      trait_use_conflict_resolution_names = elements;
      trait_use_conflict_resolution_left_brace = lb;
      trait_use_conflict_resolution_clauses = clauses;
      trait_use_conflict_resolution_right_brace = rb } ->
    Concat [
      t env kw;
      WithRule (Rule.Parental, Nest [
        handle_possible_list env ~before_each:space_split elements;
      ]);
      Space;
      t env lb;
      Newline;
      Nest [
        handle_possible_list env ~before_each:newline clauses;
      ];
      Newline;
      t env rb;
    ]
  | Syntax.TraitUse {
      trait_use_keyword = kw;
      trait_use_names = elements;
      trait_use_semicolon = semi } ->
    Concat [
      t env kw;
      (match Syntax.syntax elements with
      | Syntax.SyntaxList [x] -> Concat [Space; t env x]
      | Syntax.SyntaxList _ ->
        WithRule (Rule.Parental, Nest [
          handle_possible_list env ~before_each:space_split elements;
        ])
      | _ -> Concat [Space; t env elements]
      );
      t env semi;
      Newline;
    ]
  | Syntax.RequireClause {
      require_keyword = kw;
      require_kind = kind;
      require_name = name;
      require_semicolon = semi } ->
    let name = t env name in
    Concat [
      t env kw;
      Space;
      t env kind;
      Space;
      if has_split name then SplitWith Cost.High else Nothing;
      Nest [
        name;
        t env semi;
      ];
      Newline;
    ]
  | Syntax.ConstDeclaration {
      const_visibility = _; (* not supported in Hack *)
      const_abstract = abstr;
      const_keyword = kw;
      const_type_specifier = const_type;
      const_declarators = declarators;
      const_semicolon = semi } ->
    Concat [
      t env abstr;
      when_present abstr space;
      t env kw;
      when_present const_type space;
      t env const_type;
      WithRule (Rule.Parental, Nest [
        handle_possible_list env ~before_each:space_split declarators;
      ]);
      t env semi;
      Newline;
    ]
  | Syntax.TypeConstDeclaration {
      type_const_abstract = abs;
      type_const_keyword = kw;
      type_const_type_keyword = type_kw ;
      type_const_name = name;
      type_const_type_parameters = type_params;
      type_const_type_constraint = type_constraint;
      type_const_equal = eq;
      type_const_type_specifier = type_spec;
      type_const_semicolon = semi } ->
    Concat [
      t env abs;
      Space;
      t env kw;
      Space;
      t env type_kw;
      Space;
      t env name;
      t env type_params;
      when_present type_constraint space;
      t env type_constraint;
      when_present eq space;
      t env eq;
      when_present type_spec (fun _ -> Concat [
        Space;
        SplitWith Cost.Base;
        Nest [t env type_spec];
      ]);
      t env semi;
      Newline;
    ]
  | Syntax.ParameterDeclaration {
      parameter_attribute = attr;
      parameter_visibility = visibility;
      parameter_call_convention = callconv;
      parameter_type = param_type;
      parameter_name = name;
      parameter_default_value = default } ->
    Concat [
      t env attr;
      t env visibility;
      when_present visibility space;
      t env callconv;
      when_present callconv space;
      t env param_type;
      if Syntax.is_missing visibility
      && Syntax.is_missing callconv
      && Syntax.is_missing param_type
      then t env name
      else Concat [
        Space;
        SplitWith Cost.Base;
        Nest [t env name];
      ];
      t env default;
    ]
  | Syntax.VariadicParameter {
      variadic_parameter_call_convention = callconv;
      variadic_parameter_type = type_var;
      variadic_parameter_ellipsis = ellipsis } ->
    Concat [
      t env callconv;
      when_present callconv space;
      t env type_var;
      t env ellipsis;
    ]
  | Syntax.AttributeSpecification {
      attribute_specification_left_double_angle = left_da;
      attribute_specification_attributes = attrs;
      attribute_specification_right_double_angle = right_da; } ->
    transform_argish env ~allow_trailing:false left_da attrs right_da
  | Syntax.InclusionExpression {
      inclusion_require = kw;
      inclusion_filename = expr; } ->
    Concat [
      t env kw;
      (match Syntax.syntax expr with
      | Syntax.ParenthesizedExpression _ -> Nothing
      | _ -> Space
      );
      SplitWith Cost.Base;
      t env expr;
    ]
  | Syntax.InclusionDirective {
      inclusion_expression = expr;
      inclusion_semicolon = semi; } ->
    Concat [
      t env expr;
      t env semi;
      Newline;
    ]
  | Syntax.CompoundStatement {
      compound_left_brace;
      compound_statements;
      compound_right_brace; } ->
    Concat [
      handle_compound_statement env
        compound_left_brace
        compound_statements
        compound_right_brace;
      Newline;
    ]
  | Syntax.AlternateLoopStatement {
      alternate_loop_opening_colon;
      alternate_loop_statements;
      alternate_loop_closing_keyword;
      alternate_loop_closing_semicolon; } ->
    Concat [
      handle_alternate_loop_statement env
        alternate_loop_opening_colon
        alternate_loop_statements
        alternate_loop_closing_keyword
        alternate_loop_closing_semicolon;
      Newline;
    ]
  | Syntax.UnsetStatement {
      unset_keyword = kw;
      unset_left_paren = left_p;
      unset_variables = args;
      unset_right_paren = right_p;
      unset_semicolon = semi; } ->
    Concat [
      t env kw;
      transform_argish env ~allow_trailing:false left_p args right_p;
      t env semi;
      Newline;
    ]
  | Syntax.WhileStatement x ->
    Concat [
      t env x.while_keyword;
      Space;
      t env x.while_left_paren;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [t env x.while_condition];
        Split;
        t env x.while_right_paren;
      ]);
      handle_possible_compound_statement env x.while_body;
      Newline;
    ]
  | Syntax.DeclareDirectiveStatement x ->
    Concat [
      t env x.declare_directive_keyword;
      Space;
      t env x.declare_directive_left_paren;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [t env x.declare_directive_expression];
        Split;
        t env x.declare_directive_right_paren;
      ]);
      t env x.declare_directive_semicolon;
      Newline;
    ]
  | Syntax.DeclareBlockStatement x ->
    Concat [
      t env x.declare_block_keyword;
      Space;
      t env x.declare_block_left_paren;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [t env x.declare_block_expression];
        Split;
        t env x.declare_block_right_paren;
      ]);
      handle_possible_compound_statement env x.declare_block_body;
      Newline;
    ]
  | Syntax.UsingStatementBlockScoped x ->
    Concat [
      t env x.using_block_await_keyword;
      when_present x.using_block_await_keyword space;
      t env x.using_block_using_keyword;
      Space;
      t env x.using_block_left_paren;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [handle_possible_list env ~after_each:separate_with_space_split x.using_block_expressions];
        Split;
        t env x.using_block_right_paren;
      ]);
      handle_possible_compound_statement env x.using_block_body;
      Newline;
    ]
  | Syntax.UsingStatementFunctionScoped x ->
    Concat [
      t env x.using_function_await_keyword;
      when_present x.using_function_await_keyword space;
      t env x.using_function_using_keyword;
      Space;
      t env x.using_function_expression;
      t env x.using_function_semicolon;
      Newline;
    ]
  | Syntax.IfStatement {
      if_keyword = kw;
      if_left_paren = left_p;
      if_condition = condition;
      if_right_paren = right_p;
      if_statement = if_body;
      if_elseif_clauses = elseif_clauses;
      if_else_clause = else_clause; } ->
    Concat [
      t env kw;
      Space;
      transform_condition env left_p condition right_p;
      transform_consequence t env if_body right_p;
      handle_possible_list env elseif_clauses;
      t env else_clause;
      Newline;
    ]
  | Syntax.ElseifClause {
      elseif_keyword = kw;
      elseif_left_paren = left_p;
      elseif_condition = condition;
      elseif_right_paren = right_p;
      elseif_statement = body; } ->
    Concat [
      t env kw;
      Space;
      transform_condition env left_p condition right_p;
      transform_consequence t env body right_p;
    ]
  | Syntax.ElseClause x ->
    Concat [
      t env x.else_keyword;
      match Syntax.syntax x.else_statement with
      | Syntax.IfStatement _ -> Concat [
          Space;
          t env x.else_statement;
          Space;
        ]
      | _ -> transform_consequence t env x.else_statement x.else_keyword;
    ]
  | Syntax.AlternateIfStatement {
      alternate_if_keyword = kw;
      alternate_if_left_paren = left_p;
      alternate_if_condition = condition;
      alternate_if_right_paren = right_p;
      alternate_if_colon = colon;
      alternate_if_statement = if_body;
      alternate_if_elseif_clauses = elseif_clauses;
      alternate_if_else_clause = else_clause;
      alternate_if_endif_keyword = endif_kw;
      alternate_if_semicolon = semicolon; } ->
    Concat [
      t env kw;
      Space;
      transform_condition env left_p condition right_p;
      t env colon;
      handle_possible_compound_statement env if_body;
      handle_possible_list env elseif_clauses;
      t env else_clause;
      t env endif_kw;
      t env semicolon;
      Newline;
    ]
  | Syntax.AlternateElseifClause {
      alternate_elseif_keyword = kw;
      alternate_elseif_left_paren = left_p;
      alternate_elseif_condition = condition;
      alternate_elseif_right_paren = right_p;
      alternate_elseif_colon = colon;
      alternate_elseif_statement = body; } ->
    Concat [
      t env kw;
      Space;
      transform_condition env left_p condition right_p;
      t env colon;
      handle_possible_compound_statement env body;
    ]
  | Syntax.AlternateElseClause x ->
    Concat [
      t env x.alternate_else_keyword;
      match Syntax.syntax x.alternate_else_statement with
      | Syntax.IfStatement _ -> Concat [
          Space;
          t env x.alternate_else_statement;
          Space;
        ]
      | _ -> handle_possible_compound_statement env x.alternate_else_statement
    ]
  | Syntax.TryStatement {
      try_keyword = kw;
      try_compound_statement = body;
      try_catch_clauses = catch_clauses;
      try_finally_clause = finally_clause; } ->
    (* TODO: revisit *)
    Concat [
      t env kw;
      handle_possible_compound_statement env body;
      handle_possible_list env catch_clauses;
      t env finally_clause;
      Newline;
    ]
  | Syntax.CatchClause {
      catch_keyword = kw;
      catch_left_paren = left_p;
      catch_type = ex_type;
      catch_variable = var;
      catch_right_paren = right_p;
      catch_body = body; } ->
    Concat [
      t env kw;
      Space;
      delimited_nest env left_p right_p [
        t env ex_type;
        Space;
        SplitWith Cost.Base;
        Nest [
          t env var;
        ];
      ];
      handle_possible_compound_statement env body;
    ]
  | Syntax.FinallyClause {
      finally_keyword = kw;
      finally_body = body; } ->
    Concat [
      t env kw;
      Space;
      handle_possible_compound_statement env body;
    ]
  | Syntax.DoStatement {
      do_keyword = do_kw;
      do_body = body;
      do_while_keyword = while_kw;
      do_left_paren = left_p;
      do_condition = cond;
      do_right_paren = right_p;
      do_semicolon = semi; } ->
    Concat [
      t env do_kw;
      Space;
      handle_possible_compound_statement env body;
      t env while_kw;
      Space;
      transform_condition env left_p cond right_p;
      t env semi;
      Newline;
    ]
  | Syntax.ForStatement {
      for_keyword = kw;
      for_left_paren = left_p;
      for_initializer = init;
      for_first_semicolon = semi1;
      for_control = control;
      for_second_semicolon = semi2;
      for_end_of_loop = after_iter;
      for_right_paren = right_p;
      for_body = body; } ->
    Concat [
      t env kw;
      Space;
      t env left_p;
      WithRule (Rule.Parental, Concat [
        Split;
        Nest [
          handle_possible_list env ~after_each:separate_with_space_split init;
          t env semi1;
          Space;
          Split;
          handle_possible_list env ~after_each:separate_with_space_split control;
          t env semi2;
          Space;
          Split;
          handle_possible_list env ~after_each:separate_with_space_split after_iter;
        ];
        Split;
        t env right_p;
      ]);
      handle_possible_compound_statement env body;
      Newline;
    ]
  | Syntax.ForeachStatement {
      foreach_keyword = kw;
      foreach_left_paren = left_p;
      foreach_collection = collection;
      foreach_await_keyword = await_kw;
      foreach_as = as_kw;
      foreach_key = key;
      foreach_arrow = arrow;
      foreach_value = value;
      foreach_right_paren = right_p;
      foreach_body = body; } ->
    Concat [
      t env kw;
      Space;
      delimited_nest env left_p right_p [
        t env collection;
        Space;
        t env await_kw;
        Space;
        t env as_kw;
        Space;
        SplitWith Cost.Base;
        Nest [
          Span [
            t env key;
            Space;
            t env arrow;
            Space;
            SplitWith Cost.Base;
            Nest [
              t env value;
            ];
          ];
        ];
      ];
      handle_possible_compound_statement env body;
      Newline;
    ]
  | Syntax.SwitchStatement {
      switch_keyword = kw;
      switch_left_paren = left_p;
      switch_expression = expr;
      switch_right_paren = right_p;
      switch_left_brace = left_b;
      switch_sections = sections;
      switch_right_brace = right_b; } ->
    let sections = Syntax.syntax_node_to_list sections in
    Concat [
      t env kw;
      Space;
      delimited_nest env left_p right_p [t env expr];
      Space;
      braced_block_nest env left_b right_b (List.map sections (t env));
      Newline;
    ]
  | Syntax.AlternateSwitchStatement {
      alternate_switch_keyword = kw;
      alternate_switch_left_paren = left_p;
      alternate_switch_expression = expr;
      alternate_switch_right_paren = right_p;
      alternate_switch_opening_colon = colon;
      alternate_switch_sections = sections;
      alternate_switch_closing_endswitch = endswitch;
      alternate_switch_closing_semicolon = semicolon; } ->
    let sections = Syntax.syntax_node_to_list sections in
    Concat [
      t env kw;
      Space;
      delimited_nest env left_p right_p [t env expr];
      Space;
      coloned_block_nest env colon endswitch semicolon (List.map sections (t env));
      Newline;
    ]
  | Syntax.SwitchSection {
      switch_section_labels = labels;
      switch_section_statements = statements;
      switch_section_fallthrough = fallthrough; } ->
    (* If there is FallThrough trivia leading the first case label, handle it
     * in a BlockNest so that it is indented to the same level as the previous
     * SwitchSection's statements. *)
    let (labels_leading, labels) = remove_leading_trivia labels in
    let (after_fallthrough, upto_fallthrough) =
      List.split_while (List.rev labels_leading)
        ~f:(fun t -> Trivia.kind t <> TriviaKind.FallThrough)
    in
    let upto_fallthrough = List.rev upto_fallthrough in
    let after_fallthrough = List.rev after_fallthrough in
    let labels = Syntax.syntax_node_to_list labels in
    let statements = Syntax.syntax_node_to_list statements in
    (* When the statements in the SwitchSection are wrapped in a single
     * CompoundStatement, special-case the opening curly brace to appear on
     * the same line as the case label. *)
    let is_scoped_section =
      match statements with
      | [Syntax.{ syntax = CompoundStatement _; _ }] -> true
      | _ -> false
    in
    Concat [
      if List.is_empty upto_fallthrough
      then transform_leading_trivia after_fallthrough
      else Concat [
        BlockNest [transform_leading_trivia upto_fallthrough; Newline];
        transform_trailing_trivia after_fallthrough;
      ];
      handle_list env labels ~after_each:begin fun is_last_label ->
        if is_last_label && is_scoped_section
        then Nothing
        else Newline
      end;
      if is_scoped_section
      then handle_list env statements
      else BlockNest [handle_list env statements];
      t env fallthrough;
    ]
  | Syntax.CaseLabel {
      case_keyword = kw;
      case_expression = expr;
      case_colon = colon; } ->
    Concat [
      t env kw;
      Space;
      SplitWith Cost.Base;
      t env expr;
      t env colon;
    ]
  | Syntax.DefaultLabel {
      default_keyword = kw;
      default_colon = colon; } ->
    Concat [
      t env kw;
      t env colon;
    ]
  | Syntax.SwitchFallthrough {
      fallthrough_keyword = kw;
      fallthrough_semicolon = semi; } ->
    Concat [
      t env kw;
      t env semi;
    ]
  | Syntax.ReturnStatement {
      return_keyword = kw;
      return_expression = expr;
      return_semicolon = semi; } ->
    transform_keyword_expression_statement env kw expr semi
  | Syntax.GotoLabel { goto_label_name; goto_label_colon } ->
    Concat [
      t env goto_label_name;
      t env goto_label_colon;
      Newline;
    ]
  | Syntax.GotoStatement {
      goto_statement_keyword;
      goto_statement_label_name;
      goto_statement_semicolon; } ->
    Concat [
      t env goto_statement_keyword;
      Space;
      t env goto_statement_label_name;
      t env goto_statement_semicolon;
      Newline;
    ]
  | Syntax.ThrowStatement {
      throw_keyword = kw;
      throw_expression = expr;
      throw_semicolon = semi; } ->
    transform_keyword_expression_statement env kw expr semi
  | Syntax.BreakStatement {
      break_keyword = kw;
      break_level = expr;
      break_semicolon = semi; } ->
    transform_keyword_expression_statement env kw expr semi
  | Syntax.ContinueStatement {
      continue_keyword = kw;
      continue_level = level;
      continue_semicolon = semi; } ->
    transform_keyword_expression_statement env kw level semi
  | Syntax.FunctionStaticStatement {
      static_static_keyword = static_kw;
      static_declarations = declarators;
      static_semicolon = semi; } ->
    transform_keyword_expr_list_statement env static_kw declarators semi
  | Syntax.EchoStatement {
      echo_keyword = kw;
      echo_expressions = expr_list;
      echo_semicolon = semi; } ->
    (match Syntax.syntax expr_list with
    | Syntax.SyntaxList [
        Syntax.{ syntax = ListItem { list_item = expr; _ }; _ }]
      when Syntax.kind expr = SyntaxKind.ParenthesizedExpression ->
      Concat [
        t env kw;
        t env expr;
        t env semi;
        Newline;
      ]
    | _ ->
      transform_keyword_expr_list_statement env kw expr_list semi
    )
  | Syntax.GlobalStatement {
      global_keyword = kw;
      global_variables = var_list;
      global_semicolon = semi; } ->
    transform_keyword_expr_list_statement env kw var_list semi
  | Syntax.SimpleInitializer {
      simple_initializer_equal = eq_kw;
      simple_initializer_value = value; } ->
    Concat [
      Space;
      t env eq_kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env value];
    ]
  | Syntax.AnonymousFunction {
      anonymous_attribute_spec = attr;
      anonymous_static_keyword = static_kw;
      anonymous_async_keyword = async_kw;
      anonymous_coroutine_keyword = coroutine_kw;
      anonymous_function_keyword = fun_kw;
      anonymous_ampersand = amp;
      anonymous_left_paren = lp;
      anonymous_parameters = params;
      anonymous_right_paren = rp;
      anonymous_colon = colon;
      anonymous_type = ret_type;
      anonymous_use = use;
      anonymous_body = body; } ->
    Concat [
      t env attr;
      when_present attr space;
      t env static_kw;
      when_present static_kw space;
      t env async_kw;
      when_present async_kw space;
      t env coroutine_kw;
      when_present coroutine_kw space;
      t env fun_kw;
      when_present amp space;
      transform_argish_with_return_type env lp params rp colon ret_type;
      t env use;
      handle_possible_compound_statement env
        ~space:false
        ~allow_collapse:true
        body;
    ]
  | Syntax.Php7AnonymousFunction {
      php7_anonymous_attribute_spec = attr;
      php7_anonymous_static_keyword = static_kw;
      php7_anonymous_async_keyword = async_kw;
      php7_anonymous_coroutine_keyword = coroutine_kw;
      php7_anonymous_function_keyword = fun_kw;
      php7_anonymous_ampersand = amp;
      php7_anonymous_left_paren = lp;
      php7_anonymous_parameters = params;
      php7_anonymous_right_paren = rp;
      php7_anonymous_use = use;
      php7_anonymous_colon = colon;
      php7_anonymous_type = ret_type;
      php7_anonymous_body = body; } ->
    Concat [
      t env attr;
      when_present attr space;
      t env static_kw;
      when_present static_kw space;
      t env async_kw;
      when_present async_kw space;
      t env coroutine_kw;
      when_present coroutine_kw space;
      t env fun_kw;
      when_present amp space;
      transform_argish env lp params rp;
      t env use;
      t env colon;
      when_present colon space;
      t env ret_type;
      handle_possible_compound_statement env
        ~space:false
        ~allow_collapse:true
        body;
    ]
  | Syntax.AnonymousFunctionUseClause {
      anonymous_use_keyword = kw;
      anonymous_use_left_paren = left_p;
      anonymous_use_variables = vars;
      anonymous_use_right_paren = right_p; } ->
    (* TODO: Revisit *)
    Concat [
      Space;
      t env kw;
      Space;
      transform_argish env left_p vars right_p;
    ]
  | Syntax.LambdaExpression {
      lambda_attribute_spec = attr;
      lambda_async = async;
      lambda_coroutine = coroutine;
      lambda_signature = signature;
      lambda_arrow = arrow;
      lambda_body = body; } ->
    Concat [
      t env attr;
      when_present attr space;
      t env async;
      when_present async space;
      t env coroutine;
      when_present coroutine space;
      t env signature;
      Space;
      t env arrow;
      handle_lambda_body env body;
    ]
  | Syntax.LambdaSignature {
      lambda_left_paren = lp;
      lambda_parameters = params;
      lambda_right_paren = rp;
      lambda_colon = colon;
      lambda_type = ret_type; } ->
    transform_argish_with_return_type env lp params rp colon ret_type
  | Syntax.CastExpression _ ->
    Span (List.map (Syntax.children node) (t env))
  | Syntax.MemberSelectionExpression {
      member_object;
      member_operator;
      member_name; } ->
    handle_possible_chaining env
      (
          member_object,
          member_operator,
          member_name
        )
      None
  | Syntax.SafeMemberSelectionExpression {
      safe_member_object;
      safe_member_operator;
      safe_member_name; } ->
    handle_possible_chaining env
      (
            safe_member_object,
            safe_member_operator,
            safe_member_name
          )
      None
  | Syntax.YieldExpression {
      yield_keyword = kw;
      yield_operand = operand; } ->
    Concat [
      t env kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env operand];
    ]
  | Syntax.YieldFromExpression {
      yield_from_yield_keyword = yield_kw;
      yield_from_from_keyword = from_kw;
      yield_from_operand = operand; } ->
    Concat [
      t env yield_kw;
      Space;
      t env from_kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env operand];
    ]
  | Syntax.PrefixUnaryExpression {
      prefix_unary_operator = operator;
      prefix_unary_operand = operand; } ->
    Concat [
      t env operator;
      (match Syntax.syntax operator with
        | Syntax.Token x ->
          let is_parenthesized =
            match Syntax.syntax operand with
            | Syntax.ParenthesizedExpression _ -> true
            | _ -> false
          in
          let open TokenKind in
          (match Token.kind x with
          | Await | Clone | Suspend -> Space
          | Print -> if is_parenthesized then Nothing else Space
          | _ -> Nothing
          )
        | _ -> Nothing
      );
      t env operand;
    ]
  | Syntax.BinaryExpression {
      binary_left_operand;
      binary_operator;
      binary_right_operand; } ->
    let open Syntax in
    begin
      match syntax binary_left_operand, syntax binary_right_operand with
      | ListExpression {
          list_keyword;
          list_left_paren;
          list_members;
          list_right_paren;
        },
        PrefixUnaryExpression {
          prefix_unary_operator = {
            syntax = Token {Token.kind = TokenKind.Await; _}; _
          } as await_keyword;
          prefix_unary_operand = {
            syntax = FunctionCallExpression {
              function_call_receiver = {
                syntax = Token {Token.kind = TokenKind.Name; text = "genva"; _}; _
              } as genva_token;
              function_call_left_paren;
              function_call_right_paren;
              function_call_argument_list;
            }; _
          };
        } ->
        Concat [
          t env list_keyword;
          WithRule (Rule.Parental,
          Concat [
            t env list_left_paren;
            nest env list_right_paren [transform_arg_list env list_members];
            Space;
            t env binary_operator;
            Space;
            t env await_keyword;
            Space;
            t env genva_token;
            t env function_call_left_paren;
            nest env function_call_right_paren
              [transform_arg_list env function_call_argument_list];
          ])
        ]
      | _ ->
        transform_binary_expression env ~is_nested:false
          (binary_left_operand, binary_operator, binary_right_operand)
    end
  | Syntax.InstanceofExpression {
      instanceof_left_operand = left;
      instanceof_operator = kw;
      instanceof_right_operand = right; } ->
    Concat [
      t env left;
      Space;
      t env kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env right];
    ]
  | Syntax.IsExpression {
      is_left_operand = left;
      is_operator = kw;
      is_right_operand = right; }
  | Syntax.AsExpression {
      as_left_operand = left;
      as_operator = kw;
      as_right_operand = right; }
  | Syntax.NullableAsExpression {
      nullable_as_left_operand = left;
      nullable_as_operator = kw;
      nullable_as_right_operand = right; } ->
    Concat [
      t env left;
      Space;
      t env kw;
      Space;
      SplitWith Cost.Base;
      Nest [t env right];
    ]
  | Syntax.ConditionalExpression {
      conditional_test = test_expr;
      conditional_question = q_kw;
      conditional_consequence = true_expr;
      conditional_colon = c_kw;
      conditional_alternative = false_expr; } ->
    WithLazyRule (Rule.Parental,
      t env test_expr,
      Nest [
        Space;
        Split;
        t env q_kw;
        when_present true_expr (fun () -> Concat [
          Space;
          if env.Env.indent_width = 2
            then Nest [t env true_expr]
            else t env true_expr;
          Space;
          Split;
        ]);
        t env c_kw;
        Space;
        if not (Syntax.is_missing true_expr) && env.Env.indent_width = 2
          then Nest [t env false_expr]
          else t env false_expr;
      ])
  | Syntax.FunctionCallExpression {
      function_call_receiver;
      function_call_left_paren;
      function_call_argument_list;
      function_call_right_paren; } ->
    handle_function_call_expression env
      function_call_receiver
      function_call_left_paren
      function_call_argument_list
      function_call_right_paren
  | Syntax.FunctionCallWithTypeArgumentsExpression {
      function_call_with_type_arguments_receiver;
      function_call_with_type_arguments_type_args;
      function_call_with_type_arguments_left_paren;
      function_call_with_type_arguments_argument_list;
      function_call_with_type_arguments_right_paren; } ->
    handle_function_call_with_type_arguments_expression env
      function_call_with_type_arguments_receiver
      function_call_with_type_arguments_type_args
      function_call_with_type_arguments_left_paren
      function_call_with_type_arguments_argument_list
      function_call_with_type_arguments_right_paren
  | Syntax.EvalExpression {
      eval_keyword = kw;
      eval_left_paren = left_p;
      eval_argument = arg;
      eval_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_braced_item env left_p arg right_p;
    ]
  | Syntax.EmptyExpression {
      empty_keyword = kw;
      empty_left_paren = left_p;
      empty_argument = arg;
      empty_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_braced_item env left_p arg right_p;
    ]
  | Syntax.IssetExpression {
      isset_keyword = kw;
      isset_left_paren = left_p;
      isset_argument_list = args;
      isset_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_argish env ~allow_trailing:false left_p args right_p;
    ]
  | Syntax.DefineExpression {
      define_keyword = kw;
      define_left_paren = left_p;
      define_argument_list = args;
      define_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_argish env left_p args right_p;
    ]
  | Syntax.HaltCompilerExpression {
      halt_compiler_keyword = kw;
      halt_compiler_left_paren = left_p;
      halt_compiler_argument_list = args;
      halt_compiler_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_argish env left_p args right_p;
    ]
  | Syntax.ParenthesizedExpression {
      parenthesized_expression_left_paren = left_p;
      parenthesized_expression_expression = expr;
      parenthesized_expression_right_paren = right_p; } ->
    Concat [
      t env left_p;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [ t env expr; ];
        Split;
        t env right_p
      ]);
    ]
  | Syntax.BracedExpression {
      braced_expression_left_brace = left_b;
      braced_expression_expression = expr;
      braced_expression_right_brace = right_b; } ->
    (* TODO: revisit this *)
    Concat [
      t env left_b;
      Split;
      let rule =
        if List.is_empty (Syntax.trailing_trivia left_b)
        && List.is_empty (Syntax.trailing_trivia expr)
          then Rule.Simple Cost.Base
          else Rule.Parental
      in
      WithRule (rule, Concat [
        Nest [t env expr];
        Split;
        t env right_b
      ])
    ]
  | Syntax.EmbeddedBracedExpression {
      embedded_braced_expression_left_brace = left_b;
      embedded_braced_expression_expression = expr;
      embedded_braced_expression_right_brace = right_b; } ->
    (* TODO: Consider finding a way to avoid treating these expressions as
    opportunities for line breaks in long strings:

    $sql = "DELETE FROM `foo` WHERE `left` BETWEEN {$res->left} AND {$res
      ->right} ORDER BY `level` DESC";
    *)
    Concat [
      t env left_b;
      Nest [t env expr];
      t env right_b;
    ]
  | Syntax.ListExpression {
      list_keyword = kw;
      list_left_paren = lp;
      list_members = members;
      list_right_paren = rp; } ->
    Concat [
      t env kw;
      transform_argish env lp members rp;
    ]
  | Syntax.CollectionLiteralExpression {
      collection_literal_name = name;
      collection_literal_left_brace = left_b;
      collection_literal_initializers = initializers;
      collection_literal_right_brace = right_b; } ->
    transform_container_literal
      env ~spaces:true name left_b initializers right_b
  | Syntax.ObjectCreationExpression {
      object_creation_new_keyword = newkw;
      object_creation_object = what; } ->
    Concat [
      t env newkw;
      Space;
      t env what;
    ]
  | Syntax.ConstructorCall {
      constructor_call_type = obj_type;
      constructor_call_left_paren = left_p;
      constructor_call_argument_list = arg_list;
      constructor_call_right_paren = right_p; } ->
    Concat [
      t env obj_type;
      transform_argish env left_p arg_list right_p;
    ]
  | Syntax.AnonymousClass {
      anonymous_class_class_keyword = classkw;
      anonymous_class_left_paren = left_p;
      anonymous_class_argument_list = arg_list;
      anonymous_class_right_paren = right_p;
      anonymous_class_extends_keyword = extends_kw;
      anonymous_class_extends_list = extends;
      anonymous_class_implements_keyword = impl_kw;
      anonymous_class_implements_list = impls;
      anonymous_class_body = body; } ->
    let after_each_ancestor is_last =
      if is_last then Nothing else space_split () in
    Concat [
      t env classkw;
      transform_argish env left_p arg_list right_p;
      when_present extends_kw (fun () -> Concat [
        Space;
        Split;
        WithRule (Rule.Parental, Nest [ Span [
          t env extends_kw;
          Space;
          Split;
          WithRule (Rule.Parental, Nest [
            handle_possible_list env ~after_each:after_each_ancestor extends
          ])
        ]])
      ]);

      when_present impl_kw (fun () -> Concat [
        Space;
        Split;
        WithRule (Rule.Parental, Nest [ Span [
          t env impl_kw;
          Space;
          Split;
          WithRule (Rule.Parental, Nest [
            handle_possible_list env ~after_each:after_each_ancestor impls
          ])
        ]])
      ]);
      t env body;
    ]
  | Syntax.ArrayCreationExpression {
      array_creation_left_bracket = left_b;
      array_creation_members = members;
      array_creation_right_bracket = right_b; } ->
    transform_argish env left_b members right_b
  | Syntax.ArrayIntrinsicExpression {
      array_intrinsic_keyword = kw;
      array_intrinsic_left_paren = left_p;
      array_intrinsic_members = members;
      array_intrinsic_right_paren = right_p; } ->
    transform_container_literal env kw left_p members right_p
  | Syntax.DarrayIntrinsicExpression {
      darray_intrinsic_keyword = kw;
      darray_intrinsic_explicit_type = explicit_type;
      darray_intrinsic_left_bracket = left_p;
      darray_intrinsic_members = members;
      darray_intrinsic_right_bracket = right_p; }
  | Syntax.DictionaryIntrinsicExpression {
      dictionary_intrinsic_keyword = kw;
      dictionary_intrinsic_explicit_type = explicit_type;
      dictionary_intrinsic_left_bracket = left_p;
      dictionary_intrinsic_members = members;
      dictionary_intrinsic_right_bracket = right_p; }
  | Syntax.KeysetIntrinsicExpression {
      keyset_intrinsic_keyword = kw;
      keyset_intrinsic_explicit_type = explicit_type;
      keyset_intrinsic_left_bracket = left_p;
      keyset_intrinsic_members = members;
      keyset_intrinsic_right_bracket = right_p; }
  | Syntax.VarrayIntrinsicExpression {
      varray_intrinsic_keyword = kw;
      varray_intrinsic_explicit_type = explicit_type;
      varray_intrinsic_left_bracket = left_p;
      varray_intrinsic_members = members;
      varray_intrinsic_right_bracket = right_p; }
  | Syntax.VectorIntrinsicExpression {
      vector_intrinsic_keyword = kw;
      vector_intrinsic_explicit_type = explicit_type;
      vector_intrinsic_left_bracket = left_p;
      vector_intrinsic_members = members;
      vector_intrinsic_right_bracket = right_p; } ->
    transform_container_literal env kw ~explicit_type left_p members right_p
  | Syntax.ElementInitializer {
      element_key = key;
      element_arrow = arrow;
      element_value = value; } ->
    transform_mapish_entry env key arrow value
  | Syntax.SubscriptExpression {
      subscript_receiver = receiver;
      subscript_left_bracket = lb;
      subscript_index = expr;
      subscript_right_bracket = rb; } ->
    Concat [
      t env receiver;
      transform_braced_item env lb expr rb;
    ]
  | Syntax.AwaitableCreationExpression {
      awaitable_attribute_spec = attr;
      awaitable_async = async_kw;
      awaitable_coroutine = coroutine_kw;
      awaitable_compound_statement = body; } ->
    Concat [
      t env attr;
      when_present attr space;
      t env async_kw;
      when_present async_kw space;
      t env coroutine_kw;
      when_present coroutine_kw space;
      (* TODO: rethink possible one line bodies *)
      (* TODO: correctly handle spacing after the closing brace *)
      handle_possible_compound_statement env ~space:false body;
    ]
  | Syntax.XHPChildrenDeclaration {
      xhp_children_keyword = kw;
      xhp_children_expression = expr;
      xhp_children_semicolon = semi; } ->
    Concat [
      t env kw;
      Space;
      t env expr;
      t env semi;
      Newline;
    ]
  | Syntax.XHPChildrenParenthesizedList {
      xhp_children_list_left_paren = left_p;
      xhp_children_list_xhp_children = expressions;
      xhp_children_list_right_paren = right_p; } ->
    Concat [
      transform_argish env ~allow_trailing:false left_p expressions right_p;
    ]
  | Syntax.XHPCategoryDeclaration {
      xhp_category_keyword = kw;
      xhp_category_categories = categories;
      xhp_category_semicolon = semi; } ->
    Concat [
    t env kw;
      (* TODO: Eliminate code duplication *)
      WithRule (Rule.Parental, Nest [
        handle_possible_list env ~before_each:space_split categories;
      ]);
      t env semi;
      Newline;
    ]
  | Syntax.XHPEnumType {
      xhp_enum_optional = opt;
      xhp_enum_keyword = kw;
      xhp_enum_left_brace = left_b;
      xhp_enum_values = values;
      xhp_enum_right_brace = right_b; } ->
    Concat [
      t env opt;
      t env kw;
      Space;
      transform_argish env left_b values right_b;
    ]
  | Syntax.XHPClassAttributeDeclaration {
      xhp_attribute_keyword = kw;
      xhp_attribute_attributes = xhp_attributes;
      xhp_attribute_semicolon = semi; } ->
    Concat [
      t env kw;
      (match Syntax.syntax xhp_attributes with
      | Syntax.Missing -> Nothing
      | Syntax.SyntaxList [attr] ->
        WithRule (Rule.Parental, Nest [Space; Split; t env attr])
      | Syntax.SyntaxList attrs ->
        Nest [handle_list env ~before_each:newline attrs]
      | _ -> failwith "Expected SyntaxList"
      );
      t env semi;
      Newline;
    ]
  | Syntax.XHPClassAttribute {
      xhp_attribute_decl_type = attr_type;
      xhp_attribute_decl_name = name;
      xhp_attribute_decl_initializer = init;
      xhp_attribute_decl_required = req; } ->
    (* TODO: figure out nesting here *)
    Concat [
      t env attr_type;
      Space;
      t env name;
      when_present init space;
      t env init;
      when_present req space;
      t env req;
    ]
  | Syntax.XHPSimpleAttribute {
      xhp_simple_attribute_name = name;
      xhp_simple_attribute_equal = eq;
      xhp_simple_attribute_expression = expr; } ->
    Span [
      t env name;
      t env eq;
      SplitWith Cost.Base;
      Nest [t env expr];
    ]
  | Syntax.XHPSpreadAttribute {
      xhp_spread_attribute_left_brace =l_brace;
      xhp_spread_attribute_spread_operator =spread;
      xhp_spread_attribute_expression =expr;
      xhp_spread_attribute_right_brace = r_brace; } ->
    Span [
      t env l_brace;
      t env spread;
      SplitWith Cost.Base;
      Nest [t env expr];
      t env r_brace;
    ]
  | Syntax.XHPOpen {
      xhp_open_left_angle = left_a;
      xhp_open_name = name;
      xhp_open_attributes = attrs;
      xhp_open_right_angle = right_a; } ->
    Concat [
      t env left_a;
      t env name;
      match Syntax.syntax attrs with
      | Syntax.Missing -> handle_xhp_open_right_angle_token env attrs right_a
      | _ ->
        Concat [
          Space;
          Split;
          WithRule (Rule.Parental, Concat [
            Nest [
              handle_possible_list env ~after_each:(fun is_last ->
                if not is_last then space_split () else Nothing
              ) attrs;
            ];
            handle_xhp_open_right_angle_token env attrs right_a;
          ])
        ]
    ]
  | Syntax.XHPExpression {
      xhp_open = xhp_open;
      xhp_body = body;
      xhp_close = close; } ->
    let handle_xhp_body body =
      match Syntax.syntax body with
      | Syntax.Missing -> Nothing, true
      | Syntax.SyntaxList xs ->
        (* XHP breaks the normal rules of trivia. All trailing trivia (except
         * on XHPBody tokens) is lexed as leading trivia for the next token.
         *
         * To deal with this, we keep track of whether the last token we added
         * was one that trailing trivia is scanned for. If it wasn't, we
         * handle the next token's leading trivia with
         * transform_xhp_leading_trivia, which treats all trivia up to the
         * first newline as trailing trivia. *)
        let prev_token_was_xhpbody = ref false in
        let transformed_body = Concat (List.map xs ~f:begin fun node ->
          let leading, node = remove_leading_trivia node in
          let transformed_node = Concat [
            (* Whitespace in an XHPBody is only significant when adjacent to
             * an XHPBody token, so we are free to add splits between other
             * nodes (like XHPExpressions and BracedExpressions). We can also
             * safely add splits before XHPBody tokens, but only if they
             * already have whitespace in their leading trivia.
             *
             * Splits *after* XHPBody tokens are handled below by
             * trailing_whitespace, so if the previous token was an XHPBody
             * token, we don't need to do anything. *)
            if !prev_token_was_xhpbody
              then transform_leading_trivia leading
              else begin
                let v =
                  match Syntax.syntax node with
                  | Syntax.Token _ ->
                    if has_newline leading then Newline
                    else if has_whitespace leading then Space
                    else Nothing
                  | _ -> Split in
                Concat [v; transform_xhp_leading_trivia leading]
              end;
            t env node;
          ] in
          prev_token_was_xhpbody := begin
            match Syntax.syntax node with
            | Syntax.Token t -> Token.kind t = TokenKind.XHPBody
            | _ -> false
          end;
          (* Here, we preserve newlines after XHPBody tokens and don't add
           * splits between them. This means that we don't reflow paragraphs
           * in XHP to fit in the column limit.
           *
           * If we were to split between XHPBody tokens, we'd need a new Rule
           * type to govern word-wrap style splitting, since using independent
           * splits (e.g. SplitWith Cost.Base) between every token would make
           * solving too expensive. *)
          let trailing = Syntax.trailing_trivia node in
          let trailing_whitespace =
            match Syntax.syntax node with
            | Syntax.Token _ when has_newline trailing -> Newline
            | _ when has_whitespace trailing -> Space
            | _ -> Nothing
          in
          Concat [transformed_node; trailing_whitespace]
        end) in
        let leading_token =
          match Syntax.leading_token (List.hd_exn xs) with
          | None -> failwith "Expected token"
          | Some token -> token
        in
        let can_split_before_first_token =
          Token.kind leading_token <> TokenKind.XHPBody ||
          has_invisibles (Token.leading leading_token)
        in
        let transformed_body = Concat [
          if can_split_before_first_token then Split else Nothing;
          transformed_body;
        ] in
        let can_split_before_close = not !prev_token_was_xhpbody in
        transformed_body, can_split_before_close
      | _ -> failwith "Expected SyntaxList"
    in
    WithPossibleLazyRule (Rule.Parental, t env xhp_open,
      let transformed_body, can_split_before_close = handle_xhp_body body in
      Concat [
        Nest [transformed_body];
        when_present close begin fun () ->
          let leading, close = remove_leading_trivia close in Concat [
            (* Ignore extra newlines by treating this as trailing trivia *)
            ignore_trailing_invisibles leading;
            if can_split_before_close then Split else Nothing;
            t env close;
          ]
        end;
      ])
  | Syntax.VarrayTypeSpecifier {
      varray_keyword = kw;
      varray_left_angle = left_a;
      varray_type = varray_type;
      varray_trailing_comma = trailing_comma;
      varray_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_braced_item_with_trailer env
        left_a varray_type trailing_comma right_a;
    ]
  | Syntax.VectorArrayTypeSpecifier {
      vector_array_keyword = kw;
      vector_array_left_angle = left_a;
      vector_array_type = vec_type;
      vector_array_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_braced_item env left_a vec_type right_a;
    ]
  | Syntax.VectorTypeSpecifier {
      vector_type_keyword = kw;
      vector_type_left_angle = left_a;
      vector_type_type = vec_type;
      vector_type_trailing_comma = trailing_comma;
      vector_type_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_braced_item_with_trailer env
        left_a vec_type trailing_comma right_a;
    ]
  | Syntax.KeysetTypeSpecifier {
      keyset_type_keyword = kw;
      keyset_type_left_angle = left_a;
      keyset_type_type = ks_type;
      keyset_type_trailing_comma = trailing_comma;
      keyset_type_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_braced_item_with_trailer env
        left_a ks_type trailing_comma right_a;
    ]
  | Syntax.TypeParameter {
      type_reified = reified;
      type_variance = variance;
      type_name = name;
      type_constraints = constraints; } ->
    Concat [
      t env reified;
      when_present reified space;
      t env variance;
      t env name;
      when_present constraints space;
      handle_possible_list env constraints
        ~after_each:(fun is_last -> if is_last then Nothing else Space);
    ]
  | Syntax.TypeConstraint {
      constraint_keyword = kw;
      constraint_type = constraint_type; } ->
    Concat [
      t env kw;
      Space;
      t env constraint_type;
    ]
  | Syntax.DarrayTypeSpecifier {
      darray_keyword = kw;
      darray_left_angle = left_a;
      darray_key = key;
      darray_comma = comma_kw;
      darray_value = value;
      darray_trailing_comma = trailing_comma;
      darray_right_angle = right_a; } ->
    let key_list_item = Syntax.make_list_item key comma_kw in
    let val_list_item = Syntax.make_list_item value trailing_comma in
    let args = make_list [key_list_item; val_list_item] in
    Concat [
      t env kw;
      transform_argish env ~allow_trailing:true left_a args right_a;
    ]
  | Syntax.MapArrayTypeSpecifier {
      map_array_keyword = kw;
      map_array_left_angle = left_a;
      map_array_key = key;
      map_array_comma = comma_kw;
      map_array_value = value;
      map_array_right_angle = right_a; } ->
    Concat [
      t env kw;
      let key_list_item = Syntax.make_list_item key comma_kw in
      let val_list_item = Syntax.make_list_item value (make_missing ()) in
      let args = make_list [key_list_item; val_list_item] in
      transform_argish env ~allow_trailing:false left_a args right_a;
    ]
  | Syntax.DictionaryTypeSpecifier {
      dictionary_type_keyword = kw;
      dictionary_type_left_angle = left_a;
      dictionary_type_members = members;
      dictionary_type_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_argish env left_a members right_a;
    ]
  | Syntax.ClosureTypeSpecifier {
      closure_outer_left_paren = outer_left_p;
      closure_coroutine = coroutine;
      closure_function_keyword = kw;
      closure_inner_left_paren = inner_left_p;
      closure_parameter_list = param_list;
      closure_inner_right_paren = inner_right_p;
      closure_colon = colon;
      closure_return_type = ret_type;
      closure_outer_right_paren = outer_right_p; } ->
    Concat [
      t env outer_left_p;
      t env coroutine;
      when_present coroutine space;
      t env kw;
      transform_argish_with_return_type env
        inner_left_p param_list inner_right_p colon ret_type;
      t env outer_right_p;
    ]
  | Syntax.ClosureParameterTypeSpecifier {
      closure_parameter_call_convention = callconv;
      closure_parameter_type = cp_type; } ->
    Concat [
      t env callconv;
      when_present callconv space;
      t env cp_type;
    ]
  | Syntax.ClassnameTypeSpecifier {
      classname_keyword = kw;
      classname_left_angle = left_a;
      classname_type = class_type;
      classname_trailing_comma = trailing_comma;
      classname_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_braced_item_with_trailer env
        left_a class_type trailing_comma right_a;
    ]
  | Syntax.FieldSpecifier {
      field_question = question;
      field_name = name;
      field_arrow = arrow_kw;
      field_type = field_type; } ->
    Concat [
      t env question;
      transform_mapish_entry env name arrow_kw field_type;
    ]
  | Syntax.FieldInitializer {
      field_initializer_name = name;
      field_initializer_arrow = arrow_kw;
      field_initializer_value = value; } ->
    transform_mapish_entry env name arrow_kw value
  | Syntax.ShapeTypeSpecifier {
      shape_type_keyword = shape_kw;
      shape_type_left_paren = left_p;
      shape_type_fields = type_fields;
      shape_type_ellipsis = ellipsis;
      shape_type_right_paren = right_p; } ->
    let fields = if Syntax.is_missing ellipsis
      then type_fields
      else
        let missing_separator = make_missing () in
        let ellipsis_list =
          [Syntax.make_list_item ellipsis missing_separator] in
        make_list (Syntax.children type_fields @ ellipsis_list) in
    transform_container_literal env shape_kw left_p fields right_p
      ~allow_trailing:(Syntax.is_missing ellipsis)
  | Syntax.ShapeExpression {
      shape_expression_keyword = shape_kw;
      shape_expression_left_paren = left_p;
      shape_expression_fields = fields;
      shape_expression_right_paren = right_p; } ->
    transform_container_literal env shape_kw left_p fields right_p
  | Syntax.TupleExpression {
      tuple_expression_keyword = kw;
      tuple_expression_left_paren = left_p;
      tuple_expression_items = items;
      tuple_expression_right_paren = right_p; } ->
    Concat [
      t env kw;
      transform_argish env left_p items right_p;
    ]
  | Syntax.TypeArguments {
      type_arguments_left_angle = left_a;
      type_arguments_types = type_list;
      type_arguments_right_angle = right_a; } ->
    transform_argish env left_a type_list right_a
  | Syntax.TypeParameters {
      type_parameters_left_angle = left_a;
      type_parameters_parameters = param_list;
      type_parameters_right_angle = right_a; } ->
    transform_argish env left_a param_list right_a
  | Syntax.TupleTypeSpecifier {
      tuple_left_paren = left_p;
      tuple_types = types;
      tuple_right_paren = right_p; } ->
    transform_argish env left_p types right_p
  | Syntax.TupleTypeExplicitSpecifier {
      tuple_type_keyword = kw;
      tuple_type_left_angle = left_a;
      tuple_type_types = types;
      tuple_type_right_angle = right_a; } ->
    Concat [
      t env kw;
      transform_argish env left_a types right_a
    ]
  | Syntax.DecoratedExpression {
      decorated_expression_decorator = op;
      decorated_expression_expression = expr; } ->
    Concat [
      t env op;
      begin
        match Syntax.syntax op with
        | Syntax.Token t when Token.kind t = TokenKind.Inout -> Space
        | _ -> Nothing
      end;
      t env expr;
    ]
  | Syntax.LetStatement {
      let_statement_keyword = kw;
      let_statement_name = name;
      let_statement_colon = colon;
      let_statement_type = ty;
      let_statement_initializer = init;
      let_statement_semicolon = semi; } ->
    Concat [
      t env kw;
      Space;
      t env name;
      when_present colon space;
      t env colon;
      when_present ty space;
      t env ty;
      Space;
      t env init;
      t env semi;
      Newline;
    ]
  | Syntax.ErrorSyntax _ ->
    raise Hackfmt_error.InvalidSyntax

and when_present node f =
  match Syntax.syntax node with
  | Syntax.Missing -> Nothing
  | _ -> f ()

and transform_simple env node =
  Concat (List.map (Syntax.children node) (t env))

and transform_simple_statement env node =
  Concat ((List.map (Syntax.children node) (t env)) @ [Newline])

and braced_block_nest env ?(allow_collapse=true) open_b close_b nodes =
  let nodes = Concat nodes in
  match allow_collapse, has_printable_content nodes, Syntax.syntax open_b with
  | true, false, Syntax.Token ob
    when List.for_all (Token.trailing ob)
      (fun t -> Trivia.kind t <> TriviaKind.EndOfLine) ->
    Concat [
      t env open_b;
      t env close_b;
    ]
  | _ ->
    (* Remove the closing brace's leading trivia and handle it inside the
     * BlockNest, so that comments will be indented correctly. *)
    let leading, close_b = remove_leading_trivia close_b in
    Concat [
      t env open_b;
      Newline;
      BlockNest [
        nodes;
        transform_leading_trivia leading;
        Newline;
      ];
      t env close_b;
    ]

and coloned_block_nest env open_colon close_token close_semi nodes =
  let leading, close_token = remove_leading_trivia close_token in
  let close_token, trailing = remove_trailing_trivia close_token in
  Concat [
    t env open_colon;
    Newline;
    BlockNest [
      Concat nodes;
      transform_leading_trivia leading;
      Newline;
    ];
    t env close_token;
    t env close_semi;
    transform_trailing_trivia trailing;
  ]

and delimited_nest env
    ?(spaces=false)
    ?(split_when_children_split=true)
    ?(force_newlines=false)
    left_delim
    right_delim
    nodes
  =
  let rule =
    match () with
    | _ when force_newlines -> Rule.Always
    | _ when split_when_children_split -> Rule.Parental
    | _ -> Rule.Simple Cost.Base
  in
  Span [
    t env left_delim;
    WithRule (rule,
      nest env ~spaces right_delim nodes
    );
  ]

and nest env ?(spaces=false) right_delim nodes =
  (* Remove the right delimiter's leading trivia and handle it inside the
   * Nest, so that comments will be indented correctly. *)
  let leading, right_delim = remove_leading_trivia right_delim in
  let nested_contents =
    Nest [Concat nodes; transform_leading_trivia leading]
  in
  let content_present = has_printable_content nested_contents in
  let maybe_split =
    match content_present, spaces with
    | false, _ -> Nothing
    | true, false -> Split
    | true, true -> space_split ()
  in
  Concat [
    maybe_split;
    nested_contents;
    maybe_split;
    t env right_delim;
  ]

and after_each_argument is_last =
  if is_last
  then Split
  else space_split ()

and separate_with_space_split is_last =
  if is_last
  then Nothing
  else space_split ()

and handle_lambda_body env node =
  match Syntax.syntax node with
  | Syntax.CompoundStatement {
      compound_left_brace;
      compound_statements;
      compound_right_brace; } ->
    handle_compound_statement env ~allow_collapse:true
      compound_left_brace compound_statements compound_right_brace;
  | Syntax.XHPExpression _ ->
    WithRule (Rule.Parental, Concat [
      Space;
      Split;
      Nest [t env node];
    ])
  | _ ->
    Concat [
      Space;
      SplitWith Cost.Base;
      Nest [t env node];
    ]

and handle_possible_compound_statement env
    ?(space=true)
    ?(allow_collapse=false)
    node
  =
  match Syntax.syntax node with
  | Syntax.CompoundStatement {
      compound_left_brace;
      compound_statements;
      compound_right_brace; } ->
    Concat [
      handle_compound_statement env
        ~allow_collapse
        compound_left_brace
        compound_statements
        compound_right_brace;
      if space then Space else Nothing;
    ]
  | Syntax.AlternateLoopStatement {
      alternate_loop_opening_colon;
      alternate_loop_statements;
      alternate_loop_closing_keyword;
      alternate_loop_closing_semicolon; } ->
    Concat [
      handle_alternate_loop_statement env
        alternate_loop_opening_colon
        alternate_loop_statements
        alternate_loop_closing_keyword
        alternate_loop_closing_semicolon;
      if space then Space else Nothing;
    ]
  | _ ->
    Concat [
      Newline;
      BlockNest [
        t env node
      ];
    ]

and handle_compound_statement env
    ?(allow_collapse=false)
    left_b
    statements
    right_b
  =
  Concat [
    Space;
    braced_block_nest env ~allow_collapse left_b right_b [
      handle_possible_list env statements
    ];
  ]

and handle_alternate_loop_statement env
    open_colon
    statements
    close_keyword
    close_semi
  =
  Concat [
    coloned_block_nest env open_colon close_keyword close_semi [
      handle_possible_list env statements
    ];
  ]

(**
 * Special-case handling for lists of declarators, where we want the splits
 * between declarators to break if their children break, but we want a single
 * declarator to stay joined with the line preceding it if it fits, even when
 * its children break.
 *)
and handle_declarator_list env declarators =
  match Syntax.syntax declarators with
  | Syntax.Missing -> Nothing
  | Syntax.SyntaxList [declarator] ->
    Nest [
      Space;
      (* Use an independent split, so we don't break just because a line break
       * occurs in the declarator. *)
      SplitWith Cost.Base;
      t env declarator;
    ];
  | Syntax.SyntaxList xs ->
    (* Use Rule.Parental to break each declarator onto its own line if any
     * line break occurs in a declarator, or if they can't all fit onto one
     * line. *)
    WithRule (Rule.Parental, Nest (List.map xs (fun declarator -> Concat [
      Space;
      Split;
      t env declarator;
    ])));
  | _ -> failwith "SyntaxList expected"

and handle_list env
    ?(before_each=(fun () -> Nothing))
    ?(after_each=(fun _is_last -> Nothing))
    ?(handle_element=t env)
    ?(handle_last=handle_element)
    list =
  let rec aux l = (
    match l with
    | hd :: [] ->
      Concat [
        before_each ();
        handle_last hd;
        after_each true;
      ]
    | hd :: tl ->
      Concat [
        before_each ();
        handle_element hd;
        after_each false;
        aux tl
      ]
    | [] -> Nothing
  ) in
  aux list

and list_length node =
  match Syntax.syntax node with
  | Syntax.Missing -> 0
  | Syntax.SyntaxList x -> List.length x
  | _ -> 1

and handle_possible_list env
    ?before_each
    ?after_each
    ?handle_element
    ?handle_last
    node =
  match Syntax.syntax node with
  | Syntax.Missing -> Nothing
  | Syntax.SyntaxList x ->
    handle_list env x ?before_each ?after_each ?handle_element ?handle_last
  | _ -> handle_list env [node] ?before_each ?after_each ?handle_element ?handle_last

and handle_xhp_open_right_angle_token env attrs node =
  match Syntax.syntax node with
  | Syntax.Token token ->
    Concat [
      if Token.text token = "/>"
        then Concat [Space; when_present attrs split]
        else Nothing;
      t env node
    ]
  | _ -> failwith "expected xhp_open right_angle token"

and handle_function_call_expression env
    receiver
    lp
    args
    rp
  =
  match Syntax.syntax receiver with
  | Syntax.MemberSelectionExpression {
      member_object;
      member_operator;
      member_name; } ->
    handle_possible_chaining env
      (member_object, member_operator, member_name)
      (Some (lp, args, rp))
  | Syntax.SafeMemberSelectionExpression {
      safe_member_object;
      safe_member_operator;
      safe_member_name; } ->
    handle_possible_chaining env
      (safe_member_object, safe_member_operator, safe_member_name)
      (Some (lp, args, rp))
  | _ ->
    Concat [
      t env receiver;
      transform_argish env lp args rp
    ]

and handle_function_call_with_type_arguments_expression env
    receiever
    tyargs
    lp
    args
    rp
  =
  match Syntax.syntax receiever with
  | Syntax.MemberSelectionExpression {
      member_object;
      member_operator;
      member_name; } ->
    handle_possible_chaining env
      (member_object, member_operator, member_name)
      (Some (lp, args, rp))
  | Syntax.SafeMemberSelectionExpression {
      safe_member_object;
      safe_member_operator;
      safe_member_name; } ->
    handle_possible_chaining env
      (safe_member_object, safe_member_operator, safe_member_name)
      (Some (lp, args, rp))
  | _ ->
    Concat [
      t env receiever;
      t env tyargs;
      transform_argish env lp args rp
    ]

and handle_possible_chaining env (obj, arrow1, member1) argish =
  let rec handle_chaining obj =
    let handle_mse_or_smse (obj, arrow, member) fun_paren_args =
      let (obj, l) = handle_chaining obj in
      obj, l @ [(arrow, member, fun_paren_args)]
    in
    match Syntax.syntax obj with
    | Syntax.FunctionCallExpression {
        function_call_receiver = receiver;
        function_call_left_paren = lp;
        function_call_argument_list = args;
        function_call_right_paren = rp; } ->
      (match Syntax.syntax receiver with
        | Syntax.MemberSelectionExpression {
            member_object;
            member_operator;
            member_name; } ->
          handle_mse_or_smse
            (member_object, member_operator, member_name)
            (Some (lp, args, rp))
        | Syntax.SafeMemberSelectionExpression {
            safe_member_object;
            safe_member_operator;
            safe_member_name; } ->
          handle_mse_or_smse
            (safe_member_object, safe_member_operator, safe_member_name)
            (Some (lp, args, rp))
        | _ -> obj, []
      )
    | Syntax.MemberSelectionExpression {
        member_object;
        member_operator;
        member_name; } ->
      handle_mse_or_smse
        (member_object, member_operator, member_name)
        None
    | Syntax.SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name; } ->
      handle_mse_or_smse
        (safe_member_object, safe_member_operator, safe_member_name)
        None
    | _ -> obj, []
  in

  let (obj, chain_list) = handle_chaining obj in
  let chain_list = chain_list @ [(arrow1, member1, argish)] in

  let transform_chain (arrow, member, argish) =
    Concat [
      t env arrow;
      t env member;
      Option.value_map argish ~default:Nothing
        ~f:(fun (lp, args, rp) -> transform_argish env lp args rp);
    ]
  in
  let obj_has_trailing_newline = node_has_trailing_newline obj in
  match chain_list with
  | hd :: [] ->
    Concat [
      Span [t env obj];
      if obj_has_trailing_newline
      then Newline
      else SplitWith Cost.High;
      Nest [transform_chain hd];
    ]
  | hd :: tl ->
    let transformed_hd = transform_chain hd in
    let tl = List.map tl transform_chain in
    let rule_type = match hd with
      | (_, trailing, None)
      | (_, _, Some (_, _, trailing)) ->
        if node_has_trailing_newline trailing
        then Rule.Always
        else if obj_has_trailing_newline
        then Rule.Parental
        else
          (* If we have a chain where only the final item contains internal
             splits, use a Simple rule instead of a Parental one.
             This allows us to preserve this style:

             return $this->fooGenerator->generateFoo(
               $argument_one,
               $argument_two,
               $argument_three,
             );
          *)
          let rev_tl_except_last = List.rev tl |> List.tl_exn in
          let items_except_last = transformed_hd :: rev_tl_except_last in
          if List.exists items_except_last has_split
          then Rule.Parental
          else Rule.Simple Cost.NoCost
    in
    Span [
      WithLazyRule (rule_type,
        Concat [
          t env obj;
          if obj_has_trailing_newline
          then Newline
          else SplitWith Cost.Base;
        ],
        Concat [
          (* This needs to be nested separately due to the above SplitWith *)
          Nest [transformed_hd];
          Nest (List.map tl ~f:(fun x -> Concat [Split; x]))
        ])
    ]
  | _ -> failwith "Expected a chain of at least length 1"

and transform_fn_decl_name env modifiers kw amp name type_params leftp =
  let mods = handle_possible_list env ~after_each:(fun _ -> Space) modifiers in
  [
    mods;
    t env kw;
    Space;
    t env amp;
    t env name;
    t env type_params;
    t env leftp;
    Split;
  ]

and transform_fn_decl_args env params rightp colon ret_type where =
  (* It is a syntax error to follow a variadic parameter with a trailing
   * comma, so suppress trailing commas in that case. *)
  let allow_trailing =
    match Syntax.syntax params with
    | Syntax.SyntaxList params ->
      let last_param =
        match Syntax.syntax (List.last_exn params) with
        | Syntax.ListItem { list_item; _ } -> list_item
        | _ -> failwith "Expected ListItem"
      in
      begin
        match Syntax.syntax last_param with
        | Syntax.VariadicParameter _
        | Syntax.(ParameterDeclaration {
            parameter_name = { syntax = DecoratedExpression {
              decorated_expression_decorator = {
                syntax = Token { Token.kind = TokenKind.DotDotDot; _ }; _
              }; _
            }; _ }; _
          }) ->
          false
        | _ -> true
      end
    | _ -> true
  in
  WithRule (Rule.Parental, Concat [
    transform_possible_comma_list env ~allow_trailing params rightp;
    t env colon;
    when_present colon space;
    t env ret_type;
    when_present where space;
    t env where;
  ])

and transform_argish_with_return_type env left_p params right_p colon ret_type =
  Concat [
    t env left_p;
    when_present params split;
    WithRule (Rule.Parental, Span [
      Span [ transform_possible_comma_list env params right_p ];
      t env colon;
      when_present colon space;
      t env ret_type;
    ])
  ]

and transform_argish env
    ?(allow_trailing=true) ?(force_newlines=false) ?(spaces=false)
    left_p arg_list right_p =
  (* It is a syntax error to follow a splat argument with a trailing comma, so
     suppress trailing commas in that case. *)
  let allow_trailing =
    match Syntax.syntax arg_list with
    | Syntax.SyntaxList args ->
      let last_arg =
        match Syntax.syntax (List.last_exn args) with
        | Syntax.ListItem { list_item; _ } -> list_item
        | _ -> failwith "Expected ListItem"
      in
      begin match Syntax.syntax last_arg with
      | Syntax.(DecoratedExpression {
          decorated_expression_decorator = {
            syntax = Token { Token.kind = TokenKind.DotDotDot; _ }; _
          }; _
        }) -> false
      | _ -> allow_trailing
      end
    | _ -> allow_trailing
  in
  (* When the last argument breaks across multiple lines, we want to allow the
     arg list rule to stay unbroken even though the last argument contains
     splits that may be broken on.

     For example:

     // We do not want to break f's rule even though its child splits:
     f(vec[
       $foo, // single-line comment forces the vec's rule to split
       $bar,
     ]);

     // We do not want to break map's rule even though the lambda has splits:
     map($vec, $element ==> {
       // ...
     });
   *)
  let split_when_children_split =
    if spaces then true else
    match Syntax.syntax arg_list with
    | Syntax.SyntaxList [] -> true
    | Syntax.SyntaxList [x] ->
      let has_surrounding_whitespace =
        not (
          List.is_empty (Syntax.trailing_trivia left_p) &&
          List.is_empty (Syntax.trailing_trivia arg_list)
        )
      in
      if has_surrounding_whitespace
      then true
      else looks_bad_in_non_parental_braces x
    | Syntax.SyntaxList items ->
      let last = List.last_exn items in
      let has_surrounding_whitespace =
        not (
          List.is_empty (Syntax.leading_trivia last) &&
          List.is_empty (Syntax.trailing_trivia last)
        )
      in
      if has_surrounding_whitespace then true else
      (* When there are multiple arguments, opt into this behavior only when we
         have no splits in any of the arguments except the last. *)
      (match List.rev items with
      | [] -> assert false
      | last :: rest ->
        let prev_args_may_split =
          rest
          |> List.map ~f:(t env)
          |> List.exists ~f:has_split
        in
        if prev_args_may_split
        then true
        else looks_bad_in_non_parental_braces last
      )
    | _ -> true
  in
  delimited_nest env
    ~spaces
    ~split_when_children_split
    ~force_newlines
    left_p
    right_p
    [transform_arg_list env ~allow_trailing arg_list]

(** Sometimes, we want to use a non-Parental rule for function call argument
    lists and other similar constructs when not breaking around the argument
    list looks reasonable. For example:

      f($x ==> {
        return do_something_with($x);
      });

    Some constructs don't look so great when we do this:

      f($x ==>
        do_something_with($x));

      f($x
        ? $y
        : $z);

    This function blacklists those constructs. *)
and looks_bad_in_non_parental_braces item =
  let item =
    match Syntax.syntax item with
    | Syntax.ListItem { list_item; _ } -> list_item
    | _ -> item
  in
  match Syntax.syntax item with
  | Syntax.(LambdaExpression
      { lambda_body = { syntax = CompoundStatement _; _ }; _ }) ->
    false
  | Syntax.FunctionCallExpression { function_call_receiver; _ } ->
    Syntax.is_member_selection_expression function_call_receiver
  | Syntax.ConditionalExpression _
  | Syntax.BinaryExpression _
  | Syntax.MemberSelectionExpression _
  | Syntax.FieldSpecifier _
  | Syntax.FieldInitializer _
  | Syntax.ElementInitializer _
  | Syntax.LambdaExpression _
    -> true
  | _ -> false

and transform_braced_item env left_p item right_p =
  let has_no_surrounding_trivia =
    List.is_empty (Syntax.trailing_trivia left_p) &&
    List.is_empty (Syntax.leading_trivia item) &&
    List.is_empty (Syntax.trailing_trivia item) &&
    List.is_empty (Syntax.leading_trivia right_p)
  in
  if has_no_surrounding_trivia && not (looks_bad_in_non_parental_braces item)
  then Concat (List.map [left_p; item; right_p] (t env))
  else delimited_nest env left_p right_p [t env item]

and transform_argish_item env x =
  match Syntax.syntax x with
  | Syntax.ListItem { list_item; list_separator } ->
    Concat [
      transform_argish_item env list_item;
      t env list_separator;
    ]
  | Syntax.BinaryExpression {
      binary_left_operand  = left;
      binary_operator      = op;
      binary_right_operand = right; }
      when not (is_concat op) ->
    transform_binary_expression env ~is_nested:true (left, op, right)
  | _ -> t env x

and transform_trailing_comma env ~allow_trailing item comma =
  (* PHP does not permit trailing commas in function calls. Rather than try to
   * account for where PHP's parser permits trailing commas, we just never add
   * them in PHP files. *)
  let allow_trailing = allow_trailing && env.Env.add_trailing_commas in
  match Syntax.syntax comma with
  | Syntax.Token tok ->
    Concat [
      transform_argish_item env item;
      transform_leading_trivia (Token.leading tok);
      if allow_trailing then TrailingComma true else Nothing;
      Ignore (Token.text tok, Token.width tok);
      transform_trailing_trivia (Token.trailing tok);
    ]
  | Syntax.Missing ->
    let item, item_trailing = remove_trailing_trivia item in
    Concat [
      transform_argish_item env item;
      if allow_trailing then TrailingComma false else Nothing;
      transform_trailing_trivia item_trailing;
    ]
  | _ -> failwith "Expected Token"

and transform_braced_item_with_trailer env left_p item comma right_p =
  let has_no_surrounding_trivia =
    List.is_empty (Syntax.trailing_trivia left_p) &&
    List.is_empty (Syntax.leading_trivia item) &&
    List.is_empty (Syntax.trailing_trivia item) &&
    List.is_empty (Syntax.leading_trivia comma) &&
    List.is_empty (Syntax.trailing_trivia comma) &&
    List.is_empty (Syntax.leading_trivia right_p)
  in
  (* TODO: turn allow_trailing:true when HHVM versions that don't support
     trailing commas in all these places reach end-of-life. *)
  let item_and_comma =
    transform_trailing_comma env ~allow_trailing:false item comma
  in
  if has_no_surrounding_trivia && not (looks_bad_in_non_parental_braces item)
  then Concat [t env left_p; item_and_comma; t env right_p]
  else delimited_nest env left_p right_p [item_and_comma]

and transform_arg_list env ?(allow_trailing=true) items =
  handle_possible_list env items
    ~after_each:after_each_argument
    ~handle_last:(transform_last_arg env ~allow_trailing)
    ~handle_element:(transform_argish_item env)

and transform_possible_comma_list env ?(allow_trailing=true) ?(spaces=false)
    items right_p =
  nest env ~spaces right_p [
    transform_arg_list env ~allow_trailing items
  ]

and transform_container_literal env
    ?(spaces=false) ?allow_trailing ?explicit_type kw left_p members right_p =
  let force_newlines = node_has_trailing_newline left_p in
  let ty = match explicit_type with
  | Some ex_ty -> t env ex_ty
  | None -> Nothing in
  Concat [
    t env kw;
    ty;
    if spaces then Space else Nothing;
    transform_argish env
      ~spaces ~force_newlines ?allow_trailing left_p members right_p;
  ]

and replace_leading_trivia node new_leading_trivia =
  match Syntax.leading_token node with
  | None -> node
  | Some leading_token ->
    let rewritten_node = Rewriter.rewrite_pre (fun node_to_rewrite ->
      match Syntax.syntax node_to_rewrite with
      | Syntax.Token t when t == leading_token ->
        Rewriter.Replace
          (Syntax.make_token {t with Token.leading = new_leading_trivia})
      | _  -> Rewriter.Keep
    ) node in
  rewritten_node

and remove_leading_trivia node =
  match Syntax.leading_token node with
  | None -> [], node
  | Some leading_token ->
    let rewritten_node = Rewriter.rewrite_pre (fun rewrite_node ->
      match Syntax.syntax rewrite_node with
      | Syntax.Token t when t == leading_token ->
        Rewriter.Replace
          (Syntax.make_token {t with Token.leading = []})
      | _  -> Rewriter.Keep
    ) node in
    Token.leading leading_token, rewritten_node

and remove_trailing_trivia node =
  match Syntax.trailing_token node with
  | None -> node, []
  | Some trailing_token ->
    let rewritten_node = Rewriter.rewrite_pre (fun rewrite_node ->
      match Syntax.syntax rewrite_node with
      | Syntax.Token t when t == trailing_token ->
        Rewriter.Replace
          (Syntax.make_token {t with Token.trailing = []})
      | _  -> Rewriter.Keep
    ) node in
    rewritten_node, Token.trailing trailing_token

and transform_last_arg env ~allow_trailing node =
  match Syntax.syntax node with
  | Syntax.ListItem {
      list_item = item;
      list_separator = separator; } ->
    transform_trailing_comma env ~allow_trailing item separator
  | _ -> failwith "Expected ListItem"


and transform_mapish_entry env key arrow value =
  Concat [
    t env key;
    Space;
    t env arrow;
    Space;
    SplitWith Cost.Base;
    Nest [t env value];
  ]

and transform_keyword_expression_statement env kw expr semi =
  Concat [
    t env kw;
    when_present expr (fun () -> Concat [
      Space;
      SplitWith Cost.Base;
      Nest [t env expr];
    ]);
    t env semi;
    Newline;
  ]

and transform_keyword_expr_list_statement env kw expr_list semi =
  Concat [
    t env kw;
    handle_declarator_list env expr_list;
    t env semi;
    Newline;
  ]

and transform_condition env left_p condition right_p =
  Concat [
    t env left_p;
    Split;
    WithRule (Rule.Parental, Concat [
      Nest [t env condition];
      Split;
      t env right_p;
    ])
  ]

and get_operator_type op =
  match Syntax.syntax op with
  | Syntax.Token t -> Full_fidelity_operator.trailing_from_token
    (Token.kind t)
  | _ -> failwith "Operator should always be a token"

and is_concat op =
  get_operator_type op = Full_fidelity_operator.ConcatenationOperator

and transform_binary_expression env ~is_nested (left, operator, right) =
  let operator_has_surrounding_spaces op = not (is_concat op) in
  let operator_is_leading op =
    get_operator_type op = Full_fidelity_operator.PipeOperator in
  let operator_preserves_newlines op =
    get_operator_type op = Full_fidelity_operator.PipeOperator in

  let operator_t = get_operator_type operator in

  if Full_fidelity_operator.is_comparison operator_t then
    WithLazyRule (Rule.Parental,
      Concat [
        t env left;
        Space;
        t env operator;
      ],
      Concat [
        Space;
        Split;
        Nest [t env right];
      ])
  else if Full_fidelity_operator.is_assignment operator_t then
    Concat [
      t env left;
      Space;
      t env operator;
      Space;
      SplitWith Cost.NoCost;
      Nest [t env right];
    ]
  else
    Concat [
      let precedence = Full_fidelity_operator.precedence operator_t in

      let rec flatten_expression expr =
        match Syntax.syntax expr with
        | Syntax.BinaryExpression {
            binary_left_operand = left;
            binary_operator = operator;
            binary_right_operand = right; } ->
          let operator_t = get_operator_type operator in
          let op_precedence = Full_fidelity_operator.precedence operator_t in
          if (op_precedence = precedence) then
            (flatten_expression left) @ (operator :: flatten_expression right)
          else [expr]
        | _ -> [expr]
      in

      let transform_operand operand =
        match Syntax.syntax operand with
        | Syntax.BinaryExpression {
            binary_left_operand;
            binary_operator;
            binary_right_operand; } ->
          transform_binary_expression env ~is_nested:true
            (binary_left_operand, binary_operator, binary_right_operand)
        | _ -> t env operand
      in

      let binary_expression_syntax_list =
        flatten_expression (Syntax.make_binary_expression left operator right)
      in
      match binary_expression_syntax_list with
      | hd :: tl ->
        WithLazyRule (Rule.Parental,
          transform_operand hd,
          let expression =
            let last_operand = ref hd in
            let last_op = ref (List.hd_exn tl) in
            List.mapi tl ~f:(fun i x ->
              if i mod 2 = 0 then begin
                let op = x in
                last_op := op;
                let op_has_spaces = operator_has_surrounding_spaces op in
                let op_is_leading = operator_is_leading op in
                let newline_before_op =
                  operator_preserves_newlines op &&
                  node_has_trailing_newline !last_operand
                in
                Concat [
                  if newline_before_op then Newline
                  else
                    if op_is_leading
                    then (if op_has_spaces then space_split () else Split)
                    else (if op_has_spaces then Space else Nothing);
                  if is_concat op
                    then ConcatOperator (t env op)
                    else t env op;
                ]
              end
              else begin
                let operand = x in
                last_operand := x;
                let op_has_spaces =
                  operator_has_surrounding_spaces !last_op
                in
                let op_is_leading = operator_is_leading !last_op in
                Concat [
                  if op_is_leading
                  then (if op_has_spaces then Space else Nothing)
                  else (if op_has_spaces then space_split () else Split);
                  transform_operand operand;
                ]
              end
            )
          in
          if is_nested
            then Nest expression
            else ConditionalNest expression)
      | _ ->
        failwith "Expected non empty list of binary expression pieces"
    ]

and make_string text width =
  let split_text = (Str.split_delim (Str.regexp "\n") text) in
  begin match split_text with
    | [_] -> Text (text, width)
    | _ -> MultilineString (split_text, width)
  end

(* Check the leading trivia of the node's leading token.
   Treat the node's text as a multiline string if the leading trivia contains
   an ignore comment. *)
and transform_node_if_ignored node =
  let leading_before, leading_including_and_after =
    leading_ignore_comment (Syntax.leading_trivia node) in
  if List.length leading_including_and_after > 0 then
    let node = replace_leading_trivia node leading_including_and_after in
    let node, trailing_trivia = remove_trailing_trivia node in
    Some(Concat[
      transform_leading_trivia leading_before;
      Newline;
      make_string (Syntax.text node) (Syntax.width node);
      transform_trailing_trivia trailing_trivia;
      if has_newline trailing_trivia then Newline else Nothing;
    ])
  else
    None

and ignore_re = Str.regexp_string "hackfmt-ignore"

and leading_ignore_comment trivia_list =
  let before = List.take_while
    trivia_list
    ~f:(fun trivia ->
      try (Str.search_forward ignore_re (Trivia.text trivia) 0) < 0
      with Not_found -> true) in
  let _, including_and_after = List.split_n trivia_list (List.length before) in
  (before, including_and_after)

(* True if the trivia list contains WhiteSpace trivia.
 * Note that WhiteSpace includes spaces and tabs, but not newlines. *)
and has_whitespace trivia_list =
  List.exists trivia_list
    ~f:(fun trivia -> Trivia.kind trivia = TriviaKind.WhiteSpace)

(* True if the trivia list contains EndOfLine trivia. *)
and has_newline trivia_list =
  List.exists trivia_list
    ~f:(fun trivia -> Trivia.kind trivia = TriviaKind.EndOfLine)

and is_invisible trivia =
  match Trivia.kind trivia with
  | TriviaKind.WhiteSpace | TriviaKind.EndOfLine -> true
  | _ -> false

(* True if the trivia list contains any "invisible" trivia, meaning spaces,
 * tabs, or newlines. *)
and has_invisibles trivia_list =
  List.exists trivia_list ~f:is_invisible

and transform_leading_trivia t = transform_trivia ~is_leading:true t
and transform_trailing_trivia t = transform_trivia ~is_leading:false t

and transform_trivia ~is_leading trivia =
  let new_line_regex = Str.regexp "\n" in
  let indent = ref 0 in
  let currently_leading = ref is_leading in
  let leading_invisibles = ref [] in
  let last_comment = ref None in
  let last_comment_was_delimited = ref false in
  let newline_followed_last_comment = ref false in
  let whitespace_followed_last_comment = ref false in
  let trailing_invisibles = ref [] in
  let comments = ref [] in
  let make_comment _ =
    if Option.is_some !last_comment then begin
      newline_followed_last_comment := has_newline !trailing_invisibles;
      whitespace_followed_last_comment := has_whitespace !trailing_invisibles;
    end;
    comments :=
      (Concat [
        transform_leading_invisibles (List.rev !leading_invisibles);
        Option.value !last_comment ~default:Nothing;
        ignore_trailing_invisibles (List.rev !trailing_invisibles);
        if !last_comment_was_delimited && !whitespace_followed_last_comment
          then Space
        else if !newline_followed_last_comment
          then Newline
          else Nothing
      ])
      :: !comments;
    last_comment := None;
    leading_invisibles := [];
    trailing_invisibles := [];
  in
  List.iter trivia ~f:(fun triv ->
    match Trivia.kind triv with
    | TriviaKind.AfterHaltCompiler ->
      (* ignore content that appears after __halt_compiler *)
      ()
    | TriviaKind.ExtraTokenError
    | TriviaKind.UnsafeExpression
    | TriviaKind.FixMe
    | TriviaKind.IgnoreError
    | TriviaKind.DelimitedComment ->
      let preceded_by_whitespace =
        if !currently_leading
          then has_whitespace !leading_invisibles
          else has_whitespace !trailing_invisibles
      in
      make_comment ();
      let delimited_lines = Str.split new_line_regex (Trivia.text triv) in
      let map_tail str =
        let prefix_space_count str =
          let len = String.length str in
          let rec aux i =
            if i = len || (str.[i] <> ' ' && str.[i] <> '\t')
            then 0
            else 1 + (aux (i + 1))
          in
          aux 0
        in
        (* If we're dealing with trailing trivia, then we don't have a good
           signal for the indent level, so we just cut all leading spaces.
           Otherwise, we cut a number of spaces equal to the indent before
           the delimited comment opener. *)
        let start_index = if is_leading
          then min !indent (prefix_space_count str)
          else prefix_space_count str
        in
        let len = String.length str - start_index in
        let dc =
          Trivia.create_delimited_comment @@ String.sub str start_index len
        in
        Concat [
          Ignore ("\n", 1);
          Newline;
          Ignore ((String.make start_index ' '), start_index);
          Comment ((Trivia.text dc), (Trivia.width dc));
        ]
      in

      let hd = List.hd_exn delimited_lines in
      let tl = List.tl_exn delimited_lines in
      let hd = Comment (hd, (String.length hd)) in

      let should_break =
        match Trivia.kind triv with
        | TriviaKind.UnsafeExpression
        | TriviaKind.FixMe
        | TriviaKind.IgnoreError
          -> false
        | _ -> !currently_leading
      in

      last_comment := Some (Concat [
        if should_break then Newline
        else if preceded_by_whitespace then Space
        else Nothing;
        Concat (hd :: List.map tl ~f:map_tail);
      ]);
      last_comment_was_delimited := true;
      currently_leading := false;
    | TriviaKind.Unsafe
    | TriviaKind.FallThrough
    | TriviaKind.SingleLineComment ->
      make_comment ();
      last_comment := Some (Concat [
        if !currently_leading then Newline else Space;
        SingleLineComment ((Trivia.text triv), (Trivia.width triv));
      ]);
      last_comment_was_delimited := false;
      currently_leading := false;
    | TriviaKind.EndOfLine ->
      indent := 0;
      if !currently_leading then
        leading_invisibles := triv :: !leading_invisibles
      else begin
        trailing_invisibles := triv :: !trailing_invisibles;
        make_comment ();
      end;
      currently_leading := true;
    | TriviaKind.WhiteSpace ->
      if !currently_leading then begin
        indent := Trivia.width triv;
        leading_invisibles := triv :: !leading_invisibles
      end
      else
        trailing_invisibles := triv :: !trailing_invisibles;
  );
  if List.is_empty !comments then begin
    if is_leading
      then transform_leading_invisibles trivia
      else ignore_trailing_invisibles trivia
  end
  else begin
    make_comment ();
    Concat (List.rev !comments)
  end

and _MAX_CONSECUTIVE_BLANK_LINES = 2

and transform_leading_invisibles triv =
  let newlines = ref 0 in
  Concat (List.map triv ~f:(fun t ->
    let ignored = Ignore ((Trivia.text t), (Trivia.width t)) in
    match Trivia.kind t with
    | TriviaKind.EndOfLine ->
      newlines := !newlines + 1;
      Concat [
        ignored;
        if !newlines <= _MAX_CONSECUTIVE_BLANK_LINES
          then BlankLine
          else Nothing
      ]
    | _ -> ignored;
  ))

and ignore_trailing_invisibles triv =
  Concat
    (List.map triv ~f:(fun t -> Ignore ((Trivia.text t), (Trivia.width t))))

and transform_xhp_leading_trivia triv =
  let (up_to_first_newline, after_newline, _) =
    List.fold triv
      ~init:([], [], false)
      ~f:begin fun (upto, after, seen) t ->
        if seen then upto, t :: after, true
        else t :: upto, after, Trivia.kind t = TriviaKind.EndOfLine
      end
  in
  Concat [
    ignore_trailing_invisibles up_to_first_newline;
    transform_leading_invisibles after_newline;
  ]

and node_has_trailing_newline node =
  let trivia = Syntax.trailing_trivia node in
  List.exists trivia ~f:(fun x -> Trivia.kind x = TriviaKind.EndOfLine)

and transform_consequence t (env: Env.t) (node_body: Syntax.t) (node_newline: Syntax.t) = 
  match Syntax.syntax node_body with
  | Syntax.CompoundStatement _ -> handle_possible_compound_statement env node_body
  | _ -> Concat [
      Space;
      if has_newline (Syntax.trailing_trivia node_newline)
      then Concat [Newline; Nest [t env node_body]]
      else WithRule (Rule.Parental, Nest [ Span [
        Space;
        Split;
        t env node_body;
      ]]);
    ]

let transform (env: Env.t) (node: Syntax.t) : Doc.t =
  t env node
