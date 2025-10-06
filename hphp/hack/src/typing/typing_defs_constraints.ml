(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs_core

type destructure_kind =
  | ListDestructure
  | SplatUnpack
[@@deriving eq, ord, show]

(** see .mli *)
type destructure = {
  d_required: locl_ty list;
  d_optional: locl_ty list;
  d_variadic: locl_ty option;
  d_kind: destructure_kind;
}
[@@deriving show]

type has_member_method = {
  hmm_explicit_targs: Nast.targ list; [@opaque]
  hmm_env_capability: locl_ty;
}
[@@deriving show]

type has_member = {
  hm_name: Nast.sid;
  hm_type: locl_ty;
  hm_class_id: Nast.class_id_; [@opaque]
  hm_method: has_member_method option; [@opaque]
}
[@@deriving show]

type can_index = {
  ci_key: locl_ty;
  ci_val: locl_ty;
  ci_index_expr: Nast.expr;
  ci_lhs_of_null_coalesce: bool;
  ci_expr_pos: Pos.t;
  ci_array_pos: Pos.t;
  ci_index_pos: Pos.t;
}
[@@deriving show]

(* `$source[key] = write` update the array to type val *)
type can_index_assign = {
  cia_key: locl_ty;
  cia_write: locl_ty;
  cia_val: locl_ty;
  cia_index_expr: Nast.expr;
  cia_expr_pos: Pos.t;
  cia_array_pos: Pos.t;
  cia_index_pos: Pos.t;
  cia_write_pos: Pos.t;
}
[@@deriving show]

type can_traverse = {
  ct_key: locl_ty option;
  ct_val: locl_ty;
  ct_is_await: bool;
  ct_reason: Reason.t;
}
[@@deriving show]

type has_type_member = {
  htm_id: string;
  htm_lower: locl_ty;
  htm_upper: locl_ty;
}
[@@deriving show]

(** see .mli *)
type constraint_type_ =
  | Thas_member of has_member
  | Thas_type_member of has_type_member
  | Thas_const of {
      name: string;
      ty: locl_ty;
    }
  | Tcan_index of can_index
  | Tcan_index_assign of can_index_assign
  | Tcan_traverse of can_traverse
  | Tdestructure of destructure
  | Ttype_switch of {
      predicate: type_predicate;
      ty_true: locl_ty;
      ty_false: locl_ty;
    }

and constraint_type = Reason.t * constraint_type_ [@@deriving show]

type internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type
[@@deriving show]

let mk_constraint_type p = p

let deref_constraint_type p = p

let get_reason_i : internal_type -> Reason.t = function
  | LoclType lty -> get_reason lty
  | ConstraintType (r, _) -> r

let has_member_compare ~normalize_lists hm1 hm2 =
  let ty_compare = ty_compare ~normalize_lists in
  let {
    hm_name = (_, m1);
    hm_type = ty1;
    hm_class_id = cid1;
    hm_method = method1;
  } =
    hm1
  in
  let {
    hm_name = (_, m2);
    hm_type = ty2;
    hm_class_id = cid2;
    hm_method = method2;
  } =
    hm2
  in
  let method_compare method1 method2 =
    match (method1, method2) with
    | ( { hmm_explicit_targs = targs1; hmm_env_capability = cap1 },
        { hmm_explicit_targs = targs2; hmm_env_capability = cap2 } ) ->
      let targ_compare (_, (_, hint1)) (_, (_, hint2)) =
        Aast_defs.compare_hint_ hint1 hint2
      in
      chain_compare (List.compare targ_compare targs1 targs2) (fun _ ->
          ty_compare cap1 cap2)
  in
  chain_compare (String.compare m1 m2) (fun _ ->
      chain_compare (ty_compare ty1 ty2) (fun _ ->
          chain_compare (class_id_compare cid1 cid2) (fun _ ->
              Option.compare method_compare method1 method2)))

let can_index_compare ~normalize_lists ci1 ci2 =
  (* not comparing the ast because it is decided by expr_pos *)
  chain_compare (ty_compare ~normalize_lists ci1.ci_key ci2.ci_key) (fun _ ->
      chain_compare
        (ty_compare ~normalize_lists ci1.ci_val ci2.ci_val)
        (fun _ ->
          chain_compare
            (Bool.compare
               ci1.ci_lhs_of_null_coalesce
               ci2.ci_lhs_of_null_coalesce)
            (fun _ ->
              chain_compare
                (Aast.compare_pos ci1.ci_expr_pos ci2.ci_expr_pos)
                (fun _ ->
                  chain_compare
                    (Aast.compare_pos ci1.ci_array_pos ci2.ci_array_pos)
                    (fun _ ->
                      Aast.compare_pos ci1.ci_index_pos ci2.ci_index_pos)))))

let can_index_assign_compare ~normalize_lists cia1 cia2 =
  chain_compare
    (ty_compare ~normalize_lists cia1.cia_key cia2.cia_key)
    (fun _ ->
      chain_compare
        (ty_compare ~normalize_lists cia1.cia_write cia2.cia_write)
        (fun _ ->
          chain_compare
            (ty_compare ~normalize_lists cia1.cia_val cia2.cia_val)
            (fun _ ->
              chain_compare
                (Aast.compare_pos cia1.cia_expr_pos cia2.cia_expr_pos)
                (fun _ ->
                  chain_compare
                    (Aast.compare_pos cia1.cia_array_pos cia2.cia_array_pos)
                    (fun _ ->
                      chain_compare
                        (Aast.compare_pos cia1.cia_index_pos cia2.cia_index_pos)
                        (fun _ ->
                          Aast.compare_pos cia1.cia_write_pos cia2.cia_write_pos))))))

let can_traverse_compare ~normalize_lists ct1 ct2 =
  chain_compare
    (Option.compare (ty_compare ~normalize_lists) ct1.ct_key ct2.ct_key)
    (fun _ ->
      chain_compare
        (ty_compare ~normalize_lists ct1.ct_val ct2.ct_val)
        (fun _ -> Bool.compare ct1.ct_is_await ct2.ct_is_await))

let destructure_compare ~normalize_lists d1 d2 =
  let {
    d_required = tyl1;
    d_optional = tyl_opt1;
    d_variadic = ty_opt1;
    d_kind = e1;
  } =
    d1
  in
  let {
    d_required = tyl2;
    d_optional = tyl_opt2;
    d_variadic = ty_opt2;
    d_kind = e2;
  } =
    d2
  in
  chain_compare (tyl_compare ~normalize_lists ~sort:false tyl1 tyl2) (fun _ ->
      chain_compare
        (tyl_compare ~normalize_lists ~sort:false tyl_opt1 tyl_opt2)
        (fun _ ->
          chain_compare (Option.compare ty_compare ty_opt1 ty_opt2) (fun _ ->
              compare_destructure_kind e1 e2)))

let constraint_ty_con_ordinal cty =
  match cty with
  | Thas_member _ -> 0
  | Tdestructure _ -> 1
  | Tcan_index _ -> 2
  | Tcan_index_assign _ -> 3
  | Tcan_traverse _ -> 4
  | Thas_type_member _ -> 5
  | Ttype_switch _ -> 6
  | Thas_const _ -> 7

let constraint_ty_compare ?(normalize_lists = false) ty1 ty2 =
  let (_, ty1) = deref_constraint_type ty1 in
  let (_, ty2) = deref_constraint_type ty2 in
  match (ty1, ty2) with
  | (Thas_member hm1, Thas_member hm2) ->
    has_member_compare ~normalize_lists hm1 hm2
  | (Thas_type_member htm1, Thas_type_member htm2) ->
    let { htm_id = id1; htm_lower = lower1; htm_upper = upper1 } = htm1
    and { htm_id = id2; htm_lower = lower2; htm_upper = upper2 } = htm2 in
    chain_compare (String.compare id1 id2) (fun _ ->
        chain_compare (ty_compare lower1 lower2) (fun _ ->
            ty_compare upper1 upper2))
  | (Tcan_index ci1, Tcan_index ci2) ->
    can_index_compare ~normalize_lists ci1 ci2
  | (Tcan_index_assign cia1, Tcan_index_assign cia2) ->
    can_index_assign_compare ~normalize_lists cia1 cia2
  | (Tcan_traverse ct1, Tcan_traverse ct2) ->
    can_traverse_compare ~normalize_lists ct1 ct2
  | (Tdestructure d1, Tdestructure d2) ->
    destructure_compare ~normalize_lists d1 d2
  | ( Ttype_switch
        { predicate = predicate1; ty_true = ty_true1; ty_false = ty_false1 },
      Ttype_switch
        { predicate = predicate2; ty_true = ty_true2; ty_false = ty_false2 } )
    ->
    chain_compare (compare_type_predicate predicate1 predicate2) (fun _ ->
        chain_compare (ty_compare ~normalize_lists ty_true1 ty_true2) (fun _ ->
            ty_compare ~normalize_lists ty_false1 ty_false2))
  | ( Thas_const { name = name1; ty = ty1 },
      Thas_const { name = name2; ty = ty2 } ) ->
    chain_compare (String.compare name1 name2) (fun _ ->
        ty_compare ~normalize_lists ty1 ty2)
  | ( _,
      ( Thas_member _ | Tcan_index _ | Tcan_index_assign _ | Tcan_traverse _
      | Tdestructure _ | Thas_type_member _ | Ttype_switch _ | Thas_const _ ) )
    ->
    constraint_ty_con_ordinal ty2 - constraint_ty_con_ordinal ty1

let constraint_ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal (constraint_ty_compare ~normalize_lists ty1 ty2) 0

(** Ignore position and reason info. *)
let equal_internal_type ty1 ty2 =
  match (ty1, ty2) with
  | (LoclType ty1, LoclType ty2) -> ty_equal ~normalize_lists:true ty1 ty2
  | (ConstraintType ty1, ConstraintType ty2) ->
    constraint_ty_equal ~normalize_lists:true ty1 ty2
  | (_, (LoclType _ | ConstraintType _)) -> false

let get_var_i t =
  match t with
  | LoclType t -> get_var t
  | ConstraintType _ -> None

let is_tyvar_i t = Option.is_some (get_var_i t)

let is_has_member t =
  match deref_constraint_type t with
  | (_, Thas_member _) -> true
  | _ -> false

let is_locl_type = function
  | LoclType _ -> true
  | _ -> false

let reason = function
  | LoclType t -> get_reason t
  | ConstraintType t -> fst (deref_constraint_type t)

let is_constraint_type = function
  | ConstraintType _ -> true
  | LoclType _ -> false

module InternalType = struct
  let get_var t =
    match t with
    | LoclType t -> get_var t
    | ConstraintType _ -> None

  let is_var_v t ~v =
    match t with
    | LoclType t -> is_var_v t v
    | ConstraintType _ -> false

  let is_not_var_v t ~v = not @@ is_var_v t ~v
end
