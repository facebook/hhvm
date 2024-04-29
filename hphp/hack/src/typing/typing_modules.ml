(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env
module Cls = Folded_class

(* If no module name is declared, treat as a default module *)
let declared_module_name_with_default module_opt =
  match module_opt with
  | None -> Naming_special_names.Modules.default
  | Some m -> m

let can_access_internal
    ~(env : Typing_env_types.env)
    ~(current : string option)
    ~(target : string option) =
  match (current, target) with
  | (None, None)
  | (Some _, None) ->
    `Yes
  | (None, Some m) -> `Outside m
  | (Some m_current, Some m_target) when String.equal m_current m_target ->
    (match Env.get_self_id env with
    | None -> `Yes
    | Some self ->
      (match Env.get_class env self with
      | Decl_entry.Found cls
        when Ast_defs.is_c_trait (Cls.kind cls)
             && (not (Cls.internal cls))
             && not (Cls.is_module_level_trait cls) ->
        `OutsideViaTrait (Cls.pos cls)
      | Decl_entry.Found _
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        `Yes))
  | (Some current, Some target) -> `Disjoint (current, target)

let get_module_pos env name =
  match Env.get_module env name with
  | Some m -> m.mdt_pos
  | None -> Pos_or_decl.none

let get_package_violation env current_pkg target_pkg =
  match (current_pkg, target_pkg) with
  | (_, None) -> None
  | (None, Some target_pkg_info) ->
    if Env.is_package_loaded env (Package.get_package_name target_pkg_info) then
      None
    else
      Some Package.Unrelated
  (* Anyone can call code outside of a package *)
  | (Some current_pkg_info, Some target_pkg_info) ->
    Package.(
      (match relationship current_pkg_info target_pkg_info with
      | Equal
      | Includes ->
        None
      | (Soft_includes | Unrelated) as r ->
        if Env.is_package_loaded env (get_package_name target_pkg_info) then
          None
        else
          Some r))

type package_error_info = {
  current_module_pos: Pos_or_decl.t;
  current_package_pos: Pos.t;
  current_package_name: string option;
  target_package_name: string option;
}

let satisfies_pkg_rules env current target =
  let current_name = declared_module_name_with_default current in
  let target_name = declared_module_name_with_default target in
  (* Note, we haven't checked if these modules are actually defined:
     but they don't actually have to exist for their name to be used for package errors.
     If the module isn't defined, we'll still use the name to check package related errors
     as if it were defined, and use an empty position when needed.
  *)
  let current_pkg = Env.get_package_for_module env current_name in
  let target_pkg = Env.get_package_for_module env target_name in
  match get_package_violation env current_pkg target_pkg with
  | None -> None
  | Some r ->
    (* Some package error, get the module pos *)
    let current_module_pos = get_module_pos env current_name in
    let (current_package_pos, current_package_name) =
      match current_pkg with
      | Some pkg ->
        (Package.get_package_pos pkg, Some (Package.get_package_name pkg))
      | None -> (Pos.none, None)
    in
    let target_package_name =
      Option.map ~f:Package.get_package_name target_pkg
    in
    Some
      ( {
          current_module_pos;
          current_package_pos;
          current_package_name;
          target_package_name;
        },
        r )

let can_access_public
    ~(env : Typing_env_types.env)
    ~(current : string option)
    ~(target : string option) =
  if Option.equal String.equal current target then
    `Yes
  else
    match satisfies_pkg_rules env current target with
    | Some (err_info, Package.Soft_includes) -> `PackageSoftIncludes err_info
    | Some (err_info, Package.Unrelated) -> `PackageNotSatisfied err_info
    | Some (_, Package.(Equal | Includes)) ->
      Utils.assert_false_log_backtrace
        (Some "Package constraints are satisfied with equal and includes")
    | None -> `Yes

let is_class_visible (env : Typing_env_types.env) (cls : Cls.t) =
  if Cls.internal cls then
    match
      can_access_internal
        ~env
        ~current:(Env.get_current_module env)
        ~target:(Cls.get_module cls)
    with
    | `Yes -> true
    | _ -> false
  else
    true
