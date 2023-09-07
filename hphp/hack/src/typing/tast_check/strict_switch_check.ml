(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

module Value = struct
  type t =
    | Other  (** requires default case *)
    | Null
  [@@deriving ord, sexp, hash]

  let universe = [Other; Null]

  let finite_or_dynamic = function
    | Other -> false
    | Null -> true

  let to_json = function
    | Other -> Hh_json.string_ "other"
    | Null -> Hh_json.string_ "null"
end

module ValueSet = struct
  include Set.Make (Value)

  let universe = of_list Value.universe

  let intersection_list value_sets =
    List.fold_right ~init:universe ~f:inter value_sets

  let non_symbolic_diff = diff

  let symbolic_diff value_set1 value_set2 =
    let non_sym_diff = non_symbolic_diff value_set1 value_set2 in
    (* so { Other } - { Other }  is safely over-approximated to { Other } *)
    if mem value_set1 Value.Other then
      add non_sym_diff Value.Other
    else
      non_sym_diff
end

let prim_to_values = function
  | Tnoreturn -> ValueSet.empty
  | Tnull
  | Tvoid ->
    ValueSet.singleton Value.Null
  | Tint
  | Tbool
  | Tfloat
  | Tstring
  | Tresource
  | Tnum
  | Tarraykey ->
    ValueSet.singleton Value.Other

(* Symbolically evaluate the values corresponding to a given type; the result is
   essentially in disjunctive normal form, so that cases can be partitioned
   according to each element (transparent enums need special care, see later). *)
let rec symbolic_dnf_values env ty : ValueSet.t =
  match Typing_defs.get_node ty with
  | Tunion tyl ->
    tyl |> List.map ~f:(symbolic_dnf_values env) |> ValueSet.union_list
  | Tintersection tyl ->
    tyl |> List.map ~f:(symbolic_dnf_values env) |> ValueSet.intersection_list
  | Tprim prim -> prim_to_values prim
  | Tnonnull -> ValueSet.(symbolic_diff universe (singleton Value.Null))
  | Toption ty -> begin
    match Typing_defs.get_node ty with
    (* this won't work when dealing with types that are semantically the same as
       nonnull, but are syntactically different. For instance if a generic or type
       constant ends up with a equality bound of nonnull (can happen via use of
       where constraints). *)
    | Tnonnull -> ValueSet.universe
    | _ -> ValueSet.(add (symbolic_dnf_values env ty) Value.Null)
  end
  | Tneg (Neg_prim prim) ->
    ValueSet.(symbolic_diff universe (prim_to_values prim))
  | Tneg (Neg_class _) (* a safe over-approximation *)
  | Tany _ ->
    ValueSet.universe
  | Tnewtype (_, _, _)
  | Tdynamic
  | Ttuple _
  | Tshape _
  | Tvec_or_dict _
  | Tclass _
  | Tvar _
  | Tfun _
  | Tgeneric _
  | Tdependent _
  | Taccess _ ->
    ValueSet.singleton Value.Other
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

let key_present_data_consed table ~key ~data =
  Hashtbl.find_and_call
    table
    key
    ~if_found:(fun _ ->
      Hashtbl.add_multi table ~key ~data;
      true)
    ~if_not_found:(fun _ -> false)

type case = (Tast.ty, Tast.saved_env) Aast.expr

(* design choice: could check for cases of type null rather than the null
   literal expression. Ultimately we think case branches should only allow
   literals or class constants, and so disregard non-literal expressions *)
let case_to_value ((_, _, expr) : case) =
  match expr with
  | Null -> Value.Null
  | _ -> Value.Other

(* Partition cases based on the kind of literal expression (and later, type) *)
let partition_cases (cases : (case * _) list) values =
  let partitions : (Value.t, case list) Hashtbl.t =
    let tbl = Hashtbl.create (module Value) in
    ValueSet.iter values ~f:(fun key -> Hashtbl.add_exn tbl ~key ~data:[]);
    tbl
  in
  let unused_cases =
    List.fold_left cases ~init:[] ~f:(fun unused (case, _) ->
        let key = case_to_value case in
        if key_present_data_consed partitions ~key ~data:case then
          unused
        else
          case :: unused)
  in
  (partitions, unused_cases)

module EnumErr = Typing_error.Primary.Enum

let register_err env err =
  Typing_error_utils.add_typing_error ~env @@ Typing_error.enum err

let get_missing_cases env partitions =
  Hashtbl.fold partitions ~init:[] ~f:(fun ~key ~data missing_cases ->
      match key with
      | Value.Null -> begin
        match data with
        | [_] -> missing_cases
        | [] -> "null" :: missing_cases
        | (_, redundant_pos, _) :: (_ :: _ as tl) ->
          let (_, first_pos, _) = List.last_exn tl in
          register_err env
          @@ EnumErr.Enum_switch_redundant
               { const_name = "null"; first_pos; pos = redundant_pos };
          missing_cases
      end
      | Value.Other -> missing_cases)

let is_supported_literal : (Tast.ty, Tast.saved_env) Aast.expr_ -> bool =
  function
  | Null
  | Int _ ->
    true
  | _ -> false

let error_unused_cases env expected unused_cases =
  let expected = lazy (Typing_print.full_strip_ns env expected) in
  List.iter unused_cases ~f:(fun (ty, pos, expr) ->
      register_err env
      @@
      if is_supported_literal expr then
        EnumErr.Enum_switch_wrong_class
          {
            pos;
            kind = "";
            expected = Lazy.force expected;
            actual = Typing_print.full_strip_ns env ty;
          }
      else
        EnumErr.Enum_switch_not_const pos)

type default = (Tast.ty, Tast.saved_env) Aast.default_case

let check_default
    env pos (opt_default_case : default option) needs_default missing =
  match (opt_default_case, needs_default) with
  | (None, false)
  | (Some _, true) ->
    ()
  | (Some (default_pos, _), false) ->
    register_err env
    @@ EnumErr.Enum_switch_redundant_default
         {
           pos;
           kind = "default";
           decl_pos = Pos_or_decl.of_raw_pos default_pos;
         }
  | (None, true) ->
    register_err env
    @@ EnumErr.Enum_switch_nonexhaustive
         { pos; kind = None; decl_pos = Pos_or_decl.of_raw_pos pos; missing }

let add_default_if_needed values missing_cases =
  (* write `exists ~f:(fun x -> not @@ finite_or_dynamic x)` instead of
     `forall ~f:finite` to ensure set is non-empty *)
  if ValueSet.exists ~f:(fun x -> not @@ Value.finite_or_dynamic x) values then
    "default" :: missing_cases
  else
    missing_cases

(* The algorithm below works as follows:
   1. Partition the case list into buckets for each (non-dynamic) element of the computed set.
     a. Any leftover values are considered to be redundant/ill-typed.
   2. For each finite value (bool, null, and enums) check that all cases have been covered.
     a. Note: the code does not check if each infinite (non-dynamic) values
        has at least one case.
   3. If any of the below are true, then we need a default case.
     a. The value set is the singleton { Dynamic }.
     b. The value set contains an neither-finite-nor-dynamic value.
     c. There are missing cases for finite values.
   4. Otherwise a default is redundant. *)
let check_cases_against_values env pos expected values cases opt_default_case =
  let (partitions, unused_cases) = partition_cases cases values in
  let typing_env = Tast_env.tast_env_as_typing_env env in
  error_unused_cases typing_env expected unused_cases;
  let missing_cases =
    get_missing_cases typing_env partitions |> add_default_if_needed values
  in
  let needs_default = not (List.is_empty missing_cases) in
  check_default typing_env pos opt_default_case needs_default missing_cases

let add_fields json ~fields =
  match json with
  | Hh_json.JSON_Object old -> Hh_json.JSON_Object (old @ fields)
  | _ -> Hh_json.JSON_Object (("warning_expected_object", json) :: fields)

let check_exhaustiveness env pos ty cases opt_default =
  let values = symbolic_dnf_values env ty in
  check_cases_against_values env pos ty values cases opt_default;
  let tcopt = env |> Tast_env.get_decl_env |> Decl_env.tcopt in
  if TypecheckerOptions.tco_log_exhaustivity_check tcopt then
    let fields =
      [
        ("values", values |> ValueSet.to_list |> Hh_json.array_ Value.to_json);
        ("switch_pos", Pos.(pos |> to_absolute |> json));
      ]
    in
    ty
    |> Tast_env.ty_to_json env ~show_like_ty:true
    |> add_fields ~fields
    |> Hh_json.json_to_string
    |> Hh_logger.log "[hh_tco_enable_strict_switch] %s"

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env x =
      match snd x with
      | Switch ((scrutinee_ty, scrutinee_pos, _), cases, opt_default_case) ->
        let (_, scrutinee_ty) =
          Typing_union.simplify_unions
            (Tast_env.tast_env_as_typing_env env)
            scrutinee_ty
        in
        check_exhaustiveness
          env
          scrutinee_pos
          scrutinee_ty
          cases
          opt_default_case
      | _ -> ()
  end
