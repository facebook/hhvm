(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SN = Naming_special_names
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
  | pos, (Array _ | Darray _ | Varray _ | Shape _ | Collection _ | Record _
    | Null | True | False | Id _ | Clone _
    | Class_const _ | Call _ | Int _ | Float _ | PrefixedString _
    | String _ | String2 _ | Yield _ | Yield_break | Yield_from _
    | Await _ | Suspend _ | Expr_list _ | Cast _ | Unop _
    | Binop _ | Eif _ | InstanceOf _
    | New _ | Efun _ | Lfun _
    | Xml _ | Import _ | Pipe _ | Callconv _ | Is _ | As _
    | ParenthesizedExpr _ | PU_identifier _) ->
      errorf pos "Invalid lvalue"

(** Syntax errors detected via a pass over AST (see: check_program)
 * TODO: move here most of CST checks (module Full_fidelity_syntax_errors)
 *)

type context = {
  is_hh_file : bool;
  in_callable : bool;
  active_methodish_kind : kind list option;
  active_classish : class_ option;
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

(* Methods shared by multiple (groups of) checks, ported from full_fidelity_parser_errors.ml *)

let has_static kind_lst =
  List.exists (function Static -> true | _ -> false) kind_lst

(* Returns the whether the current context is in an active class scope *)
let is_in_active_class_scope context =
  context.active_classish <> None


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

let reducer_this_usage = object(this)
  inherit [_, _] Ast_visitor.reducer
    (fun () -> ESet.empty)
    ESet.union

  method private name_eq_this_and_in_static_method ctx name =
    match ctx.active_methodish_kind with
    | Some kind_lst when has_static kind_lst &&
      is_in_active_class_scope ctx &&
      String.lowercase_ascii name = SN.SpecialIdents.this
    -> true
    | _ -> false

  method! at_Call ctx (_, e_) _ _ _ =
    match e_ with
    | Class_const ((pos, Id (_, name)), _pstr) when
      this#name_eq_this_and_in_static_method ctx name
    ->
      ESet.singleton @@ mk_error pos SyntaxError.this_in_static
    | _ -> ESet.empty

  method! at_Lvar ctx (pos, name) =
    if ctx.is_hh_file && this#name_eq_this_and_in_static_method ctx name then
      ESet.singleton @@ mk_error pos SyntaxError.this_in_static
    else ESet.empty
end

let check_program program ~(is_hh_file : bool) =
  (* This is analogous to the iter_with but with reducers instead of handlers: *)
  let reducers : (ESet.t, context) Ast_visitor.reducer_type list =
    [ reducer_await_toplevel
    ; reducer_this_usage
    ] in
  let visitor = object(this)
    inherit [_] Ast.reduce as super

    method zero = ESet.empty
    method plus = ESet.union

    method private to_fold_fun map_fun =
      fun acc x -> this#plus acc (map_fun x)

    method private reduce map_fun =
      List.fold_left (this#to_fold_fun map_fun) this#zero reducers

    method! on_Call ctx e targl argl uargl = this#plus
      (super#on_Call ctx e targl argl uargl)
      (this#reduce (fun r -> r#at_Call ctx e targl argl uargl))

    method! on_class_ ctx class_ =
      super#on_class_ { ctx with
        active_classish = Some class_;
      } class_

    method! on_expr ctx e = this#plus
      (super#on_expr ctx e)
      (this#reduce (fun r -> r#at_expr ctx e))

    method! on_fun_ ctx =
      super#on_fun_
        { ctx with in_callable = true }

    method! on_Lvar ctx lvar = this#plus
      (super#on_Lvar ctx lvar)
      (this#reduce (fun r -> r#at_Lvar ctx lvar))

    method! on_method_ ctx method_ =
      super#on_method_ { ctx with
        in_callable = true;
        active_methodish_kind = Some method_.m_kind;
      } method_
  end
  in ESet.elements @@ visitor#on_program
    { is_hh_file = is_hh_file;
      in_callable = false;
      active_methodish_kind = None;
      active_classish = None;
    } program
