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
  type token
  type t (* state *)
  type r (* smart constructor return type *)

  val initial_state : unit -> t
  val make_token : token -> t -> t * r
  val make_missing : Full_fidelity_source_text.t -> int -> t -> t * r
  val make_list : Full_fidelity_source_text.t -> int -> r list -> t -> t * r
  val make_end_of_file : r -> t -> t * r
  val make_script : r -> t -> t * r
  val make_qualified_name : r -> t -> t * r
  val make_simple_type_specifier : r -> t -> t * r
  val make_literal_expression : r -> t -> t * r
  val make_variable_expression : r -> t -> t * r
  val make_pipe_variable_expression : r -> t -> t * r
  val make_enum_declaration : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_enumerator : r -> r -> r -> r -> t -> t * r
  val make_alias_declaration : r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_property_declaration : r -> r -> r -> r -> t -> t * r
  val make_property_declarator : r -> r -> t -> t * r
  val make_namespace_declaration : r -> r -> r -> t -> t * r
  val make_namespace_body : r -> r -> r -> t -> t * r
  val make_namespace_empty_body : r -> t -> t * r
  val make_namespace_use_declaration : r -> r -> r -> r -> t -> t * r
  val make_namespace_group_use_declaration : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_namespace_use_clause : r -> r -> r -> r -> t -> t * r
  val make_function_declaration : r -> r -> r -> t -> t * r
  val make_function_declaration_header : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_where_clause : r -> r -> t -> t * r
  val make_where_constraint : r -> r -> r -> t -> t * r
  val make_methodish_declaration : r -> r -> r -> r -> t -> t * r
  val make_classish_declaration : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_classish_body : r -> r -> r -> t -> t * r
  val make_trait_use_precedence_item : r -> r -> r -> t -> t * r
  val make_trait_use_alias_item : r -> r -> r -> r -> t -> t * r
  val make_trait_use_conflict_resolution : r -> r -> r -> r -> r -> t -> t * r
  val make_trait_use : r -> r -> r -> t -> t * r
  val make_require_clause : r -> r -> r -> r -> t -> t * r
  val make_const_declaration : r -> r -> r -> r -> r -> t -> t * r
  val make_constant_declarator : r -> r -> t -> t * r
  val make_type_const_declaration : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_decorated_expression : r -> r -> t -> t * r
  val make_parameter_declaration : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_variadic_parameter : r -> r -> r -> t -> t * r
  val make_attribute_specification : r -> r -> r -> t -> t * r
  val make_attribute : r -> r -> r -> r -> t -> t * r
  val make_inclusion_expression : r -> r -> t -> t * r
  val make_inclusion_directive : r -> r -> t -> t * r
  val make_compound_statement : r -> r -> r -> t -> t * r
  val make_expression_statement : r -> r -> t -> t * r
  val make_markup_section : r -> r -> r -> r -> t -> t * r
  val make_markup_suffix : r -> r -> t -> t * r
  val make_unset_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_using_statement_block_scoped : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_using_statement_function_scoped : r -> r -> r -> r -> t -> t * r
  val make_declare_directive_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_declare_block_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_while_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_if_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_elseif_clause : r -> r -> r -> r -> r -> t -> t * r
  val make_else_clause : r -> r -> t -> t * r
  val make_if_endif_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_elseif_colon_clause : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_else_colon_clause : r -> r -> r -> t -> t * r
  val make_try_statement : r -> r -> r -> r -> t -> t * r
  val make_catch_clause : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_finally_clause : r -> r -> t -> t * r
  val make_do_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_for_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_foreach_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_switch_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_switch_section : r -> r -> r -> t -> t * r
  val make_switch_fallthrough : r -> r -> t -> t * r
  val make_case_label : r -> r -> r -> t -> t * r
  val make_default_label : r -> r -> t -> t * r
  val make_return_statement : r -> r -> r -> t -> t * r
  val make_goto_label : r -> r -> t -> t * r
  val make_goto_statement : r -> r -> r -> t -> t * r
  val make_throw_statement : r -> r -> r -> t -> t * r
  val make_break_statement : r -> r -> r -> t -> t * r
  val make_continue_statement : r -> r -> r -> t -> t * r
  val make_function_static_statement : r -> r -> r -> t -> t * r
  val make_static_declarator : r -> r -> t -> t * r
  val make_echo_statement : r -> r -> r -> t -> t * r
  val make_global_statement : r -> r -> r -> t -> t * r
  val make_simple_initializer : r -> r -> t -> t * r
  val make_anonymous_class : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_anonymous_function : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_php7_anonymous_function : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_anonymous_function_use_clause : r -> r -> r -> r -> t -> t * r
  val make_lambda_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_lambda_signature : r -> r -> r -> r -> r -> t -> t * r
  val make_cast_expression : r -> r -> r -> r -> t -> t * r
  val make_scope_resolution_expression : r -> r -> r -> t -> t * r
  val make_member_selection_expression : r -> r -> r -> t -> t * r
  val make_safe_member_selection_expression : r -> r -> r -> t -> t * r
  val make_embedded_member_selection_expression : r -> r -> r -> t -> t * r
  val make_yield_expression : r -> r -> t -> t * r
  val make_yield_from_expression : r -> r -> r -> t -> t * r
  val make_prefix_unary_expression : r -> r -> t -> t * r
  val make_postfix_unary_expression : r -> r -> t -> t * r
  val make_binary_expression : r -> r -> r -> t -> t * r
  val make_instanceof_expression : r -> r -> r -> t -> t * r
  val make_is_expression : r -> r -> r -> t -> t * r
  val make_conditional_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_eval_expression : r -> r -> r -> r -> t -> t * r
  val make_empty_expression : r -> r -> r -> r -> t -> t * r
  val make_define_expression : r -> r -> r -> r -> t -> t * r
  val make_halt_compiler_expression : r -> r -> r -> r -> t -> t * r
  val make_isset_expression : r -> r -> r -> r -> t -> t * r
  val make_function_call_expression : r -> r -> r -> r -> t -> t * r
  val make_function_call_with_type_arguments_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_parenthesized_expression : r -> r -> r -> t -> t * r
  val make_braced_expression : r -> r -> r -> t -> t * r
  val make_embedded_braced_expression : r -> r -> r -> t -> t * r
  val make_list_expression : r -> r -> r -> r -> t -> t * r
  val make_collection_literal_expression : r -> r -> r -> r -> t -> t * r
  val make_object_creation_expression : r -> r -> t -> t * r
  val make_constructor_call : r -> r -> r -> r -> t -> t * r
  val make_array_creation_expression : r -> r -> r -> t -> t * r
  val make_array_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_darray_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_dictionary_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_keyset_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_varray_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_vector_intrinsic_expression : r -> r -> r -> r -> t -> t * r
  val make_element_initializer : r -> r -> r -> t -> t * r
  val make_subscript_expression : r -> r -> r -> r -> t -> t * r
  val make_embedded_subscript_expression : r -> r -> r -> r -> t -> t * r
  val make_awaitable_creation_expression : r -> r -> r -> t -> t * r
  val make_xhp_children_declaration : r -> r -> r -> t -> t * r
  val make_xhp_children_parenthesized_list : r -> r -> r -> t -> t * r
  val make_xhp_category_declaration : r -> r -> r -> t -> t * r
  val make_xhp_enum_type : r -> r -> r -> r -> r -> t -> t * r
  val make_xhp_required : r -> r -> t -> t * r
  val make_xhp_class_attribute_declaration : r -> r -> r -> t -> t * r
  val make_xhp_class_attribute : r -> r -> r -> r -> t -> t * r
  val make_xhp_simple_class_attribute : r -> t -> t * r
  val make_xhp_simple_attribute : r -> r -> r -> t -> t * r
  val make_xhp_spread_attribute : r -> r -> r -> r -> t -> t * r
  val make_xhp_open : r -> r -> r -> r -> t -> t * r
  val make_xhp_expression : r -> r -> r -> t -> t * r
  val make_xhp_close : r -> r -> r -> t -> t * r
  val make_type_constant : r -> r -> r -> t -> t * r
  val make_vector_type_specifier : r -> r -> r -> r -> r -> t -> t * r
  val make_keyset_type_specifier : r -> r -> r -> r -> r -> t -> t * r
  val make_tuple_type_explicit_specifier : r -> r -> r -> r -> t -> t * r
  val make_varray_type_specifier : r -> r -> r -> r -> r -> t -> t * r
  val make_vector_array_type_specifier : r -> r -> r -> r -> t -> t * r
  val make_type_parameter : r -> r -> r -> t -> t * r
  val make_type_constraint : r -> r -> t -> t * r
  val make_darray_type_specifier : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_map_array_type_specifier : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_dictionary_type_specifier : r -> r -> r -> r -> t -> t * r
  val make_closure_type_specifier : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_closure_parameter_type_specifier : r -> r -> t -> t * r
  val make_classname_type_specifier : r -> r -> r -> r -> r -> t -> t * r
  val make_field_specifier : r -> r -> r -> r -> t -> t * r
  val make_field_initializer : r -> r -> r -> t -> t * r
  val make_shape_type_specifier : r -> r -> r -> r -> r -> t -> t * r
  val make_shape_expression : r -> r -> r -> r -> t -> t * r
  val make_tuple_expression : r -> r -> r -> r -> t -> t * r
  val make_generic_type_specifier : r -> r -> t -> t * r
  val make_nullable_type_specifier : r -> r -> t -> t * r
  val make_soft_type_specifier : r -> r -> t -> t * r
  val make_type_arguments : r -> r -> r -> t -> t * r
  val make_type_parameters : r -> r -> r -> t -> t * r
  val make_tuple_type_specifier : r -> r -> r -> t -> t * r
  val make_error : r -> t -> t * r
  val make_list_item : r -> r -> t -> t * r

end (* SmartConstructors_S *)
