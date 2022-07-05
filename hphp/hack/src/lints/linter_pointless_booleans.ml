(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let which_boolean_literal ((_ty, _pos, expr_) : Tast.expr) =
  match expr_ with
  | Aast.True -> Some Aast.True
  | Aast.False -> Some Aast.False
  | _ -> None

let is_boolean_expression ((ty, _pos, _expr_) : Tast.expr) =
  Typing_defs.is_prim Aast.Tbool ty

let checking_the_expression exp1 exp2 =
  match which_boolean_literal exp1 with
  | Some x when is_boolean_expression exp2 -> Some x
  | _ -> None

let check_the_error logical_op boolean_literal =
  match logical_op with
  | Ast_defs.Ampamp ->
    (match boolean_literal with
    | Aast.True -> Some "The boolean expression is pointless"
    | Aast.False -> Some "The boolean expression is always false"
    | _ -> None)
  | Ast_defs.Barbar ->
    (match boolean_literal with
    | Aast.True -> Some "The boolean expression is always true"
    | Aast.False -> Some "The boolean expression is pointless"
    | _ -> None)
  | _ -> None

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr _env (_, pos, expr_) =
      match expr_ with
      | Aast.Binop (op, exp1, exp2) ->
        (match op with
        | Ast_defs.Ampamp
        | Ast_defs.Barbar ->
          (match checking_the_expression exp1 exp2 with
          | Some x ->
            (match check_the_error op x with
            | Some str -> Lints_errors.calling_pointless_boolean pos str
            | None -> ())
          | None ->
            (match checking_the_expression exp2 exp1 with
            | Some x ->
              (match check_the_error op x with
              | Some str -> Lints_errors.calling_pointless_boolean pos str
              | None -> ())
            | None -> ()))
        | _ -> ())
      | _ -> ()
  end
