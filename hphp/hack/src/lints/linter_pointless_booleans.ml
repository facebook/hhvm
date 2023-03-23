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

let send_to_lint_error pos str line1 line2 bol1 bol2 col1 col2 =
  let path_name = Pos.filename (Pos.to_absolute pos) in
  let pos_quickfix =
    Pos.make_from_lexing_pos
      (Pos.filename pos)
      Lexing.
        {
          pos_fname = path_name;
          pos_lnum = line1;
          pos_bol = bol1;
          pos_cnum = col1;
        }
      Lexing.
        {
          pos_fname = path_name;
          pos_lnum = line2;
          pos_bol = bol2;
          pos_cnum = col2;
        }
  in
  Lints_errors.calling_pointless_boolean pos pos_quickfix str

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr _env (_, pos, expr_) =
      match expr_ with
      | Aast.(Binop { bop; lhs; rhs }) ->
        (match bop with
        | Ast_defs.Ampamp
        | Ast_defs.Barbar ->
          (* exp1 is the boolean literal (it is of the type true ||, false && etc)*)
          (* The original string for quickfix starts from the
             start of exp1 and ends at the start of exp2 *)
          (match checking_the_expression lhs rhs with
          | Some x ->
            (match check_the_error bop x with
            | Some str ->
              (match (lhs, rhs) with
              | ((_, pos_exp1, _), (_, pos_exp2, _)) ->
                let (line1, bol1, start_col1) = Pos.line_beg_offset pos_exp1 in
                let (line2, bol2, start_col2) = Pos.line_beg_offset pos_exp2 in
                send_to_lint_error
                  pos
                  str
                  line1
                  line2
                  bol1
                  bol2
                  start_col1
                  start_col2)
            | None -> ())
          | None ->
            (* exp2 is the boolean literal (it is of the type && true, || false etc)*)
            (* The original string for quickfix starts from the
               end of exp1 and ends at the end of exp2 *)
            (match checking_the_expression rhs lhs with
            | Some x ->
              (match check_the_error bop x with
              | Some str ->
                (match (lhs, rhs) with
                | ((_, pos_exp1, _), (_, pos_exp2, _)) ->
                  let (line1, bol1, end_col1) =
                    Pos.end_line_beg_offset pos_exp1
                  in
                  let (line2, bol2, end_col2) =
                    Pos.end_line_beg_offset pos_exp2
                  in
                  send_to_lint_error
                    pos
                    str
                    line1
                    line2
                    bol1
                    bol2
                    end_col1
                    end_col2)
              | None -> ())
            | None -> ()))
        | _ -> ())
      | _ -> ()
  end
