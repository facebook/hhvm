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
module T = Tast
module Env = Typing_env
module LMap = Local_id.Map
open Typing_defs

type borrowable_args = Arg_this | Arg_local of Local_id.S.t

module Borrowable_args = Caml.Map.Make(struct
  type t = borrowable_args
  let compare (a: t) (b: t) = compare a b
end)

type args_mut_map = (Pos.t * param_mutability option) Borrowable_args.t

(* true if function has <<__ReturnMutable>> annotation, otherwise false *)
let is_fun_call_returning_mutable (env : Typing_env.env) (e : T.expr): bool =
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
  | T.Call (_, ((_, (_, Tfun fty)), T.Class_const _), _, _, _) ->
    fty_returns_mutable fty
  | _ -> false


(* Returns true if the expression is valid argument for Rx\mutable *)
let is_valid_rx_mutable_arg env e =
  match snd e with
  | T.New _
  | T.KeyValCollection ((`Map | `ImmMap), _)
  | T.ValCollection ((`Vector | `ImmVector | `Set | `ImmSet), _)
  | T.Pair _
  | T.Xml _ ->
    true
  | _ -> is_fun_call_returning_mutable env e

(* checks arguments to Rx\mutable function - should be owned mutable value
  excluding locals  *)
let check_rx_mutable_arguments
  (p : Pos.t) (env : Typing_env.env) (tel : T.expr list) =
  match tel with
  | [e] when is_valid_rx_mutable_arg env e -> ()
  | _ ->
    (* HH\Rx\mutable function expects single fresh mutably owned value *)
    Errors.invalid_argument_of_rx_mutable_function p

let handle_value_in_return
  ~function_returns_mutable
  ~function_returns_void_for_rx
  (env: Typing_env.env)
  fun_pos
  (e: T.expr): Typing_env.env =
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
    | T.This when not function_returns_mutable && Env.function_is_mutable env ->
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
        let kind =
          if is_valid_rx_mutable_arg env e
          then kind ^ " Did you forget to wrap it in Rx\\mutable?"
          else kind in
        Errors.invalid_mutable_return_result (T.get_position e) fun_pos kind
      end;
      env in
  aux e

(* true if expression is valid argument for __OwnedMutable parameter:
  - Rx\move(owned-local)
  - Rx\mutable(call or new) *)
let expr_is_valid_owned_arg (e : T.expr) : bool =
  match snd e with
  | T.Call(_, (_, T.Id (_, id)), _, _, _) -> id = SN.Rx.mutable_ || id = SN.Rx.move
  | _ -> false

let local_is_mutable ~include_borrowed env (_, id) =
  match LMap.get id (Env.get_env_mutability env) with
  | Some (_, Mutable) -> true
  | Some (_, Borrowed) -> include_borrowed
  | _ -> false

(* true if expression is valid argument for __Mutable parameter:
  - Rx\move(owned-local)
  - Rx\mutable(call or new)
  - owned-or-borrowed-local *)
let expr_is_valid_borrowed_arg env (e: T.expr): bool =
  expr_is_valid_owned_arg e || begin
    match snd e with
    | T.Callconv (Ast.Pinout, (_, T.Lvar id))
    | T.Lvar id -> local_is_mutable ~include_borrowed:true env id
    | T.This when Env.function_is_mutable env -> true
    | _ ->
      false
  end

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

let is_owned_local env e =
  match snd e with
  | T.Lvar id -> local_is_mutable ~include_borrowed:false env id
  | _ -> false

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

let with_mutable_value env e ~default ~f =
  match snd e with
  (* invoke f only for mutable values *)
  | T.This when Env.function_is_mutable env ->
    f Arg_this
  | T.Callconv (Ast.Pinout, (_, T.Lvar id))
  | T.Lvar id when local_is_mutable ~include_borrowed:true env id ->
    f (Arg_local (snd id))
  | _ -> default

let check_borrowing
  (env : Typing_env.env)
  (p: 'a fun_param)
  (mut_args : args_mut_map)
  (e: T.expr): args_mut_map =
  let mut_to_string m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe mutable" in
  let check key =
    (* only check mutable expressions *)
    match Borrowable_args.find_opt key mut_args, p.fp_mutability with
    (* first time we see the parameter - just record it *)
    | None, _ -> Borrowable_args.add key (T.get_position e, p.fp_mutability) mut_args
    (* error case 1, expression was already passed as mutable parameter *)
    (* error case 2, expression was passed a maybe mutable parameter before and
       now is passed again as mutable *)
    | Some (pos, (Some Param_owned_mutable as mut)), _
    | Some (pos, (Some Param_borrowed_mutable as mut)), _
    | Some (pos, (Some Param_maybe_mutable as mut)), Some Param_borrowed_mutable ->
      Errors.mutable_expression_as_multiple_mutable_arguments
        (T.get_position e)(mut_to_string p.fp_mutability) pos (mut_to_string mut);
      mut_args
    | _ -> mut_args in

  with_mutable_value env e ~default:mut_args ~f:check

(* Checks that each parameter that is marked mutable is mutable *)
(* There's no List.iter2_shortest so I'm stuck with this *)
(* Return the remaining expressions to check against the variadic argument *)
let rec check_param_mutability (env : Typing_env.env)
  (mut_args : args_mut_map)
  (params : 'a fun_params ) (el : T.expr list): args_mut_map * T.expr list  =
  match params, el with
  | [], _
  | _, [] -> mut_args, el
  | param::ps, e::es ->
    (* maybe mutable parameters allow anything *)
    if param.fp_mutability <> Some Param_maybe_mutable
    then Env.error_if_reactive_context env @@ begin fun () ->
      begin match param.fp_mutability with
      (* maybe-mutable argument value *)
      | _ when expr_is_maybe_mutable env e ->
        Errors.maybe_mutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | Some Param_owned_mutable when
        not (expr_is_valid_owned_arg e) ->
        (*  __OwnedMutable requires argument to be
            - Rx\mutable for all expressions except variable expressions
            - Rx\move for variable expression *)
        let arg_is_owned_local = is_owned_local env e in
        Errors.mutably_owned_argument_mismatch
          ~arg_is_owned_local
          (param.fp_pos)
          (T.get_position e)
      | Some Param_borrowed_mutable when
        not (expr_is_valid_borrowed_arg env e) ->
      (* mutable parameter, immutable argument *)
        Errors.mutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | None when expr_is_valid_borrowed_arg env e ->
      (* immutable parameter, mutable argument *)
        Errors.immutable_argument_mismatch
          (param.fp_pos)
          (T.get_position e)
      | _ -> ()
      end
    end;
    let mut_args = check_borrowing env param mut_args e in
    (* Check the rest *)
    check_param_mutability env mut_args ps es

let check_mutability_fun_params env mut_args fty el =
  (* exit early if when calling non-reactive function *)
  if fty.ft_reactive = Nonreactive then ()
  else
  let params = fty.ft_params in
  let mut_args, remaining_exprs = check_param_mutability env mut_args params el in
  let rec error_on_first_mismatched_argument ~req_mut param es =
    match es with
    | [] -> ()
    | e::es ->
      if expr_is_maybe_mutable env e then
        Errors.maybe_mutable_argument_mismatch (param.fp_pos) (T.get_position e)
      else begin
        match req_mut with
        (* non mutable parameter - disallow anythin mutable *)
        | None when expr_is_valid_borrowed_arg env e ->
          Errors.immutable_argument_mismatch (param.fp_pos) (T.get_position e)
        | Some Param_borrowed_mutable when not (expr_is_valid_borrowed_arg env e) ->
        (* mutably borrowed parameter - complain on immutable or mutably owned parameters.
          mutably owned are not allowed because Rx\move will unset the original local *)
          Errors.mutable_argument_mismatch (param.fp_pos) (T.get_position e)
        | Some Param_owned_mutable when not (expr_is_valid_owned_arg e) ->
        (* mutably owned parameter - all arguments need to be passed with Rx\move *)
          Errors.mutably_owned_argument_mismatch
            ~arg_is_owned_local:(is_owned_local env e)
            (param.fp_pos)
            (T.get_position e)
        | _ ->
          error_on_first_mismatched_argument ~req_mut param es
      end
  in
  Env.error_if_reactive_context env @@ begin fun () ->
    begin match fty.ft_arity with
    (* maybe mutable variadic parameter *)
    | Fvariadic (_, ({ fp_mutability = Some Param_maybe_mutable; _ })) ->
      ()
    | Fvariadic (_, ({ fp_mutability = req_mut; _ } as param)) ->
      error_on_first_mismatched_argument ~req_mut param remaining_exprs
    | _ -> ()
    end;
    begin match fty.ft_arity with
    | Fvariadic (_, p) ->
      List.fold_left ~init:mut_args ~f:(check_borrowing env p) remaining_exprs
      |> ignore
    | _ -> ()
    end
  end
let enforce_mutable_constructor_call env ctor_fty el =
  match ctor_fty with
  | _, Tfun fty ->
    check_mutability_fun_params env Borrowable_args.empty fty el
  | _ -> ()

let enforce_mutable_call (env : Typing_env.env) (te : T.expr) =
  match snd te with
  | T.Call (_, (_, T.Id (_, s as id)), _, el, _)
  | T.Call (_, (_, T.Fun_id (_, s as id)), _, el, _)
    when s <> SN.Rx.move && s <> SN.Rx.freeze ->
    begin match Env.get_fun env (snd id) with
    | Some fty ->
      check_mutability_fun_params env Borrowable_args.empty fty el
    | None -> ()
    end
  (* static methods *)
  | T.Call (_, ((_, (_, Tfun fty)), T.Class_const _), _, el, _)
  | T.Call (_, ((_, (_, Tfun fty)), T.Lvar _), _, el, _) ->
    check_mutability_fun_params env Borrowable_args.empty fty el
  (* $x->method() where method is mutable *)
  | T.Call (_, ((pos, (r, Tfun fty)), T.Obj_get (expr, _, _)), _, el, _) ->
    (* do not check receiver mutability when calling non-reactive function *)
    if fty.ft_reactive <> Nonreactive
    then begin Env.error_if_reactive_context env @@ begin fun () ->
      let fpos = Reason.to_pos r in
      (* OwnedMutable annotation is not allowed on methods so
         we ignore it here since it already syntax error *)
      match fty.ft_mutability with
      (* mutable-or-immutable function - ok *)
      | Some Param_maybe_mutable -> ()
      (* mutable call on mutable-or-immutable value - error *)
      | Some Param_borrowed_mutable when expr_is_maybe_mutable env expr ->
        Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:true pos fpos
      (* non-mutable call on mutable-or-immutable value - error *)
      | None when expr_is_maybe_mutable env expr ->
        Errors.invalid_call_on_maybe_mutable ~fun_is_mutable:false pos fpos
      (* mutable call on immutable value - error *)
      | Some Param_borrowed_mutable when not (expr_is_valid_borrowed_arg env expr) ->
        Errors.mutable_call_on_immutable fpos pos
      (* immutable call on mutable value - error *)
      | None when expr_is_valid_borrowed_arg env expr ->
        Errors.immutable_call_on_mutable fpos pos
      (* anything else - ok *)
      | _ -> ()
    end;
    (* record mutability for the receiver *)
    let mut_args =
      with_mutable_value env expr ~default:Borrowable_args.empty
      ~f:(fun k -> Borrowable_args.singleton k (T.get_position expr, fty.ft_mutability)) in
    check_mutability_fun_params env mut_args fty el
    end
  (* TAny, T.Calls that don't have types, etc *)
  | _ -> ()

let rec is_byval_collection_or_string_type env ty =
  let check t =
    match t with
    | (_, Tclass ((_, x), _, _)) ->
      x = SN.Collections.cVec ||
      x = SN.Collections.cDict ||
      x = SN.Collections.cKeyset
    | _, (Tarraykind _ | Ttuple _ | Tshape _)
      -> true
    | _, Tprim Nast.Tstring -> true
    | _, Tunresolved tl -> List.for_all tl ~f:(is_byval_collection_or_string_type env)
    | _ -> false in
  let _, tl = Typing_utils.get_all_supertypes env ty in
  List.for_all tl ~f:check

let rec is_valid_mutable_subscript_expression_target env v =
  match v with
  | (_, ty), T.Lvar _ ->
    is_byval_collection_or_string_type env ty
  | (_, ty), T.Array_get (e, _) ->
    is_byval_collection_or_string_type env ty &&
    is_valid_mutable_subscript_expression_target env e
  | (_, ty), T.Obj_get (e, _, _) ->
    is_byval_collection_or_string_type env ty &&
    (is_valid_mutable_subscript_expression_target env e || expr_is_valid_borrowed_arg env e)
  | _ -> false

let check_assignment_or_unset_target
  (env : Typing_env.env) (te1 : T.expr) err =
  Env.error_if_reactive_context env @@ begin fun () ->
    (* Check for modifying immutable objects *)
    match snd te1 with
     (* Setting mutable locals is okay *)
    | T.Obj_get (e1, _, _) when expr_is_valid_borrowed_arg env e1 -> ()
    | T.Array_get (e1, _)
      when expr_is_valid_borrowed_arg env e1 ||
           is_valid_mutable_subscript_expression_target env e1 -> ()
    | T.Class_get _
    | T.Obj_get _
    | T.Array_get _ -> err (T.get_position te1)
    | _ -> ()
  end

let check_unset_target
  (env : Typing_env.env) (te : T.expr): unit =
  check_assignment_or_unset_target env te Errors.invalid_unset_target_rx

let rec is_move_or_mutable_call ?(allow_move=true) te =
  match te with
  | T.Call(_, (_, T.Id (_, n)), _, _, _) ->
      n = SN.Rx.mutable_ || (allow_move && n = SN.Rx.move)
  | T.Pipe (_, _, (_, r)) -> is_move_or_mutable_call ~allow_move:false r
  | _ -> false

let check_conditional_operator
  (when_true : T.expr)
  (when_false : T.expr) =
  match is_move_or_mutable_call (snd when_true), is_move_or_mutable_call (snd when_false) with
  | true, true | false, false -> ()
  | true, _ ->
    Errors.inconsistent_mutability_for_conditional
      (T.get_position when_true)
      (T.get_position when_false)
  | false, _ ->
    Errors.inconsistent_mutability_for_conditional
      (T.get_position when_false)
      (T.get_position when_true)

(* Checks for assignment errors as a pass on the TAST *)
let handle_assignment_mutability
 (env : Typing_env.env) (te1 : T.expr) (te2 : T.expr_ option)
 : Typing_env.env =
 check_assignment_or_unset_target env te1 Errors.obj_set_reactive;
 (* If e2 is a mutable expression, then e1 is added to the mutability env *)
 let mut_env = Env.get_env_mutability env in
 let mut_env = match snd te1, te2 with
 | _, Some T.This when Env.function_is_mutable env ->
 (* aliasing $this - bad for __Mutable functions *)
   Env.error_if_reactive_context env @@ begin fun () ->
     Errors.reassign_mutable_this (T.get_position te1)
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
      | _, MaybeMutable -> Errors.reassign_maybe_mutable_var p
      | _ -> Errors.reassign_mutable_var p
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
