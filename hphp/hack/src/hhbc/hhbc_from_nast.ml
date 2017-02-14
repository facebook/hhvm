(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module A = Ast
module N = Nast
module H = Hhbc_ast

(* The various from_X functions below take some kind of AST (expression,
 * statement, etc.) and produce what is logically a sequence of instructions.
 * This could simply be represented by a list, but then we would need to
 * use an accumulator to avoid the quadratic complexity associated with
 * repeated appending to a list. Instead, we simply build a tree of
 * instructions which can easily be flattened at the end.
 *)
type instr_seq =
| Instr_list of H.instruct list
| Instr_concat of instr_seq list

(* Some helper constructors *)
let instr x = Instr_list [x]
let instrs x = Instr_list x
let gather x = Instr_concat x
let empty = Instr_list []

let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  match op with
  (* TODO Consider overflow versions of instructions *)
  | A.Plus -> instr (H.IOp H.Add)
  | A.Minus -> instr (H.IOp H.Sub)
  | A.Star -> instr (H.IOp H.Mul)
  | A.Slash -> instr (H.IOp H.Div)
  | A.Eqeq -> instr (H.IOp H.Eq)
  | A.EQeqeq -> instr (H.IOp H.Same)
  | A.Starstar -> instr (H.IOp H.Pow)
  | A.Diff -> instr (H.IOp H.Neq)
  | A.Diff2 -> instr (H.IOp H.NSame)
  | A.AMpamp -> failwith "&& not strict"
  | A.BArbar -> failwith "|| not strict"
  | A.Lt -> instr (H.IOp H.Lt)
  | A.Lte -> instr (H.IOp H.Lte)
  | A.Gt -> instr (H.IOp H.Gt)
  | A.Gte -> instr (H.IOp H.Gte)
  | A.Dot -> instr (H.IOp H.Concat)
  | A.Amp -> instr (H.IOp H.BitAnd)
  | A.Bar -> instr (H.IOp H.BitOr)
  | A.Ltlt -> instr (H.IOp H.Shl)
  | A.Gtgt -> instr (H.IOp H.Shr)
  | A.Percent -> instr (H.IOp H.Mod)
  | A.Xor -> instr (H.IOp H.BitXor)
  | A.Eq _ -> failwith "= NYI"

let rec from_expr expr =
  match snd expr with
  | A.String (_, litstr) ->
    instr (H.ILitConst (H.String litstr))
  | A.Int (_, litstr) ->
    (* TODO deal with integer out of range *)
    instr (H.ILitConst (H.Int (Int64.of_string litstr)))
  | A.Null -> instr (H.ILitConst H.Null)
  | A.False -> instr (H.ILitConst H.False)
  | A.True -> instr (H.ILitConst H.True)
  | A.Binop(op, e1, e2) ->
    gather [from_expr e1; from_expr e2; from_binop op]
  | A.Call ((_, A.Id (_, id)), el, []) when id = "echo" ->
    gather [from_exprs el; instr (H.IOp H.Print); instr (H.IBasic H.PopC)]
  (* TODO: emit warning *)
  | _ -> empty

and from_exprs exprs =
  gather (List.map from_expr exprs)

and from_stmt st =
  match st with
  | A.Expr expr -> from_expr expr
  (* TODO: emit warning *)
  | _ -> empty

and from_stmts stl =
  gather (List.map from_stmt stl)

let from_param p = {
  H.param_name = p.N.param_name;
}

let from_params xs = List.map from_param xs

let from_fun_ : Litstr.id -> Nast.fun_ -> Hhbc_ast.fun_def option =
  fun name_i nast_fun ->
  match nast_fun.N.f_body with
  | N.NamedBody _ ->
    None
  | N.UnnamedBody b ->
    let stmt_instrs = from_stmts b.N.fub_ast in
    let body_instrs = gather [
      stmt_instrs;
      instr (H.ILitConst H.Null);
      instr (H.IContFlow H.RetC)
    ] in
    Some
    (
      { H.f_name          = name_i
      ; H.f_body          = instr_seq_to_list body_instrs
      ; H.f_return_types  = from_params nast_fun.N.f_params;
      }
    )
