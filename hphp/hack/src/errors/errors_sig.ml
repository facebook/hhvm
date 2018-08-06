(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  type 'a error_
  type error = Pos.t error_
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
  type phase = Init | Parsing | Naming | Decl | Typing
  type severity = Warning | Error

  module Parsing : Error_category
  module Naming : Error_category
  module NastCheck : Error_category
  module Typing : Error_category

  (* Error codes that can never be suppressed with a FIXME. *)
  val default_ignored_fixme_codes : ISet.t
  (* Error codes that cannot be suppressed with a FIXME based on configuration. *)
  val ignored_fixme_codes : ISet.t ref

  val set_allow_errors_in_default_path : bool -> unit

  val is_hh_fixme : (Pos.t -> int -> bool) ref
  val get_hh_fixme_pos : (Pos.t -> int -> Pos.t option) ref
  val to_list : 'a error_ -> ('a * string) list
  val get_code : 'a error_ -> int
  val get_pos : error -> Pos.t
  val get_severity : 'a error_ -> severity
  val make_error : int -> (Pos.t * string) list -> error

  val error_code_to_string : int -> string

  val internal_error : Pos.t -> string -> unit
  val unimplemented_feature : Pos.t -> string -> unit
  val experimental_feature : Pos.t -> string -> unit

  val fixme_format : Pos.t -> unit
  val typeparam_alok : Pos.t * string -> unit
  val unexpected_eof : Pos.t -> unit
  val missing_field : Pos.t -> Pos.t -> string -> unit
  val generic_class_var : Pos.t -> unit
  val explain_constraint :
    use_pos:Pos.t -> definition_pos:Pos.t -> param_name:string -> error -> unit
  val explain_where_constraint :
    use_pos:Pos.t -> definition_pos:Pos.t -> error -> unit
  val explain_tconst_where_constraint :
    use_pos:Pos.t -> definition_pos:Pos.t -> error -> unit
  val explain_type_constant : (Pos.t * string) list -> error -> unit
  val unexpected_arrow : Pos.t -> string -> unit
  val missing_arrow : Pos.t -> string -> unit
  val disallowed_xhp_type : Pos.t -> string -> unit
  val overflow : Pos.t -> unit
  val unterminated_comment : Pos.t -> unit
  val unterminated_xhp_comment : Pos.t -> unit
  val name_already_bound : string -> Pos.t -> Pos.t -> unit
  val name_is_reserved : string -> Pos.t -> unit
  val dollardollar_unused : Pos.t -> unit
  val assigning_to_const : Pos.t -> unit
  val self_const_parent_not : Pos.t -> unit
  val parent_const_self_not : Pos.t -> unit
  val overriding_prop_const_mismatch : Pos.t -> bool -> Pos.t -> bool -> unit
  val method_name_already_bound : Pos.t -> string -> unit
  val error_name_already_bound : string -> string -> Pos.t -> Pos.t -> unit
  val error_class_attribute_already_bound : string -> string -> Pos.t -> Pos.t -> unit
  val unbound_name : Pos.t -> string -> [< `cls | `func | `const ] -> unit
  val different_scope : Pos.t -> string -> Pos.t -> unit
  val undefined : Pos.t -> string -> unit
  val this_reserved : Pos.t -> unit
  val start_with_T : Pos.t -> unit
  val already_bound : Pos.t -> string -> unit
  val unexpected_typedef : Pos.t -> Pos.t -> unit
  val fd_name_already_bound : Pos.t -> unit
  val primitive_toplevel : Pos.t -> unit
  val primitive_invalid_alias : Pos.t -> string -> string -> unit
  val dynamic_new_in_strict_mode : Pos.t -> unit
  val xhp_optional_required_attr : Pos.t -> string -> unit
  val xhp_required_with_default : Pos.t -> string -> unit
  val variable_variables_disallowed : Pos.t -> unit
  val array_typehints_disallowed : Pos.t -> unit
  val array_literals_disallowed : Pos.t -> unit
  val wildcard_disallowed : Pos.t -> unit
  val void_cast: Pos.t -> unit
  val object_cast: Pos.t -> string -> unit
  val unset_cast: Pos.t -> unit
  val this_no_argument : Pos.t -> unit
  val this_hint_outside_class : Pos.t -> unit
  val this_type_forbidden : Pos.t -> unit
  val lowercase_this : Pos.t -> string -> unit
  val classname_param : Pos.t -> unit
  val invalid_instanceof : Pos.t -> unit
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
  val dynamic_method_call : Pos.t -> unit
  val nullsafe_property_write_context : Pos.t -> unit
  val illegal_fun : Pos.t -> unit
  val illegal_member_variable_class : Pos.t -> unit
  val illegal_meth_fun : Pos.t -> unit
  val illegal_inst_meth : Pos.t -> unit
  val illegal_meth_caller : Pos.t -> unit
  val illegal_class_meth : Pos.t -> unit
  val assert_arity : Pos.t -> unit
  val gena_arity : Pos.t -> unit
  val genva_arity : Pos.t -> unit
  val gen_array_rec_arity : Pos.t -> unit
  val uninstantiable_class : Pos.t -> Pos.t -> string -> (Pos.t * string) list
    -> unit
  val abstract_const_usage: Pos.t -> Pos.t -> string -> unit
  val add_a_typehint : Pos.t -> unit
  val local_const : Pos.t -> unit
  val illegal_constant : Pos.t -> unit
  val parsing_error : Pos.t * string -> unit
  val format_string :
    Pos.t -> string -> string -> Pos.t -> string -> string -> unit
  val expected_literal_string : Pos.t -> unit
  val re_prefixed_non_string : Pos.t -> string -> unit
  val bad_regex_pattern : Pos.t -> string -> unit
  val generic_array_strict : Pos.t -> unit
  val strict_members_not_known : Pos.t -> string -> unit
  val option_return_only_typehint : Pos.t -> [< `void | `noreturn ] -> unit
  val tuple_syntax : Pos.t -> unit
  val class_arity : Pos.t -> Pos.t -> string -> int -> unit
  val expecting_type_hint : Pos.t -> unit
  val expecting_type_hint_suggest : Pos.t -> string -> unit
  val expecting_return_type_hint : Pos.t -> unit
  val expecting_return_type_hint_suggest : Pos.t -> string -> unit
  val expecting_awaitable_return_type_hint : Pos.t -> unit
  val field_kinds : Pos.t -> Pos.t -> unit
  val unbound_name_typing : Pos.t -> string -> unit
  val did_you_mean_naming : Pos.t -> string -> Pos.t -> string -> unit
  val previous_default : Pos.t -> unit
  val return_only_typehint : Pos.t -> [< `void | `noreturn ] -> unit
  val unexpected_type_arguments : Pos.t -> unit
  val too_many_type_arguments : Pos.t -> unit
  val return_in_void : Pos.t -> Pos.t -> unit
  val this_in_static : Pos.t -> unit
  val this_var_outside_class : Pos.t -> unit
  val unbound_global : Pos.t -> unit
  val private_inst_meth : Pos.t -> Pos.t -> unit
  val protected_inst_meth : Pos.t -> Pos.t -> unit
  val private_class_meth : Pos.t -> Pos.t -> unit
  val protected_class_meth : Pos.t -> Pos.t -> unit
  val array_cast : Pos.t -> unit
  val anonymous_recursive : Pos.t -> unit
  val static_outside_class : Pos.t -> unit
  val self_outside_class : Pos.t -> unit
  val new_inconsistent_construct : Pos.t -> (Pos.t * string)
    -> [< `static | `classname ] -> unit
  val pair_arity : Pos.t -> unit
  val tuple_arity : Pos.t -> int -> Pos.t -> int -> unit
  val undefined_parent : Pos.t -> unit
  val parent_outside_class : Pos.t -> unit
  val parent_abstract_call : string -> Pos.t -> Pos.t -> unit
  val self_abstract_call : string -> Pos.t -> Pos.t -> unit
  val classname_abstract_call : string -> string -> Pos.t -> Pos.t -> unit
  val static_synthetic_method : string -> string -> Pos.t -> Pos.t -> unit
  val empty_in_strict : Pos.t -> unit
  val isset_in_strict : Pos.t -> unit
  val unset_nonidx_in_strict : Pos.t -> (Pos.t * string) list -> unit
  val unpacking_disallowed_builtin_function : Pos.t -> string -> unit
  val array_get_arity : Pos.t -> string -> Pos.t -> unit
  val typing_error : Pos.t -> string -> unit
  val typing_error_l : error -> unit
  val undefined_field :
    use_pos: Pos.t -> name: string -> shape_type_pos: Pos.t -> unit
  val array_access : Pos.t -> Pos.t -> string -> unit
  val keyset_set : Pos.t -> Pos.t -> unit
  val array_append : Pos.t -> Pos.t -> string -> unit
  val const_mutation : Pos.t -> Pos.t -> string -> unit
  val expected_class : ?suffix:string -> Pos.t -> unit
  val smember_not_found :
    [< `class_constant | `class_variable | `static_method | `class_typeconst] ->
    Pos.t ->
    Pos.t * string ->
    string ->
    [< `closest of Pos.t * string
      | `did_you_mean of Pos.t * string
      | `no_hint ] ->
    unit
  val not_found_hint :
    [< `closest of 'a * string | `did_you_mean of 'a * string | `no_hint ] ->
    ('a * string) list
  val member_not_found :
    [< `member | `method_ ] ->
    Pos.t ->
    Pos.t * string ->
    string ->
    [< `closest of Pos.t * string
      | `did_you_mean of Pos.t * string
      | `no_hint ] ->
    (Pos.t * string) list ->
    unit
  val parent_in_trait : Pos.t -> unit
  val parent_undefined : Pos.t -> unit
  val constructor_no_args : Pos.t -> unit
  val visibility : Pos.t -> string -> Pos.t -> string -> unit
  val typing_too_many_args : Pos.t -> Pos.t -> unit
  val typing_too_few_args : Pos.t -> Pos.t -> unit
  val anonymous_recursive_call : Pos.t -> unit
  val bad_call : Pos.t -> string -> unit
  val sketchy_null_check : Pos.t -> string option -> [< `Coalesce | `Eq | `Neq ] -> unit
  val sketchy_null_check_primitive : Pos.t -> string option -> [< `Coalesce | `Eq | `Neq ] -> unit
  val extend_final : Pos.t -> Pos.t -> string -> unit
  val extend_sealed : Pos.t -> Pos.t -> string -> string -> string -> unit
  val trait_implement_sealed : Pos.t -> Pos.t -> string -> unit
  val extend_ppl : Pos.t -> string -> bool -> Pos.t -> string -> string -> string -> unit
  val sealed_final : Pos.t -> string -> unit
  val unsealable : Pos.t -> string -> unit
  val read_before_write : Pos.t * string -> unit
  val interface_final : Pos.t -> unit
  val trait_final : Pos.t -> unit
  val implement_abstract :
    is_final:bool -> Pos.t -> Pos.t -> string -> string -> unit
  val generic_static : Pos.t -> string -> unit
  val fun_too_many_args : Pos.t -> Pos.t -> unit
  val fun_too_few_args : Pos.t -> Pos.t -> unit
  val fun_unexpected_nonvariadic : Pos.t -> Pos.t -> unit
  val fun_variadicity_hh_vs_php56 : Pos.t -> Pos.t -> unit
  val expected_tparam : Pos.t -> int -> unit
  val object_string : Pos.t -> Pos.t -> unit
  val type_param_arity : Pos.t -> string -> string -> unit
  val cyclic_typedef : Pos.t -> unit
  val type_arity_mismatch : Pos.t -> string -> Pos.t -> string -> unit
  val this_final : Pos.t * string -> Pos.t -> error -> unit
  val exact_class_final : Pos.t * string -> Pos.t -> error -> unit
  val tuple_arity_mismatch : Pos.t -> string -> Pos.t -> string -> unit
  val fun_arity_mismatch : Pos.t -> Pos.t -> unit
  val discarded_awaitable : Pos.t -> Pos.t -> unit
  val gena_expects_array : Pos.t -> Pos.t -> string -> unit
  val unify_error : (Pos.t * string) list -> (Pos.t * string) list -> unit
  val static_dynamic : Pos.t -> Pos.t -> string -> unit
  val null_member : string -> Pos.t -> (Pos.t * string) list -> unit
  val non_object_member : string -> Pos.t -> string -> Pos.t -> unit
  val non_class_member : string -> Pos.t -> string -> Pos.t -> unit
  val ambiguous_member : string -> Pos.t -> string -> Pos.t -> unit
  val null_container : Pos.t -> (Pos.t * string) list -> unit
  val option_mixed : Pos.t -> unit
  val option_void : Pos.t -> unit
  val declared_covariant : Pos.t -> Pos.t -> (Pos.t * string) list -> unit
  val declared_contravariant : Pos.t -> Pos.t -> (Pos.t * string) list -> unit
  val contravariant_this: Pos.t -> string -> string -> unit
  val wrong_extend_kind : Pos.t -> string -> Pos.t -> string -> unit
  val unsatisfied_req : Pos.t -> string -> Pos.t -> unit
  val cyclic_class_def : SSet.t -> Pos.t -> unit
  val trait_reuse : (Pos.t) -> string -> (Pos.t * string) -> string -> unit
  val invalid_is_as_expression_hint : string -> Pos.t -> Pos.t -> string -> unit
  val partially_valid_is_as_expression_hint :
    string -> Pos.t -> Pos.t -> string -> unit
  val override_final : parent:Pos.t -> child:Pos.t -> unit
  val should_be_override : Pos.t -> string -> string -> unit
  val override_per_trait : Pos.t * string -> string -> Pos.t -> unit
  val missing_assign : Pos.t -> unit
  val private_override : Pos.t -> string -> string -> unit
  val invalid_memoized_param : Pos.t -> (Pos.t * string) list -> unit
  val no_construct_parent : Pos.t -> unit
  val constructor_required : Pos.t * string -> SSet.t -> unit
  val not_initialized : Pos.t * string -> SSet.t -> unit
  val call_before_init : Pos.t -> string -> unit
  val type_arity : Pos.t -> string -> string -> unit
  val invalid_req_implements : Pos.t -> unit
  val invalid_req_extends : Pos.t -> unit
  val abstract_with_body : Pos.t * 'a -> unit
  val not_abstract_without_body : Pos.t * 'a -> unit
  val return_in_gen : Pos.t -> unit
  val return_in_finally : Pos.t -> unit
  val toplevel_break: Pos.t -> unit
  val toplevel_continue: Pos.t -> unit
  val continue_in_switch: Pos.t -> unit
  val await_in_sync_function : Pos.t -> unit
  val await_not_allowed : Pos.t -> unit
  val async_in_interface : Pos.t -> unit
  val await_in_coroutine : Pos.t -> unit
  val yield_in_coroutine : Pos.t -> unit
  val suspend_outside_of_coroutine : Pos.t -> unit
  val suspend_in_finally : Pos.t -> unit
  val break_continue_n_not_supported : Pos.t -> unit
  val static_memoized_function : Pos.t -> unit
  val magic : Pos.t * string -> unit
  val non_interface : Pos.t -> string -> string -> unit
  val toString_returns_string : Pos.t -> unit
  val toString_visibility : Pos.t -> unit
  val uses_non_trait : Pos.t -> string -> string -> unit
  val requires_non_class : Pos.t -> string -> string -> unit
  val abstract_body : Pos.t -> unit
  val not_public_or_protected_interface : Pos.t -> unit
  val interface_with_member_variable : Pos.t -> unit
  val interface_with_static_member_variable : Pos.t -> unit
  val dangerous_method_name : Pos.t -> unit
  val illegal_function_name : Pos.t -> string -> unit
  val case_fallthrough : Pos.t -> Pos.t -> unit
  val default_fallthrough : Pos.t -> unit
  val visibility_extends : string -> Pos.t -> Pos.t -> string -> unit
  val member_not_implemented : string -> Pos.t -> Pos.t -> Pos.t -> unit
  val bad_decl_override : Pos.t -> string -> Pos.t -> string -> error -> unit
  val bad_method_override : Pos.t -> string -> error -> unit
  val bad_enum_decl : Pos.t -> error -> unit
  val missing_constructor : Pos.t -> unit
  val enum_constant_type_bad : Pos.t -> Pos.t -> string -> Pos.t list -> unit
  val enum_type_bad : Pos.t -> string -> Pos.t list -> unit
  val enum_type_typedef_mixed : Pos.t -> unit
  val enum_type_typedef_nonnull : Pos.t -> unit
  val enum_switch_redundant : string -> Pos.t -> Pos.t -> unit
  val enum_switch_nonexhaustive : Pos.t -> string list -> Pos.t -> unit
  val enum_switch_redundant_default : Pos.t -> Pos.t -> unit
  val enum_switch_not_const : Pos.t -> unit
  val enum_switch_wrong_class : Pos.t -> string -> string -> unit
  val invalid_shape_field_name : Pos.t -> unit
  val invalid_shape_field_name_empty : Pos.t -> unit
  val invalid_shape_field_name_number : Pos.t -> unit
  val invalid_shape_field_type : Pos.t -> Pos.t -> string -> Pos.t list -> unit
  val invalid_shape_field_literal : Pos.t -> Pos.t -> unit
  val invalid_shape_field_const : Pos.t -> Pos.t -> unit
  val shape_field_class_mismatch : Pos.t -> Pos.t -> string -> string -> unit
  val shape_field_type_mismatch : Pos.t -> Pos.t -> string -> string -> unit
  val shape_fields_unknown: Pos.t -> Pos.t  -> unit
  val invalid_shape_remove_key : Pos.t -> unit
  val shape_field_unset : Pos.t -> Pos.t -> string -> unit
  val using_internal_class : Pos.t -> string -> unit
  val nullsafe_not_needed : Pos.t -> (Pos.t * string) list -> unit
  val trivial_strict_eq : Pos.t -> string -> (Pos.t * string) list
    -> (Pos.t * string) list -> Pos.t list -> Pos.t list -> unit
  val trivial_strict_not_nullable_compare_null : Pos.t -> string
    -> (Pos.t * string) list -> unit
  val void_usage : Pos.t -> (Pos.t * string) list -> unit
  val noreturn_usage : Pos.t -> (Pos.t * string) list -> unit
  val generic_at_runtime : Pos.t -> unit
  val interface_with_partial_typeconst : Pos.t -> unit
  val multiple_xhp_category : Pos.t -> unit
  val not_abstract_without_typeconst : (Pos.t * string) -> unit
  val typeconst_depends_on_external_tparam : Pos.t -> Pos.t -> string -> unit
  val typeconst_assigned_tparam : Pos.t -> string -> unit
  val invalid_type_access_root : (Pos.t * string) -> unit
  val duplicate_user_attribute : (Pos.t * string) -> Pos.t -> unit
  val unbound_attribute_name : Pos.t -> string -> unit
  val attribute_too_many_arguments : Pos.t -> string -> int -> unit
  val attribute_too_few_arguments : Pos.t -> string -> int -> unit
  val attribute_param_type : Pos.t -> string -> unit
  val deprecated_use : Pos.t -> Pos.t -> string -> unit
  val abstract_with_typeconst : (Pos.t * string) -> unit
  val cannot_declare_constant:
    [< `enum | `trait] -> Pos.t -> (Pos.t * string) -> unit
  val ambiguous_inheritance: Pos.t -> string -> string -> error -> unit
  val cyclic_typeconst : Pos.t -> string list -> unit
  val explain_contravariance : Pos.t -> string -> error -> unit
  val explain_invariance : Pos.t -> string -> string -> error -> unit
  val this_lvalue : Pos.t -> unit
  val abstract_concrete_override:
    Pos.t -> Pos.t -> [< `method_ | `typeconst |`constant]-> unit
  val local_variable_modified_and_used : Pos.t -> Pos.t list -> unit
  val local_variable_modified_twice : Pos.t -> Pos.t list -> unit
  val assign_during_case : Pos.t -> unit
  val cyclic_enum_constraint : Pos.t -> unit
  val invalid_classname : Pos.t -> unit
  val illegal_type_structure : Pos.t -> string -> unit
  val illegal_typeconst_direct_access : Pos.t -> unit
  val class_property_only_static_literal : Pos.t -> unit
  val reference_expr : Pos.t -> unit
  val unification_cycle : Pos.t -> string -> unit
  val eq_incompatible_types : Pos.t -> (Pos.t * string) list
    -> (Pos.t * string) list -> unit
  val comparison_invalid_types : Pos.t -> (Pos.t * string) list
    -> (Pos.t * string) list -> unit
  val instanceof_generic_classname : Pos.t -> string -> unit
  val final_property : Pos.t -> unit
  val pass_by_ref_annotation_missing : Pos.t -> Pos.t -> unit
  val reffiness_invariant : Pos.t -> Pos.t -> [< `normal | `inout ] -> unit
  val pass_by_ref_annotation_unexpected : Pos.t -> Pos.t -> bool -> unit
  val invalid_new_disposable : Pos.t -> unit
  val invalid_disposable_hint : Pos.t -> string -> unit
  val invalid_disposable_return_hint : Pos.t -> string -> unit
  val invalid_return_disposable : Pos.t -> unit
  val unsupported_feature : Pos.t -> string -> unit
  val to_json : Pos.absolute error_ -> Hh_json.json
  val to_string : ?indent:bool -> Pos.absolute error_ -> string
  val try_ : (unit -> 'a) -> (error -> 'a) -> 'a
  val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a
  val try_add_err : Pos.t -> string -> (unit -> 'a) -> (unit -> 'a) -> 'a

  (* The type of collections of errors *)
  type t

  val do_ : (unit -> 'a) -> t * 'a
  val do_with_context :
    Relative_path.t -> phase ->  (unit -> 'a) -> t * 'a

  val run_in_context : Relative_path.t -> phase ->  (unit -> 'a) -> 'a
  val run_in_decl_mode : Relative_path.t -> (unit -> 'a) -> 'a
  val ignore_ : (unit -> 'a) -> 'a
  val try_when :
    (unit -> 'a) -> when_:(unit -> bool) -> do_:(error -> unit) -> 'a
  val has_no_errors : (unit -> 'a) -> bool
  val currently_has_errors : unit -> bool
  val must_error : (unit -> unit) -> (unit -> unit) -> unit
  val to_absolute : error -> Pos.absolute error_

  val merge : t -> t -> t
  val merge_into_current : t -> unit

  val incremental_update_set:
    old:t -> new_:t -> rechecked:Relative_path.Set.t -> phase -> t
  val incremental_update_map:
    old:t -> new_:t -> rechecked:'a Relative_path.Map.t -> phase -> t

  val empty : t
  val is_empty : t -> bool
  val count : t -> int
  val get_error_list : t -> error list
  val get_sorted_error_list : t -> error list
  val from_error_list : error list -> t
  val iter_error_list : (error -> unit) -> t -> unit
  val fold_errors :
    ?phase:phase ->
     t ->
    init:'a ->
    f:(Relative_path.t -> error -> 'a -> 'a) ->
    'a
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
  val darray_not_supported : Pos.t -> unit
  val varray_not_supported : Pos.t -> unit
  val nonnull_not_supported : Pos.t -> unit
  val too_few_type_arguments : Pos.t -> unit
  val required_field_is_optional : Pos.t -> Pos.t -> string -> unit
  val array_get_with_optional_field : Pos.t -> Pos.t -> string -> unit
  val goto_label_already_defined : string -> Pos.t -> Pos.t -> unit
  val goto_label_undefined : Pos.t -> string -> unit
  val goto_label_defined_in_finally : Pos.t -> unit
  val goto_invoked_in_finally : Pos.t -> unit
  val method_needs_visibility : Pos.t -> unit
  val dynamic_class_property_name_in_strict_mode : Pos.t -> unit
  val dynamic_class_name_in_strict_mode : Pos.t -> unit
  val reading_from_append: Pos.t -> unit
  val const_attribute_prohibited: Pos.t -> string -> unit
  val varray_or_darray_not_supported : Pos.t -> unit
  val unknown_field_disallowed_in_shape : Pos.t -> Pos.t -> string -> unit
  val nullable_cast : Pos.t -> string -> Pos.t -> unit
  val non_call_argument_in_suspend : Pos.t -> (Pos.t * string) list -> unit
  val non_coroutine_call_in_suspend : Pos.t -> (Pos.t * string) list -> unit
  val coroutine_call_outside_of_suspend : Pos.t -> unit
  val function_is_not_coroutine : Pos.t -> string -> unit
  val coroutinness_mismatch : bool -> Pos.t -> Pos.t -> unit
  val invalid_ppl_call : Pos.t -> string -> unit
  val invalid_ppl_static_call : Pos.t -> string -> unit
  val ppl_meth_pointer : Pos.t -> string -> unit
  val coroutine_outside_experimental : Pos.t -> unit
  val return_disposable_mismatch : bool -> Pos.t -> Pos.t -> unit
  val fun_reactivity_mismatch : Pos.t -> string -> Pos.t -> string -> unit
  val frozen_in_incorrect_scope : Pos.t -> unit
  val reassign_mutable_var : Pos.t -> unit
  val mutable_call_on_immutable : Pos.t -> Pos.t -> unit
  val mutable_argument_mismatch : Pos.t -> Pos.t -> unit
  val invalid_mutable_return_result: Pos.t -> Pos.t -> string -> unit
  val mutable_return_result_mismatch: bool -> Pos.t -> Pos.t -> unit
  val mutable_params_outside_of_sync : Pos.t -> Pos.t -> string -> string -> unit
  val mutable_async_method : Pos.t -> unit
  val mutable_attribute_on_function : Pos.t -> unit
  val mutable_methods_must_be_reactive : Pos.t -> string ->  unit
  val mutable_return_annotated_decls_must_be_reactive : string -> Pos.t -> string ->  unit
  val invalid_freeze_target : Pos.t -> Pos.t -> string -> unit
  val invalid_freeze_use : Pos.t -> unit
  val freeze_in_nonreactive_context : Pos.t -> unit
  val this_as_lexical_variable : Pos.t -> unit
  val dollardollar_lvalue : Pos.t -> unit
  val duplicate_using_var : Pos.t -> unit
  val illegal_disposable : Pos.t -> string -> unit
  val escaping_disposable : Pos.t -> unit
  val escaping_disposable_parameter : Pos.t -> unit
  val escaping_this : Pos.t -> unit
  val must_extend_disposable : Pos.t -> unit
  val accept_disposable_invariant : Pos.t -> Pos.t -> unit
  val inout_params_outside_of_sync : Pos.t -> unit
  val inout_params_special : Pos.t -> unit
  val inout_params_mix_byref : Pos.t -> Pos.t -> unit
  val inout_params_memoize : Pos.t -> Pos.t -> unit
  val obj_set_reactive : Pos.t -> unit
  val global_in_reactive_context : Pos.t -> string -> unit
  val static_property_in_reactive_context : Pos.t -> unit
  val inout_annotation_missing : Pos.t -> Pos.t -> unit
  val inout_annotation_unexpected : Pos.t -> Pos.t -> bool -> unit
  val inoutness_mismatch : Pos.t -> Pos.t -> unit
  val inout_params_ret_by_ref : Pos.t -> Pos.t -> unit
  val xhp_required : Pos.t -> string -> (Pos.t * string) list -> unit
  val illegal_xhp_child : Pos.t -> (Pos.t * string) list -> unit
  val nonreactive_function_call : Pos.t -> Pos.t -> unit
  val nonreactive_append : Pos.t -> unit
  val inout_argument_bad_expr : Pos.t -> unit
  val inout_argument_bad_type : Pos.t -> (Pos.t * string) list -> unit
  val nonreactive_call_from_shallow : Pos.t -> Pos.t -> unit
  val illegal_destructor : Pos.t -> unit
  val rx_enabled_in_non_rx_context : Pos.t -> unit
  val rx_enabled_in_lambdas : Pos.t -> unit
  val ambiguous_lambda : Pos.t -> (Pos.t * string) list -> unit
  val ellipsis_strict_mode :
    require:[< `Param_name | `Type | `Type_and_param_name ] -> Pos.t -> unit
  val untyped_lambda_strict_mode : Pos.t -> unit
  val binding_ref_in_array : Pos.t -> unit
  val return_ref_in_array : Pos.t -> unit
  val passing_array_cell_by_ref : Pos.t -> unit
  val conditionally_reactive_function : Pos.t -> unit
  val multiple_conditionally_reactive_annotations : Pos.t -> string -> unit
  val conditionally_reactive_annotation_invalid_arguments : is_method:bool -> Pos.t -> unit
  val echo_in_reactive_context : Pos.t -> unit
  val superglobal_in_reactive_context : Pos.t -> string -> unit
  val static_in_reactive_context : Pos.t -> string -> unit
  val missing_reactivity_for_condition: Pos.t -> unit
  val multiple_reactivity_annotations: Pos.t -> unit
  val rx_is_enabled_invalid_location: Pos.t -> unit
  val wrong_expression_kind_attribute: string -> Pos.t -> string -> Pos.t -> string -> string -> unit
  val attribute_class_no_constructor_args: Pos.t -> Pos.t -> unit
  val cannot_return_borrowed_value_as_immutable: Pos.t -> Pos.t -> unit
  val decl_override_missing_hint: Pos.t -> unit
  val let_var_immutability_violation : Pos.t -> string -> unit
  val onlyrx_if_rxfunc_invalid_location: Pos.t -> unit
  val no_onlyrx_if_rxfunc_for_rx_if_args: Pos.t -> unit
  val coroutine_in_constructor: Pos.t -> unit
  val illegal_return_by_ref: Pos.t -> unit
  val illegal_by_ref_expr: Pos.t -> string -> unit
  val variadic_byref_param: Pos.t -> unit
  val reference_in_strict_mode: Pos.t -> unit
  val invalid_type_for_onlyrx_if_rxfunc_parameter: Pos.t -> string -> unit
  val missing_annotation_for_onlyrx_if_rxfunc_parameter: Pos.t -> unit
  val mutable_in_nonreactive_context: Pos.t -> unit
  val invalid_argument_of_rx_mutable_function: Pos.t -> unit
  val return_void_to_rx_mismatch: pos1_has_attribute:bool -> Pos.t -> Pos.t -> unit
  val returns_void_to_rx_function_as_non_expression_statement: Pos.t -> Pos.t -> unit
  val non_awaited_awaitable_in_rx: Pos.t -> unit
  val shapes_key_exists_always_true: Pos.t -> string -> Pos.t -> unit
  val shapes_key_exists_always_false: Pos.t -> string -> Pos.t -> [< `Undefined | `Unset] -> unit
  val shapes_idx_with_non_existent_field: Pos.t -> string -> Pos.t -> [< `Undefined | `Unset] -> unit
  val ambiguous_object_access: Pos.t -> string -> Pos.t -> string -> Pos.t -> string -> string -> unit
  val forward_compatibility_not_current: Pos.t -> ForwardCompatibilityLevel.t -> unit
  val forward_compatibility_below_minimum: Pos.t -> ForwardCompatibilityLevel.t -> unit
  val invalid_argument_type_for_condition_in_rx:
    is_receiver: bool -> Pos.t -> Pos.t -> Pos.t -> string -> string -> unit
  val invalid_function_type_for_condition_in_rx:
    Pos.t -> Pos.t -> Pos.t -> string -> string -> unit
  val callsite_reactivity_mismatch: Pos.t -> Pos.t -> string -> string -> unit
  val rx_parameter_condition_mismatch: string -> Pos.t -> Pos.t -> unit
  val maybe_mutable_attribute_on_function: Pos.t -> unit
  val conflicting_mutable_and_maybe_mutable_attributes: Pos.t -> unit
  val maybe_mutable_methods_must_be_reactive: Pos.t -> string -> unit
  val reassign_maybe_mutable_var: Pos.t -> unit
  val immutable_argument_mismatch : Pos.t -> Pos.t -> unit
  val maybe_mutable_argument_mismatch : Pos.t -> Pos.t -> unit
  val immutable_call_on_mutable: Pos.t -> Pos.t -> unit
  val invalid_call_on_maybe_mutable: fun_is_mutable:bool -> Pos.t -> Pos.t -> unit
  val mutability_mismatch: is_receiver: bool -> Pos.t -> string -> Pos.t -> string -> unit
  val invalid_traversable_in_rx: Pos.t -> unit
  val reference_in_rx: Pos.t -> unit
  val reassign_mutable_this: Pos.t -> unit
  val mutable_expression_as_multiple_mutable_arguments: Pos.t -> string -> Pos.t -> string -> unit
  val invalid_unset_target_rx: Pos.t -> unit
  val declare_statement_in_hack : Pos.t -> unit
  val misplaced_rx_of_scope: Pos.t -> unit
  val rx_of_scope_and_explicit_rx: Pos.t -> unit
  val lateinit_with_default: Pos.t -> unit
end
