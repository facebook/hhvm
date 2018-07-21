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
 * This module contains smart constructors implementation that does nothing
 * and can be used as initial stubs.
 
 *)

module type Op_S = sig
  type r [@@deriving show]
  val is_zero: r -> bool
  val flatten: r list -> r
  val zero: r
end

module WithOp(Op : Op_S) = struct
  type r = Op.r [@@deriving show]

  let make_token _token state = state, Op.zero
  let make_missing _ state = state, Op.zero
  let make_list _  _ state = state, Op.zero
  let make_end_of_file arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_script arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_qualified_name arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_simple_type_specifier arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_literal_expression arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_prefixed_string_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_variable_expression arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_pipe_variable_expression arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8]
  let make_enumerator arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7]
  let make_property_declaration arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_property_declarator arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_namespace_declaration arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_namespace_body arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_namespace_empty_body arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_namespace_use_declaration arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6]
  let make_namespace_use_clause arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_function_declaration arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 && Op.is_zero arg10 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10]
  let make_where_clause arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_where_constraint arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_methodish_declaration arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9]
  let make_classish_body arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_trait_use_precedence_item arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_trait_use_alias_item arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_trait_use arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_require_clause arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_constant_declarator arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8]
  let make_decorated_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_variadic_parameter arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_attribute_specification arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_attribute arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_inclusion_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_inclusion_directive arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_compound_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_alternate_loop_statement arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_expression_statement arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_markup_section arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_markup_suffix arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_unset_statement arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_let_statement arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_using_statement_function_scoped arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_declare_directive_statement arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_declare_block_statement arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_while_statement arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6]
  let make_elseif_clause arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_else_clause arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_alternate_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9]
  let make_alternate_elseif_clause arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_alternate_else_clause arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_try_statement arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_finally_clause arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6]
  let make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8]
  let make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9]
  let make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6]
  let make_alternate_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7]
  let make_switch_section arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_switch_fallthrough arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_case_label arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_default_label arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_return_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_goto_label arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_goto_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_throw_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_break_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_continue_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_function_static_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_static_declarator arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_echo_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_global_statement arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_simple_initializer arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8]
  let make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 && Op.is_zero arg10 && Op.is_zero arg11 && Op.is_zero arg12 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10; arg11; arg12]
  let make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 && Op.is_zero arg9 && Op.is_zero arg10 && Op.is_zero arg11 && Op.is_zero arg12 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8; arg9; arg10; arg11; arg12]
  let make_anonymous_function_use_clause arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_lambda_expression arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_lambda_signature arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_cast_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_scope_resolution_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_member_selection_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_safe_member_selection_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_embedded_member_selection_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_yield_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_yield_from_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_prefix_unary_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_postfix_unary_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_binary_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_instanceof_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_is_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_as_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_nullable_as_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_conditional_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_eval_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_empty_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_define_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_halt_compiler_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_isset_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_function_call_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_parenthesized_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_braced_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_embedded_braced_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_list_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_collection_literal_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_object_creation_expression arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_constructor_call arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_array_creation_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_array_intrinsic_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_darray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_varray_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_vector_intrinsic_expression arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_element_initializer arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_subscript_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_embedded_subscript_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_awaitable_creation_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_xhp_children_declaration arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_children_parenthesized_list arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_category_declaration arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_enum_type arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_xhp_required arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_xhp_class_attribute_declaration arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_class_attribute arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_xhp_simple_class_attribute arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_xhp_simple_attribute arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_spread_attribute arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_xhp_open arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_xhp_expression arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_xhp_close arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_type_constant arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_vector_type_specifier arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_varray_type_specifier arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_vector_array_type_specifier arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_type_parameter arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_type_constraint arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6]
  let make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5]
  let make_dictionary_type_specifier arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 && Op.is_zero arg5 && Op.is_zero arg6 && Op.is_zero arg7 && Op.is_zero arg8 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4; arg5; arg6; arg7; arg8]
  let make_closure_parameter_type_specifier arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_classname_type_specifier arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_field_specifier arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_field_initializer arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_shape_type_specifier arg0 arg1 arg2 arg3 arg4 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 && Op.is_zero arg4 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3; arg4]
  let make_shape_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_tuple_expression arg0 arg1 arg2 arg3 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 && Op.is_zero arg3 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2; arg3]
  let make_generic_type_specifier arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_nullable_type_specifier arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_soft_type_specifier arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_reified_type_argument arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]
  let make_type_arguments arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_type_parameters arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_tuple_type_specifier arg0 arg1 arg2 state =
    if Op.is_zero arg0 && Op.is_zero arg1 && Op.is_zero arg2 then state, Op.zero
    else state, Op.flatten [arg0; arg1; arg2]
  let make_error arg0 state =
    if Op.is_zero arg0 then state, Op.zero
    else state, Op.flatten [arg0]
  let make_list_item arg0 arg1 state =
    if Op.is_zero arg0 && Op.is_zero arg1 then state, Op.zero
    else state, Op.flatten [arg0; arg1]


end (* WithSyntax *)
