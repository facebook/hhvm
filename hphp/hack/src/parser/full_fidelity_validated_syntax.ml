(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 * This module contains the functions to (in)validate syntax trees.
 *)

open Full_fidelity_syntax_type (* module signatures of the functor *)

module SyntaxKind = Full_fidelity_syntax_kind
module Def = Schema_definition

module Make (Token : TokenType) (SyntaxValue : SyntaxValueType) = struct
  module SyntaxBase = Full_fidelity_syntax.WithToken (Token)
  module Syntax = SyntaxBase.WithSyntaxValue (SyntaxValue)
  module Validated = MakeValidated (Token) (SyntaxValue)
  open Validated

  type 'a validator = Syntax.t -> 'a value

  type 'a invalidator = 'a value -> Syntax.t

  exception Validation_failure of SyntaxKind.t option * Syntax.t

  let validation_fail k t = raise (Validation_failure (k, t))

  exception Aggregation_failure of Def.aggregate_type * Syntax.syntax

  let aggregation_fail a s =
    Printf.eprintf
      "Aggregation failure: For %s not expecting %s\n"
      (Schema_definition.string_of_aggregate_type a)
      (SyntaxKind.to_string @@ Syntax.to_kind s);
    raise (Aggregation_failure (a, s))

  let validate_option_with : 'a. 'a validator -> 'a option validator =
   fun validate node ->
    match Syntax.syntax node with
    | Syntax.Missing -> (Syntax.value node, None)
    | _ ->
      let (value, result) = validate node in
      (value, Some result)

  let invalidate_option_with : 'a. 'a invalidator -> 'a option invalidator =
   fun invalidate (value, thing) ->
    match thing with
    | Some real_thing -> invalidate (value, real_thing)
    | None -> { Syntax.syntax = Syntax.Missing; value }

  let validate_token : Token.t validator =
   fun node ->
    match Syntax.syntax node with
    | Syntax.Token t -> (Syntax.value node, t)
    | _ -> validation_fail None node

  let invalidate_token : Token.t invalidator =
   (fun (value, token) -> { Syntax.syntax = Syntax.Token token; value })

  let validate_list_with : 'a. 'a validator -> 'a listesque validator =
   fun validate node ->
    let validate_item i =
      match Syntax.syntax i with
      | Syntax.ListItem { list_item; list_separator } ->
        let item = validate list_item in
        let separator = validate_option_with validate_token list_separator in
        (i.Syntax.value, (item, separator))
      | _ -> validation_fail (Some SyntaxKind.ListItem) i
    in
    let validate_list l =
      try Syntactic (List.map validate_item l)
      with Validation_failure (Some SyntaxKind.ListItem, _) ->
        NonSyntactic (List.map validate l)
    in
    let result =
      match Syntax.syntax node with
      | Syntax.SyntaxList l -> validate_list l
      | Syntax.Missing -> MissingList
      | _ -> SingletonList (validate node)
    in
    (node.Syntax.value, result)

  let invalidate_list_with : 'a. 'a invalidator -> 'a listesque invalidator =
   fun invalidate (value, listesque) ->
    match listesque with
    | SingletonList node -> invalidate node
    | MissingList -> { Syntax.syntax = Syntax.Missing; value }
    | NonSyntactic nodes ->
      { Syntax.syntax = Syntax.SyntaxList (List.map invalidate nodes); value }
    | Syntactic nodes ->
      let mapper (value, (node, separator)) =
        let inode = invalidate node in
        let iseparator = invalidate_option_with invalidate_token separator in
        {
          Syntax.syntax =
            Syntax.ListItem { list_item = inode; list_separator = iseparator };
          value;
        }
      in
      { Syntax.syntax = Syntax.SyntaxList (List.map mapper nodes); value }

  let rec tag : 'a 'b. 'a validator -> ('a -> 'b) -> 'b validator =
   (* Validating aggregate types means picking the right validator for the
    * expected/valid variants and then tagging the result with the constructor
    * corresponding to the variant. This is a repetative pattern. Explicit
    * polymorphism saves us this trouble.
    *)
   fun validator projection node ->
    let (value, node) = validator node in
    (value, projection node)

  and validate_top_level_declaration : top_level_declaration validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.EndOfFile _ -> tag validate_end_of_file (fun x -> TLDEndOfFile x) x
    | Syntax.FileAttributeSpecification _ ->
      tag
        validate_file_attribute_specification
        (fun x -> TLDFileAttributeSpecification x)
        x
    | Syntax.EnumDeclaration _ ->
      tag validate_enum_declaration (fun x -> TLDEnum x) x
    | Syntax.EnumClassDeclaration _ ->
      tag validate_enum_class_declaration (fun x -> TLDEnumClass x) x
    | Syntax.RecordDeclaration _ ->
      tag validate_record_declaration (fun x -> TLDRecord x) x
    | Syntax.AliasDeclaration _ ->
      tag validate_alias_declaration (fun x -> TLDAlias x) x
    | Syntax.ContextAliasDeclaration _ ->
      tag validate_context_alias_declaration (fun x -> TLDContextAlias x) x
    | Syntax.NamespaceDeclaration _ ->
      tag validate_namespace_declaration (fun x -> TLDNamespace x) x
    | Syntax.NamespaceDeclarationHeader _ ->
      tag
        validate_namespace_declaration_header
        (fun x -> TLDNamespaceDeclarationHeader x)
        x
    | Syntax.NamespaceUseDeclaration _ ->
      tag validate_namespace_use_declaration (fun x -> TLDNamespaceUse x) x
    | Syntax.NamespaceGroupUseDeclaration _ ->
      tag
        validate_namespace_group_use_declaration
        (fun x -> TLDNamespaceGroupUse x)
        x
    | Syntax.FunctionDeclaration _ ->
      tag validate_function_declaration (fun x -> TLDFunction x) x
    | Syntax.ClassishDeclaration _ ->
      tag validate_classish_declaration (fun x -> TLDClassish x) x
    | Syntax.ConstDeclaration _ ->
      tag validate_const_declaration (fun x -> TLDConst x) x
    | Syntax.InclusionDirective _ ->
      tag validate_inclusion_directive (fun x -> TLDInclusionDirective x) x
    | Syntax.CompoundStatement _ ->
      tag validate_compound_statement (fun x -> TLDCompound x) x
    | Syntax.ExpressionStatement _ ->
      tag validate_expression_statement (fun x -> TLDExpression x) x
    | Syntax.MarkupSection _ ->
      tag validate_markup_section (fun x -> TLDMarkupSection x) x
    | Syntax.MarkupSuffix _ ->
      tag validate_markup_suffix (fun x -> TLDMarkupSuffix x) x
    | Syntax.UnsetStatement _ ->
      tag validate_unset_statement (fun x -> TLDUnset x) x
    | Syntax.UsingStatementBlockScoped _ ->
      tag
        validate_using_statement_block_scoped
        (fun x -> TLDUsingStatementBlockScoped x)
        x
    | Syntax.UsingStatementFunctionScoped _ ->
      tag
        validate_using_statement_function_scoped
        (fun x -> TLDUsingStatementFunctionScoped x)
        x
    | Syntax.WhileStatement _ ->
      tag validate_while_statement (fun x -> TLDWhile x) x
    | Syntax.IfStatement _ -> tag validate_if_statement (fun x -> TLDIf x) x
    | Syntax.TryStatement _ -> tag validate_try_statement (fun x -> TLDTry x) x
    | Syntax.DoStatement _ -> tag validate_do_statement (fun x -> TLDDo x) x
    | Syntax.ForStatement _ -> tag validate_for_statement (fun x -> TLDFor x) x
    | Syntax.ForeachStatement _ ->
      tag validate_foreach_statement (fun x -> TLDForeach x) x
    | Syntax.SwitchFallthrough _ ->
      tag validate_switch_fallthrough (fun x -> TLDSwitchFallthrough x) x
    | Syntax.ReturnStatement _ ->
      tag validate_return_statement (fun x -> TLDReturn x) x
    | Syntax.YieldBreakStatement _ ->
      tag validate_yield_break_statement (fun x -> TLDYieldBreak x) x
    | Syntax.ThrowStatement _ ->
      tag validate_throw_statement (fun x -> TLDThrow x) x
    | Syntax.BreakStatement _ ->
      tag validate_break_statement (fun x -> TLDBreak x) x
    | Syntax.ContinueStatement _ ->
      tag validate_continue_statement (fun x -> TLDContinue x) x
    | Syntax.EchoStatement _ ->
      tag validate_echo_statement (fun x -> TLDEcho x) x
    | s -> aggregation_fail Def.TopLevelDeclaration s

  and invalidate_top_level_declaration : top_level_declaration invalidator =
   fun (value, thing) ->
    match thing with
    | TLDEndOfFile thing -> invalidate_end_of_file (value, thing)
    | TLDFileAttributeSpecification thing ->
      invalidate_file_attribute_specification (value, thing)
    | TLDEnum thing -> invalidate_enum_declaration (value, thing)
    | TLDEnumClass thing -> invalidate_enum_class_declaration (value, thing)
    | TLDRecord thing -> invalidate_record_declaration (value, thing)
    | TLDAlias thing -> invalidate_alias_declaration (value, thing)
    | TLDContextAlias thing ->
      invalidate_context_alias_declaration (value, thing)
    | TLDNamespace thing -> invalidate_namespace_declaration (value, thing)
    | TLDNamespaceDeclarationHeader thing ->
      invalidate_namespace_declaration_header (value, thing)
    | TLDNamespaceUse thing ->
      invalidate_namespace_use_declaration (value, thing)
    | TLDNamespaceGroupUse thing ->
      invalidate_namespace_group_use_declaration (value, thing)
    | TLDFunction thing -> invalidate_function_declaration (value, thing)
    | TLDClassish thing -> invalidate_classish_declaration (value, thing)
    | TLDConst thing -> invalidate_const_declaration (value, thing)
    | TLDInclusionDirective thing ->
      invalidate_inclusion_directive (value, thing)
    | TLDCompound thing -> invalidate_compound_statement (value, thing)
    | TLDExpression thing -> invalidate_expression_statement (value, thing)
    | TLDMarkupSection thing -> invalidate_markup_section (value, thing)
    | TLDMarkupSuffix thing -> invalidate_markup_suffix (value, thing)
    | TLDUnset thing -> invalidate_unset_statement (value, thing)
    | TLDUsingStatementBlockScoped thing ->
      invalidate_using_statement_block_scoped (value, thing)
    | TLDUsingStatementFunctionScoped thing ->
      invalidate_using_statement_function_scoped (value, thing)
    | TLDWhile thing -> invalidate_while_statement (value, thing)
    | TLDIf thing -> invalidate_if_statement (value, thing)
    | TLDTry thing -> invalidate_try_statement (value, thing)
    | TLDDo thing -> invalidate_do_statement (value, thing)
    | TLDFor thing -> invalidate_for_statement (value, thing)
    | TLDForeach thing -> invalidate_foreach_statement (value, thing)
    | TLDSwitchFallthrough thing -> invalidate_switch_fallthrough (value, thing)
    | TLDReturn thing -> invalidate_return_statement (value, thing)
    | TLDYieldBreak thing -> invalidate_yield_break_statement (value, thing)
    | TLDThrow thing -> invalidate_throw_statement (value, thing)
    | TLDBreak thing -> invalidate_break_statement (value, thing)
    | TLDContinue thing -> invalidate_continue_statement (value, thing)
    | TLDEcho thing -> invalidate_echo_statement (value, thing)

  and validate_expression : expression validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ ->
      tag validate_literal_expression (fun x -> ExprLiteral x) x
    | Syntax.PrefixedStringExpression _ ->
      tag validate_prefixed_string_expression (fun x -> ExprPrefixedString x) x
    | Syntax.PrefixedCodeExpression _ ->
      tag validate_prefixed_code_expression (fun x -> ExprPrefixedCode x) x
    | Syntax.VariableExpression _ ->
      tag validate_variable_expression (fun x -> ExprVariable x) x
    | Syntax.PipeVariableExpression _ ->
      tag validate_pipe_variable_expression (fun x -> ExprPipeVariable x) x
    | Syntax.DecoratedExpression _ ->
      tag validate_decorated_expression (fun x -> ExprDecorated x) x
    | Syntax.InclusionExpression _ ->
      tag validate_inclusion_expression (fun x -> ExprInclusion x) x
    | Syntax.AnonymousFunction _ ->
      tag validate_anonymous_function (fun x -> ExprAnonymousFunction x) x
    | Syntax.LambdaExpression _ ->
      tag validate_lambda_expression (fun x -> ExprLambda x) x
    | Syntax.CastExpression _ ->
      tag validate_cast_expression (fun x -> ExprCast x) x
    | Syntax.ScopeResolutionExpression _ ->
      tag
        validate_scope_resolution_expression
        (fun x -> ExprScopeResolution x)
        x
    | Syntax.MemberSelectionExpression _ ->
      tag
        validate_member_selection_expression
        (fun x -> ExprMemberSelection x)
        x
    | Syntax.SafeMemberSelectionExpression _ ->
      tag
        validate_safe_member_selection_expression
        (fun x -> ExprSafeMemberSelection x)
        x
    | Syntax.EmbeddedMemberSelectionExpression _ ->
      tag
        validate_embedded_member_selection_expression
        (fun x -> ExprEmbeddedMemberSelection x)
        x
    | Syntax.YieldExpression _ ->
      tag validate_yield_expression (fun x -> ExprYield x) x
    | Syntax.PrefixUnaryExpression _ ->
      tag validate_prefix_unary_expression (fun x -> ExprPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ ->
      tag validate_postfix_unary_expression (fun x -> ExprPostfixUnary x) x
    | Syntax.BinaryExpression _ ->
      tag validate_binary_expression (fun x -> ExprBinary x) x
    | Syntax.IsExpression _ -> tag validate_is_expression (fun x -> ExprIs x) x
    | Syntax.AsExpression _ -> tag validate_as_expression (fun x -> ExprAs x) x
    | Syntax.NullableAsExpression _ ->
      tag validate_nullable_as_expression (fun x -> ExprNullableAs x) x
    | Syntax.ConditionalExpression _ ->
      tag validate_conditional_expression (fun x -> ExprConditional x) x
    | Syntax.EvalExpression _ ->
      tag validate_eval_expression (fun x -> ExprEval x) x
    | Syntax.IssetExpression _ ->
      tag validate_isset_expression (fun x -> ExprIsset x) x
    | Syntax.FunctionCallExpression _ ->
      tag validate_function_call_expression (fun x -> ExprFunctionCall x) x
    | Syntax.FunctionPointerExpression _ ->
      tag
        validate_function_pointer_expression
        (fun x -> ExprFunctionPointer x)
        x
    | Syntax.ParenthesizedExpression _ ->
      tag validate_parenthesized_expression (fun x -> ExprParenthesized x) x
    | Syntax.BracedExpression _ ->
      tag validate_braced_expression (fun x -> ExprBraced x) x
    | Syntax.ETSpliceExpression _ ->
      tag validate_et_splice_expression (fun x -> ExprETSplice x) x
    | Syntax.EmbeddedBracedExpression _ ->
      tag validate_embedded_braced_expression (fun x -> ExprEmbeddedBraced x) x
    | Syntax.ListExpression _ ->
      tag validate_list_expression (fun x -> ExprList x) x
    | Syntax.CollectionLiteralExpression _ ->
      tag
        validate_collection_literal_expression
        (fun x -> ExprCollectionLiteral x)
        x
    | Syntax.ObjectCreationExpression _ ->
      tag validate_object_creation_expression (fun x -> ExprObjectCreation x) x
    | Syntax.RecordCreationExpression _ ->
      tag validate_record_creation_expression (fun x -> ExprRecordCreation x) x
    | Syntax.DarrayIntrinsicExpression _ ->
      tag
        validate_darray_intrinsic_expression
        (fun x -> ExprDarrayIntrinsic x)
        x
    | Syntax.DictionaryIntrinsicExpression _ ->
      tag
        validate_dictionary_intrinsic_expression
        (fun x -> ExprDictionaryIntrinsic x)
        x
    | Syntax.KeysetIntrinsicExpression _ ->
      tag
        validate_keyset_intrinsic_expression
        (fun x -> ExprKeysetIntrinsic x)
        x
    | Syntax.VarrayIntrinsicExpression _ ->
      tag
        validate_varray_intrinsic_expression
        (fun x -> ExprVarrayIntrinsic x)
        x
    | Syntax.VectorIntrinsicExpression _ ->
      tag
        validate_vector_intrinsic_expression
        (fun x -> ExprVectorIntrinsic x)
        x
    | Syntax.SubscriptExpression _ ->
      tag validate_subscript_expression (fun x -> ExprSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ ->
      tag
        validate_embedded_subscript_expression
        (fun x -> ExprEmbeddedSubscript x)
        x
    | Syntax.AwaitableCreationExpression _ ->
      tag
        validate_awaitable_creation_expression
        (fun x -> ExprAwaitableCreation x)
        x
    | Syntax.XHPChildrenParenthesizedList _ ->
      tag
        validate_xhp_children_parenthesized_list
        (fun x -> ExprXHPChildrenParenthesizedList x)
        x
    | Syntax.XHPExpression _ ->
      tag validate_xhp_expression (fun x -> ExprXHP x) x
    | Syntax.ShapeExpression _ ->
      tag validate_shape_expression (fun x -> ExprShape x) x
    | Syntax.TupleExpression _ ->
      tag validate_tuple_expression (fun x -> ExprTuple x) x
    | Syntax.EnumClassLabelExpression _ ->
      tag validate_enum_class_label_expression (fun x -> ExprEnumClassLabel x) x
    | s -> aggregation_fail Def.Expression s

  and invalidate_expression : expression invalidator =
   fun (value, thing) ->
    match thing with
    | ExprLiteral thing -> invalidate_literal_expression (value, thing)
    | ExprPrefixedString thing ->
      invalidate_prefixed_string_expression (value, thing)
    | ExprPrefixedCode thing ->
      invalidate_prefixed_code_expression (value, thing)
    | ExprVariable thing -> invalidate_variable_expression (value, thing)
    | ExprPipeVariable thing ->
      invalidate_pipe_variable_expression (value, thing)
    | ExprDecorated thing -> invalidate_decorated_expression (value, thing)
    | ExprInclusion thing -> invalidate_inclusion_expression (value, thing)
    | ExprAnonymousFunction thing -> invalidate_anonymous_function (value, thing)
    | ExprLambda thing -> invalidate_lambda_expression (value, thing)
    | ExprCast thing -> invalidate_cast_expression (value, thing)
    | ExprScopeResolution thing ->
      invalidate_scope_resolution_expression (value, thing)
    | ExprMemberSelection thing ->
      invalidate_member_selection_expression (value, thing)
    | ExprSafeMemberSelection thing ->
      invalidate_safe_member_selection_expression (value, thing)
    | ExprEmbeddedMemberSelection thing ->
      invalidate_embedded_member_selection_expression (value, thing)
    | ExprYield thing -> invalidate_yield_expression (value, thing)
    | ExprPrefixUnary thing -> invalidate_prefix_unary_expression (value, thing)
    | ExprPostfixUnary thing ->
      invalidate_postfix_unary_expression (value, thing)
    | ExprBinary thing -> invalidate_binary_expression (value, thing)
    | ExprIs thing -> invalidate_is_expression (value, thing)
    | ExprAs thing -> invalidate_as_expression (value, thing)
    | ExprNullableAs thing -> invalidate_nullable_as_expression (value, thing)
    | ExprConditional thing -> invalidate_conditional_expression (value, thing)
    | ExprEval thing -> invalidate_eval_expression (value, thing)
    | ExprIsset thing -> invalidate_isset_expression (value, thing)
    | ExprFunctionCall thing ->
      invalidate_function_call_expression (value, thing)
    | ExprFunctionPointer thing ->
      invalidate_function_pointer_expression (value, thing)
    | ExprParenthesized thing ->
      invalidate_parenthesized_expression (value, thing)
    | ExprBraced thing -> invalidate_braced_expression (value, thing)
    | ExprETSplice thing -> invalidate_et_splice_expression (value, thing)
    | ExprEmbeddedBraced thing ->
      invalidate_embedded_braced_expression (value, thing)
    | ExprList thing -> invalidate_list_expression (value, thing)
    | ExprCollectionLiteral thing ->
      invalidate_collection_literal_expression (value, thing)
    | ExprObjectCreation thing ->
      invalidate_object_creation_expression (value, thing)
    | ExprRecordCreation thing ->
      invalidate_record_creation_expression (value, thing)
    | ExprDarrayIntrinsic thing ->
      invalidate_darray_intrinsic_expression (value, thing)
    | ExprDictionaryIntrinsic thing ->
      invalidate_dictionary_intrinsic_expression (value, thing)
    | ExprKeysetIntrinsic thing ->
      invalidate_keyset_intrinsic_expression (value, thing)
    | ExprVarrayIntrinsic thing ->
      invalidate_varray_intrinsic_expression (value, thing)
    | ExprVectorIntrinsic thing ->
      invalidate_vector_intrinsic_expression (value, thing)
    | ExprSubscript thing -> invalidate_subscript_expression (value, thing)
    | ExprEmbeddedSubscript thing ->
      invalidate_embedded_subscript_expression (value, thing)
    | ExprAwaitableCreation thing ->
      invalidate_awaitable_creation_expression (value, thing)
    | ExprXHPChildrenParenthesizedList thing ->
      invalidate_xhp_children_parenthesized_list (value, thing)
    | ExprXHP thing -> invalidate_xhp_expression (value, thing)
    | ExprShape thing -> invalidate_shape_expression (value, thing)
    | ExprTuple thing -> invalidate_tuple_expression (value, thing)
    | ExprEnumClassLabel thing ->
      invalidate_enum_class_label_expression (value, thing)

  and validate_specifier : specifier validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.SimpleTypeSpecifier _ ->
      tag validate_simple_type_specifier (fun x -> SpecSimple x) x
    | Syntax.Contexts _ -> tag validate_contexts (fun x -> SpecContexts x) x
    | Syntax.VariadicParameter _ ->
      tag validate_variadic_parameter (fun x -> SpecVariadicParameter x) x
    | Syntax.LambdaSignature _ ->
      tag validate_lambda_signature (fun x -> SpecLambdaSignature x) x
    | Syntax.XHPEnumType _ ->
      tag validate_xhp_enum_type (fun x -> SpecXHPEnumType x) x
    | Syntax.VectorTypeSpecifier _ ->
      tag validate_vector_type_specifier (fun x -> SpecVector x) x
    | Syntax.KeysetTypeSpecifier _ ->
      tag validate_keyset_type_specifier (fun x -> SpecKeyset x) x
    | Syntax.TupleTypeExplicitSpecifier _ ->
      tag
        validate_tuple_type_explicit_specifier
        (fun x -> SpecTupleTypeExplicit x)
        x
    | Syntax.VarrayTypeSpecifier _ ->
      tag validate_varray_type_specifier (fun x -> SpecVarray x) x
    | Syntax.FunctionCtxTypeSpecifier _ ->
      tag validate_function_ctx_type_specifier (fun x -> SpecFunctionCtx x) x
    | Syntax.DarrayTypeSpecifier _ ->
      tag validate_darray_type_specifier (fun x -> SpecDarray x) x
    | Syntax.DictionaryTypeSpecifier _ ->
      tag validate_dictionary_type_specifier (fun x -> SpecDictionary x) x
    | Syntax.ClosureTypeSpecifier _ ->
      tag validate_closure_type_specifier (fun x -> SpecClosure x) x
    | Syntax.ClosureParameterTypeSpecifier _ ->
      tag
        validate_closure_parameter_type_specifier
        (fun x -> SpecClosureParameter x)
        x
    | Syntax.ClassnameTypeSpecifier _ ->
      tag validate_classname_type_specifier (fun x -> SpecClassname x) x
    | Syntax.FieldSpecifier _ ->
      tag validate_field_specifier (fun x -> SpecField x) x
    | Syntax.ShapeTypeSpecifier _ ->
      tag validate_shape_type_specifier (fun x -> SpecShape x) x
    | Syntax.GenericTypeSpecifier _ ->
      tag validate_generic_type_specifier (fun x -> SpecGeneric x) x
    | Syntax.NullableTypeSpecifier _ ->
      tag validate_nullable_type_specifier (fun x -> SpecNullable x) x
    | Syntax.LikeTypeSpecifier _ ->
      tag validate_like_type_specifier (fun x -> SpecLike x) x
    | Syntax.SoftTypeSpecifier _ ->
      tag validate_soft_type_specifier (fun x -> SpecSoft x) x
    | Syntax.TupleTypeSpecifier _ ->
      tag validate_tuple_type_specifier (fun x -> SpecTuple x) x
    | Syntax.UnionTypeSpecifier _ ->
      tag validate_union_type_specifier (fun x -> SpecUnion x) x
    | Syntax.IntersectionTypeSpecifier _ ->
      tag validate_intersection_type_specifier (fun x -> SpecIntersection x) x
    | s -> aggregation_fail Def.Specifier s

  and invalidate_specifier : specifier invalidator =
   fun (value, thing) ->
    match thing with
    | SpecSimple thing -> invalidate_simple_type_specifier (value, thing)
    | SpecContexts thing -> invalidate_contexts (value, thing)
    | SpecVariadicParameter thing -> invalidate_variadic_parameter (value, thing)
    | SpecLambdaSignature thing -> invalidate_lambda_signature (value, thing)
    | SpecXHPEnumType thing -> invalidate_xhp_enum_type (value, thing)
    | SpecVector thing -> invalidate_vector_type_specifier (value, thing)
    | SpecKeyset thing -> invalidate_keyset_type_specifier (value, thing)
    | SpecTupleTypeExplicit thing ->
      invalidate_tuple_type_explicit_specifier (value, thing)
    | SpecVarray thing -> invalidate_varray_type_specifier (value, thing)
    | SpecFunctionCtx thing ->
      invalidate_function_ctx_type_specifier (value, thing)
    | SpecDarray thing -> invalidate_darray_type_specifier (value, thing)
    | SpecDictionary thing -> invalidate_dictionary_type_specifier (value, thing)
    | SpecClosure thing -> invalidate_closure_type_specifier (value, thing)
    | SpecClosureParameter thing ->
      invalidate_closure_parameter_type_specifier (value, thing)
    | SpecClassname thing -> invalidate_classname_type_specifier (value, thing)
    | SpecField thing -> invalidate_field_specifier (value, thing)
    | SpecShape thing -> invalidate_shape_type_specifier (value, thing)
    | SpecGeneric thing -> invalidate_generic_type_specifier (value, thing)
    | SpecNullable thing -> invalidate_nullable_type_specifier (value, thing)
    | SpecLike thing -> invalidate_like_type_specifier (value, thing)
    | SpecSoft thing -> invalidate_soft_type_specifier (value, thing)
    | SpecTuple thing -> invalidate_tuple_type_specifier (value, thing)
    | SpecUnion thing -> invalidate_union_type_specifier (value, thing)
    | SpecIntersection thing ->
      invalidate_intersection_type_specifier (value, thing)

  and validate_parameter : parameter validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.ParameterDeclaration _ ->
      tag
        validate_parameter_declaration
        (fun x -> ParamParameterDeclaration x)
        x
    | Syntax.VariadicParameter _ ->
      tag validate_variadic_parameter (fun x -> ParamVariadicParameter x) x
    | s -> aggregation_fail Def.Parameter s

  and invalidate_parameter : parameter invalidator =
   fun (value, thing) ->
    match thing with
    | ParamParameterDeclaration thing ->
      invalidate_parameter_declaration (value, thing)
    | ParamVariadicParameter thing ->
      invalidate_variadic_parameter (value, thing)

  and validate_class_body_declaration : class_body_declaration validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.PropertyDeclaration _ ->
      tag validate_property_declaration (fun x -> BodyProperty x) x
    | Syntax.MethodishDeclaration _ ->
      tag validate_methodish_declaration (fun x -> BodyMethodish x) x
    | Syntax.MethodishTraitResolution _ ->
      tag
        validate_methodish_trait_resolution
        (fun x -> BodyMethodishTraitResolution x)
        x
    | Syntax.RequireClause _ ->
      tag validate_require_clause (fun x -> BodyRequireClause x) x
    | Syntax.ConstDeclaration _ ->
      tag validate_const_declaration (fun x -> BodyConst x) x
    | Syntax.TypeConstDeclaration _ ->
      tag validate_type_const_declaration (fun x -> BodyTypeConst x) x
    | Syntax.ContextConstDeclaration _ ->
      tag validate_context_const_declaration (fun x -> BodyContextConst x) x
    | Syntax.XHPChildrenDeclaration _ ->
      tag validate_xhp_children_declaration (fun x -> BodyXHPChildren x) x
    | Syntax.XHPCategoryDeclaration _ ->
      tag validate_xhp_category_declaration (fun x -> BodyXHPCategory x) x
    | Syntax.XHPClassAttributeDeclaration _ ->
      tag
        validate_xhp_class_attribute_declaration
        (fun x -> BodyXHPClassAttribute x)
        x
    | s -> aggregation_fail Def.ClassBodyDeclaration s

  and invalidate_class_body_declaration : class_body_declaration invalidator =
   fun (value, thing) ->
    match thing with
    | BodyProperty thing -> invalidate_property_declaration (value, thing)
    | BodyMethodish thing -> invalidate_methodish_declaration (value, thing)
    | BodyMethodishTraitResolution thing ->
      invalidate_methodish_trait_resolution (value, thing)
    | BodyRequireClause thing -> invalidate_require_clause (value, thing)
    | BodyConst thing -> invalidate_const_declaration (value, thing)
    | BodyTypeConst thing -> invalidate_type_const_declaration (value, thing)
    | BodyContextConst thing ->
      invalidate_context_const_declaration (value, thing)
    | BodyXHPChildren thing -> invalidate_xhp_children_declaration (value, thing)
    | BodyXHPCategory thing -> invalidate_xhp_category_declaration (value, thing)
    | BodyXHPClassAttribute thing ->
      invalidate_xhp_class_attribute_declaration (value, thing)

  and validate_statement : statement validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.InclusionDirective _ ->
      tag validate_inclusion_directive (fun x -> StmtInclusionDirective x) x
    | Syntax.CompoundStatement _ ->
      tag validate_compound_statement (fun x -> StmtCompound x) x
    | Syntax.ExpressionStatement _ ->
      tag validate_expression_statement (fun x -> StmtExpression x) x
    | Syntax.MarkupSection _ ->
      tag validate_markup_section (fun x -> StmtMarkupSection x) x
    | Syntax.MarkupSuffix _ ->
      tag validate_markup_suffix (fun x -> StmtMarkupSuffix x) x
    | Syntax.UnsetStatement _ ->
      tag validate_unset_statement (fun x -> StmtUnset x) x
    | Syntax.UsingStatementBlockScoped _ ->
      tag
        validate_using_statement_block_scoped
        (fun x -> StmtUsingStatementBlockScoped x)
        x
    | Syntax.UsingStatementFunctionScoped _ ->
      tag
        validate_using_statement_function_scoped
        (fun x -> StmtUsingStatementFunctionScoped x)
        x
    | Syntax.WhileStatement _ ->
      tag validate_while_statement (fun x -> StmtWhile x) x
    | Syntax.IfStatement _ -> tag validate_if_statement (fun x -> StmtIf x) x
    | Syntax.TryStatement _ -> tag validate_try_statement (fun x -> StmtTry x) x
    | Syntax.DoStatement _ -> tag validate_do_statement (fun x -> StmtDo x) x
    | Syntax.ForStatement _ -> tag validate_for_statement (fun x -> StmtFor x) x
    | Syntax.ForeachStatement _ ->
      tag validate_foreach_statement (fun x -> StmtForeach x) x
    | Syntax.SwitchStatement _ ->
      tag validate_switch_statement (fun x -> StmtSwitch x) x
    | Syntax.SwitchFallthrough _ ->
      tag validate_switch_fallthrough (fun x -> StmtSwitchFallthrough x) x
    | Syntax.ReturnStatement _ ->
      tag validate_return_statement (fun x -> StmtReturn x) x
    | Syntax.YieldBreakStatement _ ->
      tag validate_yield_break_statement (fun x -> StmtYieldBreak x) x
    | Syntax.ThrowStatement _ ->
      tag validate_throw_statement (fun x -> StmtThrow x) x
    | Syntax.BreakStatement _ ->
      tag validate_break_statement (fun x -> StmtBreak x) x
    | Syntax.ContinueStatement _ ->
      tag validate_continue_statement (fun x -> StmtContinue x) x
    | Syntax.EchoStatement _ ->
      tag validate_echo_statement (fun x -> StmtEcho x) x
    | Syntax.ConcurrentStatement _ ->
      tag validate_concurrent_statement (fun x -> StmtConcurrent x) x
    | Syntax.TypeConstant _ ->
      tag validate_type_constant (fun x -> StmtTypeConstant x) x
    | s -> aggregation_fail Def.Statement s

  and invalidate_statement : statement invalidator =
   fun (value, thing) ->
    match thing with
    | StmtInclusionDirective thing ->
      invalidate_inclusion_directive (value, thing)
    | StmtCompound thing -> invalidate_compound_statement (value, thing)
    | StmtExpression thing -> invalidate_expression_statement (value, thing)
    | StmtMarkupSection thing -> invalidate_markup_section (value, thing)
    | StmtMarkupSuffix thing -> invalidate_markup_suffix (value, thing)
    | StmtUnset thing -> invalidate_unset_statement (value, thing)
    | StmtUsingStatementBlockScoped thing ->
      invalidate_using_statement_block_scoped (value, thing)
    | StmtUsingStatementFunctionScoped thing ->
      invalidate_using_statement_function_scoped (value, thing)
    | StmtWhile thing -> invalidate_while_statement (value, thing)
    | StmtIf thing -> invalidate_if_statement (value, thing)
    | StmtTry thing -> invalidate_try_statement (value, thing)
    | StmtDo thing -> invalidate_do_statement (value, thing)
    | StmtFor thing -> invalidate_for_statement (value, thing)
    | StmtForeach thing -> invalidate_foreach_statement (value, thing)
    | StmtSwitch thing -> invalidate_switch_statement (value, thing)
    | StmtSwitchFallthrough thing -> invalidate_switch_fallthrough (value, thing)
    | StmtReturn thing -> invalidate_return_statement (value, thing)
    | StmtYieldBreak thing -> invalidate_yield_break_statement (value, thing)
    | StmtThrow thing -> invalidate_throw_statement (value, thing)
    | StmtBreak thing -> invalidate_break_statement (value, thing)
    | StmtContinue thing -> invalidate_continue_statement (value, thing)
    | StmtEcho thing -> invalidate_echo_statement (value, thing)
    | StmtConcurrent thing -> invalidate_concurrent_statement (value, thing)
    | StmtTypeConstant thing -> invalidate_type_constant (value, thing)

  and validate_switch_label : switch_label validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.CaseLabel _ -> tag validate_case_label (fun x -> SwitchCase x) x
    | Syntax.DefaultLabel _ ->
      tag validate_default_label (fun x -> SwitchDefault x) x
    | s -> aggregation_fail Def.SwitchLabel s

  and invalidate_switch_label : switch_label invalidator =
   fun (value, thing) ->
    match thing with
    | SwitchCase thing -> invalidate_case_label (value, thing)
    | SwitchDefault thing -> invalidate_default_label (value, thing)

  and validate_lambda_body : lambda_body validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ ->
      tag validate_literal_expression (fun x -> LambdaLiteral x) x
    | Syntax.PrefixedStringExpression _ ->
      tag
        validate_prefixed_string_expression
        (fun x -> LambdaPrefixedString x)
        x
    | Syntax.PrefixedCodeExpression _ ->
      tag validate_prefixed_code_expression (fun x -> LambdaPrefixedCode x) x
    | Syntax.VariableExpression _ ->
      tag validate_variable_expression (fun x -> LambdaVariable x) x
    | Syntax.PipeVariableExpression _ ->
      tag validate_pipe_variable_expression (fun x -> LambdaPipeVariable x) x
    | Syntax.DecoratedExpression _ ->
      tag validate_decorated_expression (fun x -> LambdaDecorated x) x
    | Syntax.InclusionExpression _ ->
      tag validate_inclusion_expression (fun x -> LambdaInclusion x) x
    | Syntax.CompoundStatement _ ->
      tag validate_compound_statement (fun x -> LambdaCompoundStatement x) x
    | Syntax.AnonymousFunction _ ->
      tag validate_anonymous_function (fun x -> LambdaAnonymousFunction x) x
    | Syntax.LambdaExpression _ ->
      tag validate_lambda_expression (fun x -> LambdaLambda x) x
    | Syntax.CastExpression _ ->
      tag validate_cast_expression (fun x -> LambdaCast x) x
    | Syntax.ScopeResolutionExpression _ ->
      tag
        validate_scope_resolution_expression
        (fun x -> LambdaScopeResolution x)
        x
    | Syntax.MemberSelectionExpression _ ->
      tag
        validate_member_selection_expression
        (fun x -> LambdaMemberSelection x)
        x
    | Syntax.SafeMemberSelectionExpression _ ->
      tag
        validate_safe_member_selection_expression
        (fun x -> LambdaSafeMemberSelection x)
        x
    | Syntax.EmbeddedMemberSelectionExpression _ ->
      tag
        validate_embedded_member_selection_expression
        (fun x -> LambdaEmbeddedMemberSelection x)
        x
    | Syntax.YieldExpression _ ->
      tag validate_yield_expression (fun x -> LambdaYield x) x
    | Syntax.PrefixUnaryExpression _ ->
      tag validate_prefix_unary_expression (fun x -> LambdaPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ ->
      tag validate_postfix_unary_expression (fun x -> LambdaPostfixUnary x) x
    | Syntax.BinaryExpression _ ->
      tag validate_binary_expression (fun x -> LambdaBinary x) x
    | Syntax.IsExpression _ ->
      tag validate_is_expression (fun x -> LambdaIs x) x
    | Syntax.AsExpression _ ->
      tag validate_as_expression (fun x -> LambdaAs x) x
    | Syntax.NullableAsExpression _ ->
      tag validate_nullable_as_expression (fun x -> LambdaNullableAs x) x
    | Syntax.ConditionalExpression _ ->
      tag validate_conditional_expression (fun x -> LambdaConditional x) x
    | Syntax.EvalExpression _ ->
      tag validate_eval_expression (fun x -> LambdaEval x) x
    | Syntax.IssetExpression _ ->
      tag validate_isset_expression (fun x -> LambdaIsset x) x
    | Syntax.FunctionCallExpression _ ->
      tag validate_function_call_expression (fun x -> LambdaFunctionCall x) x
    | Syntax.FunctionPointerExpression _ ->
      tag
        validate_function_pointer_expression
        (fun x -> LambdaFunctionPointer x)
        x
    | Syntax.ParenthesizedExpression _ ->
      tag validate_parenthesized_expression (fun x -> LambdaParenthesized x) x
    | Syntax.BracedExpression _ ->
      tag validate_braced_expression (fun x -> LambdaBraced x) x
    | Syntax.ETSpliceExpression _ ->
      tag validate_et_splice_expression (fun x -> LambdaETSplice x) x
    | Syntax.EmbeddedBracedExpression _ ->
      tag
        validate_embedded_braced_expression
        (fun x -> LambdaEmbeddedBraced x)
        x
    | Syntax.ListExpression _ ->
      tag validate_list_expression (fun x -> LambdaList x) x
    | Syntax.CollectionLiteralExpression _ ->
      tag
        validate_collection_literal_expression
        (fun x -> LambdaCollectionLiteral x)
        x
    | Syntax.ObjectCreationExpression _ ->
      tag
        validate_object_creation_expression
        (fun x -> LambdaObjectCreation x)
        x
    | Syntax.RecordCreationExpression _ ->
      tag
        validate_record_creation_expression
        (fun x -> LambdaRecordCreation x)
        x
    | Syntax.DarrayIntrinsicExpression _ ->
      tag
        validate_darray_intrinsic_expression
        (fun x -> LambdaDarrayIntrinsic x)
        x
    | Syntax.DictionaryIntrinsicExpression _ ->
      tag
        validate_dictionary_intrinsic_expression
        (fun x -> LambdaDictionaryIntrinsic x)
        x
    | Syntax.KeysetIntrinsicExpression _ ->
      tag
        validate_keyset_intrinsic_expression
        (fun x -> LambdaKeysetIntrinsic x)
        x
    | Syntax.VarrayIntrinsicExpression _ ->
      tag
        validate_varray_intrinsic_expression
        (fun x -> LambdaVarrayIntrinsic x)
        x
    | Syntax.VectorIntrinsicExpression _ ->
      tag
        validate_vector_intrinsic_expression
        (fun x -> LambdaVectorIntrinsic x)
        x
    | Syntax.SubscriptExpression _ ->
      tag validate_subscript_expression (fun x -> LambdaSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ ->
      tag
        validate_embedded_subscript_expression
        (fun x -> LambdaEmbeddedSubscript x)
        x
    | Syntax.AwaitableCreationExpression _ ->
      tag
        validate_awaitable_creation_expression
        (fun x -> LambdaAwaitableCreation x)
        x
    | Syntax.XHPChildrenParenthesizedList _ ->
      tag
        validate_xhp_children_parenthesized_list
        (fun x -> LambdaXHPChildrenParenthesizedList x)
        x
    | Syntax.XHPExpression _ ->
      tag validate_xhp_expression (fun x -> LambdaXHP x) x
    | Syntax.ShapeExpression _ ->
      tag validate_shape_expression (fun x -> LambdaShape x) x
    | Syntax.TupleExpression _ ->
      tag validate_tuple_expression (fun x -> LambdaTuple x) x
    | s -> aggregation_fail Def.LambdaBody s

  and invalidate_lambda_body : lambda_body invalidator =
   fun (value, thing) ->
    match thing with
    | LambdaLiteral thing -> invalidate_literal_expression (value, thing)
    | LambdaPrefixedString thing ->
      invalidate_prefixed_string_expression (value, thing)
    | LambdaPrefixedCode thing ->
      invalidate_prefixed_code_expression (value, thing)
    | LambdaVariable thing -> invalidate_variable_expression (value, thing)
    | LambdaPipeVariable thing ->
      invalidate_pipe_variable_expression (value, thing)
    | LambdaDecorated thing -> invalidate_decorated_expression (value, thing)
    | LambdaInclusion thing -> invalidate_inclusion_expression (value, thing)
    | LambdaCompoundStatement thing ->
      invalidate_compound_statement (value, thing)
    | LambdaAnonymousFunction thing ->
      invalidate_anonymous_function (value, thing)
    | LambdaLambda thing -> invalidate_lambda_expression (value, thing)
    | LambdaCast thing -> invalidate_cast_expression (value, thing)
    | LambdaScopeResolution thing ->
      invalidate_scope_resolution_expression (value, thing)
    | LambdaMemberSelection thing ->
      invalidate_member_selection_expression (value, thing)
    | LambdaSafeMemberSelection thing ->
      invalidate_safe_member_selection_expression (value, thing)
    | LambdaEmbeddedMemberSelection thing ->
      invalidate_embedded_member_selection_expression (value, thing)
    | LambdaYield thing -> invalidate_yield_expression (value, thing)
    | LambdaPrefixUnary thing ->
      invalidate_prefix_unary_expression (value, thing)
    | LambdaPostfixUnary thing ->
      invalidate_postfix_unary_expression (value, thing)
    | LambdaBinary thing -> invalidate_binary_expression (value, thing)
    | LambdaIs thing -> invalidate_is_expression (value, thing)
    | LambdaAs thing -> invalidate_as_expression (value, thing)
    | LambdaNullableAs thing -> invalidate_nullable_as_expression (value, thing)
    | LambdaConditional thing -> invalidate_conditional_expression (value, thing)
    | LambdaEval thing -> invalidate_eval_expression (value, thing)
    | LambdaIsset thing -> invalidate_isset_expression (value, thing)
    | LambdaFunctionCall thing ->
      invalidate_function_call_expression (value, thing)
    | LambdaFunctionPointer thing ->
      invalidate_function_pointer_expression (value, thing)
    | LambdaParenthesized thing ->
      invalidate_parenthesized_expression (value, thing)
    | LambdaBraced thing -> invalidate_braced_expression (value, thing)
    | LambdaETSplice thing -> invalidate_et_splice_expression (value, thing)
    | LambdaEmbeddedBraced thing ->
      invalidate_embedded_braced_expression (value, thing)
    | LambdaList thing -> invalidate_list_expression (value, thing)
    | LambdaCollectionLiteral thing ->
      invalidate_collection_literal_expression (value, thing)
    | LambdaObjectCreation thing ->
      invalidate_object_creation_expression (value, thing)
    | LambdaRecordCreation thing ->
      invalidate_record_creation_expression (value, thing)
    | LambdaDarrayIntrinsic thing ->
      invalidate_darray_intrinsic_expression (value, thing)
    | LambdaDictionaryIntrinsic thing ->
      invalidate_dictionary_intrinsic_expression (value, thing)
    | LambdaKeysetIntrinsic thing ->
      invalidate_keyset_intrinsic_expression (value, thing)
    | LambdaVarrayIntrinsic thing ->
      invalidate_varray_intrinsic_expression (value, thing)
    | LambdaVectorIntrinsic thing ->
      invalidate_vector_intrinsic_expression (value, thing)
    | LambdaSubscript thing -> invalidate_subscript_expression (value, thing)
    | LambdaEmbeddedSubscript thing ->
      invalidate_embedded_subscript_expression (value, thing)
    | LambdaAwaitableCreation thing ->
      invalidate_awaitable_creation_expression (value, thing)
    | LambdaXHPChildrenParenthesizedList thing ->
      invalidate_xhp_children_parenthesized_list (value, thing)
    | LambdaXHP thing -> invalidate_xhp_expression (value, thing)
    | LambdaShape thing -> invalidate_shape_expression (value, thing)
    | LambdaTuple thing -> invalidate_tuple_expression (value, thing)

  and validate_constructor_expression : constructor_expression validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ ->
      tag validate_literal_expression (fun x -> CExprLiteral x) x
    | Syntax.PrefixedStringExpression _ ->
      tag validate_prefixed_string_expression (fun x -> CExprPrefixedString x) x
    | Syntax.PrefixedCodeExpression _ ->
      tag validate_prefixed_code_expression (fun x -> CExprPrefixedCode x) x
    | Syntax.VariableExpression _ ->
      tag validate_variable_expression (fun x -> CExprVariable x) x
    | Syntax.PipeVariableExpression _ ->
      tag validate_pipe_variable_expression (fun x -> CExprPipeVariable x) x
    | Syntax.DecoratedExpression _ ->
      tag validate_decorated_expression (fun x -> CExprDecorated x) x
    | Syntax.InclusionExpression _ ->
      tag validate_inclusion_expression (fun x -> CExprInclusion x) x
    | Syntax.AnonymousFunction _ ->
      tag validate_anonymous_function (fun x -> CExprAnonymousFunction x) x
    | Syntax.LambdaExpression _ ->
      tag validate_lambda_expression (fun x -> CExprLambda x) x
    | Syntax.CastExpression _ ->
      tag validate_cast_expression (fun x -> CExprCast x) x
    | Syntax.ScopeResolutionExpression _ ->
      tag
        validate_scope_resolution_expression
        (fun x -> CExprScopeResolution x)
        x
    | Syntax.MemberSelectionExpression _ ->
      tag
        validate_member_selection_expression
        (fun x -> CExprMemberSelection x)
        x
    | Syntax.SafeMemberSelectionExpression _ ->
      tag
        validate_safe_member_selection_expression
        (fun x -> CExprSafeMemberSelection x)
        x
    | Syntax.EmbeddedMemberSelectionExpression _ ->
      tag
        validate_embedded_member_selection_expression
        (fun x -> CExprEmbeddedMemberSelection x)
        x
    | Syntax.YieldExpression _ ->
      tag validate_yield_expression (fun x -> CExprYield x) x
    | Syntax.PrefixUnaryExpression _ ->
      tag validate_prefix_unary_expression (fun x -> CExprPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ ->
      tag validate_postfix_unary_expression (fun x -> CExprPostfixUnary x) x
    | Syntax.BinaryExpression _ ->
      tag validate_binary_expression (fun x -> CExprBinary x) x
    | Syntax.IsExpression _ -> tag validate_is_expression (fun x -> CExprIs x) x
    | Syntax.AsExpression _ -> tag validate_as_expression (fun x -> CExprAs x) x
    | Syntax.NullableAsExpression _ ->
      tag validate_nullable_as_expression (fun x -> CExprNullableAs x) x
    | Syntax.ConditionalExpression _ ->
      tag validate_conditional_expression (fun x -> CExprConditional x) x
    | Syntax.EvalExpression _ ->
      tag validate_eval_expression (fun x -> CExprEval x) x
    | Syntax.IssetExpression _ ->
      tag validate_isset_expression (fun x -> CExprIsset x) x
    | Syntax.FunctionCallExpression _ ->
      tag validate_function_call_expression (fun x -> CExprFunctionCall x) x
    | Syntax.FunctionPointerExpression _ ->
      tag
        validate_function_pointer_expression
        (fun x -> CExprFunctionPointer x)
        x
    | Syntax.ParenthesizedExpression _ ->
      tag validate_parenthesized_expression (fun x -> CExprParenthesized x) x
    | Syntax.BracedExpression _ ->
      tag validate_braced_expression (fun x -> CExprBraced x) x
    | Syntax.ETSpliceExpression _ ->
      tag validate_et_splice_expression (fun x -> CExprETSplice x) x
    | Syntax.EmbeddedBracedExpression _ ->
      tag validate_embedded_braced_expression (fun x -> CExprEmbeddedBraced x) x
    | Syntax.ListExpression _ ->
      tag validate_list_expression (fun x -> CExprList x) x
    | Syntax.CollectionLiteralExpression _ ->
      tag
        validate_collection_literal_expression
        (fun x -> CExprCollectionLiteral x)
        x
    | Syntax.ObjectCreationExpression _ ->
      tag validate_object_creation_expression (fun x -> CExprObjectCreation x) x
    | Syntax.RecordCreationExpression _ ->
      tag validate_record_creation_expression (fun x -> CExprRecordCreation x) x
    | Syntax.DarrayIntrinsicExpression _ ->
      tag
        validate_darray_intrinsic_expression
        (fun x -> CExprDarrayIntrinsic x)
        x
    | Syntax.DictionaryIntrinsicExpression _ ->
      tag
        validate_dictionary_intrinsic_expression
        (fun x -> CExprDictionaryIntrinsic x)
        x
    | Syntax.KeysetIntrinsicExpression _ ->
      tag
        validate_keyset_intrinsic_expression
        (fun x -> CExprKeysetIntrinsic x)
        x
    | Syntax.VarrayIntrinsicExpression _ ->
      tag
        validate_varray_intrinsic_expression
        (fun x -> CExprVarrayIntrinsic x)
        x
    | Syntax.VectorIntrinsicExpression _ ->
      tag
        validate_vector_intrinsic_expression
        (fun x -> CExprVectorIntrinsic x)
        x
    | Syntax.ElementInitializer _ ->
      tag validate_element_initializer (fun x -> CExprElementInitializer x) x
    | Syntax.SubscriptExpression _ ->
      tag validate_subscript_expression (fun x -> CExprSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ ->
      tag
        validate_embedded_subscript_expression
        (fun x -> CExprEmbeddedSubscript x)
        x
    | Syntax.AwaitableCreationExpression _ ->
      tag
        validate_awaitable_creation_expression
        (fun x -> CExprAwaitableCreation x)
        x
    | Syntax.XHPChildrenParenthesizedList _ ->
      tag
        validate_xhp_children_parenthesized_list
        (fun x -> CExprXHPChildrenParenthesizedList x)
        x
    | Syntax.XHPExpression _ ->
      tag validate_xhp_expression (fun x -> CExprXHP x) x
    | Syntax.ShapeExpression _ ->
      tag validate_shape_expression (fun x -> CExprShape x) x
    | Syntax.TupleExpression _ ->
      tag validate_tuple_expression (fun x -> CExprTuple x) x
    | s -> aggregation_fail Def.ConstructorExpression s

  and invalidate_constructor_expression : constructor_expression invalidator =
   fun (value, thing) ->
    match thing with
    | CExprLiteral thing -> invalidate_literal_expression (value, thing)
    | CExprPrefixedString thing ->
      invalidate_prefixed_string_expression (value, thing)
    | CExprPrefixedCode thing ->
      invalidate_prefixed_code_expression (value, thing)
    | CExprVariable thing -> invalidate_variable_expression (value, thing)
    | CExprPipeVariable thing ->
      invalidate_pipe_variable_expression (value, thing)
    | CExprDecorated thing -> invalidate_decorated_expression (value, thing)
    | CExprInclusion thing -> invalidate_inclusion_expression (value, thing)
    | CExprAnonymousFunction thing ->
      invalidate_anonymous_function (value, thing)
    | CExprLambda thing -> invalidate_lambda_expression (value, thing)
    | CExprCast thing -> invalidate_cast_expression (value, thing)
    | CExprScopeResolution thing ->
      invalidate_scope_resolution_expression (value, thing)
    | CExprMemberSelection thing ->
      invalidate_member_selection_expression (value, thing)
    | CExprSafeMemberSelection thing ->
      invalidate_safe_member_selection_expression (value, thing)
    | CExprEmbeddedMemberSelection thing ->
      invalidate_embedded_member_selection_expression (value, thing)
    | CExprYield thing -> invalidate_yield_expression (value, thing)
    | CExprPrefixUnary thing -> invalidate_prefix_unary_expression (value, thing)
    | CExprPostfixUnary thing ->
      invalidate_postfix_unary_expression (value, thing)
    | CExprBinary thing -> invalidate_binary_expression (value, thing)
    | CExprIs thing -> invalidate_is_expression (value, thing)
    | CExprAs thing -> invalidate_as_expression (value, thing)
    | CExprNullableAs thing -> invalidate_nullable_as_expression (value, thing)
    | CExprConditional thing -> invalidate_conditional_expression (value, thing)
    | CExprEval thing -> invalidate_eval_expression (value, thing)
    | CExprIsset thing -> invalidate_isset_expression (value, thing)
    | CExprFunctionCall thing ->
      invalidate_function_call_expression (value, thing)
    | CExprFunctionPointer thing ->
      invalidate_function_pointer_expression (value, thing)
    | CExprParenthesized thing ->
      invalidate_parenthesized_expression (value, thing)
    | CExprBraced thing -> invalidate_braced_expression (value, thing)
    | CExprETSplice thing -> invalidate_et_splice_expression (value, thing)
    | CExprEmbeddedBraced thing ->
      invalidate_embedded_braced_expression (value, thing)
    | CExprList thing -> invalidate_list_expression (value, thing)
    | CExprCollectionLiteral thing ->
      invalidate_collection_literal_expression (value, thing)
    | CExprObjectCreation thing ->
      invalidate_object_creation_expression (value, thing)
    | CExprRecordCreation thing ->
      invalidate_record_creation_expression (value, thing)
    | CExprDarrayIntrinsic thing ->
      invalidate_darray_intrinsic_expression (value, thing)
    | CExprDictionaryIntrinsic thing ->
      invalidate_dictionary_intrinsic_expression (value, thing)
    | CExprKeysetIntrinsic thing ->
      invalidate_keyset_intrinsic_expression (value, thing)
    | CExprVarrayIntrinsic thing ->
      invalidate_varray_intrinsic_expression (value, thing)
    | CExprVectorIntrinsic thing ->
      invalidate_vector_intrinsic_expression (value, thing)
    | CExprElementInitializer thing ->
      invalidate_element_initializer (value, thing)
    | CExprSubscript thing -> invalidate_subscript_expression (value, thing)
    | CExprEmbeddedSubscript thing ->
      invalidate_embedded_subscript_expression (value, thing)
    | CExprAwaitableCreation thing ->
      invalidate_awaitable_creation_expression (value, thing)
    | CExprXHPChildrenParenthesizedList thing ->
      invalidate_xhp_children_parenthesized_list (value, thing)
    | CExprXHP thing -> invalidate_xhp_expression (value, thing)
    | CExprShape thing -> invalidate_shape_expression (value, thing)
    | CExprTuple thing -> invalidate_tuple_expression (value, thing)

  and validate_namespace_internals : namespace_internals validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.NamespaceBody _ ->
      tag validate_namespace_body (fun x -> NSINamespaceBody x) x
    | Syntax.NamespaceEmptyBody _ ->
      tag validate_namespace_empty_body (fun x -> NSINamespaceEmptyBody x) x
    | s -> aggregation_fail Def.NamespaceInternals s

  and invalidate_namespace_internals : namespace_internals invalidator =
   fun (value, thing) ->
    match thing with
    | NSINamespaceBody thing -> invalidate_namespace_body (value, thing)
    | NSINamespaceEmptyBody thing ->
      invalidate_namespace_empty_body (value, thing)

  and validate_xhp_attribute : xhp_attribute validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.XHPSimpleAttribute _ ->
      tag validate_xhp_simple_attribute (fun x -> XHPAttrXHPSimpleAttribute x) x
    | Syntax.XHPSpreadAttribute _ ->
      tag validate_xhp_spread_attribute (fun x -> XHPAttrXHPSpreadAttribute x) x
    | s -> aggregation_fail Def.XHPAttribute s

  and invalidate_xhp_attribute : xhp_attribute invalidator =
   fun (value, thing) ->
    match thing with
    | XHPAttrXHPSimpleAttribute thing ->
      invalidate_xhp_simple_attribute (value, thing)
    | XHPAttrXHPSpreadAttribute thing ->
      invalidate_xhp_spread_attribute (value, thing)

  and validate_object_creation_what : object_creation_what validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.AnonymousClass _ ->
      tag validate_anonymous_class (fun x -> NewAnonymousClass x) x
    | Syntax.ConstructorCall _ ->
      tag validate_constructor_call (fun x -> NewConstructorCall x) x
    | s -> aggregation_fail Def.ObjectCreationWhat s

  and invalidate_object_creation_what : object_creation_what invalidator =
   fun (value, thing) ->
    match thing with
    | NewAnonymousClass thing -> invalidate_anonymous_class (value, thing)
    | NewConstructorCall thing -> invalidate_constructor_call (value, thing)

  and validate_todo_aggregate : todo_aggregate validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.EndOfFile _ ->
      tag validate_end_of_file (fun x -> TODOEndOfFile x) x
    | s -> aggregation_fail Def.TODO s

  and invalidate_todo_aggregate : todo_aggregate invalidator =
   fun (value, thing) ->
    match thing with
    | TODOEndOfFile thing -> invalidate_end_of_file (value, thing)

  and validate_name_aggregate : name_aggregate validator =
   fun x ->
    match Syntax.syntax x with
    | Syntax.QualifiedName _ ->
      tag validate_qualified_name (fun x -> NameQualifiedName x) x
    | s -> aggregation_fail Def.Name s

  and invalidate_name_aggregate : name_aggregate invalidator =
   fun (value, thing) ->
    match thing with
    | NameQualifiedName thing -> invalidate_qualified_name (value, thing)

  and validate_end_of_file : end_of_file validator = function
    | { Syntax.syntax = Syntax.EndOfFile x; value = v } ->
      (v, { end_of_file_token = validate_token x.end_of_file_token })
    | s -> validation_fail (Some SyntaxKind.EndOfFile) s

  and invalidate_end_of_file : end_of_file invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EndOfFile
          { end_of_file_token = invalidate_token x.end_of_file_token };
      Syntax.value = v;
    }

  and validate_script : script validator = function
    | { Syntax.syntax = Syntax.Script x; value = v } ->
      ( v,
        {
          script_declarations =
            validate_list_with
              validate_top_level_declaration
              x.script_declarations;
        } )
    | s -> validation_fail (Some SyntaxKind.Script) s

  and invalidate_script : script invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.Script
          {
            script_declarations =
              invalidate_list_with
                invalidate_top_level_declaration
                x.script_declarations;
          };
      Syntax.value = v;
    }

  and validate_qualified_name : qualified_name validator = function
    | { Syntax.syntax = Syntax.QualifiedName x; value = v } ->
      ( v,
        {
          qualified_name_parts =
            validate_list_with validate_token x.qualified_name_parts;
        } )
    | s -> validation_fail (Some SyntaxKind.QualifiedName) s

  and invalidate_qualified_name : qualified_name invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.QualifiedName
          {
            qualified_name_parts =
              invalidate_list_with invalidate_token x.qualified_name_parts;
          };
      Syntax.value = v;
    }

  and validate_simple_type_specifier : simple_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.SimpleTypeSpecifier x; value = v } ->
      ( v,
        {
          simple_type_specifier =
            validate_name_aggregate x.simple_type_specifier;
        } )
    | s -> validation_fail (Some SyntaxKind.SimpleTypeSpecifier) s

  and invalidate_simple_type_specifier : simple_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SimpleTypeSpecifier
          {
            simple_type_specifier =
              invalidate_name_aggregate x.simple_type_specifier;
          };
      Syntax.value = v;
    }

  and validate_literal_expression : literal_expression validator = function
    | { Syntax.syntax = Syntax.LiteralExpression x; value = v } ->
      ( v,
        {
          literal_expression =
            validate_list_with validate_expression x.literal_expression;
        } )
    | s -> validation_fail (Some SyntaxKind.LiteralExpression) s

  and invalidate_literal_expression : literal_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.LiteralExpression
          {
            literal_expression =
              invalidate_list_with invalidate_expression x.literal_expression;
          };
      Syntax.value = v;
    }

  and validate_prefixed_string_expression : prefixed_string_expression validator
      = function
    | { Syntax.syntax = Syntax.PrefixedStringExpression x; value = v } ->
      ( v,
        {
          prefixed_string_str = validate_token x.prefixed_string_str;
          prefixed_string_name = validate_token x.prefixed_string_name;
        } )
    | s -> validation_fail (Some SyntaxKind.PrefixedStringExpression) s

  and invalidate_prefixed_string_expression :
      prefixed_string_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PrefixedStringExpression
          {
            prefixed_string_name = invalidate_token x.prefixed_string_name;
            prefixed_string_str = invalidate_token x.prefixed_string_str;
          };
      Syntax.value = v;
    }

  and validate_prefixed_code_expression : prefixed_code_expression validator =
    function
    | { Syntax.syntax = Syntax.PrefixedCodeExpression x; value = v } ->
      ( v,
        {
          prefixed_code_right_backtick =
            validate_token x.prefixed_code_right_backtick;
          prefixed_code_expression =
            validate_expression x.prefixed_code_expression;
          prefixed_code_left_backtick =
            validate_token x.prefixed_code_left_backtick;
          prefixed_code_prefix = validate_token x.prefixed_code_prefix;
        } )
    | s -> validation_fail (Some SyntaxKind.PrefixedCodeExpression) s

  and invalidate_prefixed_code_expression : prefixed_code_expression invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PrefixedCodeExpression
          {
            prefixed_code_prefix = invalidate_token x.prefixed_code_prefix;
            prefixed_code_left_backtick =
              invalidate_token x.prefixed_code_left_backtick;
            prefixed_code_expression =
              invalidate_expression x.prefixed_code_expression;
            prefixed_code_right_backtick =
              invalidate_token x.prefixed_code_right_backtick;
          };
      Syntax.value = v;
    }

  and validate_variable_expression : variable_expression validator = function
    | { Syntax.syntax = Syntax.VariableExpression x; value = v } ->
      (v, { variable_expression = validate_token x.variable_expression })
    | s -> validation_fail (Some SyntaxKind.VariableExpression) s

  and invalidate_variable_expression : variable_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VariableExpression
          { variable_expression = invalidate_token x.variable_expression };
      Syntax.value = v;
    }

  and validate_pipe_variable_expression : pipe_variable_expression validator =
    function
    | { Syntax.syntax = Syntax.PipeVariableExpression x; value = v } ->
      ( v,
        { pipe_variable_expression = validate_token x.pipe_variable_expression }
      )
    | s -> validation_fail (Some SyntaxKind.PipeVariableExpression) s

  and invalidate_pipe_variable_expression : pipe_variable_expression invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PipeVariableExpression
          {
            pipe_variable_expression =
              invalidate_token x.pipe_variable_expression;
          };
      Syntax.value = v;
    }

  and validate_file_attribute_specification :
      file_attribute_specification validator = function
    | { Syntax.syntax = Syntax.FileAttributeSpecification x; value = v } ->
      ( v,
        {
          file_attribute_specification_right_double_angle =
            validate_token x.file_attribute_specification_right_double_angle;
          file_attribute_specification_attributes =
            validate_list_with
              validate_constructor_call
              x.file_attribute_specification_attributes;
          file_attribute_specification_colon =
            validate_token x.file_attribute_specification_colon;
          file_attribute_specification_keyword =
            validate_token x.file_attribute_specification_keyword;
          file_attribute_specification_left_double_angle =
            validate_token x.file_attribute_specification_left_double_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.FileAttributeSpecification) s

  and invalidate_file_attribute_specification :
      file_attribute_specification invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FileAttributeSpecification
          {
            file_attribute_specification_left_double_angle =
              invalidate_token x.file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword =
              invalidate_token x.file_attribute_specification_keyword;
            file_attribute_specification_colon =
              invalidate_token x.file_attribute_specification_colon;
            file_attribute_specification_attributes =
              invalidate_list_with
                invalidate_constructor_call
                x.file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle =
              invalidate_token x.file_attribute_specification_right_double_angle;
          };
      Syntax.value = v;
    }

  and validate_enum_declaration : enum_declaration validator = function
    | { Syntax.syntax = Syntax.EnumDeclaration x; value = v } ->
      ( v,
        {
          enum_right_brace = validate_token x.enum_right_brace;
          enum_enumerators =
            validate_list_with validate_enumerator x.enum_enumerators;
          enum_use_clauses =
            validate_list_with validate_enum_use x.enum_use_clauses;
          enum_left_brace = validate_token x.enum_left_brace;
          enum_type = validate_option_with validate_type_constraint x.enum_type;
          enum_base = validate_specifier x.enum_base;
          enum_colon = validate_token x.enum_colon;
          enum_name = validate_token x.enum_name;
          enum_keyword = validate_token x.enum_keyword;
          enum_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.enum_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.EnumDeclaration) s

  and invalidate_enum_declaration : enum_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EnumDeclaration
          {
            enum_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.enum_attribute_spec;
            enum_keyword = invalidate_token x.enum_keyword;
            enum_name = invalidate_token x.enum_name;
            enum_colon = invalidate_token x.enum_colon;
            enum_base = invalidate_specifier x.enum_base;
            enum_type =
              invalidate_option_with invalidate_type_constraint x.enum_type;
            enum_left_brace = invalidate_token x.enum_left_brace;
            enum_use_clauses =
              invalidate_list_with invalidate_enum_use x.enum_use_clauses;
            enum_enumerators =
              invalidate_list_with invalidate_enumerator x.enum_enumerators;
            enum_right_brace = invalidate_token x.enum_right_brace;
          };
      Syntax.value = v;
    }

  and validate_enum_use : enum_use validator = function
    | { Syntax.syntax = Syntax.EnumUse x; value = v } ->
      ( v,
        {
          enum_use_semicolon = validate_token x.enum_use_semicolon;
          enum_use_names =
            validate_list_with validate_specifier x.enum_use_names;
          enum_use_keyword = validate_token x.enum_use_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.EnumUse) s

  and invalidate_enum_use : enum_use invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EnumUse
          {
            enum_use_keyword = invalidate_token x.enum_use_keyword;
            enum_use_names =
              invalidate_list_with invalidate_specifier x.enum_use_names;
            enum_use_semicolon = invalidate_token x.enum_use_semicolon;
          };
      Syntax.value = v;
    }

  and validate_enumerator : enumerator validator = function
    | { Syntax.syntax = Syntax.Enumerator x; value = v } ->
      ( v,
        {
          enumerator_semicolon = validate_token x.enumerator_semicolon;
          enumerator_value = validate_expression x.enumerator_value;
          enumerator_equal = validate_token x.enumerator_equal;
          enumerator_name = validate_token x.enumerator_name;
        } )
    | s -> validation_fail (Some SyntaxKind.Enumerator) s

  and invalidate_enumerator : enumerator invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.Enumerator
          {
            enumerator_name = invalidate_token x.enumerator_name;
            enumerator_equal = invalidate_token x.enumerator_equal;
            enumerator_value = invalidate_expression x.enumerator_value;
            enumerator_semicolon = invalidate_token x.enumerator_semicolon;
          };
      Syntax.value = v;
    }

  and validate_enum_class_declaration : enum_class_declaration validator =
    function
    | { Syntax.syntax = Syntax.EnumClassDeclaration x; value = v } ->
      ( v,
        {
          enum_class_right_brace = validate_token x.enum_class_right_brace;
          enum_class_elements =
            validate_list_with
              validate_enum_class_enumerator
              x.enum_class_elements;
          enum_class_left_brace = validate_token x.enum_class_left_brace;
          enum_class_extends_list =
            validate_list_with validate_specifier x.enum_class_extends_list;
          enum_class_extends =
            validate_option_with validate_token x.enum_class_extends;
          enum_class_base = validate_specifier x.enum_class_base;
          enum_class_colon = validate_token x.enum_class_colon;
          enum_class_name = validate_token x.enum_class_name;
          enum_class_class_keyword = validate_token x.enum_class_class_keyword;
          enum_class_enum_keyword = validate_token x.enum_class_enum_keyword;
          enum_class_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.enum_class_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.EnumClassDeclaration) s

  and invalidate_enum_class_declaration : enum_class_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EnumClassDeclaration
          {
            enum_class_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.enum_class_attribute_spec;
            enum_class_enum_keyword = invalidate_token x.enum_class_enum_keyword;
            enum_class_class_keyword =
              invalidate_token x.enum_class_class_keyword;
            enum_class_name = invalidate_token x.enum_class_name;
            enum_class_colon = invalidate_token x.enum_class_colon;
            enum_class_base = invalidate_specifier x.enum_class_base;
            enum_class_extends =
              invalidate_option_with invalidate_token x.enum_class_extends;
            enum_class_extends_list =
              invalidate_list_with
                invalidate_specifier
                x.enum_class_extends_list;
            enum_class_left_brace = invalidate_token x.enum_class_left_brace;
            enum_class_elements =
              invalidate_list_with
                invalidate_enum_class_enumerator
                x.enum_class_elements;
            enum_class_right_brace = invalidate_token x.enum_class_right_brace;
          };
      Syntax.value = v;
    }

  and validate_enum_class_enumerator : enum_class_enumerator validator =
    function
    | { Syntax.syntax = Syntax.EnumClassEnumerator x; value = v } ->
      ( v,
        {
          enum_class_enumerator_semicolon =
            validate_token x.enum_class_enumerator_semicolon;
          enum_class_enumerator_initial_value =
            validate_expression x.enum_class_enumerator_initial_value;
          enum_class_enumerator_equal =
            validate_token x.enum_class_enumerator_equal;
          enum_class_enumerator_name =
            validate_token x.enum_class_enumerator_name;
          enum_class_enumerator_type =
            validate_specifier x.enum_class_enumerator_type;
        } )
    | s -> validation_fail (Some SyntaxKind.EnumClassEnumerator) s

  and invalidate_enum_class_enumerator : enum_class_enumerator invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EnumClassEnumerator
          {
            enum_class_enumerator_type =
              invalidate_specifier x.enum_class_enumerator_type;
            enum_class_enumerator_name =
              invalidate_token x.enum_class_enumerator_name;
            enum_class_enumerator_equal =
              invalidate_token x.enum_class_enumerator_equal;
            enum_class_enumerator_initial_value =
              invalidate_expression x.enum_class_enumerator_initial_value;
            enum_class_enumerator_semicolon =
              invalidate_token x.enum_class_enumerator_semicolon;
          };
      Syntax.value = v;
    }

  and validate_record_declaration : record_declaration validator = function
    | { Syntax.syntax = Syntax.RecordDeclaration x; value = v } ->
      ( v,
        {
          record_right_brace = validate_token x.record_right_brace;
          record_fields =
            validate_list_with validate_record_field x.record_fields;
          record_left_brace = validate_token x.record_left_brace;
          record_extends_opt =
            validate_option_with validate_type_constraint x.record_extends_opt;
          record_extends_keyword =
            validate_option_with validate_token x.record_extends_keyword;
          record_name = validate_token x.record_name;
          record_keyword = validate_token x.record_keyword;
          record_modifier = validate_token x.record_modifier;
          record_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.record_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.RecordDeclaration) s

  and invalidate_record_declaration : record_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.RecordDeclaration
          {
            record_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.record_attribute_spec;
            record_modifier = invalidate_token x.record_modifier;
            record_keyword = invalidate_token x.record_keyword;
            record_name = invalidate_token x.record_name;
            record_extends_keyword =
              invalidate_option_with invalidate_token x.record_extends_keyword;
            record_extends_opt =
              invalidate_option_with
                invalidate_type_constraint
                x.record_extends_opt;
            record_left_brace = invalidate_token x.record_left_brace;
            record_fields =
              invalidate_list_with invalidate_record_field x.record_fields;
            record_right_brace = invalidate_token x.record_right_brace;
          };
      Syntax.value = v;
    }

  and validate_record_field : record_field validator = function
    | { Syntax.syntax = Syntax.RecordField x; value = v } ->
      ( v,
        {
          record_field_semi = validate_token x.record_field_semi;
          record_field_init =
            validate_option_with validate_simple_initializer x.record_field_init;
          record_field_name = validate_token x.record_field_name;
          record_field_type = validate_type_constraint x.record_field_type;
        } )
    | s -> validation_fail (Some SyntaxKind.RecordField) s

  and invalidate_record_field : record_field invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.RecordField
          {
            record_field_type = invalidate_type_constraint x.record_field_type;
            record_field_name = invalidate_token x.record_field_name;
            record_field_init =
              invalidate_option_with
                invalidate_simple_initializer
                x.record_field_init;
            record_field_semi = invalidate_token x.record_field_semi;
          };
      Syntax.value = v;
    }

  and validate_alias_declaration : alias_declaration validator = function
    | { Syntax.syntax = Syntax.AliasDeclaration x; value = v } ->
      ( v,
        {
          alias_semicolon = validate_token x.alias_semicolon;
          alias_type = validate_specifier x.alias_type;
          alias_equal = validate_option_with validate_token x.alias_equal;
          alias_constraint =
            validate_option_with validate_type_constraint x.alias_constraint;
          alias_generic_parameter =
            validate_option_with
              validate_type_parameters
              x.alias_generic_parameter;
          alias_name = validate_option_with validate_token x.alias_name;
          alias_keyword = validate_token x.alias_keyword;
          alias_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.alias_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.AliasDeclaration) s

  and invalidate_alias_declaration : alias_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AliasDeclaration
          {
            alias_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.alias_attribute_spec;
            alias_keyword = invalidate_token x.alias_keyword;
            alias_name = invalidate_option_with invalidate_token x.alias_name;
            alias_generic_parameter =
              invalidate_option_with
                invalidate_type_parameters
                x.alias_generic_parameter;
            alias_constraint =
              invalidate_option_with
                invalidate_type_constraint
                x.alias_constraint;
            alias_equal = invalidate_option_with invalidate_token x.alias_equal;
            alias_type = invalidate_specifier x.alias_type;
            alias_semicolon = invalidate_token x.alias_semicolon;
          };
      Syntax.value = v;
    }

  and validate_context_alias_declaration : context_alias_declaration validator =
    function
    | { Syntax.syntax = Syntax.ContextAliasDeclaration x; value = v } ->
      ( v,
        {
          ctx_alias_semicolon = validate_token x.ctx_alias_semicolon;
          ctx_alias_context = validate_specifier x.ctx_alias_context;
          ctx_alias_equal =
            validate_option_with validate_token x.ctx_alias_equal;
          ctx_alias_as_constraint =
            validate_option_with
              validate_context_constraint
              x.ctx_alias_as_constraint;
          ctx_alias_generic_parameter =
            validate_option_with
              validate_type_parameters
              x.ctx_alias_generic_parameter;
          ctx_alias_name = validate_option_with validate_token x.ctx_alias_name;
          ctx_alias_keyword = validate_token x.ctx_alias_keyword;
          ctx_alias_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.ctx_alias_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.ContextAliasDeclaration) s

  and invalidate_context_alias_declaration :
      context_alias_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ContextAliasDeclaration
          {
            ctx_alias_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.ctx_alias_attribute_spec;
            ctx_alias_keyword = invalidate_token x.ctx_alias_keyword;
            ctx_alias_name =
              invalidate_option_with invalidate_token x.ctx_alias_name;
            ctx_alias_generic_parameter =
              invalidate_option_with
                invalidate_type_parameters
                x.ctx_alias_generic_parameter;
            ctx_alias_as_constraint =
              invalidate_option_with
                invalidate_context_constraint
                x.ctx_alias_as_constraint;
            ctx_alias_equal =
              invalidate_option_with invalidate_token x.ctx_alias_equal;
            ctx_alias_context = invalidate_specifier x.ctx_alias_context;
            ctx_alias_semicolon = invalidate_token x.ctx_alias_semicolon;
          };
      Syntax.value = v;
    }

  and validate_property_declaration : property_declaration validator = function
    | { Syntax.syntax = Syntax.PropertyDeclaration x; value = v } ->
      ( v,
        {
          property_semicolon = validate_token x.property_semicolon;
          property_declarators =
            validate_list_with
              validate_property_declarator
              x.property_declarators;
          property_type =
            validate_option_with validate_specifier x.property_type;
          property_modifiers =
            validate_list_with validate_token x.property_modifiers;
          property_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.property_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.PropertyDeclaration) s

  and invalidate_property_declaration : property_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PropertyDeclaration
          {
            property_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.property_attribute_spec;
            property_modifiers =
              invalidate_list_with invalidate_token x.property_modifiers;
            property_type =
              invalidate_option_with invalidate_specifier x.property_type;
            property_declarators =
              invalidate_list_with
                invalidate_property_declarator
                x.property_declarators;
            property_semicolon = invalidate_token x.property_semicolon;
          };
      Syntax.value = v;
    }

  and validate_property_declarator : property_declarator validator = function
    | { Syntax.syntax = Syntax.PropertyDeclarator x; value = v } ->
      ( v,
        {
          property_initializer =
            validate_option_with
              validate_simple_initializer
              x.property_initializer;
          property_name = validate_token x.property_name;
        } )
    | s -> validation_fail (Some SyntaxKind.PropertyDeclarator) s

  and invalidate_property_declarator : property_declarator invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PropertyDeclarator
          {
            property_name = invalidate_token x.property_name;
            property_initializer =
              invalidate_option_with
                invalidate_simple_initializer
                x.property_initializer;
          };
      Syntax.value = v;
    }

  and validate_namespace_declaration : namespace_declaration validator =
    function
    | { Syntax.syntax = Syntax.NamespaceDeclaration x; value = v } ->
      ( v,
        {
          namespace_body = validate_namespace_internals x.namespace_body;
          namespace_header =
            validate_namespace_declaration_header x.namespace_header;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceDeclaration) s

  and invalidate_namespace_declaration : namespace_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceDeclaration
          {
            namespace_header =
              invalidate_namespace_declaration_header x.namespace_header;
            namespace_body = invalidate_namespace_internals x.namespace_body;
          };
      Syntax.value = v;
    }

  and validate_namespace_declaration_header :
      namespace_declaration_header validator = function
    | { Syntax.syntax = Syntax.NamespaceDeclarationHeader x; value = v } ->
      ( v,
        {
          namespace_name =
            validate_option_with validate_name_aggregate x.namespace_name;
          namespace_keyword = validate_token x.namespace_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceDeclarationHeader) s

  and invalidate_namespace_declaration_header :
      namespace_declaration_header invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceDeclarationHeader
          {
            namespace_keyword = invalidate_token x.namespace_keyword;
            namespace_name =
              invalidate_option_with invalidate_name_aggregate x.namespace_name;
          };
      Syntax.value = v;
    }

  and validate_namespace_body : namespace_body validator = function
    | { Syntax.syntax = Syntax.NamespaceBody x; value = v } ->
      ( v,
        {
          namespace_right_brace = validate_token x.namespace_right_brace;
          namespace_declarations =
            validate_list_with
              validate_top_level_declaration
              x.namespace_declarations;
          namespace_left_brace = validate_token x.namespace_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceBody) s

  and invalidate_namespace_body : namespace_body invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceBody
          {
            namespace_left_brace = invalidate_token x.namespace_left_brace;
            namespace_declarations =
              invalidate_list_with
                invalidate_top_level_declaration
                x.namespace_declarations;
            namespace_right_brace = invalidate_token x.namespace_right_brace;
          };
      Syntax.value = v;
    }

  and validate_namespace_empty_body : namespace_empty_body validator = function
    | { Syntax.syntax = Syntax.NamespaceEmptyBody x; value = v } ->
      (v, { namespace_semicolon = validate_token x.namespace_semicolon })
    | s -> validation_fail (Some SyntaxKind.NamespaceEmptyBody) s

  and invalidate_namespace_empty_body : namespace_empty_body invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceEmptyBody
          { namespace_semicolon = invalidate_token x.namespace_semicolon };
      Syntax.value = v;
    }

  and validate_namespace_use_declaration : namespace_use_declaration validator =
    function
    | { Syntax.syntax = Syntax.NamespaceUseDeclaration x; value = v } ->
      ( v,
        {
          namespace_use_semicolon =
            validate_option_with validate_token x.namespace_use_semicolon;
          namespace_use_clauses =
            validate_list_with
              validate_namespace_use_clause
              x.namespace_use_clauses;
          namespace_use_kind =
            validate_option_with validate_token x.namespace_use_kind;
          namespace_use_keyword = validate_token x.namespace_use_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceUseDeclaration) s

  and invalidate_namespace_use_declaration :
      namespace_use_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceUseDeclaration
          {
            namespace_use_keyword = invalidate_token x.namespace_use_keyword;
            namespace_use_kind =
              invalidate_option_with invalidate_token x.namespace_use_kind;
            namespace_use_clauses =
              invalidate_list_with
                invalidate_namespace_use_clause
                x.namespace_use_clauses;
            namespace_use_semicolon =
              invalidate_option_with invalidate_token x.namespace_use_semicolon;
          };
      Syntax.value = v;
    }

  and validate_namespace_group_use_declaration :
      namespace_group_use_declaration validator = function
    | { Syntax.syntax = Syntax.NamespaceGroupUseDeclaration x; value = v } ->
      ( v,
        {
          namespace_group_use_semicolon =
            validate_token x.namespace_group_use_semicolon;
          namespace_group_use_right_brace =
            validate_token x.namespace_group_use_right_brace;
          namespace_group_use_clauses =
            validate_list_with
              validate_namespace_use_clause
              x.namespace_group_use_clauses;
          namespace_group_use_left_brace =
            validate_token x.namespace_group_use_left_brace;
          namespace_group_use_prefix =
            validate_name_aggregate x.namespace_group_use_prefix;
          namespace_group_use_kind =
            validate_option_with validate_token x.namespace_group_use_kind;
          namespace_group_use_keyword =
            validate_token x.namespace_group_use_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceGroupUseDeclaration) s

  and invalidate_namespace_group_use_declaration :
      namespace_group_use_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceGroupUseDeclaration
          {
            namespace_group_use_keyword =
              invalidate_token x.namespace_group_use_keyword;
            namespace_group_use_kind =
              invalidate_option_with invalidate_token x.namespace_group_use_kind;
            namespace_group_use_prefix =
              invalidate_name_aggregate x.namespace_group_use_prefix;
            namespace_group_use_left_brace =
              invalidate_token x.namespace_group_use_left_brace;
            namespace_group_use_clauses =
              invalidate_list_with
                invalidate_namespace_use_clause
                x.namespace_group_use_clauses;
            namespace_group_use_right_brace =
              invalidate_token x.namespace_group_use_right_brace;
            namespace_group_use_semicolon =
              invalidate_token x.namespace_group_use_semicolon;
          };
      Syntax.value = v;
    }

  and validate_namespace_use_clause : namespace_use_clause validator = function
    | { Syntax.syntax = Syntax.NamespaceUseClause x; value = v } ->
      ( v,
        {
          namespace_use_alias =
            validate_option_with validate_token x.namespace_use_alias;
          namespace_use_as =
            validate_option_with validate_token x.namespace_use_as;
          namespace_use_name = validate_name_aggregate x.namespace_use_name;
          namespace_use_clause_kind =
            validate_option_with validate_token x.namespace_use_clause_kind;
        } )
    | s -> validation_fail (Some SyntaxKind.NamespaceUseClause) s

  and invalidate_namespace_use_clause : namespace_use_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NamespaceUseClause
          {
            namespace_use_clause_kind =
              invalidate_option_with
                invalidate_token
                x.namespace_use_clause_kind;
            namespace_use_name = invalidate_name_aggregate x.namespace_use_name;
            namespace_use_as =
              invalidate_option_with invalidate_token x.namespace_use_as;
            namespace_use_alias =
              invalidate_option_with invalidate_token x.namespace_use_alias;
          };
      Syntax.value = v;
    }

  and validate_function_declaration : function_declaration validator = function
    | { Syntax.syntax = Syntax.FunctionDeclaration x; value = v } ->
      ( v,
        {
          function_body = validate_compound_statement x.function_body;
          function_declaration_header =
            validate_function_declaration_header x.function_declaration_header;
          function_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.function_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.FunctionDeclaration) s

  and invalidate_function_declaration : function_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FunctionDeclaration
          {
            function_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.function_attribute_spec;
            function_declaration_header =
              invalidate_function_declaration_header
                x.function_declaration_header;
            function_body = invalidate_compound_statement x.function_body;
          };
      Syntax.value = v;
    }

  and validate_function_declaration_header :
      function_declaration_header validator = function
    | { Syntax.syntax = Syntax.FunctionDeclarationHeader x; value = v } ->
      ( v,
        {
          function_where_clause =
            validate_option_with validate_where_clause x.function_where_clause;
          function_type =
            validate_option_with validate_attributized_specifier x.function_type;
          function_readonly_return =
            validate_option_with validate_token x.function_readonly_return;
          function_colon = validate_option_with validate_token x.function_colon;
          function_contexts =
            validate_option_with validate_contexts x.function_contexts;
          function_right_paren = validate_token x.function_right_paren;
          function_parameter_list =
            validate_list_with validate_parameter x.function_parameter_list;
          function_left_paren = validate_token x.function_left_paren;
          function_type_parameter_list =
            validate_option_with
              validate_type_parameters
              x.function_type_parameter_list;
          function_name = validate_token x.function_name;
          function_keyword = validate_token x.function_keyword;
          function_modifiers =
            validate_list_with validate_token x.function_modifiers;
        } )
    | s -> validation_fail (Some SyntaxKind.FunctionDeclarationHeader) s

  and invalidate_function_declaration_header :
      function_declaration_header invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FunctionDeclarationHeader
          {
            function_modifiers =
              invalidate_list_with invalidate_token x.function_modifiers;
            function_keyword = invalidate_token x.function_keyword;
            function_name = invalidate_token x.function_name;
            function_type_parameter_list =
              invalidate_option_with
                invalidate_type_parameters
                x.function_type_parameter_list;
            function_left_paren = invalidate_token x.function_left_paren;
            function_parameter_list =
              invalidate_list_with
                invalidate_parameter
                x.function_parameter_list;
            function_right_paren = invalidate_token x.function_right_paren;
            function_contexts =
              invalidate_option_with invalidate_contexts x.function_contexts;
            function_colon =
              invalidate_option_with invalidate_token x.function_colon;
            function_readonly_return =
              invalidate_option_with invalidate_token x.function_readonly_return;
            function_type =
              invalidate_option_with
                invalidate_attributized_specifier
                x.function_type;
            function_where_clause =
              invalidate_option_with
                invalidate_where_clause
                x.function_where_clause;
          };
      Syntax.value = v;
    }

  and validate_contexts : contexts validator = function
    | { Syntax.syntax = Syntax.Contexts x; value = v } ->
      ( v,
        {
          contexts_right_bracket = validate_token x.contexts_right_bracket;
          contexts_types =
            validate_list_with validate_specifier x.contexts_types;
          contexts_left_bracket = validate_token x.contexts_left_bracket;
        } )
    | s -> validation_fail (Some SyntaxKind.Contexts) s

  and invalidate_contexts : contexts invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.Contexts
          {
            contexts_left_bracket = invalidate_token x.contexts_left_bracket;
            contexts_types =
              invalidate_list_with invalidate_specifier x.contexts_types;
            contexts_right_bracket = invalidate_token x.contexts_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_where_clause : where_clause validator = function
    | { Syntax.syntax = Syntax.WhereClause x; value = v } ->
      ( v,
        {
          where_clause_constraints =
            validate_list_with
              validate_where_constraint
              x.where_clause_constraints;
          where_clause_keyword = validate_token x.where_clause_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.WhereClause) s

  and invalidate_where_clause : where_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.WhereClause
          {
            where_clause_keyword = invalidate_token x.where_clause_keyword;
            where_clause_constraints =
              invalidate_list_with
                invalidate_where_constraint
                x.where_clause_constraints;
          };
      Syntax.value = v;
    }

  and validate_where_constraint : where_constraint validator = function
    | { Syntax.syntax = Syntax.WhereConstraint x; value = v } ->
      ( v,
        {
          where_constraint_right_type =
            validate_specifier x.where_constraint_right_type;
          where_constraint_operator = validate_token x.where_constraint_operator;
          where_constraint_left_type =
            validate_specifier x.where_constraint_left_type;
        } )
    | s -> validation_fail (Some SyntaxKind.WhereConstraint) s

  and invalidate_where_constraint : where_constraint invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.WhereConstraint
          {
            where_constraint_left_type =
              invalidate_specifier x.where_constraint_left_type;
            where_constraint_operator =
              invalidate_token x.where_constraint_operator;
            where_constraint_right_type =
              invalidate_specifier x.where_constraint_right_type;
          };
      Syntax.value = v;
    }

  and validate_methodish_declaration : methodish_declaration validator =
    function
    | { Syntax.syntax = Syntax.MethodishDeclaration x; value = v } ->
      ( v,
        {
          methodish_semicolon =
            validate_option_with validate_token x.methodish_semicolon;
          methodish_function_body =
            validate_option_with
              validate_compound_statement
              x.methodish_function_body;
          methodish_function_decl_header =
            validate_function_declaration_header
              x.methodish_function_decl_header;
          methodish_attribute =
            validate_option_with
              validate_attribute_specification
              x.methodish_attribute;
        } )
    | s -> validation_fail (Some SyntaxKind.MethodishDeclaration) s

  and invalidate_methodish_declaration : methodish_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.MethodishDeclaration
          {
            methodish_attribute =
              invalidate_option_with
                invalidate_attribute_specification
                x.methodish_attribute;
            methodish_function_decl_header =
              invalidate_function_declaration_header
                x.methodish_function_decl_header;
            methodish_function_body =
              invalidate_option_with
                invalidate_compound_statement
                x.methodish_function_body;
            methodish_semicolon =
              invalidate_option_with invalidate_token x.methodish_semicolon;
          };
      Syntax.value = v;
    }

  and validate_methodish_trait_resolution : methodish_trait_resolution validator
      = function
    | { Syntax.syntax = Syntax.MethodishTraitResolution x; value = v } ->
      ( v,
        {
          methodish_trait_semicolon = validate_token x.methodish_trait_semicolon;
          methodish_trait_name = validate_specifier x.methodish_trait_name;
          methodish_trait_equal = validate_token x.methodish_trait_equal;
          methodish_trait_function_decl_header =
            validate_function_declaration_header
              x.methodish_trait_function_decl_header;
          methodish_trait_attribute =
            validate_option_with
              validate_attribute_specification
              x.methodish_trait_attribute;
        } )
    | s -> validation_fail (Some SyntaxKind.MethodishTraitResolution) s

  and invalidate_methodish_trait_resolution :
      methodish_trait_resolution invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.MethodishTraitResolution
          {
            methodish_trait_attribute =
              invalidate_option_with
                invalidate_attribute_specification
                x.methodish_trait_attribute;
            methodish_trait_function_decl_header =
              invalidate_function_declaration_header
                x.methodish_trait_function_decl_header;
            methodish_trait_equal = invalidate_token x.methodish_trait_equal;
            methodish_trait_name = invalidate_specifier x.methodish_trait_name;
            methodish_trait_semicolon =
              invalidate_token x.methodish_trait_semicolon;
          };
      Syntax.value = v;
    }

  and validate_classish_declaration : classish_declaration validator = function
    | { Syntax.syntax = Syntax.ClassishDeclaration x; value = v } ->
      ( v,
        {
          classish_body = validate_classish_body x.classish_body;
          classish_where_clause =
            validate_option_with validate_where_clause x.classish_where_clause;
          classish_implements_list =
            validate_list_with validate_specifier x.classish_implements_list;
          classish_implements_keyword =
            validate_option_with validate_token x.classish_implements_keyword;
          classish_extends_list =
            validate_list_with validate_specifier x.classish_extends_list;
          classish_extends_keyword =
            validate_option_with validate_token x.classish_extends_keyword;
          classish_type_parameters =
            validate_option_with
              validate_type_parameters
              x.classish_type_parameters;
          classish_name = validate_token x.classish_name;
          classish_keyword = validate_token x.classish_keyword;
          classish_xhp = validate_option_with validate_token x.classish_xhp;
          classish_modifiers =
            validate_list_with validate_token x.classish_modifiers;
          classish_attribute =
            validate_option_with
              validate_attribute_specification
              x.classish_attribute;
        } )
    | s -> validation_fail (Some SyntaxKind.ClassishDeclaration) s

  and invalidate_classish_declaration : classish_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ClassishDeclaration
          {
            classish_attribute =
              invalidate_option_with
                invalidate_attribute_specification
                x.classish_attribute;
            classish_modifiers =
              invalidate_list_with invalidate_token x.classish_modifiers;
            classish_xhp =
              invalidate_option_with invalidate_token x.classish_xhp;
            classish_keyword = invalidate_token x.classish_keyword;
            classish_name = invalidate_token x.classish_name;
            classish_type_parameters =
              invalidate_option_with
                invalidate_type_parameters
                x.classish_type_parameters;
            classish_extends_keyword =
              invalidate_option_with invalidate_token x.classish_extends_keyword;
            classish_extends_list =
              invalidate_list_with invalidate_specifier x.classish_extends_list;
            classish_implements_keyword =
              invalidate_option_with
                invalidate_token
                x.classish_implements_keyword;
            classish_implements_list =
              invalidate_list_with
                invalidate_specifier
                x.classish_implements_list;
            classish_where_clause =
              invalidate_option_with
                invalidate_where_clause
                x.classish_where_clause;
            classish_body = invalidate_classish_body x.classish_body;
          };
      Syntax.value = v;
    }

  and validate_classish_body : classish_body validator = function
    | { Syntax.syntax = Syntax.ClassishBody x; value = v } ->
      ( v,
        {
          classish_body_right_brace = validate_token x.classish_body_right_brace;
          classish_body_elements =
            validate_list_with
              validate_class_body_declaration
              x.classish_body_elements;
          classish_body_left_brace = validate_token x.classish_body_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.ClassishBody) s

  and invalidate_classish_body : classish_body invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ClassishBody
          {
            classish_body_left_brace =
              invalidate_token x.classish_body_left_brace;
            classish_body_elements =
              invalidate_list_with
                invalidate_class_body_declaration
                x.classish_body_elements;
            classish_body_right_brace =
              invalidate_token x.classish_body_right_brace;
          };
      Syntax.value = v;
    }

  and validate_trait_use_precedence_item : trait_use_precedence_item validator =
    function
    | { Syntax.syntax = Syntax.TraitUsePrecedenceItem x; value = v } ->
      ( v,
        {
          trait_use_precedence_item_removed_names =
            validate_list_with
              validate_specifier
              x.trait_use_precedence_item_removed_names;
          trait_use_precedence_item_keyword =
            validate_token x.trait_use_precedence_item_keyword;
          trait_use_precedence_item_name =
            validate_specifier x.trait_use_precedence_item_name;
        } )
    | s -> validation_fail (Some SyntaxKind.TraitUsePrecedenceItem) s

  and invalidate_trait_use_precedence_item :
      trait_use_precedence_item invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TraitUsePrecedenceItem
          {
            trait_use_precedence_item_name =
              invalidate_specifier x.trait_use_precedence_item_name;
            trait_use_precedence_item_keyword =
              invalidate_token x.trait_use_precedence_item_keyword;
            trait_use_precedence_item_removed_names =
              invalidate_list_with
                invalidate_specifier
                x.trait_use_precedence_item_removed_names;
          };
      Syntax.value = v;
    }

  and validate_trait_use_alias_item : trait_use_alias_item validator = function
    | { Syntax.syntax = Syntax.TraitUseAliasItem x; value = v } ->
      ( v,
        {
          trait_use_alias_item_aliased_name =
            validate_option_with
              validate_specifier
              x.trait_use_alias_item_aliased_name;
          trait_use_alias_item_modifiers =
            validate_list_with validate_token x.trait_use_alias_item_modifiers;
          trait_use_alias_item_keyword =
            validate_token x.trait_use_alias_item_keyword;
          trait_use_alias_item_aliasing_name =
            validate_specifier x.trait_use_alias_item_aliasing_name;
        } )
    | s -> validation_fail (Some SyntaxKind.TraitUseAliasItem) s

  and invalidate_trait_use_alias_item : trait_use_alias_item invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TraitUseAliasItem
          {
            trait_use_alias_item_aliasing_name =
              invalidate_specifier x.trait_use_alias_item_aliasing_name;
            trait_use_alias_item_keyword =
              invalidate_token x.trait_use_alias_item_keyword;
            trait_use_alias_item_modifiers =
              invalidate_list_with
                invalidate_token
                x.trait_use_alias_item_modifiers;
            trait_use_alias_item_aliased_name =
              invalidate_option_with
                invalidate_specifier
                x.trait_use_alias_item_aliased_name;
          };
      Syntax.value = v;
    }

  and validate_trait_use_conflict_resolution :
      trait_use_conflict_resolution validator = function
    | { Syntax.syntax = Syntax.TraitUseConflictResolution x; value = v } ->
      ( v,
        {
          trait_use_conflict_resolution_right_brace =
            validate_token x.trait_use_conflict_resolution_right_brace;
          trait_use_conflict_resolution_clauses =
            validate_list_with
              validate_specifier
              x.trait_use_conflict_resolution_clauses;
          trait_use_conflict_resolution_left_brace =
            validate_token x.trait_use_conflict_resolution_left_brace;
          trait_use_conflict_resolution_names =
            validate_list_with
              validate_specifier
              x.trait_use_conflict_resolution_names;
          trait_use_conflict_resolution_keyword =
            validate_token x.trait_use_conflict_resolution_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TraitUseConflictResolution) s

  and invalidate_trait_use_conflict_resolution :
      trait_use_conflict_resolution invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TraitUseConflictResolution
          {
            trait_use_conflict_resolution_keyword =
              invalidate_token x.trait_use_conflict_resolution_keyword;
            trait_use_conflict_resolution_names =
              invalidate_list_with
                invalidate_specifier
                x.trait_use_conflict_resolution_names;
            trait_use_conflict_resolution_left_brace =
              invalidate_token x.trait_use_conflict_resolution_left_brace;
            trait_use_conflict_resolution_clauses =
              invalidate_list_with
                invalidate_specifier
                x.trait_use_conflict_resolution_clauses;
            trait_use_conflict_resolution_right_brace =
              invalidate_token x.trait_use_conflict_resolution_right_brace;
          };
      Syntax.value = v;
    }

  and validate_trait_use : trait_use validator = function
    | { Syntax.syntax = Syntax.TraitUse x; value = v } ->
      ( v,
        {
          trait_use_semicolon =
            validate_option_with validate_token x.trait_use_semicolon;
          trait_use_names =
            validate_list_with validate_specifier x.trait_use_names;
          trait_use_keyword = validate_token x.trait_use_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TraitUse) s

  and invalidate_trait_use : trait_use invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TraitUse
          {
            trait_use_keyword = invalidate_token x.trait_use_keyword;
            trait_use_names =
              invalidate_list_with invalidate_specifier x.trait_use_names;
            trait_use_semicolon =
              invalidate_option_with invalidate_token x.trait_use_semicolon;
          };
      Syntax.value = v;
    }

  and validate_require_clause : require_clause validator = function
    | { Syntax.syntax = Syntax.RequireClause x; value = v } ->
      ( v,
        {
          require_semicolon = validate_token x.require_semicolon;
          require_name = validate_specifier x.require_name;
          require_kind = validate_token x.require_kind;
          require_keyword = validate_token x.require_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.RequireClause) s

  and invalidate_require_clause : require_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.RequireClause
          {
            require_keyword = invalidate_token x.require_keyword;
            require_kind = invalidate_token x.require_kind;
            require_name = invalidate_specifier x.require_name;
            require_semicolon = invalidate_token x.require_semicolon;
          };
      Syntax.value = v;
    }

  and validate_const_declaration : const_declaration validator = function
    | { Syntax.syntax = Syntax.ConstDeclaration x; value = v } ->
      ( v,
        {
          const_semicolon = validate_token x.const_semicolon;
          const_declarators =
            validate_list_with validate_constant_declarator x.const_declarators;
          const_type_specifier =
            validate_option_with validate_specifier x.const_type_specifier;
          const_keyword = validate_token x.const_keyword;
          const_modifiers = validate_list_with validate_token x.const_modifiers;
        } )
    | s -> validation_fail (Some SyntaxKind.ConstDeclaration) s

  and invalidate_const_declaration : const_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ConstDeclaration
          {
            const_modifiers =
              invalidate_list_with invalidate_token x.const_modifiers;
            const_keyword = invalidate_token x.const_keyword;
            const_type_specifier =
              invalidate_option_with invalidate_specifier x.const_type_specifier;
            const_declarators =
              invalidate_list_with
                invalidate_constant_declarator
                x.const_declarators;
            const_semicolon = invalidate_token x.const_semicolon;
          };
      Syntax.value = v;
    }

  and validate_constant_declarator : constant_declarator validator = function
    | { Syntax.syntax = Syntax.ConstantDeclarator x; value = v } ->
      ( v,
        {
          constant_declarator_initializer =
            validate_option_with
              validate_simple_initializer
              x.constant_declarator_initializer;
          constant_declarator_name = validate_token x.constant_declarator_name;
        } )
    | s -> validation_fail (Some SyntaxKind.ConstantDeclarator) s

  and invalidate_constant_declarator : constant_declarator invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ConstantDeclarator
          {
            constant_declarator_name =
              invalidate_token x.constant_declarator_name;
            constant_declarator_initializer =
              invalidate_option_with
                invalidate_simple_initializer
                x.constant_declarator_initializer;
          };
      Syntax.value = v;
    }

  and validate_type_const_declaration : type_const_declaration validator =
    function
    | { Syntax.syntax = Syntax.TypeConstDeclaration x; value = v } ->
      ( v,
        {
          type_const_semicolon = validate_token x.type_const_semicolon;
          type_const_type_specifier =
            validate_option_with validate_specifier x.type_const_type_specifier;
          type_const_equal =
            validate_option_with validate_token x.type_const_equal;
          type_const_type_constraint =
            validate_option_with
              validate_type_constraint
              x.type_const_type_constraint;
          type_const_type_parameters =
            validate_option_with
              validate_type_parameters
              x.type_const_type_parameters;
          type_const_name = validate_token x.type_const_name;
          type_const_type_keyword = validate_token x.type_const_type_keyword;
          type_const_keyword = validate_token x.type_const_keyword;
          type_const_modifiers =
            validate_option_with validate_token x.type_const_modifiers;
          type_const_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.type_const_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeConstDeclaration) s

  and invalidate_type_const_declaration : type_const_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeConstDeclaration
          {
            type_const_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.type_const_attribute_spec;
            type_const_modifiers =
              invalidate_option_with invalidate_token x.type_const_modifiers;
            type_const_keyword = invalidate_token x.type_const_keyword;
            type_const_type_keyword = invalidate_token x.type_const_type_keyword;
            type_const_name = invalidate_token x.type_const_name;
            type_const_type_parameters =
              invalidate_option_with
                invalidate_type_parameters
                x.type_const_type_parameters;
            type_const_type_constraint =
              invalidate_option_with
                invalidate_type_constraint
                x.type_const_type_constraint;
            type_const_equal =
              invalidate_option_with invalidate_token x.type_const_equal;
            type_const_type_specifier =
              invalidate_option_with
                invalidate_specifier
                x.type_const_type_specifier;
            type_const_semicolon = invalidate_token x.type_const_semicolon;
          };
      Syntax.value = v;
    }

  and validate_context_const_declaration : context_const_declaration validator =
    function
    | { Syntax.syntax = Syntax.ContextConstDeclaration x; value = v } ->
      ( v,
        {
          context_const_semicolon = validate_token x.context_const_semicolon;
          context_const_ctx_list =
            validate_option_with validate_contexts x.context_const_ctx_list;
          context_const_equal =
            validate_option_with validate_token x.context_const_equal;
          context_const_constraint =
            validate_list_with
              validate_context_constraint
              x.context_const_constraint;
          context_const_type_parameters =
            validate_option_with
              validate_type_parameters
              x.context_const_type_parameters;
          context_const_name = validate_token x.context_const_name;
          context_const_ctx_keyword = validate_token x.context_const_ctx_keyword;
          context_const_const_keyword =
            validate_token x.context_const_const_keyword;
          context_const_modifiers =
            validate_option_with validate_token x.context_const_modifiers;
        } )
    | s -> validation_fail (Some SyntaxKind.ContextConstDeclaration) s

  and invalidate_context_const_declaration :
      context_const_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ContextConstDeclaration
          {
            context_const_modifiers =
              invalidate_option_with invalidate_token x.context_const_modifiers;
            context_const_const_keyword =
              invalidate_token x.context_const_const_keyword;
            context_const_ctx_keyword =
              invalidate_token x.context_const_ctx_keyword;
            context_const_name = invalidate_token x.context_const_name;
            context_const_type_parameters =
              invalidate_option_with
                invalidate_type_parameters
                x.context_const_type_parameters;
            context_const_constraint =
              invalidate_list_with
                invalidate_context_constraint
                x.context_const_constraint;
            context_const_equal =
              invalidate_option_with invalidate_token x.context_const_equal;
            context_const_ctx_list =
              invalidate_option_with
                invalidate_contexts
                x.context_const_ctx_list;
            context_const_semicolon = invalidate_token x.context_const_semicolon;
          };
      Syntax.value = v;
    }

  and validate_decorated_expression : decorated_expression validator = function
    | { Syntax.syntax = Syntax.DecoratedExpression x; value = v } ->
      ( v,
        {
          decorated_expression_expression =
            validate_expression x.decorated_expression_expression;
          decorated_expression_decorator =
            validate_token x.decorated_expression_decorator;
        } )
    | s -> validation_fail (Some SyntaxKind.DecoratedExpression) s

  and invalidate_decorated_expression : decorated_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DecoratedExpression
          {
            decorated_expression_decorator =
              invalidate_token x.decorated_expression_decorator;
            decorated_expression_expression =
              invalidate_expression x.decorated_expression_expression;
          };
      Syntax.value = v;
    }

  and validate_parameter_declaration : parameter_declaration validator =
    function
    | { Syntax.syntax = Syntax.ParameterDeclaration x; value = v } ->
      ( v,
        {
          parameter_default_value =
            validate_option_with
              validate_simple_initializer
              x.parameter_default_value;
          parameter_name = validate_expression x.parameter_name;
          parameter_type =
            validate_option_with validate_specifier x.parameter_type;
          parameter_readonly =
            validate_option_with validate_token x.parameter_readonly;
          parameter_call_convention =
            validate_option_with validate_token x.parameter_call_convention;
          parameter_visibility =
            validate_option_with validate_token x.parameter_visibility;
          parameter_attribute =
            validate_option_with
              validate_attribute_specification
              x.parameter_attribute;
        } )
    | s -> validation_fail (Some SyntaxKind.ParameterDeclaration) s

  and invalidate_parameter_declaration : parameter_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ParameterDeclaration
          {
            parameter_attribute =
              invalidate_option_with
                invalidate_attribute_specification
                x.parameter_attribute;
            parameter_visibility =
              invalidate_option_with invalidate_token x.parameter_visibility;
            parameter_call_convention =
              invalidate_option_with
                invalidate_token
                x.parameter_call_convention;
            parameter_readonly =
              invalidate_option_with invalidate_token x.parameter_readonly;
            parameter_type =
              invalidate_option_with invalidate_specifier x.parameter_type;
            parameter_name = invalidate_expression x.parameter_name;
            parameter_default_value =
              invalidate_option_with
                invalidate_simple_initializer
                x.parameter_default_value;
          };
      Syntax.value = v;
    }

  and validate_variadic_parameter : variadic_parameter validator = function
    | { Syntax.syntax = Syntax.VariadicParameter x; value = v } ->
      ( v,
        {
          variadic_parameter_ellipsis =
            validate_token x.variadic_parameter_ellipsis;
          variadic_parameter_type =
            validate_option_with
              validate_simple_type_specifier
              x.variadic_parameter_type;
          variadic_parameter_call_convention =
            validate_option_with
              validate_token
              x.variadic_parameter_call_convention;
        } )
    | s -> validation_fail (Some SyntaxKind.VariadicParameter) s

  and invalidate_variadic_parameter : variadic_parameter invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VariadicParameter
          {
            variadic_parameter_call_convention =
              invalidate_option_with
                invalidate_token
                x.variadic_parameter_call_convention;
            variadic_parameter_type =
              invalidate_option_with
                invalidate_simple_type_specifier
                x.variadic_parameter_type;
            variadic_parameter_ellipsis =
              invalidate_token x.variadic_parameter_ellipsis;
          };
      Syntax.value = v;
    }

  and validate_old_attribute_specification :
      old_attribute_specification validator = function
    | { Syntax.syntax = Syntax.OldAttributeSpecification x; value = v } ->
      ( v,
        {
          old_attribute_specification_right_double_angle =
            validate_token x.old_attribute_specification_right_double_angle;
          old_attribute_specification_attributes =
            validate_list_with
              validate_constructor_call
              x.old_attribute_specification_attributes;
          old_attribute_specification_left_double_angle =
            validate_token x.old_attribute_specification_left_double_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.OldAttributeSpecification) s

  and invalidate_old_attribute_specification :
      old_attribute_specification invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle =
              invalidate_token x.old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes =
              invalidate_list_with
                invalidate_constructor_call
                x.old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle =
              invalidate_token x.old_attribute_specification_right_double_angle;
          };
      Syntax.value = v;
    }

  and validate_attribute_specification : attribute_specification validator =
    function
    | { Syntax.syntax = Syntax.AttributeSpecification x; value = v } ->
      ( v,
        {
          attribute_specification_attributes =
            validate_list_with
              validate_attribute
              x.attribute_specification_attributes;
        } )
    | s -> validation_fail (Some SyntaxKind.AttributeSpecification) s

  and invalidate_attribute_specification : attribute_specification invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AttributeSpecification
          {
            attribute_specification_attributes =
              invalidate_list_with
                invalidate_attribute
                x.attribute_specification_attributes;
          };
      Syntax.value = v;
    }

  and validate_attribute : attribute validator = function
    | { Syntax.syntax = Syntax.Attribute x; value = v } ->
      ( v,
        {
          attribute_attribute_name =
            validate_constructor_call x.attribute_attribute_name;
          attribute_at = validate_token x.attribute_at;
        } )
    | s -> validation_fail (Some SyntaxKind.Attribute) s

  and invalidate_attribute : attribute invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.Attribute
          {
            attribute_at = invalidate_token x.attribute_at;
            attribute_attribute_name =
              invalidate_constructor_call x.attribute_attribute_name;
          };
      Syntax.value = v;
    }

  and validate_inclusion_expression : inclusion_expression validator = function
    | { Syntax.syntax = Syntax.InclusionExpression x; value = v } ->
      ( v,
        {
          inclusion_filename = validate_expression x.inclusion_filename;
          inclusion_require = validate_token x.inclusion_require;
        } )
    | s -> validation_fail (Some SyntaxKind.InclusionExpression) s

  and invalidate_inclusion_expression : inclusion_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.InclusionExpression
          {
            inclusion_require = invalidate_token x.inclusion_require;
            inclusion_filename = invalidate_expression x.inclusion_filename;
          };
      Syntax.value = v;
    }

  and validate_inclusion_directive : inclusion_directive validator = function
    | { Syntax.syntax = Syntax.InclusionDirective x; value = v } ->
      ( v,
        {
          inclusion_semicolon = validate_token x.inclusion_semicolon;
          inclusion_expression =
            validate_inclusion_expression x.inclusion_expression;
        } )
    | s -> validation_fail (Some SyntaxKind.InclusionDirective) s

  and invalidate_inclusion_directive : inclusion_directive invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.InclusionDirective
          {
            inclusion_expression =
              invalidate_inclusion_expression x.inclusion_expression;
            inclusion_semicolon = invalidate_token x.inclusion_semicolon;
          };
      Syntax.value = v;
    }

  and validate_compound_statement : compound_statement validator = function
    | { Syntax.syntax = Syntax.CompoundStatement x; value = v } ->
      ( v,
        {
          compound_right_brace = validate_token x.compound_right_brace;
          compound_statements =
            validate_list_with validate_statement x.compound_statements;
          compound_left_brace = validate_token x.compound_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.CompoundStatement) s

  and invalidate_compound_statement : compound_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.CompoundStatement
          {
            compound_left_brace = invalidate_token x.compound_left_brace;
            compound_statements =
              invalidate_list_with invalidate_statement x.compound_statements;
            compound_right_brace = invalidate_token x.compound_right_brace;
          };
      Syntax.value = v;
    }

  and validate_expression_statement : expression_statement validator = function
    | { Syntax.syntax = Syntax.ExpressionStatement x; value = v } ->
      ( v,
        {
          expression_statement_semicolon =
            validate_token x.expression_statement_semicolon;
          expression_statement_expression =
            validate_option_with
              validate_expression
              x.expression_statement_expression;
        } )
    | s -> validation_fail (Some SyntaxKind.ExpressionStatement) s

  and invalidate_expression_statement : expression_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ExpressionStatement
          {
            expression_statement_expression =
              invalidate_option_with
                invalidate_expression
                x.expression_statement_expression;
            expression_statement_semicolon =
              invalidate_token x.expression_statement_semicolon;
          };
      Syntax.value = v;
    }

  and validate_markup_section : markup_section validator = function
    | { Syntax.syntax = Syntax.MarkupSection x; value = v } ->
      ( v,
        {
          markup_suffix =
            validate_option_with validate_markup_suffix x.markup_suffix;
          markup_hashbang = validate_token x.markup_hashbang;
        } )
    | s -> validation_fail (Some SyntaxKind.MarkupSection) s

  and invalidate_markup_section : markup_section invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.MarkupSection
          {
            markup_hashbang = invalidate_token x.markup_hashbang;
            markup_suffix =
              invalidate_option_with invalidate_markup_suffix x.markup_suffix;
          };
      Syntax.value = v;
    }

  and validate_markup_suffix : markup_suffix validator = function
    | { Syntax.syntax = Syntax.MarkupSuffix x; value = v } ->
      ( v,
        {
          markup_suffix_name =
            validate_option_with validate_token x.markup_suffix_name;
          markup_suffix_less_than_question =
            validate_token x.markup_suffix_less_than_question;
        } )
    | s -> validation_fail (Some SyntaxKind.MarkupSuffix) s

  and invalidate_markup_suffix : markup_suffix invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.MarkupSuffix
          {
            markup_suffix_less_than_question =
              invalidate_token x.markup_suffix_less_than_question;
            markup_suffix_name =
              invalidate_option_with invalidate_token x.markup_suffix_name;
          };
      Syntax.value = v;
    }

  and validate_unset_statement : unset_statement validator = function
    | { Syntax.syntax = Syntax.UnsetStatement x; value = v } ->
      ( v,
        {
          unset_semicolon = validate_token x.unset_semicolon;
          unset_right_paren = validate_token x.unset_right_paren;
          unset_variables =
            validate_list_with validate_expression x.unset_variables;
          unset_left_paren = validate_token x.unset_left_paren;
          unset_keyword = validate_token x.unset_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.UnsetStatement) s

  and invalidate_unset_statement : unset_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.UnsetStatement
          {
            unset_keyword = invalidate_token x.unset_keyword;
            unset_left_paren = invalidate_token x.unset_left_paren;
            unset_variables =
              invalidate_list_with invalidate_expression x.unset_variables;
            unset_right_paren = invalidate_token x.unset_right_paren;
            unset_semicolon = invalidate_token x.unset_semicolon;
          };
      Syntax.value = v;
    }

  and validate_using_statement_block_scoped :
      using_statement_block_scoped validator = function
    | { Syntax.syntax = Syntax.UsingStatementBlockScoped x; value = v } ->
      ( v,
        {
          using_block_body = validate_statement x.using_block_body;
          using_block_right_paren = validate_token x.using_block_right_paren;
          using_block_expressions =
            validate_list_with validate_expression x.using_block_expressions;
          using_block_left_paren = validate_token x.using_block_left_paren;
          using_block_using_keyword = validate_token x.using_block_using_keyword;
          using_block_await_keyword =
            validate_option_with validate_token x.using_block_await_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.UsingStatementBlockScoped) s

  and invalidate_using_statement_block_scoped :
      using_statement_block_scoped invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.UsingStatementBlockScoped
          {
            using_block_await_keyword =
              invalidate_option_with
                invalidate_token
                x.using_block_await_keyword;
            using_block_using_keyword =
              invalidate_token x.using_block_using_keyword;
            using_block_left_paren = invalidate_token x.using_block_left_paren;
            using_block_expressions =
              invalidate_list_with
                invalidate_expression
                x.using_block_expressions;
            using_block_right_paren = invalidate_token x.using_block_right_paren;
            using_block_body = invalidate_statement x.using_block_body;
          };
      Syntax.value = v;
    }

  and validate_using_statement_function_scoped :
      using_statement_function_scoped validator = function
    | { Syntax.syntax = Syntax.UsingStatementFunctionScoped x; value = v } ->
      ( v,
        {
          using_function_semicolon = validate_token x.using_function_semicolon;
          using_function_expression =
            validate_expression x.using_function_expression;
          using_function_using_keyword =
            validate_token x.using_function_using_keyword;
          using_function_await_keyword =
            validate_option_with validate_token x.using_function_await_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.UsingStatementFunctionScoped) s

  and invalidate_using_statement_function_scoped :
      using_statement_function_scoped invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.UsingStatementFunctionScoped
          {
            using_function_await_keyword =
              invalidate_option_with
                invalidate_token
                x.using_function_await_keyword;
            using_function_using_keyword =
              invalidate_token x.using_function_using_keyword;
            using_function_expression =
              invalidate_expression x.using_function_expression;
            using_function_semicolon =
              invalidate_token x.using_function_semicolon;
          };
      Syntax.value = v;
    }

  and validate_while_statement : while_statement validator = function
    | { Syntax.syntax = Syntax.WhileStatement x; value = v } ->
      ( v,
        {
          while_body = validate_statement x.while_body;
          while_right_paren = validate_token x.while_right_paren;
          while_condition = validate_expression x.while_condition;
          while_left_paren = validate_token x.while_left_paren;
          while_keyword = validate_token x.while_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.WhileStatement) s

  and invalidate_while_statement : while_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.WhileStatement
          {
            while_keyword = invalidate_token x.while_keyword;
            while_left_paren = invalidate_token x.while_left_paren;
            while_condition = invalidate_expression x.while_condition;
            while_right_paren = invalidate_token x.while_right_paren;
            while_body = invalidate_statement x.while_body;
          };
      Syntax.value = v;
    }

  and validate_if_statement : if_statement validator = function
    | { Syntax.syntax = Syntax.IfStatement x; value = v } ->
      ( v,
        {
          if_else_clause =
            validate_option_with validate_else_clause x.if_else_clause;
          if_elseif_clauses =
            validate_list_with validate_elseif_clause x.if_elseif_clauses;
          if_statement = validate_statement x.if_statement;
          if_right_paren = validate_token x.if_right_paren;
          if_condition = validate_expression x.if_condition;
          if_left_paren = validate_token x.if_left_paren;
          if_keyword = validate_token x.if_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.IfStatement) s

  and invalidate_if_statement : if_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.IfStatement
          {
            if_keyword = invalidate_token x.if_keyword;
            if_left_paren = invalidate_token x.if_left_paren;
            if_condition = invalidate_expression x.if_condition;
            if_right_paren = invalidate_token x.if_right_paren;
            if_statement = invalidate_statement x.if_statement;
            if_elseif_clauses =
              invalidate_list_with invalidate_elseif_clause x.if_elseif_clauses;
            if_else_clause =
              invalidate_option_with invalidate_else_clause x.if_else_clause;
          };
      Syntax.value = v;
    }

  and validate_elseif_clause : elseif_clause validator = function
    | { Syntax.syntax = Syntax.ElseifClause x; value = v } ->
      ( v,
        {
          elseif_statement = validate_statement x.elseif_statement;
          elseif_right_paren = validate_token x.elseif_right_paren;
          elseif_condition = validate_expression x.elseif_condition;
          elseif_left_paren = validate_token x.elseif_left_paren;
          elseif_keyword = validate_token x.elseif_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ElseifClause) s

  and invalidate_elseif_clause : elseif_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ElseifClause
          {
            elseif_keyword = invalidate_token x.elseif_keyword;
            elseif_left_paren = invalidate_token x.elseif_left_paren;
            elseif_condition = invalidate_expression x.elseif_condition;
            elseif_right_paren = invalidate_token x.elseif_right_paren;
            elseif_statement = invalidate_statement x.elseif_statement;
          };
      Syntax.value = v;
    }

  and validate_else_clause : else_clause validator = function
    | { Syntax.syntax = Syntax.ElseClause x; value = v } ->
      ( v,
        {
          else_statement = validate_statement x.else_statement;
          else_keyword = validate_token x.else_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ElseClause) s

  and invalidate_else_clause : else_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ElseClause
          {
            else_keyword = invalidate_token x.else_keyword;
            else_statement = invalidate_statement x.else_statement;
          };
      Syntax.value = v;
    }

  and validate_try_statement : try_statement validator = function
    | { Syntax.syntax = Syntax.TryStatement x; value = v } ->
      ( v,
        {
          try_finally_clause =
            validate_option_with validate_finally_clause x.try_finally_clause;
          try_catch_clauses =
            validate_list_with validate_catch_clause x.try_catch_clauses;
          try_compound_statement =
            validate_compound_statement x.try_compound_statement;
          try_keyword = validate_token x.try_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TryStatement) s

  and invalidate_try_statement : try_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TryStatement
          {
            try_keyword = invalidate_token x.try_keyword;
            try_compound_statement =
              invalidate_compound_statement x.try_compound_statement;
            try_catch_clauses =
              invalidate_list_with invalidate_catch_clause x.try_catch_clauses;
            try_finally_clause =
              invalidate_option_with
                invalidate_finally_clause
                x.try_finally_clause;
          };
      Syntax.value = v;
    }

  and validate_catch_clause : catch_clause validator = function
    | { Syntax.syntax = Syntax.CatchClause x; value = v } ->
      ( v,
        {
          catch_body = validate_compound_statement x.catch_body;
          catch_right_paren = validate_token x.catch_right_paren;
          catch_variable = validate_token x.catch_variable;
          catch_type = validate_simple_type_specifier x.catch_type;
          catch_left_paren = validate_token x.catch_left_paren;
          catch_keyword = validate_token x.catch_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.CatchClause) s

  and invalidate_catch_clause : catch_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.CatchClause
          {
            catch_keyword = invalidate_token x.catch_keyword;
            catch_left_paren = invalidate_token x.catch_left_paren;
            catch_type = invalidate_simple_type_specifier x.catch_type;
            catch_variable = invalidate_token x.catch_variable;
            catch_right_paren = invalidate_token x.catch_right_paren;
            catch_body = invalidate_compound_statement x.catch_body;
          };
      Syntax.value = v;
    }

  and validate_finally_clause : finally_clause validator = function
    | { Syntax.syntax = Syntax.FinallyClause x; value = v } ->
      ( v,
        {
          finally_body = validate_compound_statement x.finally_body;
          finally_keyword = validate_token x.finally_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.FinallyClause) s

  and invalidate_finally_clause : finally_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FinallyClause
          {
            finally_keyword = invalidate_token x.finally_keyword;
            finally_body = invalidate_compound_statement x.finally_body;
          };
      Syntax.value = v;
    }

  and validate_do_statement : do_statement validator = function
    | { Syntax.syntax = Syntax.DoStatement x; value = v } ->
      ( v,
        {
          do_semicolon = validate_token x.do_semicolon;
          do_right_paren = validate_token x.do_right_paren;
          do_condition = validate_expression x.do_condition;
          do_left_paren = validate_token x.do_left_paren;
          do_while_keyword = validate_token x.do_while_keyword;
          do_body = validate_statement x.do_body;
          do_keyword = validate_token x.do_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DoStatement) s

  and invalidate_do_statement : do_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DoStatement
          {
            do_keyword = invalidate_token x.do_keyword;
            do_body = invalidate_statement x.do_body;
            do_while_keyword = invalidate_token x.do_while_keyword;
            do_left_paren = invalidate_token x.do_left_paren;
            do_condition = invalidate_expression x.do_condition;
            do_right_paren = invalidate_token x.do_right_paren;
            do_semicolon = invalidate_token x.do_semicolon;
          };
      Syntax.value = v;
    }

  and validate_for_statement : for_statement validator = function
    | { Syntax.syntax = Syntax.ForStatement x; value = v } ->
      ( v,
        {
          for_body = validate_statement x.for_body;
          for_right_paren = validate_token x.for_right_paren;
          for_end_of_loop =
            validate_list_with validate_expression x.for_end_of_loop;
          for_second_semicolon = validate_token x.for_second_semicolon;
          for_control = validate_list_with validate_expression x.for_control;
          for_first_semicolon = validate_token x.for_first_semicolon;
          for_initializer =
            validate_list_with validate_expression x.for_initializer;
          for_left_paren = validate_token x.for_left_paren;
          for_keyword = validate_token x.for_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ForStatement) s

  and invalidate_for_statement : for_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ForStatement
          {
            for_keyword = invalidate_token x.for_keyword;
            for_left_paren = invalidate_token x.for_left_paren;
            for_initializer =
              invalidate_list_with invalidate_expression x.for_initializer;
            for_first_semicolon = invalidate_token x.for_first_semicolon;
            for_control =
              invalidate_list_with invalidate_expression x.for_control;
            for_second_semicolon = invalidate_token x.for_second_semicolon;
            for_end_of_loop =
              invalidate_list_with invalidate_expression x.for_end_of_loop;
            for_right_paren = invalidate_token x.for_right_paren;
            for_body = invalidate_statement x.for_body;
          };
      Syntax.value = v;
    }

  and validate_foreach_statement : foreach_statement validator = function
    | { Syntax.syntax = Syntax.ForeachStatement x; value = v } ->
      ( v,
        {
          foreach_body = validate_statement x.foreach_body;
          foreach_right_paren = validate_token x.foreach_right_paren;
          foreach_value = validate_expression x.foreach_value;
          foreach_arrow = validate_option_with validate_token x.foreach_arrow;
          foreach_key = validate_option_with validate_expression x.foreach_key;
          foreach_as = validate_token x.foreach_as;
          foreach_await_keyword =
            validate_option_with validate_token x.foreach_await_keyword;
          foreach_collection = validate_expression x.foreach_collection;
          foreach_left_paren = validate_token x.foreach_left_paren;
          foreach_keyword = validate_token x.foreach_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ForeachStatement) s

  and invalidate_foreach_statement : foreach_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ForeachStatement
          {
            foreach_keyword = invalidate_token x.foreach_keyword;
            foreach_left_paren = invalidate_token x.foreach_left_paren;
            foreach_collection = invalidate_expression x.foreach_collection;
            foreach_await_keyword =
              invalidate_option_with invalidate_token x.foreach_await_keyword;
            foreach_as = invalidate_token x.foreach_as;
            foreach_key =
              invalidate_option_with invalidate_expression x.foreach_key;
            foreach_arrow =
              invalidate_option_with invalidate_token x.foreach_arrow;
            foreach_value = invalidate_expression x.foreach_value;
            foreach_right_paren = invalidate_token x.foreach_right_paren;
            foreach_body = invalidate_statement x.foreach_body;
          };
      Syntax.value = v;
    }

  and validate_switch_statement : switch_statement validator = function
    | { Syntax.syntax = Syntax.SwitchStatement x; value = v } ->
      ( v,
        {
          switch_right_brace = validate_token x.switch_right_brace;
          switch_sections =
            validate_list_with validate_switch_section x.switch_sections;
          switch_left_brace = validate_token x.switch_left_brace;
          switch_right_paren = validate_token x.switch_right_paren;
          switch_expression = validate_expression x.switch_expression;
          switch_left_paren = validate_token x.switch_left_paren;
          switch_keyword = validate_token x.switch_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.SwitchStatement) s

  and invalidate_switch_statement : switch_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SwitchStatement
          {
            switch_keyword = invalidate_token x.switch_keyword;
            switch_left_paren = invalidate_token x.switch_left_paren;
            switch_expression = invalidate_expression x.switch_expression;
            switch_right_paren = invalidate_token x.switch_right_paren;
            switch_left_brace = invalidate_token x.switch_left_brace;
            switch_sections =
              invalidate_list_with invalidate_switch_section x.switch_sections;
            switch_right_brace = invalidate_token x.switch_right_brace;
          };
      Syntax.value = v;
    }

  and validate_switch_section : switch_section validator = function
    | { Syntax.syntax = Syntax.SwitchSection x; value = v } ->
      ( v,
        {
          switch_section_fallthrough =
            validate_option_with
              validate_switch_fallthrough
              x.switch_section_fallthrough;
          switch_section_statements =
            validate_list_with
              validate_top_level_declaration
              x.switch_section_statements;
          switch_section_labels =
            validate_list_with validate_switch_label x.switch_section_labels;
        } )
    | s -> validation_fail (Some SyntaxKind.SwitchSection) s

  and invalidate_switch_section : switch_section invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SwitchSection
          {
            switch_section_labels =
              invalidate_list_with
                invalidate_switch_label
                x.switch_section_labels;
            switch_section_statements =
              invalidate_list_with
                invalidate_top_level_declaration
                x.switch_section_statements;
            switch_section_fallthrough =
              invalidate_option_with
                invalidate_switch_fallthrough
                x.switch_section_fallthrough;
          };
      Syntax.value = v;
    }

  and validate_switch_fallthrough : switch_fallthrough validator = function
    | { Syntax.syntax = Syntax.SwitchFallthrough x; value = v } ->
      ( v,
        {
          fallthrough_semicolon = validate_token x.fallthrough_semicolon;
          fallthrough_keyword = validate_token x.fallthrough_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.SwitchFallthrough) s

  and invalidate_switch_fallthrough : switch_fallthrough invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SwitchFallthrough
          {
            fallthrough_keyword = invalidate_token x.fallthrough_keyword;
            fallthrough_semicolon = invalidate_token x.fallthrough_semicolon;
          };
      Syntax.value = v;
    }

  and validate_case_label : case_label validator = function
    | { Syntax.syntax = Syntax.CaseLabel x; value = v } ->
      ( v,
        {
          case_colon = validate_token x.case_colon;
          case_expression = validate_expression x.case_expression;
          case_keyword = validate_token x.case_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.CaseLabel) s

  and invalidate_case_label : case_label invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.CaseLabel
          {
            case_keyword = invalidate_token x.case_keyword;
            case_expression = invalidate_expression x.case_expression;
            case_colon = invalidate_token x.case_colon;
          };
      Syntax.value = v;
    }

  and validate_default_label : default_label validator = function
    | { Syntax.syntax = Syntax.DefaultLabel x; value = v } ->
      ( v,
        {
          default_colon = validate_token x.default_colon;
          default_keyword = validate_token x.default_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DefaultLabel) s

  and invalidate_default_label : default_label invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DefaultLabel
          {
            default_keyword = invalidate_token x.default_keyword;
            default_colon = invalidate_token x.default_colon;
          };
      Syntax.value = v;
    }

  and validate_return_statement : return_statement validator = function
    | { Syntax.syntax = Syntax.ReturnStatement x; value = v } ->
      ( v,
        {
          return_semicolon =
            validate_option_with validate_token x.return_semicolon;
          return_expression =
            validate_option_with validate_expression x.return_expression;
          return_keyword = validate_token x.return_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ReturnStatement) s

  and invalidate_return_statement : return_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ReturnStatement
          {
            return_keyword = invalidate_token x.return_keyword;
            return_expression =
              invalidate_option_with invalidate_expression x.return_expression;
            return_semicolon =
              invalidate_option_with invalidate_token x.return_semicolon;
          };
      Syntax.value = v;
    }

  and validate_yield_break_statement : yield_break_statement validator =
    function
    | { Syntax.syntax = Syntax.YieldBreakStatement x; value = v } ->
      ( v,
        {
          yield_break_semicolon = validate_token x.yield_break_semicolon;
          yield_break_break = validate_token x.yield_break_break;
          yield_break_keyword = validate_token x.yield_break_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.YieldBreakStatement) s

  and invalidate_yield_break_statement : yield_break_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.YieldBreakStatement
          {
            yield_break_keyword = invalidate_token x.yield_break_keyword;
            yield_break_break = invalidate_token x.yield_break_break;
            yield_break_semicolon = invalidate_token x.yield_break_semicolon;
          };
      Syntax.value = v;
    }

  and validate_throw_statement : throw_statement validator = function
    | { Syntax.syntax = Syntax.ThrowStatement x; value = v } ->
      ( v,
        {
          throw_semicolon = validate_token x.throw_semicolon;
          throw_expression = validate_expression x.throw_expression;
          throw_keyword = validate_token x.throw_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ThrowStatement) s

  and invalidate_throw_statement : throw_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ThrowStatement
          {
            throw_keyword = invalidate_token x.throw_keyword;
            throw_expression = invalidate_expression x.throw_expression;
            throw_semicolon = invalidate_token x.throw_semicolon;
          };
      Syntax.value = v;
    }

  and validate_break_statement : break_statement validator = function
    | { Syntax.syntax = Syntax.BreakStatement x; value = v } ->
      ( v,
        {
          break_semicolon = validate_token x.break_semicolon;
          break_keyword = validate_token x.break_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.BreakStatement) s

  and invalidate_break_statement : break_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.BreakStatement
          {
            break_keyword = invalidate_token x.break_keyword;
            break_semicolon = invalidate_token x.break_semicolon;
          };
      Syntax.value = v;
    }

  and validate_continue_statement : continue_statement validator = function
    | { Syntax.syntax = Syntax.ContinueStatement x; value = v } ->
      ( v,
        {
          continue_semicolon = validate_token x.continue_semicolon;
          continue_keyword = validate_token x.continue_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ContinueStatement) s

  and invalidate_continue_statement : continue_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ContinueStatement
          {
            continue_keyword = invalidate_token x.continue_keyword;
            continue_semicolon = invalidate_token x.continue_semicolon;
          };
      Syntax.value = v;
    }

  and validate_echo_statement : echo_statement validator = function
    | { Syntax.syntax = Syntax.EchoStatement x; value = v } ->
      ( v,
        {
          echo_semicolon = validate_token x.echo_semicolon;
          echo_expressions =
            validate_list_with validate_expression x.echo_expressions;
          echo_keyword = validate_token x.echo_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.EchoStatement) s

  and invalidate_echo_statement : echo_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EchoStatement
          {
            echo_keyword = invalidate_token x.echo_keyword;
            echo_expressions =
              invalidate_list_with invalidate_expression x.echo_expressions;
            echo_semicolon = invalidate_token x.echo_semicolon;
          };
      Syntax.value = v;
    }

  and validate_concurrent_statement : concurrent_statement validator = function
    | { Syntax.syntax = Syntax.ConcurrentStatement x; value = v } ->
      ( v,
        {
          concurrent_statement = validate_statement x.concurrent_statement;
          concurrent_keyword = validate_token x.concurrent_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ConcurrentStatement) s

  and invalidate_concurrent_statement : concurrent_statement invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ConcurrentStatement
          {
            concurrent_keyword = invalidate_token x.concurrent_keyword;
            concurrent_statement = invalidate_statement x.concurrent_statement;
          };
      Syntax.value = v;
    }

  and validate_simple_initializer : simple_initializer validator = function
    | { Syntax.syntax = Syntax.SimpleInitializer x; value = v } ->
      ( v,
        {
          simple_initializer_value =
            validate_expression x.simple_initializer_value;
          simple_initializer_equal = validate_token x.simple_initializer_equal;
        } )
    | s -> validation_fail (Some SyntaxKind.SimpleInitializer) s

  and invalidate_simple_initializer : simple_initializer invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SimpleInitializer
          {
            simple_initializer_equal =
              invalidate_token x.simple_initializer_equal;
            simple_initializer_value =
              invalidate_expression x.simple_initializer_value;
          };
      Syntax.value = v;
    }

  and validate_anonymous_class : anonymous_class validator = function
    | { Syntax.syntax = Syntax.AnonymousClass x; value = v } ->
      ( v,
        {
          anonymous_class_body = validate_classish_body x.anonymous_class_body;
          anonymous_class_implements_list =
            validate_list_with
              validate_specifier
              x.anonymous_class_implements_list;
          anonymous_class_implements_keyword =
            validate_option_with
              validate_token
              x.anonymous_class_implements_keyword;
          anonymous_class_extends_list =
            validate_list_with validate_specifier x.anonymous_class_extends_list;
          anonymous_class_extends_keyword =
            validate_option_with
              validate_token
              x.anonymous_class_extends_keyword;
          anonymous_class_right_paren =
            validate_option_with validate_token x.anonymous_class_right_paren;
          anonymous_class_argument_list =
            validate_list_with
              validate_expression
              x.anonymous_class_argument_list;
          anonymous_class_left_paren =
            validate_option_with validate_token x.anonymous_class_left_paren;
          anonymous_class_class_keyword =
            validate_token x.anonymous_class_class_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.AnonymousClass) s

  and invalidate_anonymous_class : anonymous_class invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AnonymousClass
          {
            anonymous_class_class_keyword =
              invalidate_token x.anonymous_class_class_keyword;
            anonymous_class_left_paren =
              invalidate_option_with
                invalidate_token
                x.anonymous_class_left_paren;
            anonymous_class_argument_list =
              invalidate_list_with
                invalidate_expression
                x.anonymous_class_argument_list;
            anonymous_class_right_paren =
              invalidate_option_with
                invalidate_token
                x.anonymous_class_right_paren;
            anonymous_class_extends_keyword =
              invalidate_option_with
                invalidate_token
                x.anonymous_class_extends_keyword;
            anonymous_class_extends_list =
              invalidate_list_with
                invalidate_specifier
                x.anonymous_class_extends_list;
            anonymous_class_implements_keyword =
              invalidate_option_with
                invalidate_token
                x.anonymous_class_implements_keyword;
            anonymous_class_implements_list =
              invalidate_list_with
                invalidate_specifier
                x.anonymous_class_implements_list;
            anonymous_class_body =
              invalidate_classish_body x.anonymous_class_body;
          };
      Syntax.value = v;
    }

  and validate_anonymous_function : anonymous_function validator = function
    | { Syntax.syntax = Syntax.AnonymousFunction x; value = v } ->
      ( v,
        {
          anonymous_body = validate_compound_statement x.anonymous_body;
          anonymous_use =
            validate_option_with
              validate_anonymous_function_use_clause
              x.anonymous_use;
          anonymous_type =
            validate_option_with validate_specifier x.anonymous_type;
          anonymous_readonly_return =
            validate_option_with validate_token x.anonymous_readonly_return;
          anonymous_colon =
            validate_option_with validate_token x.anonymous_colon;
          anonymous_ctx_list =
            validate_option_with validate_contexts x.anonymous_ctx_list;
          anonymous_right_paren = validate_token x.anonymous_right_paren;
          anonymous_parameters =
            validate_list_with validate_parameter x.anonymous_parameters;
          anonymous_left_paren = validate_token x.anonymous_left_paren;
          anonymous_function_keyword =
            validate_token x.anonymous_function_keyword;
          anonymous_async_keyword =
            validate_option_with validate_token x.anonymous_async_keyword;
          anonymous_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.anonymous_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.AnonymousFunction) s

  and invalidate_anonymous_function : anonymous_function invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AnonymousFunction
          {
            anonymous_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.anonymous_attribute_spec;
            anonymous_async_keyword =
              invalidate_option_with invalidate_token x.anonymous_async_keyword;
            anonymous_function_keyword =
              invalidate_token x.anonymous_function_keyword;
            anonymous_left_paren = invalidate_token x.anonymous_left_paren;
            anonymous_parameters =
              invalidate_list_with invalidate_parameter x.anonymous_parameters;
            anonymous_right_paren = invalidate_token x.anonymous_right_paren;
            anonymous_ctx_list =
              invalidate_option_with invalidate_contexts x.anonymous_ctx_list;
            anonymous_colon =
              invalidate_option_with invalidate_token x.anonymous_colon;
            anonymous_readonly_return =
              invalidate_option_with
                invalidate_token
                x.anonymous_readonly_return;
            anonymous_type =
              invalidate_option_with invalidate_specifier x.anonymous_type;
            anonymous_use =
              invalidate_option_with
                invalidate_anonymous_function_use_clause
                x.anonymous_use;
            anonymous_body = invalidate_compound_statement x.anonymous_body;
          };
      Syntax.value = v;
    }

  and validate_anonymous_function_use_clause :
      anonymous_function_use_clause validator = function
    | { Syntax.syntax = Syntax.AnonymousFunctionUseClause x; value = v } ->
      ( v,
        {
          anonymous_use_right_paren = validate_token x.anonymous_use_right_paren;
          anonymous_use_variables =
            validate_list_with validate_expression x.anonymous_use_variables;
          anonymous_use_left_paren = validate_token x.anonymous_use_left_paren;
          anonymous_use_keyword = validate_token x.anonymous_use_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.AnonymousFunctionUseClause) s

  and invalidate_anonymous_function_use_clause :
      anonymous_function_use_clause invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AnonymousFunctionUseClause
          {
            anonymous_use_keyword = invalidate_token x.anonymous_use_keyword;
            anonymous_use_left_paren =
              invalidate_token x.anonymous_use_left_paren;
            anonymous_use_variables =
              invalidate_list_with
                invalidate_expression
                x.anonymous_use_variables;
            anonymous_use_right_paren =
              invalidate_token x.anonymous_use_right_paren;
          };
      Syntax.value = v;
    }

  and validate_lambda_expression : lambda_expression validator = function
    | { Syntax.syntax = Syntax.LambdaExpression x; value = v } ->
      ( v,
        {
          lambda_body = validate_lambda_body x.lambda_body;
          lambda_arrow = validate_token x.lambda_arrow;
          lambda_signature = validate_specifier x.lambda_signature;
          lambda_async = validate_option_with validate_token x.lambda_async;
          lambda_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.lambda_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.LambdaExpression) s

  and invalidate_lambda_expression : lambda_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.LambdaExpression
          {
            lambda_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.lambda_attribute_spec;
            lambda_async =
              invalidate_option_with invalidate_token x.lambda_async;
            lambda_signature = invalidate_specifier x.lambda_signature;
            lambda_arrow = invalidate_token x.lambda_arrow;
            lambda_body = invalidate_lambda_body x.lambda_body;
          };
      Syntax.value = v;
    }

  and validate_lambda_signature : lambda_signature validator = function
    | { Syntax.syntax = Syntax.LambdaSignature x; value = v } ->
      ( v,
        {
          lambda_type = validate_option_with validate_specifier x.lambda_type;
          lambda_readonly_return =
            validate_option_with validate_token x.lambda_readonly_return;
          lambda_colon = validate_option_with validate_token x.lambda_colon;
          lambda_contexts =
            validate_option_with validate_contexts x.lambda_contexts;
          lambda_right_paren = validate_token x.lambda_right_paren;
          lambda_parameters =
            validate_list_with validate_parameter x.lambda_parameters;
          lambda_left_paren = validate_token x.lambda_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.LambdaSignature) s

  and invalidate_lambda_signature : lambda_signature invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.LambdaSignature
          {
            lambda_left_paren = invalidate_token x.lambda_left_paren;
            lambda_parameters =
              invalidate_list_with invalidate_parameter x.lambda_parameters;
            lambda_right_paren = invalidate_token x.lambda_right_paren;
            lambda_contexts =
              invalidate_option_with invalidate_contexts x.lambda_contexts;
            lambda_colon =
              invalidate_option_with invalidate_token x.lambda_colon;
            lambda_readonly_return =
              invalidate_option_with invalidate_token x.lambda_readonly_return;
            lambda_type =
              invalidate_option_with invalidate_specifier x.lambda_type;
          };
      Syntax.value = v;
    }

  and validate_cast_expression : cast_expression validator = function
    | { Syntax.syntax = Syntax.CastExpression x; value = v } ->
      ( v,
        {
          cast_operand = validate_expression x.cast_operand;
          cast_right_paren = validate_token x.cast_right_paren;
          cast_type = validate_token x.cast_type;
          cast_left_paren = validate_token x.cast_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.CastExpression) s

  and invalidate_cast_expression : cast_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.CastExpression
          {
            cast_left_paren = invalidate_token x.cast_left_paren;
            cast_type = invalidate_token x.cast_type;
            cast_right_paren = invalidate_token x.cast_right_paren;
            cast_operand = invalidate_expression x.cast_operand;
          };
      Syntax.value = v;
    }

  and validate_scope_resolution_expression :
      scope_resolution_expression validator = function
    | { Syntax.syntax = Syntax.ScopeResolutionExpression x; value = v } ->
      ( v,
        {
          scope_resolution_name = validate_expression x.scope_resolution_name;
          scope_resolution_operator = validate_token x.scope_resolution_operator;
          scope_resolution_qualifier =
            validate_expression x.scope_resolution_qualifier;
        } )
    | s -> validation_fail (Some SyntaxKind.ScopeResolutionExpression) s

  and invalidate_scope_resolution_expression :
      scope_resolution_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ScopeResolutionExpression
          {
            scope_resolution_qualifier =
              invalidate_expression x.scope_resolution_qualifier;
            scope_resolution_operator =
              invalidate_token x.scope_resolution_operator;
            scope_resolution_name =
              invalidate_expression x.scope_resolution_name;
          };
      Syntax.value = v;
    }

  and validate_member_selection_expression :
      member_selection_expression validator = function
    | { Syntax.syntax = Syntax.MemberSelectionExpression x; value = v } ->
      ( v,
        {
          member_name = validate_token x.member_name;
          member_operator = validate_token x.member_operator;
          member_object = validate_expression x.member_object;
        } )
    | s -> validation_fail (Some SyntaxKind.MemberSelectionExpression) s

  and invalidate_member_selection_expression :
      member_selection_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.MemberSelectionExpression
          {
            member_object = invalidate_expression x.member_object;
            member_operator = invalidate_token x.member_operator;
            member_name = invalidate_token x.member_name;
          };
      Syntax.value = v;
    }

  and validate_safe_member_selection_expression :
      safe_member_selection_expression validator = function
    | { Syntax.syntax = Syntax.SafeMemberSelectionExpression x; value = v } ->
      ( v,
        {
          safe_member_name = validate_token x.safe_member_name;
          safe_member_operator = validate_token x.safe_member_operator;
          safe_member_object = validate_expression x.safe_member_object;
        } )
    | s -> validation_fail (Some SyntaxKind.SafeMemberSelectionExpression) s

  and invalidate_safe_member_selection_expression :
      safe_member_selection_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SafeMemberSelectionExpression
          {
            safe_member_object = invalidate_expression x.safe_member_object;
            safe_member_operator = invalidate_token x.safe_member_operator;
            safe_member_name = invalidate_token x.safe_member_name;
          };
      Syntax.value = v;
    }

  and validate_embedded_member_selection_expression :
      embedded_member_selection_expression validator = function
    | { Syntax.syntax = Syntax.EmbeddedMemberSelectionExpression x; value = v }
      ->
      ( v,
        {
          embedded_member_name = validate_token x.embedded_member_name;
          embedded_member_operator = validate_token x.embedded_member_operator;
          embedded_member_object =
            validate_variable_expression x.embedded_member_object;
        } )
    | s -> validation_fail (Some SyntaxKind.EmbeddedMemberSelectionExpression) s

  and invalidate_embedded_member_selection_expression :
      embedded_member_selection_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EmbeddedMemberSelectionExpression
          {
            embedded_member_object =
              invalidate_variable_expression x.embedded_member_object;
            embedded_member_operator =
              invalidate_token x.embedded_member_operator;
            embedded_member_name = invalidate_token x.embedded_member_name;
          };
      Syntax.value = v;
    }

  and validate_yield_expression : yield_expression validator = function
    | { Syntax.syntax = Syntax.YieldExpression x; value = v } ->
      ( v,
        {
          yield_operand = validate_constructor_expression x.yield_operand;
          yield_keyword = validate_token x.yield_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.YieldExpression) s

  and invalidate_yield_expression : yield_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.YieldExpression
          {
            yield_keyword = invalidate_token x.yield_keyword;
            yield_operand = invalidate_constructor_expression x.yield_operand;
          };
      Syntax.value = v;
    }

  and validate_prefix_unary_expression : prefix_unary_expression validator =
    function
    | { Syntax.syntax = Syntax.PrefixUnaryExpression x; value = v } ->
      ( v,
        {
          prefix_unary_operand = validate_expression x.prefix_unary_operand;
          prefix_unary_operator = validate_token x.prefix_unary_operator;
        } )
    | s -> validation_fail (Some SyntaxKind.PrefixUnaryExpression) s

  and invalidate_prefix_unary_expression : prefix_unary_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PrefixUnaryExpression
          {
            prefix_unary_operator = invalidate_token x.prefix_unary_operator;
            prefix_unary_operand = invalidate_expression x.prefix_unary_operand;
          };
      Syntax.value = v;
    }

  and validate_postfix_unary_expression : postfix_unary_expression validator =
    function
    | { Syntax.syntax = Syntax.PostfixUnaryExpression x; value = v } ->
      ( v,
        {
          postfix_unary_operator = validate_token x.postfix_unary_operator;
          postfix_unary_operand = validate_expression x.postfix_unary_operand;
        } )
    | s -> validation_fail (Some SyntaxKind.PostfixUnaryExpression) s

  and invalidate_postfix_unary_expression : postfix_unary_expression invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.PostfixUnaryExpression
          {
            postfix_unary_operand =
              invalidate_expression x.postfix_unary_operand;
            postfix_unary_operator = invalidate_token x.postfix_unary_operator;
          };
      Syntax.value = v;
    }

  and validate_binary_expression : binary_expression validator = function
    | { Syntax.syntax = Syntax.BinaryExpression x; value = v } ->
      ( v,
        {
          binary_right_operand = validate_expression x.binary_right_operand;
          binary_operator = validate_token x.binary_operator;
          binary_left_operand = validate_expression x.binary_left_operand;
        } )
    | s -> validation_fail (Some SyntaxKind.BinaryExpression) s

  and invalidate_binary_expression : binary_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.BinaryExpression
          {
            binary_left_operand = invalidate_expression x.binary_left_operand;
            binary_operator = invalidate_token x.binary_operator;
            binary_right_operand = invalidate_expression x.binary_right_operand;
          };
      Syntax.value = v;
    }

  and validate_is_expression : is_expression validator = function
    | { Syntax.syntax = Syntax.IsExpression x; value = v } ->
      ( v,
        {
          is_right_operand = validate_specifier x.is_right_operand;
          is_operator = validate_token x.is_operator;
          is_left_operand = validate_expression x.is_left_operand;
        } )
    | s -> validation_fail (Some SyntaxKind.IsExpression) s

  and invalidate_is_expression : is_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.IsExpression
          {
            is_left_operand = invalidate_expression x.is_left_operand;
            is_operator = invalidate_token x.is_operator;
            is_right_operand = invalidate_specifier x.is_right_operand;
          };
      Syntax.value = v;
    }

  and validate_as_expression : as_expression validator = function
    | { Syntax.syntax = Syntax.AsExpression x; value = v } ->
      ( v,
        {
          as_right_operand = validate_specifier x.as_right_operand;
          as_operator = validate_token x.as_operator;
          as_left_operand = validate_expression x.as_left_operand;
        } )
    | s -> validation_fail (Some SyntaxKind.AsExpression) s

  and invalidate_as_expression : as_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AsExpression
          {
            as_left_operand = invalidate_expression x.as_left_operand;
            as_operator = invalidate_token x.as_operator;
            as_right_operand = invalidate_specifier x.as_right_operand;
          };
      Syntax.value = v;
    }

  and validate_nullable_as_expression : nullable_as_expression validator =
    function
    | { Syntax.syntax = Syntax.NullableAsExpression x; value = v } ->
      ( v,
        {
          nullable_as_right_operand =
            validate_specifier x.nullable_as_right_operand;
          nullable_as_operator = validate_token x.nullable_as_operator;
          nullable_as_left_operand =
            validate_expression x.nullable_as_left_operand;
        } )
    | s -> validation_fail (Some SyntaxKind.NullableAsExpression) s

  and invalidate_nullable_as_expression : nullable_as_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NullableAsExpression
          {
            nullable_as_left_operand =
              invalidate_expression x.nullable_as_left_operand;
            nullable_as_operator = invalidate_token x.nullable_as_operator;
            nullable_as_right_operand =
              invalidate_specifier x.nullable_as_right_operand;
          };
      Syntax.value = v;
    }

  and validate_conditional_expression : conditional_expression validator =
    function
    | { Syntax.syntax = Syntax.ConditionalExpression x; value = v } ->
      ( v,
        {
          conditional_alternative =
            validate_expression x.conditional_alternative;
          conditional_colon = validate_token x.conditional_colon;
          conditional_consequence =
            validate_option_with validate_expression x.conditional_consequence;
          conditional_question = validate_token x.conditional_question;
          conditional_test = validate_expression x.conditional_test;
        } )
    | s -> validation_fail (Some SyntaxKind.ConditionalExpression) s

  and invalidate_conditional_expression : conditional_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ConditionalExpression
          {
            conditional_test = invalidate_expression x.conditional_test;
            conditional_question = invalidate_token x.conditional_question;
            conditional_consequence =
              invalidate_option_with
                invalidate_expression
                x.conditional_consequence;
            conditional_colon = invalidate_token x.conditional_colon;
            conditional_alternative =
              invalidate_expression x.conditional_alternative;
          };
      Syntax.value = v;
    }

  and validate_eval_expression : eval_expression validator = function
    | { Syntax.syntax = Syntax.EvalExpression x; value = v } ->
      ( v,
        {
          eval_right_paren = validate_token x.eval_right_paren;
          eval_argument = validate_expression x.eval_argument;
          eval_left_paren = validate_token x.eval_left_paren;
          eval_keyword = validate_token x.eval_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.EvalExpression) s

  and invalidate_eval_expression : eval_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EvalExpression
          {
            eval_keyword = invalidate_token x.eval_keyword;
            eval_left_paren = invalidate_token x.eval_left_paren;
            eval_argument = invalidate_expression x.eval_argument;
            eval_right_paren = invalidate_token x.eval_right_paren;
          };
      Syntax.value = v;
    }

  and validate_isset_expression : isset_expression validator = function
    | { Syntax.syntax = Syntax.IssetExpression x; value = v } ->
      ( v,
        {
          isset_right_paren = validate_token x.isset_right_paren;
          isset_argument_list =
            validate_list_with validate_expression x.isset_argument_list;
          isset_left_paren = validate_token x.isset_left_paren;
          isset_keyword = validate_token x.isset_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.IssetExpression) s

  and invalidate_isset_expression : isset_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.IssetExpression
          {
            isset_keyword = invalidate_token x.isset_keyword;
            isset_left_paren = invalidate_token x.isset_left_paren;
            isset_argument_list =
              invalidate_list_with invalidate_expression x.isset_argument_list;
            isset_right_paren = invalidate_token x.isset_right_paren;
          };
      Syntax.value = v;
    }

  and validate_function_call_expression : function_call_expression validator =
    function
    | { Syntax.syntax = Syntax.FunctionCallExpression x; value = v } ->
      ( v,
        {
          function_call_right_paren = validate_token x.function_call_right_paren;
          function_call_argument_list =
            validate_list_with validate_expression x.function_call_argument_list;
          function_call_left_paren = validate_token x.function_call_left_paren;
          function_call_enum_class_label =
            validate_option_with
              validate_expression
              x.function_call_enum_class_label;
          function_call_type_args =
            validate_option_with
              validate_type_arguments
              x.function_call_type_args;
          function_call_receiver = validate_expression x.function_call_receiver;
        } )
    | s -> validation_fail (Some SyntaxKind.FunctionCallExpression) s

  and invalidate_function_call_expression : function_call_expression invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FunctionCallExpression
          {
            function_call_receiver =
              invalidate_expression x.function_call_receiver;
            function_call_type_args =
              invalidate_option_with
                invalidate_type_arguments
                x.function_call_type_args;
            function_call_enum_class_label =
              invalidate_option_with
                invalidate_expression
                x.function_call_enum_class_label;
            function_call_left_paren =
              invalidate_token x.function_call_left_paren;
            function_call_argument_list =
              invalidate_list_with
                invalidate_expression
                x.function_call_argument_list;
            function_call_right_paren =
              invalidate_token x.function_call_right_paren;
          };
      Syntax.value = v;
    }

  and validate_function_pointer_expression :
      function_pointer_expression validator = function
    | { Syntax.syntax = Syntax.FunctionPointerExpression x; value = v } ->
      ( v,
        {
          function_pointer_type_args =
            validate_type_arguments x.function_pointer_type_args;
          function_pointer_receiver =
            validate_expression x.function_pointer_receiver;
        } )
    | s -> validation_fail (Some SyntaxKind.FunctionPointerExpression) s

  and invalidate_function_pointer_expression :
      function_pointer_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FunctionPointerExpression
          {
            function_pointer_receiver =
              invalidate_expression x.function_pointer_receiver;
            function_pointer_type_args =
              invalidate_type_arguments x.function_pointer_type_args;
          };
      Syntax.value = v;
    }

  and validate_parenthesized_expression : parenthesized_expression validator =
    function
    | { Syntax.syntax = Syntax.ParenthesizedExpression x; value = v } ->
      ( v,
        {
          parenthesized_expression_right_paren =
            validate_token x.parenthesized_expression_right_paren;
          parenthesized_expression_expression =
            validate_expression x.parenthesized_expression_expression;
          parenthesized_expression_left_paren =
            validate_token x.parenthesized_expression_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.ParenthesizedExpression) s

  and invalidate_parenthesized_expression : parenthesized_expression invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ParenthesizedExpression
          {
            parenthesized_expression_left_paren =
              invalidate_token x.parenthesized_expression_left_paren;
            parenthesized_expression_expression =
              invalidate_expression x.parenthesized_expression_expression;
            parenthesized_expression_right_paren =
              invalidate_token x.parenthesized_expression_right_paren;
          };
      Syntax.value = v;
    }

  and validate_braced_expression : braced_expression validator = function
    | { Syntax.syntax = Syntax.BracedExpression x; value = v } ->
      ( v,
        {
          braced_expression_right_brace =
            validate_token x.braced_expression_right_brace;
          braced_expression_expression =
            validate_expression x.braced_expression_expression;
          braced_expression_left_brace =
            validate_token x.braced_expression_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.BracedExpression) s

  and invalidate_braced_expression : braced_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.BracedExpression
          {
            braced_expression_left_brace =
              invalidate_token x.braced_expression_left_brace;
            braced_expression_expression =
              invalidate_expression x.braced_expression_expression;
            braced_expression_right_brace =
              invalidate_token x.braced_expression_right_brace;
          };
      Syntax.value = v;
    }

  and validate_et_splice_expression : et_splice_expression validator = function
    | { Syntax.syntax = Syntax.ETSpliceExpression x; value = v } ->
      ( v,
        {
          et_splice_expression_right_brace =
            validate_token x.et_splice_expression_right_brace;
          et_splice_expression_expression =
            validate_expression x.et_splice_expression_expression;
          et_splice_expression_left_brace =
            validate_token x.et_splice_expression_left_brace;
          et_splice_expression_dollar =
            validate_token x.et_splice_expression_dollar;
        } )
    | s -> validation_fail (Some SyntaxKind.ETSpliceExpression) s

  and invalidate_et_splice_expression : et_splice_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ETSpliceExpression
          {
            et_splice_expression_dollar =
              invalidate_token x.et_splice_expression_dollar;
            et_splice_expression_left_brace =
              invalidate_token x.et_splice_expression_left_brace;
            et_splice_expression_expression =
              invalidate_expression x.et_splice_expression_expression;
            et_splice_expression_right_brace =
              invalidate_token x.et_splice_expression_right_brace;
          };
      Syntax.value = v;
    }

  and validate_embedded_braced_expression : embedded_braced_expression validator
      = function
    | { Syntax.syntax = Syntax.EmbeddedBracedExpression x; value = v } ->
      ( v,
        {
          embedded_braced_expression_right_brace =
            validate_token x.embedded_braced_expression_right_brace;
          embedded_braced_expression_expression =
            validate_expression x.embedded_braced_expression_expression;
          embedded_braced_expression_left_brace =
            validate_token x.embedded_braced_expression_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.EmbeddedBracedExpression) s

  and invalidate_embedded_braced_expression :
      embedded_braced_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EmbeddedBracedExpression
          {
            embedded_braced_expression_left_brace =
              invalidate_token x.embedded_braced_expression_left_brace;
            embedded_braced_expression_expression =
              invalidate_expression x.embedded_braced_expression_expression;
            embedded_braced_expression_right_brace =
              invalidate_token x.embedded_braced_expression_right_brace;
          };
      Syntax.value = v;
    }

  and validate_list_expression : list_expression validator = function
    | { Syntax.syntax = Syntax.ListExpression x; value = v } ->
      ( v,
        {
          list_right_paren = validate_token x.list_right_paren;
          list_members = validate_list_with validate_expression x.list_members;
          list_left_paren = validate_token x.list_left_paren;
          list_keyword = validate_token x.list_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ListExpression) s

  and invalidate_list_expression : list_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ListExpression
          {
            list_keyword = invalidate_token x.list_keyword;
            list_left_paren = invalidate_token x.list_left_paren;
            list_members =
              invalidate_list_with invalidate_expression x.list_members;
            list_right_paren = invalidate_token x.list_right_paren;
          };
      Syntax.value = v;
    }

  and validate_collection_literal_expression :
      collection_literal_expression validator = function
    | { Syntax.syntax = Syntax.CollectionLiteralExpression x; value = v } ->
      ( v,
        {
          collection_literal_right_brace =
            validate_token x.collection_literal_right_brace;
          collection_literal_initializers =
            validate_list_with
              validate_constructor_expression
              x.collection_literal_initializers;
          collection_literal_left_brace =
            validate_token x.collection_literal_left_brace;
          collection_literal_name = validate_specifier x.collection_literal_name;
        } )
    | s -> validation_fail (Some SyntaxKind.CollectionLiteralExpression) s

  and invalidate_collection_literal_expression :
      collection_literal_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.CollectionLiteralExpression
          {
            collection_literal_name =
              invalidate_specifier x.collection_literal_name;
            collection_literal_left_brace =
              invalidate_token x.collection_literal_left_brace;
            collection_literal_initializers =
              invalidate_list_with
                invalidate_constructor_expression
                x.collection_literal_initializers;
            collection_literal_right_brace =
              invalidate_token x.collection_literal_right_brace;
          };
      Syntax.value = v;
    }

  and validate_object_creation_expression : object_creation_expression validator
      = function
    | { Syntax.syntax = Syntax.ObjectCreationExpression x; value = v } ->
      ( v,
        {
          object_creation_object =
            validate_object_creation_what x.object_creation_object;
          object_creation_new_keyword =
            validate_token x.object_creation_new_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ObjectCreationExpression) s

  and invalidate_object_creation_expression :
      object_creation_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ObjectCreationExpression
          {
            object_creation_new_keyword =
              invalidate_token x.object_creation_new_keyword;
            object_creation_object =
              invalidate_object_creation_what x.object_creation_object;
          };
      Syntax.value = v;
    }

  and validate_constructor_call : constructor_call validator = function
    | { Syntax.syntax = Syntax.ConstructorCall x; value = v } ->
      ( v,
        {
          constructor_call_right_paren =
            validate_option_with validate_token x.constructor_call_right_paren;
          constructor_call_argument_list =
            validate_list_with
              validate_expression
              x.constructor_call_argument_list;
          constructor_call_left_paren =
            validate_option_with validate_token x.constructor_call_left_paren;
          constructor_call_type =
            validate_todo_aggregate x.constructor_call_type;
        } )
    | s -> validation_fail (Some SyntaxKind.ConstructorCall) s

  and invalidate_constructor_call : constructor_call invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ConstructorCall
          {
            constructor_call_type =
              invalidate_todo_aggregate x.constructor_call_type;
            constructor_call_left_paren =
              invalidate_option_with
                invalidate_token
                x.constructor_call_left_paren;
            constructor_call_argument_list =
              invalidate_list_with
                invalidate_expression
                x.constructor_call_argument_list;
            constructor_call_right_paren =
              invalidate_option_with
                invalidate_token
                x.constructor_call_right_paren;
          };
      Syntax.value = v;
    }

  and validate_record_creation_expression : record_creation_expression validator
      = function
    | { Syntax.syntax = Syntax.RecordCreationExpression x; value = v } ->
      ( v,
        {
          record_creation_right_bracket =
            validate_token x.record_creation_right_bracket;
          record_creation_members =
            validate_list_with
              validate_element_initializer
              x.record_creation_members;
          record_creation_left_bracket =
            validate_token x.record_creation_left_bracket;
          record_creation_type = validate_todo_aggregate x.record_creation_type;
        } )
    | s -> validation_fail (Some SyntaxKind.RecordCreationExpression) s

  and invalidate_record_creation_expression :
      record_creation_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.RecordCreationExpression
          {
            record_creation_type =
              invalidate_todo_aggregate x.record_creation_type;
            record_creation_left_bracket =
              invalidate_token x.record_creation_left_bracket;
            record_creation_members =
              invalidate_list_with
                invalidate_element_initializer
                x.record_creation_members;
            record_creation_right_bracket =
              invalidate_token x.record_creation_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_darray_intrinsic_expression :
      darray_intrinsic_expression validator = function
    | { Syntax.syntax = Syntax.DarrayIntrinsicExpression x; value = v } ->
      ( v,
        {
          darray_intrinsic_right_bracket =
            validate_token x.darray_intrinsic_right_bracket;
          darray_intrinsic_members =
            validate_list_with
              validate_element_initializer
              x.darray_intrinsic_members;
          darray_intrinsic_left_bracket =
            validate_token x.darray_intrinsic_left_bracket;
          darray_intrinsic_explicit_type =
            validate_option_with
              validate_type_arguments
              x.darray_intrinsic_explicit_type;
          darray_intrinsic_keyword = validate_token x.darray_intrinsic_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DarrayIntrinsicExpression) s

  and invalidate_darray_intrinsic_expression :
      darray_intrinsic_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DarrayIntrinsicExpression
          {
            darray_intrinsic_keyword =
              invalidate_token x.darray_intrinsic_keyword;
            darray_intrinsic_explicit_type =
              invalidate_option_with
                invalidate_type_arguments
                x.darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket =
              invalidate_token x.darray_intrinsic_left_bracket;
            darray_intrinsic_members =
              invalidate_list_with
                invalidate_element_initializer
                x.darray_intrinsic_members;
            darray_intrinsic_right_bracket =
              invalidate_token x.darray_intrinsic_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_dictionary_intrinsic_expression :
      dictionary_intrinsic_expression validator = function
    | { Syntax.syntax = Syntax.DictionaryIntrinsicExpression x; value = v } ->
      ( v,
        {
          dictionary_intrinsic_right_bracket =
            validate_token x.dictionary_intrinsic_right_bracket;
          dictionary_intrinsic_members =
            validate_list_with
              validate_element_initializer
              x.dictionary_intrinsic_members;
          dictionary_intrinsic_left_bracket =
            validate_token x.dictionary_intrinsic_left_bracket;
          dictionary_intrinsic_explicit_type =
            validate_option_with
              validate_type_arguments
              x.dictionary_intrinsic_explicit_type;
          dictionary_intrinsic_keyword =
            validate_token x.dictionary_intrinsic_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DictionaryIntrinsicExpression) s

  and invalidate_dictionary_intrinsic_expression :
      dictionary_intrinsic_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DictionaryIntrinsicExpression
          {
            dictionary_intrinsic_keyword =
              invalidate_token x.dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type =
              invalidate_option_with
                invalidate_type_arguments
                x.dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket =
              invalidate_token x.dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members =
              invalidate_list_with
                invalidate_element_initializer
                x.dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket =
              invalidate_token x.dictionary_intrinsic_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_keyset_intrinsic_expression :
      keyset_intrinsic_expression validator = function
    | { Syntax.syntax = Syntax.KeysetIntrinsicExpression x; value = v } ->
      ( v,
        {
          keyset_intrinsic_right_bracket =
            validate_token x.keyset_intrinsic_right_bracket;
          keyset_intrinsic_members =
            validate_list_with validate_expression x.keyset_intrinsic_members;
          keyset_intrinsic_left_bracket =
            validate_token x.keyset_intrinsic_left_bracket;
          keyset_intrinsic_explicit_type =
            validate_option_with
              validate_type_arguments
              x.keyset_intrinsic_explicit_type;
          keyset_intrinsic_keyword = validate_token x.keyset_intrinsic_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.KeysetIntrinsicExpression) s

  and invalidate_keyset_intrinsic_expression :
      keyset_intrinsic_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.KeysetIntrinsicExpression
          {
            keyset_intrinsic_keyword =
              invalidate_token x.keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type =
              invalidate_option_with
                invalidate_type_arguments
                x.keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket =
              invalidate_token x.keyset_intrinsic_left_bracket;
            keyset_intrinsic_members =
              invalidate_list_with
                invalidate_expression
                x.keyset_intrinsic_members;
            keyset_intrinsic_right_bracket =
              invalidate_token x.keyset_intrinsic_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_varray_intrinsic_expression :
      varray_intrinsic_expression validator = function
    | { Syntax.syntax = Syntax.VarrayIntrinsicExpression x; value = v } ->
      ( v,
        {
          varray_intrinsic_right_bracket =
            validate_token x.varray_intrinsic_right_bracket;
          varray_intrinsic_members =
            validate_list_with validate_expression x.varray_intrinsic_members;
          varray_intrinsic_left_bracket =
            validate_token x.varray_intrinsic_left_bracket;
          varray_intrinsic_explicit_type =
            validate_option_with
              validate_type_arguments
              x.varray_intrinsic_explicit_type;
          varray_intrinsic_keyword = validate_token x.varray_intrinsic_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.VarrayIntrinsicExpression) s

  and invalidate_varray_intrinsic_expression :
      varray_intrinsic_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VarrayIntrinsicExpression
          {
            varray_intrinsic_keyword =
              invalidate_token x.varray_intrinsic_keyword;
            varray_intrinsic_explicit_type =
              invalidate_option_with
                invalidate_type_arguments
                x.varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket =
              invalidate_token x.varray_intrinsic_left_bracket;
            varray_intrinsic_members =
              invalidate_list_with
                invalidate_expression
                x.varray_intrinsic_members;
            varray_intrinsic_right_bracket =
              invalidate_token x.varray_intrinsic_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_vector_intrinsic_expression :
      vector_intrinsic_expression validator = function
    | { Syntax.syntax = Syntax.VectorIntrinsicExpression x; value = v } ->
      ( v,
        {
          vector_intrinsic_right_bracket =
            validate_token x.vector_intrinsic_right_bracket;
          vector_intrinsic_members =
            validate_list_with validate_expression x.vector_intrinsic_members;
          vector_intrinsic_left_bracket =
            validate_token x.vector_intrinsic_left_bracket;
          vector_intrinsic_explicit_type =
            validate_option_with
              validate_type_arguments
              x.vector_intrinsic_explicit_type;
          vector_intrinsic_keyword = validate_token x.vector_intrinsic_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.VectorIntrinsicExpression) s

  and invalidate_vector_intrinsic_expression :
      vector_intrinsic_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VectorIntrinsicExpression
          {
            vector_intrinsic_keyword =
              invalidate_token x.vector_intrinsic_keyword;
            vector_intrinsic_explicit_type =
              invalidate_option_with
                invalidate_type_arguments
                x.vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket =
              invalidate_token x.vector_intrinsic_left_bracket;
            vector_intrinsic_members =
              invalidate_list_with
                invalidate_expression
                x.vector_intrinsic_members;
            vector_intrinsic_right_bracket =
              invalidate_token x.vector_intrinsic_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_element_initializer : element_initializer validator = function
    | { Syntax.syntax = Syntax.ElementInitializer x; value = v } ->
      ( v,
        {
          element_value = validate_expression x.element_value;
          element_arrow = validate_token x.element_arrow;
          element_key = validate_expression x.element_key;
        } )
    | s -> validation_fail (Some SyntaxKind.ElementInitializer) s

  and invalidate_element_initializer : element_initializer invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ElementInitializer
          {
            element_key = invalidate_expression x.element_key;
            element_arrow = invalidate_token x.element_arrow;
            element_value = invalidate_expression x.element_value;
          };
      Syntax.value = v;
    }

  and validate_subscript_expression : subscript_expression validator = function
    | { Syntax.syntax = Syntax.SubscriptExpression x; value = v } ->
      ( v,
        {
          subscript_right_bracket = validate_token x.subscript_right_bracket;
          subscript_index =
            validate_option_with validate_expression x.subscript_index;
          subscript_left_bracket = validate_token x.subscript_left_bracket;
          subscript_receiver = validate_expression x.subscript_receiver;
        } )
    | s -> validation_fail (Some SyntaxKind.SubscriptExpression) s

  and invalidate_subscript_expression : subscript_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SubscriptExpression
          {
            subscript_receiver = invalidate_expression x.subscript_receiver;
            subscript_left_bracket = invalidate_token x.subscript_left_bracket;
            subscript_index =
              invalidate_option_with invalidate_expression x.subscript_index;
            subscript_right_bracket = invalidate_token x.subscript_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_embedded_subscript_expression :
      embedded_subscript_expression validator = function
    | { Syntax.syntax = Syntax.EmbeddedSubscriptExpression x; value = v } ->
      ( v,
        {
          embedded_subscript_right_bracket =
            validate_token x.embedded_subscript_right_bracket;
          embedded_subscript_index =
            validate_expression x.embedded_subscript_index;
          embedded_subscript_left_bracket =
            validate_token x.embedded_subscript_left_bracket;
          embedded_subscript_receiver =
            validate_variable_expression x.embedded_subscript_receiver;
        } )
    | s -> validation_fail (Some SyntaxKind.EmbeddedSubscriptExpression) s

  and invalidate_embedded_subscript_expression :
      embedded_subscript_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EmbeddedSubscriptExpression
          {
            embedded_subscript_receiver =
              invalidate_variable_expression x.embedded_subscript_receiver;
            embedded_subscript_left_bracket =
              invalidate_token x.embedded_subscript_left_bracket;
            embedded_subscript_index =
              invalidate_expression x.embedded_subscript_index;
            embedded_subscript_right_bracket =
              invalidate_token x.embedded_subscript_right_bracket;
          };
      Syntax.value = v;
    }

  and validate_awaitable_creation_expression :
      awaitable_creation_expression validator = function
    | { Syntax.syntax = Syntax.AwaitableCreationExpression x; value = v } ->
      ( v,
        {
          awaitable_compound_statement =
            validate_compound_statement x.awaitable_compound_statement;
          awaitable_async = validate_token x.awaitable_async;
          awaitable_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.awaitable_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.AwaitableCreationExpression) s

  and invalidate_awaitable_creation_expression :
      awaitable_creation_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AwaitableCreationExpression
          {
            awaitable_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.awaitable_attribute_spec;
            awaitable_async = invalidate_token x.awaitable_async;
            awaitable_compound_statement =
              invalidate_compound_statement x.awaitable_compound_statement;
          };
      Syntax.value = v;
    }

  and validate_xhp_children_declaration : xhp_children_declaration validator =
    function
    | { Syntax.syntax = Syntax.XHPChildrenDeclaration x; value = v } ->
      ( v,
        {
          xhp_children_semicolon = validate_token x.xhp_children_semicolon;
          xhp_children_expression =
            validate_expression x.xhp_children_expression;
          xhp_children_keyword = validate_token x.xhp_children_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPChildrenDeclaration) s

  and invalidate_xhp_children_declaration : xhp_children_declaration invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPChildrenDeclaration
          {
            xhp_children_keyword = invalidate_token x.xhp_children_keyword;
            xhp_children_expression =
              invalidate_expression x.xhp_children_expression;
            xhp_children_semicolon = invalidate_token x.xhp_children_semicolon;
          };
      Syntax.value = v;
    }

  and validate_xhp_children_parenthesized_list :
      xhp_children_parenthesized_list validator = function
    | { Syntax.syntax = Syntax.XHPChildrenParenthesizedList x; value = v } ->
      ( v,
        {
          xhp_children_list_right_paren =
            validate_token x.xhp_children_list_right_paren;
          xhp_children_list_xhp_children =
            validate_list_with
              validate_expression
              x.xhp_children_list_xhp_children;
          xhp_children_list_left_paren =
            validate_token x.xhp_children_list_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPChildrenParenthesizedList) s

  and invalidate_xhp_children_parenthesized_list :
      xhp_children_parenthesized_list invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPChildrenParenthesizedList
          {
            xhp_children_list_left_paren =
              invalidate_token x.xhp_children_list_left_paren;
            xhp_children_list_xhp_children =
              invalidate_list_with
                invalidate_expression
                x.xhp_children_list_xhp_children;
            xhp_children_list_right_paren =
              invalidate_token x.xhp_children_list_right_paren;
          };
      Syntax.value = v;
    }

  and validate_xhp_category_declaration : xhp_category_declaration validator =
    function
    | { Syntax.syntax = Syntax.XHPCategoryDeclaration x; value = v } ->
      ( v,
        {
          xhp_category_semicolon = validate_token x.xhp_category_semicolon;
          xhp_category_categories =
            validate_list_with validate_token x.xhp_category_categories;
          xhp_category_keyword = validate_token x.xhp_category_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPCategoryDeclaration) s

  and invalidate_xhp_category_declaration : xhp_category_declaration invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPCategoryDeclaration
          {
            xhp_category_keyword = invalidate_token x.xhp_category_keyword;
            xhp_category_categories =
              invalidate_list_with invalidate_token x.xhp_category_categories;
            xhp_category_semicolon = invalidate_token x.xhp_category_semicolon;
          };
      Syntax.value = v;
    }

  and validate_xhp_enum_type : xhp_enum_type validator = function
    | { Syntax.syntax = Syntax.XHPEnumType x; value = v } ->
      ( v,
        {
          xhp_enum_right_brace = validate_token x.xhp_enum_right_brace;
          xhp_enum_values =
            validate_list_with validate_literal_expression x.xhp_enum_values;
          xhp_enum_left_brace = validate_token x.xhp_enum_left_brace;
          xhp_enum_keyword = validate_token x.xhp_enum_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPEnumType) s

  and invalidate_xhp_enum_type : xhp_enum_type invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPEnumType
          {
            xhp_enum_keyword = invalidate_token x.xhp_enum_keyword;
            xhp_enum_left_brace = invalidate_token x.xhp_enum_left_brace;
            xhp_enum_values =
              invalidate_list_with
                invalidate_literal_expression
                x.xhp_enum_values;
            xhp_enum_right_brace = invalidate_token x.xhp_enum_right_brace;
          };
      Syntax.value = v;
    }

  and validate_xhp_lateinit : xhp_lateinit validator = function
    | { Syntax.syntax = Syntax.XHPLateinit x; value = v } ->
      ( v,
        {
          xhp_lateinit_keyword = validate_token x.xhp_lateinit_keyword;
          xhp_lateinit_at = validate_token x.xhp_lateinit_at;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPLateinit) s

  and invalidate_xhp_lateinit : xhp_lateinit invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPLateinit
          {
            xhp_lateinit_at = invalidate_token x.xhp_lateinit_at;
            xhp_lateinit_keyword = invalidate_token x.xhp_lateinit_keyword;
          };
      Syntax.value = v;
    }

  and validate_xhp_required : xhp_required validator = function
    | { Syntax.syntax = Syntax.XHPRequired x; value = v } ->
      ( v,
        {
          xhp_required_keyword = validate_token x.xhp_required_keyword;
          xhp_required_at = validate_token x.xhp_required_at;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPRequired) s

  and invalidate_xhp_required : xhp_required invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPRequired
          {
            xhp_required_at = invalidate_token x.xhp_required_at;
            xhp_required_keyword = invalidate_token x.xhp_required_keyword;
          };
      Syntax.value = v;
    }

  and validate_xhp_class_attribute_declaration :
      xhp_class_attribute_declaration validator = function
    | { Syntax.syntax = Syntax.XHPClassAttributeDeclaration x; value = v } ->
      ( v,
        {
          xhp_attribute_semicolon = validate_token x.xhp_attribute_semicolon;
          xhp_attribute_attributes =
            validate_list_with
              validate_todo_aggregate
              x.xhp_attribute_attributes;
          xhp_attribute_keyword = validate_token x.xhp_attribute_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPClassAttributeDeclaration) s

  and invalidate_xhp_class_attribute_declaration :
      xhp_class_attribute_declaration invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPClassAttributeDeclaration
          {
            xhp_attribute_keyword = invalidate_token x.xhp_attribute_keyword;
            xhp_attribute_attributes =
              invalidate_list_with
                invalidate_todo_aggregate
                x.xhp_attribute_attributes;
            xhp_attribute_semicolon = invalidate_token x.xhp_attribute_semicolon;
          };
      Syntax.value = v;
    }

  and validate_xhp_class_attribute : xhp_class_attribute validator = function
    | { Syntax.syntax = Syntax.XHPClassAttribute x; value = v } ->
      ( v,
        {
          xhp_attribute_decl_required =
            validate_option_with
              validate_xhp_required
              x.xhp_attribute_decl_required;
          xhp_attribute_decl_initializer =
            validate_option_with
              validate_simple_initializer
              x.xhp_attribute_decl_initializer;
          xhp_attribute_decl_name = validate_token x.xhp_attribute_decl_name;
          xhp_attribute_decl_type = validate_specifier x.xhp_attribute_decl_type;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPClassAttribute) s

  and invalidate_xhp_class_attribute : xhp_class_attribute invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPClassAttribute
          {
            xhp_attribute_decl_type =
              invalidate_specifier x.xhp_attribute_decl_type;
            xhp_attribute_decl_name = invalidate_token x.xhp_attribute_decl_name;
            xhp_attribute_decl_initializer =
              invalidate_option_with
                invalidate_simple_initializer
                x.xhp_attribute_decl_initializer;
            xhp_attribute_decl_required =
              invalidate_option_with
                invalidate_xhp_required
                x.xhp_attribute_decl_required;
          };
      Syntax.value = v;
    }

  and validate_xhp_simple_class_attribute : xhp_simple_class_attribute validator
      = function
    | { Syntax.syntax = Syntax.XHPSimpleClassAttribute x; value = v } ->
      ( v,
        {
          xhp_simple_class_attribute_type =
            validate_simple_type_specifier x.xhp_simple_class_attribute_type;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPSimpleClassAttribute) s

  and invalidate_xhp_simple_class_attribute :
      xhp_simple_class_attribute invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPSimpleClassAttribute
          {
            xhp_simple_class_attribute_type =
              invalidate_simple_type_specifier x.xhp_simple_class_attribute_type;
          };
      Syntax.value = v;
    }

  and validate_xhp_simple_attribute : xhp_simple_attribute validator = function
    | { Syntax.syntax = Syntax.XHPSimpleAttribute x; value = v } ->
      ( v,
        {
          xhp_simple_attribute_expression =
            validate_expression x.xhp_simple_attribute_expression;
          xhp_simple_attribute_equal =
            validate_token x.xhp_simple_attribute_equal;
          xhp_simple_attribute_name = validate_token x.xhp_simple_attribute_name;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPSimpleAttribute) s

  and invalidate_xhp_simple_attribute : xhp_simple_attribute invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPSimpleAttribute
          {
            xhp_simple_attribute_name =
              invalidate_token x.xhp_simple_attribute_name;
            xhp_simple_attribute_equal =
              invalidate_token x.xhp_simple_attribute_equal;
            xhp_simple_attribute_expression =
              invalidate_expression x.xhp_simple_attribute_expression;
          };
      Syntax.value = v;
    }

  and validate_xhp_spread_attribute : xhp_spread_attribute validator = function
    | { Syntax.syntax = Syntax.XHPSpreadAttribute x; value = v } ->
      ( v,
        {
          xhp_spread_attribute_right_brace =
            validate_token x.xhp_spread_attribute_right_brace;
          xhp_spread_attribute_expression =
            validate_expression x.xhp_spread_attribute_expression;
          xhp_spread_attribute_spread_operator =
            validate_token x.xhp_spread_attribute_spread_operator;
          xhp_spread_attribute_left_brace =
            validate_token x.xhp_spread_attribute_left_brace;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPSpreadAttribute) s

  and invalidate_xhp_spread_attribute : xhp_spread_attribute invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPSpreadAttribute
          {
            xhp_spread_attribute_left_brace =
              invalidate_token x.xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator =
              invalidate_token x.xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression =
              invalidate_expression x.xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace =
              invalidate_token x.xhp_spread_attribute_right_brace;
          };
      Syntax.value = v;
    }

  and validate_xhp_open : xhp_open validator = function
    | { Syntax.syntax = Syntax.XHPOpen x; value = v } ->
      ( v,
        {
          xhp_open_right_angle = validate_token x.xhp_open_right_angle;
          xhp_open_attributes =
            validate_list_with validate_xhp_attribute x.xhp_open_attributes;
          xhp_open_name = validate_token x.xhp_open_name;
          xhp_open_left_angle = validate_token x.xhp_open_left_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPOpen) s

  and invalidate_xhp_open : xhp_open invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPOpen
          {
            xhp_open_left_angle = invalidate_token x.xhp_open_left_angle;
            xhp_open_name = invalidate_token x.xhp_open_name;
            xhp_open_attributes =
              invalidate_list_with
                invalidate_xhp_attribute
                x.xhp_open_attributes;
            xhp_open_right_angle = invalidate_token x.xhp_open_right_angle;
          };
      Syntax.value = v;
    }

  and validate_xhp_expression : xhp_expression validator = function
    | { Syntax.syntax = Syntax.XHPExpression x; value = v } ->
      ( v,
        {
          xhp_close = validate_option_with validate_xhp_close x.xhp_close;
          xhp_body = validate_list_with validate_expression x.xhp_body;
          xhp_open = validate_xhp_open x.xhp_open;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPExpression) s

  and invalidate_xhp_expression : xhp_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPExpression
          {
            xhp_open = invalidate_xhp_open x.xhp_open;
            xhp_body = invalidate_list_with invalidate_expression x.xhp_body;
            xhp_close = invalidate_option_with invalidate_xhp_close x.xhp_close;
          };
      Syntax.value = v;
    }

  and validate_xhp_close : xhp_close validator = function
    | { Syntax.syntax = Syntax.XHPClose x; value = v } ->
      ( v,
        {
          xhp_close_right_angle = validate_token x.xhp_close_right_angle;
          xhp_close_name = validate_token x.xhp_close_name;
          xhp_close_left_angle = validate_token x.xhp_close_left_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.XHPClose) s

  and invalidate_xhp_close : xhp_close invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.XHPClose
          {
            xhp_close_left_angle = invalidate_token x.xhp_close_left_angle;
            xhp_close_name = invalidate_token x.xhp_close_name;
            xhp_close_right_angle = invalidate_token x.xhp_close_right_angle;
          };
      Syntax.value = v;
    }

  and validate_type_constant : type_constant validator = function
    | { Syntax.syntax = Syntax.TypeConstant x; value = v } ->
      ( v,
        {
          type_constant_right_type = validate_token x.type_constant_right_type;
          type_constant_separator = validate_token x.type_constant_separator;
          type_constant_left_type = validate_specifier x.type_constant_left_type;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeConstant) s

  and invalidate_type_constant : type_constant invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeConstant
          {
            type_constant_left_type =
              invalidate_specifier x.type_constant_left_type;
            type_constant_separator = invalidate_token x.type_constant_separator;
            type_constant_right_type =
              invalidate_token x.type_constant_right_type;
          };
      Syntax.value = v;
    }

  and validate_vector_type_specifier : vector_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.VectorTypeSpecifier x; value = v } ->
      ( v,
        {
          vector_type_right_angle = validate_token x.vector_type_right_angle;
          vector_type_trailing_comma =
            validate_option_with validate_token x.vector_type_trailing_comma;
          vector_type_type = validate_specifier x.vector_type_type;
          vector_type_left_angle = validate_token x.vector_type_left_angle;
          vector_type_keyword = validate_token x.vector_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.VectorTypeSpecifier) s

  and invalidate_vector_type_specifier : vector_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VectorTypeSpecifier
          {
            vector_type_keyword = invalidate_token x.vector_type_keyword;
            vector_type_left_angle = invalidate_token x.vector_type_left_angle;
            vector_type_type = invalidate_specifier x.vector_type_type;
            vector_type_trailing_comma =
              invalidate_option_with
                invalidate_token
                x.vector_type_trailing_comma;
            vector_type_right_angle = invalidate_token x.vector_type_right_angle;
          };
      Syntax.value = v;
    }

  and validate_keyset_type_specifier : keyset_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.KeysetTypeSpecifier x; value = v } ->
      ( v,
        {
          keyset_type_right_angle = validate_token x.keyset_type_right_angle;
          keyset_type_trailing_comma =
            validate_option_with validate_token x.keyset_type_trailing_comma;
          keyset_type_type = validate_specifier x.keyset_type_type;
          keyset_type_left_angle = validate_token x.keyset_type_left_angle;
          keyset_type_keyword = validate_token x.keyset_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.KeysetTypeSpecifier) s

  and invalidate_keyset_type_specifier : keyset_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.KeysetTypeSpecifier
          {
            keyset_type_keyword = invalidate_token x.keyset_type_keyword;
            keyset_type_left_angle = invalidate_token x.keyset_type_left_angle;
            keyset_type_type = invalidate_specifier x.keyset_type_type;
            keyset_type_trailing_comma =
              invalidate_option_with
                invalidate_token
                x.keyset_type_trailing_comma;
            keyset_type_right_angle = invalidate_token x.keyset_type_right_angle;
          };
      Syntax.value = v;
    }

  and validate_tuple_type_explicit_specifier :
      tuple_type_explicit_specifier validator = function
    | { Syntax.syntax = Syntax.TupleTypeExplicitSpecifier x; value = v } ->
      ( v,
        {
          tuple_type_right_angle = validate_token x.tuple_type_right_angle;
          tuple_type_types = validate_simple_type_specifier x.tuple_type_types;
          tuple_type_left_angle = validate_token x.tuple_type_left_angle;
          tuple_type_keyword = validate_token x.tuple_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TupleTypeExplicitSpecifier) s

  and invalidate_tuple_type_explicit_specifier :
      tuple_type_explicit_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TupleTypeExplicitSpecifier
          {
            tuple_type_keyword = invalidate_token x.tuple_type_keyword;
            tuple_type_left_angle = invalidate_token x.tuple_type_left_angle;
            tuple_type_types =
              invalidate_simple_type_specifier x.tuple_type_types;
            tuple_type_right_angle = invalidate_token x.tuple_type_right_angle;
          };
      Syntax.value = v;
    }

  and validate_varray_type_specifier : varray_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.VarrayTypeSpecifier x; value = v } ->
      ( v,
        {
          varray_right_angle = validate_token x.varray_right_angle;
          varray_trailing_comma =
            validate_option_with validate_token x.varray_trailing_comma;
          varray_type = validate_simple_type_specifier x.varray_type;
          varray_left_angle = validate_token x.varray_left_angle;
          varray_keyword = validate_token x.varray_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.VarrayTypeSpecifier) s

  and invalidate_varray_type_specifier : varray_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.VarrayTypeSpecifier
          {
            varray_keyword = invalidate_token x.varray_keyword;
            varray_left_angle = invalidate_token x.varray_left_angle;
            varray_type = invalidate_simple_type_specifier x.varray_type;
            varray_trailing_comma =
              invalidate_option_with invalidate_token x.varray_trailing_comma;
            varray_right_angle = invalidate_token x.varray_right_angle;
          };
      Syntax.value = v;
    }

  and validate_function_ctx_type_specifier :
      function_ctx_type_specifier validator = function
    | { Syntax.syntax = Syntax.FunctionCtxTypeSpecifier x; value = v } ->
      ( v,
        {
          function_ctx_type_variable =
            validate_variable_expression x.function_ctx_type_variable;
          function_ctx_type_keyword = validate_token x.function_ctx_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.FunctionCtxTypeSpecifier) s

  and invalidate_function_ctx_type_specifier :
      function_ctx_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FunctionCtxTypeSpecifier
          {
            function_ctx_type_keyword =
              invalidate_token x.function_ctx_type_keyword;
            function_ctx_type_variable =
              invalidate_variable_expression x.function_ctx_type_variable;
          };
      Syntax.value = v;
    }

  and validate_type_parameter : type_parameter validator = function
    | { Syntax.syntax = Syntax.TypeParameter x; value = v } ->
      ( v,
        {
          type_constraints =
            validate_list_with validate_type_constraint x.type_constraints;
          type_param_params =
            validate_option_with validate_type_parameters x.type_param_params;
          type_name = validate_token x.type_name;
          type_variance = validate_option_with validate_token x.type_variance;
          type_reified = validate_option_with validate_token x.type_reified;
          type_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.type_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeParameter) s

  and invalidate_type_parameter : type_parameter invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeParameter
          {
            type_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.type_attribute_spec;
            type_reified =
              invalidate_option_with invalidate_token x.type_reified;
            type_variance =
              invalidate_option_with invalidate_token x.type_variance;
            type_name = invalidate_token x.type_name;
            type_param_params =
              invalidate_option_with
                invalidate_type_parameters
                x.type_param_params;
            type_constraints =
              invalidate_list_with invalidate_type_constraint x.type_constraints;
          };
      Syntax.value = v;
    }

  and validate_type_constraint : type_constraint validator = function
    | { Syntax.syntax = Syntax.TypeConstraint x; value = v } ->
      ( v,
        {
          constraint_type = validate_specifier x.constraint_type;
          constraint_keyword = validate_token x.constraint_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeConstraint) s

  and invalidate_type_constraint : type_constraint invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeConstraint
          {
            constraint_keyword = invalidate_token x.constraint_keyword;
            constraint_type = invalidate_specifier x.constraint_type;
          };
      Syntax.value = v;
    }

  and validate_context_constraint : context_constraint validator = function
    | { Syntax.syntax = Syntax.ContextConstraint x; value = v } ->
      ( v,
        {
          ctx_constraint_ctx_list =
            validate_option_with validate_contexts x.ctx_constraint_ctx_list;
          ctx_constraint_keyword = validate_token x.ctx_constraint_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ContextConstraint) s

  and invalidate_context_constraint : context_constraint invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ContextConstraint
          {
            ctx_constraint_keyword = invalidate_token x.ctx_constraint_keyword;
            ctx_constraint_ctx_list =
              invalidate_option_with
                invalidate_contexts
                x.ctx_constraint_ctx_list;
          };
      Syntax.value = v;
    }

  and validate_darray_type_specifier : darray_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.DarrayTypeSpecifier x; value = v } ->
      ( v,
        {
          darray_right_angle = validate_token x.darray_right_angle;
          darray_trailing_comma =
            validate_option_with validate_token x.darray_trailing_comma;
          darray_value = validate_simple_type_specifier x.darray_value;
          darray_comma = validate_token x.darray_comma;
          darray_key = validate_simple_type_specifier x.darray_key;
          darray_left_angle = validate_token x.darray_left_angle;
          darray_keyword = validate_token x.darray_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DarrayTypeSpecifier) s

  and invalidate_darray_type_specifier : darray_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DarrayTypeSpecifier
          {
            darray_keyword = invalidate_token x.darray_keyword;
            darray_left_angle = invalidate_token x.darray_left_angle;
            darray_key = invalidate_simple_type_specifier x.darray_key;
            darray_comma = invalidate_token x.darray_comma;
            darray_value = invalidate_simple_type_specifier x.darray_value;
            darray_trailing_comma =
              invalidate_option_with invalidate_token x.darray_trailing_comma;
            darray_right_angle = invalidate_token x.darray_right_angle;
          };
      Syntax.value = v;
    }

  and validate_dictionary_type_specifier : dictionary_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.DictionaryTypeSpecifier x; value = v } ->
      ( v,
        {
          dictionary_type_right_angle =
            validate_token x.dictionary_type_right_angle;
          dictionary_type_members =
            validate_list_with validate_specifier x.dictionary_type_members;
          dictionary_type_left_angle =
            validate_token x.dictionary_type_left_angle;
          dictionary_type_keyword = validate_token x.dictionary_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.DictionaryTypeSpecifier) s

  and invalidate_dictionary_type_specifier :
      dictionary_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.DictionaryTypeSpecifier
          {
            dictionary_type_keyword = invalidate_token x.dictionary_type_keyword;
            dictionary_type_left_angle =
              invalidate_token x.dictionary_type_left_angle;
            dictionary_type_members =
              invalidate_list_with
                invalidate_specifier
                x.dictionary_type_members;
            dictionary_type_right_angle =
              invalidate_token x.dictionary_type_right_angle;
          };
      Syntax.value = v;
    }

  and validate_closure_type_specifier : closure_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.ClosureTypeSpecifier x; value = v } ->
      ( v,
        {
          closure_outer_right_paren = validate_token x.closure_outer_right_paren;
          closure_return_type = validate_specifier x.closure_return_type;
          closure_readonly_return =
            validate_option_with validate_token x.closure_readonly_return;
          closure_colon = validate_token x.closure_colon;
          closure_contexts =
            validate_option_with validate_contexts x.closure_contexts;
          closure_inner_right_paren = validate_token x.closure_inner_right_paren;
          closure_parameter_list =
            validate_list_with
              validate_closure_parameter_type_specifier
              x.closure_parameter_list;
          closure_inner_left_paren = validate_token x.closure_inner_left_paren;
          closure_function_keyword = validate_token x.closure_function_keyword;
          closure_readonly_keyword =
            validate_option_with validate_token x.closure_readonly_keyword;
          closure_outer_left_paren = validate_token x.closure_outer_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.ClosureTypeSpecifier) s

  and invalidate_closure_type_specifier : closure_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ClosureTypeSpecifier
          {
            closure_outer_left_paren =
              invalidate_token x.closure_outer_left_paren;
            closure_readonly_keyword =
              invalidate_option_with invalidate_token x.closure_readonly_keyword;
            closure_function_keyword =
              invalidate_token x.closure_function_keyword;
            closure_inner_left_paren =
              invalidate_token x.closure_inner_left_paren;
            closure_parameter_list =
              invalidate_list_with
                invalidate_closure_parameter_type_specifier
                x.closure_parameter_list;
            closure_inner_right_paren =
              invalidate_token x.closure_inner_right_paren;
            closure_contexts =
              invalidate_option_with invalidate_contexts x.closure_contexts;
            closure_colon = invalidate_token x.closure_colon;
            closure_readonly_return =
              invalidate_option_with invalidate_token x.closure_readonly_return;
            closure_return_type = invalidate_specifier x.closure_return_type;
            closure_outer_right_paren =
              invalidate_token x.closure_outer_right_paren;
          };
      Syntax.value = v;
    }

  and validate_closure_parameter_type_specifier :
      closure_parameter_type_specifier validator = function
    | { Syntax.syntax = Syntax.ClosureParameterTypeSpecifier x; value = v } ->
      ( v,
        {
          closure_parameter_type = validate_specifier x.closure_parameter_type;
          closure_parameter_readonly =
            validate_option_with validate_token x.closure_parameter_readonly;
          closure_parameter_call_convention =
            validate_option_with
              validate_token
              x.closure_parameter_call_convention;
        } )
    | s -> validation_fail (Some SyntaxKind.ClosureParameterTypeSpecifier) s

  and invalidate_closure_parameter_type_specifier :
      closure_parameter_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ClosureParameterTypeSpecifier
          {
            closure_parameter_call_convention =
              invalidate_option_with
                invalidate_token
                x.closure_parameter_call_convention;
            closure_parameter_readonly =
              invalidate_option_with
                invalidate_token
                x.closure_parameter_readonly;
            closure_parameter_type =
              invalidate_specifier x.closure_parameter_type;
          };
      Syntax.value = v;
    }

  and validate_classname_type_specifier : classname_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.ClassnameTypeSpecifier x; value = v } ->
      ( v,
        {
          classname_right_angle = validate_token x.classname_right_angle;
          classname_trailing_comma =
            validate_option_with validate_token x.classname_trailing_comma;
          classname_type = validate_specifier x.classname_type;
          classname_left_angle = validate_token x.classname_left_angle;
          classname_keyword = validate_token x.classname_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ClassnameTypeSpecifier) s

  and invalidate_classname_type_specifier : classname_type_specifier invalidator
      =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ClassnameTypeSpecifier
          {
            classname_keyword = invalidate_token x.classname_keyword;
            classname_left_angle = invalidate_token x.classname_left_angle;
            classname_type = invalidate_specifier x.classname_type;
            classname_trailing_comma =
              invalidate_option_with invalidate_token x.classname_trailing_comma;
            classname_right_angle = invalidate_token x.classname_right_angle;
          };
      Syntax.value = v;
    }

  and validate_field_specifier : field_specifier validator = function
    | { Syntax.syntax = Syntax.FieldSpecifier x; value = v } ->
      ( v,
        {
          field_type = validate_specifier x.field_type;
          field_arrow = validate_token x.field_arrow;
          field_name = validate_expression x.field_name;
          field_question = validate_option_with validate_token x.field_question;
        } )
    | s -> validation_fail (Some SyntaxKind.FieldSpecifier) s

  and invalidate_field_specifier : field_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FieldSpecifier
          {
            field_question =
              invalidate_option_with invalidate_token x.field_question;
            field_name = invalidate_expression x.field_name;
            field_arrow = invalidate_token x.field_arrow;
            field_type = invalidate_specifier x.field_type;
          };
      Syntax.value = v;
    }

  and validate_field_initializer : field_initializer validator = function
    | { Syntax.syntax = Syntax.FieldInitializer x; value = v } ->
      ( v,
        {
          field_initializer_value =
            validate_expression x.field_initializer_value;
          field_initializer_arrow = validate_token x.field_initializer_arrow;
          field_initializer_name = validate_expression x.field_initializer_name;
        } )
    | s -> validation_fail (Some SyntaxKind.FieldInitializer) s

  and invalidate_field_initializer : field_initializer invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.FieldInitializer
          {
            field_initializer_name =
              invalidate_expression x.field_initializer_name;
            field_initializer_arrow = invalidate_token x.field_initializer_arrow;
            field_initializer_value =
              invalidate_expression x.field_initializer_value;
          };
      Syntax.value = v;
    }

  and validate_shape_type_specifier : shape_type_specifier validator = function
    | { Syntax.syntax = Syntax.ShapeTypeSpecifier x; value = v } ->
      ( v,
        {
          shape_type_right_paren = validate_token x.shape_type_right_paren;
          shape_type_ellipsis =
            validate_option_with validate_token x.shape_type_ellipsis;
          shape_type_fields =
            validate_list_with validate_field_specifier x.shape_type_fields;
          shape_type_left_paren = validate_token x.shape_type_left_paren;
          shape_type_keyword = validate_token x.shape_type_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ShapeTypeSpecifier) s

  and invalidate_shape_type_specifier : shape_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ShapeTypeSpecifier
          {
            shape_type_keyword = invalidate_token x.shape_type_keyword;
            shape_type_left_paren = invalidate_token x.shape_type_left_paren;
            shape_type_fields =
              invalidate_list_with
                invalidate_field_specifier
                x.shape_type_fields;
            shape_type_ellipsis =
              invalidate_option_with invalidate_token x.shape_type_ellipsis;
            shape_type_right_paren = invalidate_token x.shape_type_right_paren;
          };
      Syntax.value = v;
    }

  and validate_shape_expression : shape_expression validator = function
    | { Syntax.syntax = Syntax.ShapeExpression x; value = v } ->
      ( v,
        {
          shape_expression_right_paren =
            validate_token x.shape_expression_right_paren;
          shape_expression_fields =
            validate_list_with
              validate_field_initializer
              x.shape_expression_fields;
          shape_expression_left_paren =
            validate_token x.shape_expression_left_paren;
          shape_expression_keyword = validate_token x.shape_expression_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.ShapeExpression) s

  and invalidate_shape_expression : shape_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ShapeExpression
          {
            shape_expression_keyword =
              invalidate_token x.shape_expression_keyword;
            shape_expression_left_paren =
              invalidate_token x.shape_expression_left_paren;
            shape_expression_fields =
              invalidate_list_with
                invalidate_field_initializer
                x.shape_expression_fields;
            shape_expression_right_paren =
              invalidate_token x.shape_expression_right_paren;
          };
      Syntax.value = v;
    }

  and validate_tuple_expression : tuple_expression validator = function
    | { Syntax.syntax = Syntax.TupleExpression x; value = v } ->
      ( v,
        {
          tuple_expression_right_paren =
            validate_token x.tuple_expression_right_paren;
          tuple_expression_items =
            validate_list_with validate_expression x.tuple_expression_items;
          tuple_expression_left_paren =
            validate_token x.tuple_expression_left_paren;
          tuple_expression_keyword = validate_token x.tuple_expression_keyword;
        } )
    | s -> validation_fail (Some SyntaxKind.TupleExpression) s

  and invalidate_tuple_expression : tuple_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TupleExpression
          {
            tuple_expression_keyword =
              invalidate_token x.tuple_expression_keyword;
            tuple_expression_left_paren =
              invalidate_token x.tuple_expression_left_paren;
            tuple_expression_items =
              invalidate_list_with
                invalidate_expression
                x.tuple_expression_items;
            tuple_expression_right_paren =
              invalidate_token x.tuple_expression_right_paren;
          };
      Syntax.value = v;
    }

  and validate_generic_type_specifier : generic_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.GenericTypeSpecifier x; value = v } ->
      ( v,
        {
          generic_argument_list =
            validate_type_arguments x.generic_argument_list;
          generic_class_type = validate_token x.generic_class_type;
        } )
    | s -> validation_fail (Some SyntaxKind.GenericTypeSpecifier) s

  and invalidate_generic_type_specifier : generic_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.GenericTypeSpecifier
          {
            generic_class_type = invalidate_token x.generic_class_type;
            generic_argument_list =
              invalidate_type_arguments x.generic_argument_list;
          };
      Syntax.value = v;
    }

  and validate_nullable_type_specifier : nullable_type_specifier validator =
    function
    | { Syntax.syntax = Syntax.NullableTypeSpecifier x; value = v } ->
      ( v,
        {
          nullable_type = validate_specifier x.nullable_type;
          nullable_question = validate_token x.nullable_question;
        } )
    | s -> validation_fail (Some SyntaxKind.NullableTypeSpecifier) s

  and invalidate_nullable_type_specifier : nullable_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.NullableTypeSpecifier
          {
            nullable_question = invalidate_token x.nullable_question;
            nullable_type = invalidate_specifier x.nullable_type;
          };
      Syntax.value = v;
    }

  and validate_like_type_specifier : like_type_specifier validator = function
    | { Syntax.syntax = Syntax.LikeTypeSpecifier x; value = v } ->
      ( v,
        {
          like_type = validate_specifier x.like_type;
          like_tilde = validate_token x.like_tilde;
        } )
    | s -> validation_fail (Some SyntaxKind.LikeTypeSpecifier) s

  and invalidate_like_type_specifier : like_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.LikeTypeSpecifier
          {
            like_tilde = invalidate_token x.like_tilde;
            like_type = invalidate_specifier x.like_type;
          };
      Syntax.value = v;
    }

  and validate_soft_type_specifier : soft_type_specifier validator = function
    | { Syntax.syntax = Syntax.SoftTypeSpecifier x; value = v } ->
      ( v,
        {
          soft_type = validate_specifier x.soft_type;
          soft_at = validate_token x.soft_at;
        } )
    | s -> validation_fail (Some SyntaxKind.SoftTypeSpecifier) s

  and invalidate_soft_type_specifier : soft_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.SoftTypeSpecifier
          {
            soft_at = invalidate_token x.soft_at;
            soft_type = invalidate_specifier x.soft_type;
          };
      Syntax.value = v;
    }

  and validate_attributized_specifier : attributized_specifier validator =
    function
    | { Syntax.syntax = Syntax.AttributizedSpecifier x; value = v } ->
      ( v,
        {
          attributized_specifier_type =
            validate_specifier x.attributized_specifier_type;
          attributized_specifier_attribute_spec =
            validate_option_with
              validate_attribute_specification
              x.attributized_specifier_attribute_spec;
        } )
    | s -> validation_fail (Some SyntaxKind.AttributizedSpecifier) s

  and invalidate_attributized_specifier : attributized_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.AttributizedSpecifier
          {
            attributized_specifier_attribute_spec =
              invalidate_option_with
                invalidate_attribute_specification
                x.attributized_specifier_attribute_spec;
            attributized_specifier_type =
              invalidate_specifier x.attributized_specifier_type;
          };
      Syntax.value = v;
    }

  and validate_reified_type_argument : reified_type_argument validator =
    function
    | { Syntax.syntax = Syntax.ReifiedTypeArgument x; value = v } ->
      ( v,
        {
          reified_type_argument_type =
            validate_specifier x.reified_type_argument_type;
          reified_type_argument_reified =
            validate_token x.reified_type_argument_reified;
        } )
    | s -> validation_fail (Some SyntaxKind.ReifiedTypeArgument) s

  and invalidate_reified_type_argument : reified_type_argument invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.ReifiedTypeArgument
          {
            reified_type_argument_reified =
              invalidate_token x.reified_type_argument_reified;
            reified_type_argument_type =
              invalidate_specifier x.reified_type_argument_type;
          };
      Syntax.value = v;
    }

  and validate_type_arguments : type_arguments validator = function
    | { Syntax.syntax = Syntax.TypeArguments x; value = v } ->
      ( v,
        {
          type_arguments_right_angle =
            validate_token x.type_arguments_right_angle;
          type_arguments_types =
            validate_list_with
              validate_attributized_specifier
              x.type_arguments_types;
          type_arguments_left_angle = validate_token x.type_arguments_left_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeArguments) s

  and invalidate_type_arguments : type_arguments invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeArguments
          {
            type_arguments_left_angle =
              invalidate_token x.type_arguments_left_angle;
            type_arguments_types =
              invalidate_list_with
                invalidate_attributized_specifier
                x.type_arguments_types;
            type_arguments_right_angle =
              invalidate_token x.type_arguments_right_angle;
          };
      Syntax.value = v;
    }

  and validate_type_parameters : type_parameters validator = function
    | { Syntax.syntax = Syntax.TypeParameters x; value = v } ->
      ( v,
        {
          type_parameters_right_angle =
            validate_token x.type_parameters_right_angle;
          type_parameters_parameters =
            validate_list_with
              validate_type_parameter
              x.type_parameters_parameters;
          type_parameters_left_angle =
            validate_token x.type_parameters_left_angle;
        } )
    | s -> validation_fail (Some SyntaxKind.TypeParameters) s

  and invalidate_type_parameters : type_parameters invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TypeParameters
          {
            type_parameters_left_angle =
              invalidate_token x.type_parameters_left_angle;
            type_parameters_parameters =
              invalidate_list_with
                invalidate_type_parameter
                x.type_parameters_parameters;
            type_parameters_right_angle =
              invalidate_token x.type_parameters_right_angle;
          };
      Syntax.value = v;
    }

  and validate_tuple_type_specifier : tuple_type_specifier validator = function
    | { Syntax.syntax = Syntax.TupleTypeSpecifier x; value = v } ->
      ( v,
        {
          tuple_right_paren = validate_token x.tuple_right_paren;
          tuple_types = validate_list_with validate_specifier x.tuple_types;
          tuple_left_paren = validate_token x.tuple_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.TupleTypeSpecifier) s

  and invalidate_tuple_type_specifier : tuple_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.TupleTypeSpecifier
          {
            tuple_left_paren = invalidate_token x.tuple_left_paren;
            tuple_types =
              invalidate_list_with invalidate_specifier x.tuple_types;
            tuple_right_paren = invalidate_token x.tuple_right_paren;
          };
      Syntax.value = v;
    }

  and validate_union_type_specifier : union_type_specifier validator = function
    | { Syntax.syntax = Syntax.UnionTypeSpecifier x; value = v } ->
      ( v,
        {
          union_right_paren = validate_token x.union_right_paren;
          union_types = validate_list_with validate_specifier x.union_types;
          union_left_paren = validate_token x.union_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.UnionTypeSpecifier) s

  and invalidate_union_type_specifier : union_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.UnionTypeSpecifier
          {
            union_left_paren = invalidate_token x.union_left_paren;
            union_types =
              invalidate_list_with invalidate_specifier x.union_types;
            union_right_paren = invalidate_token x.union_right_paren;
          };
      Syntax.value = v;
    }

  and validate_intersection_type_specifier :
      intersection_type_specifier validator = function
    | { Syntax.syntax = Syntax.IntersectionTypeSpecifier x; value = v } ->
      ( v,
        {
          intersection_right_paren = validate_token x.intersection_right_paren;
          intersection_types =
            validate_list_with validate_specifier x.intersection_types;
          intersection_left_paren = validate_token x.intersection_left_paren;
        } )
    | s -> validation_fail (Some SyntaxKind.IntersectionTypeSpecifier) s

  and invalidate_intersection_type_specifier :
      intersection_type_specifier invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.IntersectionTypeSpecifier
          {
            intersection_left_paren = invalidate_token x.intersection_left_paren;
            intersection_types =
              invalidate_list_with invalidate_specifier x.intersection_types;
            intersection_right_paren =
              invalidate_token x.intersection_right_paren;
          };
      Syntax.value = v;
    }

  and validate_enum_class_label_expression :
      enum_class_label_expression validator = function
    | { Syntax.syntax = Syntax.EnumClassLabelExpression x; value = v } ->
      ( v,
        {
          enum_class_label_expression =
            validate_token x.enum_class_label_expression;
          enum_class_label_hash = validate_token x.enum_class_label_hash;
          enum_class_label_qualifier =
            validate_option_with
              validate_expression
              x.enum_class_label_qualifier;
        } )
    | s -> validation_fail (Some SyntaxKind.EnumClassLabelExpression) s

  and invalidate_enum_class_label_expression :
      enum_class_label_expression invalidator =
   fun (v, x) ->
    {
      Syntax.syntax =
        Syntax.EnumClassLabelExpression
          {
            enum_class_label_qualifier =
              invalidate_option_with
                invalidate_expression
                x.enum_class_label_qualifier;
            enum_class_label_hash = invalidate_token x.enum_class_label_hash;
            enum_class_label_expression =
              invalidate_token x.enum_class_label_expression;
          };
      Syntax.value = v;
    }
end
