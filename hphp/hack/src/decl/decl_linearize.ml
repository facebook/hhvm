(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Nast
(* Module calculating the Member Resolution Order of a class *)
type result = Decl_defs.linearization

let get_linearization (env : Decl_env.env) (class_name : string) : result =
  match Decl_env.get_class_dep env class_name with
  | Some l -> l.Decl_defs.dc_linearization
  | None -> []

let add_linearization (acc : result) (lin : result) : result =
  List.fold_left lin ~init:acc ~f:(fun acc e ->
    if List.mem acc e ~equal:(=) then acc
    else e::acc
  )

let from_class (env : Decl_env.env) (hint : Nast.hint) : result =
  let _, class_name, _ = Decl_utils.unwrap_class_hint hint in
  get_linearization env class_name

let from_parent (env : Decl_env.env) (c : Nast.class_) (acc : result) : result =
  let extends =
    (* In an abstract class or a trait, we assume the interfaces
     * will be implemented in the future, so we take them as
     * part of the class (as requested by dependency injection implementers)
     *)
    match c.c_kind with
      | Ast.Cabstract -> c.c_implements @ c.c_extends
      | Ast.Ctrait -> c.c_implements @ c.c_extends @ c.c_req_implements
      | _ -> c.c_extends
  in
  List.fold_left extends ~init:acc
    ~f:(fun acc hint -> add_linearization acc (from_class env hint))

(* Linearize a class declaration given its nast *)
let linearize (env : Decl_env.env) (c : Nast.class_) : result =
  let child = snd c.c_name in
  let acc = add_linearization [] [child] in
  let result = from_parent env c acc in
  List.rev result
