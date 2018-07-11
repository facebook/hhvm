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
 | T.New _
 | T.KeyValCollection ((`Map | `ImmMap), _)
 | T.ValCollection ((`Vector | `ImmVector | `Set | `ImmSet), _)
 | T.Pair _ ->
  true
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
 | T.Binop (Ast.QuestionQuestion, l, r) ->
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
  ~function_returns_void_for_rx
  (env: Typing_env.env)
  fun_pos
  (e: T.expr) =
  let error_mutable e mut_opt =
    let kind =
      match mut_opt with
      | None -> "non-mutable"
      | Some MaybeMutable -> "maybe-mutable"
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
        (* attempt to return borrowed value as immutable
           unless function is marked with __ReturnsVoidToRx in which case caller
           will not be able to alias the value *)
        if not function_returns_void_for_rx
        then error_borrowed_as_immutable e
      | _ ->
        if function_returns_mutable then error_mutable e None
      end
    | T.This when not function_returns_mutable && Env.function_is_mutable env ->
      (* mutable this is treated as borrowed and this cannot be returned as immutable
         unless function is marked with __ReturnsVoidToRx in which case caller
         will not be able to alias the value *)
      if not function_returns_void_for_rx
      then error_borrowed_as_immutable e
    | T.Eif (_, e1_opt, e2) ->
      Option.iter e1_opt ~f:aux;
      aux e2
      (* ?? operator returns owned mutable if both left hand side and right hand side
      return owned mutable *)
    | T.Binop (Ast.QuestionQuestion, l, r) ->
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
 | T.Callconv (Ast.Pinout, (_, T.Lvar id))
 | T.Lvar id ->
    Env.is_mutable env (snd id)
 | T.This when Env.function_is_mutable env -> true
 | T.Call(_, (_, T.Id (_, id)), _, _, _) when id = SN.Rx.mutable_ -> true
 | _ -> false

let expr_is_maybe_mutable
  (env: Typing_env.env)
  (e: T.expr): bool =
  match e with
  | _, T.Lvar (_, id) ->
    let mut_env = Env.get_env_mutability env in
    begin match LMap.get id mut_env with
    | Some (_, MaybeMutable) -> true
    | _ -> false
    end
  | _ -> false

let check_rx_mutable_arguments
  (p : Pos.t) (env : Typing_env.env) (tel : T.expr list) =
  match tel with
  | [e] when expr_returns_owned_mutable env e -> ()
  | _ ->
    (* HH\Rx\mutable function expects single fresh mutably owned value *)
    Errors.invalid_argument_of_rx_mutable_function p

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
    (* maybe mutable parameters allow anything *)
    if param.fp_mutability <> Some Param_maybe_mutable
    then Env.error_if_reactive_context env @@ begin fun () ->
      let is_mutable = expr_is_mutable env e in
      let is_maybe_mutable = expr_is_maybe_mutable env e in
      begin match param.fp_mutability with
      (* maybe-mutable argument value *)
      | _ when is_maybe_mutable ->
        Errors.maybe_mutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | Some Param_mutable when not is_mutable ->
      (* mutable parameter, immutable argument *)
        Errors.mutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | None when is_mutable ->
      (* immutable parameter, mutable argument *)
        Errors.immutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | _ -> ()
      end
    end;
    (* Check the rest *)
    check_param_mutability env ps es

let check_mutability_fun_params env fty el =
  (* exit early if when calling non-reactive function *)
  if fty.ft_reactive = Nonreactive then ()
  else
  let params = fty.ft_params in
  let remaining_exprs = check_param_mutability env params el in
  let rec error_on_first_mismatched_argument ~needs_mutable param es =
    match es with
    | [] -> ()
    | e::es ->
      if expr_is_maybe_mutable env e then
        Errors.maybe_mutable_argument_mismatch (param.fp_pos) (T.get_position e)
      else if expr_is_mutable env e <> needs_mutable then begin
        if needs_mutable
        then Errors.mutable_argument_mismatch (param.fp_pos) (T.get_position e)
        else Errors.immutable_argument_mismatch (param.fp_pos) (T.get_position e)
      end
      else error_on_first_mismatched_argument ~needs_mutable param es in
  Env.error_if_reactive_context env @@ begin fun () ->
    begin match fty.ft_arity with
    (* maybe mutable variadic parameter *)
    | Fvariadic (_, ({ fp_mutability = Some Param_maybe_mutable; _ })) ->
      ()
    (* mutable variadic parameter, ensure that all values being passed are mutable *)
    | Fvariadic (_, ({ fp_mutability = Some Param_mutable; _ } as param)) ->
      error_on_first_mismatched_argument ~needs_mutable:true param remaining_exprs
    (* immutable variadic parameter, ensure that all values being passed are immutable *)
    | Fvariadic (_, ({ fp_mutability = None; _ } as param)) ->
      error_on_first_mismatched_argument ~needs_mutable:false param remaining_exprs
    | _ -> ()
    end
  end


let enforce_mutable_call (env : Typing_env.env) (te : T.expr) =
  match snd te with
  | T.Call (_, (_, T.Id id), _, el, _)
  | T.Call (_, (_, T.Fun_id id), _, el, _) ->
    begin match Env.get_fun env (snd id) with
    | Some fty ->
      check_mutability_fun_params env fty el
    | None -> ()
    end
  (* static methods *)
  | T.Call (_, ((_, (_, Tfun fty)), T.Class_const _), _, el, _) ->
    check_mutability_fun_params env fty el
  (* $x->method() where method is mutable *)
  | T.Call (_, ((pos, (r, Tfun fty)), T.Obj_get (expr, _, _)), _, el, _) ->
    (* do not check receiver mutability when calling non-reactive function *)
    if fty.ft_reactive <> Nonreactive
    then Env.error_if_reactive_context env @@ begin fun () ->
      let fpos = Reason.to_pos r in
      match fty.ft_mutability with
      (* mutable-or-immutable function - ok *)
      | Some Param_maybe_mutable -> ()
      (* mutable call on mutable-or-immutable value - error *)
      | Some Param_mutable when expr_is_maybe_mutable env expr ->
        Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:true pos fpos
      (* non-mutable call on mutable-or-immutable value - error *)
      | None when expr_is_maybe_mutable env expr ->
        Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:false pos fpos
      (* mutable call on immutable value - error *)
      | Some Param_mutable when not (expr_is_mutable env expr) ->
        Errors.mutable_call_on_immutable fpos pos
      (* immutable call on mutable value - error *)
      | None when expr_is_mutable env expr ->
        Errors.immutable_call_on_mutable fpos pos
      (* anything else - ok *)
      | _ -> ()
    end;
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

let rec is_valid_mutable_subscript_expression_target env v =
  match v with
  | (_, ty), T.Lvar _ ->
    is_byval_collection_type env ty
  | (_, ty), T.Array_get (e, _) ->
    is_byval_collection_type env ty &&
    is_valid_mutable_subscript_expression_target env e
  | (_, ty), T.Obj_get (e, _, _) ->
    is_byval_collection_type env ty &&
    (is_valid_mutable_subscript_expression_target env e || expr_is_mutable env e)
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
   when expr_is_mutable env e1 ||
        is_valid_mutable_subscript_expression_target env e1 -> ()
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
     match LMap.find id2 mut_env with
     | _, MaybeMutable -> Errors.reassign_maybe_mutable_var p
     | _ -> Errors.reassign_mutable_var p
   end;
   mut_env
 (* var = mutable(v) - add the var to the env since it points to a owned mutable value *)
 | T.Lvar (_, id), T.Call(_, (_, T.Id (_, n)), _, _, _) when n = SN.Rx.mutable_ ->
    LMap.add id (T.get_position te1, Mutable) mut_env
 (* If the Lvar gets reassigned and shadowed to something that
   isn't a mutable, it is now a regular immutable variable.
 *)
 | T.Lvar (_, id), _ ->
   LMap.remove id mut_env
 | _ ->
   mut_env in
 Env.env_with_mut env mut_env
