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
 * This module contains the type describing the structure of a syntax tree.
 *
 * The structure of the syntax tree is described by the collection of recursive
 * types that makes up the bulk of this file. The type `t` is the type of a node
 * in the syntax tree; each node has associated with it an arbitrary value of
 * type `SyntaxValue.t`, and syntax node proper, which has structure given by
 * the `syntax` type.
 *
 * Note that every child in the syntax tree is of type `t`, except for the
 * `Token.t` type. This should be the *only* child of a type other than `t`.
 * We are explicitly NOT attempting to impose a type structure on the parse
 * tree beyond what is already implied by the types here. For example,
 * we are not attempting to put into the type system here the restriction that
 * the children of a binary operator must be expressions. The reason for this
 * is because we are potentially parsing code as it is being typed, and we
 * do not want to restrict our ability to make good error recovery by imposing
 * a restriction that will only be valid in correct program text.
 *
 * That said, it would of course be ideal if the only children of a compound
 * statement were statements, and so on. But those invariants should be
 * imposed by the design of the parser, not by the type system of the syntax
 * tree code.
 *
 * We want to be able to use different kinds of tokens, with different
 * performance characteristics. Moreover, we want to associate arbitrary values
 * with the syntax nodes, so that we can construct syntax trees with various
 * properties -- trees that only know their widths and are thereby cheap to
 * serialize, trees that have full position data for each node, trees where the
 * tokens know their text and can therefore be edited, trees that have name
 * annotations or type annotations, and so on.
 *
 * We wish to associate arbitrary values with the syntax nodes so that we can
 * construct syntax trees with various properties -- trees that only know
 * their widths and are thereby cheap to serialize, trees that have full
 * position data for each node, trees where the tokens know their text and
 * can therefore be edited, trees that have name annotations or type
 * annotations, and so on.
 *
 * Therefore this module is functorized by the types for token and value to be
 * associated with the node.
 *)

module type TokenType = sig
  type t
  val kind: t -> Full_fidelity_token_kind.t
  val to_json: t -> Hh_json.json
end

module type SyntaxValueType = sig
  type t
end

(* This functor describe the shape of a parse tree that has a particular kind of
 * token in the leaves, and a particular kind of value associated with each
 * node.
 *)
module MakeSyntaxType(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type t = {
    syntax : syntax ;
    value : SyntaxValue.t
  }
  and end_of_file =
    { end_of_file_token                                  : t
    }
  and script =
    { script_declarations                                : t
    }
  and simple_type_specifier =
    { simple_type_specifier                              : t
    }
  and literal_expression =
    { literal_expression                                 : t
    }
  and variable_expression =
    { variable_expression                                : t
    }
  and qualified_name_expression =
    { qualified_name_expression                          : t
    }
  and pipe_variable_expression =
    { pipe_variable_expression                           : t
    }
  and enum_declaration =
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
  and enumerator =
    { enumerator_name                                    : t
    ; enumerator_equal                                   : t
    ; enumerator_value                                   : t
    ; enumerator_semicolon                               : t
    }
  and alias_declaration =
    { alias_attribute_spec                               : t
    ; alias_keyword                                      : t
    ; alias_name                                         : t
    ; alias_generic_parameter                            : t
    ; alias_constraint                                   : t
    ; alias_equal                                        : t
    ; alias_type                                         : t
    ; alias_semicolon                                    : t
    }
  and property_declaration =
    { property_modifiers                                 : t
    ; property_type                                      : t
    ; property_declarators                               : t
    ; property_semicolon                                 : t
    }
  and property_declarator =
    { property_name                                      : t
    ; property_initializer                               : t
    }
  and namespace_declaration =
    { namespace_keyword                                  : t
    ; namespace_name                                     : t
    ; namespace_body                                     : t
    }
  and namespace_body =
    { namespace_left_brace                               : t
    ; namespace_declarations                             : t
    ; namespace_right_brace                              : t
    }
  and namespace_empty_body =
    { namespace_semicolon                                : t
    }
  and namespace_use_declaration =
    { namespace_use_keyword                              : t
    ; namespace_use_kind                                 : t
    ; namespace_use_clauses                              : t
    ; namespace_use_semicolon                            : t
    }
  and namespace_group_use_declaration =
    { namespace_group_use_keyword                        : t
    ; namespace_group_use_kind                           : t
    ; namespace_group_use_prefix                         : t
    ; namespace_group_use_left_brace                     : t
    ; namespace_group_use_clauses                        : t
    ; namespace_group_use_right_brace                    : t
    ; namespace_group_use_semicolon                      : t
    }
  and namespace_use_clause =
    { namespace_use_clause_kind                          : t
    ; namespace_use_name                                 : t
    ; namespace_use_as                                   : t
    ; namespace_use_alias                                : t
    }
  and function_declaration =
    { function_attribute_spec                            : t
    ; function_declaration_header                        : t
    ; function_body                                      : t
    }
  and function_declaration_header =
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
  and where_clause =
    { where_clause_keyword                               : t
    ; where_clause_constraints                           : t
    }
  and where_constraint =
    { where_constraint_left_type                         : t
    ; where_constraint_operator                          : t
    ; where_constraint_right_type                        : t
    }
  and methodish_declaration =
    { methodish_attribute                                : t
    ; methodish_modifiers                                : t
    ; methodish_function_decl_header                     : t
    ; methodish_function_body                            : t
    ; methodish_semicolon                                : t
    }
  and classish_declaration =
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
  and classish_body =
    { classish_body_left_brace                           : t
    ; classish_body_elements                             : t
    ; classish_body_right_brace                          : t
    }
  and trait_use_precedence_item =
    { trait_use_precedence_item_name                     : t
    ; trait_use_precedence_item_keyword                  : t
    ; trait_use_precedence_item_removed_names            : t
    }
  and trait_use_alias_item =
    { trait_use_alias_item_aliasing_name                 : t
    ; trait_use_alias_item_keyword                       : t
    ; trait_use_alias_item_visibility                    : t
    ; trait_use_alias_item_aliased_name                  : t
    }
  and trait_use_conflict_resolution =
    { trait_use_conflict_resolution_keyword              : t
    ; trait_use_conflict_resolution_names                : t
    ; trait_use_conflict_resolution_left_brace           : t
    ; trait_use_conflict_resolution_clauses              : t
    ; trait_use_conflict_resolution_right_brace          : t
    }
  and trait_use =
    { trait_use_keyword                                  : t
    ; trait_use_names                                    : t
    ; trait_use_semicolon                                : t
    }
  and require_clause =
    { require_keyword                                    : t
    ; require_kind                                       : t
    ; require_name                                       : t
    ; require_semicolon                                  : t
    }
  and const_declaration =
    { const_abstract                                     : t
    ; const_keyword                                      : t
    ; const_type_specifier                               : t
    ; const_declarators                                  : t
    ; const_semicolon                                    : t
    }
  and constant_declarator =
    { constant_declarator_name                           : t
    ; constant_declarator_initializer                    : t
    }
  and type_const_declaration =
    { type_const_abstract                                : t
    ; type_const_keyword                                 : t
    ; type_const_type_keyword                            : t
    ; type_const_name                                    : t
    ; type_const_type_constraint                         : t
    ; type_const_equal                                   : t
    ; type_const_type_specifier                          : t
    ; type_const_semicolon                               : t
    }
  and decorated_expression =
    { decorated_expression_decorator                     : t
    ; decorated_expression_expression                    : t
    }
  and parameter_declaration =
    { parameter_attribute                                : t
    ; parameter_visibility                               : t
    ; parameter_type                                     : t
    ; parameter_name                                     : t
    ; parameter_default_value                            : t
    }
  and variadic_parameter =
    { variadic_parameter_ellipsis                        : t
    }
  and attribute_specification =
    { attribute_specification_left_double_angle          : t
    ; attribute_specification_attributes                 : t
    ; attribute_specification_right_double_angle         : t
    }
  and attribute =
    { attribute_name                                     : t
    ; attribute_left_paren                               : t
    ; attribute_values                                   : t
    ; attribute_right_paren                              : t
    }
  and inclusion_expression =
    { inclusion_require                                  : t
    ; inclusion_filename                                 : t
    }
  and inclusion_directive =
    { inclusion_expression                               : t
    ; inclusion_semicolon                                : t
    }
  and compound_statement =
    { compound_left_brace                                : t
    ; compound_statements                                : t
    ; compound_right_brace                               : t
    }
  and expression_statement =
    { expression_statement_expression                    : t
    ; expression_statement_semicolon                     : t
    }
  and markup_section =
    { markup_prefix                                      : t
    ; markup_text                                        : t
    ; markup_suffix                                      : t
    ; markup_expression                                  : t
    }
  and markup_suffix =
    { markup_suffix_less_than_question                   : t
    ; markup_suffix_name                                 : t
    }
  and unset_statement =
    { unset_keyword                                      : t
    ; unset_left_paren                                   : t
    ; unset_variables                                    : t
    ; unset_right_paren                                  : t
    ; unset_semicolon                                    : t
    }
  and while_statement =
    { while_keyword                                      : t
    ; while_left_paren                                   : t
    ; while_condition                                    : t
    ; while_right_paren                                  : t
    ; while_body                                         : t
    }
  and if_statement =
    { if_keyword                                         : t
    ; if_left_paren                                      : t
    ; if_condition                                       : t
    ; if_right_paren                                     : t
    ; if_statement                                       : t
    ; if_elseif_clauses                                  : t
    ; if_else_clause                                     : t
    }
  and elseif_clause =
    { elseif_keyword                                     : t
    ; elseif_left_paren                                  : t
    ; elseif_condition                                   : t
    ; elseif_right_paren                                 : t
    ; elseif_statement                                   : t
    }
  and else_clause =
    { else_keyword                                       : t
    ; else_statement                                     : t
    }
  and try_statement =
    { try_keyword                                        : t
    ; try_compound_statement                             : t
    ; try_catch_clauses                                  : t
    ; try_finally_clause                                 : t
    }
  and catch_clause =
    { catch_keyword                                      : t
    ; catch_left_paren                                   : t
    ; catch_type                                         : t
    ; catch_variable                                     : t
    ; catch_right_paren                                  : t
    ; catch_body                                         : t
    }
  and finally_clause =
    { finally_keyword                                    : t
    ; finally_body                                       : t
    }
  and do_statement =
    { do_keyword                                         : t
    ; do_body                                            : t
    ; do_while_keyword                                   : t
    ; do_left_paren                                      : t
    ; do_condition                                       : t
    ; do_right_paren                                     : t
    ; do_semicolon                                       : t
    }
  and for_statement =
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
  and foreach_statement =
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
  and switch_statement =
    { switch_keyword                                     : t
    ; switch_left_paren                                  : t
    ; switch_expression                                  : t
    ; switch_right_paren                                 : t
    ; switch_left_brace                                  : t
    ; switch_sections                                    : t
    ; switch_right_brace                                 : t
    }
  and switch_section =
    { switch_section_labels                              : t
    ; switch_section_statements                          : t
    ; switch_section_fallthrough                         : t
    }
  and switch_fallthrough =
    { fallthrough_keyword                                : t
    ; fallthrough_semicolon                              : t
    }
  and case_label =
    { case_keyword                                       : t
    ; case_expression                                    : t
    ; case_colon                                         : t
    }
  and default_label =
    { default_keyword                                    : t
    ; default_colon                                      : t
    }
  and return_statement =
    { return_keyword                                     : t
    ; return_expression                                  : t
    ; return_semicolon                                   : t
    }
  and goto_label =
    { goto_label_name                                    : t
    ; goto_label_colon                                   : t
    }
  and goto_statement =
    { goto_statement_keyword                             : t
    ; goto_statement_label_name                          : t
    ; goto_statement_semicolon                           : t
    }
  and throw_statement =
    { throw_keyword                                      : t
    ; throw_expression                                   : t
    ; throw_semicolon                                    : t
    }
  and break_statement =
    { break_keyword                                      : t
    ; break_level                                        : t
    ; break_semicolon                                    : t
    }
  and continue_statement =
    { continue_keyword                                   : t
    ; continue_level                                     : t
    ; continue_semicolon                                 : t
    }
  and function_static_statement =
    { static_static_keyword                              : t
    ; static_declarations                                : t
    ; static_semicolon                                   : t
    }
  and static_declarator =
    { static_name                                        : t
    ; static_initializer                                 : t
    }
  and echo_statement =
    { echo_keyword                                       : t
    ; echo_expressions                                   : t
    ; echo_semicolon                                     : t
    }
  and global_statement =
    { global_keyword                                     : t
    ; global_variables                                   : t
    ; global_semicolon                                   : t
    }
  and simple_initializer =
    { simple_initializer_equal                           : t
    ; simple_initializer_value                           : t
    }
  and anonymous_function =
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
  and anonymous_function_use_clause =
    { anonymous_use_keyword                              : t
    ; anonymous_use_left_paren                           : t
    ; anonymous_use_variables                            : t
    ; anonymous_use_right_paren                          : t
    }
  and lambda_expression =
    { lambda_async                                       : t
    ; lambda_coroutine                                   : t
    ; lambda_signature                                   : t
    ; lambda_arrow                                       : t
    ; lambda_body                                        : t
    }
  and lambda_signature =
    { lambda_left_paren                                  : t
    ; lambda_parameters                                  : t
    ; lambda_right_paren                                 : t
    ; lambda_colon                                       : t
    ; lambda_type                                        : t
    }
  and cast_expression =
    { cast_left_paren                                    : t
    ; cast_type                                          : t
    ; cast_right_paren                                   : t
    ; cast_operand                                       : t
    }
  and scope_resolution_expression =
    { scope_resolution_qualifier                         : t
    ; scope_resolution_operator                          : t
    ; scope_resolution_name                              : t
    }
  and member_selection_expression =
    { member_object                                      : t
    ; member_operator                                    : t
    ; member_name                                        : t
    }
  and safe_member_selection_expression =
    { safe_member_object                                 : t
    ; safe_member_operator                               : t
    ; safe_member_name                                   : t
    }
  and embedded_member_selection_expression =
    { embedded_member_object                             : t
    ; embedded_member_operator                           : t
    ; embedded_member_name                               : t
    }
  and yield_expression =
    { yield_keyword                                      : t
    ; yield_operand                                      : t
    }
  and yield_from_expression =
    { yield_from_yield_keyword                           : t
    ; yield_from_from_keyword                            : t
    ; yield_from_operand                                 : t
    }
  and prefix_unary_expression =
    { prefix_unary_operator                              : t
    ; prefix_unary_operand                               : t
    }
  and postfix_unary_expression =
    { postfix_unary_operand                              : t
    ; postfix_unary_operator                             : t
    }
  and binary_expression =
    { binary_left_operand                                : t
    ; binary_operator                                    : t
    ; binary_right_operand                               : t
    }
  and instanceof_expression =
    { instanceof_left_operand                            : t
    ; instanceof_operator                                : t
    ; instanceof_right_operand                           : t
    }
  and conditional_expression =
    { conditional_test                                   : t
    ; conditional_question                               : t
    ; conditional_consequence                            : t
    ; conditional_colon                                  : t
    ; conditional_alternative                            : t
    }
  and eval_expression =
    { eval_keyword                                       : t
    ; eval_left_paren                                    : t
    ; eval_argument                                      : t
    ; eval_right_paren                                   : t
    }
  and empty_expression =
    { empty_keyword                                      : t
    ; empty_left_paren                                   : t
    ; empty_argument                                     : t
    ; empty_right_paren                                  : t
    }
  and define_expression =
    { define_keyword                                     : t
    ; define_left_paren                                  : t
    ; define_argument_list                               : t
    ; define_right_paren                                 : t
    }
  and isset_expression =
    { isset_keyword                                      : t
    ; isset_left_paren                                   : t
    ; isset_argument_list                                : t
    ; isset_right_paren                                  : t
    }
  and function_call_expression =
    { function_call_receiver                             : t
    ; function_call_left_paren                           : t
    ; function_call_argument_list                        : t
    ; function_call_right_paren                          : t
    }
  and function_call_with_type_arguments_expression =
    { function_call_with_type_arguments_receiver         : t
    ; function_call_with_type_arguments_type_args        : t
    ; function_call_with_type_arguments_left_paren       : t
    ; function_call_with_type_arguments_argument_list    : t
    ; function_call_with_type_arguments_right_paren      : t
    }
  and parenthesized_expression =
    { parenthesized_expression_left_paren                : t
    ; parenthesized_expression_expression                : t
    ; parenthesized_expression_right_paren               : t
    }
  and braced_expression =
    { braced_expression_left_brace                       : t
    ; braced_expression_expression                       : t
    ; braced_expression_right_brace                      : t
    }
  and embedded_braced_expression =
    { embedded_braced_expression_left_brace              : t
    ; embedded_braced_expression_expression              : t
    ; embedded_braced_expression_right_brace             : t
    }
  and list_expression =
    { list_keyword                                       : t
    ; list_left_paren                                    : t
    ; list_members                                       : t
    ; list_right_paren                                   : t
    }
  and collection_literal_expression =
    { collection_literal_name                            : t
    ; collection_literal_left_brace                      : t
    ; collection_literal_initializers                    : t
    ; collection_literal_right_brace                     : t
    }
  and object_creation_expression =
    { object_creation_new_keyword                        : t
    ; object_creation_type                               : t
    ; object_creation_left_paren                         : t
    ; object_creation_argument_list                      : t
    ; object_creation_right_paren                        : t
    }
  and array_creation_expression =
    { array_creation_left_bracket                        : t
    ; array_creation_members                             : t
    ; array_creation_right_bracket                       : t
    }
  and array_intrinsic_expression =
    { array_intrinsic_keyword                            : t
    ; array_intrinsic_left_paren                         : t
    ; array_intrinsic_members                            : t
    ; array_intrinsic_right_paren                        : t
    }
  and darray_intrinsic_expression =
    { darray_intrinsic_keyword                           : t
    ; darray_intrinsic_left_bracket                      : t
    ; darray_intrinsic_members                           : t
    ; darray_intrinsic_right_bracket                     : t
    }
  and dictionary_intrinsic_expression =
    { dictionary_intrinsic_keyword                       : t
    ; dictionary_intrinsic_left_bracket                  : t
    ; dictionary_intrinsic_members                       : t
    ; dictionary_intrinsic_right_bracket                 : t
    }
  and keyset_intrinsic_expression =
    { keyset_intrinsic_keyword                           : t
    ; keyset_intrinsic_left_bracket                      : t
    ; keyset_intrinsic_members                           : t
    ; keyset_intrinsic_right_bracket                     : t
    }
  and varray_intrinsic_expression =
    { varray_intrinsic_keyword                           : t
    ; varray_intrinsic_left_bracket                      : t
    ; varray_intrinsic_members                           : t
    ; varray_intrinsic_right_bracket                     : t
    }
  and vector_intrinsic_expression =
    { vector_intrinsic_keyword                           : t
    ; vector_intrinsic_left_bracket                      : t
    ; vector_intrinsic_members                           : t
    ; vector_intrinsic_right_bracket                     : t
    }
  and element_initializer =
    { element_key                                        : t
    ; element_arrow                                      : t
    ; element_value                                      : t
    }
  and subscript_expression =
    { subscript_receiver                                 : t
    ; subscript_left_bracket                             : t
    ; subscript_index                                    : t
    ; subscript_right_bracket                            : t
    }
  and embedded_subscript_expression =
    { embedded_subscript_receiver                        : t
    ; embedded_subscript_left_bracket                    : t
    ; embedded_subscript_index                           : t
    ; embedded_subscript_right_bracket                   : t
    }
  and awaitable_creation_expression =
    { awaitable_async                                    : t
    ; awaitable_coroutine                                : t
    ; awaitable_compound_statement                       : t
    }
  and xhp_children_declaration =
    { xhp_children_keyword                               : t
    ; xhp_children_expression                            : t
    ; xhp_children_semicolon                             : t
    }
  and xhp_children_parenthesized_list =
    { xhp_children_list_left_paren                       : t
    ; xhp_children_list_xhp_children                     : t
    ; xhp_children_list_right_paren                      : t
    }
  and xhp_category_declaration =
    { xhp_category_keyword                               : t
    ; xhp_category_categories                            : t
    ; xhp_category_semicolon                             : t
    }
  and xhp_enum_type =
    { xhp_enum_keyword                                   : t
    ; xhp_enum_left_brace                                : t
    ; xhp_enum_values                                    : t
    ; xhp_enum_right_brace                               : t
    }
  and xhp_required =
    { xhp_required_at                                    : t
    ; xhp_required_keyword                               : t
    }
  and xhp_class_attribute_declaration =
    { xhp_attribute_keyword                              : t
    ; xhp_attribute_attributes                           : t
    ; xhp_attribute_semicolon                            : t
    }
  and xhp_class_attribute =
    { xhp_attribute_decl_type                            : t
    ; xhp_attribute_decl_name                            : t
    ; xhp_attribute_decl_initializer                     : t
    ; xhp_attribute_decl_required                        : t
    }
  and xhp_simple_class_attribute =
    { xhp_simple_class_attribute_type                    : t
    }
  and xhp_attribute =
    { xhp_attribute_name                                 : t
    ; xhp_attribute_equal                                : t
    ; xhp_attribute_expression                           : t
    }
  and xhp_open =
    { xhp_open_left_angle                                : t
    ; xhp_open_name                                      : t
    ; xhp_open_attributes                                : t
    ; xhp_open_right_angle                               : t
    }
  and xhp_expression =
    { xhp_open                                           : t
    ; xhp_body                                           : t
    ; xhp_close                                          : t
    }
  and xhp_close =
    { xhp_close_left_angle                               : t
    ; xhp_close_name                                     : t
    ; xhp_close_right_angle                              : t
    }
  and type_constant =
    { type_constant_left_type                            : t
    ; type_constant_separator                            : t
    ; type_constant_right_type                           : t
    }
  and vector_type_specifier =
    { vector_type_keyword                                : t
    ; vector_type_left_angle                             : t
    ; vector_type_type                                   : t
    ; vector_type_trailing_comma                         : t
    ; vector_type_right_angle                            : t
    }
  and keyset_type_specifier =
    { keyset_type_keyword                                : t
    ; keyset_type_left_angle                             : t
    ; keyset_type_type                                   : t
    ; keyset_type_trailing_comma                         : t
    ; keyset_type_right_angle                            : t
    }
  and tuple_type_explicit_specifier =
    { tuple_type_keyword                                 : t
    ; tuple_type_left_angle                              : t
    ; tuple_type_types                                   : t
    ; tuple_type_right_angle                             : t
    }
  and varray_type_specifier =
    { varray_keyword                                     : t
    ; varray_left_angle                                  : t
    ; varray_type                                        : t
    ; varray_trailing_comma                              : t
    ; varray_right_angle                                 : t
    }
  and vector_array_type_specifier =
    { vector_array_keyword                               : t
    ; vector_array_left_angle                            : t
    ; vector_array_type                                  : t
    ; vector_array_right_angle                           : t
    }
  and type_parameter =
    { type_variance                                      : t
    ; type_name                                          : t
    ; type_constraints                                   : t
    }
  and type_constraint =
    { constraint_keyword                                 : t
    ; constraint_type                                    : t
    }
  and darray_type_specifier =
    { darray_keyword                                     : t
    ; darray_left_angle                                  : t
    ; darray_key                                         : t
    ; darray_comma                                       : t
    ; darray_value                                       : t
    ; darray_trailing_comma                              : t
    ; darray_right_angle                                 : t
    }
  and map_array_type_specifier =
    { map_array_keyword                                  : t
    ; map_array_left_angle                               : t
    ; map_array_key                                      : t
    ; map_array_comma                                    : t
    ; map_array_value                                    : t
    ; map_array_right_angle                              : t
    }
  and dictionary_type_specifier =
    { dictionary_type_keyword                            : t
    ; dictionary_type_left_angle                         : t
    ; dictionary_type_members                            : t
    ; dictionary_type_right_angle                        : t
    }
  and closure_type_specifier =
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
  and classname_type_specifier =
    { classname_keyword                                  : t
    ; classname_left_angle                               : t
    ; classname_type                                     : t
    ; classname_trailing_comma                           : t
    ; classname_right_angle                              : t
    }
  and field_specifier =
    { field_question                                     : t
    ; field_name                                         : t
    ; field_arrow                                        : t
    ; field_type                                         : t
    }
  and field_initializer =
    { field_initializer_name                             : t
    ; field_initializer_arrow                            : t
    ; field_initializer_value                            : t
    }
  and shape_type_specifier =
    { shape_type_keyword                                 : t
    ; shape_type_left_paren                              : t
    ; shape_type_fields                                  : t
    ; shape_type_ellipsis                                : t
    ; shape_type_right_paren                             : t
    }
  and shape_expression =
    { shape_expression_keyword                           : t
    ; shape_expression_left_paren                        : t
    ; shape_expression_fields                            : t
    ; shape_expression_right_paren                       : t
    }
  and tuple_expression =
    { tuple_expression_keyword                           : t
    ; tuple_expression_left_paren                        : t
    ; tuple_expression_items                             : t
    ; tuple_expression_right_paren                       : t
    }
  and generic_type_specifier =
    { generic_class_type                                 : t
    ; generic_argument_list                              : t
    }
  and nullable_type_specifier =
    { nullable_question                                  : t
    ; nullable_type                                      : t
    }
  and soft_type_specifier =
    { soft_at                                            : t
    ; soft_type                                          : t
    }
  and type_arguments =
    { type_arguments_left_angle                          : t
    ; type_arguments_types                               : t
    ; type_arguments_right_angle                         : t
    }
  and type_parameters =
    { type_parameters_left_angle                         : t
    ; type_parameters_parameters                         : t
    ; type_parameters_right_angle                        : t
    }
  and tuple_type_specifier =
    { tuple_left_paren                                   : t
    ; tuple_types                                        : t
    ; tuple_right_paren                                  : t
    }
  and error =
    { error_error                                        : t
    }
  and list_item =
    { list_item                                          : t
    ; list_separator                                     : t
    }
  and syntax =
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
  | WhileStatement                          of while_statement
  | IfStatement                             of if_statement
  | ElseifClause                            of elseif_clause
  | ElseClause                              of else_clause
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

end (* MakeSyntaxType *)

module MakeValidated(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type 'a value = SyntaxValue.t * 'a
  (* TODO: Different styles of list seem to only happen in predetermined places,
   * so split this out again into specific variants
   *)
  type 'a listesque =
  | Syntactic of ('a value * Token.t option value) value list
  | NonSyntactic of 'a value list
  | MissingList
  | SingletonList of 'a value
  and top_level_declaration =
  | TLDEndOfFile          of end_of_file
  | TLDEnum               of enum_declaration
  | TLDAlias              of alias_declaration
  | TLDNamespace          of namespace_declaration
  | TLDNamespaceUse       of namespace_use_declaration
  | TLDNamespaceGroupUse  of namespace_group_use_declaration
  | TLDFunction           of function_declaration
  | TLDClassish           of classish_declaration
  | TLDConst              of const_declaration
  | TLDInclusionDirective of inclusion_directive
  | TLDCompound           of compound_statement
  | TLDExpression         of expression_statement
  | TLDMarkupSection      of markup_section
  | TLDMarkupSuffix       of markup_suffix
  | TLDUnset              of unset_statement
  | TLDWhile              of while_statement
  | TLDIf                 of if_statement
  | TLDTry                of try_statement
  | TLDDo                 of do_statement
  | TLDFor                of for_statement
  | TLDForeach            of foreach_statement
  | TLDSwitchFallthrough  of switch_fallthrough
  | TLDReturn             of return_statement
  | TLDGotoLabel          of goto_label
  | TLDGoto               of goto_statement
  | TLDThrow              of throw_statement
  | TLDBreak              of break_statement
  | TLDContinue           of continue_statement
  | TLDFunctionStatic     of function_static_statement
  | TLDEcho               of echo_statement
  | TLDGlobal             of global_statement
  and expression =
  | ExprLiteral                       of literal_expression
  | ExprVariable                      of variable_expression
  | ExprQualifiedName                 of qualified_name_expression
  | ExprPipeVariable                  of pipe_variable_expression
  | ExprDecorated                     of decorated_expression
  | ExprInclusion                     of inclusion_expression
  | ExprAnonymousFunction             of anonymous_function
  | ExprLambda                        of lambda_expression
  | ExprCast                          of cast_expression
  | ExprScopeResolution               of scope_resolution_expression
  | ExprMemberSelection               of member_selection_expression
  | ExprSafeMemberSelection           of safe_member_selection_expression
  | ExprEmbeddedMemberSelection       of embedded_member_selection_expression
  | ExprYield                         of yield_expression
  | ExprYieldFrom                     of yield_from_expression
  | ExprPrefixUnary                   of prefix_unary_expression
  | ExprPostfixUnary                  of postfix_unary_expression
  | ExprBinary                        of binary_expression
  | ExprInstanceof                    of instanceof_expression
  | ExprConditional                   of conditional_expression
  | ExprEval                          of eval_expression
  | ExprEmpty                         of empty_expression
  | ExprDefine                        of define_expression
  | ExprIsset                         of isset_expression
  | ExprFunctionCall                  of function_call_expression
  | ExprFunctionCallWithTypeArguments of function_call_with_type_arguments_expression
  | ExprParenthesized                 of parenthesized_expression
  | ExprBraced                        of braced_expression
  | ExprEmbeddedBraced                of embedded_braced_expression
  | ExprList                          of list_expression
  | ExprCollectionLiteral             of collection_literal_expression
  | ExprObjectCreation                of object_creation_expression
  | ExprArrayCreation                 of array_creation_expression
  | ExprArrayIntrinsic                of array_intrinsic_expression
  | ExprDarrayIntrinsic               of darray_intrinsic_expression
  | ExprDictionaryIntrinsic           of dictionary_intrinsic_expression
  | ExprKeysetIntrinsic               of keyset_intrinsic_expression
  | ExprVarrayIntrinsic               of varray_intrinsic_expression
  | ExprVectorIntrinsic               of vector_intrinsic_expression
  | ExprSubscript                     of subscript_expression
  | ExprEmbeddedSubscript             of embedded_subscript_expression
  | ExprAwaitableCreation             of awaitable_creation_expression
  | ExprXHPChildrenParenthesizedList  of xhp_children_parenthesized_list
  | ExprXHP                           of xhp_expression
  | ExprShape                         of shape_expression
  | ExprTuple                         of tuple_expression
  and specifier =
  | SpecSimple            of simple_type_specifier
  | SpecVariadicParameter of variadic_parameter
  | SpecLambdaSignature   of lambda_signature
  | SpecXHPEnumType       of xhp_enum_type
  | SpecVector            of vector_type_specifier
  | SpecKeyset            of keyset_type_specifier
  | SpecTupleTypeExplicit of tuple_type_explicit_specifier
  | SpecVarray            of varray_type_specifier
  | SpecVectorArray       of vector_array_type_specifier
  | SpecDarray            of darray_type_specifier
  | SpecMapArray          of map_array_type_specifier
  | SpecDictionary        of dictionary_type_specifier
  | SpecClosure           of closure_type_specifier
  | SpecClassname         of classname_type_specifier
  | SpecField             of field_specifier
  | SpecShape             of shape_type_specifier
  | SpecGeneric           of generic_type_specifier
  | SpecNullable          of nullable_type_specifier
  | SpecSoft              of soft_type_specifier
  | SpecTuple             of tuple_type_specifier
  and parameter =
  | ParamParameterDeclaration of parameter_declaration
  | ParamVariadicParameter    of variadic_parameter
  and class_body_declaration =
  | BodyProperty          of property_declaration
  | BodyMethodish         of methodish_declaration
  | BodyRequireClause     of require_clause
  | BodyConst             of const_declaration
  | BodyTypeConst         of type_const_declaration
  | BodyXHPChildren       of xhp_children_declaration
  | BodyXHPCategory       of xhp_category_declaration
  | BodyXHPClassAttribute of xhp_class_attribute_declaration
  and statement =
  | StmtInclusionDirective of inclusion_directive
  | StmtCompound           of compound_statement
  | StmtExpression         of expression_statement
  | StmtMarkupSection      of markup_section
  | StmtMarkupSuffix       of markup_suffix
  | StmtUnset              of unset_statement
  | StmtWhile              of while_statement
  | StmtIf                 of if_statement
  | StmtTry                of try_statement
  | StmtDo                 of do_statement
  | StmtFor                of for_statement
  | StmtForeach            of foreach_statement
  | StmtSwitch             of switch_statement
  | StmtSwitchFallthrough  of switch_fallthrough
  | StmtReturn             of return_statement
  | StmtGotoLabel          of goto_label
  | StmtGoto               of goto_statement
  | StmtThrow              of throw_statement
  | StmtBreak              of break_statement
  | StmtContinue           of continue_statement
  | StmtFunctionStatic     of function_static_statement
  | StmtEcho               of echo_statement
  | StmtGlobal             of global_statement
  | StmtTypeConstant       of type_constant
  and switch_label =
  | SwitchCase    of case_label
  | SwitchDefault of default_label
  and lambda_body =
  | LambdaLiteral                       of literal_expression
  | LambdaVariable                      of variable_expression
  | LambdaQualifiedName                 of qualified_name_expression
  | LambdaPipeVariable                  of pipe_variable_expression
  | LambdaDecorated                     of decorated_expression
  | LambdaInclusion                     of inclusion_expression
  | LambdaCompoundStatement             of compound_statement
  | LambdaAnonymousFunction             of anonymous_function
  | LambdaLambda                        of lambda_expression
  | LambdaCast                          of cast_expression
  | LambdaScopeResolution               of scope_resolution_expression
  | LambdaMemberSelection               of member_selection_expression
  | LambdaSafeMemberSelection           of safe_member_selection_expression
  | LambdaEmbeddedMemberSelection       of embedded_member_selection_expression
  | LambdaYield                         of yield_expression
  | LambdaYieldFrom                     of yield_from_expression
  | LambdaPrefixUnary                   of prefix_unary_expression
  | LambdaPostfixUnary                  of postfix_unary_expression
  | LambdaBinary                        of binary_expression
  | LambdaInstanceof                    of instanceof_expression
  | LambdaConditional                   of conditional_expression
  | LambdaEval                          of eval_expression
  | LambdaEmpty                         of empty_expression
  | LambdaDefine                        of define_expression
  | LambdaIsset                         of isset_expression
  | LambdaFunctionCall                  of function_call_expression
  | LambdaFunctionCallWithTypeArguments of function_call_with_type_arguments_expression
  | LambdaParenthesized                 of parenthesized_expression
  | LambdaBraced                        of braced_expression
  | LambdaEmbeddedBraced                of embedded_braced_expression
  | LambdaList                          of list_expression
  | LambdaCollectionLiteral             of collection_literal_expression
  | LambdaObjectCreation                of object_creation_expression
  | LambdaArrayCreation                 of array_creation_expression
  | LambdaArrayIntrinsic                of array_intrinsic_expression
  | LambdaDarrayIntrinsic               of darray_intrinsic_expression
  | LambdaDictionaryIntrinsic           of dictionary_intrinsic_expression
  | LambdaKeysetIntrinsic               of keyset_intrinsic_expression
  | LambdaVarrayIntrinsic               of varray_intrinsic_expression
  | LambdaVectorIntrinsic               of vector_intrinsic_expression
  | LambdaSubscript                     of subscript_expression
  | LambdaEmbeddedSubscript             of embedded_subscript_expression
  | LambdaAwaitableCreation             of awaitable_creation_expression
  | LambdaXHPChildrenParenthesizedList  of xhp_children_parenthesized_list
  | LambdaXHP                           of xhp_expression
  | LambdaShape                         of shape_expression
  | LambdaTuple                         of tuple_expression
  and constructor_expression =
  | CExprLiteral                       of literal_expression
  | CExprVariable                      of variable_expression
  | CExprQualifiedName                 of qualified_name_expression
  | CExprPipeVariable                  of pipe_variable_expression
  | CExprDecorated                     of decorated_expression
  | CExprInclusion                     of inclusion_expression
  | CExprAnonymousFunction             of anonymous_function
  | CExprLambda                        of lambda_expression
  | CExprCast                          of cast_expression
  | CExprScopeResolution               of scope_resolution_expression
  | CExprMemberSelection               of member_selection_expression
  | CExprSafeMemberSelection           of safe_member_selection_expression
  | CExprEmbeddedMemberSelection       of embedded_member_selection_expression
  | CExprYield                         of yield_expression
  | CExprYieldFrom                     of yield_from_expression
  | CExprPrefixUnary                   of prefix_unary_expression
  | CExprPostfixUnary                  of postfix_unary_expression
  | CExprBinary                        of binary_expression
  | CExprInstanceof                    of instanceof_expression
  | CExprConditional                   of conditional_expression
  | CExprEval                          of eval_expression
  | CExprEmpty                         of empty_expression
  | CExprDefine                        of define_expression
  | CExprIsset                         of isset_expression
  | CExprFunctionCall                  of function_call_expression
  | CExprFunctionCallWithTypeArguments of function_call_with_type_arguments_expression
  | CExprParenthesized                 of parenthesized_expression
  | CExprBraced                        of braced_expression
  | CExprEmbeddedBraced                of embedded_braced_expression
  | CExprList                          of list_expression
  | CExprCollectionLiteral             of collection_literal_expression
  | CExprObjectCreation                of object_creation_expression
  | CExprArrayCreation                 of array_creation_expression
  | CExprArrayIntrinsic                of array_intrinsic_expression
  | CExprDarrayIntrinsic               of darray_intrinsic_expression
  | CExprDictionaryIntrinsic           of dictionary_intrinsic_expression
  | CExprKeysetIntrinsic               of keyset_intrinsic_expression
  | CExprVarrayIntrinsic               of varray_intrinsic_expression
  | CExprVectorIntrinsic               of vector_intrinsic_expression
  | CExprElementInitializer            of element_initializer
  | CExprSubscript                     of subscript_expression
  | CExprEmbeddedSubscript             of embedded_subscript_expression
  | CExprAwaitableCreation             of awaitable_creation_expression
  | CExprXHPChildrenParenthesizedList  of xhp_children_parenthesized_list
  | CExprXHP                           of xhp_expression
  | CExprShape                         of shape_expression
  | CExprTuple                         of tuple_expression
  and namespace_internals =
  | NSINamespaceBody      of namespace_body
  | NSINamespaceEmptyBody of namespace_empty_body
  and todo_aggregate =
  | TODOEndOfFile of end_of_file
  and end_of_file =
    { end_of_file_token: Token.t value
    }
  and script =
    { script_declarations: top_level_declaration listesque value
    }
  and simple_type_specifier =
    { simple_type_specifier: Token.t value
    }
  and literal_expression =
    { literal_expression: expression listesque value
    }
  and variable_expression =
    { variable_expression: Token.t value
    }
  and qualified_name_expression =
    { qualified_name_expression: Token.t value
    }
  and pipe_variable_expression =
    { pipe_variable_expression: Token.t value
    }
  and enum_declaration =
    { enum_attribute_spec: attribute_specification option value
    ; enum_keyword: Token.t value
    ; enum_name: Token.t value
    ; enum_colon: Token.t value
    ; enum_base: specifier value
    ; enum_type: type_constraint option value
    ; enum_left_brace: Token.t value
    ; enum_enumerators: enumerator listesque value
    ; enum_right_brace: Token.t value
    }
  and enumerator =
    { enumerator_name: Token.t value
    ; enumerator_equal: Token.t value
    ; enumerator_value: expression value
    ; enumerator_semicolon: Token.t value
    }
  and alias_declaration =
    { alias_attribute_spec: attribute_specification option value
    ; alias_keyword: Token.t value
    ; alias_name: Token.t option value
    ; alias_generic_parameter: type_parameters option value
    ; alias_constraint: type_constraint option value
    ; alias_equal: Token.t option value
    ; alias_type: specifier value
    ; alias_semicolon: Token.t value
    }
  and property_declaration =
    { property_modifiers: Token.t listesque value
    ; property_type: specifier option value
    ; property_declarators: property_declarator listesque value
    ; property_semicolon: Token.t value
    }
  and property_declarator =
    { property_name: Token.t value
    ; property_initializer: simple_initializer option value
    }
  and namespace_declaration =
    { namespace_keyword: Token.t value
    ; namespace_name: Token.t option value
    ; namespace_body: namespace_internals value
    }
  and namespace_body =
    { namespace_left_brace: Token.t value
    ; namespace_declarations: top_level_declaration listesque value
    ; namespace_right_brace: Token.t value
    }
  and namespace_empty_body =
    { namespace_semicolon: Token.t value
    }
  and namespace_use_declaration =
    { namespace_use_keyword: Token.t value
    ; namespace_use_kind: Token.t option value
    ; namespace_use_clauses: (namespace_use_clause option) listesque value
    ; namespace_use_semicolon: Token.t option value
    }
  and namespace_group_use_declaration =
    { namespace_group_use_keyword: Token.t value
    ; namespace_group_use_kind: Token.t option value
    ; namespace_group_use_prefix: Token.t value
    ; namespace_group_use_left_brace: Token.t value
    ; namespace_group_use_clauses: namespace_use_clause listesque value
    ; namespace_group_use_right_brace: Token.t value
    ; namespace_group_use_semicolon: Token.t value
    }
  and namespace_use_clause =
    { namespace_use_clause_kind: Token.t option value
    ; namespace_use_name: Token.t value
    ; namespace_use_as: Token.t option value
    ; namespace_use_alias: Token.t option value
    }
  and function_declaration =
    { function_attribute_spec: attribute_specification option value
    ; function_declaration_header: function_declaration_header value
    ; function_body: compound_statement value
    }
  and function_declaration_header =
    { function_async: Token.t option value
    ; function_coroutine: Token.t option value
    ; function_keyword: Token.t value
    ; function_ampersand: Token.t option value
    ; function_name: Token.t value
    ; function_type_parameter_list: type_parameters option value
    ; function_left_paren: Token.t value
    ; function_parameter_list: (parameter option) listesque value
    ; function_right_paren: Token.t value
    ; function_colon: Token.t option value
    ; function_type: specifier option value
    ; function_where_clause: where_clause option value
    }
  and where_clause =
    { where_clause_keyword: Token.t value
    ; where_clause_constraints: where_constraint listesque value
    }
  and where_constraint =
    { where_constraint_left_type: specifier value
    ; where_constraint_operator: Token.t value
    ; where_constraint_right_type: specifier value
    }
  and methodish_declaration =
    { methodish_attribute: attribute_specification option value
    ; methodish_modifiers: Token.t listesque value
    ; methodish_function_decl_header: function_declaration_header value
    ; methodish_function_body: compound_statement option value
    ; methodish_semicolon: Token.t option value
    }
  and classish_declaration =
    { classish_attribute: attribute_specification option value
    ; classish_modifiers: Token.t listesque value
    ; classish_keyword: Token.t value
    ; classish_name: Token.t value
    ; classish_type_parameters: type_parameters option value
    ; classish_extends_keyword: Token.t option value
    ; classish_extends_list: specifier listesque value
    ; classish_implements_keyword: Token.t option value
    ; classish_implements_list: specifier listesque value
    ; classish_body: classish_body value
    }
  and classish_body =
    { classish_body_left_brace: Token.t value
    ; classish_body_elements: class_body_declaration listesque value
    ; classish_body_right_brace: Token.t value
    }
  and trait_use_precedence_item =
    { trait_use_precedence_item_name: specifier value
    ; trait_use_precedence_item_keyword: Token.t value
    ; trait_use_precedence_item_removed_names: specifier listesque value
    }
  and trait_use_alias_item =
    { trait_use_alias_item_aliasing_name: specifier value
    ; trait_use_alias_item_keyword: Token.t value
    ; trait_use_alias_item_visibility: Token.t option value
    ; trait_use_alias_item_aliased_name: specifier option value
    }
  and trait_use_conflict_resolution =
    { trait_use_conflict_resolution_keyword: Token.t value
    ; trait_use_conflict_resolution_names: specifier listesque value
    ; trait_use_conflict_resolution_left_brace: Token.t value
    ; trait_use_conflict_resolution_clauses: specifier listesque value
    ; trait_use_conflict_resolution_right_brace: Token.t value
    }
  and trait_use =
    { trait_use_keyword: Token.t value
    ; trait_use_names: specifier listesque value
    ; trait_use_semicolon: Token.t option value
    }
  and require_clause =
    { require_keyword: Token.t value
    ; require_kind: Token.t value
    ; require_name: specifier value
    ; require_semicolon: Token.t value
    }
  and const_declaration =
    { const_abstract: Token.t option value
    ; const_keyword: Token.t value
    ; const_type_specifier: specifier option value
    ; const_declarators: constant_declarator listesque value
    ; const_semicolon: Token.t value
    }
  and constant_declarator =
    { constant_declarator_name: Token.t value
    ; constant_declarator_initializer: simple_initializer option value
    }
  and type_const_declaration =
    { type_const_abstract: Token.t option value
    ; type_const_keyword: Token.t value
    ; type_const_type_keyword: Token.t value
    ; type_const_name: Token.t value
    ; type_const_type_constraint: type_constraint option value
    ; type_const_equal: Token.t option value
    ; type_const_type_specifier: specifier option value
    ; type_const_semicolon: Token.t value
    }
  and decorated_expression =
    { decorated_expression_decorator: Token.t value
    ; decorated_expression_expression: expression value
    }
  and parameter_declaration =
    { parameter_attribute: attribute_specification option value
    ; parameter_visibility: Token.t option value
    ; parameter_type: specifier option value
    ; parameter_name: expression value
    ; parameter_default_value: simple_initializer option value
    }
  and variadic_parameter =
    { variadic_parameter_ellipsis: Token.t value
    }
  and attribute_specification =
    { attribute_specification_left_double_angle: Token.t value
    ; attribute_specification_attributes: attribute listesque value
    ; attribute_specification_right_double_angle: Token.t value
    }
  and attribute =
    { attribute_name: Token.t value
    ; attribute_left_paren: Token.t option value
    ; attribute_values: expression listesque value
    ; attribute_right_paren: Token.t option value
    }
  and inclusion_expression =
    { inclusion_require: Token.t value
    ; inclusion_filename: expression value
    }
  and inclusion_directive =
    { inclusion_expression: inclusion_expression value
    ; inclusion_semicolon: Token.t value
    }
  and compound_statement =
    { compound_left_brace: Token.t value
    ; compound_statements: statement listesque value
    ; compound_right_brace: Token.t value
    }
  and expression_statement =
    { expression_statement_expression: expression option value
    ; expression_statement_semicolon: Token.t value
    }
  and markup_section =
    { markup_prefix: Token.t option value
    ; markup_text: Token.t value
    ; markup_suffix: markup_suffix option value
    ; markup_expression: expression option value
    }
  and markup_suffix =
    { markup_suffix_less_than_question: Token.t value
    ; markup_suffix_name: Token.t option value
    }
  and unset_statement =
    { unset_keyword: Token.t value
    ; unset_left_paren: Token.t value
    ; unset_variables: expression listesque value
    ; unset_right_paren: Token.t value
    ; unset_semicolon: Token.t value
    }
  and while_statement =
    { while_keyword: Token.t value
    ; while_left_paren: Token.t value
    ; while_condition: expression value
    ; while_right_paren: Token.t value
    ; while_body: statement value
    }
  and if_statement =
    { if_keyword: Token.t value
    ; if_left_paren: Token.t value
    ; if_condition: expression value
    ; if_right_paren: Token.t value
    ; if_statement: statement value
    ; if_elseif_clauses: elseif_clause listesque value
    ; if_else_clause: else_clause option value
    }
  and elseif_clause =
    { elseif_keyword: Token.t value
    ; elseif_left_paren: Token.t value
    ; elseif_condition: expression value
    ; elseif_right_paren: Token.t value
    ; elseif_statement: statement value
    }
  and else_clause =
    { else_keyword: Token.t value
    ; else_statement: statement value
    }
  and try_statement =
    { try_keyword: Token.t value
    ; try_compound_statement: compound_statement value
    ; try_catch_clauses: catch_clause listesque value
    ; try_finally_clause: finally_clause option value
    }
  and catch_clause =
    { catch_keyword: Token.t value
    ; catch_left_paren: Token.t value
    ; catch_type: simple_type_specifier value
    ; catch_variable: Token.t value
    ; catch_right_paren: Token.t value
    ; catch_body: compound_statement value
    }
  and finally_clause =
    { finally_keyword: Token.t value
    ; finally_body: compound_statement value
    }
  and do_statement =
    { do_keyword: Token.t value
    ; do_body: statement value
    ; do_while_keyword: Token.t value
    ; do_left_paren: Token.t value
    ; do_condition: expression value
    ; do_right_paren: Token.t value
    ; do_semicolon: Token.t value
    }
  and for_statement =
    { for_keyword: Token.t value
    ; for_left_paren: Token.t value
    ; for_initializer: expression listesque value
    ; for_first_semicolon: Token.t value
    ; for_control: expression listesque value
    ; for_second_semicolon: Token.t value
    ; for_end_of_loop: expression listesque value
    ; for_right_paren: Token.t value
    ; for_body: statement value
    }
  and foreach_statement =
    { foreach_keyword: Token.t value
    ; foreach_left_paren: Token.t value
    ; foreach_collection: expression value
    ; foreach_await_keyword: Token.t option value
    ; foreach_as: Token.t value
    ; foreach_key: expression option value
    ; foreach_arrow: Token.t option value
    ; foreach_value: expression value
    ; foreach_right_paren: Token.t value
    ; foreach_body: statement value
    }
  and switch_statement =
    { switch_keyword: Token.t value
    ; switch_left_paren: Token.t value
    ; switch_expression: expression value
    ; switch_right_paren: Token.t value
    ; switch_left_brace: Token.t value
    ; switch_sections: switch_section listesque value
    ; switch_right_brace: Token.t value
    }
  and switch_section =
    { switch_section_labels: switch_label listesque value
    ; switch_section_statements: top_level_declaration listesque value
    ; switch_section_fallthrough: switch_fallthrough option value
    }
  and switch_fallthrough =
    { fallthrough_keyword: Token.t value
    ; fallthrough_semicolon: Token.t value
    }
  and case_label =
    { case_keyword: Token.t value
    ; case_expression: expression value
    ; case_colon: Token.t value
    }
  and default_label =
    { default_keyword: Token.t value
    ; default_colon: Token.t value
    }
  and return_statement =
    { return_keyword: Token.t value
    ; return_expression: expression option value
    ; return_semicolon: Token.t option value
    }
  and goto_label =
    { goto_label_name: Token.t value
    ; goto_label_colon: Token.t value
    }
  and goto_statement =
    { goto_statement_keyword: Token.t value
    ; goto_statement_label_name: Token.t value
    ; goto_statement_semicolon: Token.t value
    }
  and throw_statement =
    { throw_keyword: Token.t value
    ; throw_expression: expression value
    ; throw_semicolon: Token.t value
    }
  and break_statement =
    { break_keyword: Token.t value
    ; break_level: literal_expression option value
    ; break_semicolon: Token.t value
    }
  and continue_statement =
    { continue_keyword: Token.t value
    ; continue_level: literal_expression option value
    ; continue_semicolon: Token.t value
    }
  and function_static_statement =
    { static_static_keyword: Token.t value
    ; static_declarations: static_declarator listesque value
    ; static_semicolon: Token.t value
    }
  and static_declarator =
    { static_name: Token.t value
    ; static_initializer: simple_initializer option value
    }
  and echo_statement =
    { echo_keyword: Token.t value
    ; echo_expressions: expression listesque value
    ; echo_semicolon: Token.t value
    }
  and global_statement =
    { global_keyword: Token.t value
    ; global_variables: Token.t listesque value
    ; global_semicolon: Token.t value
    }
  and simple_initializer =
    { simple_initializer_equal: Token.t value
    ; simple_initializer_value: expression value
    }
  and anonymous_function =
    { anonymous_static_keyword: Token.t option value
    ; anonymous_async_keyword: Token.t option value
    ; anonymous_coroutine_keyword: Token.t option value
    ; anonymous_function_keyword: Token.t value
    ; anonymous_left_paren: Token.t value
    ; anonymous_parameters: parameter listesque value
    ; anonymous_right_paren: Token.t value
    ; anonymous_colon: Token.t option value
    ; anonymous_type: specifier option value
    ; anonymous_use: anonymous_function_use_clause option value
    ; anonymous_body: compound_statement value
    }
  and anonymous_function_use_clause =
    { anonymous_use_keyword: Token.t value
    ; anonymous_use_left_paren: Token.t value
    ; anonymous_use_variables: expression listesque value
    ; anonymous_use_right_paren: Token.t value
    }
  and lambda_expression =
    { lambda_async: Token.t option value
    ; lambda_coroutine: Token.t option value
    ; lambda_signature: specifier value
    ; lambda_arrow: Token.t value
    ; lambda_body: lambda_body value
    }
  and lambda_signature =
    { lambda_left_paren: Token.t value
    ; lambda_parameters: parameter listesque value
    ; lambda_right_paren: Token.t value
    ; lambda_colon: Token.t option value
    ; lambda_type: specifier option value
    }
  and cast_expression =
    { cast_left_paren: Token.t value
    ; cast_type: Token.t value
    ; cast_right_paren: Token.t value
    ; cast_operand: expression value
    }
  and scope_resolution_expression =
    { scope_resolution_qualifier: expression value
    ; scope_resolution_operator: Token.t value
    ; scope_resolution_name: expression value
    }
  and member_selection_expression =
    { member_object: expression value
    ; member_operator: Token.t value
    ; member_name: Token.t value
    }
  and safe_member_selection_expression =
    { safe_member_object: expression value
    ; safe_member_operator: Token.t value
    ; safe_member_name: Token.t value
    }
  and embedded_member_selection_expression =
    { embedded_member_object: variable_expression value
    ; embedded_member_operator: Token.t value
    ; embedded_member_name: Token.t value
    }
  and yield_expression =
    { yield_keyword: Token.t value
    ; yield_operand: constructor_expression value
    }
  and yield_from_expression =
    { yield_from_yield_keyword: Token.t value
    ; yield_from_from_keyword: Token.t value
    ; yield_from_operand: expression value
    }
  and prefix_unary_expression =
    { prefix_unary_operator: Token.t value
    ; prefix_unary_operand: expression value
    }
  and postfix_unary_expression =
    { postfix_unary_operand: expression value
    ; postfix_unary_operator: Token.t value
    }
  and binary_expression =
    { binary_left_operand: expression value
    ; binary_operator: Token.t value
    ; binary_right_operand: expression value
    }
  and instanceof_expression =
    { instanceof_left_operand: expression value
    ; instanceof_operator: Token.t value
    ; instanceof_right_operand: expression value
    }
  and conditional_expression =
    { conditional_test: expression value
    ; conditional_question: Token.t value
    ; conditional_consequence: expression option value
    ; conditional_colon: Token.t value
    ; conditional_alternative: expression value
    }
  and eval_expression =
    { eval_keyword: Token.t value
    ; eval_left_paren: Token.t value
    ; eval_argument: expression value
    ; eval_right_paren: Token.t value
    }
  and empty_expression =
    { empty_keyword: Token.t value
    ; empty_left_paren: Token.t value
    ; empty_argument: expression value
    ; empty_right_paren: Token.t value
    }
  and define_expression =
    { define_keyword: Token.t value
    ; define_left_paren: Token.t value
    ; define_argument_list: expression listesque value
    ; define_right_paren: Token.t value
    }
  and isset_expression =
    { isset_keyword: Token.t value
    ; isset_left_paren: Token.t value
    ; isset_argument_list: expression listesque value
    ; isset_right_paren: Token.t value
    }
  and function_call_expression =
    { function_call_receiver: expression value
    ; function_call_left_paren: Token.t value
    ; function_call_argument_list: expression listesque value
    ; function_call_right_paren: Token.t value
    }
  and function_call_with_type_arguments_expression =
    { function_call_with_type_arguments_receiver: expression value
    ; function_call_with_type_arguments_type_args: type_arguments value
    ; function_call_with_type_arguments_left_paren: Token.t value
    ; function_call_with_type_arguments_argument_list: expression listesque value
    ; function_call_with_type_arguments_right_paren: Token.t value
    }
  and parenthesized_expression =
    { parenthesized_expression_left_paren: Token.t value
    ; parenthesized_expression_expression: expression value
    ; parenthesized_expression_right_paren: Token.t value
    }
  and braced_expression =
    { braced_expression_left_brace: Token.t value
    ; braced_expression_expression: expression value
    ; braced_expression_right_brace: Token.t value
    }
  and embedded_braced_expression =
    { embedded_braced_expression_left_brace: Token.t value
    ; embedded_braced_expression_expression: expression value
    ; embedded_braced_expression_right_brace: Token.t value
    }
  and list_expression =
    { list_keyword: Token.t value
    ; list_left_paren: Token.t value
    ; list_members: (expression option) listesque value
    ; list_right_paren: Token.t value
    }
  and collection_literal_expression =
    { collection_literal_name: Token.t value
    ; collection_literal_left_brace: Token.t value
    ; collection_literal_initializers: constructor_expression listesque value
    ; collection_literal_right_brace: Token.t value
    }
  and object_creation_expression =
    { object_creation_new_keyword: Token.t value
    ; object_creation_type: todo_aggregate value
    ; object_creation_left_paren: Token.t option value
    ; object_creation_argument_list: expression listesque value
    ; object_creation_right_paren: Token.t option value
    }
  and array_creation_expression =
    { array_creation_left_bracket: Token.t value
    ; array_creation_members: constructor_expression listesque value
    ; array_creation_right_bracket: Token.t value
    }
  and array_intrinsic_expression =
    { array_intrinsic_keyword: Token.t value
    ; array_intrinsic_left_paren: Token.t value
    ; array_intrinsic_members: constructor_expression listesque value
    ; array_intrinsic_right_paren: Token.t value
    }
  and darray_intrinsic_expression =
    { darray_intrinsic_keyword: Token.t value
    ; darray_intrinsic_left_bracket: Token.t value
    ; darray_intrinsic_members: element_initializer listesque value
    ; darray_intrinsic_right_bracket: Token.t value
    }
  and dictionary_intrinsic_expression =
    { dictionary_intrinsic_keyword: Token.t value
    ; dictionary_intrinsic_left_bracket: Token.t value
    ; dictionary_intrinsic_members: element_initializer listesque value
    ; dictionary_intrinsic_right_bracket: Token.t value
    }
  and keyset_intrinsic_expression =
    { keyset_intrinsic_keyword: Token.t value
    ; keyset_intrinsic_left_bracket: Token.t value
    ; keyset_intrinsic_members: expression listesque value
    ; keyset_intrinsic_right_bracket: Token.t value
    }
  and varray_intrinsic_expression =
    { varray_intrinsic_keyword: Token.t value
    ; varray_intrinsic_left_bracket: Token.t value
    ; varray_intrinsic_members: expression listesque value
    ; varray_intrinsic_right_bracket: Token.t value
    }
  and vector_intrinsic_expression =
    { vector_intrinsic_keyword: Token.t value
    ; vector_intrinsic_left_bracket: Token.t value
    ; vector_intrinsic_members: expression listesque value
    ; vector_intrinsic_right_bracket: Token.t value
    }
  and element_initializer =
    { element_key: expression value
    ; element_arrow: Token.t value
    ; element_value: expression value
    }
  and subscript_expression =
    { subscript_receiver: expression value
    ; subscript_left_bracket: Token.t value
    ; subscript_index: expression option value
    ; subscript_right_bracket: Token.t value
    }
  and embedded_subscript_expression =
    { embedded_subscript_receiver: variable_expression value
    ; embedded_subscript_left_bracket: Token.t value
    ; embedded_subscript_index: expression value
    ; embedded_subscript_right_bracket: Token.t value
    }
  and awaitable_creation_expression =
    { awaitable_async: Token.t value
    ; awaitable_coroutine: Token.t option value
    ; awaitable_compound_statement: compound_statement value
    }
  and xhp_children_declaration =
    { xhp_children_keyword: Token.t value
    ; xhp_children_expression: expression value
    ; xhp_children_semicolon: Token.t value
    }
  and xhp_children_parenthesized_list =
    { xhp_children_list_left_paren: Token.t value
    ; xhp_children_list_xhp_children: expression listesque value
    ; xhp_children_list_right_paren: Token.t value
    }
  and xhp_category_declaration =
    { xhp_category_keyword: Token.t value
    ; xhp_category_categories: Token.t listesque value
    ; xhp_category_semicolon: Token.t value
    }
  and xhp_enum_type =
    { xhp_enum_keyword: Token.t value
    ; xhp_enum_left_brace: Token.t value
    ; xhp_enum_values: literal_expression listesque value
    ; xhp_enum_right_brace: Token.t value
    }
  and xhp_required =
    { xhp_required_at: Token.t value
    ; xhp_required_keyword: Token.t value
    }
  and xhp_class_attribute_declaration =
    { xhp_attribute_keyword: Token.t value
    ; xhp_attribute_attributes: todo_aggregate listesque value
    ; xhp_attribute_semicolon: Token.t value
    }
  and xhp_class_attribute =
    { xhp_attribute_decl_type: specifier value
    ; xhp_attribute_decl_name: Token.t value
    ; xhp_attribute_decl_initializer: simple_initializer option value
    ; xhp_attribute_decl_required: xhp_required option value
    }
  and xhp_simple_class_attribute =
    { xhp_simple_class_attribute_type: simple_type_specifier value
    }
  and xhp_attribute =
    { xhp_attribute_name: Token.t value
    ; xhp_attribute_equal: Token.t value
    ; xhp_attribute_expression: expression value
    }
  and xhp_open =
    { xhp_open_left_angle: Token.t value
    ; xhp_open_name: Token.t value
    ; xhp_open_attributes: xhp_attribute listesque value
    ; xhp_open_right_angle: Token.t value
    }
  and xhp_expression =
    { xhp_open: xhp_open value
    ; xhp_body: expression listesque value
    ; xhp_close: xhp_close option value
    }
  and xhp_close =
    { xhp_close_left_angle: Token.t value
    ; xhp_close_name: Token.t value
    ; xhp_close_right_angle: Token.t value
    }
  and type_constant =
    { type_constant_left_type: specifier value
    ; type_constant_separator: Token.t value
    ; type_constant_right_type: Token.t value
    }
  and vector_type_specifier =
    { vector_type_keyword: Token.t value
    ; vector_type_left_angle: Token.t value
    ; vector_type_type: specifier value
    ; vector_type_trailing_comma: Token.t option value
    ; vector_type_right_angle: Token.t value
    }
  and keyset_type_specifier =
    { keyset_type_keyword: Token.t value
    ; keyset_type_left_angle: Token.t value
    ; keyset_type_type: specifier value
    ; keyset_type_trailing_comma: Token.t option value
    ; keyset_type_right_angle: Token.t value
    }
  and tuple_type_explicit_specifier =
    { tuple_type_keyword: Token.t value
    ; tuple_type_left_angle: Token.t value
    ; tuple_type_types: simple_type_specifier value
    ; tuple_type_right_angle: Token.t value
    }
  and varray_type_specifier =
    { varray_keyword: Token.t value
    ; varray_left_angle: Token.t value
    ; varray_type: simple_type_specifier value
    ; varray_trailing_comma: Token.t option value
    ; varray_right_angle: Token.t value
    }
  and vector_array_type_specifier =
    { vector_array_keyword: Token.t value
    ; vector_array_left_angle: Token.t value
    ; vector_array_type: specifier value
    ; vector_array_right_angle: Token.t value
    }
  and type_parameter =
    { type_variance: Token.t option value
    ; type_name: Token.t value
    ; type_constraints: type_constraint listesque value
    }
  and type_constraint =
    { constraint_keyword: Token.t value
    ; constraint_type: specifier value
    }
  and darray_type_specifier =
    { darray_keyword: Token.t value
    ; darray_left_angle: Token.t value
    ; darray_key: simple_type_specifier value
    ; darray_comma: Token.t value
    ; darray_value: simple_type_specifier value
    ; darray_trailing_comma: Token.t option value
    ; darray_right_angle: Token.t value
    }
  and map_array_type_specifier =
    { map_array_keyword: Token.t value
    ; map_array_left_angle: Token.t value
    ; map_array_key: specifier value
    ; map_array_comma: Token.t value
    ; map_array_value: specifier value
    ; map_array_right_angle: Token.t value
    }
  and dictionary_type_specifier =
    { dictionary_type_keyword: Token.t value
    ; dictionary_type_left_angle: Token.t value
    ; dictionary_type_members: specifier listesque value
    ; dictionary_type_right_angle: Token.t value
    }
  and closure_type_specifier =
    { closure_outer_left_paren: Token.t value
    ; closure_coroutine: Token.t option value
    ; closure_function_keyword: Token.t value
    ; closure_inner_left_paren: Token.t value
    ; closure_parameter_types: specifier listesque value
    ; closure_inner_right_paren: Token.t value
    ; closure_colon: Token.t value
    ; closure_return_type: specifier value
    ; closure_outer_right_paren: Token.t value
    }
  and classname_type_specifier =
    { classname_keyword: Token.t value
    ; classname_left_angle: Token.t value
    ; classname_type: specifier value
    ; classname_trailing_comma: Token.t option value
    ; classname_right_angle: Token.t value
    }
  and field_specifier =
    { field_question: Token.t option value
    ; field_name: expression value
    ; field_arrow: Token.t value
    ; field_type: specifier value
    }
  and field_initializer =
    { field_initializer_name: expression value
    ; field_initializer_arrow: Token.t value
    ; field_initializer_value: expression value
    }
  and shape_type_specifier =
    { shape_type_keyword: Token.t value
    ; shape_type_left_paren: Token.t value
    ; shape_type_fields: field_specifier listesque value
    ; shape_type_ellipsis: Token.t option value
    ; shape_type_right_paren: Token.t value
    }
  and shape_expression =
    { shape_expression_keyword: Token.t value
    ; shape_expression_left_paren: Token.t value
    ; shape_expression_fields: field_initializer listesque value
    ; shape_expression_right_paren: Token.t value
    }
  and tuple_expression =
    { tuple_expression_keyword: Token.t value
    ; tuple_expression_left_paren: Token.t value
    ; tuple_expression_items: expression listesque value
    ; tuple_expression_right_paren: Token.t value
    }
  and generic_type_specifier =
    { generic_class_type: Token.t value
    ; generic_argument_list: type_arguments value
    }
  and nullable_type_specifier =
    { nullable_question: Token.t value
    ; nullable_type: specifier value
    }
  and soft_type_specifier =
    { soft_at: Token.t value
    ; soft_type: specifier value
    }
  and type_arguments =
    { type_arguments_left_angle: Token.t value
    ; type_arguments_types: specifier listesque value
    ; type_arguments_right_angle: Token.t value
    }
  and type_parameters =
    { type_parameters_left_angle: Token.t value
    ; type_parameters_parameters: type_parameter listesque value
    ; type_parameters_right_angle: Token.t value
    }
  and tuple_type_specifier =
    { tuple_left_paren: Token.t value
    ; tuple_types: (specifier option) listesque value
    ; tuple_right_paren: Token.t value
    }

end (* MakeValidated *)
