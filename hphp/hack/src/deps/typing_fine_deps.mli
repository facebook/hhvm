(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A dependency is something (e.g., a method) that some other piece of code
  * depends on (e.g., due to calling it) *)
type dependency = Typing_deps.Dep.dependency Typing_deps.Dep.variant

(** A dependent as used by the original, "coarse" dependency graph mechanism *)
type coarse_dependent = Typing_deps.Dep.dependent Typing_deps.Dep.variant

(** A dependent offering more granularity than [coarse_dependent] *)
type fine_dependent

(** A [dependent_member] augments a [coarse_dependent], yielding a
  * [fine_dependent]. It denotes a child/member of the toplevel
  * definition represented by [coarse_dependent] *)
type dependent_member =
  | Method of string
  | SMethod of string

val fine_dependent_of_coarse_and_child :
  coarse_dependent -> dependent_member option -> fine_dependent

val add_fine_dep : Typing_deps_mode.t -> fine_dependent -> dependency -> unit

val add_coarse_dep :
  Typing_deps_mode.t -> coarse_dependent -> dependency -> unit

val try_add_fine_dep :
  Typing_deps_mode.t ->
  coarse_dependent option ->
  dependent_member option ->
  dependency ->
  unit
