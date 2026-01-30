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

(** Information about a loaded package in the current environment *)
type loaded_package_info = {
  pos: Pos.t;
  status: local_package_requirement;
  from_includes: bool;
}

(** Each package maps to its loaded package info *)
type t = loaded_package_info SMap.t

(* logging and pretty-printing *)
let show lp =
  SMap.fold
    (fun key { pos = _; status; from_includes } acc ->
      Printf.sprintf
        "%s%s: %s%s; "
        acc
        key
        (show_local_package_requirement status)
        (if from_includes then
          " (from includes)"
        else
          ""))
    lp
    ""

let as_log_value lp =
  Typing_log_value.(
    make_map
      (List.map
         (SMap.elements lp)
         ~f:(fun (key, { pos = _; status; from_includes }) ->
           ( key,
             Typing_log_value.string_as_value
               (Printf.sprintf
                  "%s%s"
                  (show_local_package_requirement status)
                  (if from_includes then
                    " (from includes)"
                  else
                    "")) ))))

(* typing_local_packages.t algebra *)
let empty = SMap.empty

let join lp1 lp2 =
  SMap.merge
    (fun _key op1 op2 ->
      match (op1, op2) with
      | (Some info1, Some info2) ->
        if equal_local_package_requirement info1.status info2.status then
          (* attempt to preserve the position of an explicit requirepackage/if package over includes *)
          match (info1.from_includes, info2.from_includes) with
          | (true, true)
          | (false, true)
          | (false, false) ->
            Some info1
          | (true, false) -> Some info2
        else
          None
      | (Some _, None) -> None
      | (None, Some _) -> None
      | (None, None) -> None)
    lp1
    lp2

(* Helper function for add: if pkg is updated with status Unsatisfiable_package_constraints,
 * then all the packages that include pkg must also be updated to status Unsatisfiable_package_constraints.
 * Uses the package_info from the parsed PACKAGES.toml to find which packages include pkg.
 *)
let update_unsatisfiable_packages package_info pkg pos lp =
  SMap.mapi
    (fun pkg_name info ->
      match PackageInfo.get_package package_info pkg_name with
      | None -> info
      | Some package ->
        let includes = List.map ~f:snd package.Package.includes in
        if List.exists includes ~f:(String.equal pkg) then
          { info with pos; status = Unsatisfiable_package_constraints }
        else
          info)
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
let add ~package_info pos pkg status lp =
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
          | Some { status = Not_exists_in_deployment; _ } -> true
          | Some { status = Unsatisfiable_package_constraints; _ } -> true
          | Some { status = Exists_in_deployment; _ } -> false
          | None -> false)
    in
    if has_conflict then
      (* If one is not_exists or unsatisfiable, pkg -> Unsatisfiable
         (no assumption on any of the other included packages) *)
      let lp = update_unsatisfiable_packages package_info pkg pos lp in
      SMap.add
        pkg
        {
          pos;
          status = Unsatisfiable_package_constraints;
          from_includes = false;
        }
        lp
    else
      (* If all are exists or not found, map includes ∪ pkg to Exists *)
      let lp =
        SMap.add
          pkg
          { pos; status = Exists_in_deployment; from_includes = false }
          lp
      in
      List.fold_left pkgs_included_by_pkg ~init:lp ~f:(fun lp pkg_name ->
          SMap.add
            ~combine:(fun old_value _new_value ->
              (* The old_value.status can only be Exists_in_deployment, otherwise
                 a conflict would have been detected.  For error reporting, preserve
                 the old_value from_includes and pos fields *)
              old_value)
            pkg_name
            { pos; status = Exists_in_deployment; from_includes = true }
            lp)
  | Not_exists_in_deployment ->
    (match SMap.find_opt pkg lp with
    | None
    | Some { status = Not_exists_in_deployment; _ } ->
      SMap.add
        pkg
        { pos; status = Not_exists_in_deployment; from_includes = false }
        lp
    | Some { status = Exists_in_deployment; _ }
    | Some { status = Unsatisfiable_package_constraints; _ } ->
      let lp = update_unsatisfiable_packages package_info pkg pos lp in
      SMap.add
        pkg
        {
          pos;
          status = Unsatisfiable_package_constraints;
          from_includes = false;
        }
        lp)
  | Unsatisfiable_package_constraints ->
    let lp = update_unsatisfiable_packages package_info pkg pos lp in
    SMap.add
      pkg
      { pos; status = Unsatisfiable_package_constraints; from_includes = false }
      lp

(* Assuming a conservative entailement of package requirements *)
let sub lp1 lp2 =
  SMap.equal
    (fun info1 info2 ->
      equal_local_package_requirement info1.status info2.status)
    lp1
    lp2
