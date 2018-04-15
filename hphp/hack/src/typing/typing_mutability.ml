(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_mutability_env
module T = Tast
module Env = Typing_env
module LMap = Local_id.Map
open Typing_defs
open Hh_core

let fun_returns_mutable (env : Typing_env.env) (id : Nast.sid) =
  match Env.get_fun env (snd id) with
  | None -> false
  | Some fty ->
    fty.ft_returns_mutable

(* Returns true if the expression returns a new owned mutable *)
let rec expr_returns_owned_mutable
 (env : Typing_env.env) (e : T.expr)
 : bool =
 match snd e with
 | T.New _ -> true
 (* Function call *)
 | T.Call (_, (_, T.Id id), _, _, _)
 | T.Call (_, (_, T.Fun_id id), _, _, _) ->
   fun_returns_mutable env id
 | T.Call (_, ((_, (_, Tfun fty)), T.Obj_get _), _, _, _)
 | T.Call (_, ((_, (_, Tfun fty)), T.Class_const _), _, _, _)->
   fty.ft_returns_mutable
 (* conditional operator returns owned mutable if both consequence and alternative
    return owned mutable *)
 | T.Eif (_, e1_opt, e2) ->
   Option.value_map e1_opt ~default:true ~f:(expr_returns_owned_mutable env) &&
   expr_returns_owned_mutable env e2
 (* ?? operator returns owned mutable if both left hand side and right hand side
    return owned mutable *)
 | T.NullCoalesce (l, r) ->
   expr_returns_owned_mutable env l && expr_returns_owned_mutable env r
 (* cast returns owned mutable if its expression part is owned mutable *)
 | T.Cast (_, e) ->
   expr_returns_owned_mutable env e
 (* XHP expression is considered owned mutable *)
 | T.Xml _ -> true
 (* l |> r returns owned mutable if r yields owned mutable *)
 | T.Pipe (_, _, r) ->
   expr_returns_owned_mutable env r
 | _ -> false

let check_function_return_value
  ~function_returns_mutable
  (env: Typing_env.env)
  fun_pos
  (e: T.expr) =
  let error_mutable e mut_opt =
    let kind =
      match mut_opt with
      | None -> "non-mutable"
      | Some Const -> "const"
      | Some Borrowed -> "borrowed"
      | Some Mutable -> assert false in
    Errors.invalid_mutable_return_result (T.get_position e) fun_pos kind in
  let error_borrowed_as_immutable e =
    (* attempt to return borrowed value as immutable *)
    Errors.cannot_return_borrowed_value_as_immutable
      fun_pos
      (T.get_position e) in
  let rec aux e =
    match snd e with
    | T.Lvar (_, id) ->
      let mut_env = Env.get_env_mutability env in
      begin match LMap.get id mut_env with
      | Some (_, Mutable) ->
        (* it is ok to return mutably owned values *)
        ()
      | Some (_, mut) when function_returns_mutable ->
        error_mutable e (Some mut)
      | Some (_, Borrowed) when not function_returns_mutable ->
        (* attempt to return borrowed value as immutable *)
        error_borrowed_as_immutable e
      | _ ->
        if function_returns_mutable then error_mutable e None
      end
    | T.This when not function_returns_mutable && Env.function_is_mutable env ->
      (* mutable this is treated as borrowed and this cannot be returned as immutable *)
      error_borrowed_as_immutable e
    | T.Eif (_, e1_opt, e2) ->
      Option.iter e1_opt ~f:aux;
      aux e2
      (* ?? operator returns owned mutable if both left hand side and right hand side
      return owned mutable *)
    | T.NullCoalesce (l, r) ->
      aux l;
      aux r;
    (* cast returns owned mutable if its expression part is owned mutable *)
    | T.Cast (_, e) ->
      aux e
    (* XHP expression is considered owned mutable *)
    | T.Xml _ -> ()
    (* l |> r returns owned mutable if r yields owned mutable *)
    | T.Pipe (_, _, r) ->
      aux r
      (* NOTE: we only consider mutable objects as legal return values so
      literals, arrays/varrays/darrays/collections, unary/binary expressions
      that does not yield objects, ints/floats are not considered valid

      CONSIDER: We might consider to report special error message for scenarios when
      return value is known to be literal\primitive\value with immutable semantics.
      *)
    | _ ->
      if function_returns_mutable && not (expr_returns_owned_mutable env e)
      then error_mutable e None in
 aux e

(* Returns true if we can modify properties of the expression *)
let expr_is_mutable
  (env : Typing_env.env) (e : T.expr) : bool =
 match snd e with
 | T.Lvar id ->
    Env.is_mutable env (snd id)
 | T.This when Env.function_is_mutable env -> true
 | _ -> expr_returns_owned_mutable env e

let freeze_local (p : Pos.t) (env : Typing_env.env) (tel : T.expr list)
: Typing_env.env =
  match tel with
  | [(_, T.Lvar (id_pos, id));] ->
    let mut_env = Env.get_env_mutability env in
    let mut_env =
    match LMap.get id mut_env with
    | Some (_, Mutable) ->
      LMap.remove id mut_env
    | Some x ->
      Errors.invalid_freeze_target p id_pos (to_string x);
      mut_env
    | None ->
      Errors.invalid_freeze_target p id_pos "immutable";
      mut_env in
    Env.env_with_mut env mut_env
  | [((id_pos, _), T.This);] ->
      Errors.invalid_freeze_target p id_pos "the this type, which is mutably borrowed";
      env
  | _ ->
    (* Error, freeze takes a single local as an argument *)
    Errors.invalid_freeze_use p;
    env

(* Checks that each parameter that is marked mutable is mutable *)
(* There's no List.iter2_shortest so I'm stuck with this *)
(* Return the remaining expressions to check against the variadic argument *)
let rec check_param_mutability (env : Typing_env.env)
  (params : 'a fun_params ) (el : T.expr list) : T.expr list  =
  match params, el with
  | [], _
  | _, [] -> el
  | param::ps, e::es ->
    if param.fp_mutable then
      if not (expr_is_mutable env e) then
        Env.error_if_reactive_context env @@ begin fun () ->
          Errors.mutable_argument_mismatch (param.fp_pos) (T.get_position e)
        end;

    (* Check the rest *)
    check_param_mutability env ps es

let check_mutability_fun_params env fty el =
  let params = fty.ft_params in
  let remaining_exprs = check_param_mutability env params el in
  begin match fty.ft_arity with
  | Fvariadic (_, param) when param.fp_mutable ->
    begin match List.find remaining_exprs
      ~f:(fun e -> not (expr_is_mutable env e)) with
    | Some expr ->
      Env.error_if_reactive_context env @@ begin fun () ->
        Errors.mutable_argument_mismatch (param.fp_pos) (T.get_position expr)
      end
    | None -> ()
    end
  | _ -> () end


let enforce_mutable_call (env : Typing_env.env) (te : T.expr) =
  match snd te with
  | T.Call (_, (_, T.Id id), _, el, _)
  | T.Call (_, (_, T.Fun_id id), _, el, _) ->
    begin match Env.get_fun env (snd id) with
    | Some fty ->
      check_mutability_fun_params env fty el
    | None -> ()
    end
  (* $x->method() where method is mutable *)
  | T.Call (_, ((pos, (r, Tfun fty)), T.Obj_get (expr, _, _)), _, el, _) ->
    (if fty.ft_mutable && not (expr_is_mutable env expr) then
      Env.error_if_reactive_context env @@ begin fun () ->
        let fpos = Reason.to_pos r in
        Errors.mutable_call_on_immutable fpos pos
      end);
    check_mutability_fun_params env fty el
  (* TAny, T.Calls that don't have types, etc *)
  | _ -> ()

let rec is_byval_collection_type env ty =
  let check t =
    match t with
    | (_, Tclass ((_, x), _)) ->
      x = SN.Collections.cVec ||
      x = SN.Collections.cDict ||
      x = SN.Collections.cKeyset
    | _, (Tarraykind _ | Ttuple _ | Tshape _)
      -> true
    | _, Tunresolved tl -> Core_list.for_all tl ~f:(is_byval_collection_type env)
    | _ -> false in
  let _, tl = Typing_utils.get_all_supertypes env ty in
  Core_list.for_all tl ~f:check

let rec is_byval_collection_value env v =
  match v with
  | (_, ty), T.Lvar _ ->
    is_byval_collection_type env ty
  | (_, ty), T.Array_get (e, _) ->
    is_byval_collection_type env ty &&
    is_byval_collection_value env e
  | _ -> false

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
 | T.Array_get (e1, _)
   when expr_is_mutable env e1 || is_byval_collection_value env e1 -> ()
 | T.Class_get _
 | T.Obj_get _
 | T.Array_get _ ->
    Env.error_if_reactive_context env @@ begin fun () ->
      let pos = T.get_position te1 in
      Errors.obj_set_reactive pos
    end
 | _ -> ());
 let mut_env = match snd te1, snd te2 with
 | _, T.Lvar(p, id2) when LMap.mem id2 mut_env ->
   Env.error_if_reactive_context env @@ begin fun () ->
     (* Reassigning mutables is not allowed; error *)
     Errors.reassign_mutable_var p
   end;
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
