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

(* Given a Ast.program, give me the list of entities it defines *)
let get_defs ast =
  (* fold_right traverses the file from top to bottom, and as such gives nicer
   * error messages than fold_left. E.g. in the case where a function is
   * declared twice in the same file, the error will say that the declaration
   * with the larger line number is a duplicate. *)
  let rec get_defs ast acc =
  List.fold_right ast ~init:acc
    ~f:begin fun def (acc1, acc2, acc3, acc4 as acc) ->
      match def with
      | Ast.Fun f ->
        (FileInfo.pos_full f.Ast.f_name) :: acc1, acc2, acc3, acc4
      | Ast.Class c ->
        acc1, (FileInfo.pos_full c.Ast.c_name) :: acc2, acc3, acc4
      | Ast.Typedef t ->
        acc1, acc2, (FileInfo.pos_full t.Ast.t_id) :: acc3, acc4
      | Ast.Constant cst ->
        acc1, acc2, acc3, (FileInfo.pos_full cst.Ast.cst_name) :: acc4
      | Ast.Namespace(_, defs) ->
        get_defs defs acc
      | Ast.NamespaceUse _ | Ast.SetNamespaceEnv _ ->
        acc
       (* toplevel statements are ignored *)
      | Ast.Stmt _ -> acc
    end
in
  get_defs ast ([],[],[],[])

(* Utility functions for getting all nodes of a particular type *)
class ast_get_defs_visitor = object
  inherit [Ast.def list] Ast_visitor.ast_visitor

  method! on_def acc def =
    def :: acc
end

let ast_no_pos_mapper = object (self)
  inherit [_] Ast.endo
  method! private on_pos () _pos = Pos.none
  (* Skip all blocks because we don't care about method bodies *)
  method! on_block env _ = self#on_list self#on_stmt env [(Pos.none,Ast.Noop)]
end

(* Given an AST, return an AST with no position info *)
let remove_pos ast =
  ast_no_pos_mapper#on_program () ast


type ignore_attribute_env = {
  ignored_attributes: string list
}

let ast_deregister_attributes_mapper = object (self)
  inherit [_] Ast.endo as super

  method ignored_attr env l =
    List.exists l
      (fun attr -> List.mem (env.ignored_attributes) (snd attr.ua_name) ~equal:(=))

  (* Filter all functions and classes with the user attributes banned *)
  method! on_program env toplevels =
    let toplevels = List.filter toplevels (fun toplevel ->
      match toplevel with
      | Fun f when self#ignored_attr env f.f_user_attributes ->
        false
      | Class c when self#ignored_attr env c.c_user_attributes ->
        false
      | _ -> true
    ) in
    super#on_program env toplevels

  method! on_class_ env this =
    (* Filter out class elements which are methods with wrong attributes *)
    let body = this.c_body in
    let body = List.filter body (fun elt ->
      match elt with
      | Method m when self#ignored_attr env m.m_user_attributes -> false
      | ClassVars cv when self#ignored_attr env cv.cv_user_attributes -> false
      | _ -> true
    ) in
    let this = { this with c_body = body } in
    super#on_class_ env this
end

let deregister_ignored_attributes ast =
  let env = {
    (* For now, only ignore the __PHPStdLib *)
    ignored_attributes = [Naming_special_names.UserAttributes.uaPHPStdLib]
  } in
  (ast_deregister_attributes_mapper)#on_program env ast


(* Given an AST, generate a unique hash for its decl tree. *)
let generate_ast_decl_hash ast =
  (* Why we marshal it into a string first: regular Hashtbl.hash will
    collide improperly because it doesn't compare ADTs with strings correctly.
    Using Marshal, we guarantee that the two ASTs are represented by a single
    primitive type, which we hash.
  *)
  let str = Marshal.to_string (remove_pos ast) [] in
  OpaqueDigest.string str


let get_def_nodes ast =
  List.rev ((new ast_get_defs_visitor)#on_program [] ast)

class ast_get_class_elts_visitor = object
  inherit [Ast.class_elt list] Ast_visitor.ast_visitor

  method! on_class_elt acc elt =
    elt :: acc
end

let get_class_elts ast =
  List.rev ((new ast_get_class_elts_visitor)#on_program [] ast)

type break_continue_level =
  | Level_ok of int option
  | Level_non_literal
  | Level_non_positive

let get_break_continue_level level_opt =
  match level_opt with
  | None -> Level_ok None
  | Some (_, Ast.Int s) ->
    let i = int_of_string s in
    if i <= 0 then Level_non_positive
    else Level_ok (Some i)
  | _ -> Level_non_literal
  | exception _ -> Level_non_literal

(* Helpers for XHP attributes *)
let map_xhp_attr (f: id -> id) (g: expr -> expr) = function
  | Xhp_simple (id, e) -> Xhp_simple (f id, g e)
  | Xhp_spread e -> Xhp_spread (g e)
