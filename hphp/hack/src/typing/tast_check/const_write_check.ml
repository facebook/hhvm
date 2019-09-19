(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Core_kernel
open Typing_defs
module Env = Tast_env

(* Requires id to be a property *)
let check_static_const_prop tenv class_ (pos, id) =
  let scprop = Typing_env.get_static_member false tenv class_ id in
  Option.iter scprop ~f:(fun { ce_const; _ } ->
      if ce_const then Errors.mutating_const_property pos)

(* Requires id to be a property *)
let check_const_prop env tenv class_ (pos, id) cty =
  let cprop = Typing_env.get_member false tenv class_ id in
  Option.iter cprop ~f:(fun { ce_const; _ } ->
      if ce_const then
        if
          not
            ( Env.get_inside_constructor env
            && (* expensive call behind short circuiting && *)
               Tast_env.is_sub_type env (Env.get_self_exn env) cty )
        then
          Errors.mutating_const_property pos)

let check_prop env c pid cty_opt =
  let class_ = Env.get_class env c in
  (* Check we're in the LHS of an assignment, so we don't get confused
     by $foo->bar(), which is an Obj_get but not a property. *)
  if Env.get_val_kind env = Typing_defs.Lval then
    Option.iter class_ ~f:(fun class_ ->
        match cty_opt with
        | Some cty ->
          check_const_prop env (Env.tast_env_as_typing_env env) class_ pid cty
        | None ->
          check_static_const_prop (Env.tast_env_as_typing_env env) class_ pid)

let rec check_expr env (_, e) =
  match e with
  | Class_get (((_, cty), _), CGstring pid) ->
    begin
      match snd cty with
      | Tclass ((_, c), _, _) -> check_prop env c pid None
      | Tabstract (AKdependent _, Some (_, Tclass ((_, c), _, _))) ->
        check_prop env c pid None
      | Tabstract (AKgeneric name, None) ->
        let upper_bounds = Env.get_upper_bounds env name in
        let check_class bound =
          match snd bound with
          | Tclass ((_, c), _, _) -> check_prop env c pid None
          | _ -> ()
        in
        Typing_set.iter check_class upper_bounds
      | _ -> ()
    end
  | Obj_get (((_, cty), _), (_, Id id), _) ->
    begin
      match snd cty with
      | Tclass ((_, c), _, _) -> check_prop env c id (Some cty)
      | Tabstract (_, (Some (_, Tclass ((_, c), _, _)) as ty)) ->
        check_prop env c id ty
      | _ -> ()
    end
  | Call (_, (_, Id (_, f)), _, el, []) when f = SN.PseudoFunctions.unset ->
    let rec check_unset_exp e =
      match e with
      | (_, Array_get (e, Some _)) -> check_unset_exp e
      | _ -> check_expr (Env.set_val_kind env Typing_defs.Lval) e
    in
    List.iter el check_unset_exp
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e = check_expr env e
  end
