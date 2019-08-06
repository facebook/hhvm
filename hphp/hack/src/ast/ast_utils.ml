(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast

(* Utility functions for getting all nodes of a particular type *)
class ast_get_defs_visitor =
  object
    inherit [Ast.def list] Ast_visitor.ast_visitor

    method! on_def acc def = def :: acc
  end

let get_def_nodes ast = List.rev ((new ast_get_defs_visitor)#on_program [] ast)

class ast_get_class_elts_visitor =
  object
    inherit [Ast.class_elt list] Ast_visitor.ast_visitor

    method! on_class_elt acc elt = elt :: acc
  end

let get_class_elts ast =
  List.rev ((new ast_get_class_elts_visitor)#on_program [] ast)

(* Helpers for XHP attributes *)
let map_xhp_attr (f : id -> id) (g : expr -> expr) = function
  | Xhp_simple (id, e) -> Xhp_simple (f id, g e)
  | Xhp_spread e -> Xhp_spread (g e)
