(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* open Typing_defs *)
module Env = Typing_env

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
  current_package_pos: Pos.t;
  current_package_name: string option;
  current_package_assignment_kind: string;
  target_package_name: string option;
  target_package_pos: Pos.t;
  target_package_assignment_kind: string;
  target_id: string;
}

type check_reason =
  [ `Yes of string
  | `No
  ]

let is_excluded env (file : Relative_path.t) =
  let filename = Relative_path.to_absolute file in
  let excluded_patterns =
    Env.get_tcopt env |> TypecheckerOptions.package_exclude_patterns
  in
  List.exists excluded_patterns ~f:(fun pattern ->
      Str.(string_match (regexp pattern) filename 0))

(* package, package name, package memeberhip position, package membership kind *)
let get_package_profile
    (env : Typing_env_types.env)
    (pkg_membership : Aast_defs.package_membership option) :
    Package.t option * string option * Pos.t * string =
  match pkg_membership with
  | Some (Aast_defs.PackageConfigAssignment pkg_name) ->
    let pkg = Env.get_package_by_name env pkg_name in
    let pos =
      match pkg with
      | Some p -> Package.get_package_pos p
      | None -> Pos.none
    in
    (pkg, Some pkg_name, pos, "package config assignment")
  | Some (Aast_defs.PackageOverride (pkg_pos, pkg_name)) ->
    ( Env.get_package_by_name env pkg_name,
      Some pkg_name,
      pkg_pos,
      "package override" )
  | _ -> (None, None, Pos.none, "")

let can_access_by_package_rules
    ~(env : Typing_env_types.env)
    ~(target_package_membership : Aast_defs.package_membership option)
    ~(target_pos : Pos_or_decl.t)
    ~(target_id : string) =
  let current_file = Env.get_file env in
  let target_file = Pos_or_decl.filename target_pos in
  (* invariant: if two symbols are in the same file they must be in the same package *)
  let in_same_file =
    String.equal
      (Relative_path.suffix current_file)
      (Relative_path.suffix target_file)
  in
  let accessing_hhi = Pos_or_decl.is_hhi target_pos in
  if
    in_same_file
    || accessing_hhi
    || is_excluded env current_file
    || is_excluded env target_file
  then
    `Yes
  else
    let ( current_pkg,
          current_package_name,
          current_package_pos,
          current_package_assignment_kind ) =
      Env.get_current_package_membership env |> get_package_profile env
    in
    let ( target_pkg,
          target_package_name,
          target_package_pos,
          target_package_assignment_kind ) =
      get_package_profile env target_package_membership
    in
    match get_package_violation env current_pkg target_pkg with
    | None -> `Yes
    | Some pkg_relationship ->
      let err_info =
        {
          current_package_pos;
          current_package_name;
          current_package_assignment_kind;
          target_package_name;
          target_package_pos;
          target_package_assignment_kind;
          target_id;
        }
      in
      (match pkg_relationship with
      | Package.Soft_includes -> `PackageSoftIncludes err_info
      | Package.Unrelated -> `PackageNotSatisfied err_info
      | Package.(Equal | Includes) ->
        Utils.assert_false_log_backtrace
          (Some "Package constraints are satisfied with equal and includes"))
