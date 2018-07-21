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
 * This module contains a signature which can be used to describe smart
 * constructors.
  
 *)

module ParserEnv = Full_fidelity_parser_env

module type SmartConstructors_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type t (* state *) [@@deriving show]
  type r (* smart constructor return type *) [@@deriving show]

  val initial_state : ParserEnv.t -> t
  val make_token : Token.t -> t -> t * r
  val make_missing : Full_fidelity_source_text.pos -> t -> t * r
  val make_list : Full_fidelity_source_text.pos -> r list -> t -> t * r
  val make_end_of_file : r -> t -> t * r
  val make_script : r -> t -> t * r
  val make_qualified_name : r -> t -> t * r
  val make_simple_type_specifier : r -> t -> t * r
  val make_literal_expression : r -> t -> t * r
  val make_prefixed_string_expression : r -> r -> t -> t * r
  val make_variable_expression : r -> t -> t * r
  val make_pipe_variable_expression : r -> t -> t * r
  val make_enum_declaration : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_enumerator : r -> r -> r -> r -> t -> t * r
  val make_alias_declaration : r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_property_declaration : r -> r -> r -> r -> r -> t -> t * r
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
  val make_const_declaration : r -> r -> r -> r -> r -> r -> t -> t * r
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
  val make_alternate_loop_statement : r -> r -> r -> r -> t -> t * r
  val make_expression_statement : r -> r -> t -> t * r
  val make_markup_section : r -> r -> r -> r -> t -> t * r
  val make_markup_suffix : r -> r -> t -> t * r
  val make_unset_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_let_statement : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_using_statement_block_scoped : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_using_statement_function_scoped : r -> r -> r -> r -> t -> t * r
  val make_declare_directive_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_declare_block_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_while_statement : r -> r -> r -> r -> r -> t -> t * r
  val make_if_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_elseif_clause : r -> r -> r -> r -> r -> t -> t * r
  val make_else_clause : r -> r -> t -> t * r
  val make_alternate_if_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_alternate_elseif_clause : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_alternate_else_clause : r -> r -> r -> t -> t * r
  val make_try_statement : r -> r -> r -> r -> t -> t * r
  val make_catch_clause : r -> r -> r -> r -> r -> r -> t -> t * r
  val make_finally_clause : r -> r -> t -> t * r
  val make_do_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_for_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_foreach_statement : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_switch_statement : r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_alternate_switch_statement : r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
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
  val make_anonymous_function : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_php7_anonymous_function : r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> r -> t -> t * r
  val make_anonymous_function_use_clause : r -> r -> r -> r -> t -> t * r
  val make_lambda_expression : r -> r -> r -> r -> r -> r -> t -> t * r
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
  val make_as_expression : r -> r -> r -> t -> t * r
  val make_nullable_as_expression : r -> r -> r -> t -> t * r
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
  val make_darray_intrinsic_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_dictionary_intrinsic_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_keyset_intrinsic_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_varray_intrinsic_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_vector_intrinsic_expression : r -> r -> r -> r -> r -> t -> t * r
  val make_element_initializer : r -> r -> r -> t -> t * r
  val make_subscript_expression : r -> r -> r -> r -> t -> t * r
  val make_embedded_subscript_expression : r -> r -> r -> r -> t -> t * r
  val make_awaitable_creation_expression : r -> r -> r -> r -> t -> t * r
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
  val make_type_parameter : r -> r -> r -> r -> t -> t * r
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
  val make_reified_type_argument : r -> r -> t -> t * r
  val make_type_arguments : r -> r -> r -> t -> t * r
  val make_type_parameters : r -> r -> r -> t -> t * r
  val make_tuple_type_specifier : r -> r -> r -> t -> t * r
  val make_error : r -> t -> t * r
  val make_list_item : r -> r -> t -> t * r

end (* SmartConstructors_S *)

module ParserWrapper (Parser : sig
  type parser_type [@@deriving show]
  module SCI : SmartConstructors_S
  val call : parser_type -> (SCI.t -> SCI.t * SCI.r) -> parser_type * SCI.r
end) = struct

  module Make = struct
    open Parser

    let token parser token = call parser (SCI.make_token token)
    let missing parser p = call parser (SCI.make_missing p)
    let list parser p items = call parser (SCI.make_list p items)
    let end_of_file parser arg0 = call parser (SCI.make_end_of_file arg0)
    let script parser arg0 = call parser (SCI.make_script arg0)
    let qualified_name parser arg0 = call parser (SCI.make_qualified_name arg0)
    let simple_type_specifier parser arg0 = call parser (SCI.make_simple_type_specifier arg0)
    let literal_expression parser arg0 = call parser (SCI.make_literal_expression arg0)
    let prefixed_string_expression parser arg0 arg1 = call parser (SCI.make_prefixed_string_expression arg0 arg1)
    let variable_expression parser arg0 = call parser (SCI.make_variable_expression arg0)
    let pipe_variable_expression parser arg0 = call parser (SCI.make_pipe_variable_expression arg0)
    let enum_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 = call parser (SCI.make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8)
    let enumerator parser arg0 arg1 arg2 arg3 = call parser (SCI.make_enumerator arg0 arg1 arg2 arg3)
    let alias_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 = call parser (SCI.make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7)
    let property_declaration parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_property_declaration arg0 arg1 arg2 arg3 arg4)
    let property_declarator parser arg0 arg1 = call parser (SCI.make_property_declarator arg0 arg1)
    let namespace_declaration parser arg0 arg1 arg2 = call parser (SCI.make_namespace_declaration arg0 arg1 arg2)
    let namespace_body parser arg0 arg1 arg2 = call parser (SCI.make_namespace_body arg0 arg1 arg2)
    let namespace_empty_body parser arg0 = call parser (SCI.make_namespace_empty_body arg0)
    let namespace_use_declaration parser arg0 arg1 arg2 arg3 = call parser (SCI.make_namespace_use_declaration arg0 arg1 arg2 arg3)
    let namespace_group_use_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 = call parser (SCI.make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6)
    let namespace_use_clause parser arg0 arg1 arg2 arg3 = call parser (SCI.make_namespace_use_clause arg0 arg1 arg2 arg3)
    let function_declaration parser arg0 arg1 arg2 = call parser (SCI.make_function_declaration arg0 arg1 arg2)
    let function_declaration_header parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 = call parser (SCI.make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10)
    let where_clause parser arg0 arg1 = call parser (SCI.make_where_clause arg0 arg1)
    let where_constraint parser arg0 arg1 arg2 = call parser (SCI.make_where_constraint arg0 arg1 arg2)
    let methodish_declaration parser arg0 arg1 arg2 arg3 = call parser (SCI.make_methodish_declaration arg0 arg1 arg2 arg3)
    let classish_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 = call parser (SCI.make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9)
    let classish_body parser arg0 arg1 arg2 = call parser (SCI.make_classish_body arg0 arg1 arg2)
    let trait_use_precedence_item parser arg0 arg1 arg2 = call parser (SCI.make_trait_use_precedence_item arg0 arg1 arg2)
    let trait_use_alias_item parser arg0 arg1 arg2 arg3 = call parser (SCI.make_trait_use_alias_item arg0 arg1 arg2 arg3)
    let trait_use_conflict_resolution parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4)
    let trait_use parser arg0 arg1 arg2 = call parser (SCI.make_trait_use arg0 arg1 arg2)
    let require_clause parser arg0 arg1 arg2 arg3 = call parser (SCI.make_require_clause arg0 arg1 arg2 arg3)
    let const_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_const_declaration arg0 arg1 arg2 arg3 arg4 arg5)
    let constant_declarator parser arg0 arg1 = call parser (SCI.make_constant_declarator arg0 arg1)
    let type_const_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 = call parser (SCI.make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8)
    let decorated_expression parser arg0 arg1 = call parser (SCI.make_decorated_expression arg0 arg1)
    let parameter_declaration parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5)
    let variadic_parameter parser arg0 arg1 arg2 = call parser (SCI.make_variadic_parameter arg0 arg1 arg2)
    let attribute_specification parser arg0 arg1 arg2 = call parser (SCI.make_attribute_specification arg0 arg1 arg2)
    let attribute parser arg0 arg1 arg2 arg3 = call parser (SCI.make_attribute arg0 arg1 arg2 arg3)
    let inclusion_expression parser arg0 arg1 = call parser (SCI.make_inclusion_expression arg0 arg1)
    let inclusion_directive parser arg0 arg1 = call parser (SCI.make_inclusion_directive arg0 arg1)
    let compound_statement parser arg0 arg1 arg2 = call parser (SCI.make_compound_statement arg0 arg1 arg2)
    let alternate_loop_statement parser arg0 arg1 arg2 arg3 = call parser (SCI.make_alternate_loop_statement arg0 arg1 arg2 arg3)
    let expression_statement parser arg0 arg1 = call parser (SCI.make_expression_statement arg0 arg1)
    let markup_section parser arg0 arg1 arg2 arg3 = call parser (SCI.make_markup_section arg0 arg1 arg2 arg3)
    let markup_suffix parser arg0 arg1 = call parser (SCI.make_markup_suffix arg0 arg1)
    let unset_statement parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_unset_statement arg0 arg1 arg2 arg3 arg4)
    let let_statement parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_let_statement arg0 arg1 arg2 arg3 arg4 arg5)
    let using_statement_block_scoped parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5)
    let using_statement_function_scoped parser arg0 arg1 arg2 arg3 = call parser (SCI.make_using_statement_function_scoped arg0 arg1 arg2 arg3)
    let declare_directive_statement parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_declare_directive_statement arg0 arg1 arg2 arg3 arg4)
    let declare_block_statement parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_declare_block_statement arg0 arg1 arg2 arg3 arg4)
    let while_statement parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_while_statement arg0 arg1 arg2 arg3 arg4)
    let if_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 = call parser (SCI.make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6)
    let elseif_clause parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_elseif_clause arg0 arg1 arg2 arg3 arg4)
    let else_clause parser arg0 arg1 = call parser (SCI.make_else_clause arg0 arg1)
    let alternate_if_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 = call parser (SCI.make_alternate_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9)
    let alternate_elseif_clause parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_alternate_elseif_clause arg0 arg1 arg2 arg3 arg4 arg5)
    let alternate_else_clause parser arg0 arg1 arg2 = call parser (SCI.make_alternate_else_clause arg0 arg1 arg2)
    let try_statement parser arg0 arg1 arg2 arg3 = call parser (SCI.make_try_statement arg0 arg1 arg2 arg3)
    let catch_clause parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5)
    let finally_clause parser arg0 arg1 = call parser (SCI.make_finally_clause arg0 arg1)
    let do_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 = call parser (SCI.make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6)
    let for_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 = call parser (SCI.make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8)
    let foreach_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 = call parser (SCI.make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9)
    let switch_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 = call parser (SCI.make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6)
    let alternate_switch_statement parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 = call parser (SCI.make_alternate_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7)
    let switch_section parser arg0 arg1 arg2 = call parser (SCI.make_switch_section arg0 arg1 arg2)
    let switch_fallthrough parser arg0 arg1 = call parser (SCI.make_switch_fallthrough arg0 arg1)
    let case_label parser arg0 arg1 arg2 = call parser (SCI.make_case_label arg0 arg1 arg2)
    let default_label parser arg0 arg1 = call parser (SCI.make_default_label arg0 arg1)
    let return_statement parser arg0 arg1 arg2 = call parser (SCI.make_return_statement arg0 arg1 arg2)
    let goto_label parser arg0 arg1 = call parser (SCI.make_goto_label arg0 arg1)
    let goto_statement parser arg0 arg1 arg2 = call parser (SCI.make_goto_statement arg0 arg1 arg2)
    let throw_statement parser arg0 arg1 arg2 = call parser (SCI.make_throw_statement arg0 arg1 arg2)
    let break_statement parser arg0 arg1 arg2 = call parser (SCI.make_break_statement arg0 arg1 arg2)
    let continue_statement parser arg0 arg1 arg2 = call parser (SCI.make_continue_statement arg0 arg1 arg2)
    let function_static_statement parser arg0 arg1 arg2 = call parser (SCI.make_function_static_statement arg0 arg1 arg2)
    let static_declarator parser arg0 arg1 = call parser (SCI.make_static_declarator arg0 arg1)
    let echo_statement parser arg0 arg1 arg2 = call parser (SCI.make_echo_statement arg0 arg1 arg2)
    let global_statement parser arg0 arg1 arg2 = call parser (SCI.make_global_statement arg0 arg1 arg2)
    let simple_initializer parser arg0 arg1 = call parser (SCI.make_simple_initializer arg0 arg1)
    let anonymous_class parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 = call parser (SCI.make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8)
    let anonymous_function parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 = call parser (SCI.make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12)
    let php7_anonymous_function parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 = call parser (SCI.make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12)
    let anonymous_function_use_clause parser arg0 arg1 arg2 arg3 = call parser (SCI.make_anonymous_function_use_clause arg0 arg1 arg2 arg3)
    let lambda_expression parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_lambda_expression arg0 arg1 arg2 arg3 arg4 arg5)
    let lambda_signature parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_lambda_signature arg0 arg1 arg2 arg3 arg4)
    let cast_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_cast_expression arg0 arg1 arg2 arg3)
    let scope_resolution_expression parser arg0 arg1 arg2 = call parser (SCI.make_scope_resolution_expression arg0 arg1 arg2)
    let member_selection_expression parser arg0 arg1 arg2 = call parser (SCI.make_member_selection_expression arg0 arg1 arg2)
    let safe_member_selection_expression parser arg0 arg1 arg2 = call parser (SCI.make_safe_member_selection_expression arg0 arg1 arg2)
    let embedded_member_selection_expression parser arg0 arg1 arg2 = call parser (SCI.make_embedded_member_selection_expression arg0 arg1 arg2)
    let yield_expression parser arg0 arg1 = call parser (SCI.make_yield_expression arg0 arg1)
    let yield_from_expression parser arg0 arg1 arg2 = call parser (SCI.make_yield_from_expression arg0 arg1 arg2)
    let prefix_unary_expression parser arg0 arg1 = call parser (SCI.make_prefix_unary_expression arg0 arg1)
    let postfix_unary_expression parser arg0 arg1 = call parser (SCI.make_postfix_unary_expression arg0 arg1)
    let binary_expression parser arg0 arg1 arg2 = call parser (SCI.make_binary_expression arg0 arg1 arg2)
    let instanceof_expression parser arg0 arg1 arg2 = call parser (SCI.make_instanceof_expression arg0 arg1 arg2)
    let is_expression parser arg0 arg1 arg2 = call parser (SCI.make_is_expression arg0 arg1 arg2)
    let as_expression parser arg0 arg1 arg2 = call parser (SCI.make_as_expression arg0 arg1 arg2)
    let nullable_as_expression parser arg0 arg1 arg2 = call parser (SCI.make_nullable_as_expression arg0 arg1 arg2)
    let conditional_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_conditional_expression arg0 arg1 arg2 arg3 arg4)
    let eval_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_eval_expression arg0 arg1 arg2 arg3)
    let empty_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_empty_expression arg0 arg1 arg2 arg3)
    let define_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_define_expression arg0 arg1 arg2 arg3)
    let halt_compiler_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_halt_compiler_expression arg0 arg1 arg2 arg3)
    let isset_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_isset_expression arg0 arg1 arg2 arg3)
    let function_call_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_function_call_expression arg0 arg1 arg2 arg3)
    let function_call_with_type_arguments_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4)
    let parenthesized_expression parser arg0 arg1 arg2 = call parser (SCI.make_parenthesized_expression arg0 arg1 arg2)
    let braced_expression parser arg0 arg1 arg2 = call parser (SCI.make_braced_expression arg0 arg1 arg2)
    let embedded_braced_expression parser arg0 arg1 arg2 = call parser (SCI.make_embedded_braced_expression arg0 arg1 arg2)
    let list_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_list_expression arg0 arg1 arg2 arg3)
    let collection_literal_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_collection_literal_expression arg0 arg1 arg2 arg3)
    let object_creation_expression parser arg0 arg1 = call parser (SCI.make_object_creation_expression arg0 arg1)
    let constructor_call parser arg0 arg1 arg2 arg3 = call parser (SCI.make_constructor_call arg0 arg1 arg2 arg3)
    let array_creation_expression parser arg0 arg1 arg2 = call parser (SCI.make_array_creation_expression arg0 arg1 arg2)
    let array_intrinsic_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_array_intrinsic_expression arg0 arg1 arg2 arg3)
    let darray_intrinsic_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_darray_intrinsic_expression arg0 arg1 arg2 arg3 arg4)
    let dictionary_intrinsic_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 arg4)
    let keyset_intrinsic_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 arg4)
    let varray_intrinsic_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_varray_intrinsic_expression arg0 arg1 arg2 arg3 arg4)
    let vector_intrinsic_expression parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_vector_intrinsic_expression arg0 arg1 arg2 arg3 arg4)
    let element_initializer parser arg0 arg1 arg2 = call parser (SCI.make_element_initializer arg0 arg1 arg2)
    let subscript_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_subscript_expression arg0 arg1 arg2 arg3)
    let embedded_subscript_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_embedded_subscript_expression arg0 arg1 arg2 arg3)
    let awaitable_creation_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_awaitable_creation_expression arg0 arg1 arg2 arg3)
    let xhp_children_declaration parser arg0 arg1 arg2 = call parser (SCI.make_xhp_children_declaration arg0 arg1 arg2)
    let xhp_children_parenthesized_list parser arg0 arg1 arg2 = call parser (SCI.make_xhp_children_parenthesized_list arg0 arg1 arg2)
    let xhp_category_declaration parser arg0 arg1 arg2 = call parser (SCI.make_xhp_category_declaration arg0 arg1 arg2)
    let xhp_enum_type parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_xhp_enum_type arg0 arg1 arg2 arg3 arg4)
    let xhp_required parser arg0 arg1 = call parser (SCI.make_xhp_required arg0 arg1)
    let xhp_class_attribute_declaration parser arg0 arg1 arg2 = call parser (SCI.make_xhp_class_attribute_declaration arg0 arg1 arg2)
    let xhp_class_attribute parser arg0 arg1 arg2 arg3 = call parser (SCI.make_xhp_class_attribute arg0 arg1 arg2 arg3)
    let xhp_simple_class_attribute parser arg0 = call parser (SCI.make_xhp_simple_class_attribute arg0)
    let xhp_simple_attribute parser arg0 arg1 arg2 = call parser (SCI.make_xhp_simple_attribute arg0 arg1 arg2)
    let xhp_spread_attribute parser arg0 arg1 arg2 arg3 = call parser (SCI.make_xhp_spread_attribute arg0 arg1 arg2 arg3)
    let xhp_open parser arg0 arg1 arg2 arg3 = call parser (SCI.make_xhp_open arg0 arg1 arg2 arg3)
    let xhp_expression parser arg0 arg1 arg2 = call parser (SCI.make_xhp_expression arg0 arg1 arg2)
    let xhp_close parser arg0 arg1 arg2 = call parser (SCI.make_xhp_close arg0 arg1 arg2)
    let type_constant parser arg0 arg1 arg2 = call parser (SCI.make_type_constant arg0 arg1 arg2)
    let vector_type_specifier parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_vector_type_specifier arg0 arg1 arg2 arg3 arg4)
    let keyset_type_specifier parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4)
    let tuple_type_explicit_specifier parser arg0 arg1 arg2 arg3 = call parser (SCI.make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3)
    let varray_type_specifier parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_varray_type_specifier arg0 arg1 arg2 arg3 arg4)
    let vector_array_type_specifier parser arg0 arg1 arg2 arg3 = call parser (SCI.make_vector_array_type_specifier arg0 arg1 arg2 arg3)
    let type_parameter parser arg0 arg1 arg2 arg3 = call parser (SCI.make_type_parameter arg0 arg1 arg2 arg3)
    let type_constraint parser arg0 arg1 = call parser (SCI.make_type_constraint arg0 arg1)
    let darray_type_specifier parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 = call parser (SCI.make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6)
    let map_array_type_specifier parser arg0 arg1 arg2 arg3 arg4 arg5 = call parser (SCI.make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5)
    let dictionary_type_specifier parser arg0 arg1 arg2 arg3 = call parser (SCI.make_dictionary_type_specifier arg0 arg1 arg2 arg3)
    let closure_type_specifier parser arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 = call parser (SCI.make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8)
    let closure_parameter_type_specifier parser arg0 arg1 = call parser (SCI.make_closure_parameter_type_specifier arg0 arg1)
    let classname_type_specifier parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_classname_type_specifier arg0 arg1 arg2 arg3 arg4)
    let field_specifier parser arg0 arg1 arg2 arg3 = call parser (SCI.make_field_specifier arg0 arg1 arg2 arg3)
    let field_initializer parser arg0 arg1 arg2 = call parser (SCI.make_field_initializer arg0 arg1 arg2)
    let shape_type_specifier parser arg0 arg1 arg2 arg3 arg4 = call parser (SCI.make_shape_type_specifier arg0 arg1 arg2 arg3 arg4)
    let shape_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_shape_expression arg0 arg1 arg2 arg3)
    let tuple_expression parser arg0 arg1 arg2 arg3 = call parser (SCI.make_tuple_expression arg0 arg1 arg2 arg3)
    let generic_type_specifier parser arg0 arg1 = call parser (SCI.make_generic_type_specifier arg0 arg1)
    let nullable_type_specifier parser arg0 arg1 = call parser (SCI.make_nullable_type_specifier arg0 arg1)
    let soft_type_specifier parser arg0 arg1 = call parser (SCI.make_soft_type_specifier arg0 arg1)
    let reified_type_argument parser arg0 arg1 = call parser (SCI.make_reified_type_argument arg0 arg1)
    let type_arguments parser arg0 arg1 arg2 = call parser (SCI.make_type_arguments arg0 arg1 arg2)
    let type_parameters parser arg0 arg1 arg2 = call parser (SCI.make_type_parameters arg0 arg1 arg2)
    let tuple_type_specifier parser arg0 arg1 arg2 = call parser (SCI.make_tuple_type_specifier arg0 arg1 arg2)
    let error parser arg0 = call parser (SCI.make_error arg0)
    let list_item parser arg0 arg1 = call parser (SCI.make_list_item arg0 arg1)

  end (* Make *)
end (* ParserWrapper *)
