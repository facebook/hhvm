(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Decl_defs
open Shallow_decl_defs
open Typing_defs


(* Module calculating the Member Resolution Order of a class *)
type result = linearization

let get_linearization (env : Decl_env.env)
                      (class_name : string)
                      (type_params : decl ty list)
                      (new_source : source_type ): result =
  let result = match Decl_env.get_class_dep env class_name with
  | Some l -> l.Decl_defs.dc_linearization
  | None -> [] in
  (* Fill in the type parameterization of the starting class *)
  let result = match result with
  | c::rest ->
    { c with mro_params = type_params }::rest
  | [] -> [] in
  List.map result (fun c -> { c with mro_source = new_source })

let add_linearization (acc : result) (lin : result) : result =
  List.fold_left lin ~init:acc ~f:(fun acc e ->
    if List.mem acc e ~equal:(=) then acc
    else e::acc
  )

let from_class (env : Decl_env.env) (ty : decl ty)  (new_source : source_type): result =
  let _, (_, class_name), type_params = Decl_utils.unwrap_class_type ty in
  get_linearization env class_name type_params new_source


let from_list (env : Decl_env.env) (l : decl ty list)
              (acc : result) (source : source_type): result =
  List.fold_left l ~init:acc
    ~f:(fun acc ty -> add_linearization acc (from_class env ty source))

let from_parent (env : Decl_env.env) (c : shallow_class) (acc : result) : result =
  let extends =
    (* In an abstract class or a trait, we assume the interfaces
     * will be implemented in the future, so we take them as
     * part of the class (as requested by dependency injection implementers)
     *)
    match c.sc_kind with
      | Ast.Cabstract -> c.sc_implements @ c.sc_extends
      | Ast.Ctrait -> c.sc_implements @ c.sc_extends @ c.sc_req_implements
      | _ -> c.sc_extends
  in
  from_list env extends acc Parent

(* Linearize a class declaration given its shallow declaration *)
let linearize (env : Decl_env.env) (c : shallow_class) : result =
  let mro_name = snd c.sc_name in
  (* The first class doesn't have its type parameters filled in *)
  let child = { mro_name; mro_params = []; mro_source = Child; } in
  let acc = add_linearization [] [child] in
  (* Add traits in backwards order *)
  let acc = from_list env (List.rev c.sc_uses) acc Trait in
  (* Add interfaces(interfaces can define constants)
  TODO(jjwu): implemented interfaces are *only* important for constants and
  otherwise don't need to take up so much space in the linearization.
  Can we get rid of this somehow? *)
  let acc = from_list env c.sc_implements acc Interface in
  let acc = from_list env c.sc_req_implements acc ReqImpl in (* Same with req_implements *)
  (* Add requirements *)
  let acc = from_list env c.sc_req_extends acc ReqExtends in
  let acc = from_list env c.sc_xhp_attr_uses acc XHPAttr in
  let result = from_parent env c acc in
  List.rev result
