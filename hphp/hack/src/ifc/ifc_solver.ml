(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types
module Logic = Ifc_logic
module Mapper = Ifc_mapper
module Pp = Ifc_pretty
module Utils = Ifc_utils
module L = Logic.Infix

type solving_error =
  | RecursiveCycle
  | MissingResults of string
  (* InvalidCall (reason, caller, callee) *)
  | InvalidCall of string * string * string

exception Error of solving_error

(* Combine the results of each individual function into a global
   constraint. If the resulting constraint is satisfiable all the
   flows in the program are safe *)
let global_exn ~subtype callable_results =
  let results_map =
    let add_result resm res = SMap.add res.res_proto.fp_name res resm in
    List.fold ~init:SMap.empty ~f:add_result callable_results
  in
  let topsort_schedule =
    let schedule = ref [] in
    let rec dfs seen name =
      if SSet.mem name seen then raise (Error RecursiveCycle);
      match SMap.find_opt name results_map with
      | None -> raise (Error (MissingResults name))
      | Some res ->
        SSet.iter (dfs (SSet.add name seen)) res.res_deps;
        if not (List.exists ~f:(String.equal name) !schedule) then
          schedule := name :: !schedule
    in
    SMap.iter (fun name _ -> dfs SSet.empty name) results_map;
    List.rev !schedule
  in
  let close_one closed_results_map res_name =
    let result = SMap.find res_name results_map in
    let rec subst depth = function
      | Chole ({ fp_name = callee_name; _ } as proto) ->
        let invalid_call reason =
          raise (Error (InvalidCall (reason, res_name, callee_name)))
        in
        let callee = SMap.find callee_name closed_results_map in
        assert (not (Scope.equal callee.res_scope result.res_scope));
        let arg_pairs =
          match List.zip proto.fp_args callee.res_proto.fp_args with
          | None -> invalid_call "arity mismatch"
          | Some l -> l
        in
        let pred (_, s) = Scope.equal s callee.res_scope in
        arg_pairs
        |> List.fold ~init:[callee.res_constraint] ~f:(fun prop (t1, t2) ->
               subtype t1 t2 prop)
        |> L.(proto.fp_pc < callee.res_proto.fp_pc)
        |> subtype callee.res_proto.fp_exn proto.fp_exn
        |> (match (proto.fp_this, callee.res_proto.fp_this) with
           | (Some t1, Some t2) -> subtype t1 t2
           | (None, Some _) ->
             invalid_call ("expected '" ^ callee_name ^ "' to be a function")
           | (Some _, None) ->
             invalid_call ("expected '" ^ callee_name ^ "' to be a method")
           | (None, None) -> Utils.identity)
        |> subtype callee.res_proto.fp_ret proto.fp_ret
        |> Logic.conjoin
        (* the assertion on scopes above ensures that we quantify
           only the callee policy variables *)
        |> Logic.quantify ~pred ~quant:Qexists ~depth
      | c -> Mapper.prop Utils.identity subst depth c
    in
    let closed_constr = subst 0 result.res_constraint in
    let closed_result = { result with res_constraint = closed_constr } in
    SMap.add res_name closed_result closed_results_map
  in
  List.fold_left ~init:SMap.empty ~f:close_one topsort_schedule
