(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type ('prim_pos, 'pos) error_

type 'a message

val get_message_pos : 'a message -> 'a

val get_message_str : 'a message -> string

type error = (Pos.t, Pos_or_decl.t) error_ [@@deriving eq]

type finalized_error = (Pos.absolute, Pos.absolute) error_

type applied_fixme = Pos.t * int

module type Error_category = sig
  type t

  val min : int

  val max : int

  val of_enum : int -> t option

  val show : t -> string

  val err_code : t -> int
end

(* The analysis phase that the error is coming from. *)
type phase =
  | Init
  | Parsing
  | Naming
  | Decl
  | Typing
[@@deriving eq]

type severity =
  | Warning
  | Error

type format =
  | Context
  | Raw
  | Highlighted

type typing_error_callback =
  ?code:int -> Pos.t * string -> (Pos_or_decl.t * string) list -> unit

type error_from_reasons_callback =
  ?code:int -> (Pos_or_decl.t * string) list -> unit

(** This will check that the first position of the given reasons is in the
    current decl and if yes use it as primary error position. If no,
    it will error at a default position in the current file and log the failed
    assertion.
    This also sets the error code to the code for unification error
    if none is provided. *)
val unify_error_assert_primary_pos_in_current_decl :
  current_decl_and_file:Pos_or_decl.ctx -> error_from_reasons_callback

(** This will check that the first position of the given reasons is in the
    current decl and if yes use it as primary error position. If no,
    it will error at a default position in the current file and log the failed
    assertion.
    This also sets the error code to the code for invalid type hint error
    if none is provided. *)
val invalid_type_hint_assert_primary_pos_in_current_decl :
  current_decl_and_file:Pos_or_decl.ctx -> error_from_reasons_callback

type name_context =
  | FunctionNamespace
  | ConstantNamespace
  | TypeNamespace
  | TraitContext
  | ClassContext
  | RecordContext

module Parsing : Error_category

module Naming : Error_category

module NastCheck : Error_category

module Typing : Error_category

val read_lines : string -> string list

val num_digits : int -> int

val add_error : error -> unit

(* Error codes that can be suppressed in strict mode with a FIXME based on configuration. *)
val allowed_fixme_codes_strict : ISet.t ref

val allowed_fixme_codes_partial : ISet.t ref

val codes_not_raised_partial : ISet.t ref

(* Error codes that should be treated strictly, regardless of their file mode. *)
val error_codes_treated_strictly : ISet.t ref

val report_pos_from_reason : bool ref

val is_strict_code : int -> bool

val set_allow_errors_in_default_path : bool -> unit

val is_hh_fixme : (Pos.t -> int -> bool) ref

val is_hh_fixme_disallowed : (Pos.t -> int -> bool) ref

val get_hh_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val to_list : ('p, 'p) error_ -> ('p * string) list

val to_list_ : error -> (Pos_or_decl.t * string) list

val get_messages : ('p, 'p) error_ -> 'p message list

val get_code : ('pp, 'p) error_ -> int

val get_pos : error -> Pos.t

val get_severity : ('pp, 'p) error_ -> severity

val make_error : int -> Pos.t * string -> (Pos_or_decl.t * string) list -> error

val make_absolute_error : int -> (Pos.absolute * string) list -> finalized_error

val error_code_to_string : int -> string

val phase_to_string : phase -> string

val phase_of_string : string -> phase option

val name_context_to_string : name_context -> string

val to_json : finalized_error -> Hh_json.json

val convert_errors_to_string :
  ?include_filename:bool -> error list -> string list

val combining_sort : 'a list -> f:('a -> string) -> 'a list

val to_string : finalized_error -> string

val format_summary :
  format -> ('pp, 'p) error_ list -> int -> int option -> string option

val try_ : (unit -> 'a) -> (error -> 'a) -> 'a

val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a

(* The type of collections of errors *)
type t [@@deriving eq]

(** Return the list of errors caused by the function passed as parameter
    along with its result. *)
val do_ : (unit -> 'a) -> t * 'a

(** Return the list of errors caused by the function passed as parameter
    along with its result.
    The phase parameter determine the phase of the returned errors. *)
val do_with_context : Relative_path.t -> phase -> (unit -> 'a) -> t * 'a

val run_in_context : Relative_path.t -> phase -> (unit -> 'a) -> 'a

(** Turn on lazy decl mode for the duration of the closure.
    This runs without returning the original state,
    since we collect it later in do_with_lazy_decls_ *)
val run_in_decl_mode : Relative_path.t -> (unit -> 'a) -> 'a

(* Run this function with span for the definition being checked.
 * This is used to check that the primary position for errors is not located
 * outside the span of the definition.
 *)
val run_with_span : Pos.t -> (unit -> 'a) -> 'a

(** ignore errors produced by function passed in argument. *)
val ignore_ : (unit -> 'a) -> 'a

val try_when : (unit -> 'a) -> when_:(unit -> bool) -> do_:(error -> unit) -> 'a

val has_no_errors : (unit -> 'a) -> bool

val currently_has_errors : unit -> bool

val try_if_no_errors : (unit -> 'a) -> ('a -> 'a) -> 'a

val to_absolute : error -> finalized_error

val to_absolute_for_test : error -> finalized_error

val merge : t -> t -> t

val merge_into_current : t -> unit

val apply_callback_to_errors : t -> error_from_reasons_callback -> unit

val incremental_update_set :
  old:t -> new_:t -> rechecked:Relative_path.Set.t -> phase -> t

val incremental_update_map :
  old:t -> new_:t -> rechecked:'a Relative_path.Map.t -> phase -> t

val empty : t

val is_empty : t -> bool

val count : t -> int

val get_error_list : t -> error list

val get_sorted_error_list : t -> error list

val from_error_list : error list -> t

val iter_error_list : (error -> unit) -> t -> unit

val fold_errors :
  ?phase:phase -> t -> init:'a -> f:(Relative_path.t -> error -> 'a -> 'a) -> 'a

val fold_errors_in :
  ?phase:phase ->
  t ->
  source:Relative_path.t ->
  init:'a ->
  f:(error -> 'a -> 'a) ->
  'a

val get_failed_files : t -> phase -> Relative_path.Set.t

val sort : error list -> error list

val get_applied_fixmes : t -> applied_fixme list

(***************************************
 *                                     *
 *       Specific errors               *
 *                                     *
 ***************************************)

val internal_error : Pos.t -> string -> unit

val unimplemented_feature : Pos.t -> string -> unit

val experimental_feature : Pos.t -> string -> unit

val fixme_format : Pos.t -> unit

val missing_field :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val violated_constraint :
  (Pos_or_decl.t * (Pos_or_decl.t * string)) list ->
  (Pos_or_decl.t * string) list ->
  error_from_reasons_callback ->
  unit

val method_variance : Pos.t -> unit

val explain_constraint : use_pos:Pos.t -> error_from_reasons_callback

val explain_where_constraint :
  in_class:bool ->
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  (Pos_or_decl.t * string) list ->
  unit

val explain_tconst_where_constraint :
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  (Pos_or_decl.t * string) list ->
  unit

val abstract_tconst_not_allowed :
  Pos_or_decl.t -> Pos_or_decl.t * string -> error_from_reasons_callback -> unit

val unexpected_arrow : Pos.t -> string -> unit

val missing_arrow : Pos.t -> string -> unit

val disallowed_xhp_type : Pos.t -> string -> unit

val name_is_reserved : string -> Pos.t -> unit

val dollardollar_unused : Pos.t -> unit

val mutating_const_property : Pos.t -> unit

val self_const_parent_not : Pos.t -> unit

val overriding_prop_const_mismatch :
  Pos_or_decl.t ->
  bool ->
  Pos_or_decl.t ->
  bool ->
  error_from_reasons_callback ->
  unit

val method_name_already_bound : Pos.t -> string -> unit

val error_name_already_bound : string -> string -> Pos.t -> Pos.t -> unit

val unbound_name : Pos.t -> string -> name_context -> unit

val invalid_fun_pointer : Pos.t -> string -> unit

val undefined : Pos.t -> string -> (string * Pos.t) option -> unit

val this_reserved : Pos.t -> unit

val start_with_T : Pos.t -> unit

val already_bound : Pos.t -> string -> unit

val unexpected_typedef : Pos.t -> Pos.t -> name_context -> unit

val fd_name_already_bound : Pos.t -> unit

val mk_fd_name_already_bound : Pos.t -> error

val repeated_record_field : string -> Pos.t -> Pos_or_decl.t -> unit

val unexpected_record_field_name :
  field_name:string ->
  field_pos:Pos.t ->
  record_name:string ->
  decl_pos:Pos_or_decl.t ->
  unit

val missing_record_field_name :
  field_name:string ->
  new_pos:Pos.t ->
  record_name:string ->
  field_decl_pos:Pos_or_decl.t ->
  unit

val type_not_record : string -> Pos.t -> unit

val primitive_toplevel : Pos.t -> unit

val primitive_invalid_alias : Pos.t -> string -> string -> unit

val dynamic_new_in_strict_mode : Pos.t -> unit

val xhp_optional_required_attr : Pos.t -> string -> unit

val xhp_required_with_default : Pos.t -> string -> unit

val array_typehints_disallowed : Pos.t -> unit

val wildcard_hint_disallowed : Pos.t -> unit

val wildcard_param_disallowed : Pos.t -> unit

val object_cast : Pos.t -> unit

val this_no_argument : Pos.t -> unit

val this_hint_outside_class : Pos.t -> unit

val this_type_forbidden : Pos.t -> unit

val nonstatic_property_with_lsb : Pos.t -> unit

val lowercase_this : Pos.t -> string -> unit

val classname_param : Pos.t -> unit

val typaram_applied_to_type : Pos.t -> string -> unit

val tparam_with_tparam : Pos.t -> string -> unit

val shadowed_type_param : Pos.t -> Pos.t -> string -> unit

val missing_typehint : Pos.t -> unit

val expected_variable : Pos.t -> unit

val clone_too_many_arguments : Pos.t -> unit

val naming_too_few_arguments : Pos.t -> unit

val naming_too_many_arguments : Pos.t -> unit

val expected_collection : Pos.t -> string -> unit

val illegal_CLASS : Pos.t -> unit

val illegal_TRAIT : Pos.t -> unit

val nullsafe_property_write_context : Pos.t -> unit

val illegal_fun : Pos.t -> unit

val illegal_member_variable_class : Pos.t -> unit

val illegal_meth_fun : Pos.t -> unit

val illegal_inst_meth : Pos.t -> unit

val illegal_meth_caller : Pos.t -> unit

val illegal_class_meth : Pos.t -> unit

val class_meth_non_final_self : Pos.t -> string -> unit

val class_meth_non_final_CLASS : Pos.t -> bool -> string -> unit

val unexpected_ty_in_tast :
  Pos.t -> actual_ty:string -> expected_ty:string -> unit

val uninstantiable_class :
  Pos.t -> Pos_or_decl.t -> string -> (Pos.t * string) option -> unit

val new_abstract_record : Pos.t * string -> unit

val abstract_const_usage : Pos.t -> Pos_or_decl.t -> string -> unit

val concrete_const_interface_override :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  error_from_reasons_callback ->
  unit

val interface_const_multiple_defs :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  string ->
  error_from_reasons_callback ->
  unit

val typeconst_concrete_concrete_override :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  current_decl_and_file:Pos_or_decl.ctx ->
  unit

val interface_typeconst_multiple_defs :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  string ->
  bool ->
  error_from_reasons_callback ->
  unit

val const_without_typehint : Pos.t * string -> unit

val prop_without_typehint : string -> Pos.t * string -> unit

val illegal_constant : Pos.t -> unit

val parsing_error : Pos.t * string -> unit

val xhp_parsing_error : Pos.t * string -> unit

val format_string :
  Pos.t -> string -> string -> Pos_or_decl.t -> string -> string -> unit

val expected_literal_format_string : Pos.t -> unit

val re_prefixed_non_string : Pos.t -> string -> unit

val bad_regex_pattern : Pos.t -> string -> unit

val generic_array_strict : Pos.t -> unit

val option_return_only_typehint : Pos.t -> [< `void | `noreturn ] -> unit

val tuple_syntax : Pos.t -> unit

val redeclaring_missing_method : Pos.t -> string -> unit

val expecting_type_hint : Pos.t -> unit

val expecting_type_hint_variadic : Pos.t -> unit

val expecting_return_type_hint : Pos.t -> unit

val field_kinds : Pos.t -> Pos_or_decl.t -> unit

val unbound_name_typing : Pos.t -> string -> unit

val did_you_mean_naming : Pos.t -> string -> Pos.t -> string -> unit

val previous_default : Pos.t -> unit

val return_only_typehint : Pos.t -> [< `void | `noreturn ] -> unit

val unexpected_type_arguments : Pos.t -> unit

val too_many_type_arguments : Pos.t -> unit

val return_in_void : Pos.t -> Pos.t -> unit

val this_var_outside_class : Pos.t -> unit

val unbound_global : Pos.t -> unit

val private_inst_meth : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val protected_inst_meth : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val private_meth_caller : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val protected_meth_caller : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val private_class_meth : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val protected_class_meth : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val array_cast : Pos.t -> unit

val string_cast : Pos.t -> string -> unit

val static_outside_class : Pos.t -> unit

val self_outside_class : Pos.t -> unit

val new_inconsistent_construct :
  Pos.t -> Pos_or_decl.t * string -> [< `static | `classname ] -> unit

val undefined_parent : Pos.t -> unit

val parent_outside_class : Pos.t -> unit

val parent_abstract_call : string -> Pos.t -> Pos_or_decl.t -> unit

val self_abstract_call : string -> Pos.t -> Pos_or_decl.t -> unit

val classname_abstract_call : string -> string -> Pos.t -> Pos_or_decl.t -> unit

val static_synthetic_method : string -> string -> Pos.t -> Pos_or_decl.t -> unit

val isset_in_strict : Pos.t -> unit

val unset_nonidx_in_strict : Pos.t -> (Pos_or_decl.t * string) list -> unit

val unpacking_disallowed_builtin_function : Pos.t -> string -> unit

val invalid_destructure :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val unpack_array_required_argument :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val unpack_array_variadic_argument :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val array_get_arity : Pos.t -> string -> Pos_or_decl.t -> unit

val typing_error : Pos.t -> string -> unit

val undefined_field :
  use_pos:Pos.t -> name:string -> shape_type_pos:Pos_or_decl.t -> unit

val array_access_read : Pos.t -> Pos_or_decl.t -> string -> unit

val array_access_write : Pos.t -> Pos_or_decl.t -> string -> unit

val keyset_set : Pos.t -> Pos_or_decl.t -> unit

val array_append : Pos.t -> Pos_or_decl.t -> string -> unit

val const_mutation : Pos.t -> Pos_or_decl.t -> string -> unit

val expected_class : ?suffix:string -> Pos.t -> unit

val unknown_type : string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val smember_not_found :
  [< `class_constant | `class_variable | `static_method | `class_typeconst ] ->
  Pos.t ->
  Pos_or_decl.t * string ->
  string ->
  [< `closest of Pos_or_decl.t * string
  | `did_you_mean of Pos_or_decl.t * string
  | `no_hint
  ] ->
  typing_error_callback ->
  unit

val smember_not_found_ :
  [< `class_constant | `class_variable | `static_method | `class_typeconst ] ->
  Pos_or_decl.t ->
  Pos_or_decl.t * string ->
  string ->
  [< `closest of Pos_or_decl.t * string
  | `did_you_mean of Pos_or_decl.t * string
  | `no_hint
  ] ->
  error_from_reasons_callback ->
  unit

val member_not_found :
  [< `property | `method_ ] ->
  Pos.t ->
  Pos_or_decl.t * string ->
  string ->
  [< `closest of Pos_or_decl.t * string
  | `did_you_mean of Pos_or_decl.t * string
  | `no_hint
  ] ->
  (Pos_or_decl.t * string) list ->
  typing_error_callback ->
  unit

val expr_tree_unsupported_operator : string -> string -> Pos.t -> unit

val parent_in_trait : Pos.t -> unit

val parent_undefined : Pos.t -> unit

val constructor_no_args : Pos.t -> unit

val visibility : Pos.t -> string -> Pos_or_decl.t -> string -> unit

val typing_too_many_args : int -> int -> Pos.t -> Pos_or_decl.t -> unit

val typing_too_many_args_w_callback :
  int ->
  int ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val typing_too_few_args : int -> int -> Pos.t -> Pos_or_decl.t -> unit

val typing_too_few_args_w_callback :
  int ->
  int ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val bad_call : Pos.t -> string -> unit

val extend_final : Pos.t -> Pos_or_decl.t -> string -> unit

val extend_non_abstract_record : string -> Pos.t -> Pos_or_decl.t -> unit

val extend_sealed : Pos.t -> Pos_or_decl.t -> string -> string -> string -> unit

val trait_prop_const_class : Pos.t -> string -> unit

val read_before_write : Pos.t * string -> unit

val implement_abstract :
  is_final:bool -> Pos.t -> Pos_or_decl.t -> string -> string -> unit

val generic_static : Pos.t -> string -> unit

val fun_too_many_args :
  int ->
  int ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val fun_too_few_args :
  int ->
  int ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val fun_unexpected_nonvariadic :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val fun_variadicity_hh_vs_php56 :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val expected_tparam :
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  int ->
  error_from_reasons_callback option ->
  unit

val object_string : Pos.t -> Pos_or_decl.t -> unit

val object_string_deprecated : Pos.t -> unit

val cyclic_typedef : Pos.t -> Pos_or_decl.t -> unit

val type_arity_mismatch :
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val this_final :
  Pos_or_decl.t * string -> Pos_or_decl.t -> (Pos_or_decl.t * string) list

val exact_class_final :
  Pos_or_decl.t * string -> Pos_or_decl.t -> (Pos_or_decl.t * string) list

val discarded_awaitable : Pos.t -> Pos_or_decl.t -> unit

(** Ignore and drops the error. *)
val ignore_error : error_from_reasons_callback

val unify_error : typing_error_callback

val unify_error_at : Pos.t -> error_from_reasons_callback

val rigid_tvar_escape_at : Pos.t -> string -> error_from_reasons_callback

val invalid_type_hint : Pos.t -> error_from_reasons_callback

val index_type_mismatch : typing_error_callback

val index_type_mismatch_at : Pos.t -> error_from_reasons_callback

val expected_stringlike : typing_error_callback

val type_constant_mismatch :
  error_from_reasons_callback -> error_from_reasons_callback

val class_constant_type_mismatch :
  error_from_reasons_callback -> error_from_reasons_callback

val constant_does_not_match_enum_type : typing_error_callback

val enum_underlying_type_must_be_arraykey : typing_error_callback

val enum_constraint_must_be_arraykey : typing_error_callback

val enum_subtype_must_have_compatible_constraint : typing_error_callback

val parameter_default_value_wrong_type : typing_error_callback

val newtype_alias_must_satisfy_constraint : typing_error_callback

val missing_return : typing_error_callback

val inout_return_type_mismatch : typing_error_callback

val class_constant_value_does_not_match_hint : typing_error_callback

val class_property_initializer_type_does_not_match_hint : typing_error_callback

val xhp_attribute_does_not_match_hint : typing_error_callback

val record_init_value_does_not_match_hint : typing_error_callback

val strict_str_concat_type_mismatch : typing_error_callback

val strict_str_interp_type_mismatch : typing_error_callback

val bitwise_math_invalid_argument : typing_error_callback

val inc_dec_invalid_argument : typing_error_callback

val math_invalid_argument : typing_error_callback

val using_error : Pos.t -> bool -> typing_error_callback

val static_redeclared_as_dynamic :
  Pos.t -> Pos_or_decl.t -> string -> elt_type:[ `Method | `Property ] -> unit

val dynamic_redeclared_as_static :
  Pos.t -> Pos_or_decl.t -> string -> elt_type:[ `Method | `Property ] -> unit

val null_member_read :
  is_method:bool -> string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val null_member_write :
  is_method:bool -> string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val top_member_read :
  is_method:bool ->
  is_nullable:bool ->
  string ->
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  unit

val top_member_write :
  is_method:bool ->
  is_nullable:bool ->
  string ->
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  unit

val non_object_member_read :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  typing_error_callback ->
  unit

val non_object_member_read_ :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val non_object_member_write :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  typing_error_callback ->
  unit

val non_object_member_write_ :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val unknown_object_member :
  is_method:bool -> string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val non_class_member :
  is_method:bool -> string -> Pos.t -> string -> Pos_or_decl.t -> unit

val null_container : Pos.t -> (Pos_or_decl.t * string) list -> unit

val option_mixed : Pos.t -> unit

val option_null : Pos.t -> unit

val declared_covariant : Pos.t -> Pos.t -> (Pos.t * string) list -> unit

val declared_contravariant : Pos.t -> Pos.t -> (Pos.t * string) list -> unit

val static_property_type_generic_param :
  class_pos:Pos_or_decl.t ->
  var_type_pos:Pos_or_decl.t ->
  generic_pos:Pos.t ->
  unit

val contravariant_this : Pos.t -> string -> string -> unit

val wrong_extend_kind :
  parent_pos:Pos_or_decl.t ->
  parent_kind:Ast_defs.class_kind ->
  parent_name:string ->
  parent_is_enum_class:bool ->
  child_pos:Pos.t ->
  child_kind:Ast_defs.class_kind ->
  child_name:string ->
  child_is_enum_class:bool ->
  unit

val unsatisfied_req :
  class_pos:Pos.t ->
  trait_pos:Pos_or_decl.t ->
  req_pos:Pos_or_decl.t ->
  string ->
  unit

val unsatisfied_req_callback :
  class_pos:Pos.t ->
  trait_pos:Pos_or_decl.t ->
  req_pos:Pos_or_decl.t ->
  string ->
  error_from_reasons_callback

val cyclic_class_def : SSet.t -> Pos.t -> unit

val cyclic_record_def : string list -> Pos.t -> unit

val trait_reuse_with_final_method :
  Pos.t -> string -> string -> (Pos_or_decl.t * string) list -> unit

val trait_reuse : Pos_or_decl.t -> string -> Pos.t * string -> string -> unit

val trait_reuse_inside_class :
  Pos.t * string -> string -> Pos_or_decl.t list -> unit

val invalid_is_as_expression_hint :
  string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val invalid_enforceable_type :
  string ->
  Pos_or_decl.t * string ->
  Pos.t ->
  (Pos_or_decl.t * string) list ->
  unit

val reifiable_attr :
  Pos_or_decl.t -> string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val invalid_newable_type_argument : Pos_or_decl.t * string -> Pos.t -> unit

val invalid_newable_type_param_constraints :
  Pos.t * string -> string list -> unit

val override_final :
  parent:Pos_or_decl.t ->
  child:Pos_or_decl.t ->
  on_error:error_from_reasons_callback ->
  unit

val override_lsb :
  member_name:string ->
  parent:Pos_or_decl.t ->
  child:Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val should_be_override :
  Pos_or_decl.t ->
  string ->
  string ->
  current_decl_and_file:Pos_or_decl.ctx ->
  unit

val override_per_trait :
  Pos.t * string -> string -> string -> Pos_or_decl.t -> unit

val missing_assign : Pos.t -> unit

val invalid_memoized_param : Pos.t -> (Pos_or_decl.t * string) list -> unit

val no_construct_parent : Pos.t -> unit

val constructor_required : Pos.t * string -> string list -> unit

val not_initialized : Pos.t * string -> (Pos_or_decl.t * string) list -> unit

val call_before_init : Pos.t -> string -> unit

val type_arity : Pos.t -> Pos_or_decl.t -> expected:int -> actual:int -> unit

val invalid_req_implements : Pos.t -> unit

val invalid_req_extends : Pos.t -> unit

val abstract_with_body : Pos.t * 'a -> unit

val return_in_gen : Pos.t -> unit

val return_in_finally : Pos.t -> unit

val toplevel_break : Pos.t -> unit

val toplevel_continue : Pos.t -> unit

val continue_in_switch : Pos.t -> unit

val await_in_sync_function : Pos.t -> unit

val static_memoized_function : Pos.t -> unit

val magic : Pos.t * string -> unit

val non_interface : Pos.t -> string -> string -> unit

val toString_returns_string : Pos.t -> unit

val toString_visibility : Pos.t -> unit

val uses_non_trait : Pos.t -> string -> string -> unit

val requires_non_class : Pos.t -> string -> string -> unit

val requires_final_class : Pos.t -> string -> unit

val abstract_body : Pos.t -> unit

val interface_with_member_variable : Pos.t -> unit

val interface_with_static_member_variable : Pos.t -> unit

val illegal_function_name : Pos.t -> string -> unit

val case_fallthrough : Pos.t -> Pos.t -> unit

val default_fallthrough : Pos.t -> unit

val visibility_extends :
  string ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val member_not_implemented :
  string -> Pos_or_decl.t -> Pos.t -> Pos_or_decl.t -> unit

val bad_decl_override :
  Pos_or_decl.t ->
  string ->
  Pos.t ->
  string ->
  (Pos_or_decl.t * string) list ->
  unit

val method_import_via_diamond :
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  (Pos_or_decl.t * string) list ->
  (Pos_or_decl.t * string) list ->
  unit

val bad_method_override :
  Pos_or_decl.t ->
  string ->
  (Pos_or_decl.t * string) list ->
  error_from_reasons_callback ->
  unit

val bad_prop_override :
  Pos_or_decl.t ->
  string ->
  (Pos_or_decl.t * string) list ->
  error_from_reasons_callback ->
  unit

val bad_enum_decl : Pos.t -> (Pos_or_decl.t * string) list -> unit

val missing_constructor : Pos_or_decl.t -> error_from_reasons_callback -> unit

val enum_constant_type_bad :
  Pos.t -> Pos_or_decl.t -> string -> Pos_or_decl.t list -> unit

val enum_type_bad : Pos.t -> bool -> string -> Pos_or_decl.t list -> unit

val enum_type_typedef_nonnull : Pos.t -> unit

val enum_switch_redundant : string -> Pos.t -> Pos.t -> unit

val enum_switch_nonexhaustive : Pos.t -> string list -> Pos_or_decl.t -> unit

val enum_switch_redundant_default : Pos.t -> Pos_or_decl.t -> unit

val enum_switch_not_const : Pos.t -> unit

val enum_switch_wrong_class : Pos.t -> string -> string -> unit

val invalid_shape_field_name : Pos.t -> unit

val invalid_shape_field_name_empty : Pos.t -> unit

val invalid_shape_field_type :
  Pos.t -> Pos_or_decl.t -> string -> Pos_or_decl.t list -> unit

val invalid_shape_field_literal : Pos.t -> Pos.t -> unit

val invalid_shape_field_const : Pos.t -> Pos.t -> unit

val shape_field_class_mismatch : Pos.t -> Pos.t -> string -> string -> unit

val shape_field_type_mismatch : Pos.t -> Pos.t -> string -> string -> unit

val shape_fields_unknown :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val invalid_shape_remove_key : Pos.t -> unit

val using_internal_class : Pos.t -> string -> unit

val trivial_strict_eq :
  Pos.t ->
  string ->
  (Pos_or_decl.t * string) list ->
  (Pos_or_decl.t * string) list ->
  Pos_or_decl.t list ->
  Pos_or_decl.t list ->
  unit

val trivial_strict_not_nullable_compare_null :
  Pos.t -> string -> (Pos_or_decl.t * string) list -> unit

val void_usage : Pos.t -> (Pos_or_decl.t * string) list -> unit

val noreturn_usage : Pos.t -> (Pos_or_decl.t * string) list -> unit

val generic_at_runtime : Pos.t -> string -> unit

val generics_not_allowed : Pos.t -> unit

val interface_with_partial_typeconst : Pos.t -> unit

val partially_abstract_typeconst_definition : Pos.t -> unit

val multiple_xhp_category : Pos.t -> unit

val mk_multiple_xhp_category : Pos.t -> error

val not_abstract_without_typeconst : Pos.t * string -> unit

val mk_not_abstract_without_typeconst : Pos.t * string -> error

val typeconst_depends_on_external_tparam : Pos.t -> Pos.t -> string -> unit

val invalid_type_access_root : Pos.t * string -> unit

val duplicate_user_attribute : Pos.t * string -> Pos.t -> unit

val unbound_attribute_name : Pos.t -> string -> unit

val attribute_too_many_arguments : Pos.t -> string -> int -> unit

val attribute_too_few_arguments : Pos.t -> string -> int -> unit

val attribute_param_type : Pos.t -> string -> unit

val deprecated_use : Pos.t -> ?pos_def:Pos_or_decl.t option -> string -> unit

val cannot_declare_constant :
  [< `enum | `record ] -> Pos.t -> Pos.t * string -> unit

val ambiguous_inheritance :
  Pos_or_decl.t ->
  string ->
  string ->
  error ->
  error_from_reasons_callback ->
  unit

val duplicate_interface : Pos.t -> string -> Pos_or_decl.t list -> unit

val cyclic_typeconst : Pos.t -> string list -> unit

val abstract_concrete_override :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  [< `method_ | `typeconst | `constant | `property ] ->
  current_decl_and_file:Pos_or_decl.ctx ->
  unit

val local_variable_modified_and_used : Pos.t -> Pos.t list -> unit

val local_variable_modified_twice : Pos.t -> Pos.t list -> unit

val assign_during_case : Pos.t -> unit

val cyclic_enum_constraint :
  Pos_or_decl.t -> error_from_reasons_callback -> unit

val invalid_classname : Pos.t -> unit

val illegal_type_structure : Pos.t -> string -> unit

val illegal_typeconst_direct_access : Pos.t -> unit

val override_no_default_typeconst :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  current_decl_and_file:Pos_or_decl.ctx ->
  unit

val unification_cycle : Pos.t -> string -> unit

val eq_incompatible_types :
  Pos.t ->
  (Pos_or_decl.t * string) list ->
  (Pos_or_decl.t * string) list ->
  unit

val strict_eq_value_incompatible_types :
  Pos.t ->
  (Pos_or_decl.t * string) list ->
  (Pos_or_decl.t * string) list ->
  unit

val comparison_invalid_types :
  Pos.t ->
  (Pos_or_decl.t * string) list ->
  (Pos_or_decl.t * string) list ->
  unit

val invalid_new_disposable : Pos.t -> unit

val invalid_disposable_hint : Pos.t -> string -> unit

val invalid_disposable_return_hint : Pos.t -> string -> unit

val invalid_return_disposable : Pos.t -> unit

val invalid_switch_case_value_type : Pos.t -> string -> string -> unit

val too_few_type_arguments : Pos.t -> unit

val required_field_is_optional :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val array_get_with_optional_field : Pos.t -> Pos_or_decl.t -> string -> unit

val method_needs_visibility : Pos.t -> unit

val mk_method_needs_visibility : Pos.t -> error

val dynamic_class_name_in_strict_mode : Pos.t -> unit

val reading_from_append : Pos.t -> unit

val list_rvalue : Pos.t -> unit

val nullable_cast : Pos.t -> string -> Pos_or_decl.t -> unit

val return_disposable_mismatch :
  bool -> Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val lvar_in_obj_get : Pos.t -> unit

val this_as_lexical_variable : Pos.t -> unit

val dollardollar_lvalue : Pos.t -> unit

val duplicate_using_var : Pos.t -> unit

val illegal_disposable : Pos.t -> string -> unit

val escaping_disposable : Pos.t -> unit

val escaping_disposable_parameter : Pos.t -> unit

val escaping_this : Pos.t -> unit

val must_extend_disposable : Pos.t -> unit

val accept_disposable_invariant :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val ifc_external_contravariant :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val inout_params_special : Pos.t -> unit

val inout_params_memoize : Pos.t -> Pos.t -> unit

val inout_annotation_missing : Pos.t -> Pos_or_decl.t -> unit

val inout_annotation_unexpected : Pos.t -> Pos_or_decl.t -> bool -> unit

val inoutness_mismatch :
  Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val xhp_required : Pos.t -> string -> (Pos_or_decl.t * string) list -> unit

val illegal_xhp_child : Pos.t -> (Pos_or_decl.t * string) list -> unit

val missing_xhp_required_attr :
  Pos.t -> string -> (Pos_or_decl.t * string) list -> unit

val inout_argument_bad_expr : Pos.t -> unit

val inout_argument_bad_type : Pos.t -> (Pos_or_decl.t * string) list -> unit

val illegal_destructor : Pos.t -> unit

val ambiguous_lambda : Pos.t -> (Pos_or_decl.t * string) list -> unit

val ellipsis_strict_mode :
  require:[< `Param_name | `Type_and_param_name ] -> Pos.t -> unit

val untyped_lambda_strict_mode : Pos.t -> unit

val wrong_expression_kind_attribute :
  string -> Pos.t -> string -> Pos_or_decl.t -> string -> string -> unit

val wrong_expression_kind_builtin_attribute : string -> Pos.t -> string -> unit

val decl_override_missing_hint :
  Pos_or_decl.t -> error_from_reasons_callback -> unit

val illegal_use_of_dynamically_callable : Pos.t -> Pos.t -> string -> unit

val dynamically_callable_reified : Pos.t -> unit

val parent_in_function_pointer : Pos.t -> string option -> string -> unit

val self_in_non_final_function_pointer :
  Pos.t -> string option -> string -> unit

val invalid_wildcard_context : Pos.t -> unit

val shapes_key_exists_always_true : Pos.t -> string -> Pos_or_decl.t -> unit

val shapes_key_exists_always_false :
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  [< `Undefined | `Nothing of (Pos_or_decl.t * string) list ] ->
  unit

val shapes_method_access_with_non_existent_field :
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  [< `Undefined | `Nothing of (Pos_or_decl.t * string) list ] ->
  unit

val shape_access_with_non_existent_field :
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  [< `Undefined | `Nothing of (Pos_or_decl.t * string) list ] ->
  unit

val ambiguous_object_access :
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  string ->
  unit

val unserializable_type : Pos.t -> string -> unit

val invalid_arraykey_read :
  Pos.t -> Pos_or_decl.t * string -> Pos_or_decl.t * string -> unit

val invalid_arraykey_write :
  Pos.t -> Pos_or_decl.t * string -> Pos_or_decl.t * string -> unit

val invalid_keyset_value :
  Pos.t -> Pos_or_decl.t * string -> Pos_or_decl.t * string -> unit

val invalid_arraykey_constraint : Pos.t -> string -> unit

val entrypoint_arguments : Pos.t -> unit

val entrypoint_generics : Pos.t -> unit

val variadic_memoize : Pos.t -> unit

val abstract_method_memoize : Pos.t -> unit

val instance_property_in_abstract_final_class : Pos.t -> unit

val lateinit_with_default : Pos.t -> unit

val bad_lateinit_override :
  bool -> Pos_or_decl.t -> Pos_or_decl.t -> error_from_reasons_callback -> unit

val bad_xhp_attr_required_override :
  string ->
  string ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  error_from_reasons_callback ->
  unit

val interface_use_trait : Pos.t -> unit

val nonstatic_method_in_abstract_final_class : Pos.t -> unit

val multiple_concrete_defs :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  string ->
  string ->
  error_from_reasons_callback ->
  unit

val require_args_reify : Pos_or_decl.t -> Pos.t -> unit

val require_generic_explicit : Pos_or_decl.t * string -> Pos.t -> unit

val invalid_reified_argument :
  Pos_or_decl.t * string -> Pos.t -> (Pos_or_decl.t * string) list -> unit

val invalid_reified_argument_reifiable :
  Pos_or_decl.t * string -> Pos.t -> Pos_or_decl.t -> string -> unit

val new_class_reified : Pos.t -> string -> string option -> unit

val class_get_reified : Pos.t -> unit

val consistent_construct_reified : Pos.t -> unit

val reified_generics_not_allowed : Pos.t -> unit

val bad_function_pointer_construction : Pos.t -> unit

val new_without_newable : Pos.t -> string -> unit

val typechecker_timeout : Pos.t * string -> int -> unit

val switch_multiple_default : Pos.t -> unit

val switch_non_terminal_default : Pos.t -> unit

val unsupported_trait_use_as : Pos.t -> unit

val mk_unsupported_trait_use_as : Pos.t -> error

val unsupported_instead_of : Pos.t -> unit

val mk_unsupported_instead_of : Pos.t -> error

val invalid_trait_use_as_visibility : Pos.t -> unit

val mk_invalid_trait_use_as_visibility : Pos.t -> error

val unresolved_type_variable : Pos.t -> unit

val invalid_sub_string : Pos.t -> string -> unit

val php_lambda_disallowed : Pos.t -> unit

val static_meth_with_class_reified_generic : Pos.t -> Pos.t -> unit

val exception_occurred :
  typechecking_is_deferring:bool -> Pos.t -> Exception.t -> unit

(* The intention is to introduce invariant violations with `report_to_user`
set to `false` initially. Then we observe and confirm that the invariant is
not repeatedly violated. Only then, we set it to `true` in a subsequent
release. This should prevent us from blocking users unexpectedly while
gradually introducing signal for unexpected compiler states. *)
val invariant_violation :
  typechecking_is_deferring:bool ->
  report_to_user:bool ->
  desc:string ->
  Pos.t ->
  Telemetry.t ->
  unit

val redundant_covariant : Pos.t -> string -> string -> unit

val meth_caller_trait : Pos.t -> string -> unit

val unsupported_hk_feature :
  because_nested:bool -> Pos.t -> string -> string -> unit

val tparam_non_shadowing_reuse : Pos.t -> string -> unit

val illegal_information_flow :
  Pos.t ->
  Pos_or_decl.t list ->
  Pos_or_decl.t list * string ->
  Pos_or_decl.t list * string ->
  unit

val context_implicit_policy_leakage :
  Pos.t ->
  Pos_or_decl.t list ->
  Pos_or_decl.t list * string ->
  Pos_or_decl.t list * string ->
  unit

val unknown_information_flow : Pos.t -> string -> unit

val reified_function_reference : Pos.t -> unit

val class_meth_abstract_call :
  string -> string -> Pos.t -> Pos_or_decl.t -> unit

val higher_kinded_partial_application : Pos.t -> int -> unit

val wildcard_for_higher_kinded_type : Pos.t -> unit

val implicit_type_argument_for_higher_kinded_type :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> string -> unit

val kind_mismatch :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  tparam_name:string ->
  expected_kind_repr:string ->
  actual_kind_repr:string ->
  unit

val class_with_constraints_used_as_hk_type : Pos.t -> string -> unit

val alias_with_implicit_constraints_as_hk_type :
  use_pos:Pos.t ->
  typedef_pos:Pos_or_decl.t ->
  used_class_in_def_pos:Pos_or_decl.t ->
  typedef_name:string ->
  typedef_tparam_name:string ->
  used_class_in_def_name:string ->
  used_class_tparam_name:string ->
  unit

val reinheriting_classish_const :
  Pos.t -> string -> Pos.t -> string -> string -> string -> unit

val redeclaring_classish_const :
  Pos.t -> string -> Pos.t -> string -> string -> unit

val incompatible_enum_inclusion_base : Pos.t -> string -> string -> unit

val incompatible_enum_inclusion_constraint : Pos.t -> string -> string -> unit

val enum_inclusion_not_enum : Pos.t -> string -> string -> unit

val call_coeffect_error :
  available_incl_unsafe:string ->
  available_pos:Pos_or_decl.t ->
  required:string ->
  required_pos:Pos_or_decl.t ->
  Pos.t ->
  unit

val coeffect_subtyping_error :
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  error_from_reasons_callback ->
  unit

val op_coeffect_error :
  locally_available:string ->
  available_pos:Pos_or_decl.t ->
  err_code:int ->
  required:string ->
  ?suggestion:(Pos_or_decl.t * string) list ->
  string ->
  Pos.t ->
  unit

val illegal_context : Pos.t -> string -> unit

val abstract_function_pointer :
  string -> string -> Pos.t -> Pos_or_decl.t -> unit

val unnecessary_attribute :
  Pos.t ->
  attr:string ->
  reason:Pos.t * string ->
  suggestion:string option ->
  unit

val inherited_class_member_with_different_case :
  string ->
  string ->
  string ->
  Pos.t ->
  string ->
  string ->
  Pos_or_decl.t ->
  unit

val multiple_inherited_class_member_with_different_case :
  member_type:string ->
  name1:string ->
  name2:string ->
  class1:string ->
  class2:string ->
  child_class:string ->
  child_p:Pos.t ->
  p1:Pos_or_decl.t ->
  p2:Pos_or_decl.t ->
  unit

val atom_invalid_parameter : Pos.t -> unit

val atom_invalid_parameter_in_enum_class : Pos.t -> unit

val atom_invalid_generic : Pos.t -> string -> unit

val enum_class_label_unknown : Pos.t -> string -> string -> unit

val enum_class_label_as_expr : Pos.t -> unit

val enum_class_label_invalid_argument : Pos.t -> is_proj:bool -> unit

val ifc_internal_error : Pos.t -> string -> unit

val ifc_policy_mismatch :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  error_from_reasons_callback ->
  unit

val override_method_support_dynamic_type :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  error_from_reasons_callback ->
  unit

val parent_support_dynamic_type :
  Pos.t ->
  string * Ast_defs.class_kind ->
  string * Ast_defs.class_kind ->
  bool ->
  unit

val method_is_not_dynamically_callable :
  Pos.t ->
  string ->
  string ->
  bool ->
  (Pos_or_decl.t * string) option ->
  error option ->
  unit

val function_is_not_dynamically_callable : Pos.t -> string -> error -> unit

val property_is_not_enforceable :
  Pos.t -> string -> string -> Pos_or_decl.t * string -> unit

val property_is_not_dynamic :
  Pos.t -> string -> string -> Pos_or_decl.t * string -> unit

val private_property_is_not_enforceable :
  Pos.t -> string -> string -> Pos_or_decl.t * string -> unit

val private_property_is_not_dynamic :
  Pos.t -> string -> string -> Pos_or_decl.t * string -> unit

val immutable_local : Pos.t -> unit

val enum_classes_reserved_syntax : Pos.t -> unit

val nonsense_member_selection : Pos.t -> string -> unit

val consider_meth_caller : Pos.t -> string -> string -> unit

val enum_supertyping_reserved_syntax : Pos.t -> unit

val readonly_modified : ?reason:Pos_or_decl.t * string -> Pos.t -> unit

val readonly_mismatch :
  string ->
  Pos.t ->
  reason_sub:(Pos_or_decl.t * string) list ->
  reason_super:(Pos_or_decl.t * string) list ->
  unit

val readonly_mismatch_on_error :
  string ->
  Pos_or_decl.t ->
  reason_sub:(Pos_or_decl.t * string) list ->
  reason_super:(Pos_or_decl.t * string) list ->
  error_from_reasons_callback ->
  unit

val explicit_readonly_cast : string -> Pos.t -> Pos_or_decl.t -> unit

val readonly_method_call : Pos.t -> Pos_or_decl.t -> unit

val invalid_meth_caller_calling_convention :
  Pos.t -> Pos_or_decl.t -> string -> unit

val unsafe_cast : Pos.t -> unit

val readonly_exception : Pos.t -> unit

val experimental_expression_trees : Pos.t -> unit

val non_void_annotation_on_return_void_function : bool -> Pos.t -> unit

val returns_with_and_without_value :
  fun_pos:Pos.t ->
  with_value_pos:Pos.t ->
  without_value_pos_opt:Pos.t option ->
  unit

val cyclic_class_constant : Pos.t -> string -> string -> unit

val readonly_closure_call : Pos.t -> Pos_or_decl.t -> string -> unit

val bad_conditional_support_dynamic :
  Pos.t ->
  child:string ->
  parent:string ->
  string ->
  (Pos_or_decl.t * string) list ->
  unit

val readonly_invalid_as_mut : Pos.t -> unit

val unresolved_type_variable_projection :
  Pos.t -> string -> proj_pos:Pos_or_decl.t -> unit

val function_pointer_with_atom : Pos.t -> Pos_or_decl.t -> unit

val reified_static_method_in_expr_tree : Pos.t -> unit

val invalid_echo_argument_at : Pos.t -> error_from_reasons_callback
