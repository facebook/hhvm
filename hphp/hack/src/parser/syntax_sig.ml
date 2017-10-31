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
 **
 *
* This module contains a signature which can be used to describe the public
* surface area of a constructable syntax tree.
  
 *)

module type Syntax_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type t
  type end_of_file =
    { end_of_file_token                                  : t
    }
  type script =
    { script_declarations                                : t
    }
  type simple_type_specifier =
    { simple_type_specifier                              : t
    }
  type literal_expression =
    { literal_expression                                 : t
    }
  type variable_expression =
    { variable_expression                                : t
    }
  type qualified_name_expression =
    { qualified_name_expression                          : t
    }
  type pipe_variable_expression =
    { pipe_variable_expression                           : t
    }
  type enum_declaration =
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
  type enumerator =
    { enumerator_name                                    : t
    ; enumerator_equal                                   : t
    ; enumerator_value                                   : t
    ; enumerator_semicolon                               : t
    }
  type alias_declaration =
    { alias_attribute_spec                               : t
    ; alias_keyword                                      : t
    ; alias_name                                         : t
    ; alias_generic_parameter                            : t
    ; alias_constraint                                   : t
    ; alias_equal                                        : t
    ; alias_type                                         : t
    ; alias_semicolon                                    : t
    }
  type property_declaration =
    { property_modifiers                                 : t
    ; property_type                                      : t
    ; property_declarators                               : t
    ; property_semicolon                                 : t
    }
  type property_declarator =
    { property_name                                      : t
    ; property_initializer                               : t
    }
  type namespace_declaration =
    { namespace_keyword                                  : t
    ; namespace_name                                     : t
    ; namespace_body                                     : t
    }
  type namespace_body =
    { namespace_left_brace                               : t
    ; namespace_declarations                             : t
    ; namespace_right_brace                              : t
    }
  type namespace_empty_body =
    { namespace_semicolon                                : t
    }
  type namespace_use_declaration =
    { namespace_use_keyword                              : t
    ; namespace_use_kind                                 : t
    ; namespace_use_clauses                              : t
    ; namespace_use_semicolon                            : t
    }
  type namespace_group_use_declaration =
    { namespace_group_use_keyword                        : t
    ; namespace_group_use_kind                           : t
    ; namespace_group_use_prefix                         : t
    ; namespace_group_use_left_brace                     : t
    ; namespace_group_use_clauses                        : t
    ; namespace_group_use_right_brace                    : t
    ; namespace_group_use_semicolon                      : t
    }
  type namespace_use_clause =
    { namespace_use_clause_kind                          : t
    ; namespace_use_name                                 : t
    ; namespace_use_as                                   : t
    ; namespace_use_alias                                : t
    }
  type function_declaration =
    { function_attribute_spec                            : t
    ; function_declaration_header                        : t
    ; function_body                                      : t
    }
  type function_declaration_header =
    { function_async                                     : t
    ; function_coroutine                                 : t
    ; function_keyword                                   : t
    ; function_ampersand                                 : t
    ; function_name                                      : t
    ; function_type_parameter_list                       : t
    ; function_left_paren                                : t
    ; function_parameter_list                            : t
    ; function_right_paren                               : t
    ; function_colon                                     : t
    ; function_type                                      : t
    ; function_where_clause                              : t
    }
  type where_clause =
    { where_clause_keyword                               : t
    ; where_clause_constraints                           : t
    }
  type where_constraint =
    { where_constraint_left_type                         : t
    ; where_constraint_operator                          : t
    ; where_constraint_right_type                        : t
    }
  type methodish_declaration =
    { methodish_attribute                                : t
    ; methodish_modifiers                                : t
    ; methodish_function_decl_header                     : t
    ; methodish_function_body                            : t
    ; methodish_semicolon                                : t
    }
  type classish_declaration =
    { classish_attribute                                 : t
    ; classish_modifiers                                 : t
    ; classish_keyword                                   : t
    ; classish_name                                      : t
    ; classish_type_parameters                           : t
    ; classish_extends_keyword                           : t
    ; classish_extends_list                              : t
    ; classish_implements_keyword                        : t
    ; classish_implements_list                           : t
    ; classish_body                                      : t
    }
  type classish_body =
    { classish_body_left_brace                           : t
    ; classish_body_elements                             : t
    ; classish_body_right_brace                          : t
    }
  type trait_use_precedence_item =
    { trait_use_precedence_item_name                     : t
    ; trait_use_precedence_item_keyword                  : t
    ; trait_use_precedence_item_removed_names            : t
    }
  type trait_use_alias_item =
    { trait_use_alias_item_aliasing_name                 : t
    ; trait_use_alias_item_keyword                       : t
    ; trait_use_alias_item_visibility                    : t
    ; trait_use_alias_item_aliased_name                  : t
    }
  type trait_use_conflict_resolution =
    { trait_use_conflict_resolution_keyword              : t
    ; trait_use_conflict_resolution_names                : t
    ; trait_use_conflict_resolution_left_brace           : t
    ; trait_use_conflict_resolution_clauses              : t
    ; trait_use_conflict_resolution_right_brace          : t
    }
  type trait_use =
    { trait_use_keyword                                  : t
    ; trait_use_names                                    : t
    ; trait_use_semicolon                                : t
    }
  type require_clause =
    { require_keyword                                    : t
    ; require_kind                                       : t
    ; require_name                                       : t
    ; require_semicolon                                  : t
    }
  type const_declaration =
    { const_abstract                                     : t
    ; const_keyword                                      : t
    ; const_type_specifier                               : t
    ; const_declarators                                  : t
    ; const_semicolon                                    : t
    }
  type constant_declarator =
    { constant_declarator_name                           : t
    ; constant_declarator_initializer                    : t
    }
  type type_const_declaration =
    { type_const_abstract                                : t
    ; type_const_keyword                                 : t
    ; type_const_type_keyword                            : t
    ; type_const_name                                    : t
    ; type_const_type_constraint                         : t
    ; type_const_equal                                   : t
    ; type_const_type_specifier                          : t
    ; type_const_semicolon                               : t
    }
  type decorated_expression =
    { decorated_expression_decorator                     : t
    ; decorated_expression_expression                    : t
    }
  type parameter_declaration =
    { parameter_attribute                                : t
    ; parameter_visibility                               : t
    ; parameter_call_convention                          : t
    ; parameter_type                                     : t
    ; parameter_name                                     : t
    ; parameter_default_value                            : t
    }
  type variadic_parameter =
    { variadic_parameter_type                            : t
    ; variadic_parameter_ellipsis                        : t
    }
  type attribute_specification =
    { attribute_specification_left_double_angle          : t
    ; attribute_specification_attributes                 : t
    ; attribute_specification_right_double_angle         : t
    }
  type attribute =
    { attribute_name                                     : t
    ; attribute_left_paren                               : t
    ; attribute_values                                   : t
    ; attribute_right_paren                              : t
    }
  type inclusion_expression =
    { inclusion_require                                  : t
    ; inclusion_filename                                 : t
    }
  type inclusion_directive =
    { inclusion_expression                               : t
    ; inclusion_semicolon                                : t
    }
  type compound_statement =
    { compound_left_brace                                : t
    ; compound_statements                                : t
    ; compound_right_brace                               : t
    }
  type expression_statement =
    { expression_statement_expression                    : t
    ; expression_statement_semicolon                     : t
    }
  type markup_section =
    { markup_prefix                                      : t
    ; markup_text                                        : t
    ; markup_suffix                                      : t
    ; markup_expression                                  : t
    }
  type markup_suffix =
    { markup_suffix_less_than_question                   : t
    ; markup_suffix_name                                 : t
    }
  type unset_statement =
    { unset_keyword                                      : t
    ; unset_left_paren                                   : t
    ; unset_variables                                    : t
    ; unset_right_paren                                  : t
    ; unset_semicolon                                    : t
    }
  type using_statement_block_scoped =
    { using_block_await_keyword                          : t
    ; using_block_using_keyword                          : t
    ; using_block_left_paren                             : t
    ; using_block_expressions                            : t
    ; using_block_right_paren                            : t
    ; using_block_body                                   : t
    }
  type using_statement_function_scoped =
    { using_function_await_keyword                       : t
    ; using_function_using_keyword                       : t
    ; using_function_expression                          : t
    ; using_function_semicolon                           : t
    }
  type while_statement =
    { while_keyword                                      : t
    ; while_left_paren                                   : t
    ; while_condition                                    : t
    ; while_right_paren                                  : t
    ; while_body                                         : t
    }
  type if_statement =
    { if_keyword                                         : t
    ; if_left_paren                                      : t
    ; if_condition                                       : t
    ; if_right_paren                                     : t
    ; if_statement                                       : t
    ; if_elseif_clauses                                  : t
    ; if_else_clause                                     : t
    }
  type elseif_clause =
    { elseif_keyword                                     : t
    ; elseif_left_paren                                  : t
    ; elseif_condition                                   : t
    ; elseif_right_paren                                 : t
    ; elseif_statement                                   : t
    }
  type else_clause =
    { else_keyword                                       : t
    ; else_statement                                     : t
    }
  type if_endif_statement =
    { if_endif_keyword                                   : t
    ; if_endif_left_paren                                : t
    ; if_endif_condition                                 : t
    ; if_endif_right_paren                               : t
    ; if_endif_colon                                     : t
    ; if_endif_statement                                 : t
    ; if_endif_elseif_colon_clauses                      : t
    ; if_endif_else_colon_clause                         : t
    ; if_endif_endif_keyword                             : t
    ; if_endif_semicolon                                 : t
    }
  type elseif_colon_clause =
    { elseif_colon_keyword                               : t
    ; elseif_colon_left_paren                            : t
    ; elseif_colon_condition                             : t
    ; elseif_colon_right_paren                           : t
    ; elseif_colon_colon                                 : t
    ; elseif_colon_statement                             : t
    }
  type else_colon_clause =
    { else_colon_keyword                                 : t
    ; else_colon_colon                                   : t
    ; else_colon_statement                               : t
    }
  type try_statement =
    { try_keyword                                        : t
    ; try_compound_statement                             : t
    ; try_catch_clauses                                  : t
    ; try_finally_clause                                 : t
    }
  type catch_clause =
    { catch_keyword                                      : t
    ; catch_left_paren                                   : t
    ; catch_type                                         : t
    ; catch_variable                                     : t
    ; catch_right_paren                                  : t
    ; catch_body                                         : t
    }
  type finally_clause =
    { finally_keyword                                    : t
    ; finally_body                                       : t
    }
  type do_statement =
    { do_keyword                                         : t
    ; do_body                                            : t
    ; do_while_keyword                                   : t
    ; do_left_paren                                      : t
    ; do_condition                                       : t
    ; do_right_paren                                     : t
    ; do_semicolon                                       : t
    }
  type for_statement =
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
  type foreach_statement =
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
  type switch_statement =
    { switch_keyword                                     : t
    ; switch_left_paren                                  : t
    ; switch_expression                                  : t
    ; switch_right_paren                                 : t
    ; switch_left_brace                                  : t
    ; switch_sections                                    : t
    ; switch_right_brace                                 : t
    }
  type switch_section =
    { switch_section_labels                              : t
    ; switch_section_statements                          : t
    ; switch_section_fallthrough                         : t
    }
  type switch_fallthrough =
    { fallthrough_keyword                                : t
    ; fallthrough_semicolon                              : t
    }
  type case_label =
    { case_keyword                                       : t
    ; case_expression                                    : t
    ; case_colon                                         : t
    }
  type default_label =
    { default_keyword                                    : t
    ; default_colon                                      : t
    }
  type return_statement =
    { return_keyword                                     : t
    ; return_expression                                  : t
    ; return_semicolon                                   : t
    }
  type goto_label =
    { goto_label_name                                    : t
    ; goto_label_colon                                   : t
    }
  type goto_statement =
    { goto_statement_keyword                             : t
    ; goto_statement_label_name                          : t
    ; goto_statement_semicolon                           : t
    }
  type throw_statement =
    { throw_keyword                                      : t
    ; throw_expression                                   : t
    ; throw_semicolon                                    : t
    }
  type break_statement =
    { break_keyword                                      : t
    ; break_level                                        : t
    ; break_semicolon                                    : t
    }
  type continue_statement =
    { continue_keyword                                   : t
    ; continue_level                                     : t
    ; continue_semicolon                                 : t
    }
  type function_static_statement =
    { static_static_keyword                              : t
    ; static_declarations                                : t
    ; static_semicolon                                   : t
    }
  type static_declarator =
    { static_name                                        : t
    ; static_initializer                                 : t
    }
  type echo_statement =
    { echo_keyword                                       : t
    ; echo_expressions                                   : t
    ; echo_semicolon                                     : t
    }
  type global_statement =
    { global_keyword                                     : t
    ; global_variables                                   : t
    ; global_semicolon                                   : t
    }
  type simple_initializer =
    { simple_initializer_equal                           : t
    ; simple_initializer_value                           : t
    }
  type anonymous_function =
    { anonymous_static_keyword                           : t
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
  type anonymous_function_use_clause =
    { anonymous_use_keyword                              : t
    ; anonymous_use_left_paren                           : t
    ; anonymous_use_variables                            : t
    ; anonymous_use_right_paren                          : t
    }
  type lambda_expression =
    { lambda_async                                       : t
    ; lambda_coroutine                                   : t
    ; lambda_signature                                   : t
    ; lambda_arrow                                       : t
    ; lambda_body                                        : t
    }
  type lambda_signature =
    { lambda_left_paren                                  : t
    ; lambda_parameters                                  : t
    ; lambda_right_paren                                 : t
    ; lambda_colon                                       : t
    ; lambda_type                                        : t
    }
  type cast_expression =
    { cast_left_paren                                    : t
    ; cast_type                                          : t
    ; cast_right_paren                                   : t
    ; cast_operand                                       : t
    }
  type scope_resolution_expression =
    { scope_resolution_qualifier                         : t
    ; scope_resolution_operator                          : t
    ; scope_resolution_name                              : t
    }
  type member_selection_expression =
    { member_object                                      : t
    ; member_operator                                    : t
    ; member_name                                        : t
    }
  type safe_member_selection_expression =
    { safe_member_object                                 : t
    ; safe_member_operator                               : t
    ; safe_member_name                                   : t
    }
  type embedded_member_selection_expression =
    { embedded_member_object                             : t
    ; embedded_member_operator                           : t
    ; embedded_member_name                               : t
    }
  type yield_expression =
    { yield_keyword                                      : t
    ; yield_operand                                      : t
    }
  type yield_from_expression =
    { yield_from_yield_keyword                           : t
    ; yield_from_from_keyword                            : t
    ; yield_from_operand                                 : t
    }
  type prefix_unary_expression =
    { prefix_unary_operator                              : t
    ; prefix_unary_operand                               : t
    }
  type postfix_unary_expression =
    { postfix_unary_operand                              : t
    ; postfix_unary_operator                             : t
    }
  type binary_expression =
    { binary_left_operand                                : t
    ; binary_operator                                    : t
    ; binary_right_operand                               : t
    }
  type instanceof_expression =
    { instanceof_left_operand                            : t
    ; instanceof_operator                                : t
    ; instanceof_right_operand                           : t
    }
  type is_expression =
    { is_left_operand                                    : t
    ; is_operator                                        : t
    ; is_right_operand                                   : t
    }
  type conditional_expression =
    { conditional_test                                   : t
    ; conditional_question                               : t
    ; conditional_consequence                            : t
    ; conditional_colon                                  : t
    ; conditional_alternative                            : t
    }
  type eval_expression =
    { eval_keyword                                       : t
    ; eval_left_paren                                    : t
    ; eval_argument                                      : t
    ; eval_right_paren                                   : t
    }
  type empty_expression =
    { empty_keyword                                      : t
    ; empty_left_paren                                   : t
    ; empty_argument                                     : t
    ; empty_right_paren                                  : t
    }
  type define_expression =
    { define_keyword                                     : t
    ; define_left_paren                                  : t
    ; define_argument_list                               : t
    ; define_right_paren                                 : t
    }
  type isset_expression =
    { isset_keyword                                      : t
    ; isset_left_paren                                   : t
    ; isset_argument_list                                : t
    ; isset_right_paren                                  : t
    }
  type function_call_expression =
    { function_call_receiver                             : t
    ; function_call_left_paren                           : t
    ; function_call_argument_list                        : t
    ; function_call_right_paren                          : t
    }
  type function_call_with_type_arguments_expression =
    { function_call_with_type_arguments_receiver         : t
    ; function_call_with_type_arguments_type_args        : t
    ; function_call_with_type_arguments_left_paren       : t
    ; function_call_with_type_arguments_argument_list    : t
    ; function_call_with_type_arguments_right_paren      : t
    }
  type parenthesized_expression =
    { parenthesized_expression_left_paren                : t
    ; parenthesized_expression_expression                : t
    ; parenthesized_expression_right_paren               : t
    }
  type braced_expression =
    { braced_expression_left_brace                       : t
    ; braced_expression_expression                       : t
    ; braced_expression_right_brace                      : t
    }
  type embedded_braced_expression =
    { embedded_braced_expression_left_brace              : t
    ; embedded_braced_expression_expression              : t
    ; embedded_braced_expression_right_brace             : t
    }
  type list_expression =
    { list_keyword                                       : t
    ; list_left_paren                                    : t
    ; list_members                                       : t
    ; list_right_paren                                   : t
    }
  type collection_literal_expression =
    { collection_literal_name                            : t
    ; collection_literal_left_brace                      : t
    ; collection_literal_initializers                    : t
    ; collection_literal_right_brace                     : t
    }
  type object_creation_expression =
    { object_creation_new_keyword                        : t
    ; object_creation_type                               : t
    ; object_creation_left_paren                         : t
    ; object_creation_argument_list                      : t
    ; object_creation_right_paren                        : t
    }
  type array_creation_expression =
    { array_creation_left_bracket                        : t
    ; array_creation_members                             : t
    ; array_creation_right_bracket                       : t
    }
  type array_intrinsic_expression =
    { array_intrinsic_keyword                            : t
    ; array_intrinsic_left_paren                         : t
    ; array_intrinsic_members                            : t
    ; array_intrinsic_right_paren                        : t
    }
  type darray_intrinsic_expression =
    { darray_intrinsic_keyword                           : t
    ; darray_intrinsic_left_bracket                      : t
    ; darray_intrinsic_members                           : t
    ; darray_intrinsic_right_bracket                     : t
    }
  type dictionary_intrinsic_expression =
    { dictionary_intrinsic_keyword                       : t
    ; dictionary_intrinsic_left_bracket                  : t
    ; dictionary_intrinsic_members                       : t
    ; dictionary_intrinsic_right_bracket                 : t
    }
  type keyset_intrinsic_expression =
    { keyset_intrinsic_keyword                           : t
    ; keyset_intrinsic_left_bracket                      : t
    ; keyset_intrinsic_members                           : t
    ; keyset_intrinsic_right_bracket                     : t
    }
  type varray_intrinsic_expression =
    { varray_intrinsic_keyword                           : t
    ; varray_intrinsic_left_bracket                      : t
    ; varray_intrinsic_members                           : t
    ; varray_intrinsic_right_bracket                     : t
    }
  type vector_intrinsic_expression =
    { vector_intrinsic_keyword                           : t
    ; vector_intrinsic_left_bracket                      : t
    ; vector_intrinsic_members                           : t
    ; vector_intrinsic_right_bracket                     : t
    }
  type element_initializer =
    { element_key                                        : t
    ; element_arrow                                      : t
    ; element_value                                      : t
    }
  type subscript_expression =
    { subscript_receiver                                 : t
    ; subscript_left_bracket                             : t
    ; subscript_index                                    : t
    ; subscript_right_bracket                            : t
    }
  type embedded_subscript_expression =
    { embedded_subscript_receiver                        : t
    ; embedded_subscript_left_bracket                    : t
    ; embedded_subscript_index                           : t
    ; embedded_subscript_right_bracket                   : t
    }
  type awaitable_creation_expression =
    { awaitable_async                                    : t
    ; awaitable_coroutine                                : t
    ; awaitable_compound_statement                       : t
    }
  type xhp_children_declaration =
    { xhp_children_keyword                               : t
    ; xhp_children_expression                            : t
    ; xhp_children_semicolon                             : t
    }
  type xhp_children_parenthesized_list =
    { xhp_children_list_left_paren                       : t
    ; xhp_children_list_xhp_children                     : t
    ; xhp_children_list_right_paren                      : t
    }
  type xhp_category_declaration =
    { xhp_category_keyword                               : t
    ; xhp_category_categories                            : t
    ; xhp_category_semicolon                             : t
    }
  type xhp_enum_type =
    { xhp_enum_keyword                                   : t
    ; xhp_enum_left_brace                                : t
    ; xhp_enum_values                                    : t
    ; xhp_enum_right_brace                               : t
    }
  type xhp_required =
    { xhp_required_at                                    : t
    ; xhp_required_keyword                               : t
    }
  type xhp_class_attribute_declaration =
    { xhp_attribute_keyword                              : t
    ; xhp_attribute_attributes                           : t
    ; xhp_attribute_semicolon                            : t
    }
  type xhp_class_attribute =
    { xhp_attribute_decl_type                            : t
    ; xhp_attribute_decl_name                            : t
    ; xhp_attribute_decl_initializer                     : t
    ; xhp_attribute_decl_required                        : t
    }
  type xhp_simple_class_attribute =
    { xhp_simple_class_attribute_type                    : t
    }
  type xhp_attribute =
    { xhp_attribute_name                                 : t
    ; xhp_attribute_equal                                : t
    ; xhp_attribute_expression                           : t
    }
  type xhp_open =
    { xhp_open_left_angle                                : t
    ; xhp_open_name                                      : t
    ; xhp_open_attributes                                : t
    ; xhp_open_right_angle                               : t
    }
  type xhp_expression =
    { xhp_open                                           : t
    ; xhp_body                                           : t
    ; xhp_close                                          : t
    }
  type xhp_close =
    { xhp_close_left_angle                               : t
    ; xhp_close_name                                     : t
    ; xhp_close_right_angle                              : t
    }
  type type_constant =
    { type_constant_left_type                            : t
    ; type_constant_separator                            : t
    ; type_constant_right_type                           : t
    }
  type vector_type_specifier =
    { vector_type_keyword                                : t
    ; vector_type_left_angle                             : t
    ; vector_type_type                                   : t
    ; vector_type_trailing_comma                         : t
    ; vector_type_right_angle                            : t
    }
  type keyset_type_specifier =
    { keyset_type_keyword                                : t
    ; keyset_type_left_angle                             : t
    ; keyset_type_type                                   : t
    ; keyset_type_trailing_comma                         : t
    ; keyset_type_right_angle                            : t
    }
  type tuple_type_explicit_specifier =
    { tuple_type_keyword                                 : t
    ; tuple_type_left_angle                              : t
    ; tuple_type_types                                   : t
    ; tuple_type_right_angle                             : t
    }
  type varray_type_specifier =
    { varray_keyword                                     : t
    ; varray_left_angle                                  : t
    ; varray_type                                        : t
    ; varray_trailing_comma                              : t
    ; varray_right_angle                                 : t
    }
  type vector_array_type_specifier =
    { vector_array_keyword                               : t
    ; vector_array_left_angle                            : t
    ; vector_array_type                                  : t
    ; vector_array_right_angle                           : t
    }
  type type_parameter =
    { type_variance                                      : t
    ; type_name                                          : t
    ; type_constraints                                   : t
    }
  type type_constraint =
    { constraint_keyword                                 : t
    ; constraint_type                                    : t
    }
  type darray_type_specifier =
    { darray_keyword                                     : t
    ; darray_left_angle                                  : t
    ; darray_key                                         : t
    ; darray_comma                                       : t
    ; darray_value                                       : t
    ; darray_trailing_comma                              : t
    ; darray_right_angle                                 : t
    }
  type map_array_type_specifier =
    { map_array_keyword                                  : t
    ; map_array_left_angle                               : t
    ; map_array_key                                      : t
    ; map_array_comma                                    : t
    ; map_array_value                                    : t
    ; map_array_right_angle                              : t
    }
  type dictionary_type_specifier =
    { dictionary_type_keyword                            : t
    ; dictionary_type_left_angle                         : t
    ; dictionary_type_members                            : t
    ; dictionary_type_right_angle                        : t
    }
  type closure_type_specifier =
    { closure_outer_left_paren                           : t
    ; closure_coroutine                                  : t
    ; closure_function_keyword                           : t
    ; closure_inner_left_paren                           : t
    ; closure_parameter_types                            : t
    ; closure_inner_right_paren                          : t
    ; closure_colon                                      : t
    ; closure_return_type                                : t
    ; closure_outer_right_paren                          : t
    }
  type classname_type_specifier =
    { classname_keyword                                  : t
    ; classname_left_angle                               : t
    ; classname_type                                     : t
    ; classname_trailing_comma                           : t
    ; classname_right_angle                              : t
    }
  type field_specifier =
    { field_question                                     : t
    ; field_name                                         : t
    ; field_arrow                                        : t
    ; field_type                                         : t
    }
  type field_initializer =
    { field_initializer_name                             : t
    ; field_initializer_arrow                            : t
    ; field_initializer_value                            : t
    }
  type shape_type_specifier =
    { shape_type_keyword                                 : t
    ; shape_type_left_paren                              : t
    ; shape_type_fields                                  : t
    ; shape_type_ellipsis                                : t
    ; shape_type_right_paren                             : t
    }
  type shape_expression =
    { shape_expression_keyword                           : t
    ; shape_expression_left_paren                        : t
    ; shape_expression_fields                            : t
    ; shape_expression_right_paren                       : t
    }
  type tuple_expression =
    { tuple_expression_keyword                           : t
    ; tuple_expression_left_paren                        : t
    ; tuple_expression_items                             : t
    ; tuple_expression_right_paren                       : t
    }
  type generic_type_specifier =
    { generic_class_type                                 : t
    ; generic_argument_list                              : t
    }
  type nullable_type_specifier =
    { nullable_question                                  : t
    ; nullable_type                                      : t
    }
  type soft_type_specifier =
    { soft_at                                            : t
    ; soft_type                                          : t
    }
  type type_arguments =
    { type_arguments_left_angle                          : t
    ; type_arguments_types                               : t
    ; type_arguments_right_angle                         : t
    }
  type type_parameters =
    { type_parameters_left_angle                         : t
    ; type_parameters_parameters                         : t
    ; type_parameters_right_angle                        : t
    }
  type tuple_type_specifier =
    { tuple_left_paren                                   : t
    ; tuple_types                                        : t
    ; tuple_right_paren                                  : t
    }
  type error =
    { error_error                                        : t
    }
  type list_item =
    { list_item                                          : t
    ; list_separator                                     : t
    }

  type syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
  | EndOfFile                               of end_of_file
  | Script                                  of script
  | SimpleTypeSpecifier                     of simple_type_specifier
  | LiteralExpression                       of literal_expression
  | VariableExpression                      of variable_expression
  | QualifiedNameExpression                 of qualified_name_expression
  | PipeVariableExpression                  of pipe_variable_expression
  | EnumDeclaration                         of enum_declaration
  | Enumerator                              of enumerator
  | AliasDeclaration                        of alias_declaration
  | PropertyDeclaration                     of property_declaration
  | PropertyDeclarator                      of property_declarator
  | NamespaceDeclaration                    of namespace_declaration
  | NamespaceBody                           of namespace_body
  | NamespaceEmptyBody                      of namespace_empty_body
  | NamespaceUseDeclaration                 of namespace_use_declaration
  | NamespaceGroupUseDeclaration            of namespace_group_use_declaration
  | NamespaceUseClause                      of namespace_use_clause
  | FunctionDeclaration                     of function_declaration
  | FunctionDeclarationHeader               of function_declaration_header
  | WhereClause                             of where_clause
  | WhereConstraint                         of where_constraint
  | MethodishDeclaration                    of methodish_declaration
  | ClassishDeclaration                     of classish_declaration
  | ClassishBody                            of classish_body
  | TraitUsePrecedenceItem                  of trait_use_precedence_item
  | TraitUseAliasItem                       of trait_use_alias_item
  | TraitUseConflictResolution              of trait_use_conflict_resolution
  | TraitUse                                of trait_use
  | RequireClause                           of require_clause
  | ConstDeclaration                        of const_declaration
  | ConstantDeclarator                      of constant_declarator
  | TypeConstDeclaration                    of type_const_declaration
  | DecoratedExpression                     of decorated_expression
  | ParameterDeclaration                    of parameter_declaration
  | VariadicParameter                       of variadic_parameter
  | AttributeSpecification                  of attribute_specification
  | Attribute                               of attribute
  | InclusionExpression                     of inclusion_expression
  | InclusionDirective                      of inclusion_directive
  | CompoundStatement                       of compound_statement
  | ExpressionStatement                     of expression_statement
  | MarkupSection                           of markup_section
  | MarkupSuffix                            of markup_suffix
  | UnsetStatement                          of unset_statement
  | UsingStatementBlockScoped               of using_statement_block_scoped
  | UsingStatementFunctionScoped            of using_statement_function_scoped
  | WhileStatement                          of while_statement
  | IfStatement                             of if_statement
  | ElseifClause                            of elseif_clause
  | ElseClause                              of else_clause
  | IfEndIfStatement                        of if_endif_statement
  | ElseifColonClause                       of elseif_colon_clause
  | ElseColonClause                         of else_colon_clause
  | TryStatement                            of try_statement
  | CatchClause                             of catch_clause
  | FinallyClause                           of finally_clause
  | DoStatement                             of do_statement
  | ForStatement                            of for_statement
  | ForeachStatement                        of foreach_statement
  | SwitchStatement                         of switch_statement
  | SwitchSection                           of switch_section
  | SwitchFallthrough                       of switch_fallthrough
  | CaseLabel                               of case_label
  | DefaultLabel                            of default_label
  | ReturnStatement                         of return_statement
  | GotoLabel                               of goto_label
  | GotoStatement                           of goto_statement
  | ThrowStatement                          of throw_statement
  | BreakStatement                          of break_statement
  | ContinueStatement                       of continue_statement
  | FunctionStaticStatement                 of function_static_statement
  | StaticDeclarator                        of static_declarator
  | EchoStatement                           of echo_statement
  | GlobalStatement                         of global_statement
  | SimpleInitializer                       of simple_initializer
  | AnonymousFunction                       of anonymous_function
  | AnonymousFunctionUseClause              of anonymous_function_use_clause
  | LambdaExpression                        of lambda_expression
  | LambdaSignature                         of lambda_signature
  | CastExpression                          of cast_expression
  | ScopeResolutionExpression               of scope_resolution_expression
  | MemberSelectionExpression               of member_selection_expression
  | SafeMemberSelectionExpression           of safe_member_selection_expression
  | EmbeddedMemberSelectionExpression       of embedded_member_selection_expression
  | YieldExpression                         of yield_expression
  | YieldFromExpression                     of yield_from_expression
  | PrefixUnaryExpression                   of prefix_unary_expression
  | PostfixUnaryExpression                  of postfix_unary_expression
  | BinaryExpression                        of binary_expression
  | InstanceofExpression                    of instanceof_expression
  | IsExpression                            of is_expression
  | ConditionalExpression                   of conditional_expression
  | EvalExpression                          of eval_expression
  | EmptyExpression                         of empty_expression
  | DefineExpression                        of define_expression
  | IssetExpression                         of isset_expression
  | FunctionCallExpression                  of function_call_expression
  | FunctionCallWithTypeArgumentsExpression of function_call_with_type_arguments_expression
  | ParenthesizedExpression                 of parenthesized_expression
  | BracedExpression                        of braced_expression
  | EmbeddedBracedExpression                of embedded_braced_expression
  | ListExpression                          of list_expression
  | CollectionLiteralExpression             of collection_literal_expression
  | ObjectCreationExpression                of object_creation_expression
  | ArrayCreationExpression                 of array_creation_expression
  | ArrayIntrinsicExpression                of array_intrinsic_expression
  | DarrayIntrinsicExpression               of darray_intrinsic_expression
  | DictionaryIntrinsicExpression           of dictionary_intrinsic_expression
  | KeysetIntrinsicExpression               of keyset_intrinsic_expression
  | VarrayIntrinsicExpression               of varray_intrinsic_expression
  | VectorIntrinsicExpression               of vector_intrinsic_expression
  | ElementInitializer                      of element_initializer
  | SubscriptExpression                     of subscript_expression
  | EmbeddedSubscriptExpression             of embedded_subscript_expression
  | AwaitableCreationExpression             of awaitable_creation_expression
  | XHPChildrenDeclaration                  of xhp_children_declaration
  | XHPChildrenParenthesizedList            of xhp_children_parenthesized_list
  | XHPCategoryDeclaration                  of xhp_category_declaration
  | XHPEnumType                             of xhp_enum_type
  | XHPRequired                             of xhp_required
  | XHPClassAttributeDeclaration            of xhp_class_attribute_declaration
  | XHPClassAttribute                       of xhp_class_attribute
  | XHPSimpleClassAttribute                 of xhp_simple_class_attribute
  | XHPAttribute                            of xhp_attribute
  | XHPOpen                                 of xhp_open
  | XHPExpression                           of xhp_expression
  | XHPClose                                of xhp_close
  | TypeConstant                            of type_constant
  | VectorTypeSpecifier                     of vector_type_specifier
  | KeysetTypeSpecifier                     of keyset_type_specifier
  | TupleTypeExplicitSpecifier              of tuple_type_explicit_specifier
  | VarrayTypeSpecifier                     of varray_type_specifier
  | VectorArrayTypeSpecifier                of vector_array_type_specifier
  | TypeParameter                           of type_parameter
  | TypeConstraint                          of type_constraint
  | DarrayTypeSpecifier                     of darray_type_specifier
  | MapArrayTypeSpecifier                   of map_array_type_specifier
  | DictionaryTypeSpecifier                 of dictionary_type_specifier
  | ClosureTypeSpecifier                    of closure_type_specifier
  | ClassnameTypeSpecifier                  of classname_type_specifier
  | FieldSpecifier                          of field_specifier
  | FieldInitializer                        of field_initializer
  | ShapeTypeSpecifier                      of shape_type_specifier
  | ShapeExpression                         of shape_expression
  | TupleExpression                         of tuple_expression
  | GenericTypeSpecifier                    of generic_type_specifier
  | NullableTypeSpecifier                   of nullable_type_specifier
  | SoftTypeSpecifier                       of soft_type_specifier
  | TypeArguments                           of type_arguments
  | TypeParameters                          of type_parameters
  | TupleTypeSpecifier                      of tuple_type_specifier
  | ErrorSyntax                             of error
  | ListItem                                of list_item


  val syntax_node_to_list : t -> t list
  val full_width : t -> int
  val trailing_width : t -> int
  val leading_width : t -> int
  val leading_token : t -> Token.t option
  val children : t -> t list
  val syntax : t -> syntax
  val kind : t -> Full_fidelity_syntax_kind.t
  val make_token : Token.t -> t
  val get_token : t -> Token.t option
  val make_missing : unit -> t
  val make_list : t list -> t
  val make_end_of_file : t -> t
  val make_script : t -> t
  val make_simple_type_specifier : t -> t
  val make_literal_expression : t -> t
  val make_variable_expression : t -> t
  val make_qualified_name_expression : t -> t
  val make_pipe_variable_expression : t -> t
  val make_enum_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_enumerator : t -> t -> t -> t -> t
  val make_alias_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_property_declaration : t -> t -> t -> t -> t
  val make_property_declarator : t -> t -> t
  val make_namespace_declaration : t -> t -> t -> t
  val make_namespace_body : t -> t -> t -> t
  val make_namespace_empty_body : t -> t
  val make_namespace_use_declaration : t -> t -> t -> t -> t
  val make_namespace_group_use_declaration : t -> t -> t -> t -> t -> t -> t -> t
  val make_namespace_use_clause : t -> t -> t -> t -> t
  val make_function_declaration : t -> t -> t -> t
  val make_function_declaration_header : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_where_clause : t -> t -> t
  val make_where_constraint : t -> t -> t -> t
  val make_methodish_declaration : t -> t -> t -> t -> t -> t
  val make_classish_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_classish_body : t -> t -> t -> t
  val make_trait_use_precedence_item : t -> t -> t -> t
  val make_trait_use_alias_item : t -> t -> t -> t -> t
  val make_trait_use_conflict_resolution : t -> t -> t -> t -> t -> t
  val make_trait_use : t -> t -> t -> t
  val make_require_clause : t -> t -> t -> t -> t
  val make_const_declaration : t -> t -> t -> t -> t -> t
  val make_constant_declarator : t -> t -> t
  val make_type_const_declaration : t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_decorated_expression : t -> t -> t
  val make_parameter_declaration : t -> t -> t -> t -> t -> t -> t
  val make_variadic_parameter : t -> t -> t
  val make_attribute_specification : t -> t -> t -> t
  val make_attribute : t -> t -> t -> t -> t
  val make_inclusion_expression : t -> t -> t
  val make_inclusion_directive : t -> t -> t
  val make_compound_statement : t -> t -> t -> t
  val make_expression_statement : t -> t -> t
  val make_markup_section : t -> t -> t -> t -> t
  val make_markup_suffix : t -> t -> t
  val make_unset_statement : t -> t -> t -> t -> t -> t
  val make_using_statement_block_scoped : t -> t -> t -> t -> t -> t -> t
  val make_using_statement_function_scoped : t -> t -> t -> t -> t
  val make_while_statement : t -> t -> t -> t -> t -> t
  val make_if_statement : t -> t -> t -> t -> t -> t -> t -> t
  val make_elseif_clause : t -> t -> t -> t -> t -> t
  val make_else_clause : t -> t -> t
  val make_if_endif_statement : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_elseif_colon_clause : t -> t -> t -> t -> t -> t -> t
  val make_else_colon_clause : t -> t -> t -> t
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
  val make_function_static_statement : t -> t -> t -> t
  val make_static_declarator : t -> t -> t
  val make_echo_statement : t -> t -> t -> t
  val make_global_statement : t -> t -> t -> t
  val make_simple_initializer : t -> t -> t
  val make_anonymous_function : t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_anonymous_function_use_clause : t -> t -> t -> t -> t
  val make_lambda_expression : t -> t -> t -> t -> t -> t
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
  val make_conditional_expression : t -> t -> t -> t -> t -> t
  val make_eval_expression : t -> t -> t -> t -> t
  val make_empty_expression : t -> t -> t -> t -> t
  val make_define_expression : t -> t -> t -> t -> t
  val make_isset_expression : t -> t -> t -> t -> t
  val make_function_call_expression : t -> t -> t -> t -> t
  val make_function_call_with_type_arguments_expression : t -> t -> t -> t -> t -> t
  val make_parenthesized_expression : t -> t -> t -> t
  val make_braced_expression : t -> t -> t -> t
  val make_embedded_braced_expression : t -> t -> t -> t
  val make_list_expression : t -> t -> t -> t -> t
  val make_collection_literal_expression : t -> t -> t -> t -> t
  val make_object_creation_expression : t -> t -> t -> t -> t -> t
  val make_array_creation_expression : t -> t -> t -> t
  val make_array_intrinsic_expression : t -> t -> t -> t -> t
  val make_darray_intrinsic_expression : t -> t -> t -> t -> t
  val make_dictionary_intrinsic_expression : t -> t -> t -> t -> t
  val make_keyset_intrinsic_expression : t -> t -> t -> t -> t
  val make_varray_intrinsic_expression : t -> t -> t -> t -> t
  val make_vector_intrinsic_expression : t -> t -> t -> t -> t
  val make_element_initializer : t -> t -> t -> t
  val make_subscript_expression : t -> t -> t -> t -> t
  val make_embedded_subscript_expression : t -> t -> t -> t -> t
  val make_awaitable_creation_expression : t -> t -> t -> t
  val make_xhp_children_declaration : t -> t -> t -> t
  val make_xhp_children_parenthesized_list : t -> t -> t -> t
  val make_xhp_category_declaration : t -> t -> t -> t
  val make_xhp_enum_type : t -> t -> t -> t -> t
  val make_xhp_required : t -> t -> t
  val make_xhp_class_attribute_declaration : t -> t -> t -> t
  val make_xhp_class_attribute : t -> t -> t -> t -> t
  val make_xhp_simple_class_attribute : t -> t
  val make_xhp_attribute : t -> t -> t -> t
  val make_xhp_open : t -> t -> t -> t -> t
  val make_xhp_expression : t -> t -> t -> t
  val make_xhp_close : t -> t -> t -> t
  val make_type_constant : t -> t -> t -> t
  val make_vector_type_specifier : t -> t -> t -> t -> t -> t
  val make_keyset_type_specifier : t -> t -> t -> t -> t -> t
  val make_tuple_type_explicit_specifier : t -> t -> t -> t -> t
  val make_varray_type_specifier : t -> t -> t -> t -> t -> t
  val make_vector_array_type_specifier : t -> t -> t -> t -> t
  val make_type_parameter : t -> t -> t -> t
  val make_type_constraint : t -> t -> t
  val make_darray_type_specifier : t -> t -> t -> t -> t -> t -> t -> t
  val make_map_array_type_specifier : t -> t -> t -> t -> t -> t -> t
  val make_dictionary_type_specifier : t -> t -> t -> t -> t
  val make_closure_type_specifier : t -> t -> t -> t -> t -> t -> t -> t -> t -> t
  val make_classname_type_specifier : t -> t -> t -> t -> t -> t
  val make_field_specifier : t -> t -> t -> t -> t
  val make_field_initializer : t -> t -> t -> t
  val make_shape_type_specifier : t -> t -> t -> t -> t -> t
  val make_shape_expression : t -> t -> t -> t -> t
  val make_tuple_expression : t -> t -> t -> t -> t
  val make_generic_type_specifier : t -> t -> t
  val make_nullable_type_specifier : t -> t -> t
  val make_soft_type_specifier : t -> t -> t
  val make_type_arguments : t -> t -> t -> t
  val make_type_parameters : t -> t -> t -> t
  val make_tuple_type_specifier : t -> t -> t -> t
  val make_error : t -> t
  val make_list_item : t -> t -> t


  val is_abstract : t -> bool
  val is_missing : t -> bool
  val is_list : t -> bool
  val is_end_of_file : t -> bool
  val is_script : t -> bool
  val is_simple_type_specifier : t -> bool
  val is_literal_expression : t -> bool
  val is_variable_expression : t -> bool
  val is_qualified_name_expression : t -> bool
  val is_pipe_variable_expression : t -> bool
  val is_enum_declaration : t -> bool
  val is_enumerator : t -> bool
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
  val is_attribute : t -> bool
  val is_inclusion_expression : t -> bool
  val is_inclusion_directive : t -> bool
  val is_compound_statement : t -> bool
  val is_expression_statement : t -> bool
  val is_markup_section : t -> bool
  val is_markup_suffix : t -> bool
  val is_unset_statement : t -> bool
  val is_using_statement_block_scoped : t -> bool
  val is_using_statement_function_scoped : t -> bool
  val is_while_statement : t -> bool
  val is_if_statement : t -> bool
  val is_elseif_clause : t -> bool
  val is_else_clause : t -> bool
  val is_if_endif_statement : t -> bool
  val is_elseif_colon_clause : t -> bool
  val is_else_colon_clause : t -> bool
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
  val is_function_static_statement : t -> bool
  val is_static_declarator : t -> bool
  val is_echo_statement : t -> bool
  val is_global_statement : t -> bool
  val is_simple_initializer : t -> bool
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
  val is_conditional_expression : t -> bool
  val is_eval_expression : t -> bool
  val is_empty_expression : t -> bool
  val is_define_expression : t -> bool
  val is_isset_expression : t -> bool
  val is_function_call_expression : t -> bool
  val is_function_call_with_type_arguments_expression : t -> bool
  val is_parenthesized_expression : t -> bool
  val is_braced_expression : t -> bool
  val is_embedded_braced_expression : t -> bool
  val is_list_expression : t -> bool
  val is_collection_literal_expression : t -> bool
  val is_object_creation_expression : t -> bool
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
  val is_xhp_required : t -> bool
  val is_xhp_class_attribute_declaration : t -> bool
  val is_xhp_class_attribute : t -> bool
  val is_xhp_simple_class_attribute : t -> bool
  val is_xhp_attribute : t -> bool
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
  val is_classname_type_specifier : t -> bool
  val is_field_specifier : t -> bool
  val is_field_initializer : t -> bool
  val is_shape_type_specifier : t -> bool
  val is_shape_expression : t -> bool
  val is_tuple_expression : t -> bool
  val is_generic_type_specifier : t -> bool
  val is_nullable_type_specifier : t -> bool
  val is_soft_type_specifier : t -> bool
  val is_type_arguments : t -> bool
  val is_type_parameters : t -> bool
  val is_tuple_type_specifier : t -> bool
  val is_error : t -> bool
  val is_list_item : t -> bool


end (* Syntax_S *)
