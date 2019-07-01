(**
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
* This module contains a signature which can be used to describe the public
* surface area of a constructable syntax tree.

 *)

module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind

module type Syntax_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type value [@@deriving show]
  type t = { syntax : syntax ; value : value } [@@deriving show]
  and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
  | EndOfFile                         of
    { end_of_file_token                                  : t
    }
  | Script                            of
    { script_declarations                                : t
    }
  | QualifiedName                     of
    { qualified_name_parts                               : t
    }
  | SimpleTypeSpecifier               of
    { simple_type_specifier                              : t
    }
  | LiteralExpression                 of
    { literal_expression                                 : t
    }
  | PrefixedStringExpression          of
    { prefixed_string_name                               : t
    ; prefixed_string_str                                : t
    }
  | VariableExpression                of
    { variable_expression                                : t
    }
  | PipeVariableExpression            of
    { pipe_variable_expression                           : t
    }
  | FileAttributeSpecification        of
    { file_attribute_specification_left_double_angle     : t
    ; file_attribute_specification_keyword               : t
    ; file_attribute_specification_colon                 : t
    ; file_attribute_specification_attributes            : t
    ; file_attribute_specification_right_double_angle    : t
    }
  | EnumDeclaration                   of
    { enum_attribute_spec                                : t
    ; enum_keyword                                       : t
    ; enum_name                                          : t
    ; enum_colon                                         : t
    ; enum_base                                          : t
    ; enum_type                                          : t
    ; enum_left_brace                                    : t
    ; enum_enumerators                                   : t
    ; enum_right_brace                                   : t
    }
  | Enumerator                        of
    { enumerator_name                                    : t
    ; enumerator_equal                                   : t
    ; enumerator_value                                   : t
    ; enumerator_semicolon                               : t
    }
  | RecordDeclaration                 of
    { record_attribute_spec                              : t
    ; record_modifier                                    : t
    ; record_keyword                                     : t
    ; record_name                                        : t
    ; record_extends_keyword                             : t
    ; record_extends_list                                : t
    ; record_left_brace                                  : t
    ; record_fields                                      : t
    ; record_right_brace                                 : t
    }
  | RecordField                       of
    { record_field_name                                  : t
    ; record_field_colon                                 : t
    ; record_field_type                                  : t
    ; record_field_init                                  : t
    ; record_field_comma                                 : t
    }
  | AliasDeclaration                  of
    { alias_attribute_spec                               : t
    ; alias_keyword                                      : t
    ; alias_name                                         : t
    ; alias_generic_parameter                            : t
    ; alias_constraint                                   : t
    ; alias_equal                                        : t
    ; alias_type                                         : t
    ; alias_semicolon                                    : t
    }
  | PropertyDeclaration               of
    { property_attribute_spec                            : t
    ; property_modifiers                                 : t
    ; property_type                                      : t
    ; property_declarators                               : t
    ; property_semicolon                                 : t
    }
  | PropertyDeclarator                of
    { property_name                                      : t
    ; property_initializer                               : t
    }
  | NamespaceDeclaration              of
    { namespace_keyword                                  : t
    ; namespace_name                                     : t
    ; namespace_body                                     : t
    }
  | NamespaceBody                     of
    { namespace_left_brace                               : t
    ; namespace_declarations                             : t
    ; namespace_right_brace                              : t
    }
  | NamespaceEmptyBody                of
    { namespace_semicolon                                : t
    }
  | NamespaceUseDeclaration           of
    { namespace_use_keyword                              : t
    ; namespace_use_kind                                 : t
    ; namespace_use_clauses                              : t
    ; namespace_use_semicolon                            : t
    }
  | NamespaceGroupUseDeclaration      of
    { namespace_group_use_keyword                        : t
    ; namespace_group_use_kind                           : t
    ; namespace_group_use_prefix                         : t
    ; namespace_group_use_left_brace                     : t
    ; namespace_group_use_clauses                        : t
    ; namespace_group_use_right_brace                    : t
    ; namespace_group_use_semicolon                      : t
    }
  | NamespaceUseClause                of
    { namespace_use_clause_kind                          : t
    ; namespace_use_name                                 : t
    ; namespace_use_as                                   : t
    ; namespace_use_alias                                : t
    }
  | FunctionDeclaration               of
    { function_attribute_spec                            : t
    ; function_declaration_header                        : t
    ; function_body                                      : t
    }
  | FunctionDeclarationHeader         of
    { function_modifiers                                 : t
    ; function_keyword                                   : t
    ; function_name                                      : t
    ; function_type_parameter_list                       : t
    ; function_left_paren                                : t
    ; function_parameter_list                            : t
    ; function_right_paren                               : t
    ; function_colon                                     : t
    ; function_type                                      : t
    ; function_where_clause                              : t
    }
  | WhereClause                       of
    { where_clause_keyword                               : t
    ; where_clause_constraints                           : t
    }
  | WhereConstraint                   of
    { where_constraint_left_type                         : t
    ; where_constraint_operator                          : t
    ; where_constraint_right_type                        : t
    }
  | MethodishDeclaration              of
    { methodish_attribute                                : t
    ; methodish_function_decl_header                     : t
    ; methodish_function_body                            : t
    ; methodish_semicolon                                : t
    }
  | MethodishTraitResolution          of
    { methodish_trait_attribute                          : t
    ; methodish_trait_function_decl_header               : t
    ; methodish_trait_equal                              : t
    ; methodish_trait_name                               : t
    ; methodish_trait_semicolon                          : t
    }
  | ClassishDeclaration               of
    { classish_attribute                                 : t
    ; classish_modifiers                                 : t
    ; classish_keyword                                   : t
    ; classish_name                                      : t
    ; classish_type_parameters                           : t
    ; classish_extends_keyword                           : t
    ; classish_extends_list                              : t
    ; classish_implements_keyword                        : t
    ; classish_implements_list                           : t
    ; classish_where_clause                              : t
    ; classish_body                                      : t
    }
  | ClassishBody                      of
    { classish_body_left_brace                           : t
    ; classish_body_elements                             : t
    ; classish_body_right_brace                          : t
    }
  | TraitUsePrecedenceItem            of
    { trait_use_precedence_item_name                     : t
    ; trait_use_precedence_item_keyword                  : t
    ; trait_use_precedence_item_removed_names            : t
    }
  | TraitUseAliasItem                 of
    { trait_use_alias_item_aliasing_name                 : t
    ; trait_use_alias_item_keyword                       : t
    ; trait_use_alias_item_modifiers                     : t
    ; trait_use_alias_item_aliased_name                  : t
    }
  | TraitUseConflictResolution        of
    { trait_use_conflict_resolution_keyword              : t
    ; trait_use_conflict_resolution_names                : t
    ; trait_use_conflict_resolution_left_brace           : t
    ; trait_use_conflict_resolution_clauses              : t
    ; trait_use_conflict_resolution_right_brace          : t
    }
  | TraitUse                          of
    { trait_use_keyword                                  : t
    ; trait_use_names                                    : t
    ; trait_use_semicolon                                : t
    }
  | RequireClause                     of
    { require_keyword                                    : t
    ; require_kind                                       : t
    ; require_name                                       : t
    ; require_semicolon                                  : t
    }
  | ConstDeclaration                  of
    { const_modifiers                                    : t
    ; const_keyword                                      : t
    ; const_type_specifier                               : t
    ; const_declarators                                  : t
    ; const_semicolon                                    : t
    }
  | ConstantDeclarator                of
    { constant_declarator_name                           : t
    ; constant_declarator_initializer                    : t
    }
  | TypeConstDeclaration              of
    { type_const_attribute_spec                          : t
    ; type_const_modifiers                               : t
    ; type_const_keyword                                 : t
    ; type_const_type_keyword                            : t
    ; type_const_name                                    : t
    ; type_const_type_parameters                         : t
    ; type_const_type_constraint                         : t
    ; type_const_equal                                   : t
    ; type_const_type_specifier                          : t
    ; type_const_semicolon                               : t
    }
  | DecoratedExpression               of
    { decorated_expression_decorator                     : t
    ; decorated_expression_expression                    : t
    }
  | ParameterDeclaration              of
    { parameter_attribute                                : t
    ; parameter_visibility                               : t
    ; parameter_call_convention                          : t
    ; parameter_type                                     : t
    ; parameter_name                                     : t
    ; parameter_default_value                            : t
    }
  | VariadicParameter                 of
    { variadic_parameter_call_convention                 : t
    ; variadic_parameter_type                            : t
    ; variadic_parameter_ellipsis                        : t
    }
  | AttributeSpecification            of
    { attribute_specification_left_double_angle          : t
    ; attribute_specification_attributes                 : t
    ; attribute_specification_right_double_angle         : t
    }
  | InclusionExpression               of
    { inclusion_require                                  : t
    ; inclusion_filename                                 : t
    }
  | InclusionDirective                of
    { inclusion_expression                               : t
    ; inclusion_semicolon                                : t
    }
  | CompoundStatement                 of
    { compound_left_brace                                : t
    ; compound_statements                                : t
    ; compound_right_brace                               : t
    }
  | ExpressionStatement               of
    { expression_statement_expression                    : t
    ; expression_statement_semicolon                     : t
    }
  | MarkupSection                     of
    { markup_prefix                                      : t
    ; markup_text                                        : t
    ; markup_suffix                                      : t
    ; markup_expression                                  : t
    }
  | MarkupSuffix                      of
    { markup_suffix_less_than_question                   : t
    ; markup_suffix_name                                 : t
    }
  | UnsetStatement                    of
    { unset_keyword                                      : t
    ; unset_left_paren                                   : t
    ; unset_variables                                    : t
    ; unset_right_paren                                  : t
    ; unset_semicolon                                    : t
    }
  | LetStatement                      of
    { let_statement_keyword                              : t
    ; let_statement_name                                 : t
    ; let_statement_colon                                : t
    ; let_statement_type                                 : t
    ; let_statement_initializer                          : t
    ; let_statement_semicolon                            : t
    }
  | UsingStatementBlockScoped         of
    { using_block_await_keyword                          : t
    ; using_block_using_keyword                          : t
    ; using_block_left_paren                             : t
    ; using_block_expressions                            : t
    ; using_block_right_paren                            : t
    ; using_block_body                                   : t
    }
  | UsingStatementFunctionScoped      of
    { using_function_await_keyword                       : t
    ; using_function_using_keyword                       : t
    ; using_function_expression                          : t
    ; using_function_semicolon                           : t
    }
  | WhileStatement                    of
    { while_keyword                                      : t
    ; while_left_paren                                   : t
    ; while_condition                                    : t
    ; while_right_paren                                  : t
    ; while_body                                         : t
    }
  | IfStatement                       of
    { if_keyword                                         : t
    ; if_left_paren                                      : t
    ; if_condition                                       : t
    ; if_right_paren                                     : t
    ; if_statement                                       : t
    ; if_elseif_clauses                                  : t
    ; if_else_clause                                     : t
    }
  | ElseifClause                      of
    { elseif_keyword                                     : t
    ; elseif_left_paren                                  : t
    ; elseif_condition                                   : t
    ; elseif_right_paren                                 : t
    ; elseif_statement                                   : t
    }
  | ElseClause                        of
    { else_keyword                                       : t
    ; else_statement                                     : t
    }
  | TryStatement                      of
    { try_keyword                                        : t
    ; try_compound_statement                             : t
    ; try_catch_clauses                                  : t
    ; try_finally_clause                                 : t
    }
  | CatchClause                       of
    { catch_keyword                                      : t
    ; catch_left_paren                                   : t
    ; catch_type                                         : t
    ; catch_variable                                     : t
    ; catch_right_paren                                  : t
    ; catch_body                                         : t
    }
  | FinallyClause                     of
    { finally_keyword                                    : t
    ; finally_body                                       : t
    }
  | DoStatement                       of
    { do_keyword                                         : t
    ; do_body                                            : t
    ; do_while_keyword                                   : t
    ; do_left_paren                                      : t
    ; do_condition                                       : t
    ; do_right_paren                                     : t
    ; do_semicolon                                       : t
    }
  | ForStatement                      of
    { for_keyword                                        : t
    ; for_left_paren                                     : t
    ; for_initializer                                    : t
    ; for_first_semicolon                                : t
    ; for_control                                        : t
    ; for_second_semicolon                               : t
    ; for_end_of_loop                                    : t
    ; for_right_paren                                    : t
    ; for_body                                           : t
    }
  | ForeachStatement                  of
    { foreach_keyword                                    : t
    ; foreach_left_paren                                 : t
    ; foreach_collection                                 : t
    ; foreach_await_keyword                              : t
    ; foreach_as                                         : t
    ; foreach_key                                        : t
    ; foreach_arrow                                      : t
    ; foreach_value                                      : t
    ; foreach_right_paren                                : t
    ; foreach_body                                       : t
    }
  | SwitchStatement                   of
    { switch_keyword                                     : t
    ; switch_left_paren                                  : t
    ; switch_expression                                  : t
    ; switch_right_paren                                 : t
    ; switch_left_brace                                  : t
    ; switch_sections                                    : t
    ; switch_right_brace                                 : t
    }
  | SwitchSection                     of
    { switch_section_labels                              : t
    ; switch_section_statements                          : t
    ; switch_section_fallthrough                         : t
    }
  | SwitchFallthrough                 of
    { fallthrough_keyword                                : t
    ; fallthrough_semicolon                              : t
    }
  | CaseLabel                         of
    { case_keyword                                       : t
    ; case_expression                                    : t
    ; case_colon                                         : t
    }
  | DefaultLabel                      of
    { default_keyword                                    : t
    ; default_colon                                      : t
    }
  | ReturnStatement                   of
    { return_keyword                                     : t
    ; return_expression                                  : t
    ; return_semicolon                                   : t
    }
  | GotoLabel                         of
    { goto_label_name                                    : t
    ; goto_label_colon                                   : t
    }
  | GotoStatement                     of
    { goto_statement_keyword                             : t
    ; goto_statement_label_name                          : t
    ; goto_statement_semicolon                           : t
    }
  | ThrowStatement                    of
    { throw_keyword                                      : t
    ; throw_expression                                   : t
    ; throw_semicolon                                    : t
    }
  | BreakStatement                    of
    { break_keyword                                      : t
    ; break_level                                        : t
    ; break_semicolon                                    : t
    }
  | ContinueStatement                 of
    { continue_keyword                                   : t
    ; continue_level                                     : t
    ; continue_semicolon                                 : t
    }
  | EchoStatement                     of
    { echo_keyword                                       : t
    ; echo_expressions                                   : t
    ; echo_semicolon                                     : t
    }
  | ConcurrentStatement               of
    { concurrent_keyword                                 : t
    ; concurrent_statement                               : t
    }
  | SimpleInitializer                 of
    { simple_initializer_equal                           : t
    ; simple_initializer_value                           : t
    }
  | AnonymousClass                    of
    { anonymous_class_class_keyword                      : t
    ; anonymous_class_left_paren                         : t
    ; anonymous_class_argument_list                      : t
    ; anonymous_class_right_paren                        : t
    ; anonymous_class_extends_keyword                    : t
    ; anonymous_class_extends_list                       : t
    ; anonymous_class_implements_keyword                 : t
    ; anonymous_class_implements_list                    : t
    ; anonymous_class_body                               : t
    }
  | AnonymousFunction                 of
    { anonymous_attribute_spec                           : t
    ; anonymous_static_keyword                           : t
    ; anonymous_async_keyword                            : t
    ; anonymous_coroutine_keyword                        : t
    ; anonymous_function_keyword                         : t
    ; anonymous_left_paren                               : t
    ; anonymous_parameters                               : t
    ; anonymous_right_paren                              : t
    ; anonymous_colon                                    : t
    ; anonymous_type                                     : t
    ; anonymous_use                                      : t
    ; anonymous_body                                     : t
    }
  | AnonymousFunctionUseClause        of
    { anonymous_use_keyword                              : t
    ; anonymous_use_left_paren                           : t
    ; anonymous_use_variables                            : t
    ; anonymous_use_right_paren                          : t
    }
  | LambdaExpression                  of
    { lambda_attribute_spec                              : t
    ; lambda_async                                       : t
    ; lambda_coroutine                                   : t
    ; lambda_signature                                   : t
    ; lambda_arrow                                       : t
    ; lambda_body                                        : t
    }
  | LambdaSignature                   of
    { lambda_left_paren                                  : t
    ; lambda_parameters                                  : t
    ; lambda_right_paren                                 : t
    ; lambda_colon                                       : t
    ; lambda_type                                        : t
    }
  | CastExpression                    of
    { cast_left_paren                                    : t
    ; cast_type                                          : t
    ; cast_right_paren                                   : t
    ; cast_operand                                       : t
    }
  | ScopeResolutionExpression         of
    { scope_resolution_qualifier                         : t
    ; scope_resolution_operator                          : t
    ; scope_resolution_name                              : t
    }
  | MemberSelectionExpression         of
    { member_object                                      : t
    ; member_operator                                    : t
    ; member_name                                        : t
    }
  | SafeMemberSelectionExpression     of
    { safe_member_object                                 : t
    ; safe_member_operator                               : t
    ; safe_member_name                                   : t
    }
  | EmbeddedMemberSelectionExpression of
    { embedded_member_object                             : t
    ; embedded_member_operator                           : t
    ; embedded_member_name                               : t
    }
  | YieldExpression                   of
    { yield_keyword                                      : t
    ; yield_operand                                      : t
    }
  | YieldFromExpression               of
    { yield_from_yield_keyword                           : t
    ; yield_from_from_keyword                            : t
    ; yield_from_operand                                 : t
    }
  | PrefixUnaryExpression             of
    { prefix_unary_operator                              : t
    ; prefix_unary_operand                               : t
    }
  | PostfixUnaryExpression            of
    { postfix_unary_operand                              : t
    ; postfix_unary_operator                             : t
    }
  | BinaryExpression                  of
    { binary_left_operand                                : t
    ; binary_operator                                    : t
    ; binary_right_operand                               : t
    }
  | InstanceofExpression              of
    { instanceof_left_operand                            : t
    ; instanceof_operator                                : t
    ; instanceof_right_operand                           : t
    }
  | IsExpression                      of
    { is_left_operand                                    : t
    ; is_operator                                        : t
    ; is_right_operand                                   : t
    }
  | AsExpression                      of
    { as_left_operand                                    : t
    ; as_operator                                        : t
    ; as_right_operand                                   : t
    }
  | NullableAsExpression              of
    { nullable_as_left_operand                           : t
    ; nullable_as_operator                               : t
    ; nullable_as_right_operand                          : t
    }
  | ConditionalExpression             of
    { conditional_test                                   : t
    ; conditional_question                               : t
    ; conditional_consequence                            : t
    ; conditional_colon                                  : t
    ; conditional_alternative                            : t
    }
  | EvalExpression                    of
    { eval_keyword                                       : t
    ; eval_left_paren                                    : t
    ; eval_argument                                      : t
    ; eval_right_paren                                   : t
    }
  | DefineExpression                  of
    { define_keyword                                     : t
    ; define_left_paren                                  : t
    ; define_argument_list                               : t
    ; define_right_paren                                 : t
    }
  | HaltCompilerExpression            of
    { halt_compiler_keyword                              : t
    ; halt_compiler_left_paren                           : t
    ; halt_compiler_argument_list                        : t
    ; halt_compiler_right_paren                          : t
    }
  | IssetExpression                   of
    { isset_keyword                                      : t
    ; isset_left_paren                                   : t
    ; isset_argument_list                                : t
    ; isset_right_paren                                  : t
    }
  | FunctionCallExpression            of
    { function_call_receiver                             : t
    ; function_call_type_args                            : t
    ; function_call_left_paren                           : t
    ; function_call_argument_list                        : t
    ; function_call_right_paren                          : t
    }
  | ParenthesizedExpression           of
    { parenthesized_expression_left_paren                : t
    ; parenthesized_expression_expression                : t
    ; parenthesized_expression_right_paren               : t
    }
  | BracedExpression                  of
    { braced_expression_left_brace                       : t
    ; braced_expression_expression                       : t
    ; braced_expression_right_brace                      : t
    }
  | EmbeddedBracedExpression          of
    { embedded_braced_expression_left_brace              : t
    ; embedded_braced_expression_expression              : t
    ; embedded_braced_expression_right_brace             : t
    }
  | ListExpression                    of
    { list_keyword                                       : t
    ; list_left_paren                                    : t
    ; list_members                                       : t
    ; list_right_paren                                   : t
    }
  | CollectionLiteralExpression       of
    { collection_literal_name                            : t
    ; collection_literal_left_brace                      : t
    ; collection_literal_initializers                    : t
    ; collection_literal_right_brace                     : t
    }
  | ObjectCreationExpression          of
    { object_creation_new_keyword                        : t
    ; object_creation_object                             : t
    }
  | ConstructorCall                   of
    { constructor_call_type                              : t
    ; constructor_call_left_paren                        : t
    ; constructor_call_argument_list                     : t
    ; constructor_call_right_paren                       : t
    }
  | RecordCreationExpression          of
    { record_creation_type                               : t
    ; record_creation_left_bracket                       : t
    ; record_creation_members                            : t
    ; record_creation_right_bracket                      : t
    }
  | ArrayCreationExpression           of
    { array_creation_left_bracket                        : t
    ; array_creation_members                             : t
    ; array_creation_right_bracket                       : t
    }
  | ArrayIntrinsicExpression          of
    { array_intrinsic_keyword                            : t
    ; array_intrinsic_left_paren                         : t
    ; array_intrinsic_members                            : t
    ; array_intrinsic_right_paren                        : t
    }
  | DarrayIntrinsicExpression         of
    { darray_intrinsic_keyword                           : t
    ; darray_intrinsic_explicit_type                     : t
    ; darray_intrinsic_left_bracket                      : t
    ; darray_intrinsic_members                           : t
    ; darray_intrinsic_right_bracket                     : t
    }
  | DictionaryIntrinsicExpression     of
    { dictionary_intrinsic_keyword                       : t
    ; dictionary_intrinsic_explicit_type                 : t
    ; dictionary_intrinsic_left_bracket                  : t
    ; dictionary_intrinsic_members                       : t
    ; dictionary_intrinsic_right_bracket                 : t
    }
  | KeysetIntrinsicExpression         of
    { keyset_intrinsic_keyword                           : t
    ; keyset_intrinsic_explicit_type                     : t
    ; keyset_intrinsic_left_bracket                      : t
    ; keyset_intrinsic_members                           : t
    ; keyset_intrinsic_right_bracket                     : t
    }
  | VarrayIntrinsicExpression         of
    { varray_intrinsic_keyword                           : t
    ; varray_intrinsic_explicit_type                     : t
    ; varray_intrinsic_left_bracket                      : t
    ; varray_intrinsic_members                           : t
    ; varray_intrinsic_right_bracket                     : t
    }
  | VectorIntrinsicExpression         of
    { vector_intrinsic_keyword                           : t
    ; vector_intrinsic_explicit_type                     : t
    ; vector_intrinsic_left_bracket                      : t
    ; vector_intrinsic_members                           : t
    ; vector_intrinsic_right_bracket                     : t
    }
  | ElementInitializer                of
    { element_key                                        : t
    ; element_arrow                                      : t
    ; element_value                                      : t
    }
  | SubscriptExpression               of
    { subscript_receiver                                 : t
    ; subscript_left_bracket                             : t
    ; subscript_index                                    : t
    ; subscript_right_bracket                            : t
    }
  | EmbeddedSubscriptExpression       of
    { embedded_subscript_receiver                        : t
    ; embedded_subscript_left_bracket                    : t
    ; embedded_subscript_index                           : t
    ; embedded_subscript_right_bracket                   : t
    }
  | AwaitableCreationExpression       of
    { awaitable_attribute_spec                           : t
    ; awaitable_async                                    : t
    ; awaitable_coroutine                                : t
    ; awaitable_compound_statement                       : t
    }
  | XHPChildrenDeclaration            of
    { xhp_children_keyword                               : t
    ; xhp_children_expression                            : t
    ; xhp_children_semicolon                             : t
    }
  | XHPChildrenParenthesizedList      of
    { xhp_children_list_left_paren                       : t
    ; xhp_children_list_xhp_children                     : t
    ; xhp_children_list_right_paren                      : t
    }
  | XHPCategoryDeclaration            of
    { xhp_category_keyword                               : t
    ; xhp_category_categories                            : t
    ; xhp_category_semicolon                             : t
    }
  | XHPEnumType                       of
    { xhp_enum_optional                                  : t
    ; xhp_enum_keyword                                   : t
    ; xhp_enum_left_brace                                : t
    ; xhp_enum_values                                    : t
    ; xhp_enum_right_brace                               : t
    }
  | XHPLateinit                       of
    { xhp_lateinit_at                                    : t
    ; xhp_lateinit_keyword                               : t
    }
  | XHPRequired                       of
    { xhp_required_at                                    : t
    ; xhp_required_keyword                               : t
    }
  | XHPClassAttributeDeclaration      of
    { xhp_attribute_keyword                              : t
    ; xhp_attribute_attributes                           : t
    ; xhp_attribute_semicolon                            : t
    }
  | XHPClassAttribute                 of
    { xhp_attribute_decl_type                            : t
    ; xhp_attribute_decl_name                            : t
    ; xhp_attribute_decl_initializer                     : t
    ; xhp_attribute_decl_required                        : t
    }
  | XHPSimpleClassAttribute           of
    { xhp_simple_class_attribute_type                    : t
    }
  | XHPSimpleAttribute                of
    { xhp_simple_attribute_name                          : t
    ; xhp_simple_attribute_equal                         : t
    ; xhp_simple_attribute_expression                    : t
    }
  | XHPSpreadAttribute                of
    { xhp_spread_attribute_left_brace                    : t
    ; xhp_spread_attribute_spread_operator               : t
    ; xhp_spread_attribute_expression                    : t
    ; xhp_spread_attribute_right_brace                   : t
    }
  | XHPOpen                           of
    { xhp_open_left_angle                                : t
    ; xhp_open_name                                      : t
    ; xhp_open_attributes                                : t
    ; xhp_open_right_angle                               : t
    }
  | XHPExpression                     of
    { xhp_open                                           : t
    ; xhp_body                                           : t
    ; xhp_close                                          : t
    }
  | XHPClose                          of
    { xhp_close_left_angle                               : t
    ; xhp_close_name                                     : t
    ; xhp_close_right_angle                              : t
    }
  | TypeConstant                      of
    { type_constant_left_type                            : t
    ; type_constant_separator                            : t
    ; type_constant_right_type                           : t
    }
  | VectorTypeSpecifier               of
    { vector_type_keyword                                : t
    ; vector_type_left_angle                             : t
    ; vector_type_type                                   : t
    ; vector_type_trailing_comma                         : t
    ; vector_type_right_angle                            : t
    }
  | KeysetTypeSpecifier               of
    { keyset_type_keyword                                : t
    ; keyset_type_left_angle                             : t
    ; keyset_type_type                                   : t
    ; keyset_type_trailing_comma                         : t
    ; keyset_type_right_angle                            : t
    }
  | TupleTypeExplicitSpecifier        of
    { tuple_type_keyword                                 : t
    ; tuple_type_left_angle                              : t
    ; tuple_type_types                                   : t
    ; tuple_type_right_angle                             : t
    }
  | VarrayTypeSpecifier               of
    { varray_keyword                                     : t
    ; varray_left_angle                                  : t
    ; varray_type                                        : t
    ; varray_trailing_comma                              : t
    ; varray_right_angle                                 : t
    }
  | VectorArrayTypeSpecifier          of
    { vector_array_keyword                               : t
    ; vector_array_left_angle                            : t
    ; vector_array_type                                  : t
    ; vector_array_right_angle                           : t
    }
  | TypeParameter                     of
    { type_attribute_spec                                : t
    ; type_reified                                       : t
    ; type_variance                                      : t
    ; type_name                                          : t
    ; type_constraints                                   : t
    }
  | TypeConstraint                    of
    { constraint_keyword                                 : t
    ; constraint_type                                    : t
    }
  | DarrayTypeSpecifier               of
    { darray_keyword                                     : t
    ; darray_left_angle                                  : t
    ; darray_key                                         : t
    ; darray_comma                                       : t
    ; darray_value                                       : t
    ; darray_trailing_comma                              : t
    ; darray_right_angle                                 : t
    }
  | MapArrayTypeSpecifier             of
    { map_array_keyword                                  : t
    ; map_array_left_angle                               : t
    ; map_array_key                                      : t
    ; map_array_comma                                    : t
    ; map_array_value                                    : t
    ; map_array_right_angle                              : t
    }
  | DictionaryTypeSpecifier           of
    { dictionary_type_keyword                            : t
    ; dictionary_type_left_angle                         : t
    ; dictionary_type_members                            : t
    ; dictionary_type_right_angle                        : t
    }
  | ClosureTypeSpecifier              of
    { closure_outer_left_paren                           : t
    ; closure_coroutine                                  : t
    ; closure_function_keyword                           : t
    ; closure_inner_left_paren                           : t
    ; closure_parameter_list                             : t
    ; closure_inner_right_paren                          : t
    ; closure_colon                                      : t
    ; closure_return_type                                : t
    ; closure_outer_right_paren                          : t
    }
  | ClosureParameterTypeSpecifier     of
    { closure_parameter_call_convention                  : t
    ; closure_parameter_type                             : t
    }
  | ClassnameTypeSpecifier            of
    { classname_keyword                                  : t
    ; classname_left_angle                               : t
    ; classname_type                                     : t
    ; classname_trailing_comma                           : t
    ; classname_right_angle                              : t
    }
  | FieldSpecifier                    of
    { field_question                                     : t
    ; field_name                                         : t
    ; field_arrow                                        : t
    ; field_type                                         : t
    }
  | FieldInitializer                  of
    { field_initializer_name                             : t
    ; field_initializer_arrow                            : t
    ; field_initializer_value                            : t
    }
  | ShapeTypeSpecifier                of
    { shape_type_keyword                                 : t
    ; shape_type_left_paren                              : t
    ; shape_type_fields                                  : t
    ; shape_type_ellipsis                                : t
    ; shape_type_right_paren                             : t
    }
  | ShapeExpression                   of
    { shape_expression_keyword                           : t
    ; shape_expression_left_paren                        : t
    ; shape_expression_fields                            : t
    ; shape_expression_right_paren                       : t
    }
  | TupleExpression                   of
    { tuple_expression_keyword                           : t
    ; tuple_expression_left_paren                        : t
    ; tuple_expression_items                             : t
    ; tuple_expression_right_paren                       : t
    }
  | GenericTypeSpecifier              of
    { generic_class_type                                 : t
    ; generic_argument_list                              : t
    }
  | NullableTypeSpecifier             of
    { nullable_question                                  : t
    ; nullable_type                                      : t
    }
  | LikeTypeSpecifier                 of
    { like_tilde                                         : t
    ; like_type                                          : t
    }
  | SoftTypeSpecifier                 of
    { soft_at                                            : t
    ; soft_type                                          : t
    }
  | ReifiedTypeArgument               of
    { reified_type_argument_reified                      : t
    ; reified_type_argument_type                         : t
    }
  | TypeArguments                     of
    { type_arguments_left_angle                          : t
    ; type_arguments_types                               : t
    ; type_arguments_right_angle                         : t
    }
  | TypeParameters                    of
    { type_parameters_left_angle                         : t
    ; type_parameters_parameters                         : t
    ; type_parameters_right_angle                        : t
    }
  | TupleTypeSpecifier                of
    { tuple_left_paren                                   : t
    ; tuple_types                                        : t
    ; tuple_right_paren                                  : t
    }
  | ErrorSyntax                       of
    { error_error                                        : t
    }
  | ListItem                          of
    { list_item                                          : t
    ; list_separator                                     : t
    }
  | PocketAtomExpression              of
    { pocket_atom_glyph                                  : t
    ; pocket_atom_expression                             : t
    }
  | PocketIdentifierExpression        of
    { pocket_identifier_qualifier                        : t
    ; pocket_identifier_pu_operator                      : t
    ; pocket_identifier_field                            : t
    ; pocket_identifier_operator                         : t
    ; pocket_identifier_name                             : t
    }
  | PocketAtomMappingDeclaration      of
    { pocket_atom_mapping_glyph                          : t
    ; pocket_atom_mapping_name                           : t
    ; pocket_atom_mapping_left_paren                     : t
    ; pocket_atom_mapping_mappings                       : t
    ; pocket_atom_mapping_right_paren                    : t
    ; pocket_atom_mapping_semicolon                      : t
    }
  | PocketEnumDeclaration             of
    { pocket_enum_modifiers                              : t
    ; pocket_enum_enum                                   : t
    ; pocket_enum_name                                   : t
    ; pocket_enum_left_brace                             : t
    ; pocket_enum_fields                                 : t
    ; pocket_enum_right_brace                            : t
    }
  | PocketFieldTypeExprDeclaration    of
    { pocket_field_type_expr_case                        : t
    ; pocket_field_type_expr_type                        : t
    ; pocket_field_type_expr_name                        : t
    ; pocket_field_type_expr_semicolon                   : t
    }
  | PocketFieldTypeDeclaration        of
    { pocket_field_type_case                             : t
    ; pocket_field_type_type                             : t
    ; pocket_field_type_name                             : t
    ; pocket_field_type_semicolon                        : t
    }
  | PocketMappingIdDeclaration        of
    { pocket_mapping_id_name                             : t
    ; pocket_mapping_id_initializer                      : t
    }
  | PocketMappingTypeDeclaration      of
    { pocket_mapping_type_keyword                        : t
    ; pocket_mapping_type_name                           : t
    ; pocket_mapping_type_equal                          : t
    ; pocket_mapping_type_type                           : t
    }


  val rust_parse :
    Full_fidelity_source_text.t ->
    Full_fidelity_parser_env.t ->
    unit * t * Full_fidelity_syntax_error.t list
  val rust_parse_with_coroutine_sc :
    Full_fidelity_source_text.t ->
    Full_fidelity_parser_env.t ->
    bool * t * Full_fidelity_syntax_error.t list
  val rust_parse_with_decl_mode_sc :
    Full_fidelity_source_text.t ->
    Full_fidelity_parser_env.t ->
    bool list * t * Full_fidelity_syntax_error.t list
  val has_leading_trivia : TriviaKind.t -> Token.t -> bool
  val to_json : ?with_value:bool -> t -> Hh_json.json
  val extract_text : t -> string option
  val is_in_body : t -> int -> bool
  val syntax_node_to_list : t -> t list
  val width : t -> int
  val full_width : t -> int
  val trailing_width : t -> int
  val leading_width : t -> int
  val leading_token : t -> Token.t option
  val children : t -> t list
  val syntax : t -> syntax
  val kind : t -> Full_fidelity_syntax_kind.t
  val value : t -> value
  val make_token : Token.t -> t
  val get_token : t -> Token.t option
  val all_tokens : t -> Token.t list
  val make_missing : Full_fidelity_source_text.t -> int -> t
  val make_list : Full_fidelity_source_text.t -> int -> t list -> t
  val is_namespace_prefix : t -> bool
  val syntax_list_fold : init:'a -> f:('a -> t -> 'a) -> t -> 'a
  val make_end_of_file : t -> t
  val make_script : t -> t
  val make_qualified_name : t -> t
  val make_simple_type_specifier : t -> t
  val make_literal_expression : t -> t
  val make_prefixed_string_expression : t -> t -> t
  val make_variable_expression : t -> t
  val make_pipe_variable_expression : t -> t
  val make_file_attribute_specification : t -> t -> t -> t -> t -> t
  val make_enum_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_enumerator : t -> t -> t -> t -> t
  val make_record_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_record_field : t -> t -> t -> t -> t -> t
  val make_alias_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_property_declaration : t -> t -> t -> t -> t -> t
  val make_property_declarator : t -> t -> t
  val make_namespace_declaration : t -> t -> t -> t
  val make_namespace_body : t -> t -> t -> t
  val make_namespace_empty_body : t -> t
  val make_namespace_use_declaration : t -> t -> t -> t -> t
  val make_namespace_group_use_declaration : t -> t -> t -> t -> t -> t -> t -> t
  val make_namespace_use_clause : t -> t -> t -> t -> t
  val make_function_declaration : t -> t -> t -> t
  val make_function_declaration_header : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_where_clause : t -> t -> t
  val make_where_constraint : t -> t -> t -> t
  val make_methodish_declaration : t -> t -> t -> t -> t
  val make_methodish_trait_resolution : t -> t -> t -> t -> t -> t
  val make_classish_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_classish_body : t -> t -> t -> t
  val make_trait_use_precedence_item : t -> t -> t -> t
  val make_trait_use_alias_item : t -> t -> t -> t -> t
  val make_trait_use_conflict_resolution : t -> t -> t -> t -> t -> t
  val make_trait_use : t -> t -> t -> t
  val make_require_clause : t -> t -> t -> t -> t
  val make_const_declaration : t -> t -> t -> t -> t -> t
  val make_constant_declarator : t -> t -> t
  val make_type_const_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_decorated_expression : t -> t -> t
  val make_parameter_declaration : t -> t -> t -> t -> t -> t -> t
  val make_variadic_parameter : t -> t -> t -> t
  val make_attribute_specification : t -> t -> t -> t
  val make_inclusion_expression : t -> t -> t
  val make_inclusion_directive : t -> t -> t
  val make_compound_statement : t -> t -> t -> t
  val make_expression_statement : t -> t -> t
  val make_markup_section : t -> t -> t -> t -> t
  val make_markup_suffix : t -> t -> t
  val make_unset_statement : t -> t -> t -> t -> t -> t
  val make_let_statement : t -> t -> t -> t -> t -> t -> t
  val make_using_statement_block_scoped : t -> t -> t -> t -> t -> t -> t
  val make_using_statement_function_scoped : t -> t -> t -> t -> t
  val make_while_statement : t -> t -> t -> t -> t -> t
  val make_if_statement : t -> t -> t -> t -> t -> t -> t -> t
  val make_elseif_clause : t -> t -> t -> t -> t -> t
  val make_else_clause : t -> t -> t
  val make_try_statement : t -> t -> t -> t -> t
  val make_catch_clause : t -> t -> t -> t -> t -> t -> t
  val make_finally_clause : t -> t -> t
  val make_do_statement : t -> t -> t -> t -> t -> t -> t -> t
  val make_for_statement : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_foreach_statement : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_switch_statement : t -> t -> t -> t -> t -> t -> t -> t
  val make_switch_section : t -> t -> t -> t
  val make_switch_fallthrough : t -> t -> t
  val make_case_label : t -> t -> t -> t
  val make_default_label : t -> t -> t
  val make_return_statement : t -> t -> t -> t
  val make_goto_label : t -> t -> t
  val make_goto_statement : t -> t -> t -> t
  val make_throw_statement : t -> t -> t -> t
  val make_break_statement : t -> t -> t -> t
  val make_continue_statement : t -> t -> t -> t
  val make_echo_statement : t -> t -> t -> t
  val make_concurrent_statement : t -> t -> t
  val make_simple_initializer : t -> t -> t
  val make_anonymous_class : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_anonymous_function : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_anonymous_function_use_clause : t -> t -> t -> t -> t
  val make_lambda_expression : t -> t -> t -> t -> t -> t -> t
  val make_lambda_signature : t -> t -> t -> t -> t -> t
  val make_cast_expression : t -> t -> t -> t -> t
  val make_scope_resolution_expression : t -> t -> t -> t
  val make_member_selection_expression : t -> t -> t -> t
  val make_safe_member_selection_expression : t -> t -> t -> t
  val make_embedded_member_selection_expression : t -> t -> t -> t
  val make_yield_expression : t -> t -> t
  val make_yield_from_expression : t -> t -> t -> t
  val make_prefix_unary_expression : t -> t -> t
  val make_postfix_unary_expression : t -> t -> t
  val make_binary_expression : t -> t -> t -> t
  val make_instanceof_expression : t -> t -> t -> t
  val make_is_expression : t -> t -> t -> t
  val make_as_expression : t -> t -> t -> t
  val make_nullable_as_expression : t -> t -> t -> t
  val make_conditional_expression : t -> t -> t -> t -> t -> t
  val make_eval_expression : t -> t -> t -> t -> t
  val make_define_expression : t -> t -> t -> t -> t
  val make_halt_compiler_expression : t -> t -> t -> t -> t
  val make_isset_expression : t -> t -> t -> t -> t
  val make_function_call_expression : t -> t -> t -> t -> t -> t
  val make_parenthesized_expression : t -> t -> t -> t
  val make_braced_expression : t -> t -> t -> t
  val make_embedded_braced_expression : t -> t -> t -> t
  val make_list_expression : t -> t -> t -> t -> t
  val make_collection_literal_expression : t -> t -> t -> t -> t
  val make_object_creation_expression : t -> t -> t
  val make_constructor_call : t -> t -> t -> t -> t
  val make_record_creation_expression : t -> t -> t -> t -> t
  val make_array_creation_expression : t -> t -> t -> t
  val make_array_intrinsic_expression : t -> t -> t -> t -> t
  val make_darray_intrinsic_expression : t -> t -> t -> t -> t -> t
  val make_dictionary_intrinsic_expression : t -> t -> t -> t -> t -> t
  val make_keyset_intrinsic_expression : t -> t -> t -> t -> t -> t
  val make_varray_intrinsic_expression : t -> t -> t -> t -> t -> t
  val make_vector_intrinsic_expression : t -> t -> t -> t -> t -> t
  val make_element_initializer : t -> t -> t -> t
  val make_subscript_expression : t -> t -> t -> t -> t
  val make_embedded_subscript_expression : t -> t -> t -> t -> t
  val make_awaitable_creation_expression : t -> t -> t -> t -> t
  val make_xhp_children_declaration : t -> t -> t -> t
  val make_xhp_children_parenthesized_list : t -> t -> t -> t
  val make_xhp_category_declaration : t -> t -> t -> t
  val make_xhp_enum_type : t -> t -> t -> t -> t -> t
  val make_xhp_lateinit : t -> t -> t
  val make_xhp_required : t -> t -> t
  val make_xhp_class_attribute_declaration : t -> t -> t -> t
  val make_xhp_class_attribute : t -> t -> t -> t -> t
  val make_xhp_simple_class_attribute : t -> t
  val make_xhp_simple_attribute : t -> t -> t -> t
  val make_xhp_spread_attribute : t -> t -> t -> t -> t
  val make_xhp_open : t -> t -> t -> t -> t
  val make_xhp_expression : t -> t -> t -> t
  val make_xhp_close : t -> t -> t -> t
  val make_type_constant : t -> t -> t -> t
  val make_vector_type_specifier : t -> t -> t -> t -> t -> t
  val make_keyset_type_specifier : t -> t -> t -> t -> t -> t
  val make_tuple_type_explicit_specifier : t -> t -> t -> t -> t
  val make_varray_type_specifier : t -> t -> t -> t -> t -> t
  val make_vector_array_type_specifier : t -> t -> t -> t -> t
  val make_type_parameter : t -> t -> t -> t -> t -> t
  val make_type_constraint : t -> t -> t
  val make_darray_type_specifier : t -> t -> t -> t -> t -> t -> t -> t
  val make_map_array_type_specifier : t -> t -> t -> t -> t -> t -> t
  val make_dictionary_type_specifier : t -> t -> t -> t -> t
  val make_closure_type_specifier : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_closure_parameter_type_specifier : t -> t -> t
  val make_classname_type_specifier : t -> t -> t -> t -> t -> t
  val make_field_specifier : t -> t -> t -> t -> t
  val make_field_initializer : t -> t -> t -> t
  val make_shape_type_specifier : t -> t -> t -> t -> t -> t
  val make_shape_expression : t -> t -> t -> t -> t
  val make_tuple_expression : t -> t -> t -> t -> t
  val make_generic_type_specifier : t -> t -> t
  val make_nullable_type_specifier : t -> t -> t
  val make_like_type_specifier : t -> t -> t
  val make_soft_type_specifier : t -> t -> t
  val make_reified_type_argument : t -> t -> t
  val make_type_arguments : t -> t -> t -> t
  val make_type_parameters : t -> t -> t -> t
  val make_tuple_type_specifier : t -> t -> t -> t
  val make_error : t -> t
  val make_list_item : t -> t -> t
  val make_pocket_atom_expression : t -> t -> t
  val make_pocket_identifier_expression : t -> t -> t -> t -> t -> t
  val make_pocket_atom_mapping_declaration : t -> t -> t -> t -> t -> t -> t
  val make_pocket_enum_declaration : t -> t -> t -> t -> t -> t -> t
  val make_pocket_field_type_expr_declaration : t -> t -> t -> t -> t
  val make_pocket_field_type_declaration : t -> t -> t -> t -> t
  val make_pocket_mapping_id_declaration : t -> t -> t
  val make_pocket_mapping_type_declaration : t -> t -> t -> t -> t


  val position : Relative_path.t -> t -> Pos.t option
  val offset : t -> int option
  val is_missing : t -> bool
  val is_list : t -> bool
  val is_end_of_file : t -> bool
  val is_script : t -> bool
  val is_qualified_name : t -> bool
  val is_simple_type_specifier : t -> bool
  val is_literal_expression : t -> bool
  val is_prefixed_string_expression : t -> bool
  val is_variable_expression : t -> bool
  val is_pipe_variable_expression : t -> bool
  val is_file_attribute_specification : t -> bool
  val is_enum_declaration : t -> bool
  val is_enumerator : t -> bool
  val is_record_declaration : t -> bool
  val is_record_field : t -> bool
  val is_alias_declaration : t -> bool
  val is_property_declaration : t -> bool
  val is_property_declarator : t -> bool
  val is_namespace_declaration : t -> bool
  val is_namespace_body : t -> bool
  val is_namespace_empty_body : t -> bool
  val is_namespace_use_declaration : t -> bool
  val is_namespace_group_use_declaration : t -> bool
  val is_namespace_use_clause : t -> bool
  val is_function_declaration : t -> bool
  val is_function_declaration_header : t -> bool
  val is_where_clause : t -> bool
  val is_where_constraint : t -> bool
  val is_methodish_declaration : t -> bool
  val is_methodish_trait_resolution : t -> bool
  val is_classish_declaration : t -> bool
  val is_classish_body : t -> bool
  val is_trait_use_precedence_item : t -> bool
  val is_trait_use_alias_item : t -> bool
  val is_trait_use_conflict_resolution : t -> bool
  val is_trait_use : t -> bool
  val is_require_clause : t -> bool
  val is_const_declaration : t -> bool
  val is_constant_declarator : t -> bool
  val is_type_const_declaration : t -> bool
  val is_decorated_expression : t -> bool
  val is_parameter_declaration : t -> bool
  val is_variadic_parameter : t -> bool
  val is_attribute_specification : t -> bool
  val is_inclusion_expression : t -> bool
  val is_inclusion_directive : t -> bool
  val is_compound_statement : t -> bool
  val is_expression_statement : t -> bool
  val is_markup_section : t -> bool
  val is_markup_suffix : t -> bool
  val is_unset_statement : t -> bool
  val is_let_statement : t -> bool
  val is_using_statement_block_scoped : t -> bool
  val is_using_statement_function_scoped : t -> bool
  val is_while_statement : t -> bool
  val is_if_statement : t -> bool
  val is_elseif_clause : t -> bool
  val is_else_clause : t -> bool
  val is_try_statement : t -> bool
  val is_catch_clause : t -> bool
  val is_finally_clause : t -> bool
  val is_do_statement : t -> bool
  val is_for_statement : t -> bool
  val is_foreach_statement : t -> bool
  val is_switch_statement : t -> bool
  val is_switch_section : t -> bool
  val is_switch_fallthrough : t -> bool
  val is_case_label : t -> bool
  val is_default_label : t -> bool
  val is_return_statement : t -> bool
  val is_goto_label : t -> bool
  val is_goto_statement : t -> bool
  val is_throw_statement : t -> bool
  val is_break_statement : t -> bool
  val is_continue_statement : t -> bool
  val is_echo_statement : t -> bool
  val is_concurrent_statement : t -> bool
  val is_simple_initializer : t -> bool
  val is_anonymous_class : t -> bool
  val is_anonymous_function : t -> bool
  val is_anonymous_function_use_clause : t -> bool
  val is_lambda_expression : t -> bool
  val is_lambda_signature : t -> bool
  val is_cast_expression : t -> bool
  val is_scope_resolution_expression : t -> bool
  val is_member_selection_expression : t -> bool
  val is_safe_member_selection_expression : t -> bool
  val is_embedded_member_selection_expression : t -> bool
  val is_yield_expression : t -> bool
  val is_yield_from_expression : t -> bool
  val is_prefix_unary_expression : t -> bool
  val is_postfix_unary_expression : t -> bool
  val is_binary_expression : t -> bool
  val is_instanceof_expression : t -> bool
  val is_is_expression : t -> bool
  val is_as_expression : t -> bool
  val is_nullable_as_expression : t -> bool
  val is_conditional_expression : t -> bool
  val is_eval_expression : t -> bool
  val is_define_expression : t -> bool
  val is_halt_compiler_expression : t -> bool
  val is_isset_expression : t -> bool
  val is_function_call_expression : t -> bool
  val is_parenthesized_expression : t -> bool
  val is_braced_expression : t -> bool
  val is_embedded_braced_expression : t -> bool
  val is_list_expression : t -> bool
  val is_collection_literal_expression : t -> bool
  val is_object_creation_expression : t -> bool
  val is_constructor_call : t -> bool
  val is_record_creation_expression : t -> bool
  val is_array_creation_expression : t -> bool
  val is_array_intrinsic_expression : t -> bool
  val is_darray_intrinsic_expression : t -> bool
  val is_dictionary_intrinsic_expression : t -> bool
  val is_keyset_intrinsic_expression : t -> bool
  val is_varray_intrinsic_expression : t -> bool
  val is_vector_intrinsic_expression : t -> bool
  val is_element_initializer : t -> bool
  val is_subscript_expression : t -> bool
  val is_embedded_subscript_expression : t -> bool
  val is_awaitable_creation_expression : t -> bool
  val is_xhp_children_declaration : t -> bool
  val is_xhp_children_parenthesized_list : t -> bool
  val is_xhp_category_declaration : t -> bool
  val is_xhp_enum_type : t -> bool
  val is_xhp_lateinit : t -> bool
  val is_xhp_required : t -> bool
  val is_xhp_class_attribute_declaration : t -> bool
  val is_xhp_class_attribute : t -> bool
  val is_xhp_simple_class_attribute : t -> bool
  val is_xhp_simple_attribute : t -> bool
  val is_xhp_spread_attribute : t -> bool
  val is_xhp_open : t -> bool
  val is_xhp_expression : t -> bool
  val is_xhp_close : t -> bool
  val is_type_constant : t -> bool
  val is_vector_type_specifier : t -> bool
  val is_keyset_type_specifier : t -> bool
  val is_tuple_type_explicit_specifier : t -> bool
  val is_varray_type_specifier : t -> bool
  val is_vector_array_type_specifier : t -> bool
  val is_type_parameter : t -> bool
  val is_type_constraint : t -> bool
  val is_darray_type_specifier : t -> bool
  val is_map_array_type_specifier : t -> bool
  val is_dictionary_type_specifier : t -> bool
  val is_closure_type_specifier : t -> bool
  val is_closure_parameter_type_specifier : t -> bool
  val is_classname_type_specifier : t -> bool
  val is_field_specifier : t -> bool
  val is_field_initializer : t -> bool
  val is_shape_type_specifier : t -> bool
  val is_shape_expression : t -> bool
  val is_tuple_expression : t -> bool
  val is_generic_type_specifier : t -> bool
  val is_nullable_type_specifier : t -> bool
  val is_like_type_specifier : t -> bool
  val is_soft_type_specifier : t -> bool
  val is_reified_type_argument : t -> bool
  val is_type_arguments : t -> bool
  val is_type_parameters : t -> bool
  val is_tuple_type_specifier : t -> bool
  val is_error : t -> bool
  val is_list_item : t -> bool
  val is_pocket_atom_expression : t -> bool
  val is_pocket_identifier_expression : t -> bool
  val is_pocket_atom_mapping_declaration : t -> bool
  val is_pocket_enum_declaration : t -> bool
  val is_pocket_field_type_expr_declaration : t -> bool
  val is_pocket_field_type_declaration : t -> bool
  val is_pocket_mapping_id_declaration : t -> bool
  val is_pocket_mapping_type_declaration : t -> bool


  val is_specific_token : TokenKind.t -> t -> bool
  val is_loop_statement : t -> bool
  val is_external       : t -> bool
  val is_name           : t -> bool
  val is_construct      : t -> bool
  val is_destruct       : t -> bool
  val is_static         : t -> bool
  val is_private        : t -> bool
  val is_public         : t -> bool
  val is_protected      : t -> bool
  val is_abstract       : t -> bool
  val is_final          : t -> bool
  val is_async          : t -> bool
  val is_coroutine      : t -> bool
  val is_void           : t -> bool
  val is_left_brace     : t -> bool
  val is_ellipsis       : t -> bool
  val is_comma          : t -> bool
  val is_array          : t -> bool
  val is_var            : t -> bool
  val is_ampersand      : t -> bool
  val is_inout          : t -> bool


end (* Syntax_S *)
