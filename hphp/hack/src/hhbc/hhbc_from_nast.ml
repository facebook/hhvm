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

type fun_env = {
  acc       : H.instruct list;
}

let rec from_expr env expr =
  let _, e = expr in
  match e with
  | A.Call ((_, A.Id (_, id)), b, []) when id = "echo"-> (
    let sub_env = from_exprs { acc = [] } b in
    { acc = (H.IBasic H.PopC :: H.IOp H.Print :: sub_env.acc @ env.acc) }
  )
  | A.String (_, litstr) -> (
    { acc = H.ILitConst (H.String litstr) :: env.acc }
  )
  | _ -> env

and from_exprs env astl = List.fold_left from_expr env astl

and from_stmt env = function
  | A.Expr expr -> from_expr env expr
  | _ -> env

and from_stmts env astl = List.fold_left from_stmt env astl

let from_param p = {
  H.param_name = p.N.param_name;
}

let from_params xs = List.map from_param xs

let from_fun_ : Litstr.id -> Nast.fun_ -> Hhbc_ast.fun_def option =
  fun name_i nast_fun ->
  match nast_fun.N.f_body with
  | N.NamedBody _ -> None
  | N.UnnamedBody b ->
    let env = from_stmts { acc = [] } b.N.fub_ast in
    Option.some_if (env.acc <> [])
      { H.f_name          = name_i
      ; H.f_body          = List.rev
        (H.IContFlow H.RetC :: H.ILitConst H.Null :: env.acc)
      ; H.f_return_types  = from_params nast_fun.N.f_params;
      }
