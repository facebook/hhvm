(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type error_type = ParseError | RuntimeError [@@deriving show]

type t = {
  child        : t option;
  start_offset : int;
  end_offset   : int;
  error_type   : error_type;
  message      : string;
} [@@deriving show]

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

val error2001 : string
val error2003 : string
val error2004 : string
val error2005 : string
val error2006 : string
val error2007 : string
val error2008 : string
val error2009 : string -> string -> string
val error2010 : string
val error2011 : string
val error2012 : string
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
val error2046 : string
val error2047 : string -> string
val error2048 : string
val error2049 : string
val error2050 : string
val error2051 : string
val error2052 : string
val error2053 : string
val error2054 : string
val error2055 : string
val error2056 : string
val error2057 : string
val error2058 : string
val error2060 : string
val error2061 : string
val error2062 : string
val error2063 : string
val error2064 : string
val error2065 : string
val error2066 : string
val error2067 : string
val error2068 : string
val error2069 : string
val error2070 : open_tag:string -> close_tag:string -> string
val error2071 : string -> string
val error2072 : string -> string
val error2073 : string
val error2074 : string -> string
val error2075 : string -> string
val error2076 : string
val error2077 : string

(* Start giving names rather than numbers *)
val async_not_last : string
val list_as_subscript : string
val vdarray_in_php : string
val using_st_function_scoped_top_level : string
val const_in_trait : string
val const_visibility : string
val strict_namespace_hh : string
val strict_namespace_not_hh : string
val original_definition : string
val name_is_already_in_use_php :
  name:string -> short_name:string -> string
val name_is_already_in_use_hh :
  line_num:int -> name:string -> short_name:string -> string
val name_is_already_in_use_implicit_hh :
  line_num:int -> name:string -> short_name:string -> string
val declared_name_is_already_in_use_implicit_hh :
  line_num:int -> name:string -> short_name:string -> string
val declared_name_is_already_in_use :
  line_num:int -> name:string -> short_name:string -> string
val namespace_name_is_already_in_use : name:string -> short_name:string -> string
val function_name_is_already_in_use : name:string -> short_name:string -> string
val const_name_is_already_in_use : name:string -> short_name:string -> string
val type_name_is_already_in_use : name:string -> short_name:string -> string
val variadic_reference : string
val double_variadic : string
val double_reference : string
val global_in_const_decl : string
val conflicting_trait_require_clauses : name:string -> string
val shape_type_ellipsis_without_trailing_comma: string
val yield_in_magic_methods : string
val reference_not_allowed_on_key : string
val reference_not_allowed_on_value : string
val reference_not_allowed_on_element : string
val yield_outside_function : string
val coloncolonclass_on_dynamic : string
val enum_elem_name_is_class : string
val expected_as_or_insteadof : string
val expected_dotdotdot : string
val not_allowed_in_write : string -> string
val reassign_this : string
val strict_types_first_statement : string
val async_magic_method : name:string -> string
val reserved_keyword_as_class_name : string
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
val strict_types_in_declare_block_mode : string
val invalid_number_of_args : string -> int -> string
val invalid_args_by_ref : string -> string
val redeclaration_error : string -> string
val reference_to_static_scope_resolution : string
val class_with_abstract_method : string -> string
val interface_has_private_method : string
val redeclaration_of_function : name:string -> loc:string -> string
val redeclaration_of_method : name:string -> string
val self_or_parent_colon_colon_class_outside_of_class : string -> string
val variadic_param_with_type_in_php : string -> string -> string
val final_property : string
val var_property : string
val invalid_is_as_expression_hint : string -> string -> string
val elvis_operator_space : string
val property_has_multiple_visibilities : string -> string
val autoload_takes_one_argument : string
val clone_destruct_takes_no_arguments : string -> string -> string
val class_destructor_cannot_be_static : string -> string -> string
val clone_cannot_be_static : string -> string -> string
val namespace_not_a_classname : string
val parent_static_const_decl : string
val xhp_class_multiple_category_decls: string
val missing_double_quote: string
val for_with_as_expression: string
val sealed_val_not_classname: string
val sealed_final: string
val interface_implements: string
val memoize_on_lambda: string
val memoize_lsb_on_non_static: string
val memoize_lsb_on_non_method: string
val instanceof_paren: string -> string
val instanceof_invalid_scope_resolution: string
val instanceof_memberselection_inside_scoperesolution: string
val instanceof_missing_subscript_index: string
val instanceof_unknown_node: string -> string
val instanceof_reference: string
val invalid_await_use: string
val invalid_constructor_method_call: string
val invalid_default_argument: string -> string
val do_not_use_xor : string
val do_not_use_or : string
val do_not_use_and : string
val invalid_foreach_element: string
val invalid_scope_resolution_qualifier : string
val invalid_variable_name : string
val function_modifier : string -> string
val invalid_yield : string
val invalid_yield_from : string
val invalid_class_in_collection_initializer: string
val invalid_brace_kind_in_collection_initializer: string
val alternate_control_flow : string
val execution_operator : string
val invalid_octal_integer : string
val php7_anonymous_function : string
val prefixed_invalid_string_kind : string
val non_re_prefix : string
val collection_intrinsic_generic: string
val invalid_shape_field_name : string
val incorrect_byref_assignment : string
val invalid_hack_mode : string
val pair_initializer_needed : string
val pair_initializer_arity : string
val nested_unary_reference : string
val toplevel_statements : string
val invalid_reified : string
val shadowing_reified : string
val dollar_unary : string
val decl_outside_global_scope : string
val experimental_in_codegen_without_hacksperimental : string
