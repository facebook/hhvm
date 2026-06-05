(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t [@@deriving show, eq]

val empty : t

val log_package_info : t -> unit

val from_packages : Package.t list -> t

val get_package : t -> string -> Package.t option

val package_exists : t -> string -> bool

(** The get_package_for_file function returns the package a file path belongs
  * without taking into account eventual __PackageOverride annotations.
  *)
val get_package_for_file :
  support_multifile_tests:bool -> t -> path:string -> Package.t option

(** The get_package_with_override function returns the package a file path belongs
  * taking into account __PackageOverride annotations.  It requires not only the file path
  * but also the file content.
  * 
  * DO NOT USE: this function scans the content of the file and is __very inefficient__.
  * It should be called ONLY from services that cannot access decls, notably the 
  * Glean indexer and the redundant PackageOverride linter, and NEVER from the typechecker.
  *)
val get_package_with_override_for_file_no_env :
  support_multifile_tests:bool ->
  t ->
  path:string ->
  content:string ->
  Package.t option * bool
