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
  | Class_set of SSet.t
  | Subclasses_of of string

type action_internal =
  | IClass of string
  | IRecord of string
  | IMember of member_class * ServerCommandTypes.Find_refs.member
  | IFunction of string
  | IGConst of string

val find_refs_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  target:action_internal ->
  (string * Pos.t) list

val find_references :
  Provider_context.t ->
  MultiWorker.worker list option ->
  action_internal ->
  bool ->
  Relative_path.t list ->
  (string * Pos.t) list

val find_child_classes :
  Provider_context.t ->
  string ->
  Naming_table.t ->
  Relative_path.Set.t ->
  SSet.t

val get_origin_class_name :
  Provider_context.t -> string -> ServerCommandTypes.Find_refs.member -> string

val get_child_classes_files :
  Provider_context.t -> string -> Relative_path.Set.t

val get_dependent_files_function :
  Provider_context.t ->
  MultiWorker.worker list option ->
  string ->
  Relative_path.Set.t

val get_dependent_files_gconst :
  Provider_context.t ->
  MultiWorker.worker list option ->
  string ->
  Relative_path.Set.t

val get_dependent_files :
  Provider_context.t ->
  MultiWorker.worker list option ->
  SSet.t ->
  Relative_path.Set.t

val result_to_ide_message :
  ('a * string Pos.pos list) option ->
  ('a * Ide_api_types.file_range list) option
