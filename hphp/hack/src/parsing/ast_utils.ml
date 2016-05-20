(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(* Given a Ast.program, give me the list of entities it defines *)
let get_defs ast =
  (* fold_right traverses the file from top to bottom, and as such gives nicer
   * error messages than fold_left. E.g. in the case where a function is
   * declared twice in the same file, the error will say that the declaration
   * with the larger line number is a duplicate. *)
  List.fold_right ast ~init:([], [], [], [])
    ~f:begin fun def (acc1, acc2, acc3, acc4 as acc) ->
      match def with
      | Ast.Fun f -> f.Ast.f_name :: acc1, acc2, acc3, acc4
      | Ast.Class c -> acc1, c.Ast.c_name :: acc2, acc3, acc4
      | Ast.Typedef t -> acc1, acc2, t.Ast.t_id :: acc3, acc4
      | Ast.Constant cst -> acc1, acc2, acc3, cst.Ast.cst_name :: acc4
      (* namespace nodes will never appear as long as elaborate_namespaces is
       * set to true when the parser is invoked *)
      | Ast.Namespace _
      | Ast.NamespaceUse _ -> assert false
       (* toplevel statements are ignored *)
      | Ast.Stmt _ -> acc
    end

let class_elt_type_to_string = function
  | Ast.Const _ -> "const"
  | Ast.AbsConst _ -> "absConst"
  | Ast.Attributes _ -> "attributes"
  | Ast.TypeConst _ -> "typeConst"
  | Ast.ClassUse _ -> "classUse"
  | Ast.XhpAttrUse _ -> "xhpAttrUse"
  | Ast.ClassTraitRequire _ -> "classTraitRequire"
  | Ast.ClassVars _ -> "classVars"
  | Ast.XhpAttr _ -> "xhpAttr"
  | Ast.Method _ -> "method"
  | Ast.XhpCategory _ -> "xhpCategory"

(* Utility functions for getting all nodes of a particular type *)
class ast_get_defs_visitor = object (this)
  inherit [Ast.def list] Ast_visitor.ast_visitor

  method! on_def acc def =
    def :: acc
end

let get_def_nodes ast =
  List.rev ((new ast_get_defs_visitor)#on_program [] ast)

let get_classes ast =
  List.filter_map (get_def_nodes ast) ~f:(function
    | Ast.Class c -> Some c
    | _ -> None
  )

class ast_get_class_elts_visitor = object (this)
  inherit [Ast.class_elt list] Ast_visitor.ast_visitor

  method! on_class_elt acc elt =
    elt :: acc
end

let get_class_elts ast =
  List.rev ((new ast_get_class_elts_visitor)#on_program [] ast)

let get_methods ast =
  List.filter_map (get_class_elts ast) ~f:(function
    | Ast.Method m -> Some m
    | _ -> None
  )

let get_typeConsts ast =
  List.filter_map (get_class_elts ast) ~f:(function
    | Ast.TypeConst tc -> Some tc
    | _ -> None
  )

let get_classUses ast =
  List.filter_map (get_class_elts ast) ~f:(function
    | Ast.ClassUse h -> Some h
    | _ -> None
  )
