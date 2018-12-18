(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_mutability_env
open Typing_defs

module Env = Typing_env
module T = Tast
module LMap = Local_id.Map

module type Env_S = sig
  type env
  val env_reactivity: env -> reactivity
  val get_fun: env -> Typing_heap.Funs.key -> Typing_heap.Funs.t option
end

module Shared(Env: Env_S) = struct
(* true if function has <<__ReturnMutable>> annotation, otherwise false *)
let is_fun_call_returning_mutable (env : Env.env) (e : T.expr): bool =
  let fty_returns_mutable fty =
    match Env.env_reactivity env, fty.ft_reactive with
    (* in localrx context assume non-reactive functions to return mutable *)
    | Local _, Nonreactive -> true
    | _ -> fty.ft_returns_mutable
  in
  let fun_returns_mutable id =
    match Env.get_fun env (snd id) with
    | None -> false
    | Some fty -> fty_returns_mutable fty
  in
  match snd e with
  (* Function call *)
  | T.Call (_, (_, T.Id id), _, _, _)
  | T.Call (_, (_, T.Fun_id id), _, _, _) ->
    fun_returns_mutable id
  | T.Call (_, ((_, (_, Tfun fty)), T.Obj_get _), _, _, _)
  | T.Call (_, ((_, (_, Tfun fty)), T.Class_const _), _, _, _)
  | T.Call (_, ((_, (_, Tfun fty)), T.Lvar _), _, _, _) ->
    fty_returns_mutable fty
  | _ -> false
end

include Shared(Env)

let handle_value_in_return
  ~function_returns_mutable
  ~function_returns_void_for_rx
  (env: Env.env)
  fun_pos
  (e: T.expr): Env.env =
  let error_mutable e mut_opt =
    let kind =
      match mut_opt with
      | Immutable -> "(non-mutable)"
      | MaybeMutable -> "(maybe-mutable)"
      | Borrowed -> "(borrowed)"
      | Mutable -> assert false in
    Errors.invalid_mutable_return_result (T.get_position e) fun_pos kind in
  let error_borrowed_as_immutable e =
    (* attempt to return borrowed value as immutable *)
    Errors.cannot_return_borrowed_value_as_immutable
      fun_pos
      (T.get_position e) in
  let rec aux e =
    match snd e with
    (* ignore nulls - it is ok to return then from functions
       that return nullable types and for non-nullable return types it will
       be an error anyways *)
    | T.Null -> env
    (* allow bare new expressions
       - implicit Rx\mutable in __MutableReturn functions *)
    | T.New _  | T.Xml _ -> env
    | T.Call(_, (_, T.Id (_, id)), _, _, _) when id = SN.Rx.mutable_ ->
      (* ok to return result of Rx\mutable - implicit Rx\move *)
      env
    | T.Pipe (_, _, r) ->
      (* ok for pipe if rhs returns mutable *)
      aux r
    | T.Lvar (_, id) ->
      let mut_env = Env.get_env_mutability env in
      begin match LMap.get id mut_env with
      | Some (p, Mutable) ->
        (* it is ok to return mutably owned values *)
        let env = Env.unset_local env id in
        Env.env_with_mut env (LMap.add id (p, Mutable) mut_env)
      | Some (_, Borrowed) when not function_returns_mutable ->
        (* attempt to return borrowed value as immutable
           unless function is marked with __ReturnsVoidToRx in which case caller
           will not be able to alias the value *)
        if not function_returns_void_for_rx
        then error_borrowed_as_immutable e;
        env
      | Some (_, mut) when function_returns_mutable ->
        error_mutable e mut;
        env
      | _ ->
        env
      end
    | T.This when not function_returns_mutable && Env.function_is_mutable env <> None ->
      (* mutable this is treated as borrowed and this cannot be returned as immutable
         unless function is marked with __ReturnsVoidToRx in which case caller
         will not be able to alias the value *)
      if not function_returns_void_for_rx
      then error_borrowed_as_immutable e;
      env
    | _ ->
      (* for __MutableReturn functions allow delegating calls
        to __MutableReturn functions *)
      if function_returns_mutable && not (is_fun_call_returning_mutable env e)
      then begin
        let kind = "not valid return value for __MutableReturn functions." in
        Errors.invalid_mutable_return_result (T.get_position e) fun_pos kind
      end;
      env in
  aux e
let freeze_or_move_local
  (p : Pos.t)
  (env : Typing_env.env)
  (tel : T.expr list)
  (invalid_target : Pos.t -> Pos.t -> string -> unit)
  (invalid_use : Pos.t -> unit)
  : Typing_env.env =
  match tel with
  | [_, T.Any] -> env
  | [(_, T.Lvar (id_pos, id));] ->
    let mut_env = Env.get_env_mutability env in
    begin match LMap.get id mut_env with
    | Some (p, Mutable) ->
      let env = Env.unset_local env id in
      Env.env_with_mut env (LMap.add id (p, Mutable) mut_env)
    | Some x ->
      invalid_target p id_pos (to_string x);
      env
    | None ->
      invalid_target p id_pos "immutable";
      env
    end
  | [((id_pos, _), T.This);] ->
      invalid_target p id_pos "the this type, which is mutably borrowed";
      env
  | _ ->
    (* Error, freeze/move takes a single local as an argument *)
    invalid_use p;
    env

let freeze_local
  (p : Pos.t)
  (env : Typing_env.env)
  (tel : T.expr list)
  : Typing_env.env =
  freeze_or_move_local p env tel
    Errors.invalid_freeze_target Errors.invalid_freeze_use

let move_local
  (p : Pos.t)
  (env : Typing_env.env)
  (tel : T.expr list)
  : Typing_env.env =
  freeze_or_move_local p env tel
    Errors.invalid_move_target Errors.invalid_move_use

let rec is_move_or_mutable_call ?(allow_move=true) te =
  match te with
  | T.Call(_, (_, T.Id (_, n)), _, _, _) ->
      n = SN.Rx.mutable_ || (allow_move && n = SN.Rx.move)
  | T.Pipe (_, _, (_, r)) -> is_move_or_mutable_call ~allow_move:false r
  | _ -> false

(* Checks for assignment errors as a pass on the TAST *)
let handle_assignment_mutability
 (env : Typing_env.env) (te1 : T.expr) (te2 : T.expr_ option)
 : Typing_env.env =
 (* If e2 is a mutable expression, then e1 is added to the mutability env *)
 let mut_env = Env.get_env_mutability env in
 let mut_env = match snd te1, te2 with
 | _, Some T.This when Env.function_is_mutable env = Some Param_borrowed_mutable ->
 (* aliasing $this - bad for __Mutable and __MaybeMutable functions *)
   Env.error_if_reactive_context env @@ begin fun () ->
     Errors.reassign_mutable_this ~in_collection:false ~is_maybe_mutable:false
      (T.get_position te1)
   end;
   mut_env
 | _, Some T.This when Env.function_is_mutable env = Some Param_maybe_mutable ->
 (* aliasing $this - bad for __Mutable and __MaybeMutable functions *)
   Env.error_if_reactive_context env @@ begin fun () ->
     Errors.reassign_mutable_this ~in_collection:false ~is_maybe_mutable:true
      (T.get_position te1)
   end;
   mut_env
 (* var = mutable(v)/move(v) - add the var to the env since it points to a owned mutable value *)
 | T.Lvar (p, id), Some e when is_move_or_mutable_call e ->
    begin match LMap.get id mut_env with
    | Some (_, (Immutable | Borrowed | MaybeMutable) as mut) ->
      (* error when assigning owned mutable to another mutability flavor *)
      Errors.invalid_mutability_flavor p
        (to_string mut)
        "mutable"
    | _ -> ()
    end;
    LMap.add id (T.get_position te1, Mutable) mut_env
  (* Reassigning mutables is not allowed; error *)
  | _, Some T.Lvar(p, id2) when
    (Option.value_map (LMap.get id2 mut_env)
      ~default:false
      ~f:(fun (_, m) -> m <> Immutable)
    ) ->
    Env.error_if_reactive_context env @@ begin fun () ->
      match LMap.find id2 mut_env with
      | _, MaybeMutable -> Errors.reassign_maybe_mutable_var ~in_collection:false p
      | _ -> Errors.reassign_mutable_var ~in_collection:false p
    end;
    mut_env
 (* If the Lvar gets reassigned and shadowed to something that
   isn't a mutable, it is now a regular immutable variable.
 *)
 | T.Lvar (p, id), _ ->
    begin match LMap.get id mut_env with
    (* ok if local is immutable*)
    | Some (_, Immutable) ->
      mut_env
    (* error assigning immutable value to local known to be mutable *)
    | Some mut ->
      Errors.invalid_mutability_flavor p
        (to_string mut)
        "immutable";
      mut_env
    | None ->
      (* ok - add new locals *)
      LMap.add id (T.get_position te1, Immutable) mut_env
    end;
 | _ ->
    mut_env in
 Env.env_with_mut env mut_env
