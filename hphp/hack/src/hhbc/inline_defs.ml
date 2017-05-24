(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Core

let add_fun (fun_list, class_list) f = f::fun_list, class_list
let add_class (fun_list, class_list) c = fun_list, c::class_list

class def_inline_visitor = object
  inherit [Ast.fun_ list * Ast.class_ list] Ast_visitor.ast_visitor as _super

  method! on_def_inline acc def =
  let acc = match def with
    | Ast.Fun f -> add_fun acc f
    | Ast.Class c -> add_class acc c
    | _ -> acc
  in
  _super#on_def_inline acc def

end

let from_ast prog =
  let visitor = new def_inline_visitor in
  let inline_fun_defs, inline_class_defs = visitor#on_program ([], []) prog in
  List.rev inline_fun_defs, List.rev inline_class_defs
