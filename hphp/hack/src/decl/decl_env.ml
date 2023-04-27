(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  mode: FileInfo.mode;
  droot: Typing_deps.Dep.dependent Typing_deps.Dep.variant option;
  droot_member: Typing_pessimisation_deps.dependent_member option;
  ctx: Provider_context.t;
}

let root_decl_reference env =
  env.droot |> Option.map ~f:Typing_deps.Dep.to_decl_reference

let make_decl_pos env pos =
  (* TODO: fail if root_decl_reference returns None *)
  Pos_or_decl.make_decl_pos_of_option pos (root_decl_reference env)

let make_decl_posed env posed =
  (* TODO: fail if root_decl_reference returns None *)
  Positioned.make_for_decl_of_option posed (root_decl_reference env)

let tcopt env = Provider_context.get_tcopt env.ctx

type class_cache = Decl_store.class_entries SMap.t

let no_fallback (_ : env) (_ : string) : Decl_defs.decl_class_type option = None

let get_class_and_add_dep
    ~(cache : class_cache) ~(shmem_fallback : bool) ~fallback env x =
  let res =
    match SMap.find_opt x cache with
    | Some c -> Some (fst c)
    | None when shmem_fallback -> Decl_store.((get ()).get_class x)
    | None -> None
  in
  match res with
  | Some c -> Some c
  | None -> fallback env x
