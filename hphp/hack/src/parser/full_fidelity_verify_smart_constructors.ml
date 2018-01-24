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
 * This module contains smart constructors implementation that can be used to
 * build AST.
 
 *)

module SCSig = Full_fidelity_smart_constructors_sig

module WithSyntax(Syntax: Syntax_sig.Syntax_S): SCSig.SmartConstructors_S =
struct
  type t = Syntax.t list
  type r = unit

  let make_missing stack = stack, ()
  let make_list stack _ = stack, ()

  let make_end_of_file stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_end_of_file arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_script stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_script arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_qualified_name stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_qualified_name arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_simple_type_specifier stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_simple_type_specifier arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_literal_expression stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_literal_expression arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_variable_expression stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_variable_expression arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_pipe_variable_expression stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_pipe_variable_expression arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_enum_declaration stack () () () () () () () () () =
    match stack with
      | arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_enum_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_enumerator stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_enumerator arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_alias_declaration stack () () () () () () () () =
    match stack with
      | arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_alias_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_property_declaration stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_property_declaration arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_property_declarator stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_property_declarator arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_declaration stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_namespace_declaration arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_body stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_namespace_body arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_empty_body stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_namespace_empty_body arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_use_declaration stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_namespace_use_declaration arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_group_use_declaration stack () () () () () () () =
    match stack with
      | arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_namespace_group_use_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_namespace_use_clause stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_namespace_use_clause arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_function_declaration stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_function_declaration arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_function_declaration_header stack () () () () () () () () () () () =
    match stack with
      | arg10 :: arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_function_declaration_header arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_where_clause stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_where_clause arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_where_constraint stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_where_constraint arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_methodish_declaration stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_methodish_declaration arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_classish_declaration stack () () () () () () () () () () =
    match stack with
      | arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_classish_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_classish_body stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_classish_body arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_trait_use_precedence_item stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_trait_use_precedence_item arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_trait_use_alias_item stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_trait_use_alias_item arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_trait_use_conflict_resolution stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_trait_use_conflict_resolution arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_trait_use stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_trait_use arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_require_clause stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_require_clause arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_const_declaration stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_const_declaration arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_constant_declarator stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_constant_declarator arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_const_declaration stack () () () () () () () () () =
    match stack with
      | arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_type_const_declaration arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_decorated_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_decorated_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_parameter_declaration stack () () () () () () =
    match stack with
      | arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_parameter_declaration arg0 arg1 arg2 arg3 arg4 arg5 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_variadic_parameter stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_variadic_parameter arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_attribute_specification stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_attribute_specification arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_attribute stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_attribute arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_inclusion_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_inclusion_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_inclusion_directive stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_inclusion_directive arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_compound_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_compound_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_expression_statement stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_expression_statement arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_markup_section stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_markup_section arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_markup_suffix stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_markup_suffix arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_unset_statement stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_unset_statement arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_using_statement_block_scoped stack () () () () () () =
    match stack with
      | arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_using_statement_block_scoped arg0 arg1 arg2 arg3 arg4 arg5 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_using_statement_function_scoped stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_using_statement_function_scoped arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_declare_directive_statement stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_declare_directive_statement arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_declare_block_statement stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_declare_block_statement arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_while_statement stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_while_statement arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_if_statement stack () () () () () () () =
    match stack with
      | arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_if_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_elseif_clause stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_elseif_clause arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_else_clause stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_else_clause arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_if_endif_statement stack () () () () () () () () () () =
    match stack with
      | arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_if_endif_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_elseif_colon_clause stack () () () () () () =
    match stack with
      | arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_elseif_colon_clause arg0 arg1 arg2 arg3 arg4 arg5 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_else_colon_clause stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_else_colon_clause arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_try_statement stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_try_statement arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_catch_clause stack () () () () () () =
    match stack with
      | arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_catch_clause arg0 arg1 arg2 arg3 arg4 arg5 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_finally_clause stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_finally_clause arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_do_statement stack () () () () () () () =
    match stack with
      | arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_do_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_for_statement stack () () () () () () () () () =
    match stack with
      | arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_for_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_foreach_statement stack () () () () () () () () () () =
    match stack with
      | arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_foreach_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_switch_statement stack () () () () () () () =
    match stack with
      | arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_switch_statement arg0 arg1 arg2 arg3 arg4 arg5 arg6 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_switch_section stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_switch_section arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_switch_fallthrough stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_switch_fallthrough arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_case_label stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_case_label arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_default_label stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_default_label arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_return_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_return_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_goto_label stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_goto_label arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_goto_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_goto_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_throw_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_throw_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_break_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_break_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_continue_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_continue_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_function_static_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_function_static_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_static_declarator stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_static_declarator arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_echo_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_echo_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_global_statement stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_global_statement arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_simple_initializer stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_simple_initializer arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_anonymous_class stack () () () () () () () () () =
    match stack with
      | arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_anonymous_class arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_anonymous_function stack () () () () () () () () () () () =
    match stack with
      | arg10 :: arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_php7_anonymous_function stack () () () () () () () () () () () =
    match stack with
      | arg10 :: arg9 :: arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_php7_anonymous_function arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_anonymous_function_use_clause stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_anonymous_function_use_clause arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_lambda_expression stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_lambda_expression arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_lambda_signature stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_lambda_signature arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_cast_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_cast_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_scope_resolution_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_scope_resolution_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_member_selection_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_member_selection_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_safe_member_selection_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_safe_member_selection_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_embedded_member_selection_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_embedded_member_selection_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_yield_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_yield_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_yield_from_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_yield_from_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_prefix_unary_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_prefix_unary_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_postfix_unary_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_postfix_unary_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_binary_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_binary_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_instanceof_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_instanceof_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_is_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_is_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_conditional_expression stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_conditional_expression arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_eval_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_eval_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_empty_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_empty_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_define_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_define_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_halt_compiler_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_halt_compiler_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_isset_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_isset_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_function_call_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_function_call_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_function_call_with_type_arguments_expression stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_function_call_with_type_arguments_expression arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_parenthesized_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_parenthesized_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_braced_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_braced_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_embedded_braced_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_embedded_braced_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_list_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_list_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_collection_literal_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_collection_literal_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_object_creation_expression stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_object_creation_expression arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_constructor_call stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_constructor_call arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_array_creation_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_array_creation_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_array_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_array_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_darray_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_darray_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_dictionary_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_dictionary_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_keyset_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_keyset_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_varray_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_varray_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_vector_intrinsic_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_vector_intrinsic_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_element_initializer stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_element_initializer arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_subscript_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_subscript_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_embedded_subscript_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_embedded_subscript_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_awaitable_creation_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_awaitable_creation_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_children_declaration stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_children_declaration arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_children_parenthesized_list stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_children_parenthesized_list arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_category_declaration stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_category_declaration arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_enum_type stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_enum_type arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_required stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_xhp_required arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_class_attribute_declaration stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_class_attribute_declaration arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_class_attribute stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_class_attribute arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_simple_class_attribute stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_xhp_simple_class_attribute arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_simple_attribute stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_simple_attribute arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_spread_attribute stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_spread_attribute arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_open stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_open arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_expression stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_expression arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_xhp_close stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_xhp_close arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_constant stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_type_constant arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_vector_type_specifier stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_vector_type_specifier arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_keyset_type_specifier stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_keyset_type_specifier arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_tuple_type_explicit_specifier stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_tuple_type_explicit_specifier arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_varray_type_specifier stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_varray_type_specifier arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_vector_array_type_specifier stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_vector_array_type_specifier arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_parameter stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_type_parameter arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_constraint stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_type_constraint arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_darray_type_specifier stack () () () () () () () =
    match stack with
      | arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_darray_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_map_array_type_specifier stack () () () () () () =
    match stack with
      | arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_map_array_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_dictionary_type_specifier stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_dictionary_type_specifier arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_closure_type_specifier stack () () () () () () () () () =
    match stack with
      | arg8 :: arg7 :: arg6 :: arg5 :: arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_closure_type_specifier arg0 arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_closure_parameter_type_specifier stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_closure_parameter_type_specifier arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_classname_type_specifier stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_classname_type_specifier arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_field_specifier stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_field_specifier arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_field_initializer stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_field_initializer arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_shape_type_specifier stack () () () () () =
    match stack with
      | arg4 :: arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_shape_type_specifier arg0 arg1 arg2 arg3 arg4 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_shape_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_shape_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_tuple_expression stack () () () () =
    match stack with
      | arg3 :: arg2 :: arg1 :: arg0 :: rem -> Syntax.make_tuple_expression arg0 arg1 arg2 arg3 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_generic_type_specifier stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_generic_type_specifier arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_nullable_type_specifier stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_nullable_type_specifier arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_soft_type_specifier stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_soft_type_specifier arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_arguments stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_type_arguments arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_type_parameters stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_type_parameters arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_tuple_type_specifier stack () () () =
    match stack with
      | arg2 :: arg1 :: arg0 :: rem -> Syntax.make_tuple_type_specifier arg0 arg1 arg2 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_error stack () =
    match stack with
      | arg0 :: rem -> Syntax.make_error arg0 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
  let make_list_item stack () () =
    match stack with
      | arg1 :: arg0 :: rem -> Syntax.make_list_item arg0 arg1 :: rem, ()
      | _ -> failwith "Unexpected stack state"
  
end (* WithSyntax *)
