(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs
module MakeType = Typing_make_type

let is_awaitable env ty =
  let mixed = MakeType.mixed Typing_reason.none in
  let awaitable_of_mixed = MakeType.awaitable Typing_reason.none mixed in
  let _, is_ty_awaitable = Tast_env.subtype env ty awaitable_of_mixed in
  is_ty_awaitable

let rec can_be_null env ty =
  let _, (_, ety) = Tast_env.expand_type env ty in
  match ety with
  | Toption _ | Tprim Nast.Tnull -> true
  | Tunresolved tyl -> List.exists tyl (can_be_null env)
  | Terr | Tany | Tnonnull | Tarraykind _ | Tprim _ | Tvar _
    | Tfun _ | Tabstract _ | Tclass _ | Ttuple _
    | Tanon _ | Tobject | Tshape _ | Tdynamic -> false

let rec enforce_not_awaitable env p ty =
  let _, ety = Tast_env.expand_type env ty in
  match ety with
  | _, Tunresolved tyl ->
    List.iter tyl (enforce_not_awaitable env p)
  | r, Tclass ((_, awaitable), _, _) when
      awaitable = SN.Classes.cAwaitable ->
    Errors.discarded_awaitable p (Typing_reason.to_pos r)
  | _, (Terr | Tany | Tnonnull | Tarraykind _ | Tprim _ | Toption _
    | Tvar _ | Tfun _ | Tabstract _ | Tclass _ | Ttuple _
    | Tanon _ | Tobject | Tshape _ | Tdynamic) -> ()

let enforce_nullable_or_not_awaitable env p ty =
  if can_be_null env ty then ()
  else enforce_not_awaitable env p ty

type ctx =
  {
    (* Is a supertype of Awaitable<t> disallowed in a given context?
     *
     * E.g.: If true, Awaitable<t>, ?Awaitable<t>, and any union containing
     * Awaitable<t> are all disallowed.
     *)
    awaitable_disallowed : bool;

    (* Is a non-nullable supertype of Awaitable<t> disallowed in a given
     * context?
     *
     * E.g.: If true, Awaitable<t> is disallowed, but ?Awaitable<t> and any
     * union containing Awaitable<t> and an nullable type are allowed.
     *)
    non_nullable_awaitable_disallowed : bool;
  }

let allow_awaitable =
  {
    awaitable_disallowed = false;
    non_nullable_awaitable_disallowed = false
  }

let disallow_awaitable ctx =
  {ctx with awaitable_disallowed = true}

let disallow_non_nullable_awaitable ctx =
  {ctx with non_nullable_awaitable_disallowed = true}

let visitor = object(this)
  inherit [_] Tast_visitor.iter_with_state as super

  method! on_expr (env, ctx) ((p, ty), e as te) = match e with
    | Unop (Ast.Unot, e) | Assert (AE_assert e) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e
    | Binop (Ast.(Eqeq | Eqeqeq | Diff | Diff2 | Barbar | Ampamp | LogXor),
             e1, e2) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e1;
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e2
    | Binop (Ast.QuestionQuestion, e1, e2) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e1;
      this#on_expr (env, ctx) e2
    | Eif (e1, e2, e3) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e1;
      Option.iter e2 ~f:(this#on_expr (env, ctx));
      this#on_expr (env, ctx) e3
    | Cast (hint, e) ->
      this#on_hint (env, ctx) hint;
      this#on_expr (env, disallow_awaitable ctx) e
    | InstanceOf (e, ((_, obj_ty), _)) ->
      let ctx' =
        if is_awaitable env obj_ty
        then allow_awaitable
        else disallow_non_nullable_awaitable ctx in
      this#on_expr (env, ctx') e
    | _ ->
      if ctx.awaitable_disallowed then
        enforce_not_awaitable env p ty
      else if ctx.non_nullable_awaitable_disallowed then
        enforce_nullable_or_not_awaitable env p ty;
      super#on_expr (env, allow_awaitable) te

  method! on_stmt (env, ctx) stmt = match snd stmt with
    | Expr (_, Binop (Ast.Eq _, _, _) as e) ->
      this#on_expr (env, allow_awaitable) e
    | Expr e ->
      this#on_expr (env, disallow_awaitable ctx) e
    | If (e, b1, b2) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e;
      this#on_block (env, ctx) b1;
      this#on_block (env, ctx) b2
    | Do (b, e) ->
      this#on_block (env, ctx) b;
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e
    | While (e, b) ->
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e;
      this#on_block (env, ctx) b
    | For (e1, e2, e3, b) ->
      this#on_expr (env, ctx) e1;
      this#on_expr (env, disallow_non_nullable_awaitable ctx) e2;
      this#on_expr (env, ctx) e3;
      this#on_block (env, ctx) b
    | Switch (e, casel) ->
      this#on_expr (env, disallow_awaitable ctx) e;
      List.iter casel ~f:(this#on_case (env, ctx))
    | _ ->
      super#on_stmt (env, allow_awaitable) stmt

  method! on_block (env, _) block =
    super#on_block (env, allow_awaitable) block
end

let handler = object
  inherit Tast_visitor.handler_base

  method! at_fun_def env = visitor#on_fun_def (env, allow_awaitable)
  method! at_method_ env = visitor#on_method_ (env, allow_awaitable)
end
