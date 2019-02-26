(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error

open Ast

let rec check_lvalue errorf = function
  | pos, Obj_get (_, (_, Id (_, _)), OG_nullsafe) ->
    errorf pos "?-> syntax is not supported for lvalues"
  | pos, Obj_get (_, (_, Id (_, name)), _) when name.[0] = ':' ->
    errorf pos "->: syntax is not supported for lvalues"
  | pos, Array_get ((_, Class_const _), _) ->
    errorf pos "Array-like class consts are not valid lvalues"
  | _, (PU_atom _ | Lvar _ | Obj_get _ | Array_get _ | Class_get _ |
    Unsafeexpr _ | Omitted | BracedExpr _) -> ()
  | pos, Call ((_, Id (_, "tuple")), _, _, _) ->
    errorf pos "Tuple cannot be used as an lvalue. Maybe you meant list?"
  | _, List el -> List.iter (check_lvalue errorf) el
  | pos, (Array _ | Darray _ | Varray _ | Shape _ | Collection _
    | Null | True | False | Id _ | Clone _
    | Class_const _ | Call _ | Int _ | Float _ | PrefixedString _
    | String _ | String2 _ | Yield _ | Yield_break | Yield_from _
    | Await _ | Suspend _ | Expr_list _ | Cast _ | Unop _
    | Binop _ | Eif _ | InstanceOf _
    | New _ | NewAnonClass _ | Efun _ | Lfun _
    | Xml _ | Import _ | Pipe _ | Callconv _ | Is _ | Execution_operator _ | As _
    | ParenthesizedExpr _) ->
      errorf pos "Invalid lvalue"


(** Syntax errors detected via a pass over AST (see: check_program)
 * TODO: move here most of CST checks (module Full_fidelity_syntax_errors)
 *)

type context = {
  _unused : unit;  (* TODO: dummy for easier rebasing until we have 2+ fields *)
}

let _mk_error ?(error_type=SyntaxError.ParseError) pos error =
  let (start_offset, end_offset) = Pos.info_raw pos in
  SyntaxError.make ~error_type start_offset end_offset error

let check_program program =
  let visitor = object(_)
    inherit [context * (SyntaxError.t list)] Ast_visitor.ast_visitor as _super

    (* TODO: override on_* and modify context and errors accordingly for each check *)
  end
  in snd @@ visitor#on_program (
    {
      _unused = ();
    }, []
    ) program
