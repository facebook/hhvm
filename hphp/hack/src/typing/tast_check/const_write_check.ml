(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
open Typing_defs
module Env = Tast_env
module SN = Naming_special_names

(* Requires id to be a property *)
let check_static_const_prop tenv class_ (pos, id) =
  let scprop = Typing_env.get_static_member false tenv class_ id in
  Option.iter scprop ~f:(fun ce ->
      if get_ce_const ce then
        Typing_error_utils.add_typing_error
          ~env:tenv
          Typing_error.(primary @@ Primary.Mutating_const_property pos))

(* Requires id to be a property *)
let check_const_prop env tenv class_ (pos, id) cty =
  let cprop = Typing_env.get_member false tenv class_ id in
  Option.iter cprop ~f:(fun ce ->
      if get_ce_const ce then
        if
          not
            (Env.get_inside_constructor env
            && (* expensive call behind short circuiting && *)
            Tast_env.is_sub_type env (Env.get_self_ty_exn env) cty)
        then
          Typing_error_utils.add_typing_error
            ~env:tenv
            Typing_error.(primary @@ Primary.Mutating_const_property pos))

let check_prop env c pid cty_opt =
  let class_ = Env.get_class env c in
  (* Check we're in the LHS of an assignment, so we don't get confused
     by $foo->bar(), which is an Obj_get but not a property. *)
  if Typing_defs.(equal_val_kind (Env.get_val_kind env) Lval) then
    match class_ with
    | Decl_entry.Found class_ ->
      (match cty_opt with
      | Some cty ->
        check_const_prop env (Env.tast_env_as_typing_env env) class_ pid cty
      | None ->
        check_static_const_prop (Env.tast_env_as_typing_env env) class_ pid)
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      ()

let rec check_expr env ((_, _, e) : Tast.expr) =
  match e with
  | Class_get ((cty, _, _), CGstring pid, _) ->
    let (env, cty) = Env.expand_type env cty in
    begin
      match get_node cty with
      | Tclass ((_, c), _, _) -> check_prop env c pid None
      | Tdependent (_, bound) -> begin
        match get_node bound with
        | Tclass ((_, c), _, _) -> check_prop env c pid None
        | _ -> ()
      end
      | Tgeneric (name, targs) ->
        let upper_bounds = Env.get_upper_bounds env name targs in
        let check_class bound =
          match get_node bound with
          | Tclass ((_, c), _, _) -> check_prop env c pid None
          | _ -> ()
        in
        Typing_set.iter check_class upper_bounds
      | _ -> ()
    end
  | Obj_get ((cty, _, _), (_, _, Id id), _, _) ->
    let (env, cty) = Env.expand_type env cty in
    let rec check_const_cty seen cty =
      (* we track already seen types to avoid infinite recursion
         when dealing with Tgeneric arguments *)
      if Typing_set.mem cty seen then
        seen
      else
        let seen = Typing_set.add cty seen in
        match get_node cty with
        | Tunion ty_list
        | Tintersection ty_list ->
          List.fold ty_list ~init:seen ~f:check_const_cty
        | Tclass ((_, c), _, _) ->
          check_prop env c id (Some cty);
          seen
        | Tnewtype (_, _, bound)
        | Tdependent (_, bound) ->
          check_const_cty seen bound
        | Tgeneric (name, targs) ->
          let upper_bounds = Env.get_upper_bounds env name targs in
          let check_class cty seen = check_const_cty seen cty in
          Typing_set.fold check_class upper_bounds seen
        | _ -> seen
    in
    ignore (check_const_cty Typing_set.empty cty)
  | Call { func = (_, _, Id (_, f)); args; unpacked_arg = None; _ }
    when String.equal f SN.PseudoFunctions.unset ->
    let rec check_unset_exp e =
      match e with
      | (_, _, Array_get (e, Some _)) -> check_unset_exp e
      | _ -> check_expr (Env.set_val_kind env Typing_defs.Lval) e
    in
    List.iter args ~f:(fun (_, e) -> check_unset_exp e)
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e = check_expr env e
  end
