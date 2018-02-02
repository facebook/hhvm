(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxKind = Full_fidelity_syntax_kind
module Syntax = Full_fidelity_editable_syntax
module TriviaKind = Full_fidelity_trivia_kind
module Trivia = Full_fidelity_editable_trivia
module Token = Full_fidelity_editable_token
module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)
module Env = Format_env
module SourceText = Full_fidelity_source_text

open Hh_core
open Syntax
open Doc

let make_list = Syntax.make_list SourceText.empty 0
let make_missing () = Syntax.make_missing SourceText.empty 0

let transform (env: Env.t) (node: Syntax.t) : Doc.t =
  let rec t node =
    match syntax node with
    | Missing ->
      Nothing
    | Token x ->
      let open EditableToken in
      let token_kind = kind x in
      Concat [
        begin
          match token_kind with
          | TokenKind.EndOfFile ->
            let leading_trivia = leading x in
            let trivia_without_trailing_invisibles =
              let reversed = List.rev leading_trivia in
              List.rev (List.drop_while reversed ~f:is_invisible)
            in
            transform_leading_trivia trivia_without_trailing_invisibles
          | _ -> transform_leading_trivia (leading x)
        end;
        begin
          let open TokenKind in
          match token_kind with
          | SingleQuotedStringLiteral
          | DoubleQuotedStringLiteral
          | DoubleQuotedStringLiteralHead
          | StringLiteralBody
          | DoubleQuotedStringLiteralTail
          | HeredocStringLiteral
          | HeredocStringLiteralHead
          | HeredocStringLiteralTail
          | NowdocStringLiteral ->
            let split_text = (Str.split_delim (Str.regexp "\n") (text x)) in
            begin match split_text with
              | [_] -> Text (text x, width x)
              | _ -> MultilineString (split_text, width x)
            end
          | _ -> Text (text x, width x)
        end;
        transform_trailing_trivia (trailing x);
      ]
    | SyntaxList _ ->
      failwith (Printf.sprintf
        "Error: SyntaxList should never be handled directly;
        offending text is '%s'." (text node));
    | EndOfFile x ->
      t x.end_of_file_token
    | Script x ->
      begin match x.script_declarations.syntax with
      | SyntaxList (header::declarations) when is_markup_section header ->
        Concat [
          t header;
          Newline;
          handle_list declarations;
        ]
      | _ ->
        Concat [
          handle_possible_list x.script_declarations;
        ]
      end
    | LiteralExpression { literal_expression } ->
      (* Double quoted string literals can create a list *)
      let open EditableToken in
      let wrap_with_literal_type token transformed =
        let open TokenKind in
        match kind token with
        | HeredocStringLiteral
        | HeredocStringLiteralHead
        | HeredocStringLiteralTail
        | NowdocStringLiteral -> DocLiteral transformed
        | DecimalLiteral
        | OctalLiteral
        | HexadecimalLiteral
        | BinaryLiteral
        | FloatingLiteral -> NumericLiteral transformed
        | _ -> transformed
      in
      begin match syntax literal_expression with
        | Token tok -> wrap_with_literal_type tok (t literal_expression)
        | SyntaxList l ->
          let last = trailing_token literal_expression in
          begin match last with
            | Some tok -> wrap_with_literal_type tok (Concat (List.map l t))
            | _ -> failwith "Expected Token"
          end
        | _ -> failwith "Expected Token or SyntaxList"
      end
    | MarkupSection {
        markup_prefix = prefix;
        markup_text = text;
        markup_suffix = suffix;
        _ } ->
      if is_missing prefix
      then
        (* leading markup section
           for hh files - strip leading whitespaces\newlines - they are not
           emitted and having them in Hack file is illegal anyways *)
        let is_hh_script = match suffix.syntax with
          | MarkupSuffix { markup_suffix_name = {
              syntax = Token t; _
            }; _ } ->
            (EditableToken.text t) = "hh"
          | _ -> false
        in
        let rec all_whitespaces s i =
          i >= String.length s
          || (match String.get s i with
              | ' ' | '\t' | '\r' | '\n' -> all_whitespaces s (i + 1)
              | _ -> false)
        in
        let text_contains_only_whitespaces = match text.syntax with
          | Token t -> all_whitespaces (EditableToken.text t) 0
          | _ -> false
        in
        if is_hh_script && text_contains_only_whitespaces
        then t suffix
        else transform_simple node
      else transform_simple node
    | MarkupSuffix _
    | SimpleTypeSpecifier _
    | VariableExpression _
    | PipeVariableExpression _
    | PropertyDeclarator _
    | ConstantDeclarator _
    | StaticDeclarator _
    | ScopeResolutionExpression _
    | EmbeddedMemberSelectionExpression _
    | EmbeddedSubscriptExpression _
    | PostfixUnaryExpression _
    | XHPRequired _
    | XHPSimpleClassAttribute _
    | XHPClose _
    | TypeConstant _
    | GenericTypeSpecifier _
    | NullableTypeSpecifier _
    | SoftTypeSpecifier _
    | ListItem _ ->
      transform_simple node
    | QualifiedName { qualified_name_parts; } ->
      handle_possible_list qualified_name_parts
    | ExpressionStatement _ ->
      transform_simple_statement node
    | EnumDeclaration {
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
        t attr;
        when_present attr newline;
        t kw;
        Space;
        t name;
        t colon_kw;
        Space;
        SplitWith Cost.Base;
        Nest [
          Space;
          t base;
          Space;
          t enum_type;
          Space;
        ];
        braced_block_nest left_b right_b [
          handle_possible_list enumerators
        ];
        Newline;
      ]
    | Enumerator {
        enumerator_name = name;
        enumerator_equal = eq_kw;
        enumerator_value = value;
        enumerator_semicolon = semi } ->
      Concat [
        t name;
        Space;
        t eq_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t value];
        t semi;
        Newline;
      ]
    | AliasDeclaration {
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
        t attr;
        when_present attr newline;
        t kw;
        Space;
        t name;
        t generic;
        Space;
        t type_constraint;
        Space;
        t eq_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t alias_type];
        t semi;
        Newline;
      ]
    | PropertyDeclaration {
        property_modifiers = modifiers;
        property_type = prop_type;
        property_declarators = declarators;
        property_semicolon = semi } ->
      Concat [
        handle_possible_list ~after_each:(fun _ -> Space) modifiers;
        t prop_type;
        handle_declarator_list declarators;
        t semi;
        Newline;
      ]
    | NamespaceDeclaration {
        namespace_keyword = kw;
        namespace_name = name;
        namespace_body = body } ->
      Concat [
        t kw;
        Space;
        t name;
        t body;
        Newline;
      ]
    | NamespaceBody {
        namespace_left_brace = left_b;
        namespace_declarations = decls;
        namespace_right_brace = right_b } ->
      Concat [
        Space;
        braced_block_nest left_b right_b [handle_possible_list decls];
      ]
    | NamespaceEmptyBody {
        namespace_semicolon = semi } ->
      Concat [
        t semi;
      ]
    | NamespaceUseDeclaration {
        namespace_use_keyword = kw;
        namespace_use_kind = use_kind;
        namespace_use_clauses = clauses;
        namespace_use_semicolon = semi } ->
      Concat [
        t kw;
        Space;
        t use_kind;
        when_present use_kind space;
        WithRule (Rule.Parental, Nest [
          handle_possible_list clauses ~after_each:after_each_argument;
        ]);
        t semi;
        Newline;
      ]
    | NamespaceGroupUseDeclaration {
        namespace_group_use_keyword = kw;
        namespace_group_use_kind = use_kind;
        namespace_group_use_prefix = prefix;
        namespace_group_use_left_brace = left_b;
        namespace_group_use_clauses = clauses;
        namespace_group_use_right_brace = right_b;
        namespace_group_use_semicolon = semi } ->
      Concat [
        t kw;
        Space;
        t use_kind;
        when_present use_kind space;
        t prefix;
        transform_argish left_b clauses right_b;
        t semi;
        Newline;
      ]
    | NamespaceUseClause {
        namespace_use_clause_kind = use_kind;
        namespace_use_name = name;
        namespace_use_as = as_kw;
        namespace_use_alias = alias } ->
      Concat [
        t use_kind;
        when_present use_kind space;
        t name;
        when_present as_kw space;
        t as_kw;
        when_present alias space;
        t alias;
      ]
    | FunctionDeclaration {
        function_attribute_spec = attr;
        function_declaration_header = header;
        function_body = body } ->
      Concat [
        t attr;
        when_present attr newline;
        t header;
        handle_possible_compound_statement ~allow_collapse:true body;
        Newline;
      ]
    | FunctionDeclarationHeader {
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
          transform_fn_decl_name modifiers kw amp name type_params leftp);
        transform_fn_decl_args params rightp colon ret_type where;
      ]
    | WhereClause {
        where_clause_keyword = where;
        where_clause_constraints = constraints } ->
      Concat [
        t where;
        Space;
        handle_possible_list constraints ~after_each:(fun _ -> Space);
      ]
    | WhereConstraint {
        where_constraint_left_type = left;
        where_constraint_operator = op;
        where_constraint_right_type = right } ->
      Concat [
        t left;
        Space;
        t op;
        Space;
        t right;
      ]
    | MethodishDeclaration {
        methodish_attribute = attr;
        methodish_function_decl_header = func_decl;
        methodish_function_body = body;
        methodish_semicolon = semi } ->
      Concat [
        t attr;
        when_present attr newline;
        (
          let fn_name, args_and_where = match syntax func_decl with
            | FunctionDeclarationHeader {
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
                transform_fn_decl_name
                  modifiers
                  kw
                  amp
                  name
                  type_params
                  leftp
              ),
              transform_fn_decl_args params rightp colon ret_type where
            | _ -> failwith "Expected FunctionDeclarationHeader"
          in
          Concat [
            Span [fn_name];
            args_and_where;
          ]
        );
        when_present body (fun () ->
          handle_possible_compound_statement ~allow_collapse:true body
        );
        t semi;
        Newline;
      ]
    | ClassishDeclaration {
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
        t attr;
        when_present attr newline;
        Span [
          handle_possible_list ~after_each:(fun _ -> Space) modifiers;
          t kw;
          Space;
          Split;
          Nest [
            t name;
            t type_params;
          ];
        ];

        when_present extends_kw (fun () -> Concat [
          Space;
          Split;
          WithRule (Rule.Parental, Nest [ Span [
            t extends_kw;
            Space;
            Split;
            WithRule (Rule.Parental, Nest [
              handle_possible_list ~after_each:after_each_ancestor extends
            ])
          ]])
        ]);

        when_present impl_kw (fun () -> Concat [
          Space;
          Split;
          WithRule (Rule.Parental, Nest [ Span [
            t impl_kw;
            Space;
            Split;
            WithRule (Rule.Parental, Nest [
              handle_possible_list ~after_each:after_each_ancestor impls
            ])
          ]])
        ]);
        t body;
      ]
    | ClassishBody {
        classish_body_left_brace = left_b;
        classish_body_elements = body;
        classish_body_right_brace = right_b } ->
      Concat [
        Space;
        braced_block_nest left_b right_b [
          handle_possible_list body
        ];
        Newline;
      ]
    | TraitUsePrecedenceItem {
        trait_use_precedence_item_name = name;
        trait_use_precedence_item_keyword = kw;
        trait_use_precedence_item_removed_names = removed_names } ->
      Concat [
        t name;
        Space;
        t kw;
        Space;
        t removed_names;
        Newline;
      ]
    | TraitUseAliasItem {
        trait_use_alias_item_aliasing_name = aliasing_name;
        trait_use_alias_item_keyword = kw;
        trait_use_alias_item_modifiers = visibility;
        trait_use_alias_item_aliased_name = aliased_name } ->
      Concat [
        t aliasing_name;
        Space;
        t kw;
        Space;
        t visibility;
        Space;
        t aliased_name;
        Newline;
      ]
    | TraitUseConflictResolution {
        trait_use_conflict_resolution_keyword = kw;
        trait_use_conflict_resolution_names = elements;
        trait_use_conflict_resolution_left_brace = lb;
        trait_use_conflict_resolution_clauses = clauses;
        trait_use_conflict_resolution_right_brace = rb } ->
      Concat [
        t kw;
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split elements;
        ]);
        t lb;
        Newline;
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split clauses;
        ]);
        Newline;
        t rb;
      ]
    | TraitUse {
        trait_use_keyword = kw;
        trait_use_names = elements;
        trait_use_semicolon = semi } ->
      Concat [
        t kw;
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split elements;
        ]);
        t semi;
        Newline;
      ]
    | RequireClause {
        require_keyword = kw;
        require_kind = kind;
        require_name = name;
        require_semicolon = semi } ->
      Concat [
        t kw;
        Space;
        t kind;
        Space;
        Split;
        t name;
        t semi;
        Newline;
      ]
    | ConstDeclaration {
        const_abstract = abstr;
        const_keyword = kw;
        const_type_specifier = const_type;
        const_declarators = declarators;
        const_semicolon = semi } ->
      Concat [
        t abstr;
        when_present abstr space;
        t kw;
        when_present const_type space;
        t const_type;
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split declarators;
        ]);
        t semi;
        Newline;
      ]
    | TypeConstDeclaration {
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
        t abs;
        Space;
        t kw;
        Space;
        t type_kw;
        Space;
        t name;
        t type_params;
        when_present type_constraint space;
        t type_constraint;
        when_present eq space;
        t eq;
        when_present type_spec (fun _ -> Concat [
          Space;
          SplitWith Cost.Base;
          Nest [t type_spec];
        ]);
        t semi;
        Newline;
      ]
    | ParameterDeclaration {
        parameter_attribute = attr;
        parameter_visibility = visibility;
        parameter_call_convention = callconv;
        parameter_type = param_type;
        parameter_name = name;
        parameter_default_value = default } ->
      Concat [
        t attr;
        t visibility;
        when_present visibility space;
        t callconv;
        when_present callconv space;
        t param_type;
        if is_missing visibility && is_missing callconv && is_missing param_type
        then t name
        else Concat [
          Space;
          SplitWith Cost.Base;
          Nest [t name];
        ];
        t default;
      ]
    | VariadicParameter {
        variadic_parameter_call_convention = callconv;
        variadic_parameter_type = type_var;
        variadic_parameter_ellipsis = ellipsis } ->
      Concat [
        t callconv;
        when_present callconv space;
        t type_var;
        t ellipsis;
      ]
    | AttributeSpecification {
        attribute_specification_left_double_angle = left_da;
        attribute_specification_attributes = attrs;
        attribute_specification_right_double_angle = right_da; } ->
      transform_argish ~allow_trailing:false left_da attrs right_da
    | Attribute {
        attribute_name = name;
        attribute_left_paren = left_p;
        attribute_values = values;
        attribute_right_paren = right_p; } ->
      Concat [
        t name;
        transform_argish left_p values right_p;
      ]
    | InclusionExpression {
        inclusion_require = kw;
        inclusion_filename = expr; } ->
      Concat [
        t kw;
        (match syntax expr with
        | ParenthesizedExpression _ -> Nothing
        | _ -> Space
        );
        SplitWith Cost.Base;
        t expr;
      ]
    | InclusionDirective {
        inclusion_expression = expr;
        inclusion_semicolon = semi; } ->
      Concat [
        t expr;
        t semi;
        Newline;
      ]
    | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace; } ->
      Concat [
        handle_compound_statement (
          compound_left_brace,
          compound_statements,
          compound_right_brace);
        Newline;
      ]
    | UnsetStatement {
        unset_keyword = kw;
        unset_left_paren = left_p;
        unset_variables = args;
        unset_right_paren = right_p;
        unset_semicolon = semi; } ->
      Concat [
        t kw;
        transform_argish ~allow_trailing:false left_p args right_p;
        t semi;
        Newline;
      ]
    | WhileStatement x ->
      Concat [
        t x.while_keyword;
        Space;
        t x.while_left_paren;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [t x.while_condition];
          Split;
          t x.while_right_paren;
        ]);
        handle_possible_compound_statement x.while_body;
        Newline;
      ]
    | DeclareDirectiveStatement x ->
      Concat [
        t x.declare_directive_keyword;
        Space;
        t x.declare_directive_left_paren;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [t x.declare_directive_expression];
          Split;
          t x.declare_directive_right_paren;
        ]);
        t x.declare_directive_semicolon;
        Newline;
      ]
    | DeclareBlockStatement x ->
      Concat [
        t x.declare_block_keyword;
        Space;
        t x.declare_block_left_paren;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [t x.declare_block_expression];
          Split;
          t x.declare_block_right_paren;
        ]);
        handle_possible_compound_statement x.declare_block_body;
        Newline;
      ]
    | UsingStatementBlockScoped x ->
      Concat [
        t x.using_block_await_keyword;
        when_present x.using_block_await_keyword space;
        t x.using_block_using_keyword;
        Space;
        t x.using_block_left_paren;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [handle_possible_list x.using_block_expressions];
          Split;
          t x.using_block_right_paren;
        ]);
        handle_possible_compound_statement x.using_block_body;
        Newline;
      ]
    | UsingStatementFunctionScoped x ->
      Concat [
        t x.using_function_await_keyword;
        when_present x.using_function_await_keyword space;
        t x.using_function_using_keyword;
        Space;
        t x.using_function_expression;
        t x.using_function_semicolon;
        Newline;
      ]
    | IfStatement {
        if_keyword = kw;
        if_left_paren = left_p;
        if_condition = condition;
        if_right_paren = right_p;
        if_statement = if_body;
        if_elseif_clauses = elseif_clauses;
        if_else_clause = else_clause; } ->
      Concat [
        t kw;
        Space;
        transform_condition left_p condition right_p;
        handle_possible_compound_statement if_body;
        handle_possible_list elseif_clauses;
        t else_clause;
        Newline;
      ]
    | ElseifClause {
        elseif_keyword = kw;
        elseif_left_paren = left_p;
        elseif_condition = condition;
        elseif_right_paren = right_p;
        elseif_statement = body; } ->
      Concat [
        t kw;
        Space;
        transform_condition left_p condition right_p;
        handle_possible_compound_statement body;
      ]
    | ElseClause x ->
      Concat [
        t x.else_keyword;
        match syntax x.else_statement with
        | IfStatement _ -> Concat [
            Space;
            t x.else_statement;
            Space;
          ]
        | _ -> handle_possible_compound_statement x.else_statement
      ]
      | IfEndIfStatement {
          if_endif_keyword = kw;
          if_endif_left_paren = left_p;
          if_endif_condition = condition;
          if_endif_right_paren = right_p;
          if_endif_colon = colon;
          if_endif_statement = if_body;
          if_endif_elseif_colon_clauses = elseif_clauses;
          if_endif_else_colon_clause = else_clause;
          if_endif_endif_keyword = endif_kw;
          if_endif_semicolon = semicolon; } ->
        Concat [
          t kw;
          Space;
          transform_condition left_p condition right_p;
          t colon;
          handle_possible_compound_statement if_body;
          handle_possible_list elseif_clauses;
          t else_clause;
          t endif_kw;
          t semicolon;
          Newline;
        ]
      | ElseifColonClause {
          elseif_colon_keyword = kw;
          elseif_colon_left_paren = left_p;
          elseif_colon_condition = condition;
          elseif_colon_right_paren = right_p;
          elseif_colon_colon = colon;
          elseif_colon_statement = body; } ->
        Concat [
          t kw;
          Space;
          transform_condition left_p condition right_p;
          t colon;
          handle_possible_compound_statement body;
        ]
      | ElseColonClause x ->
        Concat [
          t x.else_colon_keyword;
          match syntax x.else_colon_statement with
          | IfStatement _ -> Concat [
              Space;
              t x.else_colon_statement;
              Space;
            ]
          | _ -> handle_possible_compound_statement x.else_colon_statement
        ]
    | TryStatement {
        try_keyword = kw;
        try_compound_statement = body;
        try_catch_clauses = catch_clauses;
        try_finally_clause = finally_clause; } ->
      (* TODO: revisit *)
      Concat [
        t kw;
        handle_possible_compound_statement body;
        handle_possible_list catch_clauses;
        t finally_clause;
        Newline;
      ]
    | CatchClause {
        catch_keyword = kw;
        catch_left_paren = left_p;
        catch_type = ex_type;
        catch_variable = var;
        catch_right_paren = right_p;
        catch_body = body; } ->
      Concat [
        t kw;
        Space;
        delimited_nest left_p right_p [
          t ex_type;
          Space;
          SplitWith Cost.Base;
          Nest [
            t var;
          ];
        ];
        handle_possible_compound_statement body;
      ]
    | FinallyClause {
        finally_keyword = kw;
        finally_body = body; } ->
      Concat [
        t kw;
        Space;
        handle_possible_compound_statement body;
      ]
    | DoStatement {
        do_keyword = do_kw;
        do_body = body;
        do_while_keyword = while_kw;
        do_left_paren = left_p;
        do_condition = cond;
        do_right_paren = right_p;
        do_semicolon = semi; } ->
      Concat [
        t do_kw;
        Space;
        handle_possible_compound_statement body;
        t while_kw;
        Space;
        transform_condition left_p cond right_p;
        t semi;
        Newline;
      ]
    | ForStatement {
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
        t kw;
        Space;
        t left_p;
        WithRule (Rule.Parental, Concat [
          Split;
          Nest [
            handle_possible_list init;
            t semi1;
            Space;
            Split;
            handle_possible_list control;
            t semi2;
            Space;
            Split;
            handle_possible_list after_iter;
          ];
          Split;
          t right_p;
        ]);
        handle_possible_compound_statement body;
        Newline;
      ]
    | ForeachStatement {
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
        t kw;
        Space;
        delimited_nest left_p right_p [
          t collection;
          Space;
          t await_kw;
          Space;
          t as_kw;
          Space;
          SplitWith Cost.Base;
          Nest [
            Span [
              t key;
              Space;
              t arrow;
              Space;
              SplitWith Cost.Base;
              Nest [
                t value;
              ];
            ];
          ];
        ];
        handle_possible_compound_statement body;
        Newline;
      ]
    | SwitchStatement {
        switch_keyword = kw;
        switch_left_paren = left_p;
        switch_expression = expr;
        switch_right_paren = right_p;
        switch_left_brace = left_b;
        switch_sections = sections;
        switch_right_brace = right_b; } ->
      let sections = syntax_node_to_list sections in
      Concat [
        t kw;
        Space;
        delimited_nest left_p right_p [t expr];
        Space;
        braced_block_nest left_b right_b (List.map sections t);
        Newline;
      ]
    | SwitchSection {
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
      let labels = syntax_node_to_list labels in
      let statements = syntax_node_to_list statements in
      (* When the statements in the SwitchSection are wrapped in a single
       * CompoundStatement, special-case the opening curly brace to appear on
       * the same line as the case label. *)
      let is_scoped_section =
        match statements with
        | [{ syntax = CompoundStatement _; _ }] -> true
        | _ -> false
      in
      Concat [
        if List.is_empty upto_fallthrough
        then transform_leading_trivia after_fallthrough
        else Concat [
          BlockNest [transform_leading_trivia upto_fallthrough; Newline];
          transform_trailing_trivia after_fallthrough;
        ];
        handle_list labels ~after_each:begin fun is_last_label ->
          if is_last_label && is_scoped_section
          then Nothing
          else Newline
        end;
        if is_scoped_section
        then handle_list statements
        else BlockNest [handle_list statements];
        t fallthrough;
      ]
    | CaseLabel {
        case_keyword = kw;
        case_expression = expr;
        case_colon = colon; } ->
      Concat [
        t kw;
        Space;
        SplitWith Cost.Base;
        t expr;
        t colon;
      ]
    | DefaultLabel {
        default_keyword = kw;
        default_colon = colon; } ->
      Concat [
        t kw;
        t colon;
      ]
    | SwitchFallthrough {
        fallthrough_keyword = kw;
        fallthrough_semicolon = semi; } ->
      Concat [
        t kw;
        t semi;
      ]
    | ReturnStatement {
        return_keyword = kw;
        return_expression = expr;
        return_semicolon = semi; } ->
      transform_keyword_expression_statement kw expr semi
    | GotoLabel { goto_label_name; goto_label_colon } ->
      Concat [
        t goto_label_name;
        t goto_label_colon;
        Newline;
      ]
    | GotoStatement {
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon; } ->
      Concat [
        t goto_statement_keyword;
        Space;
        t goto_statement_label_name;
        t goto_statement_semicolon;
        Newline;
      ]
    | ThrowStatement {
        throw_keyword = kw;
        throw_expression = expr;
        throw_semicolon = semi; } ->
      transform_keyword_expression_statement kw expr semi
    | BreakStatement {
        break_keyword = kw;
        break_level = expr;
        break_semicolon = semi; } ->
      transform_keyword_expression_statement kw expr semi
    | ContinueStatement {
        continue_keyword = kw;
        continue_level = level;
        continue_semicolon = semi; } ->
      transform_keyword_expression_statement kw level semi
    | FunctionStaticStatement {
        static_static_keyword = static_kw;
        static_declarations = declarators;
        static_semicolon = semi; } ->
      transform_keyword_expr_list_statement static_kw declarators semi
    | EchoStatement {
        echo_keyword = kw;
        echo_expressions = expr_list;
        echo_semicolon = semi; } ->
      (match syntax expr_list with
      | SyntaxList [{ syntax = ListItem { list_item = expr; _ }; _ }]
        when kind expr = SyntaxKind.ParenthesizedExpression ->
        Concat [
          t kw;
          t expr;
          t semi;
          Newline;
        ]
      | _ ->
        transform_keyword_expr_list_statement kw expr_list semi
      )
    | GlobalStatement {
        global_keyword = kw;
        global_variables = var_list;
        global_semicolon = semi; } ->
      transform_keyword_expr_list_statement kw var_list semi
    | SimpleInitializer {
        simple_initializer_equal = eq_kw;
        simple_initializer_value = value; } ->
      Concat [
        Space;
        t eq_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t value];
      ]
    | AnonymousFunction {
        anonymous_static_keyword = static_kw;
        anonymous_async_keyword = async_kw;
        anonymous_coroutine_keyword = coroutine_kw;
        anonymous_function_keyword = fun_kw;
        anonymous_left_paren = lp;
        anonymous_parameters = params;
        anonymous_right_paren = rp;
        anonymous_colon = colon;
        anonymous_type = ret_type;
        anonymous_use = use;
        anonymous_body = body; } ->
      Concat [
        t static_kw;
        when_present static_kw space;
        t async_kw;
        when_present async_kw space;
        t coroutine_kw;
        when_present coroutine_kw space;
        t fun_kw;
        transform_argish_with_return_type lp params rp colon ret_type;
        t use;
        handle_possible_compound_statement
          ~space:false
          ~allow_collapse:true
          body;
      ]
    | Php7AnonymousFunction {
        php7_anonymous_static_keyword = static_kw;
        php7_anonymous_async_keyword = async_kw;
        php7_anonymous_coroutine_keyword = coroutine_kw;
        php7_anonymous_function_keyword = fun_kw;
        php7_anonymous_left_paren = lp;
        php7_anonymous_parameters = params;
        php7_anonymous_right_paren = rp;
        php7_anonymous_use = use;
        php7_anonymous_colon = colon;
        php7_anonymous_type = ret_type;
        php7_anonymous_body = body; } ->
      Concat [
        t static_kw;
        when_present static_kw space;
        t async_kw;
        when_present async_kw space;
        t coroutine_kw;
        when_present coroutine_kw space;
        t fun_kw;
        transform_argish lp params rp;
        t use;
        t colon;
        when_present colon space;
        t ret_type;
        handle_possible_compound_statement
          ~space:false
          ~allow_collapse:true
          body;
      ]
    | AnonymousFunctionUseClause {
        anonymous_use_keyword = kw;
        anonymous_use_left_paren = left_p;
        anonymous_use_variables = vars;
        anonymous_use_right_paren = right_p; } ->
      (* TODO: Revisit *)
      Concat [
        Space;
        t kw;
        Space;
        transform_argish left_p vars right_p;
      ]
    | LambdaExpression {
        lambda_async = async;
        lambda_coroutine = coroutine;
        lambda_signature = signature;
        lambda_arrow = arrow;
        lambda_body = body; } ->
      Concat [
        t async;
        when_present async space;
        t coroutine;
        when_present coroutine space;
        t signature;
        Space;
        t arrow;
        handle_lambda_body body;
      ]
    | LambdaSignature {
        lambda_left_paren = lp;
        lambda_parameters = params;
        lambda_right_paren = rp;
        lambda_colon = colon;
        lambda_type = ret_type; } ->
      transform_argish_with_return_type lp params rp colon ret_type
    | CastExpression _ ->
      Span (List.map (children node) t)
    | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name; } ->
      handle_possible_chaining
        (
            member_object,
            member_operator,
            member_name
          )
        None
    | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name; } ->
      handle_possible_chaining
        (
              safe_member_object,
              safe_member_operator,
              safe_member_name
            )
        None
    | YieldExpression {
        yield_keyword = kw;
        yield_operand = operand; } ->
      Concat [
        t kw;
        Space;
        SplitWith Cost.Base;
        Nest [t operand];
      ]
    | YieldFromExpression {
        yield_from_yield_keyword = yield_kw;
        yield_from_from_keyword = from_kw;
        yield_from_operand = operand; } ->
      Concat [
        t yield_kw;
        Space;
        t from_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t operand];
      ]
    | PrefixUnaryExpression {
        prefix_unary_operator = operator;
        prefix_unary_operand = operand; } ->
      Concat [
        t operator;
        (match syntax operator with
          | Token x ->
            let is_parenthesized =
              match syntax operand with
              | ParenthesizedExpression _ -> true
              | _ -> false
            in
            let open EditableToken.TokenKind in
            (match EditableToken.kind x with
            | Await | Clone | Suspend -> Space
            | Print -> if is_parenthesized then Nothing else Space
            | _ -> Nothing
            )
          | _ -> Nothing
        );
        t operand;
      ]
    | BinaryExpression {
        binary_left_operand;
        binary_operator;
        binary_right_operand; } ->
      transform_binary_expression ~is_nested:false
        (binary_left_operand, binary_operator, binary_right_operand)
    | InstanceofExpression {
        instanceof_left_operand = left;
        instanceof_operator = kw;
        instanceof_right_operand = right; } ->
      Concat [
        t left;
        Space;
        t kw;
        Space;
        SplitWith Cost.Base;
        Nest [t right];
      ]
    | IsExpression {
        is_left_operand = left;
        is_operator = kw;
        is_right_operand = right; } ->
      Concat [
        t left;
        Space;
        t kw;
        Space;
        SplitWith Cost.Base;
        Nest [t right];
      ]
    | ConditionalExpression {
        conditional_test = test_expr;
        conditional_question = q_kw;
        conditional_consequence = true_expr;
        conditional_colon = c_kw;
        conditional_alternative = false_expr; } ->
      WithLazyRule (Rule.Parental,
        t test_expr,
        Nest [
          Space;
          Split;
          t q_kw;
          when_present true_expr (fun () -> Concat [
            Space;
            if env.Env.indent_width = 2
              then Nest [t true_expr]
              else t true_expr;
            Space;
            Split;
          ]);
          t c_kw;
          Space;
          if not (is_missing true_expr) && env.Env.indent_width = 2
            then Nest [t false_expr]
            else t false_expr;
        ])
    | FunctionCallExpression {
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren; } ->
      handle_function_call_expression (
        function_call_receiver,
        function_call_left_paren,
        function_call_argument_list,
        function_call_right_paren)
    | FunctionCallWithTypeArgumentsExpression {
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren; } ->
      handle_function_call_with_type_arguments_expression (
        function_call_with_type_arguments_receiver,
        function_call_with_type_arguments_type_args,
        function_call_with_type_arguments_left_paren,
        function_call_with_type_arguments_argument_list,
        function_call_with_type_arguments_right_paren)
    | EvalExpression {
        eval_keyword = kw;
        eval_left_paren = left_p;
        eval_argument = arg;
        eval_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_braced_item left_p arg right_p;
      ]
    | EmptyExpression {
        empty_keyword = kw;
        empty_left_paren = left_p;
        empty_argument = arg;
        empty_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_braced_item left_p arg right_p;
      ]
    | IssetExpression {
        isset_keyword = kw;
        isset_left_paren = left_p;
        isset_argument_list = args;
        isset_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_argish ~allow_trailing:false left_p args right_p;
      ]
    | DefineExpression {
        define_keyword = kw;
        define_left_paren = left_p;
        define_argument_list = args;
        define_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_argish left_p args right_p;
      ]
    | HaltCompilerExpression {
        halt_compiler_keyword = kw;
        halt_compiler_left_paren = left_p;
        halt_compiler_argument_list = args;
        halt_compiler_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_argish left_p args right_p;
      ]
    | ParenthesizedExpression {
        parenthesized_expression_left_paren = left_p;
        parenthesized_expression_expression = expr;
        parenthesized_expression_right_paren = right_p; } ->
      Concat [
        t left_p;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [ t expr; ];
          Split;
          t right_p
        ]);
      ]
    | BracedExpression {
        braced_expression_left_brace = left_b;
        braced_expression_expression = expr;
        braced_expression_right_brace = right_b; } ->
      (* TODO: revisit this *)
      Concat [
        t left_b;
        Split;
        let rule =
          if List.is_empty (trailing_trivia left_b)
          && List.is_empty (trailing_trivia expr)
            then Rule.Simple Cost.Base
            else Rule.Parental
        in
        WithRule (rule, Concat [
          Nest [t expr];
          Split;
          t right_b
        ])
      ]
    | EmbeddedBracedExpression {
        embedded_braced_expression_left_brace = left_b;
        embedded_braced_expression_expression = expr;
        embedded_braced_expression_right_brace = right_b; } ->
      (* TODO: Consider finding a way to avoid treating these expressions as
      opportunities for line breaks in long strings:

      $sql = "DELETE FROM `foo` WHERE `left` BETWEEN {$res->left} AND {$res
        ->right} ORDER BY `level` DESC";
      *)
      Concat [
        t left_b;
        Nest [t expr];
        t right_b;
      ]
    | ListExpression {
        list_keyword = kw;
        list_left_paren = lp;
        list_members = members;
        list_right_paren = rp; } ->
      Concat [
        t kw;
        transform_argish lp members rp;
      ]
    | CollectionLiteralExpression {
        collection_literal_name = name;
        collection_literal_left_brace = left_b;
        collection_literal_initializers = initializers;
        collection_literal_right_brace = right_b; } ->
      transform_container_literal ~spaces:true name left_b initializers right_b
    | ObjectCreationExpression {
        object_creation_new_keyword = newkw;
        object_creation_object = what; } ->
      Concat [
        t newkw;
        Space;
        t what;
      ]
    | ConstructorCall {
        constructor_call_type = obj_type;
        constructor_call_left_paren = left_p;
        constructor_call_argument_list = arg_list;
        constructor_call_right_paren = right_p; } ->
      Concat [
        t obj_type;
        transform_argish left_p arg_list right_p;
      ]
    | AnonymousClass {
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
        t classkw;
        transform_argish left_p arg_list right_p;
        when_present extends_kw (fun () -> Concat [
          Space;
          Split;
          WithRule (Rule.Parental, Nest [ Span [
            t extends_kw;
            Space;
            Split;
            WithRule (Rule.Parental, Nest [
              handle_possible_list ~after_each:after_each_ancestor extends
            ])
          ]])
        ]);

        when_present impl_kw (fun () -> Concat [
          Space;
          Split;
          WithRule (Rule.Parental, Nest [ Span [
            t impl_kw;
            Space;
            Split;
            WithRule (Rule.Parental, Nest [
              handle_possible_list ~after_each:after_each_ancestor impls
            ])
          ]])
        ]);
        t body;
      ]
    | ArrayCreationExpression {
        array_creation_left_bracket = left_b;
        array_creation_members = members;
        array_creation_right_bracket = right_b; } ->
      transform_argish left_b members right_b
    | ArrayIntrinsicExpression {
        array_intrinsic_keyword = kw;
        array_intrinsic_left_paren = left_p;
        array_intrinsic_members = members;
        array_intrinsic_right_paren = right_p; } ->
      transform_container_literal kw left_p members right_p
    | DarrayIntrinsicExpression {
        darray_intrinsic_keyword = kw;
        darray_intrinsic_left_bracket = left_p;
        darray_intrinsic_members = members;
        darray_intrinsic_right_bracket = right_p; } ->
      transform_container_literal kw left_p members right_p
    | DictionaryIntrinsicExpression {
        dictionary_intrinsic_keyword = kw;
        dictionary_intrinsic_left_bracket = left_p;
        dictionary_intrinsic_members = members;
        dictionary_intrinsic_right_bracket = right_p; } ->
      transform_container_literal kw left_p members right_p
    | KeysetIntrinsicExpression {
        keyset_intrinsic_keyword = kw;
        keyset_intrinsic_left_bracket = left_p;
        keyset_intrinsic_members = members;
        keyset_intrinsic_right_bracket = right_p; } ->
      transform_container_literal kw left_p members right_p
    | VarrayIntrinsicExpression {
        varray_intrinsic_keyword = kw;
        varray_intrinsic_left_bracket = left_p;
        varray_intrinsic_members = members;
        varray_intrinsic_right_bracket = right_p; } ->
      transform_container_literal kw left_p members right_p
    | VectorIntrinsicExpression {
        vector_intrinsic_keyword = kw;
        vector_intrinsic_left_bracket = left_p;
        vector_intrinsic_members = members;
        vector_intrinsic_right_bracket = right_p; } ->
      transform_container_literal kw left_p members right_p
    | ElementInitializer {
        element_key = key;
        element_arrow = arrow;
        element_value = value; } ->
      transform_mapish_entry key arrow value
    | SubscriptExpression {
        subscript_receiver = receiver;
        subscript_left_bracket = lb;
        subscript_index = expr;
        subscript_right_bracket = rb; } ->
      Concat [
        t receiver;
        transform_braced_item lb expr rb;
      ]
    | AwaitableCreationExpression {
        awaitable_async = async_kw;
        awaitable_coroutine = coroutine_kw;
        awaitable_compound_statement = body; } ->
      Concat [
        t async_kw;
        when_present async_kw space;
        t coroutine_kw;
        when_present coroutine_kw space;
        (* TODO: rethink possible one line bodies *)
        (* TODO: correctly handle spacing after the closing brace *)
        handle_possible_compound_statement ~space:false body;
      ]
    | XHPChildrenDeclaration {
        xhp_children_keyword = kw;
        xhp_children_expression = expr;
        xhp_children_semicolon = semi; } ->
      Concat [
        t kw;
        Space;
        t expr;
        t semi;
        Newline;
      ]
    | XHPChildrenParenthesizedList {
        xhp_children_list_left_paren = left_p;
        xhp_children_list_xhp_children = expressions;
        xhp_children_list_right_paren = right_p; } ->
      Concat [
        transform_argish ~allow_trailing:false left_p expressions right_p;
      ]
    | XHPCategoryDeclaration {
        xhp_category_keyword = kw;
        xhp_category_categories = categories;
        xhp_category_semicolon = semi; } ->
      Concat [
      t kw;
        (* TODO: Eliminate code duplication *)
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split categories;
        ]);
        t semi;
        Newline;
      ]
    | XHPEnumType {
        xhp_enum_optional = opt;
        xhp_enum_keyword = kw;
        xhp_enum_left_brace = left_b;
        xhp_enum_values = values;
        xhp_enum_right_brace = right_b; } ->
      Concat [
        t opt;
        t kw;
        Space;
        transform_argish left_b values right_b;
      ]
    | XHPClassAttributeDeclaration {
        xhp_attribute_keyword = kw;
        xhp_attribute_attributes = xhp_attributes;
        xhp_attribute_semicolon = semi; } ->
      Concat [
        t kw;
        (match syntax xhp_attributes with
        | Missing -> Nothing
        | SyntaxList [attr] ->
          WithRule (Rule.Parental, Nest [Space; Split; t attr])
        | SyntaxList attrs ->
          Nest [handle_list ~before_each:newline attrs]
        | _ -> failwith "Expected SyntaxList"
        );
        t semi;
        Newline;
      ]
    | XHPClassAttribute {
        xhp_attribute_decl_type = attr_type;
        xhp_attribute_decl_name = name;
        xhp_attribute_decl_initializer = init;
        xhp_attribute_decl_required = req; } ->
      (* TODO: figure out nesting here *)
      Concat [
        t attr_type;
        Space;
        t name;
        when_present init space;
        t init;
        when_present req space;
        t req;
      ]
    | XHPSimpleAttribute {
        xhp_simple_attribute_name = name;
        xhp_simple_attribute_equal = eq;
        xhp_simple_attribute_expression = expr; } ->
      Span [
        t name;
        t eq;
        SplitWith Cost.Base;
        Nest [t expr];
      ]
    | XHPSpreadAttribute {
        xhp_spread_attribute_left_brace =l_brace;
        xhp_spread_attribute_spread_operator =spread;
        xhp_spread_attribute_expression =expr;
        xhp_spread_attribute_right_brace = r_brace; } ->
      Span [
        t l_brace;
        t spread;
        SplitWith Cost.Base;
        Nest [t expr];
        t r_brace;
      ]
    | XHPOpen {
        xhp_open_left_angle = left_a;
        xhp_open_name = name;
        xhp_open_attributes = attrs;
        xhp_open_right_angle = right_a; } ->
      Concat [
        t left_a;
        t name;
        match syntax attrs with
        | Missing -> handle_xhp_open_right_angle_token attrs right_a
        | _ ->
          Concat [
            Space;
            Split;
            WithRule (Rule.Parental, Concat [
              Nest [
                handle_possible_list ~after_each:(fun is_last ->
                  if not is_last then space_split () else Nothing
                ) attrs;
              ];
              handle_xhp_open_right_angle_token attrs right_a;
            ])
          ]
      ]
    | XHPExpression {
        xhp_open = xhp_open;
        xhp_body = body;
        xhp_close = close; } ->
      let handle_xhp_body body =
        match syntax body with
        | Missing -> Nothing, true
        | SyntaxList xs ->
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
                    match syntax node with
                    | Token _ -> if has_invisibles leading then Split else Nothing
                    | _ -> Split in
                  Concat [v; transform_xhp_leading_trivia leading]
                end;
              t node;
            ] in
            let open EditableToken in
            prev_token_was_xhpbody := begin
              match syntax node with
              | Token t -> kind t = TokenKind.XHPBody
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
              match syntax node with
              | Token _ when has_newline trailing -> Newline
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
            let open EditableToken in
            kind leading_token <> TokenKind.XHPBody ||
            has_invisibles (leading leading_token)
          in
          let transformed_body = Concat [
            if can_split_before_first_token then Split else Nothing;
            transformed_body;
          ] in
          let can_split_before_close = not !prev_token_was_xhpbody in
          transformed_body, can_split_before_close
        | _ -> failwith "Expected SyntaxList"
      in
      WithPossibleLazyRule (Rule.Parental, t xhp_open,
        let transformed_body, can_split_before_close = handle_xhp_body body in
        Concat [
          Nest [transformed_body];
          when_present close begin fun () ->
            let leading, close = remove_leading_trivia close in Concat [
              (* Ignore extra newlines by treating this as trailing trivia *)
              ignore_trailing_invisibles leading;
              if can_split_before_close then Split else Nothing;
              t close;
            ]
          end;
        ])
    | VarrayTypeSpecifier {
        varray_keyword = kw;
        varray_left_angle = left_a;
        varray_type = varray_type;
        varray_trailing_comma = trailing_comma;
        varray_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a varray_type trailing_comma right_a;
      ]
    | VectorArrayTypeSpecifier {
        vector_array_keyword = kw;
        vector_array_left_angle = left_a;
        vector_array_type = vec_type;
        vector_array_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_braced_item left_a vec_type right_a;
      ]
    | VectorTypeSpecifier {
        vector_type_keyword = kw;
        vector_type_left_angle = left_a;
        vector_type_type = vec_type;
        vector_type_trailing_comma = trailing_comma;
        vector_type_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a vec_type trailing_comma right_a;
      ]
    | KeysetTypeSpecifier {
        keyset_type_keyword = kw;
        keyset_type_left_angle = left_a;
        keyset_type_type = ks_type;
        keyset_type_trailing_comma = trailing_comma;
        keyset_type_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a ks_type trailing_comma right_a;
      ]
    | TypeParameter {
        type_variance = variance;
        type_name = name;
        type_constraints = constraints; } ->
      Concat [
        t variance;
        t name;
        when_present constraints space;
        handle_possible_list constraints
          ~after_each:(fun is_last -> if is_last then Nothing else Space);
      ]
    | TypeConstraint {
        constraint_keyword = kw;
        constraint_type = constraint_type; } ->
      Concat [
        t kw;
        Space;
        t constraint_type;
      ]
    | DarrayTypeSpecifier {
        darray_keyword = kw;
        darray_left_angle = left_a;
        darray_key = key;
        darray_comma = comma_kw;
        darray_value = value;
        darray_trailing_comma = trailing_comma;
        darray_right_angle = right_a; } ->
      let key_list_item = make_list_item key comma_kw in
      let val_list_item = make_list_item value trailing_comma in
      let args = make_list [key_list_item; val_list_item] in
      Concat [
        t kw;
        transform_argish ~allow_trailing:true left_a args right_a;
      ]
    | MapArrayTypeSpecifier {
        map_array_keyword = kw;
        map_array_left_angle = left_a;
        map_array_key = key;
        map_array_comma = comma_kw;
        map_array_value = value;
        map_array_right_angle = right_a; } ->
      Concat [
        t kw;
        let key_list_item = make_list_item key comma_kw in
        let val_list_item = make_list_item value (make_missing ()) in
        let args = make_list [key_list_item; val_list_item] in
        transform_argish ~allow_trailing:false left_a args right_a;
      ]
    | DictionaryTypeSpecifier {
        dictionary_type_keyword = kw;
        dictionary_type_left_angle = left_a;
        dictionary_type_members = members;
        dictionary_type_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_argish left_a members right_a;
      ]
    | ClosureTypeSpecifier {
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
        t outer_left_p;
        t coroutine;
        when_present coroutine space;
        t kw;
        transform_argish_with_return_type
          inner_left_p param_list inner_right_p colon ret_type;
        t outer_right_p;
      ]
    | ClosureParameterTypeSpecifier {
        closure_parameter_call_convention = callconv;
        closure_parameter_type = cp_type; } ->
      Concat [
        t callconv;
        when_present callconv space;
        t cp_type;
      ]
    | ClassnameTypeSpecifier {
        classname_keyword = kw;
        classname_left_angle = left_a;
        classname_type = class_type;
        classname_trailing_comma = trailing_comma;
        classname_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a class_type trailing_comma right_a;
      ]
    | FieldSpecifier {
        field_question = question;
        field_name = name;
        field_arrow = arrow_kw;
        field_type = field_type; } ->
      Concat [
        t question;
        transform_mapish_entry name arrow_kw field_type;
      ]
    | FieldInitializer {
        field_initializer_name = name;
        field_initializer_arrow = arrow_kw;
        field_initializer_value = value; } ->
      transform_mapish_entry name arrow_kw value
    | ShapeTypeSpecifier {
        shape_type_keyword = shape_kw;
        shape_type_left_paren = left_p;
        shape_type_fields = type_fields;
        shape_type_ellipsis = ellipsis;
        shape_type_right_paren = right_p; } ->
      let fields = if is_missing ellipsis
        then type_fields
        else
          let missing_separator = make_missing () in
          let ellipsis_list = [make_list_item ellipsis missing_separator] in
          make_list (children type_fields @ ellipsis_list) in
      transform_container_literal
        ~allow_trailing:(is_missing ellipsis) shape_kw left_p fields right_p
    | ShapeExpression {
        shape_expression_keyword = shape_kw;
        shape_expression_left_paren = left_p;
        shape_expression_fields = fields;
        shape_expression_right_paren = right_p; } ->
      transform_container_literal shape_kw left_p fields right_p
    | TupleExpression {
        tuple_expression_keyword = kw;
        tuple_expression_left_paren = left_p;
        tuple_expression_items = items;
        tuple_expression_right_paren = right_p; } ->
      Concat [
        t kw;
        transform_argish left_p items right_p;
      ]
    | TypeArguments {
        type_arguments_left_angle = left_a;
        type_arguments_types = type_list;
        type_arguments_right_angle = right_a; } ->
      transform_argish left_a type_list right_a
    | TypeParameters {
        type_parameters_left_angle = left_a;
        type_parameters_parameters = param_list;
        type_parameters_right_angle = right_a; } ->
      transform_argish left_a param_list right_a
    | TupleTypeSpecifier {
        tuple_left_paren = left_p;
        tuple_types = types;
        tuple_right_paren = right_p; } ->
      transform_argish left_p types right_p
    | TupleTypeExplicitSpecifier {
        tuple_type_keyword = kw;
        tuple_type_left_angle = left_a;
        tuple_type_types = types;
        tuple_type_right_angle = right_a; } ->
      Concat [
        t kw;
        transform_argish left_a types right_a
      ]
    | DecoratedExpression {
        decorated_expression_decorator = op;
        decorated_expression_expression = expr; } ->
      Concat [
        t op;
        begin
          let open EditableToken in
          match syntax op with
          | Token t when kind t = TokenKind.Inout -> Space
          | _ -> Nothing
        end;
        t expr;
      ]
    | ErrorSyntax _ ->
      raise Hackfmt_error.InvalidSyntax

  and when_present node f =
    match syntax node with
    | Missing -> Nothing
    | _ -> f ()

  and transform_simple node =
    Concat (List.map (children node) t)

  and transform_simple_statement node =
    Concat ((List.map (children node) t) @ [Newline])

  and braced_block_nest ?(allow_collapse=true) open_b close_b nodes =
    let nodes = Concat nodes in
    match allow_collapse, has_printable_content nodes, syntax open_b with
    | true, false, Token ob
      when List.for_all (EditableToken.trailing ob)
        (fun t -> Trivia.kind t <> TriviaKind.EndOfLine) ->
      Concat [
        t open_b;
        t close_b;
      ]
    | _ ->
      (* Remove the closing brace's leading trivia and handle it inside the
       * BlockNest, so that comments will be indented correctly. *)
      let leading, close_b = remove_leading_trivia close_b in
      Concat [
        t open_b;
        Newline;
        BlockNest [
          nodes;
          transform_leading_trivia leading;
          Newline;
        ];
        t close_b;
      ]

  and delimited_nest
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
      t left_delim;
      WithRule (rule,
        nest ~spaces right_delim nodes
      );
    ]

  and nest ?(spaces=false) right_delim nodes =
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
      t right_delim;
    ]

  and after_each_argument is_last =
    if is_last
    then Split
    else space_split ()

  and handle_lambda_body node =
    match syntax node with
    | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace; } ->
      handle_compound_statement ~allow_collapse:true
        (compound_left_brace, compound_statements, compound_right_brace);
    | XHPExpression _ ->
      WithRule (Rule.Parental, Concat [
        Space;
        Split;
        Nest [t node];
      ])
    | _ ->
      Concat [
        Space;
        SplitWith Cost.Base;
        Nest [t node];
      ]

  and handle_possible_compound_statement
      ?(space=true)
      ?(allow_collapse=false)
      node
    =
    match syntax node with
    | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace; } ->
      Concat [
        handle_compound_statement ~allow_collapse
          (compound_left_brace, compound_statements, compound_right_brace);
        if space then Space else Nothing;
      ]
    | _ ->
      Concat [
        Newline;
        BlockNest [
          t node
        ];
      ]

  and handle_compound_statement ?(allow_collapse=false) (left_b, statements, right_b) =
    Concat [
      Space;
      braced_block_nest ~allow_collapse left_b right_b [
        handle_possible_list statements
      ];
    ]

  (**
   * Special-case handling for lists of declarators, where we want the splits
   * between declarators to break if their children break, but we want a single
   * declarator to stay joined with the line preceding it if it fits, even when
   * its children break.
   *)
  and handle_declarator_list declarators =
    match syntax declarators with
    | Missing -> Nothing
    | SyntaxList [declarator] ->
      Nest [
        Space;
        (* Use an independent split, so we don't break just because a line break
         * occurs in the declarator. *)
        SplitWith Cost.Base;
        t declarator;
      ];
    | SyntaxList xs ->
      (* Use Rule.Parental to break each declarator onto its own line if any
       * line break occurs in a declarator, or if they can't all fit onto one
       * line. *)
      WithRule (Rule.Parental, Nest (List.map xs (fun declarator -> Concat [
        Space;
        Split;
        t declarator;
      ])));
    | _ -> failwith "SyntaxList expected"

  and handle_list
      ?(before_each=(fun () -> Nothing))
      ?(after_each=(fun _is_last -> Nothing))
      ?(handle_last=t)
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
          t hd;
          after_each false;
          aux tl
        ]
      | [] -> Nothing
    ) in
    aux list

  and handle_possible_list
      ?(before_each=(fun () -> Nothing))
      ?(after_each=(fun _is_last -> Nothing))
      ?(handle_last=t)
      node =
    match syntax node with
    | Missing -> Nothing
    | SyntaxList x -> handle_list x ~before_each ~after_each ~handle_last
    | _ -> handle_list [node] ~before_each ~after_each ~handle_last

  and handle_xhp_open_right_angle_token attrs node =
    match syntax node with
    | Token token ->
      Concat [
        if EditableToken.text token = "/>"
          then Concat [Space; when_present attrs split]
          else Nothing;
        t node
      ]
    | _ -> failwith "expected xhp_open right_angle token"

  and handle_function_call_expression (receiver, lp, args, rp) =
    match syntax receiver with
    | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name; } ->
      handle_possible_chaining
        (member_object, member_operator, member_name)
        (Some (lp, args, rp))
    | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name; } ->
      handle_possible_chaining
        (safe_member_object, safe_member_operator, safe_member_name)
        (Some (lp, args, rp))
    | _ ->
      Concat [
        t receiver;
        transform_argish lp args rp
      ]

  and handle_function_call_with_type_arguments_expression (receiever, tyargs, lp, args, rp) =
    match syntax receiever with
    | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name; } ->
      handle_possible_chaining
        (member_object, member_operator, member_name)
        (Some (lp, args, rp))
    | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name; } ->
      handle_possible_chaining
        (safe_member_object, safe_member_operator, safe_member_name)
        (Some (lp, args, rp))
    | _ ->
      Concat [
        t receiever;
        t tyargs;
        transform_argish lp args rp
      ]

  and handle_possible_chaining (obj, arrow1, member1) argish =
    let rec handle_chaining obj =
      let handle_mse_or_smse (obj, arrow, member) fun_paren_args =
        let (obj, l) = handle_chaining obj in
        obj, l @ [(arrow, member, fun_paren_args)]
      in
      match syntax obj with
      | FunctionCallExpression {
          function_call_receiver = receiver;
          function_call_left_paren = lp;
          function_call_argument_list = args;
          function_call_right_paren = rp; } ->
        (match syntax receiver with
          | MemberSelectionExpression {
              member_object;
              member_operator;
              member_name; } ->
            handle_mse_or_smse
              (member_object, member_operator, member_name)
              (Some (lp, args, rp))
          | SafeMemberSelectionExpression {
              safe_member_object;
              safe_member_operator;
              safe_member_name; } ->
            handle_mse_or_smse
              (safe_member_object, safe_member_operator, safe_member_name)
              (Some (lp, args, rp))
          | _ -> obj, []
        )
      | MemberSelectionExpression {
          member_object;
          member_operator;
          member_name; } ->
        handle_mse_or_smse
          (member_object, member_operator, member_name)
          None
      | SafeMemberSelectionExpression {
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
        t arrow;
        t member;
        Option.value_map argish ~default:Nothing
          ~f:(fun (lp, args, rp) -> transform_argish lp args rp);
      ]
    in
    match chain_list with
    | hd :: [] ->
      Concat [
        Span [t obj];
        if node_has_trailing_newline obj
        then Newline
        else SplitWith Cost.SimpleMemberSelection;
        Nest [transform_chain hd];
      ]
    | hd :: tl ->
      let rule_type = match hd with
        | (_, trailing, None)
        | (_, _, Some (_, _, trailing)) ->
          if node_has_trailing_newline trailing then Rule.Always else Rule.Parental
      in
      Span [
        WithLazyRule (rule_type,
          Concat [
            t obj;
            if node_has_trailing_newline obj
            then Newline
            else SplitWith Cost.Base;
          ],
          Concat [
            (* This needs to be nested separately due to the above SplitWith *)
            Nest [transform_chain hd];
            Nest (List.map tl ~f:(fun x -> Concat [Split; transform_chain x]))
          ])
      ]
    | _ -> failwith "Expected a chain of at least length 1"

  and transform_fn_decl_name modifiers kw amp name type_params leftp =
    let mods = handle_possible_list ~after_each:(fun _ -> Space) modifiers in
    [
      mods;
      t kw;
      Space;
      t amp;
      t name;
      t type_params;
      t leftp;
      Split;
    ]

  and transform_fn_decl_args params rightp colon ret_type where =
    (* It is a syntax error to follow a variadic parameter with a trailing
     * comma, so suppress trailing commas in that case. *)
    let allow_trailing =
      match syntax params with
      | SyntaxList params ->
        let open EditableToken in
        let open EditableToken.TokenKind in
        let last_param =
          match syntax (List.last_exn params) with
          | ListItem { list_item; _ } -> list_item
          | _ -> failwith "Expected ListItem"
        in
        begin
          match syntax last_param with
          | VariadicParameter _
          | ParameterDeclaration {
              parameter_name = { syntax = DecoratedExpression {
                decorated_expression_decorator = {
                  syntax = Token { kind = DotDotDot; _ }; _
                }; _
              }; _ }; _
            } ->
            false
          | _ -> true
        end
      | _ -> true
    in
    WithRule (Rule.Parental, Concat [
      transform_possible_comma_list ~allow_trailing params rightp;
      t colon;
      when_present colon space;
      t ret_type;
      when_present where space;
      t where;
    ])

  and transform_argish_with_return_type left_p params right_p colon ret_type =
    Concat [
      t left_p;
      when_present params split;
      WithRule (Rule.Parental, Span [
        Span [ transform_possible_comma_list params right_p ];
        t colon;
        when_present colon space;
        t ret_type;
      ])
    ]

  and transform_argish
      ?(allow_trailing=true) ?(force_newlines=false) ?(spaces=false)
      left_p arg_list right_p =
    (* When there is only one argument, with no surrounding whitespace in the
     * original source, allow that style to be preserved even when there are
     * line breaks within the argument (normally these would force the splits
     * around the argument to break). *)
    let split_when_children_split =
      match spaces, syntax arg_list with
      | false, SyntaxList [x] ->
        let has_surrounding_whitespace =
          not (
            List.is_empty (trailing_trivia left_p) &&
            List.is_empty (trailing_trivia x)
          )
        in
        let item =
          match syntax x with
          | ListItem x -> x.list_item
          | _ -> failwith "Expected ListItem"
        in
        (* Blacklist constructs which look ugly when we try to preserve the
         * lack-of-whitespace style. *)
        (match syntax item with
        | LambdaExpression
            { lambda_body = { syntax = CompoundStatement _; _ }; _ } ->
          has_surrounding_whitespace
        | FunctionCallExpression { function_call_receiver; _ } ->
          Syntax.is_member_selection_expression function_call_receiver ||
            has_surrounding_whitespace
        | ConditionalExpression _
        | BinaryExpression _
        | MemberSelectionExpression _
        | FieldSpecifier _
        | FieldInitializer _
        | ElementInitializer _
        | LambdaExpression _
          -> true
        | _ -> has_surrounding_whitespace
        )
      | _ -> true
    in
    delimited_nest
      ~spaces
      ~split_when_children_split
      ~force_newlines
      left_p
      right_p
      [transform_arg_list ~allow_trailing arg_list]

  and transform_braced_item left_p item right_p =
    delimited_nest left_p right_p [t item]

  and transform_trailing_comma ~allow_trailing item comma =
    let open EditableToken in
    (* PHP does not permit trailing commas in function calls. Rather than try to
     * account for where PHP's parser permits trailing commas, we just never add
     * them in PHP files. *)
    let allow_trailing = allow_trailing && env.Env.add_trailing_commas in
    match syntax comma with
    | Token tok ->
      Concat [
        t item;
        transform_leading_trivia (leading tok);
        if allow_trailing then TrailingComma true else Nothing;
        Ignore (text tok, width tok);
        transform_trailing_trivia (trailing tok);
      ]
    | Missing ->
      let item, item_trailing = remove_trailing_trivia item in
      Concat [
        t item;
        if allow_trailing then TrailingComma false else Nothing;
        transform_trailing_trivia item_trailing;
      ]
    | _ -> failwith "Expected Token"

  and transform_braced_item_with_trailer left_p item comma right_p =
    delimited_nest left_p right_p
      (* TODO: turn allow_trailing:true when HHVM versions that don't support
         trailing commas in all these places reach end-of-life. *)
      [transform_trailing_comma ~allow_trailing:false item comma]

  and transform_arg_list ?(allow_trailing=true) items =
    handle_possible_list items
      ~after_each:after_each_argument
      ~handle_last:(transform_last_arg ~allow_trailing)

  and transform_possible_comma_list ?(allow_trailing=true) ?(spaces=false)
      items right_p =
    nest ~spaces right_p [
      transform_arg_list ~allow_trailing items
    ]

  and transform_container_literal
      ?(spaces=false) ?allow_trailing kw left_p members right_p =
    let force_newlines = node_has_trailing_newline left_p in
    Concat [
      t kw;
      if spaces then Space else Nothing;
      transform_argish
        ~spaces ~force_newlines ?allow_trailing left_p members right_p;
    ]

  and remove_leading_trivia node =
    match Syntax.leading_token node with
    | None -> [], node
    | Some leading_token ->
      let rewritten_node = Rewriter.rewrite_pre (fun rewrite_node ->
        match syntax rewrite_node with
        | Token t when t == leading_token ->
          Rewriter.Replace
            (Syntax.make_token {t with EditableToken.leading = []})
        | _  -> Rewriter.Keep
      ) node in
      EditableToken.leading leading_token, rewritten_node

  and remove_trailing_trivia node =
    match Syntax.trailing_token node with
    | None -> node, []
    | Some trailing_token ->
      let rewritten_node = Rewriter.rewrite_pre (fun rewrite_node ->
        match syntax rewrite_node with
        | Token t when t == trailing_token ->
          Rewriter.Replace
            (Syntax.make_token {t with EditableToken.trailing = []})
        | _  -> Rewriter.Keep
      ) node in
      rewritten_node, EditableToken.trailing trailing_token

  and transform_last_arg ~allow_trailing node =
    match syntax node with
    | ListItem {
        list_item = item;
        list_separator = separator; } ->
      transform_trailing_comma ~allow_trailing item separator
    | _ -> failwith "Expected ListItem"


  and transform_mapish_entry key arrow value =
    Concat [
      t key;
      Space;
      t arrow;
      Space;
      SplitWith Cost.Base;
      Nest [t value];
    ]

  and transform_keyword_expression_statement kw expr semi =
    Concat [
      t kw;
      when_present expr (fun () -> Concat [
        Space;
        SplitWith Cost.Base;
        Nest [t expr];
      ]);
      t semi;
      Newline;
    ]

  and transform_keyword_expr_list_statement kw expr_list semi =
    Concat [
      t kw;
      handle_declarator_list expr_list;
      t semi;
      Newline;
    ]

  and transform_condition left_p condition right_p =
    Concat [
      t left_p;
      Split;
      WithRule (Rule.Parental, Concat [
        Nest [t condition];
        Split;
        t right_p;
      ])
    ]

  and transform_binary_expression ~is_nested (left, operator, right) =
    let get_operator_type op =
      match syntax op with
      | Token t -> Full_fidelity_operator.trailing_from_token
        (EditableToken.kind t)
      | _ -> failwith "Operator should always be a token"
    in
    let is_concat op =
      get_operator_type op = Full_fidelity_operator.ConcatenationOperator in
    let operator_has_surrounding_spaces op = not (is_concat op) in
    let operator_is_leading op =
      get_operator_type op = Full_fidelity_operator.PipeOperator in
    let operator_preserves_newlines op =
      get_operator_type op = Full_fidelity_operator.PipeOperator in

    let operator_t = get_operator_type operator in

    if Full_fidelity_operator.is_comparison operator_t then
      WithLazyRule (Rule.Parental,
        Concat [
          t left;
          Space;
          t operator;
        ],
        Concat [
          Space;
          Split;
          Nest [t right];
        ])
    else if Full_fidelity_operator.is_assignment operator_t then
      Concat [
        t left;
        Space;
        t operator;
        Space;
        SplitWith Cost.NoCost;
        Nest [t right];
      ]
    else
      Concat [
        let precedence = Full_fidelity_operator.precedence operator_t in

        let rec flatten_expression expr =
          match syntax expr with
          | BinaryExpression {
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
          match syntax operand with
          | BinaryExpression {
              binary_left_operand;
              binary_operator;
              binary_right_operand; } ->
            transform_binary_expression ~is_nested:true
              (binary_left_operand, binary_operator, binary_right_operand)
          | _ -> t operand
        in

        let binary_expression_syntax_list =
          flatten_expression (make_binary_expression left operator right) in
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
                      then ConcatOperator (t op)
                      else t op;
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
              if i = len || str.[i] <> ' '
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
          let dc = Trivia.make_delimited_comment @@
            String.sub str start_index len in
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
  in

  t node
