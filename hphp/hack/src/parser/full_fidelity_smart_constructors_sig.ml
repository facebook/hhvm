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
 * This module contains a signature which can be used to describe smart
 * constructors.
  
 *)

module type SmartConstructors_S = sig
  type t (* state *)
  type r (* smart constructor return type *)

  val make_missing : t -> t * r
  val make_list : t -> r list -> t * r
  val make_end_of_file : t -> r -> t * r
  val make_script : t -> r -> t * r
  val make_qualified_name : t -> r -> t * r
  val make_simple_type_specifier : t -> r -> t * r
  val make_literal_expression : t -> r -> t * r
  val make_variable_expression : t -> r -> t * r
  val make_pipe_variable_expression : t -> r -> t * r
  val make_enum_declaration : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_enumerator : t -> r -> r -> r -> r -> t * r
  val make_alias_declaration : t -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_property_declaration : t -> r -> r -> r -> r -> t * r
  val make_property_declarator : t -> r -> r -> t * r
  val make_namespace_declaration : t -> r -> r -> r -> t * r
  val make_namespace_body : t -> r -> r -> r -> t * r
  val make_namespace_empty_body : t -> r -> t * r
  val make_namespace_use_declaration : t -> r -> r -> r -> r -> t * r
  val make_namespace_group_use_declaration : t -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_namespace_use_clause : t -> r -> r -> r -> r -> t * r
  val make_function_declaration : t -> r -> r -> r -> t * r
  val make_function_declaration_header : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_where_clause : t -> r -> r -> t * r
  val make_where_constraint : t -> r -> r -> r -> t * r
  val make_methodish_declaration : t -> r -> r -> r -> r -> t * r
  val make_classish_declaration : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_classish_body : t -> r -> r -> r -> t * r
  val make_trait_use_precedence_item : t -> r -> r -> r -> t * r
  val make_trait_use_alias_item : t -> r -> r -> r -> r -> t * r
  val make_trait_use_conflict_resolution : t -> r -> r -> r -> r -> r -> t * r
  val make_trait_use : t -> r -> r -> r -> t * r
  val make_require_clause : t -> r -> r -> r -> r -> t * r
  val make_const_declaration : t -> r -> r -> r -> r -> r -> t * r
  val make_constant_declarator : t -> r -> r -> t * r
  val make_type_const_declaration : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_decorated_expression : t -> r -> r -> t * r
  val make_parameter_declaration : t -> r -> r -> r -> r -> r -> r -> t * r
  val make_variadic_parameter : t -> r -> r -> r -> t * r
  val make_attribute_specification : t -> r -> r -> r -> t * r
  val make_attribute : t -> r -> r -> r -> r -> t * r
  val make_inclusion_expression : t -> r -> r -> t * r
  val make_inclusion_directive : t -> r -> r -> t * r
  val make_compound_statement : t -> r -> r -> r -> t * r
  val make_expression_statement : t -> r -> r -> t * r
  val make_markup_section : t -> r -> r -> r -> r -> t * r
  val make_markup_suffix : t -> r -> r -> t * r
  val make_unset_statement : t -> r -> r -> r -> r -> r -> t * r
  val make_using_statement_block_scoped : t -> r -> r -> r -> r -> r -> r -> t * r
  val make_using_statement_function_scoped : t -> r -> r -> r -> r -> t * r
  val make_declare_directive_statement : t -> r -> r -> r -> r -> r -> t * r
  val make_declare_block_statement : t -> r -> r -> r -> r -> r -> t * r
  val make_while_statement : t -> r -> r -> r -> r -> r -> t * r
  val make_if_statement : t -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_elseif_clause : t -> r -> r -> r -> r -> r -> t * r
  val make_else_clause : t -> r -> r -> t * r
  val make_if_endif_statement : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_elseif_colon_clause : t -> r -> r -> r -> r -> r -> r -> t * r
  val make_else_colon_clause : t -> r -> r -> r -> t * r
  val make_try_statement : t -> r -> r -> r -> r -> t * r
  val make_catch_clause : t -> r -> r -> r -> r -> r -> r -> t * r
  val make_finally_clause : t -> r -> r -> t * r
  val make_do_statement : t -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_for_statement : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_foreach_statement : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_switch_statement : t -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_switch_section : t -> r -> r -> r -> t * r
  val make_switch_fallthrough : t -> r -> r -> t * r
  val make_case_label : t -> r -> r -> r -> t * r
  val make_default_label : t -> r -> r -> t * r
  val make_return_statement : t -> r -> r -> r -> t * r
  val make_goto_label : t -> r -> r -> t * r
  val make_goto_statement : t -> r -> r -> r -> t * r
  val make_throw_statement : t -> r -> r -> r -> t * r
  val make_break_statement : t -> r -> r -> r -> t * r
  val make_continue_statement : t -> r -> r -> r -> t * r
  val make_function_static_statement : t -> r -> r -> r -> t * r
  val make_static_declarator : t -> r -> r -> t * r
  val make_echo_statement : t -> r -> r -> r -> t * r
  val make_global_statement : t -> r -> r -> r -> t * r
  val make_simple_initializer : t -> r -> r -> t * r
  val make_anonymous_class : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_anonymous_function : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_php7_anonymous_function : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_anonymous_function_use_clause : t -> r -> r -> r -> r -> t * r
  val make_lambda_expression : t -> r -> r -> r -> r -> r -> t * r
  val make_lambda_signature : t -> r -> r -> r -> r -> r -> t * r
  val make_cast_expression : t -> r -> r -> r -> r -> t * r
  val make_scope_resolution_expression : t -> r -> r -> r -> t * r
  val make_member_selection_expression : t -> r -> r -> r -> t * r
  val make_safe_member_selection_expression : t -> r -> r -> r -> t * r
  val make_embedded_member_selection_expression : t -> r -> r -> r -> t * r
  val make_yield_expression : t -> r -> r -> t * r
  val make_yield_from_expression : t -> r -> r -> r -> t * r
  val make_prefix_unary_expression : t -> r -> r -> t * r
  val make_postfix_unary_expression : t -> r -> r -> t * r
  val make_binary_expression : t -> r -> r -> r -> t * r
  val make_instanceof_expression : t -> r -> r -> r -> t * r
  val make_is_expression : t -> r -> r -> r -> t * r
  val make_conditional_expression : t -> r -> r -> r -> r -> r -> t * r
  val make_eval_expression : t -> r -> r -> r -> r -> t * r
  val make_empty_expression : t -> r -> r -> r -> r -> t * r
  val make_define_expression : t -> r -> r -> r -> r -> t * r
  val make_halt_compiler_expression : t -> r -> r -> r -> r -> t * r
  val make_isset_expression : t -> r -> r -> r -> r -> t * r
  val make_function_call_expression : t -> r -> r -> r -> r -> t * r
  val make_function_call_with_type_arguments_expression : t -> r -> r -> r -> r -> r -> t * r
  val make_parenthesized_expression : t -> r -> r -> r -> t * r
  val make_braced_expression : t -> r -> r -> r -> t * r
  val make_embedded_braced_expression : t -> r -> r -> r -> t * r
  val make_list_expression : t -> r -> r -> r -> r -> t * r
  val make_collection_literal_expression : t -> r -> r -> r -> r -> t * r
  val make_object_creation_expression : t -> r -> r -> t * r
  val make_constructor_call : t -> r -> r -> r -> r -> t * r
  val make_array_creation_expression : t -> r -> r -> r -> t * r
  val make_array_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_darray_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_dictionary_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_keyset_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_varray_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_vector_intrinsic_expression : t -> r -> r -> r -> r -> t * r
  val make_element_initializer : t -> r -> r -> r -> t * r
  val make_subscript_expression : t -> r -> r -> r -> r -> t * r
  val make_embedded_subscript_expression : t -> r -> r -> r -> r -> t * r
  val make_awaitable_creation_expression : t -> r -> r -> r -> t * r
  val make_xhp_children_declaration : t -> r -> r -> r -> t * r
  val make_xhp_children_parenthesized_list : t -> r -> r -> r -> t * r
  val make_xhp_category_declaration : t -> r -> r -> r -> t * r
  val make_xhp_enum_type : t -> r -> r -> r -> r -> r -> t * r
  val make_xhp_required : t -> r -> r -> t * r
  val make_xhp_class_attribute_declaration : t -> r -> r -> r -> t * r
  val make_xhp_class_attribute : t -> r -> r -> r -> r -> t * r
  val make_xhp_simple_class_attribute : t -> r -> t * r
  val make_xhp_simple_attribute : t -> r -> r -> r -> t * r
  val make_xhp_spread_attribute : t -> r -> r -> r -> r -> t * r
  val make_xhp_open : t -> r -> r -> r -> r -> t * r
  val make_xhp_expression : t -> r -> r -> r -> t * r
  val make_xhp_close : t -> r -> r -> r -> t * r
  val make_type_constant : t -> r -> r -> r -> t * r
  val make_vector_type_specifier : t -> r -> r -> r -> r -> r -> t * r
  val make_keyset_type_specifier : t -> r -> r -> r -> r -> r -> t * r
  val make_tuple_type_explicit_specifier : t -> r -> r -> r -> r -> t * r
  val make_varray_type_specifier : t -> r -> r -> r -> r -> r -> t * r
  val make_vector_array_type_specifier : t -> r -> r -> r -> r -> t * r
  val make_type_parameter : t -> r -> r -> r -> t * r
  val make_type_constraint : t -> r -> r -> t * r
  val make_darray_type_specifier : t -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_map_array_type_specifier : t -> r -> r -> r -> r -> r -> r -> t * r
  val make_dictionary_type_specifier : t -> r -> r -> r -> r -> t * r
  val make_closure_type_specifier : t -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t * r
  val make_closure_parameter_type_specifier : t -> r -> r -> t * r
  val make_classname_type_specifier : t -> r -> r -> r -> r -> r -> t * r
  val make_field_specifier : t -> r -> r -> r -> r -> t * r
  val make_field_initializer : t -> r -> r -> r -> t * r
  val make_shape_type_specifier : t -> r -> r -> r -> r -> r -> t * r
  val make_shape_expression : t -> r -> r -> r -> r -> t * r
  val make_tuple_expression : t -> r -> r -> r -> r -> t * r
  val make_generic_type_specifier : t -> r -> r -> t * r
  val make_nullable_type_specifier : t -> r -> r -> t * r
  val make_soft_type_specifier : t -> r -> r -> t * r
  val make_type_arguments : t -> r -> r -> r -> t * r
  val make_type_parameters : t -> r -> r -> r -> t * r
  val make_tuple_type_specifier : t -> r -> r -> r -> t * r
  val make_error : t -> r -> t * r
  val make_list_item : t -> r -> r -> t * r

end (* SmartConstructors_S *)
