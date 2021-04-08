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
module Utils = Ifc_utils
module L = Logic.Infix

type solving_error =
  | RecursiveCycle
  | MissingResults of string
  (* InvalidCall (caller, callee) *)
  | InvalidCall of string * string

exception Error of solving_error

let call_constraint ~subtype ~pos ?(depth = 0) proto scheme =
  let (Fscheme (callee_scope, callee_proto, callee_constraint)) = scheme in
  let pred (_, s) = Scope.equal s callee_scope in
  let make_constraint this_types =
    Some
      ( [callee_constraint]
      |> subtype ~pos (Tfun callee_proto.fp_type) (Tfun proto.fp_type)
      |> (match this_types with
         | Some (t1, t2) -> subtype ~pos t1 t2
         | None -> Utils.identity)
      |> Logic.conjoin
      (* the assertion on scopes in close_one below ensures that
         we quantify only the callee policy variables *)
      |> Logic.quantify ~pred ~quant:Qexists ~depth )
  in
  match (proto.fp_this, callee_proto.fp_this) with
  | (Some this1, Some this2) -> make_constraint (Some (this1, this2))
  | (None, None) -> make_constraint None
  | _ -> None

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
      | Chole (pos, proto) ->
        let callee = SMap.find proto.fp_name closed_results_map in
        assert (not (Scope.equal callee.res_scope result.res_scope));
        let scheme =
          Fscheme (callee.res_scope, callee.res_proto, callee.res_constraint)
        in
        begin
          match call_constraint ~subtype ~depth ~pos proto scheme with
          | Some prop -> prop
          | None -> raise (Error (InvalidCall (res_name, proto.fp_name)))
        end
      | c -> Mapper.prop Utils.identity subst depth c
    in
    let closed_constr = subst 0 result.res_constraint in
    let closed_result = { result with res_constraint = closed_constr } in
    SMap.add res_name closed_result closed_results_map
  in
  List.fold_left ~init:SMap.empty ~f:close_one topsort_schedule
