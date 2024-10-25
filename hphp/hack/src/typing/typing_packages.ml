(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env

let get_module_pos env name =
  match Env.get_module env name with
  | Some m -> m.mdt_pos
  | None -> Pos_or_decl.none

(* If no module name is declared, treat as a default module *)
let declared_module_name_with_default module_opt =
  match module_opt with
  | None -> Naming_special_names.Modules.default
  | Some m -> m

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
  current_module_pos_or_filename:
    [ `ModulePos of Pos_or_decl.t | `FileName of Relative_path.t ];
  current_package_pos: Pos.t;
  current_package_name: string option;
  target_package_name: string option;
}

let is_calling_from_tests env =
  let current_file = Relative_path.to_absolute @@ Env.get_file env in
  try
    ignore (Str.search_forward (Str.regexp_string "__tests__") current_file 0);
    true
  with
  | _ -> false

(* Use the module names of current and target symbols to calculate their respective
 * packages, then determine if the current package is allowed to access the target
 * package.
 * If `package_v2` is set, use the file paths instead of modules to calculate the
 * package membership, optionally overridden by the __PackageOverride attribute.
 *)
let satisfies_pkg_rules env current_md target_md target_pos target_package :
    (package_error_info * Package.package_relationship) option =
  (* If `package_v2` is set in [env], we bypass package checks for test files to
     allow them to access any other package. *)
  if Env.package_v2 env && is_calling_from_tests env then
    None
  else
    let current_md_name = declared_module_name_with_default current_md in
    let target_md_name = declared_module_name_with_default target_md in
    (* Note, we haven't checked if these modules are actually defined:
       but they don't actually have to exist for their name to be used for package errors.
       If the module isn't defined, we'll still use the name to check package related errors
       as if it were defined, and use an empty position when needed.
    *)
    let current_file = Env.get_file env in
    let target_file = Pos_or_decl.filename target_pos in
    (* invariant: if two symbols are in the same file they must be in the same package *)
    let in_same_file =
      String.equal
        (Relative_path.suffix current_file)
        (Relative_path.suffix target_file)
    in
    let accessing_hhi = Pos_or_decl.is_hhi target_pos in
    if in_same_file || accessing_hhi then
      None
    else
      let (current_pkg, target_pkg) =
        if Env.package_v2 env then
          let current_pkg =
            match Env.get_current_package env with
            | Some pkg -> Env.get_package_by_name env pkg
            | None -> Env.get_package_for_file env current_file
          in
          let target_pkg =
            match target_package with
            | None -> Env.get_package_for_file env target_file
            | Some pkg -> Env.get_package_by_name env pkg
          in
          (current_pkg, target_pkg)
        else
          ( Env.get_package_for_module env current_md_name,
            Env.get_package_for_module env target_md_name )
      in
      match get_package_violation env current_pkg target_pkg with
      | None -> None
      | Some r ->
        (* Some package error, get the module pos *)
        let current_module_pos_or_filename =
          if Env.package_v2 env then
            `FileName (Env.get_file env)
          else
            `ModulePos (get_module_pos env current_md_name)
        in
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
              current_package_pos;
              current_package_name;
              target_package_name;
              current_module_pos_or_filename;
            },
            r )

let can_access
    ~(env : Typing_env_types.env)
    ~(current_module : string option)
    ~(target_module : string option)
    ~(target_package : string option)
    (target_pos : Pos_or_decl.t) =
  match
    satisfies_pkg_rules
      env
      current_module
      target_module
      target_pos
      target_package
  with
  | Some (err_info, Package.Soft_includes) -> `PackageSoftIncludes err_info
  | Some (err_info, Package.Unrelated) -> `PackageNotSatisfied err_info
  | Some (_, Package.(Equal | Includes)) ->
    Utils.assert_false_log_backtrace
      (Some "Package constraints are satisfied with equal and includes")
  | None -> `Yes
