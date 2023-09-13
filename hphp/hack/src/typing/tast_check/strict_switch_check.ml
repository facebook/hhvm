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
module EnumErr = Typing_error.Primary.Enum

type literal = EnumErr.Const.t

type ast_case = (Tast.ty, Tast.saved_env) Aast.expr

type case = Pos.t * literal

module Value = struct
  type t =
    | Other  (** requires default case *)
    | Null
    | Bool of bool
    | Int
  [@@deriving ord, sexp, hash]

  let bools = [Bool true; Bool false]

  let universe = Other :: Null :: Int :: bools

  let finite_or_dynamic = function
    | Other
    | Int ->
      false
    | Null
    | Bool _ ->
      true

  let to_json = function
    | Other -> Hh_json.string_ "other"
    | Null -> Hh_json.string_ "null"
    | Bool bool -> bool |> Bool.to_string |> Hh_json.string_
    | Int -> Hh_json.string_ "int"

  let to_literal : t -> literal option =
    let open EnumErr.Const in
    function
    | Null -> Some Null
    | Bool b -> Some (Bool b)
    | Int -> Some (Int None)
    | Other -> None

  type value = t

  (* design choice: could check for cases of type null rather than the null
     literal expression. Ultimately we think case branches should only allow
     literals or class constants, and so disregard non-literal expressions *)
  let and_literal_of_ast_case env ((ty, _, expr) : ast_case) :
      t * literal option =
    let open EnumErr.Const in
    match expr with
    | Null -> (Null, Some Null)
    | True -> (Bool true, Some (Bool true))
    | False -> (Bool false, Some (Bool false))
    | Int literal -> (Int, Some (Int (Some literal)))
    | Class_const ((_, _, CI (_, class_)), (_, const)) ->
      let is_sub env mk_ty ty =
        Typing_subtype.is_sub_type env ty (mk_ty Reason.Rnone)
      in

      let value : value =
        (* necessary to check this to partition class constants correctly
           according to type *)
        if is_sub env Typing_make_type.int ty then
          Int
        else
          Other
      in

      (value, Some (Label { class_; const }))
    | _ -> (Other, None)
end

module ValueSet = struct
  include Set.Make (Value)

  let universe = of_list Value.universe

  let bools = of_list Value.bools

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
  | Tbool -> ValueSet.bools
  | Tint -> ValueSet.singleton Value.Int
  | Tfloat
  | Tstring
  | Tresource
  | Tnum ->
    ValueSet.of_list Value.[Int; Other]
  | Tarraykey -> ValueSet.singleton Value.Other

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

(* Partition cases based on the kind of literal expression (and later, type) *)
let partition_cases env (cases : (ast_case * _) list) values =
  let partitions : (Value.t, literal * case list) Hashtbl.t =
    let tbl = Hashtbl.create (module Value) in

    ValueSet.iter values ~f:(fun key ->
        Option.iter (Value.to_literal key) ~f:(fun if_missing ->
            Hashtbl.add_exn tbl ~key ~data:(if_missing, [])));
    tbl
  in

  let key_present_data_consed table ~key ~data =
    Hashtbl.find_and_call
      table
      key
      ~if_found:(fun (if_missing, rest) ->
        Hashtbl.set table ~key ~data:(if_missing, data :: rest);
        true)
      ~if_not_found:(fun _ -> false)
  in

  let unused_cases =
    List.fold_left cases ~init:[] ~f:(fun unused (((ty, pos, _) as case), _) ->
        let (key, lit) = Value.and_literal_of_ast_case env case in
        match lit with
        | None -> (ty, pos, None) :: unused
        | Some lit ->
          if key_present_data_consed partitions ~key ~data:(pos, lit) then
            unused
          else
            (ty, pos, Some lit) :: unused)
  in
  (partitions, unused_cases)

let add_err env err =
  Typing_error_utils.add_typing_error ~env @@ Typing_error.enum err

let get_missing_cases env partitions =
  let opt_missing env missing = function
    | [] -> Some missing
    | _ :: _ as cases ->
      (* construct a map from literals to case positions... *)
      Hashtbl.group
        (module EnumErr.Const)
        cases
        ~get_key:snd
        ~get_data:(fun (pos, _) -> [pos])
        ~combine:( @ )
      (* ...to perform a redundant case check *)
      |> Hashtbl.iteri ~f:(fun ~key:const_name ~data:positions ->
             match positions with
             | [] ->
               (* ~get_data never produces an empty list;
                  ~combine preserves non-emptiness *)
               assert false
             | [_] -> ()
             | redundant_pos :: (_ :: _ as tl) ->
               let first_pos = List.last_exn tl in
               add_err env
               @@ EnumErr.Enum_switch_redundant
                    { const_name; first_pos; pos = redundant_pos });
      None
  in

  let opt_cons opt ~tl:default =
    Option.value_map opt ~f:(fun x -> x :: default) ~default
  in

  Hashtbl.fold
    partitions
    ~init:[]
    ~f:(fun ~key:_ ~data:(if_missing, cases) acc ->
      opt_cons (opt_missing env if_missing cases) ~tl:acc)

let error_unused_cases env expected unused_cases =
  let expected = lazy (Typing_print.full_strip_ns env expected) in
  List.iter unused_cases ~f:(fun (ty, pos, lit) ->
      add_err env
      @@
      match lit with
      | None -> EnumErr.Enum_switch_not_const pos
      | Some _ ->
        EnumErr.Enum_switch_wrong_class
          {
            pos;
            kind = "";
            expected = Lazy.force expected;
            actual = Typing_print.full_strip_ns env ty;
          })

type ast_default = (Tast.ty, Tast.saved_env) Aast.default_case

let check_default
    env pos (opt_default_case : ast_default option) needs_default missing =
  match (opt_default_case, needs_default) with
  | (None, false)
  | (Some _, true) ->
    ()
  | (Some (default_pos, _), false) ->
    add_err env
    @@ EnumErr.Enum_switch_redundant_default
         {
           pos;
           kind = "default";
           decl_pos = Pos_or_decl.of_raw_pos default_pos;
         }
  | (None, true) ->
    add_err env
    @@ EnumErr.Enum_switch_nonexhaustive
         { pos; kind = None; decl_pos = Pos_or_decl.of_raw_pos pos; missing }

let add_default_if_needed values missing_cases =
  (* write `exists ~f:(fun x -> not @@ finite_or_dynamic x)` instead of
     `forall ~f:finite` to ensure set is non-empty *)
  if ValueSet.exists ~f:(fun x -> not @@ Value.finite_or_dynamic x) values then
    None :: List.map ~f:Option.some missing_cases
  else
    List.map ~f:Option.some missing_cases

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
  let typing_env = Tast_env.tast_env_as_typing_env env in
  let (partitions, unused_cases) = partition_cases typing_env cases values in
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
