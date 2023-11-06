(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module MakeType = Typing_make_type
module SN = Naming_special_names

let is_awaitable env ty =
  let mixed = MakeType.mixed Typing_reason.none in
  let awaitable_of_mixed = MakeType.awaitable Typing_reason.none mixed in
  Tast_env.can_subtype env ty awaitable_of_mixed

let can_be_null env ty =
  let null = MakeType.null Typing_reason.none in
  Tast_env.can_subtype env null ty

let rec enforce_not_awaitable env p ty =
  let (_, ety) = Tast_env.expand_type env ty in
  match get_node ety with
  | Tunion tyl
  | Tintersection tyl ->
    List.iter tyl ~f:(enforce_not_awaitable env p)
  | Tclass ((_, awaitable), _, _)
    when String.equal awaitable SN.Classes.cAwaitable ->
    Typing_error_utils.add_typing_error
      ~env:(Env.tast_env_as_typing_env env)
      Typing_error.(
        primary
        @@ Primary.Discarded_awaitable { pos = p; decl_pos = get_pos ety })
  | Toption ty' ->
    if
      TypecheckerOptions.disallow_discarded_nullable_awaitables
        (Env.get_tcopt env)
    then
      enforce_not_awaitable env p ty'
    else
      ()
  | Tany _
  | Tnonnull
  | Tvec_or_dict _
  | Tprim _
  | Tvar _
  | Tfun _
  | Tgeneric _
  | Tnewtype _
  | Tdependent _
  | Tclass _
  | Ttuple _
  | Tshape _
  | Tdynamic
  | Taccess _
  | Tneg _ ->
    ()
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

type ctx = {
  (* Is a supertype of ?Awaitable<t> allowed in a given
   * context?
   *
   * E.g.: If true, ?Awaitable<t> is disallowed, but Awaitable<t> is allowed.
   *)
  nullable_awaitable_disallowed: bool;
  (* Is a non-nullable supertype of Awaitable<t> disallowed in a given
   * context?
   *
   * E.g.: If true, Awaitable<t> is disallowed, but ?Awaitable<t> and any
   * union containing Awaitable<t> and an nullable type are allowed.
   *)
  non_nullable_awaitable_disallowed: bool;
}

let allow_awaitable =
  {
    nullable_awaitable_disallowed = false;
    non_nullable_awaitable_disallowed = false;
  }

let disallow_awaitable =
  {
    nullable_awaitable_disallowed = true;
    non_nullable_awaitable_disallowed = true;
  }

let disallow_due_to_cast ctx env =
  if
    TypecheckerOptions.disallow_discarded_nullable_awaitables
      (Env.get_tcopt env)
  then
    {
      nullable_awaitable_disallowed = true;
      non_nullable_awaitable_disallowed = true;
    }
  else
    { ctx with non_nullable_awaitable_disallowed = true }

let disallow_due_to_cast_with_explicit_nullcheck =
  {
    nullable_awaitable_disallowed = false;
    non_nullable_awaitable_disallowed = true;
  }

let disallow_due_to_cast_with_explicit_nullcheck_and_return_nonnull ctx =
  {
    nullable_awaitable_disallowed =
      ctx.nullable_awaitable_disallowed && ctx.non_nullable_awaitable_disallowed;
    non_nullable_awaitable_disallowed = true;
  }

let visitor =
  object (this)
    inherit [_] Tast_visitor.iter_with_state as super

    method! on_expr (env, ctx) ((ty, p, e) as te) =
      match e with
      | Await e -> this#on_expr (env, allow_awaitable) e
      | Unop (Ast_defs.Unot, e)
      | Binop { bop = Ast_defs.Eqeqeq; lhs = e; rhs = (_, _, Null) }
      | Binop { bop = Ast_defs.Eqeqeq; lhs = (_, _, Null); rhs = e }
      | Binop { bop = Ast_defs.Diff2; lhs = e; rhs = (_, _, Null) }
      | Binop { bop = Ast_defs.Diff2; lhs = (_, _, Null); rhs = e } ->
        this#on_expr (env, disallow_due_to_cast_with_explicit_nullcheck) e
      | Binop
          {
            bop = Ast_defs.(Eqeq | Eqeqeq | Diff | Diff2 | Barbar | Ampamp);
            lhs;
            rhs;
          } ->
        this#on_expr (env, disallow_due_to_cast ctx env) lhs;
        this#on_expr (env, disallow_due_to_cast ctx env) rhs
      | Binop { bop = Ast_defs.QuestionQuestion; lhs; rhs } ->
        this#on_expr
          ( env,
            disallow_due_to_cast_with_explicit_nullcheck_and_return_nonnull ctx
          )
          lhs;
        this#on_expr (env, ctx) rhs
      | Eif (e1, e2, e3) ->
        this#on_expr (env, disallow_due_to_cast ctx env) e1;
        Option.iter e2 ~f:(this#on_expr (env, ctx));
        this#on_expr (env, ctx) e3
      | Cast (hint, e) ->
        this#on_hint (env, ctx) hint;
        this#on_expr (env, disallow_awaitable) e
      | Is (e, (_, Hnonnull))
      | Is (e, (_, Hprim Tnull)) ->
        this#on_expr (env, disallow_due_to_cast_with_explicit_nullcheck) e
      | Is (e, hint)
      | As { expr = e; hint; is_nullable = _; enforce_deep = _ } ->
        let hint_ty = Env.hint_to_ty env hint in
        let (env, hint_ty) =
          Env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        let ctx' =
          if is_awaitable env hint_ty then
            allow_awaitable
          else
            disallow_due_to_cast ctx env
        in
        this#on_expr (env, ctx') e
      | _ ->
        if
          ctx.nullable_awaitable_disallowed
          || ctx.non_nullable_awaitable_disallowed
        then
          if
            if can_be_null env ty then
              ctx.nullable_awaitable_disallowed
            else
              ctx.non_nullable_awaitable_disallowed
          then
            enforce_not_awaitable env p ty;
        super#on_expr (env, allow_awaitable) te

    method! on_stmt (env, ctx) stmt =
      match snd stmt with
      | Expr ((_, _, Binop { bop = Ast_defs.Eq _; _ }) as e) ->
        this#on_expr (env, allow_awaitable) e
      | Expr e -> this#on_expr (env, disallow_awaitable) e
      | If (e, b1, b2) ->
        this#on_expr (env, disallow_due_to_cast ctx env) e;
        this#on_block (env, ctx) b1;
        this#on_block (env, ctx) b2
      | Do (b, e) ->
        this#on_block (env, ctx) b;
        this#on_expr (env, disallow_due_to_cast ctx env) e
      | While (e, b) ->
        this#on_expr (env, disallow_due_to_cast ctx env) e;
        this#on_block (env, ctx) b
      | For (e1, e2, e3, b) ->
        List.iter e1 ~f:(this#on_expr (env, ctx));
        Option.iter e2 ~f:(this#on_expr (env, disallow_due_to_cast ctx env));
        List.iter e3 ~f:(this#on_expr (env, ctx));
        this#on_block (env, ctx) b
      | Switch (e, casel, dfl) ->
        this#on_expr (env, disallow_awaitable) e;
        List.iter casel ~f:(this#on_case (env, ctx));
        Option.iter dfl ~f:(this#on_default_case (env, ctx))
      | _ -> super#on_stmt (env, allow_awaitable) stmt

    method! on_block (env, _) block =
      super#on_block (env, allow_awaitable) block
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env = visitor#on_fun_def (env, allow_awaitable)

    method! at_method_ env = visitor#on_method_ (env, allow_awaitable)
  end
