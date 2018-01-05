(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_mutability_env
module T = Tast
module Env = Typing_env
module LMap = Local_id.Map
open Typing_defs

let fun_returns_mutable (env : Typing_env.env) (id : Nast.sid) =
  match Env.get_fun env (snd id) with
  | None -> false
  | Some fty ->
    fty.ft_returns_mutable

(* Returns true if the expression returns a new owned mutable *)
let expr_returns_owned_mutable
 (env : Typing_env.env) (e : T.expr)
 : bool =
 match snd e with
 | (T.New _) -> true
 | (T.Call (_, (_, T.Id id), _, _, _))
 | (T.Call (_, (_, T.Fun_id id), _, _, _)) ->
   fun_returns_mutable env id
 | _ -> false

(* Returns true if we can modify properties of the expression *)
let expr_is_mutable
  (env : Typing_env.env) (e : T.expr) : bool =
 match snd e with
 | T.Lvar id ->
    Env.is_mutable env (snd id)
 | _ -> expr_returns_owned_mutable env e


(* Checks for assignment errors as a pass on the TAST *)
let handle_assignment_mutability
 (env : Typing_env.env) (te1 : T.expr) (te2 : T.expr)
 : Typing_env.env =
 (* If e2 is a mutable expression, then e1 is added to the mutability env *)
 let mut_env = Env.get_env_mutability env in
 (* Check for modifying immutable objects *)
 (match snd te1 with
  (* Setting mutable locals is okay *)
 | T.Obj_get (e1, _, _) when expr_is_mutable env e1 -> ()
 | T.Class_get _
 | T.Obj_get _ ->
    let pos = T.get_position te1 in
    Errors.obj_set_reactive pos
 | _ -> ());
 let mut_env = match snd te1, snd te2 with
 | _, T.Lvar(p, id2) when LMap.mem id2 mut_env ->
   (* Reassigning mutables is not allowed; error *)
   Errors.reassign_mutable_var p;
   mut_env
 (* If the expression is a new owned mutable, add the var to the env *)
 | T.Lvar (_, id), _  when expr_returns_owned_mutable env te2 ->
   LMap.add id (T.get_position te1, Mutable) mut_env
 (* If the Lvar gets reassigned and shadowed to something that
   isn't a mutable, it is now a regular immutable variable.
 *)
 | T.Lvar (_, id), _ ->
   LMap.remove id mut_env
 | _ ->
   mut_env in
 Env.env_with_mut env mut_env
