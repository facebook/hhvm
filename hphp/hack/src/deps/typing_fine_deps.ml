(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type dependent_member =
  | Method of string
  | SMethod of string

type dependency = Typing_deps.Dep.dependency Typing_deps.Dep.variant

type coarse_dependent = Typing_deps.Dep.dependent Typing_deps.Dep.variant

(** [Typing_deps.Dep.dependency Typing_deps.Dep.variant] contains additionally
  * constructors that we don't need here.
  * Also, the fact that we use it for dependents rather than dependencies may
  * be confusing. Therefore we keep the type abstract in the interface *)
type fine_dependent = Typing_deps.Dep.dependency Typing_deps.Dep.variant

(** For debugging: When enabled, should record exactly the same dependencies as
  * the original/coarse grained dependency tracking implemented in
  * [Typing_deps] *)
let record_coarse_only = false

let add_fine_dep _mode _fine_dependent _dependency =
  failwith "not implemented, yet"

let add_coarse_dep mode coarse_dep =
  add_fine_dep mode (Typing_deps.Dep.dependency_of_variant coarse_dep)

let fine_dependent_of_coarse_and_child :
    coarse_dependent -> dependent_member option -> fine_dependent =
 fun coarse child ->
  let child =
    if record_coarse_only then
      None
    else
      child
  in
  match (coarse, child) with
  | (root, None) -> Typing_deps.Dep.dependency_of_variant root
  | (Typing_deps.Dep.Type t, Some (Method m)) -> Typing_deps.Dep.Method (t, m)
  | (Typing_deps.Dep.Type t, Some (SMethod m)) -> Typing_deps.Dep.SMethod (t, m)
  | (_, Some _) ->
    failwith
      "Only types/classes can have children for the purposes of dependency tracking!"

let try_add_fine_dep mode coarse child dependency =
  let child =
    if record_coarse_only then
      None
    else
      child
  in
  match (coarse, child) with
  | (None, None) -> ()
  | (None, Some _) -> failwith "Cannot have child dependent without root"
  | (Some root, child_opt) ->
    let dependent = fine_dependent_of_coarse_and_child root child_opt in
    add_fine_dep mode dependent dependency
