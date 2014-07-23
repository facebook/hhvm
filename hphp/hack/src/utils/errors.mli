(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type error
type t = error list

val to_list : error -> (Pos.t * string) list
val get_pos : error -> Pos.t
val filename : error -> string
val make_error : (Pos.t * string) list -> error

val fixme_format : Pos.t -> unit
val alok : Pos.t * string -> unit
val unexpected_eof : Pos.t -> unit
val missing_field : Pos.t -> Pos.t -> string -> unit
val generic_class_var : Pos.t -> unit
val explain_constraint : Pos.t -> string -> error -> unit
val unexpected_arrow : Pos.t -> string -> unit
val missing_arrow : Pos.t -> string -> unit
val disallowed_xhp_type : Pos.t -> string -> unit
val overflow : Pos.t -> unit
val unterminated_comment : Pos.t -> unit
val unterminated_xhp_comment : Pos.t -> unit
val name_already_bound : string -> Pos.t -> Pos.t -> unit
val method_name_already_bound : Pos.t -> string -> unit
val error_name_already_bound : string -> string -> string -> Pos.t -> Pos.t -> unit
val unbound_name : Pos.t -> string -> unit
val different_scope : Pos.t -> string -> Pos.t -> unit
val undefined : Pos.t -> string -> unit
val this_reserved : Pos.t -> unit
val start_with_T : Pos.t -> unit
val already_bound : Pos.t -> string -> unit
val unexpected_typedef : Pos.t -> Pos.t -> unit
val fd_name_already_bound : Pos.t -> unit
val primitive_toplevel : Pos.t -> unit
val integer_instead_of_int : Pos.t -> unit
val boolean_instead_of_bool : Pos.t -> unit
val double_instead_of_float : Pos.t -> unit
val real_instead_of_float : Pos.t -> unit
val this_no_argument : Pos.t -> unit
val this_outside_of_class : Pos.t -> unit
val this_must_be_return : Pos.t -> unit
val lowercase_this : Pos.t -> string -> unit
val tparam_with_tparam : Pos.t -> string -> unit
val shadowed_type_param : Pos.t -> Pos.t -> string -> unit
val missing_typehint : Pos.t -> unit
val expected_variable : Pos.t -> unit
val naming_too_few_arguments : Pos.t -> unit
val naming_too_many_arguments : Pos.t -> unit
val expected_collection : Pos.t -> string -> unit
val illegal_CLASS : Pos.t -> unit
val dynamic_method_call : Pos.t -> unit
val illegal_fun : Pos.t -> unit
val illegal_meth_fun : Pos.t -> unit
val illegal_inst_meth : Pos.t -> unit
val illegal_meth_caller : Pos.t -> unit
val illegal_class_meth : Pos.t -> unit
val assert_arity : Pos.t -> unit
val gena_arity : Pos.t -> unit
val genva_arity : Pos.t -> unit
val gen_array_rec_arity : Pos.t -> unit
val gen_array_va_rec_arity : Pos.t -> unit
val dynamic_class : Pos.t -> unit
val typedef_constraint : Pos.t -> unit
val add_a_typehint : Pos.t -> unit
val local_const : Pos.t -> unit
val illegal_constant : Pos.t -> unit
val cyclic_constraint : Pos.t -> unit
val parsing_error : Pos.t * string -> unit
val format_string :
  Pos.t -> string -> string -> Pos.t -> string -> string -> unit
val expected_literal_string : Pos.t -> unit
val generic_array_strict : Pos.t -> unit
val nullable_void : Pos.t -> unit
val tuple_syntax : Pos.t -> unit
val class_arity : Pos.t -> string -> int -> unit
val dynamic_yield_private : Pos.t -> unit
val expecting_type_hint : Pos.t -> unit
val expecting_type_hint_suggest : Pos.t -> string -> unit
val expecting_return_type_hint : Pos.t -> unit
val expecting_return_type_hint_suggest : Pos.t -> string -> unit
val field_kinds : Pos.t -> Pos.t -> unit
val unbound_name_typing : Pos.t -> string -> unit
val did_you_mean_naming : Pos.t -> string -> Pos.t -> string -> unit
val previous_default : Pos.t -> unit
val void_parameter : Pos.t -> unit
val nullable_parameter : Pos.t -> unit
val return_in_void : Pos.t -> Pos.t -> unit
val this_in_static : Pos.t -> unit
val this_outside_class : Pos.t -> unit
val unbound_global : Pos.t -> unit
val private_inst_meth : Pos.t -> Pos.t -> unit
val protected_inst_meth : Pos.t -> Pos.t -> unit
val private_class_meth : Pos.t -> Pos.t -> unit
val protected_class_meth : Pos.t -> Pos.t -> unit
val array_cast : Pos.t -> unit
val anonymous_recursive : Pos.t -> unit
val new_static_outside_class : Pos.t -> unit
val new_self_outside_class : Pos.t -> unit
val abstract_instantiate : Pos.t -> string -> unit
val pair_arity : Pos.t -> unit
val tuple_arity : Pos.t -> int -> Pos.t -> int -> unit
val undefined_parent : Pos.t -> unit
val parent_construct_in_trait : Pos.t -> unit
val parent_outside_class : Pos.t -> unit
val dont_use_isset : Pos.t -> unit
val array_get_arity : Pos.t -> string -> Pos.t -> unit
val static_overflow : Pos.t -> unit
val typing_error : Pos.t -> string -> unit
val typing_error_l : error -> unit
val undefined_field : Pos.t -> string -> unit
val shape_access : Pos.t -> unit
val array_access : Pos.t -> Pos.t -> string -> unit
val array_append : Pos.t -> Pos.t -> string -> unit
val const_mutation : Pos.t -> Pos.t -> string -> unit
val negative_tuple_index : Pos.t -> unit
val tuple_index_too_large : Pos.t -> unit
val expected_static_int : Pos.t -> unit
val expected_class : Pos.t -> unit
val snot_found_hint :
  [< `closest of 'a * string | `did_you_mean of 'a * string | `no_hint ] ->
  ('a * string) list
val string_of_class_member_kind :
  [< `class_constant | `class_variable | `static_method ] -> string
val smember_not_found :
  [< `class_constant | `class_variable | `static_method ] ->
  Pos.t ->
  string ->
  [< `closest of Pos.t * string | `did_you_mean of Pos.t * string | `no_hint ] ->
  unit
val not_found_hint :
  [< `closest of 'a * string | `did_you_mean of 'a * string | `no_hint ] ->
  ('a * string) list
val member_not_found :
  [< `member | `method_ ] ->
  Pos.t ->
  Pos.t * string ->
  string ->
  [< `closest of Pos.t * string | `did_you_mean of Pos.t * string | `no_hint ] ->
  unit
val parent_in_trait : Pos.t -> unit
val parent_undefined : Pos.t -> unit
val constructor_no_args : Pos.t -> unit
val visibility : Pos.t -> string -> Pos.t -> string -> unit
val typing_too_many_args : Pos.t -> Pos.t -> unit
val typing_too_few_args : Pos.t -> Pos.t -> unit
val anonymous_recursive_call : Pos.t -> unit
val bad_call : Pos.t -> string -> unit
val sketchy_null_check : Pos.t -> unit
val sketchy_null_check_primitive : Pos.t -> unit
val extend_final : Pos.t -> unit
val read_before_write : Pos.t * string -> unit
val interface_final : Pos.t -> unit
val abstract_class_final : Pos.t -> unit
val trait_final : Pos.t -> unit
val implement_abstract : Pos.t -> Pos.t -> string -> unit
val generic_static : Pos.t -> string -> unit
val fun_too_many_args : Pos.t -> Pos.t -> unit
val fun_too_few_args : Pos.t -> Pos.t -> unit
val fun_unexpected_nonvariadic : Pos.t -> Pos.t -> unit
val fun_variadicity_hh_vs_php56 : Pos.t -> Pos.t -> unit
val expected_tparam : Pos.t -> int -> unit
val field_missing : string -> Pos.t -> Pos.t -> unit
val object_string : Pos.t -> Pos.t -> unit
val untyped_string : Pos.t -> unit
val type_param_arity : Pos.t -> string -> string -> unit
val cyclic_typedef : Pos.t -> unit
val type_arity_mismatch : Pos.t -> string -> Pos.t -> string -> unit
val this_final : Pos.t * string -> Pos.t -> error -> unit
val tuple_arity_mismatch : Pos.t -> string -> Pos.t -> string -> unit
val fun_arity_mismatch : Pos.t -> Pos.t -> unit
val discarded_awaitable : Pos.t -> Pos.t -> unit
val gena_expects_array : Pos.t -> Pos.t -> string -> unit
val unify_error : (Pos.t * string) list -> (Pos.t * string) list -> unit
val static_dynamic : Pos.t -> Pos.t -> string -> unit
val null_member : string -> Pos.t -> (Pos.t * string) list -> unit
val non_object_member : string -> Pos.t -> string -> Pos.t -> unit
val null_container : Pos.t -> (Pos.t * string) list -> unit
val option_mixed : Pos.t -> unit
val wrong_extend_kind : Pos.t -> string -> Pos.t -> string -> unit
val unsatisfied_req : Pos.t -> string -> Pos.t -> unit
val cyclic_class_def : Utils.SSet.t -> Pos.t -> unit
val override_final : parent:Pos.t -> child:Pos.t -> unit
val should_be_override : Pos.t -> string -> string -> unit
val override_per_trait : Pos.t * string -> string -> Pos.t -> unit
val missing_assign : Pos.t -> unit
val private_override : Pos.t -> string -> string -> unit
val no_construct_parent : Pos.t -> unit
val not_initialized : Pos.t * string -> unit
val call_before_init : Pos.t -> string -> unit
val type_arity : Pos.t -> string -> string -> unit
val abstract_outside : Pos.t * 'a -> unit
val invalid_req_implements : Pos.t -> unit
val invalid_req_extends : Pos.t -> unit
val interface_with_body : Pos.t * 'a -> unit
val abstract_with_body : Pos.t * 'a -> unit
val not_abstract_without_body : Pos.t * 'a -> unit
val return_in_gen : Pos.t -> unit
val return_in_finally : Pos.t -> unit
val yield_in_async_function : Pos.t -> unit
val await_in_sync_function : Pos.t -> unit
val magic : Pos.t * string -> unit
val non_interface : Pos.t -> string -> string -> unit
val toString_returns_string : Pos.t -> unit
val toString_visibility : Pos.t -> unit
val uses_non_trait : Pos.t -> string -> string -> unit
val requires_non_class : Pos.t -> string -> string -> unit
val abstract_body : Pos.t -> unit
val not_public_interface : Pos.t -> unit
val interface_with_member_variable : Pos.t -> unit
val interface_with_static_member_variable : Pos.t -> unit
val dangerous_method_name : Pos.t -> unit
val case_fallthrough : Pos.t -> Pos.t -> unit
val default_fallthrough : Pos.t -> unit
val visibility_extends : string -> Pos.t -> Pos.t -> string -> unit
val member_not_implemented : string -> Pos.t -> Pos.t -> Pos.t -> unit
val override : Pos.t -> string -> Pos.t -> string -> error -> unit
val missing_constructor : Pos.t -> unit
val enum_constant_type_bad : Pos.t -> string -> Pos.t list -> unit
val enum_type_bad : Pos.t -> string -> Pos.t list -> unit
val enum_type_typedef_mixed : Pos.t -> unit
val invalid_shape_field_name : Pos.t -> unit
val invalid_shape_field_type : Pos.t -> string -> unit
val invalid_shape_field_literal : Pos.t -> Pos.t -> unit
val invalid_shape_field_const : Pos.t -> Pos.t -> unit
val shape_field_class_mismatch : Pos.t -> Pos.t -> string -> string -> unit
val shape_field_type_mismatch : Pos.t -> Pos.t -> string -> string -> unit

val to_json : error -> Hh_json.json
val to_string : error -> string
val try_ : (unit -> 'a) -> (error -> 'a) -> 'a
val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a
val try_add_err : Pos.t -> string -> (unit -> 'a) -> (unit -> 'a) -> 'a
val do_ : (unit -> 'a) -> error list * 'a
val try_when :
  (unit -> unit) -> when_:(unit -> bool) -> do_:(error -> unit) -> unit
