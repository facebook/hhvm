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
  Typing_env_types.env * Typing_defs.locl_ty

val expand_typedef_ref : expand_typedef ref

val expand_typedef :
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_reason.t ->
  string ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

type sub_type =
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

val sub_type_ref : sub_type ref

val sub_type :
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

type sub_type_res =
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

val sub_type_res_ref : sub_type_res ref

val sub_type_res :
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

type sub_type_i =
  Typing_env_types.env ->
  ?is_coeffect:bool ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

val sub_type_i_ref : sub_type_i ref

val sub_type_i :
  ?is_coeffect:bool ->
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

type sub_type_i_res =
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

val sub_type_i_res_ref : sub_type_i_res ref

val sub_type_i_res :
  Typing_env_types.env ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

type sub_type_with_dynamic_as_bottom =
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

val sub_type_with_dynamic_as_bottom_ref : sub_type_with_dynamic_as_bottom ref

val sub_type_with_dynamic_as_bottom :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

type sub_type_with_dynamic_as_bottom_res =
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

val sub_type_with_dynamic_as_bottom_res_ref :
  sub_type_with_dynamic_as_bottom_res ref

val sub_type_with_dynamic_as_bottom_res :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

type is_sub_type_type =
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_sub_type_ref : is_sub_type_type ref

val is_sub_type :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val is_sub_type_for_coercion_ref : is_sub_type_type ref

val is_sub_type_for_union_ref :
  (Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  bool)
  ref

val is_sub_type_for_union_i_ref :
  (Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  bool)
  ref

val is_sub_type_for_union :
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  bool

val is_sub_type_for_union_i :
  Typing_env_types.env ->
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_defs.internal_type ->
  Typing_defs.internal_type ->
  bool

val is_sub_type_ignore_generic_params_ref : is_sub_type_type ref

val is_sub_type_ignore_generic_params :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> bool

val string_of_visibility : Typing_defs.ce_visibility -> string

val unwrap_class_type :
  Typing_defs.decl_phase Typing_defs.ty ->
  Typing_defs.decl_phase Typing_defs.Reason.t_
  * Typing_defs.pos_id
  * Typing_defs.decl_ty list

val try_unwrap_class_type :
  Typing_defs.decl_phase Typing_defs.ty ->
  ( Typing_defs.decl_phase Typing_defs.Reason.t_
  * Typing_defs.pos_id
  * Typing_defs.decl_ty list )
  option

val class_is_final_and_not_contravariant : Decl_provider.Class.t -> bool

module HasTany : sig
  val check : Typing_defs.locl_ty -> bool

  val check_why : Typing_defs.locl_ty -> Typing_reason.t option
end

type localize_no_subst =
  Typing_env_types.env ->
  ignore_errors:bool ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val localize_no_subst_ref : localize_no_subst ref

val localize_no_subst :
  Typing_env_types.env ->
  ignore_errors:bool ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

type localize =
  ety_env:Typing_defs.expand_env ->
  Typing_env_types.env ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val localize_ref : localize ref

val localize :
  Typing_env_types.env ->
  ety_env:Typing_defs.expand_env ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val is_mixed_i : Typing_env_types.env -> Typing_defs.internal_type -> bool

val is_mixed : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_nothing_i : Typing_env_types.env -> Typing_defs.internal_type -> bool

val is_nothing : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val run_on_intersection :
  'env ->
  f:('env -> Typing_defs.locl_ty -> 'env * 'a) ->
  Typing_defs.locl_ty list ->
  'env * 'a list

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

val is_dynamic : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_any : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tunion : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val is_tintersection : Typing_env_types.env -> Typing_defs.locl_ty -> bool

val get_base_type :
  Typing_env_types.env -> Typing_defs.locl_ty -> Typing_defs.locl_ty

val get_printable_shape_field_name : Typing_defs.tshape_field_name -> string

val shape_field_name_ :
  Ast_defs.id option Lazy.t ->
  Ast_defs.pos * ('a, 'b, 'c, 'd) Aast.expr_ ->
  ( Ast_defs.shape_field_name,
    [> `Expected_class | `Invalid_shape_field_name ] )
  result

val shape_field_name :
  Typing_env_types.env -> Nast.expr -> Ast_defs.shape_field_name option

val simplify_constraint_type :
  Typing_env_types.env ->
  Typing_defs.constraint_type ->
  Typing_env_types.env * Typing_defs.internal_type

type add_constraint =
  Typing_env_types.env ->
  Ast_defs.constraint_kind ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

val add_constraint_ref : add_constraint ref

val add_constraint :
  Typing_env_types.env ->
  Ast_defs.constraint_kind ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Errors.error_from_reasons_callback ->
  Typing_env_types.env

type expand_typeconst =
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  Typing_defs.locl_ty ->
  Typing_defs.pos_id ->
  root_pos:Pos_or_decl.t ->
  allow_abstract_tconst:bool ->
  Typing_env_types.env * Typing_defs.locl_ty

val expand_typeconst_ref : expand_typeconst ref

val expand_typeconst :
  Typing_defs.expand_env ->
  Typing_env_types.env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  Typing_defs.locl_ty ->
  Typing_defs.pos_id ->
  root_pos:Pos_or_decl.t ->
  allow_abstract_tconst:bool ->
  Typing_env_types.env * Typing_defs.locl_ty

type union =
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val union_ref : union ref

val union :
  Typing_env_types.env ->
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
  Typing_reason.t ->
  Typing_defs.internal_type ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.internal_type

val union_i_ref : union_i ref

val union_i :
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.internal_type ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.internal_type

type union_list =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

val union_list_ref : union_list ref

val union_list :
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

type fold_union =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty

val fold_union_ref : fold_union ref

type simplify_unions =
  Typing_env_types.env ->
  ?on_tyvar:
    (Typing_env_types.env ->
    Typing_reason.t ->
    Ident.t ->
    Typing_env_types.env * Typing_defs.locl_ty) ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val simplify_unions_ref : simplify_unions ref

val contains_tvar_decl : Typing_defs.decl_ty -> bool

val wrap_union_inter_ty_in_var :
  Typing_env_types.env ->
  Typing_defs.Reason.t ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val get_concrete_supertypes :
  abstract_enum:bool ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_set.elt list

val simplify_unions :
  Typing_env_types.env ->
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

type non =
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  approx:approx ->
  Typing_env_types.env * Typing_defs.locl_ty

val non_ref : non ref

val non :
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

val collect_enum_class_upper_bounds : Typing_env_types.env -> string -> SSet.t

val default_fun_param : ?pos:Pos_or_decl.t -> 'a -> 'a Typing_defs.fun_param

val tany : Typing_env_types.env -> Typing_defs.locl_phase Typing_defs.ty_

val mk_tany :
  Typing_env_types.env -> Pos.t -> Typing_reason.locl_phase Typing_defs.ty

val mk_tany_ :
  Typing_env_types.env -> Pos_or_decl.t -> Typing_defs.locl_phase Typing_defs.ty

val terr : Typing_env_types.env -> 'a Typing_defs.Reason.t_ -> 'a Typing_defs.ty

val make_locl_subst_for_class_tparams :
  Decl_provider.Class.t ->
  Typing_defs.locl_ty list ->
  Typing_defs.locl_ty SMap.t
