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
    | Xml _ | Import _ | Pipe _ | Callconv _ | Is _ | As _
    | ParenthesizedExpr _) ->
      errorf pos "Invalid lvalue"


(** Syntax errors detected via a pass over AST (see: check_program)
 * TODO: move here most of CST checks (module Full_fidelity_syntax_errors)
 *)

type context = {
  in_callable : bool;
  _unused : unit;  (* TODO: dummy for easier rebasing until we have 2+ fields *)
}

let mk_error ?(error_type=SyntaxError.ParseError) pos error =
  let (start_offset, end_offset) = Pos.info_raw pos in
  SyntaxError.make ~error_type start_offset end_offset error

module ESet = Set.Make(
  struct
    let compare = SyntaxError.compare
    type error_t = SyntaxError.t
    type t = error_t
  end
)

(* TODO: put complicated reducers into separate files *)
let reducer_await_toplevel = object(_)
  inherit [_, _] Ast_visitor.reducer
    (fun () -> ESet.empty)
    ESet.union

  method! at_expr ctx (pos, e) =
    match e with
    | Await _ when not ctx.in_callable ->
      ESet.singleton @@ mk_error pos SyntaxError.toplevel_await_use
    | _ -> ESet.empty
end

let check_program program =
  (* This is analogous to the iter_with but with reducers instead of handlers: *)
  let reducers : (ESet.t, context) Ast_visitor.reducer_type list =
    [ reducer_await_toplevel
    ] in
  let visitor = object(this)
    inherit [_] Ast.reduce as super

    method zero = ESet.empty
    method plus = ESet.union

    method private to_fold_fun map_fun =
      fun acc x -> this#plus acc (map_fun x)

    method private reduce map_fun =
      List.fold_left (this#to_fold_fun map_fun) this#zero reducers

    method! on_expr ctx e = this#plus
      (super#on_expr ctx e)
      (this#reduce (fun r -> r#at_expr ctx e))

    method! on_fun_ ctx =
      super#on_fun_
        { ctx with in_callable = true }

    method! on_method_ ctx =
      super#on_method_
        { ctx with in_callable = true }
  end
  in ESet.elements @@ visitor#on_program
    {
      in_callable = false;
      _unused = ();
    } program
