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
 * This module contains smart constructors implementation that can be used to
 * build AST.
 
 *)

module type SC_S = SmartConstructors.SmartConstructors_S
module ParserEnv = Full_fidelity_parser_env

module type State_S = sig
  type r [@@deriving show]
  type t [@@deriving show]
  val initial : ParserEnv.t -> t
  val next : t -> r list -> t
end

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module WithState(State : State_S with type r = Syntax.t) = struct
    module Token = Syntax.Token
    type t = State.t [@@deriving show]
    type r = Syntax.t [@@deriving show]

    let initial_state = State.initial
    let make_token token state = State.next state [], Syntax.make_token token
    let make_missing (s, o) state = State.next state [], Syntax.make_missing s o
    let make_list (s, o) items state =
      if items <> []
      then State.next state items, Syntax.make_list s o items
      else make_missing (s, o) state
    let make_end_of_file arg0 state = State.next state [arg0], Syntax.make_end_of_file arg0
    let make_script arg0 state = State.next state [arg0], Syntax.make_script arg0
    let make_qualified_name arg0 state = State.next state [arg0], Syntax.make_qualified_name arg0
    let make_simple_type_specifier arg0 state = State.next state [arg0], Syntax.make_simple_type_specifier arg0
    let make_literal_expression arg0 state = State.next state [arg0], Syntax.make_literal_expression arg0
    let make_prefixed_string_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_prefixed_string_expression arg0 arg1
    let make_variable_expression arg0 state = State.next state [arg0], Syntax.make_variable_expression arg0
    let make_pipe_variable_expression arg0 state = State.next state [arg0], Syntax.make_pipe_variable_expression arg0
    let make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8], Syntax.make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8
    let make_enumerator arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_enumerator arg0 arg1 arg2 arg3
    let make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7], Syntax.make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7
    let make_property_declaration arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_property_declaration arg0 arg1 arg2 arg3 arg4
    let make_property_declarator arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_property_declarator arg0 arg1
    let make_namespace_declaration arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_namespace_declaration arg0 arg1 arg2
    let make_namespace_body arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_namespace_body arg0 arg1 arg2
    let make_namespace_empty_body arg0 state = State.next state [arg0], Syntax.make_namespace_empty_body arg0
    let make_namespace_use_declaration arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_namespace_use_declaration arg0 arg1 arg2 arg3
    let make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6], Syntax.make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6
    let make_namespace_use_clause arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_namespace_use_clause arg0 arg1 arg2 arg3
    let make_function_declaration arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_function_declaration arg0 arg1 arg2
    let make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10], Syntax.make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10
    let make_where_clause arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_where_clause arg0 arg1
    let make_where_constraint arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_where_constraint arg0 arg1 arg2
    let make_methodish_declaration arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_methodish_declaration arg0 arg1 arg2 arg3
    let make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9], Syntax.make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9
    let make_classish_body arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_classish_body arg0 arg1 arg2
    let make_trait_use_precedence_item arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_trait_use_precedence_item arg0 arg1 arg2
    let make_trait_use_alias_item arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_trait_use_alias_item arg0 arg1 arg2 arg3
    let make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4
    let make_trait_use arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_trait_use arg0 arg1 arg2
    let make_require_clause arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_require_clause arg0 arg1 arg2 arg3
    let make_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_const_declaration arg0 arg1 arg2 arg3 arg4 arg5
    let make_constant_declarator arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_constant_declarator arg0 arg1
    let make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8], Syntax.make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8
    let make_decorated_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_decorated_expression arg0 arg1
    let make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5
    let make_variadic_parameter arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_variadic_parameter arg0 arg1 arg2
    let make_attribute_specification arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_attribute_specification arg0 arg1 arg2
    let make_attribute arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_attribute arg0 arg1 arg2 arg3
    let make_inclusion_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_inclusion_expression arg0 arg1
    let make_inclusion_directive arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_inclusion_directive arg0 arg1
    let make_compound_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_compound_statement arg0 arg1 arg2
    let make_alternate_loop_statement arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_alternate_loop_statement arg0 arg1 arg2 arg3
    let make_expression_statement arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_expression_statement arg0 arg1
    let make_markup_section arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_markup_section arg0 arg1 arg2 arg3
    let make_markup_suffix arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_markup_suffix arg0 arg1
    let make_unset_statement arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_unset_statement arg0 arg1 arg2 arg3 arg4
    let make_let_statement arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_let_statement arg0 arg1 arg2 arg3 arg4 arg5
    let make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5
    let make_using_statement_function_scoped arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_using_statement_function_scoped arg0 arg1 arg2 arg3
    let make_declare_directive_statement arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_declare_directive_statement arg0 arg1 arg2 arg3 arg4
    let make_declare_block_statement arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_declare_block_statement arg0 arg1 arg2 arg3 arg4
    let make_while_statement arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_while_statement arg0 arg1 arg2 arg3 arg4
    let make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6], Syntax.make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6
    let make_elseif_clause arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_elseif_clause arg0 arg1 arg2 arg3 arg4
    let make_else_clause arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_else_clause arg0 arg1
    let make_alternate_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9], Syntax.make_alternate_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9
    let make_alternate_elseif_clause arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_alternate_elseif_clause arg0 arg1 arg2 arg3 arg4 arg5
    let make_alternate_else_clause arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_alternate_else_clause arg0 arg1 arg2
    let make_try_statement arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_try_statement arg0 arg1 arg2 arg3
    let make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5
    let make_finally_clause arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_finally_clause arg0 arg1
    let make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6], Syntax.make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6
    let make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8], Syntax.make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8
    let make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9], Syntax.make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9
    let make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6], Syntax.make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6
    let make_alternate_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7], Syntax.make_alternate_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7
    let make_switch_section arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_switch_section arg0 arg1 arg2
    let make_switch_fallthrough arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_switch_fallthrough arg0 arg1
    let make_case_label arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_case_label arg0 arg1 arg2
    let make_default_label arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_default_label arg0 arg1
    let make_return_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_return_statement arg0 arg1 arg2
    let make_goto_label arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_goto_label arg0 arg1
    let make_goto_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_goto_statement arg0 arg1 arg2
    let make_throw_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_throw_statement arg0 arg1 arg2
    let make_break_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_break_statement arg0 arg1 arg2
    let make_continue_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_continue_statement arg0 arg1 arg2
    let make_function_static_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_function_static_statement arg0 arg1 arg2
    let make_static_declarator arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_static_declarator arg0 arg1
    let make_echo_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_echo_statement arg0 arg1 arg2
    let make_global_statement arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_global_statement arg0 arg1 arg2
    let make_simple_initializer arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_simple_initializer arg0 arg1
    let make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8], Syntax.make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8
    let make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10; arg11; arg12], Syntax.make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12
    let make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10; arg11; arg12], Syntax.make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12
    let make_anonymous_function_use_clause arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_anonymous_function_use_clause arg0 arg1 arg2 arg3
    let make_lambda_expression arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_lambda_expression arg0 arg1 arg2 arg3 arg4 arg5
    let make_lambda_signature arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_lambda_signature arg0 arg1 arg2 arg3 arg4
    let make_cast_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_cast_expression arg0 arg1 arg2 arg3
    let make_scope_resolution_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_scope_resolution_expression arg0 arg1 arg2
    let make_member_selection_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_member_selection_expression arg0 arg1 arg2
    let make_safe_member_selection_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_safe_member_selection_expression arg0 arg1 arg2
    let make_embedded_member_selection_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_embedded_member_selection_expression arg0 arg1 arg2
    let make_yield_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_yield_expression arg0 arg1
    let make_yield_from_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_yield_from_expression arg0 arg1 arg2
    let make_prefix_unary_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_prefix_unary_expression arg0 arg1
    let make_postfix_unary_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_postfix_unary_expression arg0 arg1
    let make_binary_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_binary_expression arg0 arg1 arg2
    let make_instanceof_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_instanceof_expression arg0 arg1 arg2
    let make_is_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_is_expression arg0 arg1 arg2
    let make_as_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_as_expression arg0 arg1 arg2
    let make_nullable_as_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_nullable_as_expression arg0 arg1 arg2
    let make_conditional_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_conditional_expression arg0 arg1 arg2 arg3 arg4
    let make_eval_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_eval_expression arg0 arg1 arg2 arg3
    let make_empty_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_empty_expression arg0 arg1 arg2 arg3
    let make_define_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_define_expression arg0 arg1 arg2 arg3
    let make_halt_compiler_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_halt_compiler_expression arg0 arg1 arg2 arg3
    let make_isset_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_isset_expression arg0 arg1 arg2 arg3
    let make_function_call_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_function_call_expression arg0 arg1 arg2 arg3
    let make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4
    let make_parenthesized_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_parenthesized_expression arg0 arg1 arg2
    let make_braced_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_braced_expression arg0 arg1 arg2
    let make_embedded_braced_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_embedded_braced_expression arg0 arg1 arg2
    let make_list_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_list_expression arg0 arg1 arg2 arg3
    let make_collection_literal_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_collection_literal_expression arg0 arg1 arg2 arg3
    let make_object_creation_expression arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_object_creation_expression arg0 arg1
    let make_constructor_call arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_constructor_call arg0 arg1 arg2 arg3
    let make_array_creation_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_array_creation_expression arg0 arg1 arg2
    let make_array_intrinsic_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_array_intrinsic_expression arg0 arg1 arg2 arg3
    let make_darray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_darray_intrinsic_expression arg0 arg1 arg2 arg3 arg4
    let make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 arg4
    let make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 arg4
    let make_varray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_varray_intrinsic_expression arg0 arg1 arg2 arg3 arg4
    let make_vector_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_vector_intrinsic_expression arg0 arg1 arg2 arg3 arg4
    let make_element_initializer arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_element_initializer arg0 arg1 arg2
    let make_subscript_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_subscript_expression arg0 arg1 arg2 arg3
    let make_embedded_subscript_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_embedded_subscript_expression arg0 arg1 arg2 arg3
    let make_awaitable_creation_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_awaitable_creation_expression arg0 arg1 arg2 arg3
    let make_xhp_children_declaration arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_children_declaration arg0 arg1 arg2
    let make_xhp_children_parenthesized_list arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_children_parenthesized_list arg0 arg1 arg2
    let make_xhp_category_declaration arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_category_declaration arg0 arg1 arg2
    let make_xhp_enum_type arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_xhp_enum_type arg0 arg1 arg2 arg3 arg4
    let make_xhp_required arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_xhp_required arg0 arg1
    let make_xhp_class_attribute_declaration arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_class_attribute_declaration arg0 arg1 arg2
    let make_xhp_class_attribute arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_xhp_class_attribute arg0 arg1 arg2 arg3
    let make_xhp_simple_class_attribute arg0 state = State.next state [arg0], Syntax.make_xhp_simple_class_attribute arg0
    let make_xhp_simple_attribute arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_simple_attribute arg0 arg1 arg2
    let make_xhp_spread_attribute arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_xhp_spread_attribute arg0 arg1 arg2 arg3
    let make_xhp_open arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_xhp_open arg0 arg1 arg2 arg3
    let make_xhp_expression arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_expression arg0 arg1 arg2
    let make_xhp_close arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_xhp_close arg0 arg1 arg2
    let make_type_constant arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_type_constant arg0 arg1 arg2
    let make_vector_type_specifier arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_vector_type_specifier arg0 arg1 arg2 arg3 arg4
    let make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4
    let make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3
    let make_varray_type_specifier arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_varray_type_specifier arg0 arg1 arg2 arg3 arg4
    let make_vector_array_type_specifier arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_vector_array_type_specifier arg0 arg1 arg2 arg3
    let make_type_parameter arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_type_parameter arg0 arg1 arg2 arg3
    let make_type_constraint arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_type_constraint arg0 arg1
    let make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6], Syntax.make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6
    let make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5], Syntax.make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5
    let make_dictionary_type_specifier arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_dictionary_type_specifier arg0 arg1 arg2 arg3
    let make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state = State.next state [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8], Syntax.make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8
    let make_closure_parameter_type_specifier arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_closure_parameter_type_specifier arg0 arg1
    let make_classname_type_specifier arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_classname_type_specifier arg0 arg1 arg2 arg3 arg4
    let make_field_specifier arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_field_specifier arg0 arg1 arg2 arg3
    let make_field_initializer arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_field_initializer arg0 arg1 arg2
    let make_shape_type_specifier arg0 arg1 arg2 arg3 arg4 state = State.next state [arg0; arg1; arg2; arg3; arg4], Syntax.make_shape_type_specifier arg0 arg1 arg2 arg3 arg4
    let make_shape_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_shape_expression arg0 arg1 arg2 arg3
    let make_tuple_expression arg0 arg1 arg2 arg3 state = State.next state [arg0; arg1; arg2; arg3], Syntax.make_tuple_expression arg0 arg1 arg2 arg3
    let make_generic_type_specifier arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_generic_type_specifier arg0 arg1
    let make_nullable_type_specifier arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_nullable_type_specifier arg0 arg1
    let make_soft_type_specifier arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_soft_type_specifier arg0 arg1
    let make_reified_type_argument arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_reified_type_argument arg0 arg1
    let make_type_arguments arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_type_arguments arg0 arg1 arg2
    let make_type_parameters arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_type_parameters arg0 arg1 arg2
    let make_tuple_type_specifier arg0 arg1 arg2 state = State.next state [arg0; arg1; arg2], Syntax.make_tuple_type_specifier arg0 arg1 arg2
    let make_error arg0 state = State.next state [arg0], Syntax.make_error arg0
    let make_list_item arg0 arg1 state = State.next state [arg0; arg1], Syntax.make_list_item arg0 arg1

  end (* WithState *)

  include WithState(
    struct
      type r = Syntax.t [@@deriving show]
      type t = unit [@@deriving show]
      let initial _ = ()
      let next () _ = ()
    end
  )
end (* WithSyntax *)
