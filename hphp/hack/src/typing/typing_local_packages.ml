(*
 * Copyright (c) 2025, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type local_package_requirement =
  | Exists_in_deployment
  | Not_exists_in_deployment
  | Unsatisfiable_package_constraints
[@@deriving eq, show]

type t = local_package_requirement SMap.t

(* logging and pretty-printing *)
let show lp =
  SMap.fold
    (fun key status acc ->
      Printf.sprintf
        "%s%s: %s; "
        acc
        key
        (show_local_package_requirement status))
    lp
    ""

let as_log_value lp =
  Typing_log_value.(
    make_map
      (List.map (SMap.elements lp) ~f:(fun (key, status) ->
           ( key,
             Typing_log_value.string_as_value
               (show_local_package_requirement status) ))))

(* typing_local_packages.t algebra *)
let empty = SMap.empty

let join lp1 lp2 =
  SMap.merge
    (fun _key op1 op2 ->
      match (op1, op2) with
      | (Some p1, Some p2) ->
        if equal_local_package_requirement p1 p2 then
          Some p1
        else
          None
      | (Some _p1, None) -> None
      | (None, Some _p2) -> None
      | (None, None) -> None)
    lp1
    lp2

(* Helper function for add: if pkg is updated with status Unsatisfiable_package_constraints,
 * then all the packages that include pkg must also be updated to status Unsatisfiable_package_constraints.
 * Uses the package_info from the parsed PACKAGES.toml to find which packages include pkg.
 *)
let update_unsatisfiable_packages package_info pkg lp =
  SMap.mapi
    (fun pkg_name status ->
      match PackageInfo.get_package package_info pkg_name with
      | None -> status
      | Some package ->
        let includes = List.map ~f:snd package.Package.includes in
        if List.exists includes ~f:(String.equal pkg) then
          Unsatisfiable_package_constraints
        else
          status)
    lp

(* The loaded_package algebra is complicated by the fact that packages can include
 * other packages: if A includes B and A is deployed, then B is guaranteed to be
 * deployed too.  This impacts the semantics of adding `pkg` with `status` to loaded_packages:
 * - if the status of pkg is Exists_in_deployment,
 *   then the status of all the included packages of pkg is first checked:
 *   - if all of them are either assumed to already exists, or have no assumptions made,
 *     then pkg and all its included packages are added to the loaded_packages with status
 *     Exists_in_deployment;
 *   - if any of them as status Not_exists_in_deployment or Unsatisfiable_in_deployment,
 *     then the pkg is assumed to be Unsatisfiable_in_deployment and no other assumptions
 *     are made on its included packages.
 * - if the status of pkg is Not_exists_in_deployment,
 *   then the only mapping pkg -> Not_exists_in_deployment is added to loaded_packages,
 *   as no assumptions can be made about the packages included by pkg.
 * - whenever conflicting assumptions are made on a package, then the status of the
 *   package, and of _all_ the packages that include it, is set to Unsatisfiable_package_constraints
 *
 * Whenever package A includes package B, this code maintains two invariants on the local_package environment:
 * - if A -> Exists_in_deployment then B -> Exists_in_deployment;
 * - if B -> Unsatisfiable_package_constraints then A -> Unsatisfiable_package_constraints
 *)
let add ~package_info pkg status lp =
  match status with
  | Exists_in_deployment ->
    let pkgs_included_by_pkg =
      match PackageInfo.get_package package_info pkg with
      | None -> []
      | Some pkg -> List.map ~f:snd pkg.Package.includes
    in
    let all_pkgs = pkg :: pkgs_included_by_pkg in
    let has_conflict =
      List.exists all_pkgs ~f:(fun pkg_name ->
          match SMap.find_opt pkg_name lp with
          | Some Not_exists_in_deployment -> true
          | Some Unsatisfiable_package_constraints -> true
          | Some Exists_in_deployment -> false
          | None -> false)
    in
    if has_conflict then
      (* If one is not_exists or unsatisfiable, pkg -> Unsatisfiable
         (no assumption on any of the other included packages) *)
      let lp = update_unsatisfiable_packages package_info pkg lp in
      SMap.add pkg Unsatisfiable_package_constraints lp
    else
      (* If all are exists or not found, map includes ∪ pkg to Exists *)
      List.fold_left all_pkgs ~init:lp ~f:(fun lp pkg_name ->
          SMap.add pkg_name Exists_in_deployment lp)
  | Not_exists_in_deployment ->
    (match SMap.find_opt pkg lp with
    | None
    | Some Not_exists_in_deployment ->
      SMap.add pkg Not_exists_in_deployment lp
    | Some Exists_in_deployment
    | Some Unsatisfiable_package_constraints ->
      let lp = update_unsatisfiable_packages package_info pkg lp in
      SMap.add pkg Unsatisfiable_package_constraints lp)
  | Unsatisfiable_package_constraints ->
    let lp = update_unsatisfiable_packages package_info pkg lp in
    SMap.add pkg Unsatisfiable_package_constraints lp

(* Assuming a conservative entailement of package requirements *)
let sub lp1 lp2 = SMap.equal equal_local_package_requirement lp1 lp2
