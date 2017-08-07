(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 * This module contains the type describing the structure of a syntax tree.
 *
 **
 *
 * This module contains the functions to (in)validate syntax trees.
 *)

open Full_fidelity_syntax_type (* module signatures of the functor *)
module SyntaxKind = Full_fidelity_syntax_kind
module Def = Schema_definition

module Make(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  module SyntaxBase = Full_fidelity_syntax.WithToken(Token)
  module Syntax = SyntaxBase.WithSyntaxValue(SyntaxValue)
  module Validated = MakeValidated(Token)(SyntaxValue)
  open Validated

  type 'a validator = Syntax.t -> 'a value
  type 'a invalidator = 'a value -> Syntax.t

  exception Validation_failure of SyntaxKind.t * Syntax.t
  let validation_fail k t = raise (Validation_failure (k, t))

  exception Aggregation_failure of Def.aggregate_type * Syntax.syntax
  let aggregation_fail a s =
    Printf.eprintf "Aggregation failure: For %s not expecting %s\n"
      (Schema_definition.string_of_aggregate_type a)
      (SyntaxKind.to_string @@ Syntax.to_kind s);
    raise (Aggregation_failure (a, s))

  let validate_option_with : 'a . 'a validator -> 'a option validator =
    fun validate node ->
      match Syntax.syntax node with
      | Syntax.Missing -> Syntax.value node, None
      | _ -> let value, result = validate node in value, Some result
  let invalidate_option_with : 'a . 'a invalidator -> 'a option invalidator =
    fun invalidate (value, thing) -> match thing with
    | Some real_thing -> invalidate (value, real_thing)
    | None -> { Syntax.syntax = Syntax.Missing; value }

  let validate_token : Token.t validator = fun node ->
    match Syntax.syntax node with
    | Syntax.Token t -> Syntax.value node, t
    | _ -> validation_fail SyntaxKind.Token node
  let invalidate_token : Token.t invalidator = fun (value, token) ->
    { Syntax.syntax = Syntax.Token token; value }

  let validate_list_with : 'a . 'a validator -> 'a listesque validator =
    fun validate node ->
      let validate_item i =
        match Syntax.syntax i with
        | Syntax.ListItem { Syntax.list_item; list_separator } ->
          let item = validate list_item in
          let separator = validate_option_with validate_token list_separator in
          i.Syntax.value, (item, separator)
        | _ -> validation_fail SyntaxKind.ListItem i
      in
      let validate_list l =
        try Syntactic (List.map validate_item l) with
        | Validation_failure (SyntaxKind.ListItem, _) ->
          NonSyntactic (List.map validate l)
      in
      let result =
        match Syntax.syntax node with
        | Syntax.SyntaxList l -> validate_list l
        | Syntax.Missing -> MissingList
        | _ -> SingletonList (validate node)
      in
      node.Syntax.value, result

  let invalidate_list_with : 'a . 'a invalidator -> 'a listesque invalidator =
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
          { Syntax.syntax = Syntax.ListItem
            { Syntax.list_item = inode; list_separator = iseparator }
          ; value
          }
        in
        { Syntax.syntax = Syntax.SyntaxList (List.map mapper nodes); value }

  let rec tag : 'a 'b . 'a validator -> ('a -> 'b) -> 'b validator =
    (* Validating aggregate types means picking the right validator for the
     * expected/valid variants and then tagging the result with the constructor
     * corresponding to the variant. This is a repetative pattern. Explicit
     * polymorphism saves us this trouble.
     *)
    fun validator projection node ->
      let value, node = validator node in
      value, projection node
  and validate_top_level_declaration : top_level_declaration validator = fun x ->
    match Syntax.syntax x with
    | Syntax.EndOfFile _ -> tag validate_end_of_file (fun x -> TLDEndOfFile x) x
    | Syntax.EnumDeclaration _ -> tag validate_enum_declaration (fun x -> TLDEnum x) x
    | Syntax.AliasDeclaration _ -> tag validate_alias_declaration (fun x -> TLDAlias x) x
    | Syntax.NamespaceDeclaration _ -> tag validate_namespace_declaration (fun x -> TLDNamespace x) x
    | Syntax.NamespaceUseDeclaration _ -> tag validate_namespace_use_declaration (fun x -> TLDNamespaceUse x) x
    | Syntax.NamespaceGroupUseDeclaration _ -> tag validate_namespace_group_use_declaration (fun x -> TLDNamespaceGroupUse x) x
    | Syntax.FunctionDeclaration _ -> tag validate_function_declaration (fun x -> TLDFunction x) x
    | Syntax.ClassishDeclaration _ -> tag validate_classish_declaration (fun x -> TLDClassish x) x
    | Syntax.ConstDeclaration _ -> tag validate_const_declaration (fun x -> TLDConst x) x
    | Syntax.InclusionDirective _ -> tag validate_inclusion_directive (fun x -> TLDInclusionDirective x) x
    | Syntax.CompoundStatement _ -> tag validate_compound_statement (fun x -> TLDCompound x) x
    | Syntax.ExpressionStatement _ -> tag validate_expression_statement (fun x -> TLDExpression x) x
    | Syntax.MarkupSection _ -> tag validate_markup_section (fun x -> TLDMarkupSection x) x
    | Syntax.MarkupSuffix _ -> tag validate_markup_suffix (fun x -> TLDMarkupSuffix x) x
    | Syntax.UnsetStatement _ -> tag validate_unset_statement (fun x -> TLDUnset x) x
    | Syntax.WhileStatement _ -> tag validate_while_statement (fun x -> TLDWhile x) x
    | Syntax.IfStatement _ -> tag validate_if_statement (fun x -> TLDIf x) x
    | Syntax.TryStatement _ -> tag validate_try_statement (fun x -> TLDTry x) x
    | Syntax.DoStatement _ -> tag validate_do_statement (fun x -> TLDDo x) x
    | Syntax.ForStatement _ -> tag validate_for_statement (fun x -> TLDFor x) x
    | Syntax.ForeachStatement _ -> tag validate_foreach_statement (fun x -> TLDForeach x) x
    | Syntax.SwitchFallthrough _ -> tag validate_switch_fallthrough (fun x -> TLDSwitchFallthrough x) x
    | Syntax.ReturnStatement _ -> tag validate_return_statement (fun x -> TLDReturn x) x
    | Syntax.GotoLabel _ -> tag validate_goto_label (fun x -> TLDGotoLabel x) x
    | Syntax.GotoStatement _ -> tag validate_goto_statement (fun x -> TLDGoto x) x
    | Syntax.ThrowStatement _ -> tag validate_throw_statement (fun x -> TLDThrow x) x
    | Syntax.BreakStatement _ -> tag validate_break_statement (fun x -> TLDBreak x) x
    | Syntax.ContinueStatement _ -> tag validate_continue_statement (fun x -> TLDContinue x) x
    | Syntax.FunctionStaticStatement _ -> tag validate_function_static_statement (fun x -> TLDFunctionStatic x) x
    | Syntax.EchoStatement _ -> tag validate_echo_statement (fun x -> TLDEcho x) x
    | Syntax.GlobalStatement _ -> tag validate_global_statement (fun x -> TLDGlobal x) x
    | s -> aggregation_fail Def.TopLevelDeclaration s
  and invalidate_top_level_declaration : top_level_declaration invalidator = fun (value, thing) ->
    match thing with
    | TLDEndOfFile          thing -> invalidate_end_of_file                    (value, thing)
    | TLDEnum               thing -> invalidate_enum_declaration               (value, thing)
    | TLDAlias              thing -> invalidate_alias_declaration              (value, thing)
    | TLDNamespace          thing -> invalidate_namespace_declaration          (value, thing)
    | TLDNamespaceUse       thing -> invalidate_namespace_use_declaration      (value, thing)
    | TLDNamespaceGroupUse  thing -> invalidate_namespace_group_use_declaration (value, thing)
    | TLDFunction           thing -> invalidate_function_declaration           (value, thing)
    | TLDClassish           thing -> invalidate_classish_declaration           (value, thing)
    | TLDConst              thing -> invalidate_const_declaration              (value, thing)
    | TLDInclusionDirective thing -> invalidate_inclusion_directive            (value, thing)
    | TLDCompound           thing -> invalidate_compound_statement             (value, thing)
    | TLDExpression         thing -> invalidate_expression_statement           (value, thing)
    | TLDMarkupSection      thing -> invalidate_markup_section                 (value, thing)
    | TLDMarkupSuffix       thing -> invalidate_markup_suffix                  (value, thing)
    | TLDUnset              thing -> invalidate_unset_statement                (value, thing)
    | TLDWhile              thing -> invalidate_while_statement                (value, thing)
    | TLDIf                 thing -> invalidate_if_statement                   (value, thing)
    | TLDTry                thing -> invalidate_try_statement                  (value, thing)
    | TLDDo                 thing -> invalidate_do_statement                   (value, thing)
    | TLDFor                thing -> invalidate_for_statement                  (value, thing)
    | TLDForeach            thing -> invalidate_foreach_statement              (value, thing)
    | TLDSwitchFallthrough  thing -> invalidate_switch_fallthrough             (value, thing)
    | TLDReturn             thing -> invalidate_return_statement               (value, thing)
    | TLDGotoLabel          thing -> invalidate_goto_label                     (value, thing)
    | TLDGoto               thing -> invalidate_goto_statement                 (value, thing)
    | TLDThrow              thing -> invalidate_throw_statement                (value, thing)
    | TLDBreak              thing -> invalidate_break_statement                (value, thing)
    | TLDContinue           thing -> invalidate_continue_statement             (value, thing)
    | TLDFunctionStatic     thing -> invalidate_function_static_statement      (value, thing)
    | TLDEcho               thing -> invalidate_echo_statement                 (value, thing)
    | TLDGlobal             thing -> invalidate_global_statement               (value, thing)
  and validate_expression : expression validator = fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ -> tag validate_literal_expression (fun x -> ExprLiteral x) x
    | Syntax.VariableExpression _ -> tag validate_variable_expression (fun x -> ExprVariable x) x
    | Syntax.QualifiedNameExpression _ -> tag validate_qualified_name_expression (fun x -> ExprQualifiedName x) x
    | Syntax.PipeVariableExpression _ -> tag validate_pipe_variable_expression (fun x -> ExprPipeVariable x) x
    | Syntax.DecoratedExpression _ -> tag validate_decorated_expression (fun x -> ExprDecorated x) x
    | Syntax.InclusionExpression _ -> tag validate_inclusion_expression (fun x -> ExprInclusion x) x
    | Syntax.AnonymousFunction _ -> tag validate_anonymous_function (fun x -> ExprAnonymousFunction x) x
    | Syntax.LambdaExpression _ -> tag validate_lambda_expression (fun x -> ExprLambda x) x
    | Syntax.CastExpression _ -> tag validate_cast_expression (fun x -> ExprCast x) x
    | Syntax.ScopeResolutionExpression _ -> tag validate_scope_resolution_expression (fun x -> ExprScopeResolution x) x
    | Syntax.MemberSelectionExpression _ -> tag validate_member_selection_expression (fun x -> ExprMemberSelection x) x
    | Syntax.SafeMemberSelectionExpression _ -> tag validate_safe_member_selection_expression (fun x -> ExprSafeMemberSelection x) x
    | Syntax.EmbeddedMemberSelectionExpression _ -> tag validate_embedded_member_selection_expression (fun x -> ExprEmbeddedMemberSelection x) x
    | Syntax.YieldExpression _ -> tag validate_yield_expression (fun x -> ExprYield x) x
    | Syntax.YieldFromExpression _ -> tag validate_yield_from_expression (fun x -> ExprYieldFrom x) x
    | Syntax.PrefixUnaryExpression _ -> tag validate_prefix_unary_expression (fun x -> ExprPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ -> tag validate_postfix_unary_expression (fun x -> ExprPostfixUnary x) x
    | Syntax.BinaryExpression _ -> tag validate_binary_expression (fun x -> ExprBinary x) x
    | Syntax.InstanceofExpression _ -> tag validate_instanceof_expression (fun x -> ExprInstanceof x) x
    | Syntax.ConditionalExpression _ -> tag validate_conditional_expression (fun x -> ExprConditional x) x
    | Syntax.EvalExpression _ -> tag validate_eval_expression (fun x -> ExprEval x) x
    | Syntax.EmptyExpression _ -> tag validate_empty_expression (fun x -> ExprEmpty x) x
    | Syntax.DefineExpression _ -> tag validate_define_expression (fun x -> ExprDefine x) x
    | Syntax.IssetExpression _ -> tag validate_isset_expression (fun x -> ExprIsset x) x
    | Syntax.FunctionCallExpression _ -> tag validate_function_call_expression (fun x -> ExprFunctionCall x) x
    | Syntax.FunctionCallWithTypeArgumentsExpression _ -> tag validate_function_call_with_type_arguments_expression (fun x -> ExprFunctionCallWithTypeArguments x) x
    | Syntax.ParenthesizedExpression _ -> tag validate_parenthesized_expression (fun x -> ExprParenthesized x) x
    | Syntax.BracedExpression _ -> tag validate_braced_expression (fun x -> ExprBraced x) x
    | Syntax.EmbeddedBracedExpression _ -> tag validate_embedded_braced_expression (fun x -> ExprEmbeddedBraced x) x
    | Syntax.ListExpression _ -> tag validate_list_expression (fun x -> ExprList x) x
    | Syntax.CollectionLiteralExpression _ -> tag validate_collection_literal_expression (fun x -> ExprCollectionLiteral x) x
    | Syntax.ObjectCreationExpression _ -> tag validate_object_creation_expression (fun x -> ExprObjectCreation x) x
    | Syntax.ArrayCreationExpression _ -> tag validate_array_creation_expression (fun x -> ExprArrayCreation x) x
    | Syntax.ArrayIntrinsicExpression _ -> tag validate_array_intrinsic_expression (fun x -> ExprArrayIntrinsic x) x
    | Syntax.DarrayIntrinsicExpression _ -> tag validate_darray_intrinsic_expression (fun x -> ExprDarrayIntrinsic x) x
    | Syntax.DictionaryIntrinsicExpression _ -> tag validate_dictionary_intrinsic_expression (fun x -> ExprDictionaryIntrinsic x) x
    | Syntax.KeysetIntrinsicExpression _ -> tag validate_keyset_intrinsic_expression (fun x -> ExprKeysetIntrinsic x) x
    | Syntax.VarrayIntrinsicExpression _ -> tag validate_varray_intrinsic_expression (fun x -> ExprVarrayIntrinsic x) x
    | Syntax.VectorIntrinsicExpression _ -> tag validate_vector_intrinsic_expression (fun x -> ExprVectorIntrinsic x) x
    | Syntax.SubscriptExpression _ -> tag validate_subscript_expression (fun x -> ExprSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ -> tag validate_embedded_subscript_expression (fun x -> ExprEmbeddedSubscript x) x
    | Syntax.AwaitableCreationExpression _ -> tag validate_awaitable_creation_expression (fun x -> ExprAwaitableCreation x) x
    | Syntax.XHPChildrenParenthesizedList _ -> tag validate_xhp_children_parenthesized_list (fun x -> ExprXHPChildrenParenthesizedList x) x
    | Syntax.XHPExpression _ -> tag validate_xhp_expression (fun x -> ExprXHP x) x
    | Syntax.ShapeExpression _ -> tag validate_shape_expression (fun x -> ExprShape x) x
    | Syntax.TupleExpression _ -> tag validate_tuple_expression (fun x -> ExprTuple x) x
    | s -> aggregation_fail Def.Expression s
  and invalidate_expression : expression invalidator = fun (value, thing) ->
    match thing with
    | ExprLiteral                       thing -> invalidate_literal_expression             (value, thing)
    | ExprVariable                      thing -> invalidate_variable_expression            (value, thing)
    | ExprQualifiedName                 thing -> invalidate_qualified_name_expression      (value, thing)
    | ExprPipeVariable                  thing -> invalidate_pipe_variable_expression       (value, thing)
    | ExprDecorated                     thing -> invalidate_decorated_expression           (value, thing)
    | ExprInclusion                     thing -> invalidate_inclusion_expression           (value, thing)
    | ExprAnonymousFunction             thing -> invalidate_anonymous_function             (value, thing)
    | ExprLambda                        thing -> invalidate_lambda_expression              (value, thing)
    | ExprCast                          thing -> invalidate_cast_expression                (value, thing)
    | ExprScopeResolution               thing -> invalidate_scope_resolution_expression    (value, thing)
    | ExprMemberSelection               thing -> invalidate_member_selection_expression    (value, thing)
    | ExprSafeMemberSelection           thing -> invalidate_safe_member_selection_expression (value, thing)
    | ExprEmbeddedMemberSelection       thing -> invalidate_embedded_member_selection_expression (value, thing)
    | ExprYield                         thing -> invalidate_yield_expression               (value, thing)
    | ExprYieldFrom                     thing -> invalidate_yield_from_expression          (value, thing)
    | ExprPrefixUnary                   thing -> invalidate_prefix_unary_expression        (value, thing)
    | ExprPostfixUnary                  thing -> invalidate_postfix_unary_expression       (value, thing)
    | ExprBinary                        thing -> invalidate_binary_expression              (value, thing)
    | ExprInstanceof                    thing -> invalidate_instanceof_expression          (value, thing)
    | ExprConditional                   thing -> invalidate_conditional_expression         (value, thing)
    | ExprEval                          thing -> invalidate_eval_expression                (value, thing)
    | ExprEmpty                         thing -> invalidate_empty_expression               (value, thing)
    | ExprDefine                        thing -> invalidate_define_expression              (value, thing)
    | ExprIsset                         thing -> invalidate_isset_expression               (value, thing)
    | ExprFunctionCall                  thing -> invalidate_function_call_expression       (value, thing)
    | ExprFunctionCallWithTypeArguments thing -> invalidate_function_call_with_type_arguments_expression (value, thing)
    | ExprParenthesized                 thing -> invalidate_parenthesized_expression       (value, thing)
    | ExprBraced                        thing -> invalidate_braced_expression              (value, thing)
    | ExprEmbeddedBraced                thing -> invalidate_embedded_braced_expression     (value, thing)
    | ExprList                          thing -> invalidate_list_expression                (value, thing)
    | ExprCollectionLiteral             thing -> invalidate_collection_literal_expression  (value, thing)
    | ExprObjectCreation                thing -> invalidate_object_creation_expression     (value, thing)
    | ExprArrayCreation                 thing -> invalidate_array_creation_expression      (value, thing)
    | ExprArrayIntrinsic                thing -> invalidate_array_intrinsic_expression     (value, thing)
    | ExprDarrayIntrinsic               thing -> invalidate_darray_intrinsic_expression    (value, thing)
    | ExprDictionaryIntrinsic           thing -> invalidate_dictionary_intrinsic_expression (value, thing)
    | ExprKeysetIntrinsic               thing -> invalidate_keyset_intrinsic_expression    (value, thing)
    | ExprVarrayIntrinsic               thing -> invalidate_varray_intrinsic_expression    (value, thing)
    | ExprVectorIntrinsic               thing -> invalidate_vector_intrinsic_expression    (value, thing)
    | ExprSubscript                     thing -> invalidate_subscript_expression           (value, thing)
    | ExprEmbeddedSubscript             thing -> invalidate_embedded_subscript_expression  (value, thing)
    | ExprAwaitableCreation             thing -> invalidate_awaitable_creation_expression  (value, thing)
    | ExprXHPChildrenParenthesizedList  thing -> invalidate_xhp_children_parenthesized_list (value, thing)
    | ExprXHP                           thing -> invalidate_xhp_expression                 (value, thing)
    | ExprShape                         thing -> invalidate_shape_expression               (value, thing)
    | ExprTuple                         thing -> invalidate_tuple_expression               (value, thing)
  and validate_specifier : specifier validator = fun x ->
    match Syntax.syntax x with
    | Syntax.SimpleTypeSpecifier _ -> tag validate_simple_type_specifier (fun x -> SpecSimple x) x
    | Syntax.VariadicParameter _ -> tag validate_variadic_parameter (fun x -> SpecVariadicParameter x) x
    | Syntax.LambdaSignature _ -> tag validate_lambda_signature (fun x -> SpecLambdaSignature x) x
    | Syntax.XHPEnumType _ -> tag validate_xhp_enum_type (fun x -> SpecXHPEnumType x) x
    | Syntax.VectorTypeSpecifier _ -> tag validate_vector_type_specifier (fun x -> SpecVector x) x
    | Syntax.KeysetTypeSpecifier _ -> tag validate_keyset_type_specifier (fun x -> SpecKeyset x) x
    | Syntax.TupleTypeExplicitSpecifier _ -> tag validate_tuple_type_explicit_specifier (fun x -> SpecTupleTypeExplicit x) x
    | Syntax.VarrayTypeSpecifier _ -> tag validate_varray_type_specifier (fun x -> SpecVarray x) x
    | Syntax.VectorArrayTypeSpecifier _ -> tag validate_vector_array_type_specifier (fun x -> SpecVectorArray x) x
    | Syntax.DarrayTypeSpecifier _ -> tag validate_darray_type_specifier (fun x -> SpecDarray x) x
    | Syntax.MapArrayTypeSpecifier _ -> tag validate_map_array_type_specifier (fun x -> SpecMapArray x) x
    | Syntax.DictionaryTypeSpecifier _ -> tag validate_dictionary_type_specifier (fun x -> SpecDictionary x) x
    | Syntax.ClosureTypeSpecifier _ -> tag validate_closure_type_specifier (fun x -> SpecClosure x) x
    | Syntax.ClassnameTypeSpecifier _ -> tag validate_classname_type_specifier (fun x -> SpecClassname x) x
    | Syntax.FieldSpecifier _ -> tag validate_field_specifier (fun x -> SpecField x) x
    | Syntax.ShapeTypeSpecifier _ -> tag validate_shape_type_specifier (fun x -> SpecShape x) x
    | Syntax.GenericTypeSpecifier _ -> tag validate_generic_type_specifier (fun x -> SpecGeneric x) x
    | Syntax.NullableTypeSpecifier _ -> tag validate_nullable_type_specifier (fun x -> SpecNullable x) x
    | Syntax.SoftTypeSpecifier _ -> tag validate_soft_type_specifier (fun x -> SpecSoft x) x
    | Syntax.TupleTypeSpecifier _ -> tag validate_tuple_type_specifier (fun x -> SpecTuple x) x
    | s -> aggregation_fail Def.Specifier s
  and invalidate_specifier : specifier invalidator = fun (value, thing) ->
    match thing with
    | SpecSimple            thing -> invalidate_simple_type_specifier          (value, thing)
    | SpecVariadicParameter thing -> invalidate_variadic_parameter             (value, thing)
    | SpecLambdaSignature   thing -> invalidate_lambda_signature               (value, thing)
    | SpecXHPEnumType       thing -> invalidate_xhp_enum_type                  (value, thing)
    | SpecVector            thing -> invalidate_vector_type_specifier          (value, thing)
    | SpecKeyset            thing -> invalidate_keyset_type_specifier          (value, thing)
    | SpecTupleTypeExplicit thing -> invalidate_tuple_type_explicit_specifier  (value, thing)
    | SpecVarray            thing -> invalidate_varray_type_specifier          (value, thing)
    | SpecVectorArray       thing -> invalidate_vector_array_type_specifier    (value, thing)
    | SpecDarray            thing -> invalidate_darray_type_specifier          (value, thing)
    | SpecMapArray          thing -> invalidate_map_array_type_specifier       (value, thing)
    | SpecDictionary        thing -> invalidate_dictionary_type_specifier      (value, thing)
    | SpecClosure           thing -> invalidate_closure_type_specifier         (value, thing)
    | SpecClassname         thing -> invalidate_classname_type_specifier       (value, thing)
    | SpecField             thing -> invalidate_field_specifier                (value, thing)
    | SpecShape             thing -> invalidate_shape_type_specifier           (value, thing)
    | SpecGeneric           thing -> invalidate_generic_type_specifier         (value, thing)
    | SpecNullable          thing -> invalidate_nullable_type_specifier        (value, thing)
    | SpecSoft              thing -> invalidate_soft_type_specifier            (value, thing)
    | SpecTuple             thing -> invalidate_tuple_type_specifier           (value, thing)
  and validate_parameter : parameter validator = fun x ->
    match Syntax.syntax x with
    | Syntax.ParameterDeclaration _ -> tag validate_parameter_declaration (fun x -> ParamParameterDeclaration x) x
    | Syntax.VariadicParameter _ -> tag validate_variadic_parameter (fun x -> ParamVariadicParameter x) x
    | s -> aggregation_fail Def.Parameter s
  and invalidate_parameter : parameter invalidator = fun (value, thing) ->
    match thing with
    | ParamParameterDeclaration thing -> invalidate_parameter_declaration          (value, thing)
    | ParamVariadicParameter    thing -> invalidate_variadic_parameter             (value, thing)
  and validate_class_body_declaration : class_body_declaration validator = fun x ->
    match Syntax.syntax x with
    | Syntax.PropertyDeclaration _ -> tag validate_property_declaration (fun x -> BodyProperty x) x
    | Syntax.MethodishDeclaration _ -> tag validate_methodish_declaration (fun x -> BodyMethodish x) x
    | Syntax.RequireClause _ -> tag validate_require_clause (fun x -> BodyRequireClause x) x
    | Syntax.ConstDeclaration _ -> tag validate_const_declaration (fun x -> BodyConst x) x
    | Syntax.TypeConstDeclaration _ -> tag validate_type_const_declaration (fun x -> BodyTypeConst x) x
    | Syntax.XHPChildrenDeclaration _ -> tag validate_xhp_children_declaration (fun x -> BodyXHPChildren x) x
    | Syntax.XHPCategoryDeclaration _ -> tag validate_xhp_category_declaration (fun x -> BodyXHPCategory x) x
    | Syntax.XHPClassAttributeDeclaration _ -> tag validate_xhp_class_attribute_declaration (fun x -> BodyXHPClassAttribute x) x
    | s -> aggregation_fail Def.ClassBodyDeclaration s
  and invalidate_class_body_declaration : class_body_declaration invalidator = fun (value, thing) ->
    match thing with
    | BodyProperty          thing -> invalidate_property_declaration           (value, thing)
    | BodyMethodish         thing -> invalidate_methodish_declaration          (value, thing)
    | BodyRequireClause     thing -> invalidate_require_clause                 (value, thing)
    | BodyConst             thing -> invalidate_const_declaration              (value, thing)
    | BodyTypeConst         thing -> invalidate_type_const_declaration         (value, thing)
    | BodyXHPChildren       thing -> invalidate_xhp_children_declaration       (value, thing)
    | BodyXHPCategory       thing -> invalidate_xhp_category_declaration       (value, thing)
    | BodyXHPClassAttribute thing -> invalidate_xhp_class_attribute_declaration (value, thing)
  and validate_statement : statement validator = fun x ->
    match Syntax.syntax x with
    | Syntax.InclusionDirective _ -> tag validate_inclusion_directive (fun x -> StmtInclusionDirective x) x
    | Syntax.CompoundStatement _ -> tag validate_compound_statement (fun x -> StmtCompound x) x
    | Syntax.ExpressionStatement _ -> tag validate_expression_statement (fun x -> StmtExpression x) x
    | Syntax.MarkupSection _ -> tag validate_markup_section (fun x -> StmtMarkupSection x) x
    | Syntax.MarkupSuffix _ -> tag validate_markup_suffix (fun x -> StmtMarkupSuffix x) x
    | Syntax.UnsetStatement _ -> tag validate_unset_statement (fun x -> StmtUnset x) x
    | Syntax.WhileStatement _ -> tag validate_while_statement (fun x -> StmtWhile x) x
    | Syntax.IfStatement _ -> tag validate_if_statement (fun x -> StmtIf x) x
    | Syntax.TryStatement _ -> tag validate_try_statement (fun x -> StmtTry x) x
    | Syntax.DoStatement _ -> tag validate_do_statement (fun x -> StmtDo x) x
    | Syntax.ForStatement _ -> tag validate_for_statement (fun x -> StmtFor x) x
    | Syntax.ForeachStatement _ -> tag validate_foreach_statement (fun x -> StmtForeach x) x
    | Syntax.SwitchStatement _ -> tag validate_switch_statement (fun x -> StmtSwitch x) x
    | Syntax.SwitchFallthrough _ -> tag validate_switch_fallthrough (fun x -> StmtSwitchFallthrough x) x
    | Syntax.ReturnStatement _ -> tag validate_return_statement (fun x -> StmtReturn x) x
    | Syntax.GotoLabel _ -> tag validate_goto_label (fun x -> StmtGotoLabel x) x
    | Syntax.GotoStatement _ -> tag validate_goto_statement (fun x -> StmtGoto x) x
    | Syntax.ThrowStatement _ -> tag validate_throw_statement (fun x -> StmtThrow x) x
    | Syntax.BreakStatement _ -> tag validate_break_statement (fun x -> StmtBreak x) x
    | Syntax.ContinueStatement _ -> tag validate_continue_statement (fun x -> StmtContinue x) x
    | Syntax.FunctionStaticStatement _ -> tag validate_function_static_statement (fun x -> StmtFunctionStatic x) x
    | Syntax.EchoStatement _ -> tag validate_echo_statement (fun x -> StmtEcho x) x
    | Syntax.GlobalStatement _ -> tag validate_global_statement (fun x -> StmtGlobal x) x
    | Syntax.TypeConstant _ -> tag validate_type_constant (fun x -> StmtTypeConstant x) x
    | s -> aggregation_fail Def.Statement s
  and invalidate_statement : statement invalidator = fun (value, thing) ->
    match thing with
    | StmtInclusionDirective thing -> invalidate_inclusion_directive            (value, thing)
    | StmtCompound           thing -> invalidate_compound_statement             (value, thing)
    | StmtExpression         thing -> invalidate_expression_statement           (value, thing)
    | StmtMarkupSection      thing -> invalidate_markup_section                 (value, thing)
    | StmtMarkupSuffix       thing -> invalidate_markup_suffix                  (value, thing)
    | StmtUnset              thing -> invalidate_unset_statement                (value, thing)
    | StmtWhile              thing -> invalidate_while_statement                (value, thing)
    | StmtIf                 thing -> invalidate_if_statement                   (value, thing)
    | StmtTry                thing -> invalidate_try_statement                  (value, thing)
    | StmtDo                 thing -> invalidate_do_statement                   (value, thing)
    | StmtFor                thing -> invalidate_for_statement                  (value, thing)
    | StmtForeach            thing -> invalidate_foreach_statement              (value, thing)
    | StmtSwitch             thing -> invalidate_switch_statement               (value, thing)
    | StmtSwitchFallthrough  thing -> invalidate_switch_fallthrough             (value, thing)
    | StmtReturn             thing -> invalidate_return_statement               (value, thing)
    | StmtGotoLabel          thing -> invalidate_goto_label                     (value, thing)
    | StmtGoto               thing -> invalidate_goto_statement                 (value, thing)
    | StmtThrow              thing -> invalidate_throw_statement                (value, thing)
    | StmtBreak              thing -> invalidate_break_statement                (value, thing)
    | StmtContinue           thing -> invalidate_continue_statement             (value, thing)
    | StmtFunctionStatic     thing -> invalidate_function_static_statement      (value, thing)
    | StmtEcho               thing -> invalidate_echo_statement                 (value, thing)
    | StmtGlobal             thing -> invalidate_global_statement               (value, thing)
    | StmtTypeConstant       thing -> invalidate_type_constant                  (value, thing)
  and validate_switch_label : switch_label validator = fun x ->
    match Syntax.syntax x with
    | Syntax.CaseLabel _ -> tag validate_case_label (fun x -> SwitchCase x) x
    | Syntax.DefaultLabel _ -> tag validate_default_label (fun x -> SwitchDefault x) x
    | s -> aggregation_fail Def.SwitchLabel s
  and invalidate_switch_label : switch_label invalidator = fun (value, thing) ->
    match thing with
    | SwitchCase    thing -> invalidate_case_label                     (value, thing)
    | SwitchDefault thing -> invalidate_default_label                  (value, thing)
  and validate_lambda_body : lambda_body validator = fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ -> tag validate_literal_expression (fun x -> LambdaLiteral x) x
    | Syntax.VariableExpression _ -> tag validate_variable_expression (fun x -> LambdaVariable x) x
    | Syntax.QualifiedNameExpression _ -> tag validate_qualified_name_expression (fun x -> LambdaQualifiedName x) x
    | Syntax.PipeVariableExpression _ -> tag validate_pipe_variable_expression (fun x -> LambdaPipeVariable x) x
    | Syntax.DecoratedExpression _ -> tag validate_decorated_expression (fun x -> LambdaDecorated x) x
    | Syntax.InclusionExpression _ -> tag validate_inclusion_expression (fun x -> LambdaInclusion x) x
    | Syntax.CompoundStatement _ -> tag validate_compound_statement (fun x -> LambdaCompoundStatement x) x
    | Syntax.AnonymousFunction _ -> tag validate_anonymous_function (fun x -> LambdaAnonymousFunction x) x
    | Syntax.LambdaExpression _ -> tag validate_lambda_expression (fun x -> LambdaLambda x) x
    | Syntax.CastExpression _ -> tag validate_cast_expression (fun x -> LambdaCast x) x
    | Syntax.ScopeResolutionExpression _ -> tag validate_scope_resolution_expression (fun x -> LambdaScopeResolution x) x
    | Syntax.MemberSelectionExpression _ -> tag validate_member_selection_expression (fun x -> LambdaMemberSelection x) x
    | Syntax.SafeMemberSelectionExpression _ -> tag validate_safe_member_selection_expression (fun x -> LambdaSafeMemberSelection x) x
    | Syntax.EmbeddedMemberSelectionExpression _ -> tag validate_embedded_member_selection_expression (fun x -> LambdaEmbeddedMemberSelection x) x
    | Syntax.YieldExpression _ -> tag validate_yield_expression (fun x -> LambdaYield x) x
    | Syntax.YieldFromExpression _ -> tag validate_yield_from_expression (fun x -> LambdaYieldFrom x) x
    | Syntax.PrefixUnaryExpression _ -> tag validate_prefix_unary_expression (fun x -> LambdaPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ -> tag validate_postfix_unary_expression (fun x -> LambdaPostfixUnary x) x
    | Syntax.BinaryExpression _ -> tag validate_binary_expression (fun x -> LambdaBinary x) x
    | Syntax.InstanceofExpression _ -> tag validate_instanceof_expression (fun x -> LambdaInstanceof x) x
    | Syntax.ConditionalExpression _ -> tag validate_conditional_expression (fun x -> LambdaConditional x) x
    | Syntax.EvalExpression _ -> tag validate_eval_expression (fun x -> LambdaEval x) x
    | Syntax.EmptyExpression _ -> tag validate_empty_expression (fun x -> LambdaEmpty x) x
    | Syntax.DefineExpression _ -> tag validate_define_expression (fun x -> LambdaDefine x) x
    | Syntax.IssetExpression _ -> tag validate_isset_expression (fun x -> LambdaIsset x) x
    | Syntax.FunctionCallExpression _ -> tag validate_function_call_expression (fun x -> LambdaFunctionCall x) x
    | Syntax.FunctionCallWithTypeArgumentsExpression _ -> tag validate_function_call_with_type_arguments_expression (fun x -> LambdaFunctionCallWithTypeArguments x) x
    | Syntax.ParenthesizedExpression _ -> tag validate_parenthesized_expression (fun x -> LambdaParenthesized x) x
    | Syntax.BracedExpression _ -> tag validate_braced_expression (fun x -> LambdaBraced x) x
    | Syntax.EmbeddedBracedExpression _ -> tag validate_embedded_braced_expression (fun x -> LambdaEmbeddedBraced x) x
    | Syntax.ListExpression _ -> tag validate_list_expression (fun x -> LambdaList x) x
    | Syntax.CollectionLiteralExpression _ -> tag validate_collection_literal_expression (fun x -> LambdaCollectionLiteral x) x
    | Syntax.ObjectCreationExpression _ -> tag validate_object_creation_expression (fun x -> LambdaObjectCreation x) x
    | Syntax.ArrayCreationExpression _ -> tag validate_array_creation_expression (fun x -> LambdaArrayCreation x) x
    | Syntax.ArrayIntrinsicExpression _ -> tag validate_array_intrinsic_expression (fun x -> LambdaArrayIntrinsic x) x
    | Syntax.DarrayIntrinsicExpression _ -> tag validate_darray_intrinsic_expression (fun x -> LambdaDarrayIntrinsic x) x
    | Syntax.DictionaryIntrinsicExpression _ -> tag validate_dictionary_intrinsic_expression (fun x -> LambdaDictionaryIntrinsic x) x
    | Syntax.KeysetIntrinsicExpression _ -> tag validate_keyset_intrinsic_expression (fun x -> LambdaKeysetIntrinsic x) x
    | Syntax.VarrayIntrinsicExpression _ -> tag validate_varray_intrinsic_expression (fun x -> LambdaVarrayIntrinsic x) x
    | Syntax.VectorIntrinsicExpression _ -> tag validate_vector_intrinsic_expression (fun x -> LambdaVectorIntrinsic x) x
    | Syntax.SubscriptExpression _ -> tag validate_subscript_expression (fun x -> LambdaSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ -> tag validate_embedded_subscript_expression (fun x -> LambdaEmbeddedSubscript x) x
    | Syntax.AwaitableCreationExpression _ -> tag validate_awaitable_creation_expression (fun x -> LambdaAwaitableCreation x) x
    | Syntax.XHPChildrenParenthesizedList _ -> tag validate_xhp_children_parenthesized_list (fun x -> LambdaXHPChildrenParenthesizedList x) x
    | Syntax.XHPExpression _ -> tag validate_xhp_expression (fun x -> LambdaXHP x) x
    | Syntax.ShapeExpression _ -> tag validate_shape_expression (fun x -> LambdaShape x) x
    | Syntax.TupleExpression _ -> tag validate_tuple_expression (fun x -> LambdaTuple x) x
    | s -> aggregation_fail Def.LambdaBody s
  and invalidate_lambda_body : lambda_body invalidator = fun (value, thing) ->
    match thing with
    | LambdaLiteral                       thing -> invalidate_literal_expression             (value, thing)
    | LambdaVariable                      thing -> invalidate_variable_expression            (value, thing)
    | LambdaQualifiedName                 thing -> invalidate_qualified_name_expression      (value, thing)
    | LambdaPipeVariable                  thing -> invalidate_pipe_variable_expression       (value, thing)
    | LambdaDecorated                     thing -> invalidate_decorated_expression           (value, thing)
    | LambdaInclusion                     thing -> invalidate_inclusion_expression           (value, thing)
    | LambdaCompoundStatement             thing -> invalidate_compound_statement             (value, thing)
    | LambdaAnonymousFunction             thing -> invalidate_anonymous_function             (value, thing)
    | LambdaLambda                        thing -> invalidate_lambda_expression              (value, thing)
    | LambdaCast                          thing -> invalidate_cast_expression                (value, thing)
    | LambdaScopeResolution               thing -> invalidate_scope_resolution_expression    (value, thing)
    | LambdaMemberSelection               thing -> invalidate_member_selection_expression    (value, thing)
    | LambdaSafeMemberSelection           thing -> invalidate_safe_member_selection_expression (value, thing)
    | LambdaEmbeddedMemberSelection       thing -> invalidate_embedded_member_selection_expression (value, thing)
    | LambdaYield                         thing -> invalidate_yield_expression               (value, thing)
    | LambdaYieldFrom                     thing -> invalidate_yield_from_expression          (value, thing)
    | LambdaPrefixUnary                   thing -> invalidate_prefix_unary_expression        (value, thing)
    | LambdaPostfixUnary                  thing -> invalidate_postfix_unary_expression       (value, thing)
    | LambdaBinary                        thing -> invalidate_binary_expression              (value, thing)
    | LambdaInstanceof                    thing -> invalidate_instanceof_expression          (value, thing)
    | LambdaConditional                   thing -> invalidate_conditional_expression         (value, thing)
    | LambdaEval                          thing -> invalidate_eval_expression                (value, thing)
    | LambdaEmpty                         thing -> invalidate_empty_expression               (value, thing)
    | LambdaDefine                        thing -> invalidate_define_expression              (value, thing)
    | LambdaIsset                         thing -> invalidate_isset_expression               (value, thing)
    | LambdaFunctionCall                  thing -> invalidate_function_call_expression       (value, thing)
    | LambdaFunctionCallWithTypeArguments thing -> invalidate_function_call_with_type_arguments_expression (value, thing)
    | LambdaParenthesized                 thing -> invalidate_parenthesized_expression       (value, thing)
    | LambdaBraced                        thing -> invalidate_braced_expression              (value, thing)
    | LambdaEmbeddedBraced                thing -> invalidate_embedded_braced_expression     (value, thing)
    | LambdaList                          thing -> invalidate_list_expression                (value, thing)
    | LambdaCollectionLiteral             thing -> invalidate_collection_literal_expression  (value, thing)
    | LambdaObjectCreation                thing -> invalidate_object_creation_expression     (value, thing)
    | LambdaArrayCreation                 thing -> invalidate_array_creation_expression      (value, thing)
    | LambdaArrayIntrinsic                thing -> invalidate_array_intrinsic_expression     (value, thing)
    | LambdaDarrayIntrinsic               thing -> invalidate_darray_intrinsic_expression    (value, thing)
    | LambdaDictionaryIntrinsic           thing -> invalidate_dictionary_intrinsic_expression (value, thing)
    | LambdaKeysetIntrinsic               thing -> invalidate_keyset_intrinsic_expression    (value, thing)
    | LambdaVarrayIntrinsic               thing -> invalidate_varray_intrinsic_expression    (value, thing)
    | LambdaVectorIntrinsic               thing -> invalidate_vector_intrinsic_expression    (value, thing)
    | LambdaSubscript                     thing -> invalidate_subscript_expression           (value, thing)
    | LambdaEmbeddedSubscript             thing -> invalidate_embedded_subscript_expression  (value, thing)
    | LambdaAwaitableCreation             thing -> invalidate_awaitable_creation_expression  (value, thing)
    | LambdaXHPChildrenParenthesizedList  thing -> invalidate_xhp_children_parenthesized_list (value, thing)
    | LambdaXHP                           thing -> invalidate_xhp_expression                 (value, thing)
    | LambdaShape                         thing -> invalidate_shape_expression               (value, thing)
    | LambdaTuple                         thing -> invalidate_tuple_expression               (value, thing)
  and validate_constructor_expression : constructor_expression validator = fun x ->
    match Syntax.syntax x with
    | Syntax.LiteralExpression _ -> tag validate_literal_expression (fun x -> CExprLiteral x) x
    | Syntax.VariableExpression _ -> tag validate_variable_expression (fun x -> CExprVariable x) x
    | Syntax.QualifiedNameExpression _ -> tag validate_qualified_name_expression (fun x -> CExprQualifiedName x) x
    | Syntax.PipeVariableExpression _ -> tag validate_pipe_variable_expression (fun x -> CExprPipeVariable x) x
    | Syntax.DecoratedExpression _ -> tag validate_decorated_expression (fun x -> CExprDecorated x) x
    | Syntax.InclusionExpression _ -> tag validate_inclusion_expression (fun x -> CExprInclusion x) x
    | Syntax.AnonymousFunction _ -> tag validate_anonymous_function (fun x -> CExprAnonymousFunction x) x
    | Syntax.LambdaExpression _ -> tag validate_lambda_expression (fun x -> CExprLambda x) x
    | Syntax.CastExpression _ -> tag validate_cast_expression (fun x -> CExprCast x) x
    | Syntax.ScopeResolutionExpression _ -> tag validate_scope_resolution_expression (fun x -> CExprScopeResolution x) x
    | Syntax.MemberSelectionExpression _ -> tag validate_member_selection_expression (fun x -> CExprMemberSelection x) x
    | Syntax.SafeMemberSelectionExpression _ -> tag validate_safe_member_selection_expression (fun x -> CExprSafeMemberSelection x) x
    | Syntax.EmbeddedMemberSelectionExpression _ -> tag validate_embedded_member_selection_expression (fun x -> CExprEmbeddedMemberSelection x) x
    | Syntax.YieldExpression _ -> tag validate_yield_expression (fun x -> CExprYield x) x
    | Syntax.YieldFromExpression _ -> tag validate_yield_from_expression (fun x -> CExprYieldFrom x) x
    | Syntax.PrefixUnaryExpression _ -> tag validate_prefix_unary_expression (fun x -> CExprPrefixUnary x) x
    | Syntax.PostfixUnaryExpression _ -> tag validate_postfix_unary_expression (fun x -> CExprPostfixUnary x) x
    | Syntax.BinaryExpression _ -> tag validate_binary_expression (fun x -> CExprBinary x) x
    | Syntax.InstanceofExpression _ -> tag validate_instanceof_expression (fun x -> CExprInstanceof x) x
    | Syntax.ConditionalExpression _ -> tag validate_conditional_expression (fun x -> CExprConditional x) x
    | Syntax.EvalExpression _ -> tag validate_eval_expression (fun x -> CExprEval x) x
    | Syntax.EmptyExpression _ -> tag validate_empty_expression (fun x -> CExprEmpty x) x
    | Syntax.DefineExpression _ -> tag validate_define_expression (fun x -> CExprDefine x) x
    | Syntax.IssetExpression _ -> tag validate_isset_expression (fun x -> CExprIsset x) x
    | Syntax.FunctionCallExpression _ -> tag validate_function_call_expression (fun x -> CExprFunctionCall x) x
    | Syntax.FunctionCallWithTypeArgumentsExpression _ -> tag validate_function_call_with_type_arguments_expression (fun x -> CExprFunctionCallWithTypeArguments x) x
    | Syntax.ParenthesizedExpression _ -> tag validate_parenthesized_expression (fun x -> CExprParenthesized x) x
    | Syntax.BracedExpression _ -> tag validate_braced_expression (fun x -> CExprBraced x) x
    | Syntax.EmbeddedBracedExpression _ -> tag validate_embedded_braced_expression (fun x -> CExprEmbeddedBraced x) x
    | Syntax.ListExpression _ -> tag validate_list_expression (fun x -> CExprList x) x
    | Syntax.CollectionLiteralExpression _ -> tag validate_collection_literal_expression (fun x -> CExprCollectionLiteral x) x
    | Syntax.ObjectCreationExpression _ -> tag validate_object_creation_expression (fun x -> CExprObjectCreation x) x
    | Syntax.ArrayCreationExpression _ -> tag validate_array_creation_expression (fun x -> CExprArrayCreation x) x
    | Syntax.ArrayIntrinsicExpression _ -> tag validate_array_intrinsic_expression (fun x -> CExprArrayIntrinsic x) x
    | Syntax.DarrayIntrinsicExpression _ -> tag validate_darray_intrinsic_expression (fun x -> CExprDarrayIntrinsic x) x
    | Syntax.DictionaryIntrinsicExpression _ -> tag validate_dictionary_intrinsic_expression (fun x -> CExprDictionaryIntrinsic x) x
    | Syntax.KeysetIntrinsicExpression _ -> tag validate_keyset_intrinsic_expression (fun x -> CExprKeysetIntrinsic x) x
    | Syntax.VarrayIntrinsicExpression _ -> tag validate_varray_intrinsic_expression (fun x -> CExprVarrayIntrinsic x) x
    | Syntax.VectorIntrinsicExpression _ -> tag validate_vector_intrinsic_expression (fun x -> CExprVectorIntrinsic x) x
    | Syntax.ElementInitializer _ -> tag validate_element_initializer (fun x -> CExprElementInitializer x) x
    | Syntax.SubscriptExpression _ -> tag validate_subscript_expression (fun x -> CExprSubscript x) x
    | Syntax.EmbeddedSubscriptExpression _ -> tag validate_embedded_subscript_expression (fun x -> CExprEmbeddedSubscript x) x
    | Syntax.AwaitableCreationExpression _ -> tag validate_awaitable_creation_expression (fun x -> CExprAwaitableCreation x) x
    | Syntax.XHPChildrenParenthesizedList _ -> tag validate_xhp_children_parenthesized_list (fun x -> CExprXHPChildrenParenthesizedList x) x
    | Syntax.XHPExpression _ -> tag validate_xhp_expression (fun x -> CExprXHP x) x
    | Syntax.ShapeExpression _ -> tag validate_shape_expression (fun x -> CExprShape x) x
    | Syntax.TupleExpression _ -> tag validate_tuple_expression (fun x -> CExprTuple x) x
    | s -> aggregation_fail Def.ConstructorExpression s
  and invalidate_constructor_expression : constructor_expression invalidator = fun (value, thing) ->
    match thing with
    | CExprLiteral                       thing -> invalidate_literal_expression             (value, thing)
    | CExprVariable                      thing -> invalidate_variable_expression            (value, thing)
    | CExprQualifiedName                 thing -> invalidate_qualified_name_expression      (value, thing)
    | CExprPipeVariable                  thing -> invalidate_pipe_variable_expression       (value, thing)
    | CExprDecorated                     thing -> invalidate_decorated_expression           (value, thing)
    | CExprInclusion                     thing -> invalidate_inclusion_expression           (value, thing)
    | CExprAnonymousFunction             thing -> invalidate_anonymous_function             (value, thing)
    | CExprLambda                        thing -> invalidate_lambda_expression              (value, thing)
    | CExprCast                          thing -> invalidate_cast_expression                (value, thing)
    | CExprScopeResolution               thing -> invalidate_scope_resolution_expression    (value, thing)
    | CExprMemberSelection               thing -> invalidate_member_selection_expression    (value, thing)
    | CExprSafeMemberSelection           thing -> invalidate_safe_member_selection_expression (value, thing)
    | CExprEmbeddedMemberSelection       thing -> invalidate_embedded_member_selection_expression (value, thing)
    | CExprYield                         thing -> invalidate_yield_expression               (value, thing)
    | CExprYieldFrom                     thing -> invalidate_yield_from_expression          (value, thing)
    | CExprPrefixUnary                   thing -> invalidate_prefix_unary_expression        (value, thing)
    | CExprPostfixUnary                  thing -> invalidate_postfix_unary_expression       (value, thing)
    | CExprBinary                        thing -> invalidate_binary_expression              (value, thing)
    | CExprInstanceof                    thing -> invalidate_instanceof_expression          (value, thing)
    | CExprConditional                   thing -> invalidate_conditional_expression         (value, thing)
    | CExprEval                          thing -> invalidate_eval_expression                (value, thing)
    | CExprEmpty                         thing -> invalidate_empty_expression               (value, thing)
    | CExprDefine                        thing -> invalidate_define_expression              (value, thing)
    | CExprIsset                         thing -> invalidate_isset_expression               (value, thing)
    | CExprFunctionCall                  thing -> invalidate_function_call_expression       (value, thing)
    | CExprFunctionCallWithTypeArguments thing -> invalidate_function_call_with_type_arguments_expression (value, thing)
    | CExprParenthesized                 thing -> invalidate_parenthesized_expression       (value, thing)
    | CExprBraced                        thing -> invalidate_braced_expression              (value, thing)
    | CExprEmbeddedBraced                thing -> invalidate_embedded_braced_expression     (value, thing)
    | CExprList                          thing -> invalidate_list_expression                (value, thing)
    | CExprCollectionLiteral             thing -> invalidate_collection_literal_expression  (value, thing)
    | CExprObjectCreation                thing -> invalidate_object_creation_expression     (value, thing)
    | CExprArrayCreation                 thing -> invalidate_array_creation_expression      (value, thing)
    | CExprArrayIntrinsic                thing -> invalidate_array_intrinsic_expression     (value, thing)
    | CExprDarrayIntrinsic               thing -> invalidate_darray_intrinsic_expression    (value, thing)
    | CExprDictionaryIntrinsic           thing -> invalidate_dictionary_intrinsic_expression (value, thing)
    | CExprKeysetIntrinsic               thing -> invalidate_keyset_intrinsic_expression    (value, thing)
    | CExprVarrayIntrinsic               thing -> invalidate_varray_intrinsic_expression    (value, thing)
    | CExprVectorIntrinsic               thing -> invalidate_vector_intrinsic_expression    (value, thing)
    | CExprElementInitializer            thing -> invalidate_element_initializer            (value, thing)
    | CExprSubscript                     thing -> invalidate_subscript_expression           (value, thing)
    | CExprEmbeddedSubscript             thing -> invalidate_embedded_subscript_expression  (value, thing)
    | CExprAwaitableCreation             thing -> invalidate_awaitable_creation_expression  (value, thing)
    | CExprXHPChildrenParenthesizedList  thing -> invalidate_xhp_children_parenthesized_list (value, thing)
    | CExprXHP                           thing -> invalidate_xhp_expression                 (value, thing)
    | CExprShape                         thing -> invalidate_shape_expression               (value, thing)
    | CExprTuple                         thing -> invalidate_tuple_expression               (value, thing)
  and validate_namespace_internals : namespace_internals validator = fun x ->
    match Syntax.syntax x with
    | Syntax.NamespaceBody _ -> tag validate_namespace_body (fun x -> NSINamespaceBody x) x
    | Syntax.NamespaceEmptyBody _ -> tag validate_namespace_empty_body (fun x -> NSINamespaceEmptyBody x) x
    | s -> aggregation_fail Def.NamespaceInternals s
  and invalidate_namespace_internals : namespace_internals invalidator = fun (value, thing) ->
    match thing with
    | NSINamespaceBody      thing -> invalidate_namespace_body                 (value, thing)
    | NSINamespaceEmptyBody thing -> invalidate_namespace_empty_body           (value, thing)
  and validate_todo_aggregate : todo_aggregate validator = fun x ->
    match Syntax.syntax x with
    | Syntax.EndOfFile _ -> tag validate_end_of_file (fun x -> TODOEndOfFile x) x
    | s -> aggregation_fail Def.TODO s
  and invalidate_todo_aggregate : todo_aggregate invalidator = fun (value, thing) ->
    match thing with
    | TODOEndOfFile thing -> invalidate_end_of_file                    (value, thing)

  and validate_end_of_file : end_of_file validator = function
  | { Syntax.syntax = Syntax.EndOfFile x; value = v } -> v,
    { end_of_file_token = validate_token x.Syntax.end_of_file_token
    }
  | s -> validation_fail SyntaxKind.EndOfFile s
  and invalidate_end_of_file : end_of_file invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EndOfFile
      { Syntax.end_of_file_token = invalidate_token x.end_of_file_token
      }
    ; Syntax.value = v
    }
  and validate_script : script validator = function
  | { Syntax.syntax = Syntax.Script x; value = v } -> v,
    { script_declarations = validate_list_with (validate_top_level_declaration) x.Syntax.script_declarations
    }
  | s -> validation_fail SyntaxKind.Script s
  and invalidate_script : script invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.Script
      { Syntax.script_declarations = invalidate_list_with (invalidate_top_level_declaration) x.script_declarations
      }
    ; Syntax.value = v
    }
  and validate_simple_type_specifier : simple_type_specifier validator = function
  | { Syntax.syntax = Syntax.SimpleTypeSpecifier x; value = v } -> v,
    { simple_type_specifier = validate_token x.Syntax.simple_type_specifier
    }
  | s -> validation_fail SyntaxKind.SimpleTypeSpecifier s
  and invalidate_simple_type_specifier : simple_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SimpleTypeSpecifier
      { Syntax.simple_type_specifier = invalidate_token x.simple_type_specifier
      }
    ; Syntax.value = v
    }
  and validate_literal_expression : literal_expression validator = function
  | { Syntax.syntax = Syntax.LiteralExpression x; value = v } -> v,
    { literal_expression = validate_list_with (validate_expression) x.Syntax.literal_expression
    }
  | s -> validation_fail SyntaxKind.LiteralExpression s
  and invalidate_literal_expression : literal_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.LiteralExpression
      { Syntax.literal_expression = invalidate_list_with (invalidate_expression) x.literal_expression
      }
    ; Syntax.value = v
    }
  and validate_variable_expression : variable_expression validator = function
  | { Syntax.syntax = Syntax.VariableExpression x; value = v } -> v,
    { variable_expression = validate_token x.Syntax.variable_expression
    }
  | s -> validation_fail SyntaxKind.VariableExpression s
  and invalidate_variable_expression : variable_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VariableExpression
      { Syntax.variable_expression = invalidate_token x.variable_expression
      }
    ; Syntax.value = v
    }
  and validate_qualified_name_expression : qualified_name_expression validator = function
  | { Syntax.syntax = Syntax.QualifiedNameExpression x; value = v } -> v,
    { qualified_name_expression = validate_token x.Syntax.qualified_name_expression
    }
  | s -> validation_fail SyntaxKind.QualifiedNameExpression s
  and invalidate_qualified_name_expression : qualified_name_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.QualifiedNameExpression
      { Syntax.qualified_name_expression = invalidate_token x.qualified_name_expression
      }
    ; Syntax.value = v
    }
  and validate_pipe_variable_expression : pipe_variable_expression validator = function
  | { Syntax.syntax = Syntax.PipeVariableExpression x; value = v } -> v,
    { pipe_variable_expression = validate_token x.Syntax.pipe_variable_expression
    }
  | s -> validation_fail SyntaxKind.PipeVariableExpression s
  and invalidate_pipe_variable_expression : pipe_variable_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.PipeVariableExpression
      { Syntax.pipe_variable_expression = invalidate_token x.pipe_variable_expression
      }
    ; Syntax.value = v
    }
  and validate_enum_declaration : enum_declaration validator = function
  | { Syntax.syntax = Syntax.EnumDeclaration x; value = v } -> v,
    { enum_right_brace = validate_token x.Syntax.enum_right_brace
    ; enum_enumerators = validate_list_with (validate_enumerator) x.Syntax.enum_enumerators
    ; enum_left_brace = validate_token x.Syntax.enum_left_brace
    ; enum_type = validate_option_with (validate_type_constraint) x.Syntax.enum_type
    ; enum_base = validate_specifier x.Syntax.enum_base
    ; enum_colon = validate_token x.Syntax.enum_colon
    ; enum_name = validate_token x.Syntax.enum_name
    ; enum_keyword = validate_token x.Syntax.enum_keyword
    ; enum_attribute_spec = validate_option_with (validate_attribute_specification) x.Syntax.enum_attribute_spec
    }
  | s -> validation_fail SyntaxKind.EnumDeclaration s
  and invalidate_enum_declaration : enum_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EnumDeclaration
      { Syntax.enum_attribute_spec = invalidate_option_with (invalidate_attribute_specification) x.enum_attribute_spec
      ; Syntax.enum_keyword = invalidate_token x.enum_keyword
      ; Syntax.enum_name = invalidate_token x.enum_name
      ; Syntax.enum_colon = invalidate_token x.enum_colon
      ; Syntax.enum_base = invalidate_specifier x.enum_base
      ; Syntax.enum_type = invalidate_option_with (invalidate_type_constraint) x.enum_type
      ; Syntax.enum_left_brace = invalidate_token x.enum_left_brace
      ; Syntax.enum_enumerators = invalidate_list_with (invalidate_enumerator) x.enum_enumerators
      ; Syntax.enum_right_brace = invalidate_token x.enum_right_brace
      }
    ; Syntax.value = v
    }
  and validate_enumerator : enumerator validator = function
  | { Syntax.syntax = Syntax.Enumerator x; value = v } -> v,
    { enumerator_semicolon = validate_token x.Syntax.enumerator_semicolon
    ; enumerator_value = validate_expression x.Syntax.enumerator_value
    ; enumerator_equal = validate_token x.Syntax.enumerator_equal
    ; enumerator_name = validate_token x.Syntax.enumerator_name
    }
  | s -> validation_fail SyntaxKind.Enumerator s
  and invalidate_enumerator : enumerator invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.Enumerator
      { Syntax.enumerator_name = invalidate_token x.enumerator_name
      ; Syntax.enumerator_equal = invalidate_token x.enumerator_equal
      ; Syntax.enumerator_value = invalidate_expression x.enumerator_value
      ; Syntax.enumerator_semicolon = invalidate_token x.enumerator_semicolon
      }
    ; Syntax.value = v
    }
  and validate_alias_declaration : alias_declaration validator = function
  | { Syntax.syntax = Syntax.AliasDeclaration x; value = v } -> v,
    { alias_semicolon = validate_token x.Syntax.alias_semicolon
    ; alias_type = validate_specifier x.Syntax.alias_type
    ; alias_equal = validate_option_with (validate_token) x.Syntax.alias_equal
    ; alias_constraint = validate_option_with (validate_type_constraint) x.Syntax.alias_constraint
    ; alias_generic_parameter = validate_option_with (validate_type_parameters) x.Syntax.alias_generic_parameter
    ; alias_name = validate_option_with (validate_token) x.Syntax.alias_name
    ; alias_keyword = validate_token x.Syntax.alias_keyword
    ; alias_attribute_spec = validate_option_with (validate_attribute_specification) x.Syntax.alias_attribute_spec
    }
  | s -> validation_fail SyntaxKind.AliasDeclaration s
  and invalidate_alias_declaration : alias_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.AliasDeclaration
      { Syntax.alias_attribute_spec = invalidate_option_with (invalidate_attribute_specification) x.alias_attribute_spec
      ; Syntax.alias_keyword = invalidate_token x.alias_keyword
      ; Syntax.alias_name = invalidate_option_with (invalidate_token) x.alias_name
      ; Syntax.alias_generic_parameter = invalidate_option_with (invalidate_type_parameters) x.alias_generic_parameter
      ; Syntax.alias_constraint = invalidate_option_with (invalidate_type_constraint) x.alias_constraint
      ; Syntax.alias_equal = invalidate_option_with (invalidate_token) x.alias_equal
      ; Syntax.alias_type = invalidate_specifier x.alias_type
      ; Syntax.alias_semicolon = invalidate_token x.alias_semicolon
      }
    ; Syntax.value = v
    }
  and validate_property_declaration : property_declaration validator = function
  | { Syntax.syntax = Syntax.PropertyDeclaration x; value = v } -> v,
    { property_semicolon = validate_token x.Syntax.property_semicolon
    ; property_declarators = validate_list_with (validate_property_declarator) x.Syntax.property_declarators
    ; property_type = validate_option_with (validate_specifier) x.Syntax.property_type
    ; property_modifiers = validate_list_with (validate_token) x.Syntax.property_modifiers
    }
  | s -> validation_fail SyntaxKind.PropertyDeclaration s
  and invalidate_property_declaration : property_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.PropertyDeclaration
      { Syntax.property_modifiers = invalidate_list_with (invalidate_token) x.property_modifiers
      ; Syntax.property_type = invalidate_option_with (invalidate_specifier) x.property_type
      ; Syntax.property_declarators = invalidate_list_with (invalidate_property_declarator) x.property_declarators
      ; Syntax.property_semicolon = invalidate_token x.property_semicolon
      }
    ; Syntax.value = v
    }
  and validate_property_declarator : property_declarator validator = function
  | { Syntax.syntax = Syntax.PropertyDeclarator x; value = v } -> v,
    { property_initializer = validate_option_with (validate_simple_initializer) x.Syntax.property_initializer
    ; property_name = validate_token x.Syntax.property_name
    }
  | s -> validation_fail SyntaxKind.PropertyDeclarator s
  and invalidate_property_declarator : property_declarator invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.PropertyDeclarator
      { Syntax.property_name = invalidate_token x.property_name
      ; Syntax.property_initializer = invalidate_option_with (invalidate_simple_initializer) x.property_initializer
      }
    ; Syntax.value = v
    }
  and validate_namespace_declaration : namespace_declaration validator = function
  | { Syntax.syntax = Syntax.NamespaceDeclaration x; value = v } -> v,
    { namespace_body = validate_namespace_internals x.Syntax.namespace_body
    ; namespace_name = validate_option_with (validate_token) x.Syntax.namespace_name
    ; namespace_keyword = validate_token x.Syntax.namespace_keyword
    }
  | s -> validation_fail SyntaxKind.NamespaceDeclaration s
  and invalidate_namespace_declaration : namespace_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceDeclaration
      { Syntax.namespace_keyword = invalidate_token x.namespace_keyword
      ; Syntax.namespace_name = invalidate_option_with (invalidate_token) x.namespace_name
      ; Syntax.namespace_body = invalidate_namespace_internals x.namespace_body
      }
    ; Syntax.value = v
    }
  and validate_namespace_body : namespace_body validator = function
  | { Syntax.syntax = Syntax.NamespaceBody x; value = v } -> v,
    { namespace_right_brace = validate_token x.Syntax.namespace_right_brace
    ; namespace_declarations = validate_list_with (validate_top_level_declaration) x.Syntax.namespace_declarations
    ; namespace_left_brace = validate_token x.Syntax.namespace_left_brace
    }
  | s -> validation_fail SyntaxKind.NamespaceBody s
  and invalidate_namespace_body : namespace_body invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceBody
      { Syntax.namespace_left_brace = invalidate_token x.namespace_left_brace
      ; Syntax.namespace_declarations = invalidate_list_with (invalidate_top_level_declaration) x.namespace_declarations
      ; Syntax.namespace_right_brace = invalidate_token x.namespace_right_brace
      }
    ; Syntax.value = v
    }
  and validate_namespace_empty_body : namespace_empty_body validator = function
  | { Syntax.syntax = Syntax.NamespaceEmptyBody x; value = v } -> v,
    { namespace_semicolon = validate_token x.Syntax.namespace_semicolon
    }
  | s -> validation_fail SyntaxKind.NamespaceEmptyBody s
  and invalidate_namespace_empty_body : namespace_empty_body invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceEmptyBody
      { Syntax.namespace_semicolon = invalidate_token x.namespace_semicolon
      }
    ; Syntax.value = v
    }
  and validate_namespace_use_declaration : namespace_use_declaration validator = function
  | { Syntax.syntax = Syntax.NamespaceUseDeclaration x; value = v } -> v,
    { namespace_use_semicolon = validate_option_with (validate_token) x.Syntax.namespace_use_semicolon
    ; namespace_use_clauses = validate_list_with (validate_option_with (validate_namespace_use_clause)) x.Syntax.namespace_use_clauses
    ; namespace_use_kind = validate_option_with (validate_token) x.Syntax.namespace_use_kind
    ; namespace_use_keyword = validate_token x.Syntax.namespace_use_keyword
    }
  | s -> validation_fail SyntaxKind.NamespaceUseDeclaration s
  and invalidate_namespace_use_declaration : namespace_use_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceUseDeclaration
      { Syntax.namespace_use_keyword = invalidate_token x.namespace_use_keyword
      ; Syntax.namespace_use_kind = invalidate_option_with (invalidate_token) x.namespace_use_kind
      ; Syntax.namespace_use_clauses = invalidate_list_with (invalidate_option_with (invalidate_namespace_use_clause)) x.namespace_use_clauses
      ; Syntax.namespace_use_semicolon = invalidate_option_with (invalidate_token) x.namespace_use_semicolon
      }
    ; Syntax.value = v
    }
  and validate_namespace_group_use_declaration : namespace_group_use_declaration validator = function
  | { Syntax.syntax = Syntax.NamespaceGroupUseDeclaration x; value = v } -> v,
    { namespace_group_use_semicolon = validate_token x.Syntax.namespace_group_use_semicolon
    ; namespace_group_use_right_brace = validate_token x.Syntax.namespace_group_use_right_brace
    ; namespace_group_use_clauses = validate_list_with (validate_namespace_use_clause) x.Syntax.namespace_group_use_clauses
    ; namespace_group_use_left_brace = validate_token x.Syntax.namespace_group_use_left_brace
    ; namespace_group_use_prefix = validate_token x.Syntax.namespace_group_use_prefix
    ; namespace_group_use_kind = validate_option_with (validate_token) x.Syntax.namespace_group_use_kind
    ; namespace_group_use_keyword = validate_token x.Syntax.namespace_group_use_keyword
    }
  | s -> validation_fail SyntaxKind.NamespaceGroupUseDeclaration s
  and invalidate_namespace_group_use_declaration : namespace_group_use_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceGroupUseDeclaration
      { Syntax.namespace_group_use_keyword = invalidate_token x.namespace_group_use_keyword
      ; Syntax.namespace_group_use_kind = invalidate_option_with (invalidate_token) x.namespace_group_use_kind
      ; Syntax.namespace_group_use_prefix = invalidate_token x.namespace_group_use_prefix
      ; Syntax.namespace_group_use_left_brace = invalidate_token x.namespace_group_use_left_brace
      ; Syntax.namespace_group_use_clauses = invalidate_list_with (invalidate_namespace_use_clause) x.namespace_group_use_clauses
      ; Syntax.namespace_group_use_right_brace = invalidate_token x.namespace_group_use_right_brace
      ; Syntax.namespace_group_use_semicolon = invalidate_token x.namespace_group_use_semicolon
      }
    ; Syntax.value = v
    }
  and validate_namespace_use_clause : namespace_use_clause validator = function
  | { Syntax.syntax = Syntax.NamespaceUseClause x; value = v } -> v,
    { namespace_use_alias = validate_option_with (validate_token) x.Syntax.namespace_use_alias
    ; namespace_use_as = validate_option_with (validate_token) x.Syntax.namespace_use_as
    ; namespace_use_name = validate_token x.Syntax.namespace_use_name
    ; namespace_use_clause_kind = validate_option_with (validate_token) x.Syntax.namespace_use_clause_kind
    }
  | s -> validation_fail SyntaxKind.NamespaceUseClause s
  and invalidate_namespace_use_clause : namespace_use_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NamespaceUseClause
      { Syntax.namespace_use_clause_kind = invalidate_option_with (invalidate_token) x.namespace_use_clause_kind
      ; Syntax.namespace_use_name = invalidate_token x.namespace_use_name
      ; Syntax.namespace_use_as = invalidate_option_with (invalidate_token) x.namespace_use_as
      ; Syntax.namespace_use_alias = invalidate_option_with (invalidate_token) x.namespace_use_alias
      }
    ; Syntax.value = v
    }
  and validate_function_declaration : function_declaration validator = function
  | { Syntax.syntax = Syntax.FunctionDeclaration x; value = v } -> v,
    { function_body = validate_compound_statement x.Syntax.function_body
    ; function_declaration_header = validate_function_declaration_header x.Syntax.function_declaration_header
    ; function_attribute_spec = validate_option_with (validate_attribute_specification) x.Syntax.function_attribute_spec
    }
  | s -> validation_fail SyntaxKind.FunctionDeclaration s
  and invalidate_function_declaration : function_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FunctionDeclaration
      { Syntax.function_attribute_spec = invalidate_option_with (invalidate_attribute_specification) x.function_attribute_spec
      ; Syntax.function_declaration_header = invalidate_function_declaration_header x.function_declaration_header
      ; Syntax.function_body = invalidate_compound_statement x.function_body
      }
    ; Syntax.value = v
    }
  and validate_function_declaration_header : function_declaration_header validator = function
  | { Syntax.syntax = Syntax.FunctionDeclarationHeader x; value = v } -> v,
    { function_where_clause = validate_option_with (validate_where_clause) x.Syntax.function_where_clause
    ; function_type = validate_option_with (validate_specifier) x.Syntax.function_type
    ; function_colon = validate_option_with (validate_token) x.Syntax.function_colon
    ; function_right_paren = validate_token x.Syntax.function_right_paren
    ; function_parameter_list = validate_list_with (validate_option_with (validate_parameter)) x.Syntax.function_parameter_list
    ; function_left_paren = validate_token x.Syntax.function_left_paren
    ; function_type_parameter_list = validate_option_with (validate_type_parameters) x.Syntax.function_type_parameter_list
    ; function_name = validate_token x.Syntax.function_name
    ; function_ampersand = validate_option_with (validate_token) x.Syntax.function_ampersand
    ; function_keyword = validate_token x.Syntax.function_keyword
    ; function_coroutine = validate_option_with (validate_token) x.Syntax.function_coroutine
    ; function_async = validate_option_with (validate_token) x.Syntax.function_async
    }
  | s -> validation_fail SyntaxKind.FunctionDeclarationHeader s
  and invalidate_function_declaration_header : function_declaration_header invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FunctionDeclarationHeader
      { Syntax.function_async = invalidate_option_with (invalidate_token) x.function_async
      ; Syntax.function_coroutine = invalidate_option_with (invalidate_token) x.function_coroutine
      ; Syntax.function_keyword = invalidate_token x.function_keyword
      ; Syntax.function_ampersand = invalidate_option_with (invalidate_token) x.function_ampersand
      ; Syntax.function_name = invalidate_token x.function_name
      ; Syntax.function_type_parameter_list = invalidate_option_with (invalidate_type_parameters) x.function_type_parameter_list
      ; Syntax.function_left_paren = invalidate_token x.function_left_paren
      ; Syntax.function_parameter_list = invalidate_list_with (invalidate_option_with (invalidate_parameter)) x.function_parameter_list
      ; Syntax.function_right_paren = invalidate_token x.function_right_paren
      ; Syntax.function_colon = invalidate_option_with (invalidate_token) x.function_colon
      ; Syntax.function_type = invalidate_option_with (invalidate_specifier) x.function_type
      ; Syntax.function_where_clause = invalidate_option_with (invalidate_where_clause) x.function_where_clause
      }
    ; Syntax.value = v
    }
  and validate_where_clause : where_clause validator = function
  | { Syntax.syntax = Syntax.WhereClause x; value = v } -> v,
    { where_clause_constraints = validate_list_with (validate_where_constraint) x.Syntax.where_clause_constraints
    ; where_clause_keyword = validate_token x.Syntax.where_clause_keyword
    }
  | s -> validation_fail SyntaxKind.WhereClause s
  and invalidate_where_clause : where_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.WhereClause
      { Syntax.where_clause_keyword = invalidate_token x.where_clause_keyword
      ; Syntax.where_clause_constraints = invalidate_list_with (invalidate_where_constraint) x.where_clause_constraints
      }
    ; Syntax.value = v
    }
  and validate_where_constraint : where_constraint validator = function
  | { Syntax.syntax = Syntax.WhereConstraint x; value = v } -> v,
    { where_constraint_right_type = validate_specifier x.Syntax.where_constraint_right_type
    ; where_constraint_operator = validate_token x.Syntax.where_constraint_operator
    ; where_constraint_left_type = validate_specifier x.Syntax.where_constraint_left_type
    }
  | s -> validation_fail SyntaxKind.WhereConstraint s
  and invalidate_where_constraint : where_constraint invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.WhereConstraint
      { Syntax.where_constraint_left_type = invalidate_specifier x.where_constraint_left_type
      ; Syntax.where_constraint_operator = invalidate_token x.where_constraint_operator
      ; Syntax.where_constraint_right_type = invalidate_specifier x.where_constraint_right_type
      }
    ; Syntax.value = v
    }
  and validate_methodish_declaration : methodish_declaration validator = function
  | { Syntax.syntax = Syntax.MethodishDeclaration x; value = v } -> v,
    { methodish_semicolon = validate_option_with (validate_token) x.Syntax.methodish_semicolon
    ; methodish_function_body = validate_option_with (validate_compound_statement) x.Syntax.methodish_function_body
    ; methodish_function_decl_header = validate_function_declaration_header x.Syntax.methodish_function_decl_header
    ; methodish_modifiers = validate_list_with (validate_token) x.Syntax.methodish_modifiers
    ; methodish_attribute = validate_option_with (validate_attribute_specification) x.Syntax.methodish_attribute
    }
  | s -> validation_fail SyntaxKind.MethodishDeclaration s
  and invalidate_methodish_declaration : methodish_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.MethodishDeclaration
      { Syntax.methodish_attribute = invalidate_option_with (invalidate_attribute_specification) x.methodish_attribute
      ; Syntax.methodish_modifiers = invalidate_list_with (invalidate_token) x.methodish_modifiers
      ; Syntax.methodish_function_decl_header = invalidate_function_declaration_header x.methodish_function_decl_header
      ; Syntax.methodish_function_body = invalidate_option_with (invalidate_compound_statement) x.methodish_function_body
      ; Syntax.methodish_semicolon = invalidate_option_with (invalidate_token) x.methodish_semicolon
      }
    ; Syntax.value = v
    }
  and validate_classish_declaration : classish_declaration validator = function
  | { Syntax.syntax = Syntax.ClassishDeclaration x; value = v } -> v,
    { classish_body = validate_classish_body x.Syntax.classish_body
    ; classish_implements_list = validate_list_with (validate_specifier) x.Syntax.classish_implements_list
    ; classish_implements_keyword = validate_option_with (validate_token) x.Syntax.classish_implements_keyword
    ; classish_extends_list = validate_list_with (validate_specifier) x.Syntax.classish_extends_list
    ; classish_extends_keyword = validate_option_with (validate_token) x.Syntax.classish_extends_keyword
    ; classish_type_parameters = validate_option_with (validate_type_parameters) x.Syntax.classish_type_parameters
    ; classish_name = validate_token x.Syntax.classish_name
    ; classish_keyword = validate_token x.Syntax.classish_keyword
    ; classish_modifiers = validate_list_with (validate_token) x.Syntax.classish_modifiers
    ; classish_attribute = validate_option_with (validate_attribute_specification) x.Syntax.classish_attribute
    }
  | s -> validation_fail SyntaxKind.ClassishDeclaration s
  and invalidate_classish_declaration : classish_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ClassishDeclaration
      { Syntax.classish_attribute = invalidate_option_with (invalidate_attribute_specification) x.classish_attribute
      ; Syntax.classish_modifiers = invalidate_list_with (invalidate_token) x.classish_modifiers
      ; Syntax.classish_keyword = invalidate_token x.classish_keyword
      ; Syntax.classish_name = invalidate_token x.classish_name
      ; Syntax.classish_type_parameters = invalidate_option_with (invalidate_type_parameters) x.classish_type_parameters
      ; Syntax.classish_extends_keyword = invalidate_option_with (invalidate_token) x.classish_extends_keyword
      ; Syntax.classish_extends_list = invalidate_list_with (invalidate_specifier) x.classish_extends_list
      ; Syntax.classish_implements_keyword = invalidate_option_with (invalidate_token) x.classish_implements_keyword
      ; Syntax.classish_implements_list = invalidate_list_with (invalidate_specifier) x.classish_implements_list
      ; Syntax.classish_body = invalidate_classish_body x.classish_body
      }
    ; Syntax.value = v
    }
  and validate_classish_body : classish_body validator = function
  | { Syntax.syntax = Syntax.ClassishBody x; value = v } -> v,
    { classish_body_right_brace = validate_token x.Syntax.classish_body_right_brace
    ; classish_body_elements = validate_list_with (validate_class_body_declaration) x.Syntax.classish_body_elements
    ; classish_body_left_brace = validate_token x.Syntax.classish_body_left_brace
    }
  | s -> validation_fail SyntaxKind.ClassishBody s
  and invalidate_classish_body : classish_body invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ClassishBody
      { Syntax.classish_body_left_brace = invalidate_token x.classish_body_left_brace
      ; Syntax.classish_body_elements = invalidate_list_with (invalidate_class_body_declaration) x.classish_body_elements
      ; Syntax.classish_body_right_brace = invalidate_token x.classish_body_right_brace
      }
    ; Syntax.value = v
    }
  and validate_trait_use_precedence_item : trait_use_precedence_item validator = function
  | { Syntax.syntax = Syntax.TraitUsePrecedenceItem x; value = v } -> v,
    { trait_use_precedence_item_removed_names = validate_list_with (validate_specifier) x.Syntax.trait_use_precedence_item_removed_names
    ; trait_use_precedence_item_keyword = validate_token x.Syntax.trait_use_precedence_item_keyword
    ; trait_use_precedence_item_name = validate_specifier x.Syntax.trait_use_precedence_item_name
    }
  | s -> validation_fail SyntaxKind.TraitUsePrecedenceItem s
  and invalidate_trait_use_precedence_item : trait_use_precedence_item invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TraitUsePrecedenceItem
      { Syntax.trait_use_precedence_item_name = invalidate_specifier x.trait_use_precedence_item_name
      ; Syntax.trait_use_precedence_item_keyword = invalidate_token x.trait_use_precedence_item_keyword
      ; Syntax.trait_use_precedence_item_removed_names = invalidate_list_with (invalidate_specifier) x.trait_use_precedence_item_removed_names
      }
    ; Syntax.value = v
    }
  and validate_trait_use_alias_item : trait_use_alias_item validator = function
  | { Syntax.syntax = Syntax.TraitUseAliasItem x; value = v } -> v,
    { trait_use_alias_item_aliased_name = validate_option_with (validate_specifier) x.Syntax.trait_use_alias_item_aliased_name
    ; trait_use_alias_item_visibility = validate_option_with (validate_token) x.Syntax.trait_use_alias_item_visibility
    ; trait_use_alias_item_keyword = validate_token x.Syntax.trait_use_alias_item_keyword
    ; trait_use_alias_item_aliasing_name = validate_specifier x.Syntax.trait_use_alias_item_aliasing_name
    }
  | s -> validation_fail SyntaxKind.TraitUseAliasItem s
  and invalidate_trait_use_alias_item : trait_use_alias_item invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TraitUseAliasItem
      { Syntax.trait_use_alias_item_aliasing_name = invalidate_specifier x.trait_use_alias_item_aliasing_name
      ; Syntax.trait_use_alias_item_keyword = invalidate_token x.trait_use_alias_item_keyword
      ; Syntax.trait_use_alias_item_visibility = invalidate_option_with (invalidate_token) x.trait_use_alias_item_visibility
      ; Syntax.trait_use_alias_item_aliased_name = invalidate_option_with (invalidate_specifier) x.trait_use_alias_item_aliased_name
      }
    ; Syntax.value = v
    }
  and validate_trait_use_conflict_resolution : trait_use_conflict_resolution validator = function
  | { Syntax.syntax = Syntax.TraitUseConflictResolution x; value = v } -> v,
    { trait_use_conflict_resolution_right_brace = validate_token x.Syntax.trait_use_conflict_resolution_right_brace
    ; trait_use_conflict_resolution_clauses = validate_list_with (validate_specifier) x.Syntax.trait_use_conflict_resolution_clauses
    ; trait_use_conflict_resolution_left_brace = validate_token x.Syntax.trait_use_conflict_resolution_left_brace
    ; trait_use_conflict_resolution_names = validate_list_with (validate_specifier) x.Syntax.trait_use_conflict_resolution_names
    ; trait_use_conflict_resolution_keyword = validate_token x.Syntax.trait_use_conflict_resolution_keyword
    }
  | s -> validation_fail SyntaxKind.TraitUseConflictResolution s
  and invalidate_trait_use_conflict_resolution : trait_use_conflict_resolution invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TraitUseConflictResolution
      { Syntax.trait_use_conflict_resolution_keyword = invalidate_token x.trait_use_conflict_resolution_keyword
      ; Syntax.trait_use_conflict_resolution_names = invalidate_list_with (invalidate_specifier) x.trait_use_conflict_resolution_names
      ; Syntax.trait_use_conflict_resolution_left_brace = invalidate_token x.trait_use_conflict_resolution_left_brace
      ; Syntax.trait_use_conflict_resolution_clauses = invalidate_list_with (invalidate_specifier) x.trait_use_conflict_resolution_clauses
      ; Syntax.trait_use_conflict_resolution_right_brace = invalidate_token x.trait_use_conflict_resolution_right_brace
      }
    ; Syntax.value = v
    }
  and validate_trait_use : trait_use validator = function
  | { Syntax.syntax = Syntax.TraitUse x; value = v } -> v,
    { trait_use_semicolon = validate_option_with (validate_token) x.Syntax.trait_use_semicolon
    ; trait_use_names = validate_list_with (validate_specifier) x.Syntax.trait_use_names
    ; trait_use_keyword = validate_token x.Syntax.trait_use_keyword
    }
  | s -> validation_fail SyntaxKind.TraitUse s
  and invalidate_trait_use : trait_use invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TraitUse
      { Syntax.trait_use_keyword = invalidate_token x.trait_use_keyword
      ; Syntax.trait_use_names = invalidate_list_with (invalidate_specifier) x.trait_use_names
      ; Syntax.trait_use_semicolon = invalidate_option_with (invalidate_token) x.trait_use_semicolon
      }
    ; Syntax.value = v
    }
  and validate_require_clause : require_clause validator = function
  | { Syntax.syntax = Syntax.RequireClause x; value = v } -> v,
    { require_semicolon = validate_token x.Syntax.require_semicolon
    ; require_name = validate_specifier x.Syntax.require_name
    ; require_kind = validate_token x.Syntax.require_kind
    ; require_keyword = validate_token x.Syntax.require_keyword
    }
  | s -> validation_fail SyntaxKind.RequireClause s
  and invalidate_require_clause : require_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.RequireClause
      { Syntax.require_keyword = invalidate_token x.require_keyword
      ; Syntax.require_kind = invalidate_token x.require_kind
      ; Syntax.require_name = invalidate_specifier x.require_name
      ; Syntax.require_semicolon = invalidate_token x.require_semicolon
      }
    ; Syntax.value = v
    }
  and validate_const_declaration : const_declaration validator = function
  | { Syntax.syntax = Syntax.ConstDeclaration x; value = v } -> v,
    { const_semicolon = validate_token x.Syntax.const_semicolon
    ; const_declarators = validate_list_with (validate_constant_declarator) x.Syntax.const_declarators
    ; const_type_specifier = validate_option_with (validate_specifier) x.Syntax.const_type_specifier
    ; const_keyword = validate_token x.Syntax.const_keyword
    ; const_abstract = validate_option_with (validate_token) x.Syntax.const_abstract
    }
  | s -> validation_fail SyntaxKind.ConstDeclaration s
  and invalidate_const_declaration : const_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ConstDeclaration
      { Syntax.const_abstract = invalidate_option_with (invalidate_token) x.const_abstract
      ; Syntax.const_keyword = invalidate_token x.const_keyword
      ; Syntax.const_type_specifier = invalidate_option_with (invalidate_specifier) x.const_type_specifier
      ; Syntax.const_declarators = invalidate_list_with (invalidate_constant_declarator) x.const_declarators
      ; Syntax.const_semicolon = invalidate_token x.const_semicolon
      }
    ; Syntax.value = v
    }
  and validate_constant_declarator : constant_declarator validator = function
  | { Syntax.syntax = Syntax.ConstantDeclarator x; value = v } -> v,
    { constant_declarator_initializer = validate_option_with (validate_simple_initializer) x.Syntax.constant_declarator_initializer
    ; constant_declarator_name = validate_token x.Syntax.constant_declarator_name
    }
  | s -> validation_fail SyntaxKind.ConstantDeclarator s
  and invalidate_constant_declarator : constant_declarator invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ConstantDeclarator
      { Syntax.constant_declarator_name = invalidate_token x.constant_declarator_name
      ; Syntax.constant_declarator_initializer = invalidate_option_with (invalidate_simple_initializer) x.constant_declarator_initializer
      }
    ; Syntax.value = v
    }
  and validate_type_const_declaration : type_const_declaration validator = function
  | { Syntax.syntax = Syntax.TypeConstDeclaration x; value = v } -> v,
    { type_const_semicolon = validate_token x.Syntax.type_const_semicolon
    ; type_const_type_specifier = validate_option_with (validate_specifier) x.Syntax.type_const_type_specifier
    ; type_const_equal = validate_option_with (validate_token) x.Syntax.type_const_equal
    ; type_const_type_constraint = validate_option_with (validate_type_constraint) x.Syntax.type_const_type_constraint
    ; type_const_name = validate_token x.Syntax.type_const_name
    ; type_const_type_keyword = validate_token x.Syntax.type_const_type_keyword
    ; type_const_keyword = validate_token x.Syntax.type_const_keyword
    ; type_const_abstract = validate_option_with (validate_token) x.Syntax.type_const_abstract
    }
  | s -> validation_fail SyntaxKind.TypeConstDeclaration s
  and invalidate_type_const_declaration : type_const_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeConstDeclaration
      { Syntax.type_const_abstract = invalidate_option_with (invalidate_token) x.type_const_abstract
      ; Syntax.type_const_keyword = invalidate_token x.type_const_keyword
      ; Syntax.type_const_type_keyword = invalidate_token x.type_const_type_keyword
      ; Syntax.type_const_name = invalidate_token x.type_const_name
      ; Syntax.type_const_type_constraint = invalidate_option_with (invalidate_type_constraint) x.type_const_type_constraint
      ; Syntax.type_const_equal = invalidate_option_with (invalidate_token) x.type_const_equal
      ; Syntax.type_const_type_specifier = invalidate_option_with (invalidate_specifier) x.type_const_type_specifier
      ; Syntax.type_const_semicolon = invalidate_token x.type_const_semicolon
      }
    ; Syntax.value = v
    }
  and validate_decorated_expression : decorated_expression validator = function
  | { Syntax.syntax = Syntax.DecoratedExpression x; value = v } -> v,
    { decorated_expression_expression = validate_expression x.Syntax.decorated_expression_expression
    ; decorated_expression_decorator = validate_token x.Syntax.decorated_expression_decorator
    }
  | s -> validation_fail SyntaxKind.DecoratedExpression s
  and invalidate_decorated_expression : decorated_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DecoratedExpression
      { Syntax.decorated_expression_decorator = invalidate_token x.decorated_expression_decorator
      ; Syntax.decorated_expression_expression = invalidate_expression x.decorated_expression_expression
      }
    ; Syntax.value = v
    }
  and validate_parameter_declaration : parameter_declaration validator = function
  | { Syntax.syntax = Syntax.ParameterDeclaration x; value = v } -> v,
    { parameter_default_value = validate_option_with (validate_simple_initializer) x.Syntax.parameter_default_value
    ; parameter_name = validate_expression x.Syntax.parameter_name
    ; parameter_type = validate_option_with (validate_specifier) x.Syntax.parameter_type
    ; parameter_visibility = validate_option_with (validate_token) x.Syntax.parameter_visibility
    ; parameter_attribute = validate_option_with (validate_attribute_specification) x.Syntax.parameter_attribute
    }
  | s -> validation_fail SyntaxKind.ParameterDeclaration s
  and invalidate_parameter_declaration : parameter_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ParameterDeclaration
      { Syntax.parameter_attribute = invalidate_option_with (invalidate_attribute_specification) x.parameter_attribute
      ; Syntax.parameter_visibility = invalidate_option_with (invalidate_token) x.parameter_visibility
      ; Syntax.parameter_type = invalidate_option_with (invalidate_specifier) x.parameter_type
      ; Syntax.parameter_name = invalidate_expression x.parameter_name
      ; Syntax.parameter_default_value = invalidate_option_with (invalidate_simple_initializer) x.parameter_default_value
      }
    ; Syntax.value = v
    }
  and validate_variadic_parameter : variadic_parameter validator = function
  | { Syntax.syntax = Syntax.VariadicParameter x; value = v } -> v,
    { variadic_parameter_ellipsis = validate_token x.Syntax.variadic_parameter_ellipsis
    }
  | s -> validation_fail SyntaxKind.VariadicParameter s
  and invalidate_variadic_parameter : variadic_parameter invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VariadicParameter
      { Syntax.variadic_parameter_ellipsis = invalidate_token x.variadic_parameter_ellipsis
      }
    ; Syntax.value = v
    }
  and validate_attribute_specification : attribute_specification validator = function
  | { Syntax.syntax = Syntax.AttributeSpecification x; value = v } -> v,
    { attribute_specification_right_double_angle = validate_token x.Syntax.attribute_specification_right_double_angle
    ; attribute_specification_attributes = validate_list_with (validate_attribute) x.Syntax.attribute_specification_attributes
    ; attribute_specification_left_double_angle = validate_token x.Syntax.attribute_specification_left_double_angle
    }
  | s -> validation_fail SyntaxKind.AttributeSpecification s
  and invalidate_attribute_specification : attribute_specification invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.AttributeSpecification
      { Syntax.attribute_specification_left_double_angle = invalidate_token x.attribute_specification_left_double_angle
      ; Syntax.attribute_specification_attributes = invalidate_list_with (invalidate_attribute) x.attribute_specification_attributes
      ; Syntax.attribute_specification_right_double_angle = invalidate_token x.attribute_specification_right_double_angle
      }
    ; Syntax.value = v
    }
  and validate_attribute : attribute validator = function
  | { Syntax.syntax = Syntax.Attribute x; value = v } -> v,
    { attribute_right_paren = validate_option_with (validate_token) x.Syntax.attribute_right_paren
    ; attribute_values = validate_list_with (validate_expression) x.Syntax.attribute_values
    ; attribute_left_paren = validate_option_with (validate_token) x.Syntax.attribute_left_paren
    ; attribute_name = validate_token x.Syntax.attribute_name
    }
  | s -> validation_fail SyntaxKind.Attribute s
  and invalidate_attribute : attribute invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.Attribute
      { Syntax.attribute_name = invalidate_token x.attribute_name
      ; Syntax.attribute_left_paren = invalidate_option_with (invalidate_token) x.attribute_left_paren
      ; Syntax.attribute_values = invalidate_list_with (invalidate_expression) x.attribute_values
      ; Syntax.attribute_right_paren = invalidate_option_with (invalidate_token) x.attribute_right_paren
      }
    ; Syntax.value = v
    }
  and validate_inclusion_expression : inclusion_expression validator = function
  | { Syntax.syntax = Syntax.InclusionExpression x; value = v } -> v,
    { inclusion_filename = validate_expression x.Syntax.inclusion_filename
    ; inclusion_require = validate_token x.Syntax.inclusion_require
    }
  | s -> validation_fail SyntaxKind.InclusionExpression s
  and invalidate_inclusion_expression : inclusion_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.InclusionExpression
      { Syntax.inclusion_require = invalidate_token x.inclusion_require
      ; Syntax.inclusion_filename = invalidate_expression x.inclusion_filename
      }
    ; Syntax.value = v
    }
  and validate_inclusion_directive : inclusion_directive validator = function
  | { Syntax.syntax = Syntax.InclusionDirective x; value = v } -> v,
    { inclusion_semicolon = validate_token x.Syntax.inclusion_semicolon
    ; inclusion_expression = validate_inclusion_expression x.Syntax.inclusion_expression
    }
  | s -> validation_fail SyntaxKind.InclusionDirective s
  and invalidate_inclusion_directive : inclusion_directive invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.InclusionDirective
      { Syntax.inclusion_expression = invalidate_inclusion_expression x.inclusion_expression
      ; Syntax.inclusion_semicolon = invalidate_token x.inclusion_semicolon
      }
    ; Syntax.value = v
    }
  and validate_compound_statement : compound_statement validator = function
  | { Syntax.syntax = Syntax.CompoundStatement x; value = v } -> v,
    { compound_right_brace = validate_token x.Syntax.compound_right_brace
    ; compound_statements = validate_list_with (validate_statement) x.Syntax.compound_statements
    ; compound_left_brace = validate_token x.Syntax.compound_left_brace
    }
  | s -> validation_fail SyntaxKind.CompoundStatement s
  and invalidate_compound_statement : compound_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.CompoundStatement
      { Syntax.compound_left_brace = invalidate_token x.compound_left_brace
      ; Syntax.compound_statements = invalidate_list_with (invalidate_statement) x.compound_statements
      ; Syntax.compound_right_brace = invalidate_token x.compound_right_brace
      }
    ; Syntax.value = v
    }
  and validate_expression_statement : expression_statement validator = function
  | { Syntax.syntax = Syntax.ExpressionStatement x; value = v } -> v,
    { expression_statement_semicolon = validate_token x.Syntax.expression_statement_semicolon
    ; expression_statement_expression = validate_option_with (validate_expression) x.Syntax.expression_statement_expression
    }
  | s -> validation_fail SyntaxKind.ExpressionStatement s
  and invalidate_expression_statement : expression_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ExpressionStatement
      { Syntax.expression_statement_expression = invalidate_option_with (invalidate_expression) x.expression_statement_expression
      ; Syntax.expression_statement_semicolon = invalidate_token x.expression_statement_semicolon
      }
    ; Syntax.value = v
    }
  and validate_markup_section : markup_section validator = function
  | { Syntax.syntax = Syntax.MarkupSection x; value = v } -> v,
    { markup_expression = validate_option_with (validate_expression) x.Syntax.markup_expression
    ; markup_suffix = validate_option_with (validate_markup_suffix) x.Syntax.markup_suffix
    ; markup_text = validate_token x.Syntax.markup_text
    ; markup_prefix = validate_option_with (validate_token) x.Syntax.markup_prefix
    }
  | s -> validation_fail SyntaxKind.MarkupSection s
  and invalidate_markup_section : markup_section invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.MarkupSection
      { Syntax.markup_prefix = invalidate_option_with (invalidate_token) x.markup_prefix
      ; Syntax.markup_text = invalidate_token x.markup_text
      ; Syntax.markup_suffix = invalidate_option_with (invalidate_markup_suffix) x.markup_suffix
      ; Syntax.markup_expression = invalidate_option_with (invalidate_expression) x.markup_expression
      }
    ; Syntax.value = v
    }
  and validate_markup_suffix : markup_suffix validator = function
  | { Syntax.syntax = Syntax.MarkupSuffix x; value = v } -> v,
    { markup_suffix_name = validate_option_with (validate_token) x.Syntax.markup_suffix_name
    ; markup_suffix_less_than_question = validate_token x.Syntax.markup_suffix_less_than_question
    }
  | s -> validation_fail SyntaxKind.MarkupSuffix s
  and invalidate_markup_suffix : markup_suffix invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.MarkupSuffix
      { Syntax.markup_suffix_less_than_question = invalidate_token x.markup_suffix_less_than_question
      ; Syntax.markup_suffix_name = invalidate_option_with (invalidate_token) x.markup_suffix_name
      }
    ; Syntax.value = v
    }
  and validate_unset_statement : unset_statement validator = function
  | { Syntax.syntax = Syntax.UnsetStatement x; value = v } -> v,
    { unset_semicolon = validate_token x.Syntax.unset_semicolon
    ; unset_right_paren = validate_token x.Syntax.unset_right_paren
    ; unset_variables = validate_list_with (validate_expression) x.Syntax.unset_variables
    ; unset_left_paren = validate_token x.Syntax.unset_left_paren
    ; unset_keyword = validate_token x.Syntax.unset_keyword
    }
  | s -> validation_fail SyntaxKind.UnsetStatement s
  and invalidate_unset_statement : unset_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.UnsetStatement
      { Syntax.unset_keyword = invalidate_token x.unset_keyword
      ; Syntax.unset_left_paren = invalidate_token x.unset_left_paren
      ; Syntax.unset_variables = invalidate_list_with (invalidate_expression) x.unset_variables
      ; Syntax.unset_right_paren = invalidate_token x.unset_right_paren
      ; Syntax.unset_semicolon = invalidate_token x.unset_semicolon
      }
    ; Syntax.value = v
    }
  and validate_while_statement : while_statement validator = function
  | { Syntax.syntax = Syntax.WhileStatement x; value = v } -> v,
    { while_body = validate_statement x.Syntax.while_body
    ; while_right_paren = validate_token x.Syntax.while_right_paren
    ; while_condition = validate_expression x.Syntax.while_condition
    ; while_left_paren = validate_token x.Syntax.while_left_paren
    ; while_keyword = validate_token x.Syntax.while_keyword
    }
  | s -> validation_fail SyntaxKind.WhileStatement s
  and invalidate_while_statement : while_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.WhileStatement
      { Syntax.while_keyword = invalidate_token x.while_keyword
      ; Syntax.while_left_paren = invalidate_token x.while_left_paren
      ; Syntax.while_condition = invalidate_expression x.while_condition
      ; Syntax.while_right_paren = invalidate_token x.while_right_paren
      ; Syntax.while_body = invalidate_statement x.while_body
      }
    ; Syntax.value = v
    }
  and validate_if_statement : if_statement validator = function
  | { Syntax.syntax = Syntax.IfStatement x; value = v } -> v,
    { if_else_clause = validate_option_with (validate_else_clause) x.Syntax.if_else_clause
    ; if_elseif_clauses = validate_list_with (validate_elseif_clause) x.Syntax.if_elseif_clauses
    ; if_statement = validate_statement x.Syntax.if_statement
    ; if_right_paren = validate_token x.Syntax.if_right_paren
    ; if_condition = validate_expression x.Syntax.if_condition
    ; if_left_paren = validate_token x.Syntax.if_left_paren
    ; if_keyword = validate_token x.Syntax.if_keyword
    }
  | s -> validation_fail SyntaxKind.IfStatement s
  and invalidate_if_statement : if_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.IfStatement
      { Syntax.if_keyword = invalidate_token x.if_keyword
      ; Syntax.if_left_paren = invalidate_token x.if_left_paren
      ; Syntax.if_condition = invalidate_expression x.if_condition
      ; Syntax.if_right_paren = invalidate_token x.if_right_paren
      ; Syntax.if_statement = invalidate_statement x.if_statement
      ; Syntax.if_elseif_clauses = invalidate_list_with (invalidate_elseif_clause) x.if_elseif_clauses
      ; Syntax.if_else_clause = invalidate_option_with (invalidate_else_clause) x.if_else_clause
      }
    ; Syntax.value = v
    }
  and validate_elseif_clause : elseif_clause validator = function
  | { Syntax.syntax = Syntax.ElseifClause x; value = v } -> v,
    { elseif_statement = validate_statement x.Syntax.elseif_statement
    ; elseif_right_paren = validate_token x.Syntax.elseif_right_paren
    ; elseif_condition = validate_expression x.Syntax.elseif_condition
    ; elseif_left_paren = validate_token x.Syntax.elseif_left_paren
    ; elseif_keyword = validate_token x.Syntax.elseif_keyword
    }
  | s -> validation_fail SyntaxKind.ElseifClause s
  and invalidate_elseif_clause : elseif_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ElseifClause
      { Syntax.elseif_keyword = invalidate_token x.elseif_keyword
      ; Syntax.elseif_left_paren = invalidate_token x.elseif_left_paren
      ; Syntax.elseif_condition = invalidate_expression x.elseif_condition
      ; Syntax.elseif_right_paren = invalidate_token x.elseif_right_paren
      ; Syntax.elseif_statement = invalidate_statement x.elseif_statement
      }
    ; Syntax.value = v
    }
  and validate_else_clause : else_clause validator = function
  | { Syntax.syntax = Syntax.ElseClause x; value = v } -> v,
    { else_statement = validate_statement x.Syntax.else_statement
    ; else_keyword = validate_token x.Syntax.else_keyword
    }
  | s -> validation_fail SyntaxKind.ElseClause s
  and invalidate_else_clause : else_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ElseClause
      { Syntax.else_keyword = invalidate_token x.else_keyword
      ; Syntax.else_statement = invalidate_statement x.else_statement
      }
    ; Syntax.value = v
    }
  and validate_try_statement : try_statement validator = function
  | { Syntax.syntax = Syntax.TryStatement x; value = v } -> v,
    { try_finally_clause = validate_option_with (validate_finally_clause) x.Syntax.try_finally_clause
    ; try_catch_clauses = validate_list_with (validate_catch_clause) x.Syntax.try_catch_clauses
    ; try_compound_statement = validate_compound_statement x.Syntax.try_compound_statement
    ; try_keyword = validate_token x.Syntax.try_keyword
    }
  | s -> validation_fail SyntaxKind.TryStatement s
  and invalidate_try_statement : try_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TryStatement
      { Syntax.try_keyword = invalidate_token x.try_keyword
      ; Syntax.try_compound_statement = invalidate_compound_statement x.try_compound_statement
      ; Syntax.try_catch_clauses = invalidate_list_with (invalidate_catch_clause) x.try_catch_clauses
      ; Syntax.try_finally_clause = invalidate_option_with (invalidate_finally_clause) x.try_finally_clause
      }
    ; Syntax.value = v
    }
  and validate_catch_clause : catch_clause validator = function
  | { Syntax.syntax = Syntax.CatchClause x; value = v } -> v,
    { catch_body = validate_compound_statement x.Syntax.catch_body
    ; catch_right_paren = validate_token x.Syntax.catch_right_paren
    ; catch_variable = validate_token x.Syntax.catch_variable
    ; catch_type = validate_simple_type_specifier x.Syntax.catch_type
    ; catch_left_paren = validate_token x.Syntax.catch_left_paren
    ; catch_keyword = validate_token x.Syntax.catch_keyword
    }
  | s -> validation_fail SyntaxKind.CatchClause s
  and invalidate_catch_clause : catch_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.CatchClause
      { Syntax.catch_keyword = invalidate_token x.catch_keyword
      ; Syntax.catch_left_paren = invalidate_token x.catch_left_paren
      ; Syntax.catch_type = invalidate_simple_type_specifier x.catch_type
      ; Syntax.catch_variable = invalidate_token x.catch_variable
      ; Syntax.catch_right_paren = invalidate_token x.catch_right_paren
      ; Syntax.catch_body = invalidate_compound_statement x.catch_body
      }
    ; Syntax.value = v
    }
  and validate_finally_clause : finally_clause validator = function
  | { Syntax.syntax = Syntax.FinallyClause x; value = v } -> v,
    { finally_body = validate_compound_statement x.Syntax.finally_body
    ; finally_keyword = validate_token x.Syntax.finally_keyword
    }
  | s -> validation_fail SyntaxKind.FinallyClause s
  and invalidate_finally_clause : finally_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FinallyClause
      { Syntax.finally_keyword = invalidate_token x.finally_keyword
      ; Syntax.finally_body = invalidate_compound_statement x.finally_body
      }
    ; Syntax.value = v
    }
  and validate_do_statement : do_statement validator = function
  | { Syntax.syntax = Syntax.DoStatement x; value = v } -> v,
    { do_semicolon = validate_token x.Syntax.do_semicolon
    ; do_right_paren = validate_token x.Syntax.do_right_paren
    ; do_condition = validate_expression x.Syntax.do_condition
    ; do_left_paren = validate_token x.Syntax.do_left_paren
    ; do_while_keyword = validate_token x.Syntax.do_while_keyword
    ; do_body = validate_statement x.Syntax.do_body
    ; do_keyword = validate_token x.Syntax.do_keyword
    }
  | s -> validation_fail SyntaxKind.DoStatement s
  and invalidate_do_statement : do_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DoStatement
      { Syntax.do_keyword = invalidate_token x.do_keyword
      ; Syntax.do_body = invalidate_statement x.do_body
      ; Syntax.do_while_keyword = invalidate_token x.do_while_keyword
      ; Syntax.do_left_paren = invalidate_token x.do_left_paren
      ; Syntax.do_condition = invalidate_expression x.do_condition
      ; Syntax.do_right_paren = invalidate_token x.do_right_paren
      ; Syntax.do_semicolon = invalidate_token x.do_semicolon
      }
    ; Syntax.value = v
    }
  and validate_for_statement : for_statement validator = function
  | { Syntax.syntax = Syntax.ForStatement x; value = v } -> v,
    { for_body = validate_statement x.Syntax.for_body
    ; for_right_paren = validate_token x.Syntax.for_right_paren
    ; for_end_of_loop = validate_list_with (validate_expression) x.Syntax.for_end_of_loop
    ; for_second_semicolon = validate_token x.Syntax.for_second_semicolon
    ; for_control = validate_list_with (validate_expression) x.Syntax.for_control
    ; for_first_semicolon = validate_token x.Syntax.for_first_semicolon
    ; for_initializer = validate_list_with (validate_expression) x.Syntax.for_initializer
    ; for_left_paren = validate_token x.Syntax.for_left_paren
    ; for_keyword = validate_token x.Syntax.for_keyword
    }
  | s -> validation_fail SyntaxKind.ForStatement s
  and invalidate_for_statement : for_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ForStatement
      { Syntax.for_keyword = invalidate_token x.for_keyword
      ; Syntax.for_left_paren = invalidate_token x.for_left_paren
      ; Syntax.for_initializer = invalidate_list_with (invalidate_expression) x.for_initializer
      ; Syntax.for_first_semicolon = invalidate_token x.for_first_semicolon
      ; Syntax.for_control = invalidate_list_with (invalidate_expression) x.for_control
      ; Syntax.for_second_semicolon = invalidate_token x.for_second_semicolon
      ; Syntax.for_end_of_loop = invalidate_list_with (invalidate_expression) x.for_end_of_loop
      ; Syntax.for_right_paren = invalidate_token x.for_right_paren
      ; Syntax.for_body = invalidate_statement x.for_body
      }
    ; Syntax.value = v
    }
  and validate_foreach_statement : foreach_statement validator = function
  | { Syntax.syntax = Syntax.ForeachStatement x; value = v } -> v,
    { foreach_body = validate_statement x.Syntax.foreach_body
    ; foreach_right_paren = validate_token x.Syntax.foreach_right_paren
    ; foreach_value = validate_expression x.Syntax.foreach_value
    ; foreach_arrow = validate_option_with (validate_token) x.Syntax.foreach_arrow
    ; foreach_key = validate_option_with (validate_expression) x.Syntax.foreach_key
    ; foreach_as = validate_token x.Syntax.foreach_as
    ; foreach_await_keyword = validate_option_with (validate_token) x.Syntax.foreach_await_keyword
    ; foreach_collection = validate_expression x.Syntax.foreach_collection
    ; foreach_left_paren = validate_token x.Syntax.foreach_left_paren
    ; foreach_keyword = validate_token x.Syntax.foreach_keyword
    }
  | s -> validation_fail SyntaxKind.ForeachStatement s
  and invalidate_foreach_statement : foreach_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ForeachStatement
      { Syntax.foreach_keyword = invalidate_token x.foreach_keyword
      ; Syntax.foreach_left_paren = invalidate_token x.foreach_left_paren
      ; Syntax.foreach_collection = invalidate_expression x.foreach_collection
      ; Syntax.foreach_await_keyword = invalidate_option_with (invalidate_token) x.foreach_await_keyword
      ; Syntax.foreach_as = invalidate_token x.foreach_as
      ; Syntax.foreach_key = invalidate_option_with (invalidate_expression) x.foreach_key
      ; Syntax.foreach_arrow = invalidate_option_with (invalidate_token) x.foreach_arrow
      ; Syntax.foreach_value = invalidate_expression x.foreach_value
      ; Syntax.foreach_right_paren = invalidate_token x.foreach_right_paren
      ; Syntax.foreach_body = invalidate_statement x.foreach_body
      }
    ; Syntax.value = v
    }
  and validate_switch_statement : switch_statement validator = function
  | { Syntax.syntax = Syntax.SwitchStatement x; value = v } -> v,
    { switch_right_brace = validate_token x.Syntax.switch_right_brace
    ; switch_sections = validate_list_with (validate_switch_section) x.Syntax.switch_sections
    ; switch_left_brace = validate_token x.Syntax.switch_left_brace
    ; switch_right_paren = validate_token x.Syntax.switch_right_paren
    ; switch_expression = validate_expression x.Syntax.switch_expression
    ; switch_left_paren = validate_token x.Syntax.switch_left_paren
    ; switch_keyword = validate_token x.Syntax.switch_keyword
    }
  | s -> validation_fail SyntaxKind.SwitchStatement s
  and invalidate_switch_statement : switch_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SwitchStatement
      { Syntax.switch_keyword = invalidate_token x.switch_keyword
      ; Syntax.switch_left_paren = invalidate_token x.switch_left_paren
      ; Syntax.switch_expression = invalidate_expression x.switch_expression
      ; Syntax.switch_right_paren = invalidate_token x.switch_right_paren
      ; Syntax.switch_left_brace = invalidate_token x.switch_left_brace
      ; Syntax.switch_sections = invalidate_list_with (invalidate_switch_section) x.switch_sections
      ; Syntax.switch_right_brace = invalidate_token x.switch_right_brace
      }
    ; Syntax.value = v
    }
  and validate_switch_section : switch_section validator = function
  | { Syntax.syntax = Syntax.SwitchSection x; value = v } -> v,
    { switch_section_fallthrough = validate_option_with (validate_switch_fallthrough) x.Syntax.switch_section_fallthrough
    ; switch_section_statements = validate_list_with (validate_top_level_declaration) x.Syntax.switch_section_statements
    ; switch_section_labels = validate_list_with (validate_switch_label) x.Syntax.switch_section_labels
    }
  | s -> validation_fail SyntaxKind.SwitchSection s
  and invalidate_switch_section : switch_section invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SwitchSection
      { Syntax.switch_section_labels = invalidate_list_with (invalidate_switch_label) x.switch_section_labels
      ; Syntax.switch_section_statements = invalidate_list_with (invalidate_top_level_declaration) x.switch_section_statements
      ; Syntax.switch_section_fallthrough = invalidate_option_with (invalidate_switch_fallthrough) x.switch_section_fallthrough
      }
    ; Syntax.value = v
    }
  and validate_switch_fallthrough : switch_fallthrough validator = function
  | { Syntax.syntax = Syntax.SwitchFallthrough x; value = v } -> v,
    { fallthrough_semicolon = validate_token x.Syntax.fallthrough_semicolon
    ; fallthrough_keyword = validate_token x.Syntax.fallthrough_keyword
    }
  | s -> validation_fail SyntaxKind.SwitchFallthrough s
  and invalidate_switch_fallthrough : switch_fallthrough invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SwitchFallthrough
      { Syntax.fallthrough_keyword = invalidate_token x.fallthrough_keyword
      ; Syntax.fallthrough_semicolon = invalidate_token x.fallthrough_semicolon
      }
    ; Syntax.value = v
    }
  and validate_case_label : case_label validator = function
  | { Syntax.syntax = Syntax.CaseLabel x; value = v } -> v,
    { case_colon = validate_token x.Syntax.case_colon
    ; case_expression = validate_expression x.Syntax.case_expression
    ; case_keyword = validate_token x.Syntax.case_keyword
    }
  | s -> validation_fail SyntaxKind.CaseLabel s
  and invalidate_case_label : case_label invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.CaseLabel
      { Syntax.case_keyword = invalidate_token x.case_keyword
      ; Syntax.case_expression = invalidate_expression x.case_expression
      ; Syntax.case_colon = invalidate_token x.case_colon
      }
    ; Syntax.value = v
    }
  and validate_default_label : default_label validator = function
  | { Syntax.syntax = Syntax.DefaultLabel x; value = v } -> v,
    { default_colon = validate_token x.Syntax.default_colon
    ; default_keyword = validate_token x.Syntax.default_keyword
    }
  | s -> validation_fail SyntaxKind.DefaultLabel s
  and invalidate_default_label : default_label invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DefaultLabel
      { Syntax.default_keyword = invalidate_token x.default_keyword
      ; Syntax.default_colon = invalidate_token x.default_colon
      }
    ; Syntax.value = v
    }
  and validate_return_statement : return_statement validator = function
  | { Syntax.syntax = Syntax.ReturnStatement x; value = v } -> v,
    { return_semicolon = validate_option_with (validate_token) x.Syntax.return_semicolon
    ; return_expression = validate_option_with (validate_expression) x.Syntax.return_expression
    ; return_keyword = validate_token x.Syntax.return_keyword
    }
  | s -> validation_fail SyntaxKind.ReturnStatement s
  and invalidate_return_statement : return_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ReturnStatement
      { Syntax.return_keyword = invalidate_token x.return_keyword
      ; Syntax.return_expression = invalidate_option_with (invalidate_expression) x.return_expression
      ; Syntax.return_semicolon = invalidate_option_with (invalidate_token) x.return_semicolon
      }
    ; Syntax.value = v
    }
  and validate_goto_label : goto_label validator = function
  | { Syntax.syntax = Syntax.GotoLabel x; value = v } -> v,
    { goto_label_colon = validate_token x.Syntax.goto_label_colon
    ; goto_label_name = validate_token x.Syntax.goto_label_name
    }
  | s -> validation_fail SyntaxKind.GotoLabel s
  and invalidate_goto_label : goto_label invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.GotoLabel
      { Syntax.goto_label_name = invalidate_token x.goto_label_name
      ; Syntax.goto_label_colon = invalidate_token x.goto_label_colon
      }
    ; Syntax.value = v
    }
  and validate_goto_statement : goto_statement validator = function
  | { Syntax.syntax = Syntax.GotoStatement x; value = v } -> v,
    { goto_statement_semicolon = validate_token x.Syntax.goto_statement_semicolon
    ; goto_statement_label_name = validate_token x.Syntax.goto_statement_label_name
    ; goto_statement_keyword = validate_token x.Syntax.goto_statement_keyword
    }
  | s -> validation_fail SyntaxKind.GotoStatement s
  and invalidate_goto_statement : goto_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.GotoStatement
      { Syntax.goto_statement_keyword = invalidate_token x.goto_statement_keyword
      ; Syntax.goto_statement_label_name = invalidate_token x.goto_statement_label_name
      ; Syntax.goto_statement_semicolon = invalidate_token x.goto_statement_semicolon
      }
    ; Syntax.value = v
    }
  and validate_throw_statement : throw_statement validator = function
  | { Syntax.syntax = Syntax.ThrowStatement x; value = v } -> v,
    { throw_semicolon = validate_token x.Syntax.throw_semicolon
    ; throw_expression = validate_expression x.Syntax.throw_expression
    ; throw_keyword = validate_token x.Syntax.throw_keyword
    }
  | s -> validation_fail SyntaxKind.ThrowStatement s
  and invalidate_throw_statement : throw_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ThrowStatement
      { Syntax.throw_keyword = invalidate_token x.throw_keyword
      ; Syntax.throw_expression = invalidate_expression x.throw_expression
      ; Syntax.throw_semicolon = invalidate_token x.throw_semicolon
      }
    ; Syntax.value = v
    }
  and validate_break_statement : break_statement validator = function
  | { Syntax.syntax = Syntax.BreakStatement x; value = v } -> v,
    { break_semicolon = validate_token x.Syntax.break_semicolon
    ; break_level = validate_option_with (validate_literal_expression) x.Syntax.break_level
    ; break_keyword = validate_token x.Syntax.break_keyword
    }
  | s -> validation_fail SyntaxKind.BreakStatement s
  and invalidate_break_statement : break_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.BreakStatement
      { Syntax.break_keyword = invalidate_token x.break_keyword
      ; Syntax.break_level = invalidate_option_with (invalidate_literal_expression) x.break_level
      ; Syntax.break_semicolon = invalidate_token x.break_semicolon
      }
    ; Syntax.value = v
    }
  and validate_continue_statement : continue_statement validator = function
  | { Syntax.syntax = Syntax.ContinueStatement x; value = v } -> v,
    { continue_semicolon = validate_token x.Syntax.continue_semicolon
    ; continue_level = validate_option_with (validate_literal_expression) x.Syntax.continue_level
    ; continue_keyword = validate_token x.Syntax.continue_keyword
    }
  | s -> validation_fail SyntaxKind.ContinueStatement s
  and invalidate_continue_statement : continue_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ContinueStatement
      { Syntax.continue_keyword = invalidate_token x.continue_keyword
      ; Syntax.continue_level = invalidate_option_with (invalidate_literal_expression) x.continue_level
      ; Syntax.continue_semicolon = invalidate_token x.continue_semicolon
      }
    ; Syntax.value = v
    }
  and validate_function_static_statement : function_static_statement validator = function
  | { Syntax.syntax = Syntax.FunctionStaticStatement x; value = v } -> v,
    { static_semicolon = validate_token x.Syntax.static_semicolon
    ; static_declarations = validate_list_with (validate_static_declarator) x.Syntax.static_declarations
    ; static_static_keyword = validate_token x.Syntax.static_static_keyword
    }
  | s -> validation_fail SyntaxKind.FunctionStaticStatement s
  and invalidate_function_static_statement : function_static_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FunctionStaticStatement
      { Syntax.static_static_keyword = invalidate_token x.static_static_keyword
      ; Syntax.static_declarations = invalidate_list_with (invalidate_static_declarator) x.static_declarations
      ; Syntax.static_semicolon = invalidate_token x.static_semicolon
      }
    ; Syntax.value = v
    }
  and validate_static_declarator : static_declarator validator = function
  | { Syntax.syntax = Syntax.StaticDeclarator x; value = v } -> v,
    { static_initializer = validate_option_with (validate_simple_initializer) x.Syntax.static_initializer
    ; static_name = validate_token x.Syntax.static_name
    }
  | s -> validation_fail SyntaxKind.StaticDeclarator s
  and invalidate_static_declarator : static_declarator invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.StaticDeclarator
      { Syntax.static_name = invalidate_token x.static_name
      ; Syntax.static_initializer = invalidate_option_with (invalidate_simple_initializer) x.static_initializer
      }
    ; Syntax.value = v
    }
  and validate_echo_statement : echo_statement validator = function
  | { Syntax.syntax = Syntax.EchoStatement x; value = v } -> v,
    { echo_semicolon = validate_token x.Syntax.echo_semicolon
    ; echo_expressions = validate_list_with (validate_expression) x.Syntax.echo_expressions
    ; echo_keyword = validate_token x.Syntax.echo_keyword
    }
  | s -> validation_fail SyntaxKind.EchoStatement s
  and invalidate_echo_statement : echo_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EchoStatement
      { Syntax.echo_keyword = invalidate_token x.echo_keyword
      ; Syntax.echo_expressions = invalidate_list_with (invalidate_expression) x.echo_expressions
      ; Syntax.echo_semicolon = invalidate_token x.echo_semicolon
      }
    ; Syntax.value = v
    }
  and validate_global_statement : global_statement validator = function
  | { Syntax.syntax = Syntax.GlobalStatement x; value = v } -> v,
    { global_semicolon = validate_token x.Syntax.global_semicolon
    ; global_variables = validate_list_with (validate_token) x.Syntax.global_variables
    ; global_keyword = validate_token x.Syntax.global_keyword
    }
  | s -> validation_fail SyntaxKind.GlobalStatement s
  and invalidate_global_statement : global_statement invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.GlobalStatement
      { Syntax.global_keyword = invalidate_token x.global_keyword
      ; Syntax.global_variables = invalidate_list_with (invalidate_token) x.global_variables
      ; Syntax.global_semicolon = invalidate_token x.global_semicolon
      }
    ; Syntax.value = v
    }
  and validate_simple_initializer : simple_initializer validator = function
  | { Syntax.syntax = Syntax.SimpleInitializer x; value = v } -> v,
    { simple_initializer_value = validate_expression x.Syntax.simple_initializer_value
    ; simple_initializer_equal = validate_token x.Syntax.simple_initializer_equal
    }
  | s -> validation_fail SyntaxKind.SimpleInitializer s
  and invalidate_simple_initializer : simple_initializer invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SimpleInitializer
      { Syntax.simple_initializer_equal = invalidate_token x.simple_initializer_equal
      ; Syntax.simple_initializer_value = invalidate_expression x.simple_initializer_value
      }
    ; Syntax.value = v
    }
  and validate_anonymous_function : anonymous_function validator = function
  | { Syntax.syntax = Syntax.AnonymousFunction x; value = v } -> v,
    { anonymous_body = validate_compound_statement x.Syntax.anonymous_body
    ; anonymous_use = validate_option_with (validate_anonymous_function_use_clause) x.Syntax.anonymous_use
    ; anonymous_type = validate_option_with (validate_specifier) x.Syntax.anonymous_type
    ; anonymous_colon = validate_option_with (validate_token) x.Syntax.anonymous_colon
    ; anonymous_right_paren = validate_token x.Syntax.anonymous_right_paren
    ; anonymous_parameters = validate_list_with (validate_parameter) x.Syntax.anonymous_parameters
    ; anonymous_left_paren = validate_token x.Syntax.anonymous_left_paren
    ; anonymous_function_keyword = validate_token x.Syntax.anonymous_function_keyword
    ; anonymous_coroutine_keyword = validate_option_with (validate_token) x.Syntax.anonymous_coroutine_keyword
    ; anonymous_async_keyword = validate_option_with (validate_token) x.Syntax.anonymous_async_keyword
    ; anonymous_static_keyword = validate_option_with (validate_token) x.Syntax.anonymous_static_keyword
    }
  | s -> validation_fail SyntaxKind.AnonymousFunction s
  and invalidate_anonymous_function : anonymous_function invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.AnonymousFunction
      { Syntax.anonymous_static_keyword = invalidate_option_with (invalidate_token) x.anonymous_static_keyword
      ; Syntax.anonymous_async_keyword = invalidate_option_with (invalidate_token) x.anonymous_async_keyword
      ; Syntax.anonymous_coroutine_keyword = invalidate_option_with (invalidate_token) x.anonymous_coroutine_keyword
      ; Syntax.anonymous_function_keyword = invalidate_token x.anonymous_function_keyword
      ; Syntax.anonymous_left_paren = invalidate_token x.anonymous_left_paren
      ; Syntax.anonymous_parameters = invalidate_list_with (invalidate_parameter) x.anonymous_parameters
      ; Syntax.anonymous_right_paren = invalidate_token x.anonymous_right_paren
      ; Syntax.anonymous_colon = invalidate_option_with (invalidate_token) x.anonymous_colon
      ; Syntax.anonymous_type = invalidate_option_with (invalidate_specifier) x.anonymous_type
      ; Syntax.anonymous_use = invalidate_option_with (invalidate_anonymous_function_use_clause) x.anonymous_use
      ; Syntax.anonymous_body = invalidate_compound_statement x.anonymous_body
      }
    ; Syntax.value = v
    }
  and validate_anonymous_function_use_clause : anonymous_function_use_clause validator = function
  | { Syntax.syntax = Syntax.AnonymousFunctionUseClause x; value = v } -> v,
    { anonymous_use_right_paren = validate_token x.Syntax.anonymous_use_right_paren
    ; anonymous_use_variables = validate_list_with (validate_expression) x.Syntax.anonymous_use_variables
    ; anonymous_use_left_paren = validate_token x.Syntax.anonymous_use_left_paren
    ; anonymous_use_keyword = validate_token x.Syntax.anonymous_use_keyword
    }
  | s -> validation_fail SyntaxKind.AnonymousFunctionUseClause s
  and invalidate_anonymous_function_use_clause : anonymous_function_use_clause invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.AnonymousFunctionUseClause
      { Syntax.anonymous_use_keyword = invalidate_token x.anonymous_use_keyword
      ; Syntax.anonymous_use_left_paren = invalidate_token x.anonymous_use_left_paren
      ; Syntax.anonymous_use_variables = invalidate_list_with (invalidate_expression) x.anonymous_use_variables
      ; Syntax.anonymous_use_right_paren = invalidate_token x.anonymous_use_right_paren
      }
    ; Syntax.value = v
    }
  and validate_lambda_expression : lambda_expression validator = function
  | { Syntax.syntax = Syntax.LambdaExpression x; value = v } -> v,
    { lambda_body = validate_lambda_body x.Syntax.lambda_body
    ; lambda_arrow = validate_token x.Syntax.lambda_arrow
    ; lambda_signature = validate_specifier x.Syntax.lambda_signature
    ; lambda_coroutine = validate_option_with (validate_token) x.Syntax.lambda_coroutine
    ; lambda_async = validate_option_with (validate_token) x.Syntax.lambda_async
    }
  | s -> validation_fail SyntaxKind.LambdaExpression s
  and invalidate_lambda_expression : lambda_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.LambdaExpression
      { Syntax.lambda_async = invalidate_option_with (invalidate_token) x.lambda_async
      ; Syntax.lambda_coroutine = invalidate_option_with (invalidate_token) x.lambda_coroutine
      ; Syntax.lambda_signature = invalidate_specifier x.lambda_signature
      ; Syntax.lambda_arrow = invalidate_token x.lambda_arrow
      ; Syntax.lambda_body = invalidate_lambda_body x.lambda_body
      }
    ; Syntax.value = v
    }
  and validate_lambda_signature : lambda_signature validator = function
  | { Syntax.syntax = Syntax.LambdaSignature x; value = v } -> v,
    { lambda_type = validate_option_with (validate_specifier) x.Syntax.lambda_type
    ; lambda_colon = validate_option_with (validate_token) x.Syntax.lambda_colon
    ; lambda_right_paren = validate_token x.Syntax.lambda_right_paren
    ; lambda_parameters = validate_list_with (validate_parameter) x.Syntax.lambda_parameters
    ; lambda_left_paren = validate_token x.Syntax.lambda_left_paren
    }
  | s -> validation_fail SyntaxKind.LambdaSignature s
  and invalidate_lambda_signature : lambda_signature invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.LambdaSignature
      { Syntax.lambda_left_paren = invalidate_token x.lambda_left_paren
      ; Syntax.lambda_parameters = invalidate_list_with (invalidate_parameter) x.lambda_parameters
      ; Syntax.lambda_right_paren = invalidate_token x.lambda_right_paren
      ; Syntax.lambda_colon = invalidate_option_with (invalidate_token) x.lambda_colon
      ; Syntax.lambda_type = invalidate_option_with (invalidate_specifier) x.lambda_type
      }
    ; Syntax.value = v
    }
  and validate_cast_expression : cast_expression validator = function
  | { Syntax.syntax = Syntax.CastExpression x; value = v } -> v,
    { cast_operand = validate_expression x.Syntax.cast_operand
    ; cast_right_paren = validate_token x.Syntax.cast_right_paren
    ; cast_type = validate_token x.Syntax.cast_type
    ; cast_left_paren = validate_token x.Syntax.cast_left_paren
    }
  | s -> validation_fail SyntaxKind.CastExpression s
  and invalidate_cast_expression : cast_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.CastExpression
      { Syntax.cast_left_paren = invalidate_token x.cast_left_paren
      ; Syntax.cast_type = invalidate_token x.cast_type
      ; Syntax.cast_right_paren = invalidate_token x.cast_right_paren
      ; Syntax.cast_operand = invalidate_expression x.cast_operand
      }
    ; Syntax.value = v
    }
  and validate_scope_resolution_expression : scope_resolution_expression validator = function
  | { Syntax.syntax = Syntax.ScopeResolutionExpression x; value = v } -> v,
    { scope_resolution_name = validate_expression x.Syntax.scope_resolution_name
    ; scope_resolution_operator = validate_token x.Syntax.scope_resolution_operator
    ; scope_resolution_qualifier = validate_expression x.Syntax.scope_resolution_qualifier
    }
  | s -> validation_fail SyntaxKind.ScopeResolutionExpression s
  and invalidate_scope_resolution_expression : scope_resolution_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ScopeResolutionExpression
      { Syntax.scope_resolution_qualifier = invalidate_expression x.scope_resolution_qualifier
      ; Syntax.scope_resolution_operator = invalidate_token x.scope_resolution_operator
      ; Syntax.scope_resolution_name = invalidate_expression x.scope_resolution_name
      }
    ; Syntax.value = v
    }
  and validate_member_selection_expression : member_selection_expression validator = function
  | { Syntax.syntax = Syntax.MemberSelectionExpression x; value = v } -> v,
    { member_name = validate_token x.Syntax.member_name
    ; member_operator = validate_token x.Syntax.member_operator
    ; member_object = validate_expression x.Syntax.member_object
    }
  | s -> validation_fail SyntaxKind.MemberSelectionExpression s
  and invalidate_member_selection_expression : member_selection_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.MemberSelectionExpression
      { Syntax.member_object = invalidate_expression x.member_object
      ; Syntax.member_operator = invalidate_token x.member_operator
      ; Syntax.member_name = invalidate_token x.member_name
      }
    ; Syntax.value = v
    }
  and validate_safe_member_selection_expression : safe_member_selection_expression validator = function
  | { Syntax.syntax = Syntax.SafeMemberSelectionExpression x; value = v } -> v,
    { safe_member_name = validate_token x.Syntax.safe_member_name
    ; safe_member_operator = validate_token x.Syntax.safe_member_operator
    ; safe_member_object = validate_expression x.Syntax.safe_member_object
    }
  | s -> validation_fail SyntaxKind.SafeMemberSelectionExpression s
  and invalidate_safe_member_selection_expression : safe_member_selection_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SafeMemberSelectionExpression
      { Syntax.safe_member_object = invalidate_expression x.safe_member_object
      ; Syntax.safe_member_operator = invalidate_token x.safe_member_operator
      ; Syntax.safe_member_name = invalidate_token x.safe_member_name
      }
    ; Syntax.value = v
    }
  and validate_embedded_member_selection_expression : embedded_member_selection_expression validator = function
  | { Syntax.syntax = Syntax.EmbeddedMemberSelectionExpression x; value = v } -> v,
    { embedded_member_name = validate_token x.Syntax.embedded_member_name
    ; embedded_member_operator = validate_token x.Syntax.embedded_member_operator
    ; embedded_member_object = validate_variable_expression x.Syntax.embedded_member_object
    }
  | s -> validation_fail SyntaxKind.EmbeddedMemberSelectionExpression s
  and invalidate_embedded_member_selection_expression : embedded_member_selection_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EmbeddedMemberSelectionExpression
      { Syntax.embedded_member_object = invalidate_variable_expression x.embedded_member_object
      ; Syntax.embedded_member_operator = invalidate_token x.embedded_member_operator
      ; Syntax.embedded_member_name = invalidate_token x.embedded_member_name
      }
    ; Syntax.value = v
    }
  and validate_yield_expression : yield_expression validator = function
  | { Syntax.syntax = Syntax.YieldExpression x; value = v } -> v,
    { yield_operand = validate_constructor_expression x.Syntax.yield_operand
    ; yield_keyword = validate_token x.Syntax.yield_keyword
    }
  | s -> validation_fail SyntaxKind.YieldExpression s
  and invalidate_yield_expression : yield_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.YieldExpression
      { Syntax.yield_keyword = invalidate_token x.yield_keyword
      ; Syntax.yield_operand = invalidate_constructor_expression x.yield_operand
      }
    ; Syntax.value = v
    }
  and validate_yield_from_expression : yield_from_expression validator = function
  | { Syntax.syntax = Syntax.YieldFromExpression x; value = v } -> v,
    { yield_from_operand = validate_expression x.Syntax.yield_from_operand
    ; yield_from_from_keyword = validate_token x.Syntax.yield_from_from_keyword
    ; yield_from_yield_keyword = validate_token x.Syntax.yield_from_yield_keyword
    }
  | s -> validation_fail SyntaxKind.YieldFromExpression s
  and invalidate_yield_from_expression : yield_from_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.YieldFromExpression
      { Syntax.yield_from_yield_keyword = invalidate_token x.yield_from_yield_keyword
      ; Syntax.yield_from_from_keyword = invalidate_token x.yield_from_from_keyword
      ; Syntax.yield_from_operand = invalidate_expression x.yield_from_operand
      }
    ; Syntax.value = v
    }
  and validate_prefix_unary_expression : prefix_unary_expression validator = function
  | { Syntax.syntax = Syntax.PrefixUnaryExpression x; value = v } -> v,
    { prefix_unary_operand = validate_expression x.Syntax.prefix_unary_operand
    ; prefix_unary_operator = validate_token x.Syntax.prefix_unary_operator
    }
  | s -> validation_fail SyntaxKind.PrefixUnaryExpression s
  and invalidate_prefix_unary_expression : prefix_unary_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.PrefixUnaryExpression
      { Syntax.prefix_unary_operator = invalidate_token x.prefix_unary_operator
      ; Syntax.prefix_unary_operand = invalidate_expression x.prefix_unary_operand
      }
    ; Syntax.value = v
    }
  and validate_postfix_unary_expression : postfix_unary_expression validator = function
  | { Syntax.syntax = Syntax.PostfixUnaryExpression x; value = v } -> v,
    { postfix_unary_operator = validate_token x.Syntax.postfix_unary_operator
    ; postfix_unary_operand = validate_expression x.Syntax.postfix_unary_operand
    }
  | s -> validation_fail SyntaxKind.PostfixUnaryExpression s
  and invalidate_postfix_unary_expression : postfix_unary_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.PostfixUnaryExpression
      { Syntax.postfix_unary_operand = invalidate_expression x.postfix_unary_operand
      ; Syntax.postfix_unary_operator = invalidate_token x.postfix_unary_operator
      }
    ; Syntax.value = v
    }
  and validate_binary_expression : binary_expression validator = function
  | { Syntax.syntax = Syntax.BinaryExpression x; value = v } -> v,
    { binary_right_operand = validate_expression x.Syntax.binary_right_operand
    ; binary_operator = validate_token x.Syntax.binary_operator
    ; binary_left_operand = validate_expression x.Syntax.binary_left_operand
    }
  | s -> validation_fail SyntaxKind.BinaryExpression s
  and invalidate_binary_expression : binary_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.BinaryExpression
      { Syntax.binary_left_operand = invalidate_expression x.binary_left_operand
      ; Syntax.binary_operator = invalidate_token x.binary_operator
      ; Syntax.binary_right_operand = invalidate_expression x.binary_right_operand
      }
    ; Syntax.value = v
    }
  and validate_instanceof_expression : instanceof_expression validator = function
  | { Syntax.syntax = Syntax.InstanceofExpression x; value = v } -> v,
    { instanceof_right_operand = validate_expression x.Syntax.instanceof_right_operand
    ; instanceof_operator = validate_token x.Syntax.instanceof_operator
    ; instanceof_left_operand = validate_expression x.Syntax.instanceof_left_operand
    }
  | s -> validation_fail SyntaxKind.InstanceofExpression s
  and invalidate_instanceof_expression : instanceof_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.InstanceofExpression
      { Syntax.instanceof_left_operand = invalidate_expression x.instanceof_left_operand
      ; Syntax.instanceof_operator = invalidate_token x.instanceof_operator
      ; Syntax.instanceof_right_operand = invalidate_expression x.instanceof_right_operand
      }
    ; Syntax.value = v
    }
  and validate_conditional_expression : conditional_expression validator = function
  | { Syntax.syntax = Syntax.ConditionalExpression x; value = v } -> v,
    { conditional_alternative = validate_expression x.Syntax.conditional_alternative
    ; conditional_colon = validate_token x.Syntax.conditional_colon
    ; conditional_consequence = validate_option_with (validate_expression) x.Syntax.conditional_consequence
    ; conditional_question = validate_token x.Syntax.conditional_question
    ; conditional_test = validate_expression x.Syntax.conditional_test
    }
  | s -> validation_fail SyntaxKind.ConditionalExpression s
  and invalidate_conditional_expression : conditional_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ConditionalExpression
      { Syntax.conditional_test = invalidate_expression x.conditional_test
      ; Syntax.conditional_question = invalidate_token x.conditional_question
      ; Syntax.conditional_consequence = invalidate_option_with (invalidate_expression) x.conditional_consequence
      ; Syntax.conditional_colon = invalidate_token x.conditional_colon
      ; Syntax.conditional_alternative = invalidate_expression x.conditional_alternative
      }
    ; Syntax.value = v
    }
  and validate_eval_expression : eval_expression validator = function
  | { Syntax.syntax = Syntax.EvalExpression x; value = v } -> v,
    { eval_right_paren = validate_token x.Syntax.eval_right_paren
    ; eval_argument = validate_expression x.Syntax.eval_argument
    ; eval_left_paren = validate_token x.Syntax.eval_left_paren
    ; eval_keyword = validate_token x.Syntax.eval_keyword
    }
  | s -> validation_fail SyntaxKind.EvalExpression s
  and invalidate_eval_expression : eval_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EvalExpression
      { Syntax.eval_keyword = invalidate_token x.eval_keyword
      ; Syntax.eval_left_paren = invalidate_token x.eval_left_paren
      ; Syntax.eval_argument = invalidate_expression x.eval_argument
      ; Syntax.eval_right_paren = invalidate_token x.eval_right_paren
      }
    ; Syntax.value = v
    }
  and validate_empty_expression : empty_expression validator = function
  | { Syntax.syntax = Syntax.EmptyExpression x; value = v } -> v,
    { empty_right_paren = validate_token x.Syntax.empty_right_paren
    ; empty_argument = validate_expression x.Syntax.empty_argument
    ; empty_left_paren = validate_token x.Syntax.empty_left_paren
    ; empty_keyword = validate_token x.Syntax.empty_keyword
    }
  | s -> validation_fail SyntaxKind.EmptyExpression s
  and invalidate_empty_expression : empty_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EmptyExpression
      { Syntax.empty_keyword = invalidate_token x.empty_keyword
      ; Syntax.empty_left_paren = invalidate_token x.empty_left_paren
      ; Syntax.empty_argument = invalidate_expression x.empty_argument
      ; Syntax.empty_right_paren = invalidate_token x.empty_right_paren
      }
    ; Syntax.value = v
    }
  and validate_define_expression : define_expression validator = function
  | { Syntax.syntax = Syntax.DefineExpression x; value = v } -> v,
    { define_right_paren = validate_token x.Syntax.define_right_paren
    ; define_argument_list = validate_list_with (validate_expression) x.Syntax.define_argument_list
    ; define_left_paren = validate_token x.Syntax.define_left_paren
    ; define_keyword = validate_token x.Syntax.define_keyword
    }
  | s -> validation_fail SyntaxKind.DefineExpression s
  and invalidate_define_expression : define_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DefineExpression
      { Syntax.define_keyword = invalidate_token x.define_keyword
      ; Syntax.define_left_paren = invalidate_token x.define_left_paren
      ; Syntax.define_argument_list = invalidate_list_with (invalidate_expression) x.define_argument_list
      ; Syntax.define_right_paren = invalidate_token x.define_right_paren
      }
    ; Syntax.value = v
    }
  and validate_isset_expression : isset_expression validator = function
  | { Syntax.syntax = Syntax.IssetExpression x; value = v } -> v,
    { isset_right_paren = validate_token x.Syntax.isset_right_paren
    ; isset_argument_list = validate_list_with (validate_expression) x.Syntax.isset_argument_list
    ; isset_left_paren = validate_token x.Syntax.isset_left_paren
    ; isset_keyword = validate_token x.Syntax.isset_keyword
    }
  | s -> validation_fail SyntaxKind.IssetExpression s
  and invalidate_isset_expression : isset_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.IssetExpression
      { Syntax.isset_keyword = invalidate_token x.isset_keyword
      ; Syntax.isset_left_paren = invalidate_token x.isset_left_paren
      ; Syntax.isset_argument_list = invalidate_list_with (invalidate_expression) x.isset_argument_list
      ; Syntax.isset_right_paren = invalidate_token x.isset_right_paren
      }
    ; Syntax.value = v
    }
  and validate_function_call_expression : function_call_expression validator = function
  | { Syntax.syntax = Syntax.FunctionCallExpression x; value = v } -> v,
    { function_call_right_paren = validate_token x.Syntax.function_call_right_paren
    ; function_call_argument_list = validate_list_with (validate_expression) x.Syntax.function_call_argument_list
    ; function_call_left_paren = validate_token x.Syntax.function_call_left_paren
    ; function_call_receiver = validate_expression x.Syntax.function_call_receiver
    }
  | s -> validation_fail SyntaxKind.FunctionCallExpression s
  and invalidate_function_call_expression : function_call_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FunctionCallExpression
      { Syntax.function_call_receiver = invalidate_expression x.function_call_receiver
      ; Syntax.function_call_left_paren = invalidate_token x.function_call_left_paren
      ; Syntax.function_call_argument_list = invalidate_list_with (invalidate_expression) x.function_call_argument_list
      ; Syntax.function_call_right_paren = invalidate_token x.function_call_right_paren
      }
    ; Syntax.value = v
    }
  and validate_function_call_with_type_arguments_expression : function_call_with_type_arguments_expression validator = function
  | { Syntax.syntax = Syntax.FunctionCallWithTypeArgumentsExpression x; value = v } -> v,
    { function_call_with_type_arguments_right_paren = validate_token x.Syntax.function_call_with_type_arguments_right_paren
    ; function_call_with_type_arguments_argument_list = validate_list_with (validate_expression) x.Syntax.function_call_with_type_arguments_argument_list
    ; function_call_with_type_arguments_left_paren = validate_token x.Syntax.function_call_with_type_arguments_left_paren
    ; function_call_with_type_arguments_type_args = validate_type_arguments x.Syntax.function_call_with_type_arguments_type_args
    ; function_call_with_type_arguments_receiver = validate_expression x.Syntax.function_call_with_type_arguments_receiver
    }
  | s -> validation_fail SyntaxKind.FunctionCallWithTypeArgumentsExpression s
  and invalidate_function_call_with_type_arguments_expression : function_call_with_type_arguments_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FunctionCallWithTypeArgumentsExpression
      { Syntax.function_call_with_type_arguments_receiver = invalidate_expression x.function_call_with_type_arguments_receiver
      ; Syntax.function_call_with_type_arguments_type_args = invalidate_type_arguments x.function_call_with_type_arguments_type_args
      ; Syntax.function_call_with_type_arguments_left_paren = invalidate_token x.function_call_with_type_arguments_left_paren
      ; Syntax.function_call_with_type_arguments_argument_list = invalidate_list_with (invalidate_expression) x.function_call_with_type_arguments_argument_list
      ; Syntax.function_call_with_type_arguments_right_paren = invalidate_token x.function_call_with_type_arguments_right_paren
      }
    ; Syntax.value = v
    }
  and validate_parenthesized_expression : parenthesized_expression validator = function
  | { Syntax.syntax = Syntax.ParenthesizedExpression x; value = v } -> v,
    { parenthesized_expression_right_paren = validate_token x.Syntax.parenthesized_expression_right_paren
    ; parenthesized_expression_expression = validate_expression x.Syntax.parenthesized_expression_expression
    ; parenthesized_expression_left_paren = validate_token x.Syntax.parenthesized_expression_left_paren
    }
  | s -> validation_fail SyntaxKind.ParenthesizedExpression s
  and invalidate_parenthesized_expression : parenthesized_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ParenthesizedExpression
      { Syntax.parenthesized_expression_left_paren = invalidate_token x.parenthesized_expression_left_paren
      ; Syntax.parenthesized_expression_expression = invalidate_expression x.parenthesized_expression_expression
      ; Syntax.parenthesized_expression_right_paren = invalidate_token x.parenthesized_expression_right_paren
      }
    ; Syntax.value = v
    }
  and validate_braced_expression : braced_expression validator = function
  | { Syntax.syntax = Syntax.BracedExpression x; value = v } -> v,
    { braced_expression_right_brace = validate_token x.Syntax.braced_expression_right_brace
    ; braced_expression_expression = validate_expression x.Syntax.braced_expression_expression
    ; braced_expression_left_brace = validate_token x.Syntax.braced_expression_left_brace
    }
  | s -> validation_fail SyntaxKind.BracedExpression s
  and invalidate_braced_expression : braced_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.BracedExpression
      { Syntax.braced_expression_left_brace = invalidate_token x.braced_expression_left_brace
      ; Syntax.braced_expression_expression = invalidate_expression x.braced_expression_expression
      ; Syntax.braced_expression_right_brace = invalidate_token x.braced_expression_right_brace
      }
    ; Syntax.value = v
    }
  and validate_embedded_braced_expression : embedded_braced_expression validator = function
  | { Syntax.syntax = Syntax.EmbeddedBracedExpression x; value = v } -> v,
    { embedded_braced_expression_right_brace = validate_token x.Syntax.embedded_braced_expression_right_brace
    ; embedded_braced_expression_expression = validate_expression x.Syntax.embedded_braced_expression_expression
    ; embedded_braced_expression_left_brace = validate_token x.Syntax.embedded_braced_expression_left_brace
    }
  | s -> validation_fail SyntaxKind.EmbeddedBracedExpression s
  and invalidate_embedded_braced_expression : embedded_braced_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EmbeddedBracedExpression
      { Syntax.embedded_braced_expression_left_brace = invalidate_token x.embedded_braced_expression_left_brace
      ; Syntax.embedded_braced_expression_expression = invalidate_expression x.embedded_braced_expression_expression
      ; Syntax.embedded_braced_expression_right_brace = invalidate_token x.embedded_braced_expression_right_brace
      }
    ; Syntax.value = v
    }
  and validate_list_expression : list_expression validator = function
  | { Syntax.syntax = Syntax.ListExpression x; value = v } -> v,
    { list_right_paren = validate_token x.Syntax.list_right_paren
    ; list_members = validate_list_with (validate_option_with (validate_expression)) x.Syntax.list_members
    ; list_left_paren = validate_token x.Syntax.list_left_paren
    ; list_keyword = validate_token x.Syntax.list_keyword
    }
  | s -> validation_fail SyntaxKind.ListExpression s
  and invalidate_list_expression : list_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ListExpression
      { Syntax.list_keyword = invalidate_token x.list_keyword
      ; Syntax.list_left_paren = invalidate_token x.list_left_paren
      ; Syntax.list_members = invalidate_list_with (invalidate_option_with (invalidate_expression)) x.list_members
      ; Syntax.list_right_paren = invalidate_token x.list_right_paren
      }
    ; Syntax.value = v
    }
  and validate_collection_literal_expression : collection_literal_expression validator = function
  | { Syntax.syntax = Syntax.CollectionLiteralExpression x; value = v } -> v,
    { collection_literal_right_brace = validate_token x.Syntax.collection_literal_right_brace
    ; collection_literal_initializers = validate_list_with (validate_constructor_expression) x.Syntax.collection_literal_initializers
    ; collection_literal_left_brace = validate_token x.Syntax.collection_literal_left_brace
    ; collection_literal_name = validate_token x.Syntax.collection_literal_name
    }
  | s -> validation_fail SyntaxKind.CollectionLiteralExpression s
  and invalidate_collection_literal_expression : collection_literal_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.CollectionLiteralExpression
      { Syntax.collection_literal_name = invalidate_token x.collection_literal_name
      ; Syntax.collection_literal_left_brace = invalidate_token x.collection_literal_left_brace
      ; Syntax.collection_literal_initializers = invalidate_list_with (invalidate_constructor_expression) x.collection_literal_initializers
      ; Syntax.collection_literal_right_brace = invalidate_token x.collection_literal_right_brace
      }
    ; Syntax.value = v
    }
  and validate_object_creation_expression : object_creation_expression validator = function
  | { Syntax.syntax = Syntax.ObjectCreationExpression x; value = v } -> v,
    { object_creation_right_paren = validate_option_with (validate_token) x.Syntax.object_creation_right_paren
    ; object_creation_argument_list = validate_list_with (validate_expression) x.Syntax.object_creation_argument_list
    ; object_creation_left_paren = validate_option_with (validate_token) x.Syntax.object_creation_left_paren
    ; object_creation_type = validate_todo_aggregate x.Syntax.object_creation_type
    ; object_creation_new_keyword = validate_token x.Syntax.object_creation_new_keyword
    }
  | s -> validation_fail SyntaxKind.ObjectCreationExpression s
  and invalidate_object_creation_expression : object_creation_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ObjectCreationExpression
      { Syntax.object_creation_new_keyword = invalidate_token x.object_creation_new_keyword
      ; Syntax.object_creation_type = invalidate_todo_aggregate x.object_creation_type
      ; Syntax.object_creation_left_paren = invalidate_option_with (invalidate_token) x.object_creation_left_paren
      ; Syntax.object_creation_argument_list = invalidate_list_with (invalidate_expression) x.object_creation_argument_list
      ; Syntax.object_creation_right_paren = invalidate_option_with (invalidate_token) x.object_creation_right_paren
      }
    ; Syntax.value = v
    }
  and validate_array_creation_expression : array_creation_expression validator = function
  | { Syntax.syntax = Syntax.ArrayCreationExpression x; value = v } -> v,
    { array_creation_right_bracket = validate_token x.Syntax.array_creation_right_bracket
    ; array_creation_members = validate_list_with (validate_constructor_expression) x.Syntax.array_creation_members
    ; array_creation_left_bracket = validate_token x.Syntax.array_creation_left_bracket
    }
  | s -> validation_fail SyntaxKind.ArrayCreationExpression s
  and invalidate_array_creation_expression : array_creation_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ArrayCreationExpression
      { Syntax.array_creation_left_bracket = invalidate_token x.array_creation_left_bracket
      ; Syntax.array_creation_members = invalidate_list_with (invalidate_constructor_expression) x.array_creation_members
      ; Syntax.array_creation_right_bracket = invalidate_token x.array_creation_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_array_intrinsic_expression : array_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.ArrayIntrinsicExpression x; value = v } -> v,
    { array_intrinsic_right_paren = validate_token x.Syntax.array_intrinsic_right_paren
    ; array_intrinsic_members = validate_list_with (validate_constructor_expression) x.Syntax.array_intrinsic_members
    ; array_intrinsic_left_paren = validate_token x.Syntax.array_intrinsic_left_paren
    ; array_intrinsic_keyword = validate_token x.Syntax.array_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.ArrayIntrinsicExpression s
  and invalidate_array_intrinsic_expression : array_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ArrayIntrinsicExpression
      { Syntax.array_intrinsic_keyword = invalidate_token x.array_intrinsic_keyword
      ; Syntax.array_intrinsic_left_paren = invalidate_token x.array_intrinsic_left_paren
      ; Syntax.array_intrinsic_members = invalidate_list_with (invalidate_constructor_expression) x.array_intrinsic_members
      ; Syntax.array_intrinsic_right_paren = invalidate_token x.array_intrinsic_right_paren
      }
    ; Syntax.value = v
    }
  and validate_darray_intrinsic_expression : darray_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.DarrayIntrinsicExpression x; value = v } -> v,
    { darray_intrinsic_right_bracket = validate_token x.Syntax.darray_intrinsic_right_bracket
    ; darray_intrinsic_members = validate_list_with (validate_element_initializer) x.Syntax.darray_intrinsic_members
    ; darray_intrinsic_left_bracket = validate_token x.Syntax.darray_intrinsic_left_bracket
    ; darray_intrinsic_keyword = validate_token x.Syntax.darray_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.DarrayIntrinsicExpression s
  and invalidate_darray_intrinsic_expression : darray_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DarrayIntrinsicExpression
      { Syntax.darray_intrinsic_keyword = invalidate_token x.darray_intrinsic_keyword
      ; Syntax.darray_intrinsic_left_bracket = invalidate_token x.darray_intrinsic_left_bracket
      ; Syntax.darray_intrinsic_members = invalidate_list_with (invalidate_element_initializer) x.darray_intrinsic_members
      ; Syntax.darray_intrinsic_right_bracket = invalidate_token x.darray_intrinsic_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_dictionary_intrinsic_expression : dictionary_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.DictionaryIntrinsicExpression x; value = v } -> v,
    { dictionary_intrinsic_right_bracket = validate_token x.Syntax.dictionary_intrinsic_right_bracket
    ; dictionary_intrinsic_members = validate_list_with (validate_element_initializer) x.Syntax.dictionary_intrinsic_members
    ; dictionary_intrinsic_left_bracket = validate_token x.Syntax.dictionary_intrinsic_left_bracket
    ; dictionary_intrinsic_keyword = validate_token x.Syntax.dictionary_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.DictionaryIntrinsicExpression s
  and invalidate_dictionary_intrinsic_expression : dictionary_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DictionaryIntrinsicExpression
      { Syntax.dictionary_intrinsic_keyword = invalidate_token x.dictionary_intrinsic_keyword
      ; Syntax.dictionary_intrinsic_left_bracket = invalidate_token x.dictionary_intrinsic_left_bracket
      ; Syntax.dictionary_intrinsic_members = invalidate_list_with (invalidate_element_initializer) x.dictionary_intrinsic_members
      ; Syntax.dictionary_intrinsic_right_bracket = invalidate_token x.dictionary_intrinsic_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_keyset_intrinsic_expression : keyset_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.KeysetIntrinsicExpression x; value = v } -> v,
    { keyset_intrinsic_right_bracket = validate_token x.Syntax.keyset_intrinsic_right_bracket
    ; keyset_intrinsic_members = validate_list_with (validate_expression) x.Syntax.keyset_intrinsic_members
    ; keyset_intrinsic_left_bracket = validate_token x.Syntax.keyset_intrinsic_left_bracket
    ; keyset_intrinsic_keyword = validate_token x.Syntax.keyset_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.KeysetIntrinsicExpression s
  and invalidate_keyset_intrinsic_expression : keyset_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.KeysetIntrinsicExpression
      { Syntax.keyset_intrinsic_keyword = invalidate_token x.keyset_intrinsic_keyword
      ; Syntax.keyset_intrinsic_left_bracket = invalidate_token x.keyset_intrinsic_left_bracket
      ; Syntax.keyset_intrinsic_members = invalidate_list_with (invalidate_expression) x.keyset_intrinsic_members
      ; Syntax.keyset_intrinsic_right_bracket = invalidate_token x.keyset_intrinsic_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_varray_intrinsic_expression : varray_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.VarrayIntrinsicExpression x; value = v } -> v,
    { varray_intrinsic_right_bracket = validate_token x.Syntax.varray_intrinsic_right_bracket
    ; varray_intrinsic_members = validate_list_with (validate_expression) x.Syntax.varray_intrinsic_members
    ; varray_intrinsic_left_bracket = validate_token x.Syntax.varray_intrinsic_left_bracket
    ; varray_intrinsic_keyword = validate_token x.Syntax.varray_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.VarrayIntrinsicExpression s
  and invalidate_varray_intrinsic_expression : varray_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VarrayIntrinsicExpression
      { Syntax.varray_intrinsic_keyword = invalidate_token x.varray_intrinsic_keyword
      ; Syntax.varray_intrinsic_left_bracket = invalidate_token x.varray_intrinsic_left_bracket
      ; Syntax.varray_intrinsic_members = invalidate_list_with (invalidate_expression) x.varray_intrinsic_members
      ; Syntax.varray_intrinsic_right_bracket = invalidate_token x.varray_intrinsic_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_vector_intrinsic_expression : vector_intrinsic_expression validator = function
  | { Syntax.syntax = Syntax.VectorIntrinsicExpression x; value = v } -> v,
    { vector_intrinsic_right_bracket = validate_token x.Syntax.vector_intrinsic_right_bracket
    ; vector_intrinsic_members = validate_list_with (validate_expression) x.Syntax.vector_intrinsic_members
    ; vector_intrinsic_left_bracket = validate_token x.Syntax.vector_intrinsic_left_bracket
    ; vector_intrinsic_keyword = validate_token x.Syntax.vector_intrinsic_keyword
    }
  | s -> validation_fail SyntaxKind.VectorIntrinsicExpression s
  and invalidate_vector_intrinsic_expression : vector_intrinsic_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VectorIntrinsicExpression
      { Syntax.vector_intrinsic_keyword = invalidate_token x.vector_intrinsic_keyword
      ; Syntax.vector_intrinsic_left_bracket = invalidate_token x.vector_intrinsic_left_bracket
      ; Syntax.vector_intrinsic_members = invalidate_list_with (invalidate_expression) x.vector_intrinsic_members
      ; Syntax.vector_intrinsic_right_bracket = invalidate_token x.vector_intrinsic_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_element_initializer : element_initializer validator = function
  | { Syntax.syntax = Syntax.ElementInitializer x; value = v } -> v,
    { element_value = validate_expression x.Syntax.element_value
    ; element_arrow = validate_token x.Syntax.element_arrow
    ; element_key = validate_expression x.Syntax.element_key
    }
  | s -> validation_fail SyntaxKind.ElementInitializer s
  and invalidate_element_initializer : element_initializer invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ElementInitializer
      { Syntax.element_key = invalidate_expression x.element_key
      ; Syntax.element_arrow = invalidate_token x.element_arrow
      ; Syntax.element_value = invalidate_expression x.element_value
      }
    ; Syntax.value = v
    }
  and validate_subscript_expression : subscript_expression validator = function
  | { Syntax.syntax = Syntax.SubscriptExpression x; value = v } -> v,
    { subscript_right_bracket = validate_token x.Syntax.subscript_right_bracket
    ; subscript_index = validate_option_with (validate_expression) x.Syntax.subscript_index
    ; subscript_left_bracket = validate_token x.Syntax.subscript_left_bracket
    ; subscript_receiver = validate_expression x.Syntax.subscript_receiver
    }
  | s -> validation_fail SyntaxKind.SubscriptExpression s
  and invalidate_subscript_expression : subscript_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SubscriptExpression
      { Syntax.subscript_receiver = invalidate_expression x.subscript_receiver
      ; Syntax.subscript_left_bracket = invalidate_token x.subscript_left_bracket
      ; Syntax.subscript_index = invalidate_option_with (invalidate_expression) x.subscript_index
      ; Syntax.subscript_right_bracket = invalidate_token x.subscript_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_embedded_subscript_expression : embedded_subscript_expression validator = function
  | { Syntax.syntax = Syntax.EmbeddedSubscriptExpression x; value = v } -> v,
    { embedded_subscript_right_bracket = validate_token x.Syntax.embedded_subscript_right_bracket
    ; embedded_subscript_index = validate_expression x.Syntax.embedded_subscript_index
    ; embedded_subscript_left_bracket = validate_token x.Syntax.embedded_subscript_left_bracket
    ; embedded_subscript_receiver = validate_variable_expression x.Syntax.embedded_subscript_receiver
    }
  | s -> validation_fail SyntaxKind.EmbeddedSubscriptExpression s
  and invalidate_embedded_subscript_expression : embedded_subscript_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.EmbeddedSubscriptExpression
      { Syntax.embedded_subscript_receiver = invalidate_variable_expression x.embedded_subscript_receiver
      ; Syntax.embedded_subscript_left_bracket = invalidate_token x.embedded_subscript_left_bracket
      ; Syntax.embedded_subscript_index = invalidate_expression x.embedded_subscript_index
      ; Syntax.embedded_subscript_right_bracket = invalidate_token x.embedded_subscript_right_bracket
      }
    ; Syntax.value = v
    }
  and validate_awaitable_creation_expression : awaitable_creation_expression validator = function
  | { Syntax.syntax = Syntax.AwaitableCreationExpression x; value = v } -> v,
    { awaitable_compound_statement = validate_compound_statement x.Syntax.awaitable_compound_statement
    ; awaitable_coroutine = validate_option_with (validate_token) x.Syntax.awaitable_coroutine
    ; awaitable_async = validate_token x.Syntax.awaitable_async
    }
  | s -> validation_fail SyntaxKind.AwaitableCreationExpression s
  and invalidate_awaitable_creation_expression : awaitable_creation_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.AwaitableCreationExpression
      { Syntax.awaitable_async = invalidate_token x.awaitable_async
      ; Syntax.awaitable_coroutine = invalidate_option_with (invalidate_token) x.awaitable_coroutine
      ; Syntax.awaitable_compound_statement = invalidate_compound_statement x.awaitable_compound_statement
      }
    ; Syntax.value = v
    }
  and validate_xhp_children_declaration : xhp_children_declaration validator = function
  | { Syntax.syntax = Syntax.XHPChildrenDeclaration x; value = v } -> v,
    { xhp_children_semicolon = validate_token x.Syntax.xhp_children_semicolon
    ; xhp_children_expression = validate_expression x.Syntax.xhp_children_expression
    ; xhp_children_keyword = validate_token x.Syntax.xhp_children_keyword
    }
  | s -> validation_fail SyntaxKind.XHPChildrenDeclaration s
  and invalidate_xhp_children_declaration : xhp_children_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPChildrenDeclaration
      { Syntax.xhp_children_keyword = invalidate_token x.xhp_children_keyword
      ; Syntax.xhp_children_expression = invalidate_expression x.xhp_children_expression
      ; Syntax.xhp_children_semicolon = invalidate_token x.xhp_children_semicolon
      }
    ; Syntax.value = v
    }
  and validate_xhp_children_parenthesized_list : xhp_children_parenthesized_list validator = function
  | { Syntax.syntax = Syntax.XHPChildrenParenthesizedList x; value = v } -> v,
    { xhp_children_list_right_paren = validate_token x.Syntax.xhp_children_list_right_paren
    ; xhp_children_list_xhp_children = validate_list_with (validate_expression) x.Syntax.xhp_children_list_xhp_children
    ; xhp_children_list_left_paren = validate_token x.Syntax.xhp_children_list_left_paren
    }
  | s -> validation_fail SyntaxKind.XHPChildrenParenthesizedList s
  and invalidate_xhp_children_parenthesized_list : xhp_children_parenthesized_list invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPChildrenParenthesizedList
      { Syntax.xhp_children_list_left_paren = invalidate_token x.xhp_children_list_left_paren
      ; Syntax.xhp_children_list_xhp_children = invalidate_list_with (invalidate_expression) x.xhp_children_list_xhp_children
      ; Syntax.xhp_children_list_right_paren = invalidate_token x.xhp_children_list_right_paren
      }
    ; Syntax.value = v
    }
  and validate_xhp_category_declaration : xhp_category_declaration validator = function
  | { Syntax.syntax = Syntax.XHPCategoryDeclaration x; value = v } -> v,
    { xhp_category_semicolon = validate_token x.Syntax.xhp_category_semicolon
    ; xhp_category_categories = validate_list_with (validate_token) x.Syntax.xhp_category_categories
    ; xhp_category_keyword = validate_token x.Syntax.xhp_category_keyword
    }
  | s -> validation_fail SyntaxKind.XHPCategoryDeclaration s
  and invalidate_xhp_category_declaration : xhp_category_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPCategoryDeclaration
      { Syntax.xhp_category_keyword = invalidate_token x.xhp_category_keyword
      ; Syntax.xhp_category_categories = invalidate_list_with (invalidate_token) x.xhp_category_categories
      ; Syntax.xhp_category_semicolon = invalidate_token x.xhp_category_semicolon
      }
    ; Syntax.value = v
    }
  and validate_xhp_enum_type : xhp_enum_type validator = function
  | { Syntax.syntax = Syntax.XHPEnumType x; value = v } -> v,
    { xhp_enum_right_brace = validate_token x.Syntax.xhp_enum_right_brace
    ; xhp_enum_values = validate_list_with (validate_literal_expression) x.Syntax.xhp_enum_values
    ; xhp_enum_left_brace = validate_token x.Syntax.xhp_enum_left_brace
    ; xhp_enum_keyword = validate_token x.Syntax.xhp_enum_keyword
    }
  | s -> validation_fail SyntaxKind.XHPEnumType s
  and invalidate_xhp_enum_type : xhp_enum_type invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPEnumType
      { Syntax.xhp_enum_keyword = invalidate_token x.xhp_enum_keyword
      ; Syntax.xhp_enum_left_brace = invalidate_token x.xhp_enum_left_brace
      ; Syntax.xhp_enum_values = invalidate_list_with (invalidate_literal_expression) x.xhp_enum_values
      ; Syntax.xhp_enum_right_brace = invalidate_token x.xhp_enum_right_brace
      }
    ; Syntax.value = v
    }
  and validate_xhp_required : xhp_required validator = function
  | { Syntax.syntax = Syntax.XHPRequired x; value = v } -> v,
    { xhp_required_keyword = validate_token x.Syntax.xhp_required_keyword
    ; xhp_required_at = validate_token x.Syntax.xhp_required_at
    }
  | s -> validation_fail SyntaxKind.XHPRequired s
  and invalidate_xhp_required : xhp_required invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPRequired
      { Syntax.xhp_required_at = invalidate_token x.xhp_required_at
      ; Syntax.xhp_required_keyword = invalidate_token x.xhp_required_keyword
      }
    ; Syntax.value = v
    }
  and validate_xhp_class_attribute_declaration : xhp_class_attribute_declaration validator = function
  | { Syntax.syntax = Syntax.XHPClassAttributeDeclaration x; value = v } -> v,
    { xhp_attribute_semicolon = validate_token x.Syntax.xhp_attribute_semicolon
    ; xhp_attribute_attributes = validate_list_with (validate_todo_aggregate) x.Syntax.xhp_attribute_attributes
    ; xhp_attribute_keyword = validate_token x.Syntax.xhp_attribute_keyword
    }
  | s -> validation_fail SyntaxKind.XHPClassAttributeDeclaration s
  and invalidate_xhp_class_attribute_declaration : xhp_class_attribute_declaration invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPClassAttributeDeclaration
      { Syntax.xhp_attribute_keyword = invalidate_token x.xhp_attribute_keyword
      ; Syntax.xhp_attribute_attributes = invalidate_list_with (invalidate_todo_aggregate) x.xhp_attribute_attributes
      ; Syntax.xhp_attribute_semicolon = invalidate_token x.xhp_attribute_semicolon
      }
    ; Syntax.value = v
    }
  and validate_xhp_class_attribute : xhp_class_attribute validator = function
  | { Syntax.syntax = Syntax.XHPClassAttribute x; value = v } -> v,
    { xhp_attribute_decl_required = validate_option_with (validate_xhp_required) x.Syntax.xhp_attribute_decl_required
    ; xhp_attribute_decl_initializer = validate_option_with (validate_simple_initializer) x.Syntax.xhp_attribute_decl_initializer
    ; xhp_attribute_decl_name = validate_token x.Syntax.xhp_attribute_decl_name
    ; xhp_attribute_decl_type = validate_specifier x.Syntax.xhp_attribute_decl_type
    }
  | s -> validation_fail SyntaxKind.XHPClassAttribute s
  and invalidate_xhp_class_attribute : xhp_class_attribute invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPClassAttribute
      { Syntax.xhp_attribute_decl_type = invalidate_specifier x.xhp_attribute_decl_type
      ; Syntax.xhp_attribute_decl_name = invalidate_token x.xhp_attribute_decl_name
      ; Syntax.xhp_attribute_decl_initializer = invalidate_option_with (invalidate_simple_initializer) x.xhp_attribute_decl_initializer
      ; Syntax.xhp_attribute_decl_required = invalidate_option_with (invalidate_xhp_required) x.xhp_attribute_decl_required
      }
    ; Syntax.value = v
    }
  and validate_xhp_simple_class_attribute : xhp_simple_class_attribute validator = function
  | { Syntax.syntax = Syntax.XHPSimpleClassAttribute x; value = v } -> v,
    { xhp_simple_class_attribute_type = validate_simple_type_specifier x.Syntax.xhp_simple_class_attribute_type
    }
  | s -> validation_fail SyntaxKind.XHPSimpleClassAttribute s
  and invalidate_xhp_simple_class_attribute : xhp_simple_class_attribute invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPSimpleClassAttribute
      { Syntax.xhp_simple_class_attribute_type = invalidate_simple_type_specifier x.xhp_simple_class_attribute_type
      }
    ; Syntax.value = v
    }
  and validate_xhp_attribute : xhp_attribute validator = function
  | { Syntax.syntax = Syntax.XHPAttribute x; value = v } -> v,
    { xhp_attribute_expression = validate_expression x.Syntax.xhp_attribute_expression
    ; xhp_attribute_equal = validate_token x.Syntax.xhp_attribute_equal
    ; xhp_attribute_name = validate_token x.Syntax.xhp_attribute_name
    }
  | s -> validation_fail SyntaxKind.XHPAttribute s
  and invalidate_xhp_attribute : xhp_attribute invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPAttribute
      { Syntax.xhp_attribute_name = invalidate_token x.xhp_attribute_name
      ; Syntax.xhp_attribute_equal = invalidate_token x.xhp_attribute_equal
      ; Syntax.xhp_attribute_expression = invalidate_expression x.xhp_attribute_expression
      }
    ; Syntax.value = v
    }
  and validate_xhp_open : xhp_open validator = function
  | { Syntax.syntax = Syntax.XHPOpen x; value = v } -> v,
    { xhp_open_right_angle = validate_token x.Syntax.xhp_open_right_angle
    ; xhp_open_attributes = validate_list_with (validate_xhp_attribute) x.Syntax.xhp_open_attributes
    ; xhp_open_name = validate_token x.Syntax.xhp_open_name
    ; xhp_open_left_angle = validate_token x.Syntax.xhp_open_left_angle
    }
  | s -> validation_fail SyntaxKind.XHPOpen s
  and invalidate_xhp_open : xhp_open invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPOpen
      { Syntax.xhp_open_left_angle = invalidate_token x.xhp_open_left_angle
      ; Syntax.xhp_open_name = invalidate_token x.xhp_open_name
      ; Syntax.xhp_open_attributes = invalidate_list_with (invalidate_xhp_attribute) x.xhp_open_attributes
      ; Syntax.xhp_open_right_angle = invalidate_token x.xhp_open_right_angle
      }
    ; Syntax.value = v
    }
  and validate_xhp_expression : xhp_expression validator = function
  | { Syntax.syntax = Syntax.XHPExpression x; value = v } -> v,
    { xhp_close = validate_option_with (validate_xhp_close) x.Syntax.xhp_close
    ; xhp_body = validate_list_with (validate_expression) x.Syntax.xhp_body
    ; xhp_open = validate_xhp_open x.Syntax.xhp_open
    }
  | s -> validation_fail SyntaxKind.XHPExpression s
  and invalidate_xhp_expression : xhp_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPExpression
      { Syntax.xhp_open = invalidate_xhp_open x.xhp_open
      ; Syntax.xhp_body = invalidate_list_with (invalidate_expression) x.xhp_body
      ; Syntax.xhp_close = invalidate_option_with (invalidate_xhp_close) x.xhp_close
      }
    ; Syntax.value = v
    }
  and validate_xhp_close : xhp_close validator = function
  | { Syntax.syntax = Syntax.XHPClose x; value = v } -> v,
    { xhp_close_right_angle = validate_token x.Syntax.xhp_close_right_angle
    ; xhp_close_name = validate_token x.Syntax.xhp_close_name
    ; xhp_close_left_angle = validate_token x.Syntax.xhp_close_left_angle
    }
  | s -> validation_fail SyntaxKind.XHPClose s
  and invalidate_xhp_close : xhp_close invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.XHPClose
      { Syntax.xhp_close_left_angle = invalidate_token x.xhp_close_left_angle
      ; Syntax.xhp_close_name = invalidate_token x.xhp_close_name
      ; Syntax.xhp_close_right_angle = invalidate_token x.xhp_close_right_angle
      }
    ; Syntax.value = v
    }
  and validate_type_constant : type_constant validator = function
  | { Syntax.syntax = Syntax.TypeConstant x; value = v } -> v,
    { type_constant_right_type = validate_token x.Syntax.type_constant_right_type
    ; type_constant_separator = validate_token x.Syntax.type_constant_separator
    ; type_constant_left_type = validate_specifier x.Syntax.type_constant_left_type
    }
  | s -> validation_fail SyntaxKind.TypeConstant s
  and invalidate_type_constant : type_constant invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeConstant
      { Syntax.type_constant_left_type = invalidate_specifier x.type_constant_left_type
      ; Syntax.type_constant_separator = invalidate_token x.type_constant_separator
      ; Syntax.type_constant_right_type = invalidate_token x.type_constant_right_type
      }
    ; Syntax.value = v
    }
  and validate_vector_type_specifier : vector_type_specifier validator = function
  | { Syntax.syntax = Syntax.VectorTypeSpecifier x; value = v } -> v,
    { vector_type_right_angle = validate_token x.Syntax.vector_type_right_angle
    ; vector_type_trailing_comma = validate_option_with (validate_token) x.Syntax.vector_type_trailing_comma
    ; vector_type_type = validate_specifier x.Syntax.vector_type_type
    ; vector_type_left_angle = validate_token x.Syntax.vector_type_left_angle
    ; vector_type_keyword = validate_token x.Syntax.vector_type_keyword
    }
  | s -> validation_fail SyntaxKind.VectorTypeSpecifier s
  and invalidate_vector_type_specifier : vector_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VectorTypeSpecifier
      { Syntax.vector_type_keyword = invalidate_token x.vector_type_keyword
      ; Syntax.vector_type_left_angle = invalidate_token x.vector_type_left_angle
      ; Syntax.vector_type_type = invalidate_specifier x.vector_type_type
      ; Syntax.vector_type_trailing_comma = invalidate_option_with (invalidate_token) x.vector_type_trailing_comma
      ; Syntax.vector_type_right_angle = invalidate_token x.vector_type_right_angle
      }
    ; Syntax.value = v
    }
  and validate_keyset_type_specifier : keyset_type_specifier validator = function
  | { Syntax.syntax = Syntax.KeysetTypeSpecifier x; value = v } -> v,
    { keyset_type_right_angle = validate_token x.Syntax.keyset_type_right_angle
    ; keyset_type_trailing_comma = validate_option_with (validate_token) x.Syntax.keyset_type_trailing_comma
    ; keyset_type_type = validate_specifier x.Syntax.keyset_type_type
    ; keyset_type_left_angle = validate_token x.Syntax.keyset_type_left_angle
    ; keyset_type_keyword = validate_token x.Syntax.keyset_type_keyword
    }
  | s -> validation_fail SyntaxKind.KeysetTypeSpecifier s
  and invalidate_keyset_type_specifier : keyset_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.KeysetTypeSpecifier
      { Syntax.keyset_type_keyword = invalidate_token x.keyset_type_keyword
      ; Syntax.keyset_type_left_angle = invalidate_token x.keyset_type_left_angle
      ; Syntax.keyset_type_type = invalidate_specifier x.keyset_type_type
      ; Syntax.keyset_type_trailing_comma = invalidate_option_with (invalidate_token) x.keyset_type_trailing_comma
      ; Syntax.keyset_type_right_angle = invalidate_token x.keyset_type_right_angle
      }
    ; Syntax.value = v
    }
  and validate_tuple_type_explicit_specifier : tuple_type_explicit_specifier validator = function
  | { Syntax.syntax = Syntax.TupleTypeExplicitSpecifier x; value = v } -> v,
    { tuple_type_right_angle = validate_token x.Syntax.tuple_type_right_angle
    ; tuple_type_types = validate_simple_type_specifier x.Syntax.tuple_type_types
    ; tuple_type_left_angle = validate_token x.Syntax.tuple_type_left_angle
    ; tuple_type_keyword = validate_token x.Syntax.tuple_type_keyword
    }
  | s -> validation_fail SyntaxKind.TupleTypeExplicitSpecifier s
  and invalidate_tuple_type_explicit_specifier : tuple_type_explicit_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TupleTypeExplicitSpecifier
      { Syntax.tuple_type_keyword = invalidate_token x.tuple_type_keyword
      ; Syntax.tuple_type_left_angle = invalidate_token x.tuple_type_left_angle
      ; Syntax.tuple_type_types = invalidate_simple_type_specifier x.tuple_type_types
      ; Syntax.tuple_type_right_angle = invalidate_token x.tuple_type_right_angle
      }
    ; Syntax.value = v
    }
  and validate_varray_type_specifier : varray_type_specifier validator = function
  | { Syntax.syntax = Syntax.VarrayTypeSpecifier x; value = v } -> v,
    { varray_right_angle = validate_token x.Syntax.varray_right_angle
    ; varray_trailing_comma = validate_option_with (validate_token) x.Syntax.varray_trailing_comma
    ; varray_type = validate_simple_type_specifier x.Syntax.varray_type
    ; varray_left_angle = validate_token x.Syntax.varray_left_angle
    ; varray_keyword = validate_token x.Syntax.varray_keyword
    }
  | s -> validation_fail SyntaxKind.VarrayTypeSpecifier s
  and invalidate_varray_type_specifier : varray_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VarrayTypeSpecifier
      { Syntax.varray_keyword = invalidate_token x.varray_keyword
      ; Syntax.varray_left_angle = invalidate_token x.varray_left_angle
      ; Syntax.varray_type = invalidate_simple_type_specifier x.varray_type
      ; Syntax.varray_trailing_comma = invalidate_option_with (invalidate_token) x.varray_trailing_comma
      ; Syntax.varray_right_angle = invalidate_token x.varray_right_angle
      }
    ; Syntax.value = v
    }
  and validate_vector_array_type_specifier : vector_array_type_specifier validator = function
  | { Syntax.syntax = Syntax.VectorArrayTypeSpecifier x; value = v } -> v,
    { vector_array_right_angle = validate_token x.Syntax.vector_array_right_angle
    ; vector_array_type = validate_specifier x.Syntax.vector_array_type
    ; vector_array_left_angle = validate_token x.Syntax.vector_array_left_angle
    ; vector_array_keyword = validate_token x.Syntax.vector_array_keyword
    }
  | s -> validation_fail SyntaxKind.VectorArrayTypeSpecifier s
  and invalidate_vector_array_type_specifier : vector_array_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.VectorArrayTypeSpecifier
      { Syntax.vector_array_keyword = invalidate_token x.vector_array_keyword
      ; Syntax.vector_array_left_angle = invalidate_token x.vector_array_left_angle
      ; Syntax.vector_array_type = invalidate_specifier x.vector_array_type
      ; Syntax.vector_array_right_angle = invalidate_token x.vector_array_right_angle
      }
    ; Syntax.value = v
    }
  and validate_type_parameter : type_parameter validator = function
  | { Syntax.syntax = Syntax.TypeParameter x; value = v } -> v,
    { type_constraints = validate_list_with (validate_type_constraint) x.Syntax.type_constraints
    ; type_name = validate_token x.Syntax.type_name
    ; type_variance = validate_option_with (validate_token) x.Syntax.type_variance
    }
  | s -> validation_fail SyntaxKind.TypeParameter s
  and invalidate_type_parameter : type_parameter invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeParameter
      { Syntax.type_variance = invalidate_option_with (invalidate_token) x.type_variance
      ; Syntax.type_name = invalidate_token x.type_name
      ; Syntax.type_constraints = invalidate_list_with (invalidate_type_constraint) x.type_constraints
      }
    ; Syntax.value = v
    }
  and validate_type_constraint : type_constraint validator = function
  | { Syntax.syntax = Syntax.TypeConstraint x; value = v } -> v,
    { constraint_type = validate_specifier x.Syntax.constraint_type
    ; constraint_keyword = validate_token x.Syntax.constraint_keyword
    }
  | s -> validation_fail SyntaxKind.TypeConstraint s
  and invalidate_type_constraint : type_constraint invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeConstraint
      { Syntax.constraint_keyword = invalidate_token x.constraint_keyword
      ; Syntax.constraint_type = invalidate_specifier x.constraint_type
      }
    ; Syntax.value = v
    }
  and validate_darray_type_specifier : darray_type_specifier validator = function
  | { Syntax.syntax = Syntax.DarrayTypeSpecifier x; value = v } -> v,
    { darray_right_angle = validate_token x.Syntax.darray_right_angle
    ; darray_trailing_comma = validate_option_with (validate_token) x.Syntax.darray_trailing_comma
    ; darray_value = validate_simple_type_specifier x.Syntax.darray_value
    ; darray_comma = validate_token x.Syntax.darray_comma
    ; darray_key = validate_simple_type_specifier x.Syntax.darray_key
    ; darray_left_angle = validate_token x.Syntax.darray_left_angle
    ; darray_keyword = validate_token x.Syntax.darray_keyword
    }
  | s -> validation_fail SyntaxKind.DarrayTypeSpecifier s
  and invalidate_darray_type_specifier : darray_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DarrayTypeSpecifier
      { Syntax.darray_keyword = invalidate_token x.darray_keyword
      ; Syntax.darray_left_angle = invalidate_token x.darray_left_angle
      ; Syntax.darray_key = invalidate_simple_type_specifier x.darray_key
      ; Syntax.darray_comma = invalidate_token x.darray_comma
      ; Syntax.darray_value = invalidate_simple_type_specifier x.darray_value
      ; Syntax.darray_trailing_comma = invalidate_option_with (invalidate_token) x.darray_trailing_comma
      ; Syntax.darray_right_angle = invalidate_token x.darray_right_angle
      }
    ; Syntax.value = v
    }
  and validate_map_array_type_specifier : map_array_type_specifier validator = function
  | { Syntax.syntax = Syntax.MapArrayTypeSpecifier x; value = v } -> v,
    { map_array_right_angle = validate_token x.Syntax.map_array_right_angle
    ; map_array_value = validate_specifier x.Syntax.map_array_value
    ; map_array_comma = validate_token x.Syntax.map_array_comma
    ; map_array_key = validate_specifier x.Syntax.map_array_key
    ; map_array_left_angle = validate_token x.Syntax.map_array_left_angle
    ; map_array_keyword = validate_token x.Syntax.map_array_keyword
    }
  | s -> validation_fail SyntaxKind.MapArrayTypeSpecifier s
  and invalidate_map_array_type_specifier : map_array_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.MapArrayTypeSpecifier
      { Syntax.map_array_keyword = invalidate_token x.map_array_keyword
      ; Syntax.map_array_left_angle = invalidate_token x.map_array_left_angle
      ; Syntax.map_array_key = invalidate_specifier x.map_array_key
      ; Syntax.map_array_comma = invalidate_token x.map_array_comma
      ; Syntax.map_array_value = invalidate_specifier x.map_array_value
      ; Syntax.map_array_right_angle = invalidate_token x.map_array_right_angle
      }
    ; Syntax.value = v
    }
  and validate_dictionary_type_specifier : dictionary_type_specifier validator = function
  | { Syntax.syntax = Syntax.DictionaryTypeSpecifier x; value = v } -> v,
    { dictionary_type_right_angle = validate_token x.Syntax.dictionary_type_right_angle
    ; dictionary_type_members = validate_list_with (validate_specifier) x.Syntax.dictionary_type_members
    ; dictionary_type_left_angle = validate_token x.Syntax.dictionary_type_left_angle
    ; dictionary_type_keyword = validate_token x.Syntax.dictionary_type_keyword
    }
  | s -> validation_fail SyntaxKind.DictionaryTypeSpecifier s
  and invalidate_dictionary_type_specifier : dictionary_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.DictionaryTypeSpecifier
      { Syntax.dictionary_type_keyword = invalidate_token x.dictionary_type_keyword
      ; Syntax.dictionary_type_left_angle = invalidate_token x.dictionary_type_left_angle
      ; Syntax.dictionary_type_members = invalidate_list_with (invalidate_specifier) x.dictionary_type_members
      ; Syntax.dictionary_type_right_angle = invalidate_token x.dictionary_type_right_angle
      }
    ; Syntax.value = v
    }
  and validate_closure_type_specifier : closure_type_specifier validator = function
  | { Syntax.syntax = Syntax.ClosureTypeSpecifier x; value = v } -> v,
    { closure_outer_right_paren = validate_token x.Syntax.closure_outer_right_paren
    ; closure_return_type = validate_specifier x.Syntax.closure_return_type
    ; closure_colon = validate_token x.Syntax.closure_colon
    ; closure_inner_right_paren = validate_token x.Syntax.closure_inner_right_paren
    ; closure_parameter_types = validate_list_with (validate_specifier) x.Syntax.closure_parameter_types
    ; closure_inner_left_paren = validate_token x.Syntax.closure_inner_left_paren
    ; closure_function_keyword = validate_token x.Syntax.closure_function_keyword
    ; closure_coroutine = validate_option_with (validate_token) x.Syntax.closure_coroutine
    ; closure_outer_left_paren = validate_token x.Syntax.closure_outer_left_paren
    }
  | s -> validation_fail SyntaxKind.ClosureTypeSpecifier s
  and invalidate_closure_type_specifier : closure_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ClosureTypeSpecifier
      { Syntax.closure_outer_left_paren = invalidate_token x.closure_outer_left_paren
      ; Syntax.closure_coroutine = invalidate_option_with (invalidate_token) x.closure_coroutine
      ; Syntax.closure_function_keyword = invalidate_token x.closure_function_keyword
      ; Syntax.closure_inner_left_paren = invalidate_token x.closure_inner_left_paren
      ; Syntax.closure_parameter_types = invalidate_list_with (invalidate_specifier) x.closure_parameter_types
      ; Syntax.closure_inner_right_paren = invalidate_token x.closure_inner_right_paren
      ; Syntax.closure_colon = invalidate_token x.closure_colon
      ; Syntax.closure_return_type = invalidate_specifier x.closure_return_type
      ; Syntax.closure_outer_right_paren = invalidate_token x.closure_outer_right_paren
      }
    ; Syntax.value = v
    }
  and validate_classname_type_specifier : classname_type_specifier validator = function
  | { Syntax.syntax = Syntax.ClassnameTypeSpecifier x; value = v } -> v,
    { classname_right_angle = validate_token x.Syntax.classname_right_angle
    ; classname_trailing_comma = validate_option_with (validate_token) x.Syntax.classname_trailing_comma
    ; classname_type = validate_specifier x.Syntax.classname_type
    ; classname_left_angle = validate_token x.Syntax.classname_left_angle
    ; classname_keyword = validate_token x.Syntax.classname_keyword
    }
  | s -> validation_fail SyntaxKind.ClassnameTypeSpecifier s
  and invalidate_classname_type_specifier : classname_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ClassnameTypeSpecifier
      { Syntax.classname_keyword = invalidate_token x.classname_keyword
      ; Syntax.classname_left_angle = invalidate_token x.classname_left_angle
      ; Syntax.classname_type = invalidate_specifier x.classname_type
      ; Syntax.classname_trailing_comma = invalidate_option_with (invalidate_token) x.classname_trailing_comma
      ; Syntax.classname_right_angle = invalidate_token x.classname_right_angle
      }
    ; Syntax.value = v
    }
  and validate_field_specifier : field_specifier validator = function
  | { Syntax.syntax = Syntax.FieldSpecifier x; value = v } -> v,
    { field_type = validate_specifier x.Syntax.field_type
    ; field_arrow = validate_token x.Syntax.field_arrow
    ; field_name = validate_expression x.Syntax.field_name
    ; field_question = validate_option_with (validate_token) x.Syntax.field_question
    }
  | s -> validation_fail SyntaxKind.FieldSpecifier s
  and invalidate_field_specifier : field_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FieldSpecifier
      { Syntax.field_question = invalidate_option_with (invalidate_token) x.field_question
      ; Syntax.field_name = invalidate_expression x.field_name
      ; Syntax.field_arrow = invalidate_token x.field_arrow
      ; Syntax.field_type = invalidate_specifier x.field_type
      }
    ; Syntax.value = v
    }
  and validate_field_initializer : field_initializer validator = function
  | { Syntax.syntax = Syntax.FieldInitializer x; value = v } -> v,
    { field_initializer_value = validate_expression x.Syntax.field_initializer_value
    ; field_initializer_arrow = validate_token x.Syntax.field_initializer_arrow
    ; field_initializer_name = validate_expression x.Syntax.field_initializer_name
    }
  | s -> validation_fail SyntaxKind.FieldInitializer s
  and invalidate_field_initializer : field_initializer invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.FieldInitializer
      { Syntax.field_initializer_name = invalidate_expression x.field_initializer_name
      ; Syntax.field_initializer_arrow = invalidate_token x.field_initializer_arrow
      ; Syntax.field_initializer_value = invalidate_expression x.field_initializer_value
      }
    ; Syntax.value = v
    }
  and validate_shape_type_specifier : shape_type_specifier validator = function
  | { Syntax.syntax = Syntax.ShapeTypeSpecifier x; value = v } -> v,
    { shape_type_right_paren = validate_token x.Syntax.shape_type_right_paren
    ; shape_type_ellipsis = validate_option_with (validate_token) x.Syntax.shape_type_ellipsis
    ; shape_type_fields = validate_list_with (validate_field_specifier) x.Syntax.shape_type_fields
    ; shape_type_left_paren = validate_token x.Syntax.shape_type_left_paren
    ; shape_type_keyword = validate_token x.Syntax.shape_type_keyword
    }
  | s -> validation_fail SyntaxKind.ShapeTypeSpecifier s
  and invalidate_shape_type_specifier : shape_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ShapeTypeSpecifier
      { Syntax.shape_type_keyword = invalidate_token x.shape_type_keyword
      ; Syntax.shape_type_left_paren = invalidate_token x.shape_type_left_paren
      ; Syntax.shape_type_fields = invalidate_list_with (invalidate_field_specifier) x.shape_type_fields
      ; Syntax.shape_type_ellipsis = invalidate_option_with (invalidate_token) x.shape_type_ellipsis
      ; Syntax.shape_type_right_paren = invalidate_token x.shape_type_right_paren
      }
    ; Syntax.value = v
    }
  and validate_shape_expression : shape_expression validator = function
  | { Syntax.syntax = Syntax.ShapeExpression x; value = v } -> v,
    { shape_expression_right_paren = validate_token x.Syntax.shape_expression_right_paren
    ; shape_expression_fields = validate_list_with (validate_field_initializer) x.Syntax.shape_expression_fields
    ; shape_expression_left_paren = validate_token x.Syntax.shape_expression_left_paren
    ; shape_expression_keyword = validate_token x.Syntax.shape_expression_keyword
    }
  | s -> validation_fail SyntaxKind.ShapeExpression s
  and invalidate_shape_expression : shape_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.ShapeExpression
      { Syntax.shape_expression_keyword = invalidate_token x.shape_expression_keyword
      ; Syntax.shape_expression_left_paren = invalidate_token x.shape_expression_left_paren
      ; Syntax.shape_expression_fields = invalidate_list_with (invalidate_field_initializer) x.shape_expression_fields
      ; Syntax.shape_expression_right_paren = invalidate_token x.shape_expression_right_paren
      }
    ; Syntax.value = v
    }
  and validate_tuple_expression : tuple_expression validator = function
  | { Syntax.syntax = Syntax.TupleExpression x; value = v } -> v,
    { tuple_expression_right_paren = validate_token x.Syntax.tuple_expression_right_paren
    ; tuple_expression_items = validate_list_with (validate_expression) x.Syntax.tuple_expression_items
    ; tuple_expression_left_paren = validate_token x.Syntax.tuple_expression_left_paren
    ; tuple_expression_keyword = validate_token x.Syntax.tuple_expression_keyword
    }
  | s -> validation_fail SyntaxKind.TupleExpression s
  and invalidate_tuple_expression : tuple_expression invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TupleExpression
      { Syntax.tuple_expression_keyword = invalidate_token x.tuple_expression_keyword
      ; Syntax.tuple_expression_left_paren = invalidate_token x.tuple_expression_left_paren
      ; Syntax.tuple_expression_items = invalidate_list_with (invalidate_expression) x.tuple_expression_items
      ; Syntax.tuple_expression_right_paren = invalidate_token x.tuple_expression_right_paren
      }
    ; Syntax.value = v
    }
  and validate_generic_type_specifier : generic_type_specifier validator = function
  | { Syntax.syntax = Syntax.GenericTypeSpecifier x; value = v } -> v,
    { generic_argument_list = validate_type_arguments x.Syntax.generic_argument_list
    ; generic_class_type = validate_token x.Syntax.generic_class_type
    }
  | s -> validation_fail SyntaxKind.GenericTypeSpecifier s
  and invalidate_generic_type_specifier : generic_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.GenericTypeSpecifier
      { Syntax.generic_class_type = invalidate_token x.generic_class_type
      ; Syntax.generic_argument_list = invalidate_type_arguments x.generic_argument_list
      }
    ; Syntax.value = v
    }
  and validate_nullable_type_specifier : nullable_type_specifier validator = function
  | { Syntax.syntax = Syntax.NullableTypeSpecifier x; value = v } -> v,
    { nullable_type = validate_specifier x.Syntax.nullable_type
    ; nullable_question = validate_token x.Syntax.nullable_question
    }
  | s -> validation_fail SyntaxKind.NullableTypeSpecifier s
  and invalidate_nullable_type_specifier : nullable_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.NullableTypeSpecifier
      { Syntax.nullable_question = invalidate_token x.nullable_question
      ; Syntax.nullable_type = invalidate_specifier x.nullable_type
      }
    ; Syntax.value = v
    }
  and validate_soft_type_specifier : soft_type_specifier validator = function
  | { Syntax.syntax = Syntax.SoftTypeSpecifier x; value = v } -> v,
    { soft_type = validate_specifier x.Syntax.soft_type
    ; soft_at = validate_token x.Syntax.soft_at
    }
  | s -> validation_fail SyntaxKind.SoftTypeSpecifier s
  and invalidate_soft_type_specifier : soft_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.SoftTypeSpecifier
      { Syntax.soft_at = invalidate_token x.soft_at
      ; Syntax.soft_type = invalidate_specifier x.soft_type
      }
    ; Syntax.value = v
    }
  and validate_type_arguments : type_arguments validator = function
  | { Syntax.syntax = Syntax.TypeArguments x; value = v } -> v,
    { type_arguments_right_angle = validate_token x.Syntax.type_arguments_right_angle
    ; type_arguments_types = validate_list_with (validate_specifier) x.Syntax.type_arguments_types
    ; type_arguments_left_angle = validate_token x.Syntax.type_arguments_left_angle
    }
  | s -> validation_fail SyntaxKind.TypeArguments s
  and invalidate_type_arguments : type_arguments invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeArguments
      { Syntax.type_arguments_left_angle = invalidate_token x.type_arguments_left_angle
      ; Syntax.type_arguments_types = invalidate_list_with (invalidate_specifier) x.type_arguments_types
      ; Syntax.type_arguments_right_angle = invalidate_token x.type_arguments_right_angle
      }
    ; Syntax.value = v
    }
  and validate_type_parameters : type_parameters validator = function
  | { Syntax.syntax = Syntax.TypeParameters x; value = v } -> v,
    { type_parameters_right_angle = validate_token x.Syntax.type_parameters_right_angle
    ; type_parameters_parameters = validate_list_with (validate_type_parameter) x.Syntax.type_parameters_parameters
    ; type_parameters_left_angle = validate_token x.Syntax.type_parameters_left_angle
    }
  | s -> validation_fail SyntaxKind.TypeParameters s
  and invalidate_type_parameters : type_parameters invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TypeParameters
      { Syntax.type_parameters_left_angle = invalidate_token x.type_parameters_left_angle
      ; Syntax.type_parameters_parameters = invalidate_list_with (invalidate_type_parameter) x.type_parameters_parameters
      ; Syntax.type_parameters_right_angle = invalidate_token x.type_parameters_right_angle
      }
    ; Syntax.value = v
    }
  and validate_tuple_type_specifier : tuple_type_specifier validator = function
  | { Syntax.syntax = Syntax.TupleTypeSpecifier x; value = v } -> v,
    { tuple_right_paren = validate_token x.Syntax.tuple_right_paren
    ; tuple_types = validate_list_with (validate_option_with (validate_specifier)) x.Syntax.tuple_types
    ; tuple_left_paren = validate_token x.Syntax.tuple_left_paren
    }
  | s -> validation_fail SyntaxKind.TupleTypeSpecifier s
  and invalidate_tuple_type_specifier : tuple_type_specifier invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.TupleTypeSpecifier
      { Syntax.tuple_left_paren = invalidate_token x.tuple_left_paren
      ; Syntax.tuple_types = invalidate_list_with (invalidate_option_with (invalidate_specifier)) x.tuple_types
      ; Syntax.tuple_right_paren = invalidate_token x.tuple_right_paren
      }
    ; Syntax.value = v
    }

end (* Make *)
