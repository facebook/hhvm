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

type package_warning_info = {
  current_package: Package.pos_id option;
  target_package: Package.pos_id option;
  target_package_before_override: string option;
}

type package_error_info = {
  current_package: Package.pos_id option;
  current_package_assignment_kind: string;
  target_package: Package.pos_id option;
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

(* package, package info (name * pos), package membership kind *)
let get_package_profile
    (env : Typing_env_types.env)
    (pkg_membership : Aast_defs.package_membership option) :
    Package.t option * Package.pos_id option * string =
  match pkg_membership with
  | Some (Aast_defs.PackageConfigAssignment pkg_name) ->
    let pkg = Env.get_package_by_name env pkg_name in
    let pos =
      match pkg with
      | Some p -> Package.get_package_pos p
      | None -> Pos.none
    in
    (pkg, Some (pos, pkg_name), "package definition")
  | Some (Aast_defs.PackageOverride (pkg_pos, pkg_name)) ->
    ( Env.get_package_by_name env pkg_name,
      Some (pkg_pos, pkg_name),
      "package override" )
  | _ -> (None, None, "")

let package_includes current_pkg target_pkg =
  match (current_pkg, target_pkg) with
  | (Some current_pkg, Some target_pkg) ->
    (match Package.relationship current_pkg target_pkg with
    | Equal
    | Includes ->
      true
    | Soft_includes
    | Unrelated ->
      false)
  | _ -> false

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
    let current_package_membership = Env.get_current_package_membership env in
    let (current_pkg, current_package, current_package_assignment_kind) =
      get_package_profile env current_package_membership
    in
    let (target_pkg, target_package, target_package_assignment_kind) =
      get_package_profile env target_package_membership
    in
    match get_package_violation env current_pkg target_pkg with
    | None ->
      (* There are no package errrors, but emit a warning if the edge introduces a new
       * dependency a file in a package due to package rules and a file with a packageoverride *)
      (match (current_package_membership, target_package_membership) with
      | ( Some (Aast_defs.PackageConfigAssignment _),
          Some (Aast_defs.PackageOverride _) )
        when package_includes current_pkg target_pkg ->
        let target_package_before_override =
          Option.map
            (Package_info.get_package_for_file
               (Env.get_tcopt env |> TypecheckerOptions.package_info)
               (Relative_path.suffix target_file))
            ~f:Package.get_package_name
        in
        let is_target_package_before_override_loaded =
          match target_package_before_override with
          | Some pkg_name -> Env.is_package_loaded env pkg_name
          | None -> true
        in
        let is_target_package_before_override_soft_required =
          match
            ( Env.get_soft_package_requirement env,
              target_package_before_override )
          with
          | (Some soft_required_package_name, Some pkg_name) ->
            let soft_required_package =
              Env.get_package_by_name env (snd soft_required_package_name)
            in
            let target_package_before_override =
              Env.get_package_by_name env pkg_name
            in
            package_includes
              soft_required_package
              target_package_before_override
          | (Some _, None) -> true
          | _ -> false
        in
        if
          is_target_package_before_override_loaded
          || is_target_package_before_override_soft_required
        then
          `Yes
        else
          let warn_info =
            { current_package; target_package; target_package_before_override }
          in
          `YesWarning warn_info
      | _ -> `Yes)
    | Some pkg_relationship ->
      let err_info =
        {
          current_package;
          current_package_assignment_kind;
          target_package;
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
