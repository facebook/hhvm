(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type expand_typedef =
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_reason.t ->
  string ->
  Typing_defs.locl_ty list ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

val expand_typedef_ref : expand_typedef ref

val expand_typedef : expand_typedef

type sub_type =
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Reasons_callback.t option ->
  Typing_env_types.env * Typing_error.t option

val sub_type_ref : sub_type ref

val sub_type : sub_type

type sub_type_i =
  Typing_env_types.env ->
  ?is_coeffect:bool ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Typing_error.Reasons_callback.t option ->
  Typing_env_types.env * Typing_error.t option

val sub_type_i_ref : sub_type_i ref

val sub_type_i : sub_type_i

type sub_type_with_dynamic_as_bottom =
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Reasons_callback.t option ->
  Typing_env_types.env * Typing_error.t option

val sub_type_with_dynamic_as_bottom_ref : sub_type_with_dynamic_as_bottom ref

val sub_type_with_dynamic_as_bottom : sub_type_with_dynamic_as_bottom

type is_sub_type_type =
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_sub_type_ref : is_sub_type_type ref

val is_sub_type :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_sub_type_for_union_ref :
  (Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool)
  ref

val is_sub_type_for_union_i_ref :
  (Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  bool)
  ref

val is_sub_type_for_union :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_sub_type_for_union_i :
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  bool

val is_sub_type_ignore_generic_params_ref : is_sub_type_type ref

val is_sub_type_ignore_generic_params :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val can_sub_type_ref : is_sub_type_type ref

val can_sub_type :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val unwrap_class_type :
  Typing_defs.decl_phase Typing_defs.ty ->
  Typing_defs.decl_phase Typing_defs.Reason.t_
  * Typing_defs.pos_id
  * Typing_defs.decl_ty list

val try_unwrap_class_type :
  Typing_defs.decl_phase Typing_defs.ty ->
  (Typing_defs.decl_phase Typing_defs.Reason.t_
  * Typing_defs.pos_id
  * Typing_defs.decl_ty list)
  option

val class_is_final_and_invariant : Decl_provider.Class.t -> bool

type localize_no_subst =
  Typing_env_types.env ->
  ignore_errors:bool ->
  Typing_defs.decl_ty ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

val localize_no_subst_ref : localize_no_subst ref

val localize_no_subst :
  Typing_env_types.env ->
  ignore_errors:bool ->
  Typing_defs.decl_ty ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

type localize =
  ety_env:Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

val localize_ref : localize ref

val localize :
  Typing_env_types.env ->
  ety_env:Typing_defs.expand_env ->
  Typing_defs.decl_ty ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

val is_class_i : Typing_defs.internal_type -> bool

val is_class : Typing_defs.locl_ty -> bool

val is_mixed_i : Typing_env_types.env -> Typing_defs.internal_type -> bool

val is_mixed : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_nothing_i : Typing_env_types.env -> Typing_defs.internal_type -> bool

val is_nothing : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val run_on_intersection :
  'env ->
  f:('env -> Typing_defs.locl_ty -> 'env * 'a) ->
  Typing_defs.locl_ty list ->
  'env * 'a list

val run_on_intersection_with_ty_err :
  'env ->
  f:('env -> Typing_defs.locl_ty -> ('env * Typing_error.t option) * 'a) ->
  Typing_defs.locl_ty list ->
  ('env * Typing_error.t option) * 'a list

val run_on_intersection_res :
  'a ->
  f:('a -> Typing_defs.locl_ty -> 'a * 'b * 'c) ->
  Typing_defs.locl_ty list ->
  'a * 'b list * 'c list

val run_on_intersection_key_value_res :
  'a ->
  f:('a -> Typing_defs.locl_ty -> 'a * 'b * 'c * 'd) ->
  Typing_defs.locl_ty list ->
  'a * 'b list * 'c list * 'd list

val run_on_intersection_array_key_value_res :
  'a ->
  f:('a -> Typing_defs.locl_ty -> 'a * 'b * 'c * 'd * 'e) ->
  Typing_defs.locl_ty list ->
  'a * 'b list * 'c list * 'd list * 'e list

val is_dynamic : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_any : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tunion : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tintersection : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tyvar : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_opt_tyvar : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tyvar_error : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val get_base_type :
  ?expand_supportdyn:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty

val get_printable_shape_field_name : Typing_defs.tshape_field_name -> string

val shape_field_name_with_ty_err :
  Typing_env_types.env ->
  ('a, 'b) Aast.expr ->
  Ast_defs.shape_field_name option * Typing_error.t option

val simplify_constraint_type :
  Typing_env_types.env ->
  Typing_defs.constraint_type ->
  Typing_env_types.env * Typing_defs.internal_type

type add_constraint =
  Typing_env_types.env ->
  Ast_defs.constraint_kind ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Reasons_callback.t option ->
  Typing_env_types.env

val add_constraint_ref : add_constraint ref

val add_constraint : add_constraint

type expand_typeconst =
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  Typing_defs.locl_ty ->
  Typing_defs.pos_id ->
  allow_abstract_tconst:bool ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

val expand_typeconst_ref : expand_typeconst ref

val expand_typeconst :
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  Typing_defs.locl_ty ->
  Typing_defs.pos_id ->
  allow_abstract_tconst:bool ->
  (Typing_env_types.env * Typing_error.t option) * Typing_defs.locl_ty

type union =
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val union_ref : union ref

val union :
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

type make_union =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_reason.t option ->
  Typing_reason.t option ->
  Typing_env_types.env * Typing_defs.locl_ty

val make_union_ref : make_union ref

val make_union :
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_reason.t option ->
  Typing_reason.t option ->
  Typing_env_types.env * Typing_defs.locl_ty

type union_i =
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  Typing_defs.internal_type ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.internal_type

val union_i_ref : union_i ref

val union_i :
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  Typing_defs.internal_type ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.internal_type

type union_list =
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

val union_list_ref : union_list ref

val union_list :
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

type fold_union =
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

val fold_union_ref : fold_union ref

type simplify_unions =
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  ?on_tyvar:
    (Typing_env_types.env ->
    Typing_reason.t ->
    Ident.t ->
    Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val simplify_unions_ref : simplify_unions ref

type intersect_list =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

val intersect_list_ref : intersect_list ref

val intersect_list : intersect_list

val contains_tvar_decl : Typing_defs.decl_ty -> bool

val contains_generic_decl : Typing_defs.decl_ty -> Pos_or_decl.t option

(** [wrap_union_inter_ty_in_var env r ty] wraps [ty] in a type variable
  if [ty] is a union, intersection or option containing an unsolved type variable.
  Wrapping in a type variable means a new type variable is created and mapped to [ty].
  This is a way to mark [ty] as needing simplification when the unsolved type variable
  later gets solved. *)
val wrap_union_inter_ty_in_var :
  Typing_env_types.env ->
  Typing_defs.Reason.t ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val get_concrete_supertypes :
  ?expand_supportdyn:bool ->
  ?include_case_types:bool ->
  abstract_enum:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_set.elt list

val simplify_unions :
  Typing_env_types.env ->
  ?approx_cancel_neg:bool ->
  ?on_tyvar:
    (Typing_env_types.env ->
    Typing_reason.t ->
    Ident.t ->
    Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

type approx =
  | ApproxUp
  | ApproxDown
[@@deriving eq]

type negate_type =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  approx:approx ->
  Typing_env_types.env * Typing_defs.locl_ty

val negate_type_ref : negate_type ref

val negate_type :
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  approx:approx ->
  Typing_env_types.env * Typing_defs.locl_ty

val simplify_intersections :
  Typing_env_types.env ->
  ?on_tyvar:
    (Typing_env_types.env ->
    Typing_reason.t ->
    int ->
    Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

type simplify_intersections =
  Typing_env_types.env ->
  ?on_tyvar:
    (Typing_env_types.env ->
    Typing_reason.t ->
    int ->
    Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val simplify_intersections_ref : simplify_intersections ref

val is_type_disjoint :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_type_disjoint_ref :
  (Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool)
  ref

val collect_enum_class_upper_bounds :
  Typing_env_types.env ->
  string ->
  Typing_env_types.env * Typing_defs.locl_ty option

val default_fun_param : ?pos:Pos_or_decl.t -> 'a -> 'a Typing_defs.fun_param

val mk_tany :
  Typing_env_types.env -> Pos.t -> Typing_reason.locl_phase Typing_defs.ty

val make_locl_subst_for_class_tparams :
  Decl_provider.Class.t ->
  Typing_defs.locl_ty list ->
  Typing_defs.locl_ty SMap.t

val is_sub_class_refl : Typing_env_types.env -> string -> string -> bool

val class_has_no_params : Typing_env_types.env -> string -> bool

val has_ancestor_including_req_refl :
  Typing_env_types.env -> string -> string -> bool

val has_ancestor_including_req :
  Typing_env_types.env -> Decl_provider.Class.t -> string -> bool

(* If input is dynamic | t or t | dynamic then return Some t.
 * Otherwise return None.
 *)
val try_strip_dynamic :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty option

val try_strip_dynamic_from_union :
  Typing_env_types.env ->
  Typing_defs.locl_ty list ->
  (Typing_defs.locl_ty * Typing_defs.locl_ty list) option

(* If input is dynamic | t or t | dynamic then return t,
 * otherwise return type unchanged. *)
val strip_dynamic :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty

(* Wrap supportdyn<_> around a type. Push through intersections
 * and unions, and leave type alone if it is a subtype of dynamic.
 *)
val make_supportdyn :
  Typing_reason.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val simple_make_supportdyn :
  Typing_reason.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val make_supportdyn_decl_type :
  Pos_or_decl.t ->
  Typing_defs.Reason.decl_t ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty

(* Wrap ~ around a type, unless it is already a dynamic or a like type *)
val make_like :
  ?reason:Typing_reason.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty

(* Wrap ~ around a type if it is enforced. This is used for returns, property
 * assignment, and default expression typing
 *)
val make_like_if_enforced :
  Typing_env_types.env ->
  Typing_defs.locl_possibly_enforced_ty ->
  Typing_defs.locl_possibly_enforced_ty

val is_capability : Typing_defs.locl_ty -> bool

val is_capability_i : Typing_defs.internal_type -> bool

val supports_dynamic :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_error.Reasons_callback.t option ->
  Typing_env_types.env * Typing_error.t option

(* Return true if type definitely is a subtype of supportdyn<mixed> *)
val is_supportdyn : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val strip_supportdyn :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  bool * Typing_env_types.env * Typing_defs.locl_ty

val no_upper_bound :
  include_sd_mixed:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * bool

val recompose_like_type :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val make_simplify_typed_expr :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  (Typing_defs.locl_ty, Tast.saved_env) Aast_defs.expr_ ->
  Typing_env_types.env * Tast.expr

val partition_union :
  f:('a Typing_defs.ty -> bool) ->
  'a Typing_defs.ty list ->
  'a Typing_defs.ty list * 'a Typing_defs.ty list
