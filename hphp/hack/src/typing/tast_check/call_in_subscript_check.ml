(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Tast_env

let rec check_for_call_expr (_, p, e) =
  match e with
  | Aast.List es -> List.iter ~f:check_for_call_expr es
  | Aast.Array_get (recv, _) -> check_for_call_expr recv
  | Aast.Hole (_, _, _, Aast.Typing) ->
    (* This isn't a real function call, it's introduced via type inference, so
     * don't check it otherwise we'd error on any Hack error in an lval
     *)
    ()
  (* These `Hole`s, however, _are_ function calls. *)
  | Aast.Hole (_, _, _, (Aast.UnsafeCast _ | Aast.EnforcedCast _))
  | Aast.Call _ ->
    Errors.add_typing_error Typing_error.(primary @@ Primary.Call_lvalue p)
  (* Obj_get and Class_get are lvalues, but they're "shallow" lvalues. Their
   * left-hand and right-hand side are both rvalues, so we don't need to
   * recurse on them, we can just return.
   *)
  | Aast.Obj_get _
  | Aast.Class_get _ ->
    ()
  (* For this check it doesn't matter that these expressions aren't lvalues,
   * only that there's no way there could be an lval hiding inside them (that
   * isn't picked up by the visitor ).
   *)
  | Aast.Darray _
  | Aast.Varray _
  | Aast.Shape _
  | Aast.ValCollection _
  | Aast.KeyValCollection _
  | Aast.Null
  | Aast.This
  | Aast.True
  | Aast.False
  | Aast.Omitted
  | Aast.Id _
  | Aast.Lvar _
  | Aast.Dollardollar _
  | Aast.Clone _
  | Aast.Class_const _
  | Aast.FunctionPointer _
  | Aast.Int _
  | Aast.Float _
  | Aast.String _
  | Aast.String2 _
  | Aast.PrefixedString _
  | Aast.Yield _
  | Aast.Await _
  | Aast.ReadonlyExpr _
  | Aast.Tuple _
  | Aast.Cast _
  | Aast.Unop _
  | Aast.Binop _
  | Aast.Pipe _
  | Aast.Eif _
  | Aast.Is _
  | Aast.As _
  | Aast.Upcast _
  | Aast.New _
  | Aast.Efun _
  | Aast.Lfun _
  | Aast.Xml _
  | Aast.Import _
  | Aast.Collection _
  | Aast.ExpressionTree _
  | Aast.Lplaceholder _
  | Aast.Fun_id _
  | Aast.Method_id _
  | Aast.Method_caller _
  | Aast.Smethod_id _
  | Aast.Pair _
  | Aast.ET_Splice _
  | Aast.EnumClassLabel _ ->
    ()

let check_foreach_lval = function
  | Aast.As_v e
  | Aast.Await_as_v (_, e) ->
    check_for_call_expr e
  | Aast.As_kv (k, v)
  | Aast.Await_as_kv (_, k, v) ->
    check_for_call_expr k;
    check_for_call_expr v

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt _ (_, s) =
      match s with
      | Aast.Foreach (_, as_expr, _) -> check_foreach_lval as_expr
      | _ -> ()

    method! at_expr _ (_, _, e) =
      match e with
      | Aast.Binop (Ast_defs.Eq _, lhs, _) -> check_for_call_expr lhs
      (* This currently doesn't handle two other known lval cases:
       * - `unset` statements
       * - increment / decrement operators
       *)
      | _ -> ()
  end
