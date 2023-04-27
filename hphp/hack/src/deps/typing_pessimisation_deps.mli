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

(** A dependent (= something with dependencies) as used by the original,
  * "coarse" dependency graph mechanism *)
type coarse_dependent = Typing_deps.Dep.dependent Typing_deps.Dep.variant

(** A [dependent_member] augments a [coarse_dependent]. It denotes a
  * child/member of the toplevel definition represented by
  * [coarse_dependent] *)
type dependent_member =
  | Constructor
  | Method of string
  | SMethod of string

val add_coarse_dep :
  Typing_deps_mode.t -> coarse_dependent -> dependency -> unit

val try_add_fine_dep :
  Typing_deps_mode.t ->
  coarse_dependent option ->
  dependent_member option ->
  dependency ->
  unit

val add_override_dep :
  Typing_deps_mode.t ->
  child_name:string ->
  parent_name:string ->
  dependent_member ->
  unit

(** Informs the pessimisation dependency graph about the existence of a node.
  This allows ensuring that certain nodes are added to the the graph even if
  they may not have any incoming or outgoing edges. *)
val add_node :
  Typing_deps_mode.t -> coarse_dependent -> dependent_member option -> unit

(** Persists all currently cached dependencies to disk *)
val finalize : Typing_deps_mode.t -> unit

module SQLitePersistence : sig
  (** A glob pattern for the file names used by the per-worker output files.
   A star is used in place of the worker id *)
  val worker_file_name_glob : string
end
