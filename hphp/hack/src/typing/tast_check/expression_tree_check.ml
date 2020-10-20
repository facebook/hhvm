(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(**
 * Since all Hack types are truthy, typically, most syntactic places that
 * expect booleans allow all types. However, as to not leak these truthy
 * Hack semantics to Expression Trees, ensure that those syntactic positions
 * only accept booleans, rather than any truthy expression.
 *)
let enforce_boolean env ((pos, ty), _) =
  let open Typing_defs in
  let bool_ty = Typing_make_type.bool (Reason.Ret_boolean pos) in
  let _ =
    Tast_env.assert_subtype pos Reason.URnone env ty bool_ty Errors.unify_error
  in
  ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env (_, s) =
      if not (Tast_env.in_et env) then
        ()
      else
        match s with
        | While (e, _)
        | If (e, _, _)
        | For (_, Some e, _, _) ->
          enforce_boolean env e
        | _ -> ()

    method! at_expr env (_ty, e) =
      if not (Tast_env.in_et env) then
        ()
      else
        match e with
        | Binop ((Ast_defs.Ampamp | Ast_defs.Barbar), e1, e2) ->
          let () = enforce_boolean env e1 in
          enforce_boolean env e2
        | Unop (Ast_defs.Unot, e)
        | Eif (e, _, _) ->
          enforce_boolean env e
        | _ -> ()
  end
