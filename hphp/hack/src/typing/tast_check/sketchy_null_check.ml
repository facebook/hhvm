(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast_defs
open Tast
open Typing_defs

module Env = Tast_env
module SN = Naming_special_names

(* Return {Some} when {ty} contains falsey values. *)
let rec find_sketchy_type env ty =
  (* Find sketchy nulls hidden under Tunresolved *)
  let env, ty = Env.fold_unresolved env ty in
  match snd ty with
  | Tnonnull
  | Tabstract (AKenum _, _)
  | Tarraykind _ -> Some false

  | Tclass ((_, cid), _, _) when (
      (* Hack arrays *)
      cid = SN.Collections.cVec ||
      cid = SN.Collections.cDict ||
      cid = SN.Collections.cKeyset ||
      (* Concrete collection classes *)
      cid = SN.Collections.cVector ||
      cid = SN.Collections.cImmVector ||
      cid = SN.Collections.cSet ||
      cid = SN.Collections.cImmSet ||
      cid = SN.Collections.cMap ||
      cid = SN.Collections.cStableMap ||
      cid = SN.Collections.cImmMap ||
      (* Interfaces *)
      cid = SN.Collections.cConstVector ||
      cid = SN.Collections.cConstMap
    )
    -> Some false

  | Tprim Tbool -> Some true
  | Tprim Tresource -> None
  | Tprim _ -> Some true

  | Tunresolved tyl -> List.find_map tyl (find_sketchy_type env)
  | Tabstract _ ->
    let env, tyl = Env.get_concrete_supertypes env ty in
    List.find_map tyl (find_sketchy_type env)
  | _ -> None

let get_lvar_name = function
  | Lvar (_, id) -> Some (Local_id.get_name id)
  | _ -> None

let sketchy_null_check env ((p, ty), e) kind =
  if Env.is_strict env
  then begin
    let env, ty = Env.fold_unresolved env ty in
    let env, ety = Env.push_option_out env ty in
    match snd ety with
    | Toption ty ->
      find_sketchy_type env ty
      |> Option.iter ~f:begin fun is_prim ->
        let name = get_lvar_name e in
        if is_prim
        then Errors.sketchy_null_check_primitive p name kind
        else Errors.sketchy_null_check p name kind
      end
    | _ -> ()
  end

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env x =
    match snd x with
    | Eif (e, None, _) -> sketchy_null_check env e `Coalesce
    | Unop (Unot, e)
    | Binop (Eqeq, (_, Null), e)
    | Binop (Eqeq, e, (_, Null)) -> sketchy_null_check env e `Eq
    | Eif (e, Some _, _)
    | Assert (AE_assert e) -> sketchy_null_check env e `Neq
    | Binop (Ampamp, e1, e2)
    | Binop (Barbar, e1, e2) ->
      sketchy_null_check env e1 `Neq;
      sketchy_null_check env e2 `Neq;
    | _ -> ()

  method! at_stmt env x =
    match snd x with
    | If (e, _, _)
    | Do (_, e)
    | While (e, _)
    | For (_, e, _, _) -> sketchy_null_check env e `Neq
    | _ -> ()
end
