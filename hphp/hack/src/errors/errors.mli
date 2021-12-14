(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error = (Pos.t, Pos_or_decl.t) User_error.t [@@deriving eq, show]

type finalized_error = (Pos.absolute, Pos.absolute) User_error.t
[@@deriving eq, show]

type applied_fixme = Pos.t * int

(* The analysis phase that the error is coming from. *)
type phase =
  | Init
  | Parsing
  | Naming
  | Decl
  | Typing
[@@deriving eq]

module PhaseMap : sig
  include
    Reordered_argument_collections.Reordered_argument_map_S
      with type key = phase

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string
end

type format =
  | Context
  | Raw
  | Highlighted

(** Type representing the errors for a single file. *)
type per_file_errors

(** The type of collections of errors *)
type t [@@deriving eq]

module ErrorSet : Caml.Set.S with type elt := error

module FinalizedErrorSet : Caml.Set.S with type elt := finalized_error

val phases_up_to_excl : phase -> phase list

module Parsing : Error_category.S

module Naming : Error_category.S

module NastCheck : Error_category.S

module Typing : Error_category.S

val read_lines : string -> string list

val num_digits : int -> int

val add_error : error -> unit

val add_parsing_error : Parsing_error.t -> unit

val add_naming_error : Naming_error.t -> unit

val add_nast_check_error : Nast_check_error.t -> unit

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

val phase_to_string : phase -> string

val phase_of_string : string -> phase option

val convert_errors_to_string :
  ?include_filename:bool -> error list -> string list

val combining_sort : 'a list -> f:('a -> string) -> 'a list

val to_string : finalized_error -> string

val format_summary :
  format -> ('pp, 'p) User_error.t list -> int -> int option -> string option

val try_ : (unit -> 'a) -> (error -> 'a) -> 'a

val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a

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

val merge : t -> t -> t

val merge_into_current : t -> unit

val incremental_update :
  old:t -> new_:t -> rechecked:Relative_path.Set.t -> phase -> t

val empty : t

val is_empty : t -> bool

val count : t -> int

val get_error_list : t -> error list

val get_sorted_error_list : t -> error list

val as_map : t -> error list Relative_path.Map.t

val from_error_list : error list -> t

(** Default applied phase is Typing. *)
val from_file_error_list : ?phase:phase -> (Relative_path.t * error) list -> t

val per_file_error_count : per_file_errors -> int

val errors_in_file : t -> Relative_path.t -> error list

val get_file_errors : t -> Relative_path.t -> per_file_errors

val iter_error_list : (error -> unit) -> t -> unit

val fold_per_file :
  t ->
  init:'acc ->
  f:(Relative_path.t -> per_file_errors -> 'acc -> 'acc) ->
  'acc

val fold_errors :
  ?phase:phase -> t -> init:'a -> f:(Relative_path.t -> error -> 'a -> 'a) -> 'a

val fold_errors_in :
  ?phase:phase ->
  t ->
  file:Relative_path.t ->
  init:'a ->
  f:(error -> 'a -> 'a) ->
  'a

val get_failed_files : t -> phase -> Relative_path.Set.t

val as_telemetry : t -> Telemetry.t

val sort : error list -> error list

val get_applied_fixmes : t -> applied_fixme list

module Callback : sig
  type t

  val apply :
    t ->
    ?code:int ->
    ?quickfixes:Quickfix.t list ->
    Pos.t * string ->
    (Pos_or_decl.t * string) list ->
    unit

  val from_on_error : (error -> unit) -> dflt_code:int -> t

  val always : (unit -> unit) -> t

  val with_side_effect : t -> eff:(unit -> unit) -> t

  val with_default_code : dflt_code:Typing.t -> t

  val retain_code : t -> t

  val current_claim_as_reason : t -> new_claim:Pos.t * string -> t

  val unify_error : t

  val index_type_mismatch : t

  val covariant_index_type_mismatch : t

  val expected_stringlike : t

  val constant_does_not_match_enum_type : t

  val enum_underlying_type_must_be_arraykey : t

  val enum_constraint_must_be_arraykey : t

  val enum_subtype_must_have_compatible_constraint : t

  val parameter_default_value_wrong_type : t

  val newtype_alias_must_satisfy_constraint : t

  val missing_return : t

  val inout_return_type_mismatch : t

  val class_constant_value_does_not_match_hint : t

  val class_property_initializer_type_does_not_match_hint : t

  val xhp_attribute_does_not_match_hint : t

  val record_init_value_does_not_match_hint : t

  val strict_str_concat_type_mismatch : t

  val strict_str_interp_type_mismatch : t

  val bitwise_math_invalid_argument : t

  val inc_dec_invalid_argument : t

  val math_invalid_argument : t

  val using_error : Pos.t -> bool -> t
end

module Reasons_callback : sig
  type t

  val apply :
    t ->
    ?code:int ->
    ?quickfixes:Quickfix.t list ->
    (Pos_or_decl.t * string) list ->
    unit

  val apply_with_reasons : t -> reasons:(Pos_or_decl.t * string) list -> unit

  val ignore_error : t

  val always : (unit -> unit) -> t

  val with_claim : Callback.t -> claim:Pos.t * string -> t

  val of_error : Typing.t -> Pos.t * string -> t

  val error_assert_primary_pos_in_current_decl_callback :
    default_code:Typing.t -> current_decl_and_file:Pos_or_decl.ctx -> t

  val with_code : t -> code:int -> t

  val with_default_code : t -> dflt_code:int -> t

  val retain_code : t -> t

  val with_reasons : t -> reasons:(Pos_or_decl.t * string) list -> t

  val append_reason : t -> reason:Pos_or_decl.t * string -> t

  val prepend_reason : t -> reason:Pos_or_decl.t * string -> t

  val prepend_reasons : t -> reasons:(Pos_or_decl.t * string) list -> t

  val retain_quickfixes : t -> t

  val unify_error_assert_primary_pos_in_current_decl :
    current_decl_and_file:Pos_or_decl.ctx -> t

  val invalid_type_hint_assert_primary_pos_in_current_decl :
    current_decl_and_file:Pos_or_decl.ctx -> t

  val unify_error_at : Pos.t -> t

  val hh_expect_error : equivalent:bool -> Pos.t -> t

  val bad_enum_decl : Pos.t -> t

  val bad_conditional_support_dynamic :
    Pos.t ->
    child:string ->
    parent:string ->
    ty_str:string ->
    self_ty_str:string ->
    t

  val bad_decl_override :
    Pos.t -> name:string -> parent_pos:Pos_or_decl.t -> parent_name:string -> t

  val explain_where_constraint :
    Pos.t -> in_class:bool -> decl_pos:Pos_or_decl.t -> t

  val explain_constraint : use_pos:Pos.t -> t

  val rigid_tvar_escape_at : Pos.t -> string -> t

  val invalid_type_hint : Pos.t -> t

  val type_constant_mismatch : t -> t

  val class_constant_type_mismatch : t -> t

  val unsatisfied_req_callback :
    class_pos:Pos.t ->
    trait_pos:Pos_or_decl.t ->
    req_pos:Pos_or_decl.t ->
    string ->
    t

  val invalid_echo_argument_at : Pos.t -> t

  val index_type_mismatch_at : Pos.t -> t
end

(***************************************
 *                                     *
 *       Specific errors               *
 *                                     *
 ***************************************)

val apply_callback_to_errors : t -> Reasons_callback.t -> unit

val internal_error : Pos.t -> string -> unit

val unimplemented_feature : Pos.t -> string -> unit

val experimental_feature : Pos.t -> string -> unit

val missing_field :
  Pos_or_decl.t -> Pos_or_decl.t -> string -> Reasons_callback.t -> unit

val violated_constraint :
  (Pos_or_decl.t * (Pos_or_decl.t * string)) list ->
  (Pos_or_decl.t * string) list ->
  Reasons_callback.t ->
  unit

val method_variance : Pos.t -> unit

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
  Pos_or_decl.t -> Pos_or_decl.t * string -> Reasons_callback.t -> unit

val mutating_const_property : Pos.t -> unit

val self_const_parent_not : Pos.t -> unit

val overriding_prop_const_mismatch :
  Pos_or_decl.t -> bool -> Pos_or_decl.t -> bool -> Reasons_callback.t -> unit

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

val nullsafe_property_write_context : Pos.t -> unit

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
  Reasons_callback.t ->
  unit

val interface_or_trait_const_multiple_defs :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  string ->
  Reasons_callback.t ->
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
  Reasons_callback.t ->
  unit

val format_string :
  Pos.t -> string -> string -> Pos_or_decl.t -> string -> string -> unit

val expected_literal_format_string : Pos.t -> unit

val re_prefixed_non_string : Pos.t -> string -> unit

val bad_regex_pattern : Pos.t -> string -> unit

val generic_array_strict : Pos.t -> unit

val option_return_only_typehint : Pos.t -> [ `void | `noreturn ] -> unit

val tuple_syntax : Pos.t -> unit

val redeclaring_missing_method : Pos.t -> string -> unit

val expecting_type_hint : Pos.t -> unit

val expecting_type_hint_variadic : Pos.t -> unit

val expecting_return_type_hint : Pos.t -> unit

val field_kinds : Pos.t -> Pos_or_decl.t -> unit

val unbound_name_typing : Pos.t -> string -> unit

val previous_default : Pos.t -> unit

val return_in_void : Pos.t -> Pos.t -> unit

val this_var_outside_class : Pos.t -> unit

val this_var_in_expr_tree : Pos.t -> unit

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

val self_abstract_call : string -> Pos.t -> Pos.t -> Pos_or_decl.t -> unit

val classname_abstract_call : string -> string -> Pos.t -> Pos_or_decl.t -> unit

val static_synthetic_method : string -> string -> Pos.t -> Pos_or_decl.t -> unit

val isset_in_strict : Pos.t -> unit

val isset_inout_arg : Pos.t -> unit

val unset_nonidx_in_strict : Pos.t -> (Pos_or_decl.t * string) list -> unit

val unpacking_disallowed_builtin_function : Pos.t -> string -> unit

val invalid_destructure :
  Pos_or_decl.t -> Pos_or_decl.t -> string -> Reasons_callback.t -> unit

val unpack_array_required_argument :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val unpack_array_variadic_argument :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

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
  ([< `instance | `static ] * Pos_or_decl.t * string) option ->
  Callback.t ->
  unit

val smember_not_found_ :
  [< `class_constant | `class_variable | `static_method | `class_typeconst ] ->
  Pos_or_decl.t ->
  Pos_or_decl.t * string ->
  string ->
  ([< `instance | `static ] * Pos_or_decl.t * string) option ->
  Reasons_callback.t ->
  unit

val member_not_found :
  [< `property | `method_ ] ->
  Pos.t ->
  Pos_or_decl.t * string ->
  string ->
  ([< `instance | `static ] * Pos_or_decl.t * string) option ->
  (Pos_or_decl.t * string) list ->
  Callback.t ->
  unit

val expr_tree_unsupported_operator : string -> string -> Pos.t -> unit

val parent_in_trait : Pos.t -> unit

val parent_undefined : Pos.t -> unit

val constructor_no_args : Pos.t -> unit

val visibility : Pos.t -> string -> Pos_or_decl.t -> string -> unit

val typing_too_many_args : int -> int -> Pos.t -> Pos_or_decl.t -> unit

val typing_too_many_args_w_callback :
  int -> int -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val typing_too_few_args : int -> int -> Pos.t -> Pos_or_decl.t -> unit

val typing_too_few_args_w_callback :
  int -> int -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val bad_call : Pos.t -> string -> unit

val extend_final : Pos.t -> Pos_or_decl.t -> string -> unit

val extend_non_abstract_record : string -> Pos.t -> Pos_or_decl.t -> unit

val extend_sealed : Pos.t -> Pos_or_decl.t -> string -> string -> string -> unit

val sealed_not_subtype :
  string -> Pos.t -> Pos_or_decl.t -> string -> string -> string -> unit

val trait_prop_const_class : Pos.t -> string -> unit

val read_before_write : Pos.t * string -> unit

val implement_abstract :
  ?quickfixes:Quickfix.t list ->
  is_final:bool ->
  Pos.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  unit

val generic_static : Pos.t -> string -> unit

val fun_too_many_args :
  int -> int -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val fun_too_few_args :
  int -> int -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val fun_unexpected_nonvariadic :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val fun_variadicity_hh_vs_php56 :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val expected_tparam :
  use_pos:Pos.t ->
  definition_pos:Pos_or_decl.t ->
  int ->
  Reasons_callback.t option ->
  unit

val object_string : Pos.t -> Pos_or_decl.t -> unit

val object_string_deprecated : Pos.t -> unit

val cyclic_typedef : Pos.t -> Pos_or_decl.t -> unit

val type_arity_mismatch :
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  string ->
  Reasons_callback.t ->
  unit

val this_final :
  Pos_or_decl.t * string -> Pos_or_decl.t -> (Pos_or_decl.t * string) list

val exact_class_final :
  Pos_or_decl.t * string -> Pos_or_decl.t -> (Pos_or_decl.t * string) list

val discarded_awaitable : Pos.t -> Pos_or_decl.t -> unit

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
  Callback.t ->
  unit

val non_object_member_read_ :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  Reasons_callback.t ->
  unit

val non_object_member_write :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos.t ->
  string ->
  Pos_or_decl.t ->
  Callback.t ->
  unit

val non_object_member_write_ :
  kind:[< `property | `method_ | `class_typeconst ] ->
  string ->
  Pos_or_decl.t ->
  string ->
  Pos_or_decl.t ->
  Reasons_callback.t ->
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
  parent_kind:Ast_defs.classish_kind ->
  parent_name:string ->
  child_pos:Pos.t ->
  child_kind:Ast_defs.classish_kind ->
  child_name:string ->
  unit

val unsatisfied_req :
  class_pos:Pos.t ->
  trait_pos:Pos_or_decl.t ->
  req_pos:Pos_or_decl.t ->
  string ->
  unit

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
  on_error:Reasons_callback.t ->
  unit

val override_lsb :
  member_name:string ->
  parent:Pos_or_decl.t ->
  child:Pos_or_decl.t ->
  Reasons_callback.t ->
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

val type_arity : Pos.t -> Pos_or_decl.t -> expected:int -> actual:int -> unit

val visibility_extends :
  string ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  Reasons_callback.t ->
  unit

val visibility_override_internal :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string option ->
  string ->
  Reasons_callback.t ->
  unit

val member_not_implemented :
  string -> Pos_or_decl.t -> Pos.t -> Pos_or_decl.t -> Quickfix.t list -> unit

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
  Reasons_callback.t ->
  pos:Pos_or_decl.t ->
  member_name:string ->
  Reasons_callback.t

val bad_prop_override :
  Reasons_callback.t ->
  pos:Pos_or_decl.t ->
  member_name:string ->
  Reasons_callback.t

val bad_enum_decl : Pos.t -> (Pos_or_decl.t * string) list -> unit

val missing_constructor : Pos_or_decl.t -> Reasons_callback.t -> unit

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
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val invalid_shape_remove_key : Pos.t -> unit

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

val attribute_too_many_arguments : Pos.t -> string -> int -> unit

val attribute_too_few_arguments : Pos.t -> string -> int -> unit

val attribute_not_exact_number_of_args :
  Pos.t -> attr_name:string -> expected_args:int -> actual_args:int -> unit

val attribute_param_type : Pos.t -> string -> unit

val deprecated_use : Pos.t -> ?pos_def:Pos_or_decl.t option -> string -> unit

val cannot_declare_constant :
  [< `enum | `record ] -> Pos.t -> Pos.t * string -> unit

val ambiguous_inheritance :
  Pos_or_decl.t -> string -> string -> error -> Reasons_callback.t -> unit

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

val cyclic_enum_constraint : Pos_or_decl.t -> Reasons_callback.t -> unit

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

val required_field_is_optional :
  Pos_or_decl.t -> Pos_or_decl.t -> string -> Reasons_callback.t -> unit

val array_get_with_optional_field : Pos.t -> Pos_or_decl.t -> string -> unit

val nullable_cast : Pos.t -> string -> Pos_or_decl.t -> unit

val return_disposable_mismatch :
  bool -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val dollardollar_lvalue : Pos.t -> unit

val unsafe_cast_lvalue : Pos.t -> unit

val unsafe_cast_await : Pos.t -> unit

val duplicate_using_var : Pos.t -> unit

val illegal_disposable : Pos.t -> string -> unit

val escaping_disposable : Pos.t -> unit

val escaping_disposable_parameter : Pos.t -> unit

val escaping_this : Pos.t -> unit

val must_extend_disposable : Pos.t -> unit

val accept_disposable_invariant :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val ifc_external_contravariant :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val inout_annotation_missing : Pos.t -> Pos_or_decl.t -> unit

val inout_annotation_unexpected :
  Pos.t -> Pos_or_decl.t -> bool -> Pos.t -> unit

val inoutness_mismatch :
  Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val xhp_required : Pos.t -> string -> (Pos_or_decl.t * string) list -> unit

val illegal_xhp_child : Pos.t -> (Pos_or_decl.t * string) list -> unit

val missing_xhp_required_attr :
  Pos.t -> string -> (Pos_or_decl.t * string) list -> unit

val inout_argument_bad_type : Pos.t -> (Pos_or_decl.t * string) list -> unit

val ambiguous_lambda : Pos.t -> (Pos_or_decl.t * string) list -> unit

val ellipsis_strict_mode :
  require:[< `Param_name | `Type_and_param_name ] -> Pos.t -> unit

val untyped_lambda_strict_mode : Pos.t -> unit

val wrong_expression_kind_attribute :
  string -> Pos.t -> string -> Pos_or_decl.t -> string -> string -> unit

val wrong_expression_kind_builtin_attribute : string -> Pos.t -> string -> unit

val decl_override_missing_hint : Pos_or_decl.t -> Reasons_callback.t -> unit

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

val invalid_set_value :
  Pos.t -> Pos_or_decl.t * string -> Pos_or_decl.t * string -> unit

val invalid_arraykey_constraint : Pos.t -> string -> unit

val lateinit_with_default : Pos.t -> unit

val bad_lateinit_override :
  bool -> Pos_or_decl.t -> Pos_or_decl.t -> Reasons_callback.t -> unit

val bad_xhp_attr_required_override :
  string ->
  string ->
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  Reasons_callback.t ->
  unit

val multiple_concrete_defs :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  string ->
  string ->
  Reasons_callback.t ->
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

val unresolved_type_variable : Pos.t -> unit

val invalid_sub_string : Pos.t -> string -> unit

val static_meth_with_class_reified_generic : Pos.t -> Pos.t -> unit

val exception_occurred : Pos.t -> Exception.t -> unit

(* The intention is to introduce invariant violations with `report_to_user`
set to `false` initially. Then we observe and confirm that the invariant is
not repeatedly violated. Only then, we set it to `true` in a subsequent
release. This should prevent us from blocking users unexpectedly while
gradually introducing signal for unexpected compiler states. *)
val invariant_violation :
  report_to_user:bool -> desc:string -> Pos.t -> Telemetry.t -> unit

val redundant_covariant : Pos.t -> string -> string -> unit

val meth_caller_trait : Pos.t -> string -> unit

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

val kind_mismatch :
  use_pos:Pos.t ->
  def_pos:Pos_or_decl.t ->
  tparam_name:string ->
  expected_kind_repr:string ->
  actual_kind_repr:string ->
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
  Reasons_callback.t ->
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

val via_label_invalid_parameter : Pos.t -> unit

val via_label_invalid_parameter_in_enum_class : Pos.t -> unit

val via_label_invalid_generic : Pos.t -> string -> unit

val enum_class_label_unknown : Pos.t -> string -> string -> unit

val enum_class_label_as_expr : Pos.t -> unit

val enum_class_label_invalid_argument : Pos.t -> is_proj:bool -> unit

val ifc_internal_error : Pos.t -> string -> unit

val ifc_policy_mismatch :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  Reasons_callback.t ->
  unit

val override_method_support_dynamic_type :
  Pos_or_decl.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  Reasons_callback.t ->
  unit

val parent_support_dynamic_type :
  Pos.t ->
  string * Ast_defs.classish_kind ->
  string * Ast_defs.classish_kind ->
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
  Reasons_callback.t ->
  unit

val explicit_readonly_cast : string -> Pos.t -> Pos_or_decl.t -> unit

val readonly_method_call : Pos.t -> Pos_or_decl.t -> unit

val invalid_meth_caller_calling_convention :
  Pos.t -> Pos_or_decl.t -> string -> unit

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

val function_pointer_with_via_label : Pos.t -> Pos_or_decl.t -> unit

val reified_static_method_in_expr_tree : Pos.t -> unit

val module_mismatch : Pos.t -> Pos_or_decl.t -> string option -> string -> unit

val module_hint : def_pos:Pos_or_decl.t -> use_pos:Pos.t -> unit

val expression_tree_non_public_member :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> unit

val not_sub_dynamic :
  (Pos_or_decl.t * string) list ->
  Pos_or_decl.t ->
  string ->
  Reasons_callback.t ->
  unit

val trait_parent_construct_inconsistent : Pos.t -> Pos_or_decl.t -> unit

val explicit_consistent_constructor : Ast_defs.classish_kind -> Pos.t -> unit
