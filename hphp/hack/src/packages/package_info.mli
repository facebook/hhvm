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

val get_package : t -> string -> Package.t option

val package_exists : t -> string -> bool

(** [info] with one more package, for a caller that needs one the configuration
    does not declare.

    The new package wins for any path its include paths match, and every other
    path resolves exactly as before: this adds to the configuration rather than
    standing in for it. Membership is a prefix test over paths, not a set of
    files, so an include path also claims anything extending it — [a/B.php.expect]
    beside [a/B.php].

    Raises if a package of the same name is already configured, since replacing
    it would remove it from resolution. A caller needs a name the configuration
    will not use. *)
val add_package : t -> Package.t -> t

(** The package a file path belongs to, ignoring any __PackageOverride
  * annotation. [path] must already be repo-relative and normalized; callers in
  * the typechecker should go through [Package_provider] rather than calling this
  * directly, so that multifile test paths are handled in one place.
  *)
val get_package_for_file : t -> path:string -> Package.t option
