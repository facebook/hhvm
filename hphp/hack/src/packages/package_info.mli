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

(** The package a file path belongs to, ignoring any __PackageOverride
  * annotation. [path] must already be repo-relative and normalized; callers in
  * the typechecker should go through [Package_provider] rather than calling this
  * directly, so that multifile test paths are handled in one place.
  *)
val get_package_for_file : t -> path:string -> Package.t option

(** The get_package_with_override function returns the package a file path belongs
  * taking into account __PackageOverride annotations.  It requires not only the file path
  * but also the file content.
  * 
  * DO NOT USE: this function scans the content of the file and is __very inefficient__.
  * It should be called ONLY from services that cannot access decls, notably the 
  * Glean indexer and the redundant PackageOverride linter, and NEVER from the typechecker.
  *)
val get_package_with_override_for_file_no_env :
  t -> path:string -> content:string -> Package.t option * bool
