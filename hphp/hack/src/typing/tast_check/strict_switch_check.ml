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
module Env = Tast_env

module Value = struct
  type t = Universe  (** unit of intersection, but requires default case *)
  [@@deriving eq, ord, sexp]

  let to_json = function
    | Universe -> Hh_json.string_ "universe"
end

module ValueSet = Set.Make (Value)

(* Symbolically evaluate the values corresponding to a given type; the result is
   essentially in disjunctive normal form, so that cases can be partitioned
   according to each element (transparent enums need special care, see later). *)
let rec symbolic_dnf_values env ty : ValueSet.t =
  let (env, ty) = Env.expand_type env ty in
  let open Value in
  match get_node ty with
  | Tunion tyl ->
    tyl |> List.map ~f:(symbolic_dnf_values env) |> ValueSet.union_list
  | Tintersection tyl ->
    List.fold_right tyl ~init:(ValueSet.singleton Universe) ~f:(fun ty acc ->
        ValueSet.inter (symbolic_dnf_values env ty) acc)
  | Tnewtype (_, _, _)
  | Toption _ ->
    ValueSet.singleton Universe
  | Tprim prim ->
    (match prim with
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
      ValueSet.singleton Universe)
  | Tany _
  | Tnonnull
  | Tneg _
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
    ValueSet.singleton Universe
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

(* The algorithm below works as follows:
   1. Partition the case list into buckets for each (non-dynamic) element of the computed set.
     a. Any leftover values are considered to be redundant/ill-typed.
   2. For each finite value (bool, null, and enums) check that all cases have been covered.
   3. If any of the below are true, then we need a default case.
     a. The value set is the singleton { Dyn }.
     b. The value set contains an infinite value.
     c. There are missing cases for finite values.
   4. Otherwise a default is redundant. *)
let check_cases_against_values
    env
    pos
    values
    (opt_default_case : (Tast.ty, Tast.saved_env) Aast.default_case option) =
  let open Value in
  let needs_default = ValueSet.exists values ~f:(equal Universe) in
  match (opt_default_case, needs_default) with
  | (None, false)
  | (Some _, true) ->
    ()
  | (Some (default_pos, _), false) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        enum
        @@ Primary.Enum.Enum_switch_redundant_default
             {
               pos;
               kind = "default";
               decl_pos = Pos_or_decl.of_raw_pos default_pos;
             })
  | (None, true) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        enum
        @@ Primary.Enum.Enum_switch_nonexhaustive
             {
               pos;
               kind = None;
               decl_pos = Pos_or_decl.of_raw_pos pos;
               missing = ["default"];
             })

let add_fields json ~fields =
  match json with
  | Hh_json.JSON_Object old -> Hh_json.JSON_Object (old @ fields)
  | _ -> Hh_json.JSON_Object (("warning_expected_object", json) :: fields)

let check_exhaustiveness env pos ty dfl =
  let values = symbolic_dnf_values env ty in
  check_cases_against_values env pos values dfl;
  let tcopt = env |> Env.get_decl_env |> Decl_env.tcopt in
  if TypecheckerOptions.tco_log_exhaustivity_check tcopt then
    let fields =
      [
        ("values", values |> ValueSet.to_list |> Hh_json.array_ Value.to_json);
        ("switch_pos", Pos.(pos |> to_absolute |> json));
      ]
    in
    ty
    |> Env.ty_to_json env ~show_like_ty:true
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
