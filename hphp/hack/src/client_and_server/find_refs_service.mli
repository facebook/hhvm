(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The class containing the member can be specified in two ways:
 * - Class_set - as an explicit, pre-computed set of names, which are then
 *   compared using string comparison
 * - Subclasses_of - the class's name, in which comparison will use the
 *   subtyping relation
 *)
type member_class =
  | Class_set of S_set.t
  | Subclasses_of of string

type action_internal =
  | IClass of string
  | IExplicitClass of string
  | IMember of member_class * Server_command_types.Find_refs.member
  | IFunction of string
  | IGConst of string

val find_refs_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  target:action_internal ->
  Search_types.Find_refs.t list

val find_references :
  Provider_context.t ->
  Multi_worker.worker list option ->
  action_internal ->
  bool ->
  Relative_path.t list ->
  deadline:float option ->
  stream_file:Path.t option ->
  Search_types.Find_refs.t list

val find_references_single_file :
  Provider_context.t ->
  action_internal ->
  Relative_path.t ->
  Search_types.Find_refs.t list

val find_child_classes_in_files :
  Provider_context.t ->
  string ->
  Naming_table.t ->
  Relative_path.Set.t ->
  S_set.t

val get_origin_class_name :
  Provider_context.t ->
  string ->
  Server_command_types.Find_refs.member ->
  string

val get_child_classes_files :
  Provider_context.t -> string -> Relative_path.Set.t

(** If [max_deps] is provided and the number of descendants or their dependents
    exceeds that value, we return Error. If no [max_deps] is provided, we
    always return Ok. *)
val get_files_for_descendants_and_dependents_of_members_in_descendants :
  Provider_context.t ->
  class_name:string ->
  max_deps:int option ->
  Typing_deps.Dep.Member.t list ->
  (Relative_path.Set.t * Relative_path.Set.t, unit) result

val get_dependent_files_function :
  Provider_context.t ->
  Multi_worker.worker list option ->
  string ->
  Relative_path.Set.t

val get_dependent_files_gconst :
  Provider_context.t ->
  Multi_worker.worker list option ->
  string ->
  Relative_path.Set.t

val get_dependent_files :
  Provider_context.t ->
  Multi_worker.worker list option ->
  S_set.t ->
  Relative_path.Set.t
