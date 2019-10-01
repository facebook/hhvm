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

val error1001 : string

val error1060 : string

val shape_field_int_like_string : string

val shape_type_ellipsis_without_trailing_comma : string

val this_in_static : string

val autoload_takes_one_argument : string

val constants_as_attribute_arguments : string

val toplevel_await_use : string

val invalid_constructor_method_call : string

val invalid_variable_name : string

val invalid_variable_variable : string

val function_modifier : string -> string

val invalid_yield : string

val invalid_yield_from : string

val goto : string

val goto_label : string

val invalid_octal_integer : string

val non_re_prefix : string

val collection_intrinsic_many_typeargs : string

val invalid_shape_field_name : string

val empty_method_name : string

val toplevel_statements : string

val invalid_reified : string

val cls_reified_generic_in_static_method : string

val static_method_reified_obj_creation : string

val non_invariant_reified_generic : string

val no_generics_on_constructors : string

val experimental_in_codegen_without_hacksperimental : string

val xhp_class_attribute_type_constant : string

val lowering_parsing_error : string -> string -> string

val static_closures_are_disabled : string

val halt_compiler_is_disabled : string

val tparams_in_tconst : string

val targs_not_allowed : string

val reified_attribute : string

val only_soft_allowed : string

val soft_no_arguments : string

val outside_dollar_str_interp : string

val no_silence : string

val declared_final : string -> string

val const_mutation : string

val no_attributes_on_variadic_parameter : string

val statement_without_await_in_concurrent_block : string

val trait_alias_rule_allows_only_final_and_visibility_modifiers : string

val invalid_typehint_alias : string -> string -> string
