(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
module Dep = Typing_deps.Dep

type env = {
  mode: FileInfo.mode;
  droot: Typing_deps.Dep.dependent Typing_deps.Dep.variant option;
  droot_member: Typing_fine_deps.dependent_member option;
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

let deps_mode env = Provider_context.get_deps_mode env.ctx

let is_hhi cd = Pos_or_decl.is_hhi cd.dc_pos

let add_wclass env x =
  let dep = Dep.Type x in
  Option.iter env.droot ~f:(fun root ->
      Typing_deps.add_idep (deps_mode env) root dep);
  if
    TypecheckerOptions.record_fine_grained_dependencies
    @@ Provider_context.get_tcopt env.ctx
  then
    Typing_fine_deps.try_add_fine_dep
      (deps_mode env)
      env.droot
      env.droot_member
      dep;
  ()

let add_extends_dependency env x =
  let deps_mode = deps_mode env in
  Option.iter env.droot ~f:(fun root ->
      let dep = Dep.Type x in
      Typing_deps.add_idep deps_mode root (Dep.Extends x);
      Typing_deps.add_idep deps_mode root dep);
  if
    TypecheckerOptions.record_fine_grained_dependencies
    @@ Provider_context.get_tcopt env.ctx
  then (
    Typing_fine_deps.try_add_fine_dep deps_mode env.droot None (Dep.Extends x);
    Typing_fine_deps.try_add_fine_dep deps_mode env.droot None (Dep.Type x)
  );
  ()

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
  let res =
    match res with
    | Some c -> Some c
    | None -> fallback env x
  in
  Option.iter res ~f:(fun cd ->
      if not (is_hhi cd) then add_extends_dependency env x);
  res

let get_construct env class_ =
  if not (is_hhi class_) then begin
    add_wclass env class_.dc_name;
    let dep = Dep.Constructor class_.dc_name in
    Option.iter env.droot ~f:(fun root ->
        Typing_deps.add_idep (deps_mode env) root dep);
    if
      TypecheckerOptions.record_fine_grained_dependencies
      @@ Provider_context.get_tcopt env.ctx
    then
      Typing_fine_deps.try_add_fine_dep
        (deps_mode env)
        env.droot
        env.droot_member
        dep
  end;

  class_.dc_construct
