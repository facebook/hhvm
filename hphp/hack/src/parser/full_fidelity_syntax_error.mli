(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error_type =
  | ParseError
  | RuntimeError
[@@deriving show]

type t = {
  child: t option;
  start_offset: int;
  end_offset: int;
  error_type: error_type;
  message: string;
}
[@@deriving show]

exception ParserFatal of t * Pos.t

val make :
  ?child:t option -> ?error_type:error_type -> int -> int -> string -> t

val to_positioned_string : t -> (int -> int * int) -> string

val compare : t -> t -> int

val exactly_equal : t -> t -> bool

val error_type : t -> error_type

val message : t -> string

val start_offset : t -> int

val end_offset : t -> int

val error0001 : string

val error0002 : string

val error0003 : string

val error0006 : string

val error0007 : string

val error0008 : string

val error0010 : string

val error0011 : string

val error0012 : string

val error0013 : string

val error0014 : string

(* Syntactic errors *)
val error1001 : string

val error1003 : string

val error1004 : string

val error1006 : string

val error1007 : string

val error1008 : string

val error1010 : string

val error1011 : string

val error1013 : string

val error1014 : string

val error1015 : string

val error1016 : string

val error1017 : string

val error1018 : string

val error1019 : string

val error1020 : string

val error1021 : string

val error1022 : string

val error1023 : string

val error1025 : string

val error1026 : string

val error1028 : string

val error1029 : string

val error1031 : string

val error1032 : string

val error1033 : string

val error1034 : string

val error1035 : string

val error1036 : string

val error1037 : string

val error1038 : string

val error1039 : string

val error1041 : string

val error1044 : string

val error1045 : string

val error1046 : string

val error1047 : string

val error1048 : string

val error1050 : string

val error1051 : string

val error1052 : string

val error1053 : string

val error1054 : string

val error1055 : string

val error1056 : string

val error1057 : string -> string

val error1058 : string -> string -> string

val error1059 : Full_fidelity_token_kind.t -> string

val error1060 : string

val error1061 : string

val error1062 : string

val error2001 : string

val error2003 : string

val error2004 : string

val error2005 : string

val error2006 : string

val error2007 : string

val error2008 : string

val error2009 : string -> string -> string

val error2010 : string

val error2013 : string

val error2014 : string

val error2015 : string -> string -> string

val error2016 : string -> string -> string

val error2017 : string

val error2018 : string

val error2019 : string -> string -> string

val error2020 : string

val error2021 : string

val error2022 : string

val error2023 : string

val error2024 : string

val error2025 : string -> string -> string

val error2029 : string

val error2030 : string

val error2031 : string

val error2032 : string

val error2033 : string

val error2034 : string

val error2035 : string

val error2036 : string

val error2037 : string

val error2038 : string -> string

val error2039 : string -> string -> string -> string

val error2040 : string

val error2041 : string

val error2042 : string

val error2043 : string

val error2044 : string -> string -> string

val error2045 : string

val error2046 : string -> string

val error2047 : string -> string

val error2048 : string

val error2049 : string

val error2050 : string

val const_static_prop_init : string

val error2051 : string

val abstract_prop_init : string

val error2052 : string

val error2053 : string

val error2054 : string

val error2055 : string

val error2056 : string

val error2057 : string

val error2058 : string

val shape_field_int_like_string : string

val error2061 : string

val error2062 : string

val error2063 : string

val error2064 : string

val error2065 : string

val error2066 : string

val error2067 : string

val error2068 : string

val error2070 : open_tag:string -> close_tag:string -> string

val error2071 : string -> string

val error2072 : string -> string

val error2073 : string

val error2074 : string -> string

val error2075 : string -> string

val error2076 : string

val error2077 : string

val error2078 : string

(* Start giving names rather than numbers *)
val list_must_be_lvar : string

val async_not_last : string

val uppercase_kw : string -> string

val using_st_function_scoped_top_level : string

val const_in_trait : string

val const_visibility : string

val type_const_visibility : string

val strict_namespace_hh : string

val original_definition : string

val name_is_already_in_use_php : name:string -> short_name:string -> string

val name_is_already_in_use_hh :
  line_num:int -> name:string -> short_name:string -> string

val name_is_already_in_use_implicit_hh :
  line_num:int -> name:string -> short_name:string -> string

val declared_name_is_already_in_use_implicit_hh :
  line_num:int -> name:string -> short_name:string -> string

val declared_name_is_already_in_use :
  line_num:int -> name:string -> short_name:string -> string

val namespace_name_is_already_in_use :
  name:string -> short_name:string -> string

val function_name_is_already_in_use :
  name:string -> short_name:string -> string

val const_name_is_already_in_use : name:string -> short_name:string -> string

val type_name_is_already_in_use : name:string -> short_name:string -> string

val variadic_reference : string

val reference_variadic : string

val double_variadic : string

val double_reference : string

val global_in_const_decl : string

val conflicting_trait_require_clauses : name:string -> string

val shape_type_ellipsis_without_trailing_comma : string

val yield_in_magic_methods : string

val reference_not_allowed_on_key : string

val reference_not_allowed_on_value : string

val reference_not_allowed_on_element : string

val yield_outside_function : string

val reference_param_in_construct : string

val coloncolonclass_on_dynamic : string

val enum_elem_name_is_class : string

val expected_as_or_insteadof : string

val expected_dotdotdot : string

val not_allowed_in_write : string -> string

val references_not_allowed : string

val reassign_this : string

val this_in_static : string

val async_magic_method : name:string -> string

val call_static_method : string

val reserved_keyword_as_class_name : string -> string

val inout_param_in_async_generator : string

val inout_param_in_generator : string

val inout_param_in_async : string

val inout_param_in_construct : string

val fun_arg_inout_set : string

val fun_arg_inout_const : string

val fun_arg_invalid_arg : string

val fun_arg_inout_containers : string

val memoize_with_inout : string

val fn_with_inout_and_ref_params : string

val method_calls_on_xhp_attributes : string

val invalid_constant_initializer : string

val no_args_in_halt_compiler : string

val no_async_before_lambda_body : string

val halt_compiler_top_level_only : string

val trait_alias_rule_allows_only_final_and_visibility_modifiers : string

val namespace_decl_first_statement : string

val code_outside_namespace : string

val invalid_number_of_args : string -> int -> string

val invalid_args_by_ref : string -> string

val redeclaration_error : string -> string

val class_with_abstract_method : string -> string

val interface_has_private_method : string

val redeclaration_of_function : name:string -> loc:string -> string

val redeclaration_of_method : name:string -> string

val self_or_parent_colon_colon_class_outside_of_class : string -> string

val invalid_is_as_expression_hint : string -> string -> string

val elvis_operator_space : string

val property_has_multiple_visibilities : string -> string

val property_has_multiple_modifiers : string -> string

val property_requires_visibility : string

val autoload_takes_one_argument : string

val clone_takes_no_arguments : string -> string -> string

val clone_cannot_be_static : string -> string -> string

val namespace_not_a_classname : string

val parent_static_const_decl : string

val parent_static_prop_decl : string

val xhp_class_multiple_category_decls : string

val xhp_class_multiple_children_decls : string

val missing_double_quote : string

val for_with_as_expression : string

val sealed_val_not_classname : string

val sealed_final : string

val sealed_enum : string

val interface_implements : string

val memoize_on_lambda : string

val memoize_lsb_on_non_static : string

val memoize_lsb_on_non_method : string

val constants_as_attribute_arguments : string

val instanceof_invalid_scope_resolution : string

val instanceof_memberselection_inside_scoperesolution : string

val instanceof_missing_subscript_index : string

val instanceof_new_unknown_node : string -> string

val instanceof_reference : string

val instanceof_disabled : string

val invalid_await_use : string

val toplevel_await_use : string

val invalid_constructor_method_call : string

val invalid_foreach_element : string

val invalid_scope_resolution_qualifier : string

val invalid_variable_name : string

val invalid_variable_variable : string

val function_modifier : string -> string

val invalid_yield : string

val invalid_yield_from : string

val invalid_class_in_collection_initializer : string

val invalid_brace_kind_in_collection_initializer : string

val nested_ternary : string

val alternate_control_flow : string

val execution_operator : string

val goto : string

val goto_label : string

val invalid_octal_integer : string

val prefixed_invalid_string_kind : string

val non_re_prefix : string

val collection_intrinsic_generic : string

val collection_intrinsic_many_typeargs : string

val invalid_shape_field_name : string

val invalid_reference : string

val invalid_hack_mode : string

val empty_method_name : string

val pair_initializer_needed : string

val pair_initializer_arity : string

val nested_unary_reference : string

val toplevel_statements : string

val invalid_reified : string

val reified_in_invalid_classish : string -> string

val shadowing_reified : string

val static_property_in_reified_class : string

val cls_reified_generic_in_static_method : string

val static_method_reified_obj_creation : string

val non_invariant_reified_generic : string

val no_generics_on_constructors : string

val no_type_parameters_on_dynamic_method_calls : string

val dollar_unary : string

val decl_outside_global_scope : string

val experimental_in_codegen_without_hacksperimental : string

val expected_simple_offset_expression : string

val illegal_interpolated_brace_with_embedded_dollar_expression : string

val type_alias_to_type_constant : string

val method_calls_on_xhp_expression : string

val interface_with_memoize : string

val xhp_class_attribute_type_constant : string

val inline_function_def : string

val lowering_parsing_error : string -> string -> string

val multiple_reactivity_annotations : string

val functions_cannot_implement_reactive : string

val missing_reactivity_for_condition : string

val misplaced_owned_mutable : string

val conflicting_mutable_and_owned_mutable_attributes : string

val conflicting_mutable_and_maybe_mutable_attributes : string

val conflicting_owned_mutable_and_maybe_mutable_attributes : string

val mutably_owned_attribute_on_non_rx_function : string

val invalid_non_rx_argument_for_lambda : string

val invalid_non_rx_argument_for_declaration : string

val nested_concurrent_blocks : string

val fewer_than_two_statements_in_concurrent_block : string

val invalid_syntax_concurrent_block : string

val statement_without_await_in_concurrent_block : string

val concurrent_is_disabled : string

val static_closures_are_disabled : string

val halt_compiler_is_disabled : string

val invalid_await_position : string

val invalid_await_position_pipe : string

val invalid_await_position_dependent : string

val misplaced_reactivity_annotation : string

val mutability_annotation_on_constructor : string

val mutability_annotation_on_static_method : string

val mutability_annotation_on_inout_parameter : string

val mutable_parameter_in_memoize_function : is_this:bool -> string

val mutable_return_in_memoize_function : string

val vararg_and_mutable : string

val expected_user_attribute : string

val tparams_in_tconst : string

val targs_not_allowed : string

val reified_attribute : string

val lval_as_expression : string

val pocket_universe_final_expected : string

val pocket_universe_enum_expected : string

val pocket_universe_invalid_field : string

val type_keyword : string

val non_public_const_in_interface : string

val elt_abstract_private : string -> string

val const_has_multiple_visibilities : string

val const_has_duplicate_modifiers : string

val only_soft_allowed : string

val soft_no_arguments : string

val no_legacy_soft_typehints : string

val static_const : string

val outside_dollar_str_interp : string

val no_const_interfaces_traits_enums : string

val no_const_late_init_props : string

val no_const_static_props : string

val no_const_abstract_final_class : string

val no_legacy_attribute_syntax : string

val no_silence : string

val declared_final : string -> string

val abstract_instance_property : string

val const_mutation : string

val no_attributes_on_variadic_parameter : string

val globals_without_subscript : string
