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
  type t = Other  (** requires default case *) [@@deriving ord, sexp]

  let universe = [Other]

  let finite_or_dynamic _ = false

  let to_json = function
    | Other -> Hh_json.string_ "other"
end

module ValueSet = struct
  include Set.Make (Value)

  let universe = of_list Value.universe

  let intersection_list value_sets =
    List.fold_right ~init:universe ~f:inter value_sets
end

let prim_to_values = function
  | Tnoreturn -> ValueSet.empty
  | Tnull
  | Tvoid
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
  | Tnonnull
  | Toption _
  | Tneg _
  | Tnewtype (_, _, _)
  | Tany _ ->
    ValueSet.universe
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

module EnumErr = Typing_error.Primary.Enum

let register_err env err =
  Typing_error_utils.add_typing_error ~env @@ Typing_error.enum err

type default = (Tast.ty, Tast.saved_env) Aast.default_case

let check_default env pos (opt_default_case : default option) needs_default =
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
         {
           pos;
           kind = None;
           decl_pos = Pos_or_decl.of_raw_pos pos;
           missing = ["default"];
         }

(* The algorithm below works as follows:
   1. Partition the case list into buckets for each (non-dynamic) element of the computed set.
     a. Any leftover values are considered to be redundant/ill-typed.
   2. For each finite value (bool, null, and enums) check that all cases have been covered.
   3. If any of the below are true, then we need a default case.
     a. The value set is the singleton { Dynamic }.
     b. The value set contains an neither-finite-nor-dynamic value.
     c. There are missing cases for finite values.
   4. Otherwise a default is redundant. *)
let check_cases_against_values env pos values opt_default_case =
  let needs_default =
    (* I write `exists ~f:(fun x -> not @@ finite x)` instead of all
       `forall ~f:finite` to ensure set is non-empty *)
    ValueSet.exists ~f:(fun x -> not @@ Value.finite_or_dynamic x) values
  in
  let typing_env = Tast_env.tast_env_as_typing_env env in
  check_default typing_env pos opt_default_case needs_default

let add_fields json ~fields =
  match json with
  | Hh_json.JSON_Object old -> Hh_json.JSON_Object (old @ fields)
  | _ -> Hh_json.JSON_Object (("warning_expected_object", json) :: fields)

let check_exhaustiveness env pos ty opt_default =
  let values = symbolic_dnf_values env ty in
  check_cases_against_values env pos values opt_default;
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
      | Switch ((scrutinee_ty, scrutinee_pos, _), _cases, opt_default_case) ->
        check_exhaustiveness env scrutinee_pos scrutinee_ty opt_default_case
      | _ -> ()
  end
