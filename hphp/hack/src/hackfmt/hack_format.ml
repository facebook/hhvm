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

open Core
open Syntax
open Doc

let transform (env: Env.t) (node: Syntax.t) : Doc.t =
  let rec t node =
    match syntax node with
    | Missing ->
      Nothing
    | Token x ->
      let open EditableToken in
      Concat [
        transform_leading_trivia (leading x);
        begin
          let open TokenKind in
          match kind x with
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
      let token = get_end_of_file_children x in
      t token
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
          handle_possible_list (get_script_children x);
        ]
      end
    | LiteralExpression x ->
      (* Double quoted string literals can create a list *)
      let children = get_literal_expression_children x in
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
      begin match syntax children with
        | Token tok -> wrap_with_literal_type tok (t children)
        | SyntaxList l ->
          let last = trailing_token children in
          begin match last with
            | Some tok -> wrap_with_literal_type tok (Concat (List.map l t))
            | _ -> failwith "Expected Token"
          end
        | _ -> failwith "Expected Token or SyntaxList"
      end
    | MarkupSection x ->
      let (prefix, text, suffix, _) = get_markup_section_children x in
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
    | QualifiedNameExpression _
    | PipeVariableExpression _
    | PropertyDeclarator _
    | ConstantDeclarator _
    | DecoratedExpression _
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
    | ExpressionStatement _ ->
      transform_simple_statement node
    | EnumDeclaration x ->
      let (attr, kw, name, colon_kw, base, enum_type, left_b, enumerators,
        right_b) = get_enum_declaration_children x in
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
    | Enumerator x ->
      let (name, eq_kw, value, semi) = get_enumerator_children x in
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
    | AliasDeclaration x ->
      (* TODO: revisit this for long names *)
      let (attr, kw, name, generic, type_constraint, eq_kw, alias_type, semi) =
        get_alias_declaration_children x in
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
    | PropertyDeclaration x ->
      let (modifiers, prop_type, declarators, semi) =
        get_property_declaration_children x in
      Concat [
        handle_possible_list ~after_each:(fun _ -> Space) modifiers;
        t prop_type;
        handle_declarator_list declarators;
        t semi;
        Newline;
      ]
    | NamespaceDeclaration x ->
      let (kw, name, body) = get_namespace_declaration_children x in
      Concat [
        t kw;
        Space;
        t name;
        t body;
        Newline;
      ]
    | NamespaceBody x ->
      let (left_b, decls, right_b) = get_namespace_body_children x in
      Concat [
        Space;
        braced_block_nest left_b right_b [handle_possible_list decls];
      ]
    | NamespaceEmptyBody x ->
      let semi = get_namespace_empty_body_children x in
      Concat [
        t semi;
      ]
    | NamespaceUseDeclaration x ->
      let (kw, use_kind, clauses, semi) =
        get_namespace_use_declaration_children x in
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
    | NamespaceGroupUseDeclaration x ->
      let (kw, use_kind, prefix, left_b, clauses, right_b, semi) =
        get_namespace_group_use_declaration_children x in
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
    | NamespaceUseClause x ->
      let (use_kind, name, as_kw, alias) =
        get_namespace_use_clause_children x
      in
      Concat [
        t use_kind;
        t name;
        when_present as_kw space;
        t as_kw;
        when_present alias space;
        t alias;
      ]
    | FunctionDeclaration x ->
      let (attr, header, body) = get_function_declaration_children x in
      Concat [
        t attr;
        when_present attr newline;
        t header;
        handle_possible_compound_statement body;
        Newline;
      ]
    | FunctionDeclarationHeader x ->
      let (
        async,
        coroutine,
        kw,
        amp,
        name,
        type_params,
        leftp,
        params,
        rightp,
        colon,
        ret_type,
        where
      ) = get_function_declaration_header_children x
      in
      Concat [
        Span (
          transform_fn_decl_name async coroutine kw amp name type_params leftp);
        transform_fn_decl_args params rightp colon ret_type where;
      ]
    | WhereClause x ->
      let (where, constraints) = get_where_clause_children x in
      Concat [
        t where;
        Space;
        handle_possible_list constraints ~after_each:(fun _ -> Space);
      ]
    | WhereConstraint x ->
      let (left, op, right) = get_where_constraint_children x in
      Concat [
        t left;
        Space;
        t op;
        Space;
        t right;
      ]
    | MethodishDeclaration x ->
      let (attr, modifiers, func_decl, body, semi) =
        get_methodish_declaration_children x
      in
      Concat [
        t attr;
        when_present attr newline;
        (
          let mods =
            handle_possible_list ~after_each:(fun _ -> Space) modifiers
          in
          let fn_name, args_and_where = match syntax func_decl with
            | FunctionDeclarationHeader x ->
              let (
                async,
                coroutine,
                kw,
                amp,
                name,
                type_params,
                leftp,
                params,
                rightp,
                colon,
                ret_type,
                where
              ) = get_function_declaration_header_children x
              in
              Concat (
                transform_fn_decl_name
                  async
                  coroutine
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
            Span [mods; fn_name];
            args_and_where;
          ]
        );
        when_present body (fun () -> handle_possible_compound_statement body);
        t semi;
        Newline;
      ]
    | ClassishDeclaration x ->
      let (attr, modifiers, kw, name, type_params, extends_kw, extends,
        impl_kw, impls, body) = get_classish_declaration_children x
      in
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
    | ClassishBody x ->
      let (left_b, body, right_b) = get_classish_body_children x in
      Concat [
        Space;
        braced_block_nest left_b right_b [
          handle_possible_list body
        ];
        Newline;
      ]
    | TraitUsePrecedenceItem x ->
      let (name, kw, removed_names) =
        get_trait_use_precedence_item_children x
      in
      Concat [
        t name;
        Space;
        t kw;
        Space;
        t removed_names;
        Newline;
      ]
    | TraitUseAliasItem x ->
      let (aliasing_name, kw, visibility, aliased_name) =
        get_trait_use_alias_item_children x
      in
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
    | TraitUseConflictResolution x ->
      let (kw, elements, lb, clauses, rb) =
        get_trait_use_conflict_resolution_children x
      in
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
    | TraitUse x ->
      let (kw, elements, semi) = get_trait_use_children x in
      Concat [
        t kw;
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split elements;
        ]);
        t semi;
        Newline;
      ]
    | RequireClause x ->
      let (kw, kind, name, semi) = get_require_clause_children x in
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
    | ConstDeclaration x ->
      let (abstr, kw, const_type, declarators, semi) =
        get_const_declaration_children x in
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
    | TypeConstDeclaration x ->
      let (abs, kw, type_kw, name, type_constraint, eq, type_spec, semi) =
        get_type_const_declaration_children x in
      Concat [
        t abs;
        Space;
        t kw;
        Space;
        t type_kw;
        Space;
        t name;
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
    | ParameterDeclaration x ->
      let (attr, visibility, param_type, name, default) =
        get_parameter_declaration_children x
      in
      Concat [
        t attr;
        t visibility;
        when_present visibility space;
        t param_type;
        if is_missing visibility && is_missing param_type
        then t name
        else Concat [
          Space;
          SplitWith Cost.Base;
          Nest [t name];
        ];
        t default;
      ]
    | VariadicParameter x ->
      let ellipsis = get_variadic_parameter_children x in
      t ellipsis;
    | AttributeSpecification x ->
      let (left_da, attrs, right_da) = get_attribute_specification_children x in
      transform_argish ~allow_trailing:false left_da attrs right_da
    | Attribute x ->
      let (name, left_p, values, right_p) = get_attribute_children x in
      Concat [
        t name;
        transform_argish left_p values right_p;
      ]
    | InclusionExpression x ->
      let (kw, expr) = get_inclusion_expression_children x in
      Concat [
        t kw;
        Space;
        SplitWith Cost.Base;
        t expr;
      ]
    | InclusionDirective x ->
      let (expr, semi) = get_inclusion_directive_children x in
      Concat [
        t expr;
        t semi;
        Newline;
      ]
    | CompoundStatement x ->
      Concat [
        handle_compound_statement x;
        Newline;
      ]
    | UnsetStatement x ->
      let (kw, left_p, args, right_p, semi) = get_unset_statement_children x in
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
    | IfStatement x ->
      let (kw, left_p, condition, right_p, if_body,
        elseif_clauses, else_clause) = get_if_statement_children x in
      Concat [
        t kw;
        Space;
        transform_condition left_p condition right_p;
        handle_possible_compound_statement if_body;
        handle_possible_list elseif_clauses;
        t else_clause;
        Newline;
      ]
    | ElseifClause x ->
      let (kw, left_p, condition, right_p, body) =
        get_elseif_clause_children x
      in
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
    | TryStatement x ->
      (* TODO: revisit *)
      let (kw, body, catch_clauses, finally_clause) =
        get_try_statement_children x in
      Concat [
        t kw;
        handle_possible_compound_statement body;
        handle_possible_list catch_clauses;
        t finally_clause;
        Newline;
      ]
    | CatchClause x ->
      let (kw, left_p, ex_type, var, right_p, body) =
        get_catch_clause_children x in
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
    | FinallyClause x ->
      let (kw, body) = get_finally_clause_children x in
      Concat [
        t kw;
        Space;
        handle_possible_compound_statement body;
      ]
    | DoStatement x ->
      let (do_kw, body, while_kw, left_p, cond, right_p, semi) =
        get_do_statement_children x in
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
    | ForStatement x ->
      let (kw, left_p, init, semi1, control, semi2, after_iter, right_p, body) =
        get_for_statement_children x in
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
    | ForeachStatement x ->
      let (kw, left_p, collection, await_kw, as_kw, key, arrow, value, right_p,
        body) = get_foreach_statement_children x in
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
    | SwitchStatement x ->
      let (kw, left_p, expr, right_p, left_b, sections, right_b) =
        get_switch_statement_children x in
      let sections = syntax_node_to_list sections in
      Concat [
        t kw;
        Space;
        delimited_nest left_p right_p [t expr];
        Space;
        braced_block_nest left_b right_b (List.map sections t);
        Newline;
      ]
    | SwitchSection x ->
      let (labels, statements, fallthrough) = get_switch_section_children x in
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
          BlockNest [transform_leading_trivia upto_fallthrough];
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
    | CaseLabel x ->
      let (kw, expr, colon) = get_case_label_children x in
      Concat [
        t kw;
        Space;
        SplitWith Cost.Base;
        t expr;
        t colon;
      ]
    | DefaultLabel x ->
      let (kw, colon) = get_default_label_children x in
      Concat [
        t kw;
        t colon;
      ]
    | SwitchFallthrough x ->
      let (kw, semi) = get_switch_fallthrough_children x in
      Concat [
        t kw;
        t semi;
      ]
    | ReturnStatement x ->
      let (kw, expr, semi) = get_return_statement_children x in
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
    | ThrowStatement x ->
      let (kw, expr, semi) = get_throw_statement_children x in
      transform_keyword_expression_statement kw expr semi
    | BreakStatement x ->
      let (kw, expr, semi) = get_break_statement_children x in
      transform_keyword_expression_statement kw expr semi
    | ContinueStatement x ->
      let (kw, level, semi) = get_continue_statement_children x in
      transform_keyword_expression_statement kw level semi
    | FunctionStaticStatement x ->
      let (static_kw, declarators, semi) =
        get_function_static_statement_children x in
      transform_keyword_expr_list_statement static_kw declarators semi
    | EchoStatement x ->
      let (kw, expr_list, semi) = get_echo_statement_children x in
      transform_keyword_expr_list_statement kw expr_list semi
    | GlobalStatement x ->
      let (kw, var_list, semi) = get_global_statement_children x in
      transform_keyword_expr_list_statement kw var_list semi
    | SimpleInitializer x ->
      let (eq_kw, value) = get_simple_initializer_children x in
      Concat [
        Space;
        t eq_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t value];
      ]
    | AnonymousFunction x ->
      let (
        static_kw,
        async_kw,
        coroutine_kw,
        fun_kw,
        lp,
        params,
        rp,
        colon,
        ret_type,
        use,
        body
      ) = get_anonymous_function_children x in
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
        handle_possible_compound_statement ~space:false body;
      ]
    | AnonymousFunctionUseClause x ->
      (* TODO: Revisit *)
      let (kw, left_p, vars, right_p) =
        get_anonymous_function_use_clause_children x in
      Concat [
        Space;
        t kw;
        Space;
        transform_argish left_p vars right_p;
      ]
    | LambdaExpression x ->
      let (async, coroutine, signature, arrow, body) =
        get_lambda_expression_children x in
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
    | LambdaSignature x ->
      let (lp, params, rp, colon, ret_type) = get_lambda_signature_children x in
      transform_argish_with_return_type lp params rp colon ret_type
    | CastExpression _ ->
      Span (List.map (children node) t)
    | MemberSelectionExpression x ->
      handle_possible_chaining
        (get_member_selection_expression_children x)
        None
    | SafeMemberSelectionExpression x ->
      handle_possible_chaining
        (get_safe_member_selection_expression_children x)
        None
    | YieldExpression x ->
      let (kw, operand) = get_yield_expression_children x in
      Concat [
        t kw;
        Space;
        SplitWith Cost.Base;
        Nest [t operand];
      ]
    | YieldFromExpression x ->
      let (yield_kw, from_kw, operand) = get_yield_from_expression_children x in
      Concat [
        t yield_kw;
        Space;
        t from_kw;
        Space;
        SplitWith Cost.Base;
        Nest [t operand];
      ]
    | PrefixUnaryExpression x ->
      let (operator, operand) = get_prefix_unary_expression_children x in
      Concat [
        t operator;
        (match syntax operator with
          | Token x ->
            let open EditableToken in
            if   kind x = TokenKind.Await
              || kind x = TokenKind.Clone
              || kind x = TokenKind.Print  then Space
            else Nothing
          | _ -> Nothing
        );
        t operand;
      ]
    | BinaryExpression x ->
      transform_binary_expression ~is_nested:false x
    | InstanceofExpression x ->
      let (left, kw, right) = get_instanceof_expression_children x in
      Concat [
        t left;
        Space;
        t kw;
        Space;
        SplitWith Cost.Base;
        Nest [t right];
      ]
    | ConditionalExpression x ->
      let (test_expr, q_kw, true_expr, c_kw, false_expr) =
        get_conditional_expression_children x in
      WithLazyRule (Rule.Parental,
        t test_expr,
        Nest [
          Space;
          Split;
          t q_kw;
          when_present true_expr (fun () -> Concat [
            Space;
            if Env.indent_width env = 2
              then Nest [t true_expr]
              else t true_expr;
            Space;
            Split;
          ]);
          t c_kw;
          Space;
          if not (is_missing true_expr) && Env.indent_width env = 2
            then Nest [t false_expr]
            else t false_expr;
        ])
    | FunctionCallExpression x ->
      handle_function_call_expression x
    | FunctionCallWithTypeArgumentsExpression x ->
      handle_function_call_with_type_arguments_expression x
    | EvalExpression x ->
      let (kw, left_p, arg, right_p) = get_eval_expression_children x in
      Concat [
        t kw;
        transform_braced_item left_p arg right_p;
      ]
    | EmptyExpression x ->
      let (kw, left_p, arg, right_p) = get_empty_expression_children x in
      Concat [
        t kw;
        transform_braced_item left_p arg right_p;
      ]
    | IssetExpression x ->
      let (kw, left_p, args, right_p) = get_isset_expression_children x in
      Concat [
        t kw;
        transform_argish ~allow_trailing:false left_p args right_p;
      ]
    | DefineExpression x ->
      let (kw, left_p, args, right_p) = get_define_expression_children x in
      Concat [
        t kw;
        transform_argish left_p args right_p;
      ]
    | ParenthesizedExpression x ->
      let (left_p, expr, right_p) = get_parenthesized_expression_children x in
      Concat [
        t left_p;
        Split;
        WithRule (Rule.Parental, Concat [
          Nest [ t expr; ];
          Split;
          t right_p
        ]);
      ]
    | BracedExpression x ->
      (* TODO: revisit this *)
      let (left_b, expr, right_b) = get_braced_expression_children x in
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
    | EmbeddedBracedExpression x ->
      (* TODO: Consider finding a way to avoid treating these expressions as
      opportunities for line breaks in long strings:

      $sql = "DELETE FROM `foo` WHERE `left` BETWEEN {$res->left} AND {$res
        ->right} ORDER BY `level` DESC";
      *)
      let (left_b, expr, right_b) = get_embedded_braced_expression_children x in
      Concat [
        t left_b;
        Nest [t expr];
        t right_b;
      ]
    | ListExpression x ->
      let (kw, lp, members, rp) = get_list_expression_children x in
      Concat [
        t kw;
        transform_argish lp members rp;
      ]
    | CollectionLiteralExpression x ->
      let (name, left_b, initializers, right_b) =
        get_collection_literal_expression_children x
      in
      transform_container_literal ~spaces:true name left_b initializers right_b
    | ObjectCreationExpression x ->
      let (kw, obj_type, left_p, arg_list, right_p) =
        get_object_creation_expression_children x
      in
      Concat [
        t kw;
        Space;
        t obj_type;
        transform_argish left_p arg_list right_p;
      ]
    | ArrayCreationExpression x ->
      let (left_b, members, right_b) =
        get_array_creation_expression_children x
      in
      transform_argish left_b members right_b
    | ArrayIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_array_intrinsic_expression_children x
      in
      transform_container_literal kw left_p members right_p
    | DarrayIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_darray_intrinsic_expression_children x in
      transform_container_literal kw left_p members right_p
    | DictionaryIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_dictionary_intrinsic_expression_children x
      in
      transform_container_literal kw left_p members right_p
    | KeysetIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_keyset_intrinsic_expression_children x
      in
      transform_container_literal kw left_p members right_p
    | VarrayIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_varray_intrinsic_expression_children x in
      transform_container_literal kw left_p members right_p
    | VectorIntrinsicExpression x ->
      let (kw, left_p, members, right_p) =
        get_vector_intrinsic_expression_children x
      in
      transform_container_literal kw left_p members right_p
    | ElementInitializer x ->
      let (key, arrow, value) = get_element_initializer_children x in
      transform_mapish_entry key arrow value
    | SubscriptExpression x ->
      let (receiver, lb, expr, rb) = get_subscript_expression_children x in
      Concat [
        t receiver;
        transform_braced_item lb expr rb;
      ]
    | AwaitableCreationExpression x ->
      let (async_kw, coroutine_kw, body) =
        get_awaitable_creation_expression_children x in
      Concat [
        t async_kw;
        when_present async_kw space;
        t coroutine_kw;
        when_present coroutine_kw space;
        (* TODO: rethink possible one line bodies *)
        (* TODO: correctly handle spacing after the closing brace *)
        handle_possible_compound_statement ~space:false body;
      ]
    | XHPChildrenDeclaration x ->
      let (kw, expr, semi) = get_xhp_children_declaration_children x in
      Concat [
        t kw;
        Space;
        t expr;
        t semi;
        Newline;
      ]
    | XHPChildrenParenthesizedList x ->
      let (left_p, expressions, right_p) =
        get_xhp_children_parenthesized_list_children x in
      Concat [
        transform_argish ~allow_trailing:false left_p expressions right_p;
      ]
    | XHPCategoryDeclaration x ->
      let (kw, categories, semi) = get_xhp_category_declaration_children x in
      Concat [
      t kw;
        (* TODO: Eliminate code duplication *)
        WithRule (Rule.Parental, Nest [
          handle_possible_list ~before_each:space_split categories;
        ]);
        t semi;
        Newline;
      ]
    | XHPEnumType x ->
      let (kw, left_b, values, right_b) = get_xhp_enum_type_children x in
      Concat [
        t kw;
        Space;
        transform_argish left_b values right_b;
      ]
    | XHPClassAttributeDeclaration x ->
      let (kw, xhp_attributes, semi) =
        get_xhp_class_attribute_declaration_children x in
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
    | XHPClassAttribute x ->
      (* TODO: figure out nesting here *)
      let (attr_type, name, init, req) = get_xhp_class_attribute_children x in
      Concat [
        t attr_type;
        Space;
        t name;
        when_present init space;
        t init;
        when_present req space;
        t req;
      ]
    | XHPAttribute x ->
      let (name, eq, expr) = get_xhp_attribute_children x in
      Span [
        t name;
        t eq;
        SplitWith Cost.Base;
        Nest [t expr];
      ]
    | XHPOpen x ->
      let (left_a, name, attrs, right_a) = get_xhp_open_children x in
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
    | XHPExpression x ->
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
          let prev_token_scanned_trailing_trivia = ref false in
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
                then Nothing
                else begin
                  match syntax node with
                  | Token _ -> if has_invisibles leading then Split else Nothing
                  | _ -> Split
                end;
              if !prev_token_scanned_trailing_trivia
                then transform_leading_trivia leading
                else transform_xhp_leading_trivia leading;
              t node;
            ] in
            (* XHPExpressions currently have trailing trivia when in an XHPBody,
             * but they shouldn't--see T16787398. Once that issue is resolved,
             * prev_token_scanned_trailing_trivia and prev_token_was_xhpbody
             * will be equivalent and one can be removed. *)
            let open EditableToken in
            prev_token_scanned_trailing_trivia := begin
              match syntax node with
              | XHPExpression _ -> true
              | Token t -> kind t = TokenKind.XHPBody
              | _ -> false
            end;
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

      let (xhp_open, body, close) = get_xhp_expression_children x in
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
    | VarrayTypeSpecifier x ->
      let (kw, left_a, varray_type, trailing_comma, right_a) =
        get_varray_type_specifier_children x in
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a varray_type trailing_comma right_a;
      ]
    | VectorArrayTypeSpecifier x ->
      let (kw, left_a, vec_type, right_a) =
        get_vector_array_type_specifier_children x in
      Concat [
        t kw;
        transform_braced_item left_a vec_type right_a;
      ]
    | VectorTypeSpecifier x ->
      let (kw, left_a, vec_type, trailing_comma, right_a) =
        get_vector_type_specifier_children x in
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a vec_type trailing_comma right_a;
      ]
    | KeysetTypeSpecifier x ->
      let (kw, left_a, ks_type, trailing_comma, right_a) =
        get_keyset_type_specifier_children x in
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a ks_type trailing_comma right_a;
      ]
    | TypeParameter x ->
      let (variance, name, constraints) = get_type_parameter_children x in
      Concat [
        t variance;
        t name;
        when_present constraints space;
        handle_possible_list constraints;
      ]
    | TypeConstraint x ->
      let (kw, constraint_type) = get_type_constraint_children x in
      Concat [
        t kw;
        Space;
        t constraint_type;
      ]
    | DarrayTypeSpecifier x ->
      let (kw, left_a, key, comma_kw, value, trailing_comma, right_a) =
        get_darray_type_specifier_children x in
      let key_list_item = make_list_item key comma_kw in
      let val_list_item = make_list_item value trailing_comma in
      let args = make_list [key_list_item; val_list_item] in
      Concat [
        t kw;
        transform_argish ~allow_trailing:true left_a args right_a;
      ]
    | MapArrayTypeSpecifier x ->
      let (kw, left_a, key, comma_kw, value, right_a) =
        get_map_array_type_specifier_children x in
      Concat [
        t kw;
        let key_list_item = make_list_item key comma_kw in
        let val_list_item = make_list_item value (make_missing ()) in
        let args = make_list [key_list_item; val_list_item] in
        transform_argish ~allow_trailing:false left_a args right_a;
      ]
    | DictionaryTypeSpecifier x ->
      let (kw, left_a, members, right_a) =
        get_dictionary_type_specifier_children x
      in
      Concat [
        t kw;
        transform_argish left_a members right_a;
      ]
    | ClosureTypeSpecifier x ->
      let (
        outer_left_p,
        coroutine,
        kw,
        inner_left_p,
        param_types,
        inner_right_p,
        colon,
        ret_type,
        outer_right_p
      ) = get_closure_type_specifier_children x in
      Concat [
        t outer_left_p;
        t coroutine;
        when_present coroutine space;
        t kw;
        transform_argish_with_return_type
          inner_left_p param_types inner_right_p colon ret_type;
        t outer_right_p;
      ]
    | ClassnameTypeSpecifier x ->
      let (kw, left_a, class_type, trailing_comma, right_a) =
        get_classname_type_specifier_children x in
      Concat [
        t kw;
        transform_braced_item_with_trailer
          left_a class_type trailing_comma right_a;
      ]
    | FieldSpecifier x ->
      let (question, name, arrow_kw, field_type) =
        get_field_specifier_children x in
      Concat [
        t question;
        transform_mapish_entry name arrow_kw field_type;
      ]
    | FieldInitializer x ->
      let (name, arrow_kw, value) = get_field_initializer_children x in
      transform_mapish_entry name arrow_kw value
    | ShapeTypeSpecifier x ->
      let (shape_kw, left_p, type_fields, ellipsis, right_p) =
        get_shape_type_specifier_children x in
      let fields = if is_missing ellipsis
        then type_fields
        else
          let missing_separator = make_missing () in
          let ellipsis_list = [make_list_item ellipsis missing_separator] in
          make_list (children type_fields @ ellipsis_list) in
      transform_container_literal
        ~allow_trailing:(is_missing ellipsis) shape_kw left_p fields right_p
    | ShapeExpression x ->
      let (shape_kw, left_p, fields, right_p) =
        get_shape_expression_children x
      in
      transform_container_literal shape_kw left_p fields right_p
    | TupleExpression x ->
      let (kw, left_p, items, right_p) = get_tuple_expression_children x in
      Concat [
        t kw;
        transform_argish left_p items right_p;
      ]
    | TypeArguments x ->
      let (left_a, type_list, right_a) = get_type_arguments_children x in
      transform_argish left_a type_list right_a
    | TypeParameters x ->
      let (left_a, param_list, right_a) = get_type_parameters_children x in
      transform_argish left_a param_list right_a
    | TupleTypeSpecifier x ->
      let (left_p, types, right_p) = get_tuple_type_specifier_children x in
      transform_argish left_p types right_p
    | TupleTypeExplicitSpecifier x ->
      let (kw, left_a, types, right_a) =
        get_tuple_type_explicit_specifier_children x in
      Concat [
        t kw;
        transform_argish left_a types right_a
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

  and braced_block_nest open_b close_b nodes =
    (* Remove the closing brace's leading trivia and handle it inside the
     * BlockNest, so that comments will be indented correctly. *)
    let leading, close_b = remove_leading_trivia close_b in
    Concat [
      t open_b;
      Newline;
      BlockNest [
        Concat nodes;
        transform_leading_trivia leading;
        Newline;
      ];
      t close_b;
    ]

  and delimited_nest
      ?(spaces=false)
      ?(split_when_children_split=true)
      left_delim
      right_delim
      nodes
    =
    let rule =
      if split_when_children_split
      then Rule.Parental
      else Rule.Simple Cost.Base
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

  and after_each_argument ?(force_newlines=false) is_last =
    if force_newlines
    then Newline
    else
      if is_last
      then Split
      else space_split ()

  and handle_lambda_body node =
    match syntax node with
    | CompoundStatement x ->
      handle_compound_statement x;
    | _ ->
      Concat [
        Space;
        SplitWith Cost.Base;
        Nest [t node];
      ]

  and handle_possible_compound_statement ?space:(space=true) node =
    match syntax node with
    | CompoundStatement x ->
      Concat [
        handle_compound_statement x;
        if space then Space else Nothing;
      ]
    | _ ->
      Concat [
        Newline;
        BlockNest [
          t node
        ];
      ]

  and handle_compound_statement cs =
    let (left_b, statements, right_b) = get_compound_statement_children cs in
    Concat [
      Space;
      braced_block_nest left_b right_b [
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

  and handle_function_call_expression fce =
    let (receiver, lp, args, rp) = get_function_call_expression_children fce in
    match syntax receiver with
    | MemberSelectionExpression mse ->
      handle_possible_chaining
        (get_member_selection_expression_children mse)
        (Some (lp, args, rp))
    | SafeMemberSelectionExpression smse ->
      handle_possible_chaining
        (get_safe_member_selection_expression_children smse)
        (Some (lp, args, rp))
    | _ ->
      Concat [
        t receiver;
        transform_argish lp args rp
      ]

  and handle_function_call_with_type_arguments_expression afce =
    let (receiever, tyargs, lp, args, rp) =
      get_function_call_with_type_arguments_expression_children afce
    in
    match syntax receiever with
    | MemberSelectionExpression mse ->
      handle_possible_chaining
        (get_member_selection_expression_children mse)
        (Some (lp, args, rp))
    | SafeMemberSelectionExpression smse ->
      handle_possible_chaining
        (get_safe_member_selection_expression_children smse)
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
      | FunctionCallExpression x ->
        let (receiver, lp, args, rp) =
          get_function_call_expression_children x in
        (match syntax receiver with
          | MemberSelectionExpression mse ->
            handle_mse_or_smse
              (get_member_selection_expression_children mse)
              (Some (lp, args, rp))
          | SafeMemberSelectionExpression smse ->
            handle_mse_or_smse
              (get_safe_member_selection_expression_children smse)
              (Some (lp, args, rp))
          | _ -> obj, []
        )
      | MemberSelectionExpression mse ->
        handle_mse_or_smse
          (get_member_selection_expression_children mse) None
      | SafeMemberSelectionExpression smse ->
        handle_mse_or_smse
          (get_safe_member_selection_expression_children smse) None
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
        SplitWith Cost.SimpleMemberSelection;
        Nest [transform_chain hd];
      ]
    | hd :: tl ->
      WithLazyRule (Rule.Parental,
        Concat [
          t obj;
          Split;
        ],
        Nest [
          transform_chain hd;
          Concat (List.map tl ~f:(fun x -> Concat [Split; transform_chain x]));
        ])
    | _ -> failwith "Expected a chain of at least length 1"

  and transform_fn_decl_name async coroutine kw amp name type_params leftp =
    [
      t async;
      when_present async space;
      t coroutine;
      when_present coroutine space;
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
          | ListItem x -> fst (get_list_item_children x)
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
    delimited_nest ~spaces ~split_when_children_split left_p right_p [
      transform_arg_list ~allow_trailing ~force_newlines arg_list
    ]

  and transform_braced_item left_p item right_p =
    delimited_nest left_p right_p [t item]

  and transform_trailing_comma ~allow_trailing item comma =
    let open EditableToken in
    (* PHP does not permit trailing commas in function calls. Rather than try to
     * account for where PHP's parser permits trailing commas, we just never add
     * them in PHP files. *)
    let allow_trailing = allow_trailing && (Env.add_trailing_commas env) in
    let item, item_trailing = remove_trailing_trivia item in
    match syntax comma with
    | Token tok ->
      Concat [
        Concat [
          t item;
          if allow_trailing then TrailingComma else Nothing;
          transform_trailing_trivia item_trailing;
        ];
        Concat [
          transform_leading_trivia (leading tok);
          Ignore (text tok, width tok);
          transform_trailing_trivia (trailing tok);
        ]
      ]
    | Missing ->
      Concat [
        t item;
        if allow_trailing then TrailingComma else Nothing;
        transform_trailing_trivia item_trailing;
      ]
    | _ -> failwith "Expected Token"

  and transform_braced_item_with_trailer left_p item comma right_p =
    delimited_nest left_p right_p
      (* TODO: turn allow_trailing:true when HHVM versions that don't support
         trailing commas in all these places reach end-of-life. *)
      [transform_trailing_comma ~allow_trailing:false item comma]

  and transform_arg_list ?(allow_trailing=true) ?(force_newlines=false) items =
    handle_possible_list items
      ~after_each:(after_each_argument ~force_newlines)
      ~handle_last:(transform_last_arg ~allow_trailing)

  and transform_possible_comma_list ?(allow_trailing=true) ?(spaces=false)
      items right_p =
    nest ~spaces right_p [
      transform_arg_list ~allow_trailing items
    ]

  and transform_container_literal
      ?(spaces=false) ?allow_trailing kw left_p members right_p =
    let force_newlines =
      let trivia = trailing_trivia left_p in
      List.exists trivia ~f:(fun x -> Trivia.kind x = TriviaKind.EndOfLine)
    in
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
    | ListItem x ->
      let (item, separator) = get_list_item_children x in
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

  and transform_binary_expression ~is_nested expr =
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

    let (left, operator, right) = get_binary_expression_children expr in
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
        SplitWith Cost.Base;
        Nest [t right];
      ]
    else
      Concat [
        let precedence = Full_fidelity_operator.precedence operator_t in

        let rec flatten_expression expr =
          match syntax expr with
          | BinaryExpression x ->
            let (left, operator, right) = get_binary_expression_children x in
            let operator_t = get_operator_type operator in
            let op_precedence = Full_fidelity_operator.precedence operator_t in
            if (op_precedence = precedence) then
              (flatten_expression left) @ (operator :: flatten_expression right)
            else [expr]
          | _ -> [expr]
        in

        let transform_operand operand =
          match syntax operand with
          | BinaryExpression x -> transform_binary_expression ~is_nested:true x
          | _ -> t operand
        in

        let binary_expression_syntax_list =
          flatten_expression (make_binary_expression left operator right) in
        match binary_expression_syntax_list with
        | hd :: tl ->
          WithLazyRule (Rule.Parental,
            transform_operand hd,
            let expression =
              let last_op = ref (List.hd_exn tl) in
              List.mapi tl ~f:(fun i x ->
                if i mod 2 = 0 then begin
                  let op = x in
                  last_op := op;
                  let op_has_spaces = operator_has_surrounding_spaces op in
                  let op_is_leading = operator_is_leading op in
                  Concat [
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
                  let op_has_spaces =
                    operator_has_surrounding_spaces !last_op
                  in
                  let op_is_leading = operator_is_leading !last_op in
                  Concat [
                    if op_is_leading then begin
                      (* TODO: We only have this split to ensure that range
                       * formatting works when it starts or ends here. We should
                       * remove it once we can return an expanded formatting
                       * range. *)
                      if op_has_spaces
                        then Concat [Space; SplitWith Cost.Base]
                        else SplitWith Cost.Base
                    end
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

  (* True if the trivia list contains any "invisible" trivia, meaning spaces,
   * tabs, or newlines. *)
  and has_invisibles trivia_list =
    List.exists trivia_list ~f:begin fun trivia ->
      Trivia.kind trivia = TriviaKind.WhiteSpace ||
      Trivia.kind trivia = TriviaKind.EndOfLine
    end

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
          if !last_comment_was_delimited then begin
            if !whitespace_followed_last_comment then Space
            else if !newline_followed_last_comment then Newline
            else Nothing
          end
          else if Option.is_some !last_comment
            then Newline (* Always add a newline after a single-line comment *)
            else Nothing;
        ])
        :: !comments;
      last_comment := None;
      leading_invisibles := [];
      trailing_invisibles := [];
    in
    List.iter trivia ~f:(fun triv ->
      match Trivia.kind triv with
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
            Newline;
            Ignore ("\n", 1);
            Ignore ((String.make start_index ' '), start_index);
            Comment ((Trivia.text dc), (Trivia.width dc));
          ]
        in

        let hd = List.hd_exn delimited_lines in
        let tl = List.tl_exn delimited_lines in
        let hd = Comment (hd, (String.length hd)) in

        last_comment := Some (Concat [
          if !currently_leading then Newline
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
          Comment ((Trivia.text triv), (Trivia.width triv));
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
  in

  t node
