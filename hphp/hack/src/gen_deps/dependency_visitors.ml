(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 (* This file defines a visitor on the AST which calculates dependencies
  * between toplevel entities in Hack. It grabs toplevel entities from the
  * global naming table and adds the corresponding dependencies.
  *)
open Utils
open Ast


type toplevel =
| Class of string
| Function of string
| Typedef of string
| Const of string
| Other


type dep_env = {
  popt: ParserOptions.t;
  (* Current toplevel entity we are in *)
  top : toplevel;
}


let default_env popt = {
  popt;
  top = Other;
}

let toplevel_to_string = function
| Class s -> "class "^(strip_ns s)
| Function f -> "function "^(strip_ns f)
| Typedef t -> "typedef "^(strip_ns t)
| Const c -> "const "^(strip_ns c)
| Other -> "*UNKNOWN*"


class dependency_visitor = object
  inherit [_] Ast_visitors_iter.iter as super
  method! on_fun_ dep_env f =
    let dep_env = {
      dep_env with
      top = Function (snd f.f_name);
    } in
    super#on_fun_ dep_env f

  method! on_class_ dep_env c =
    let dep_env = {
      dep_env with
      top = Class (snd c.c_name);
    } in
    super#on_class_ dep_env c

  method! on_typedef dep_env t =
  let dep_env = {
    dep_env with
    top = Typedef (snd t.t_id);
  } in
  super#on_typedef dep_env t

  method! on_gconst dep_env c =
  let dep_env = {
    dep_env with
    top = Const (snd c.cst_name);
  } in
  super#on_gconst dep_env c

  method! on_hint dep_env hint =
  (* Check if the hint refers to a class of some sort,
    and then add a dependency if it's the case *)
    (match (snd hint) with
    | Happly ((_, ty_name), _)
    (* Only care about the base class's name *)
    | Haccess ((_, ty_name), _, _) ->
      (* For now, just print out any types we find. We'll add deps later *)
      let top_ent = toplevel_to_string dep_env.top in
      let pos = Pos.string (Pos.to_absolute (fst hint)) in
      Printf.printf "In %s, %s \n" top_ent pos;
      Printf.printf "Found type: %s\n" ty_name
    (* No need to recurse here, since the parent class does it for us  *)
    | Hshape _
    | Hoption _
    | Hsoft _
    | Hfun _
    | Htuple _ -> ());
    super#on_hint dep_env hint

end

let print_deps popt ast =
  let env = default_env popt in
  let _ = (new dependency_visitor)#on_program env ast in
  ()
