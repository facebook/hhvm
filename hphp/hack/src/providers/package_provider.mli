(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Resolves the package a file belongs to. The typechecker must use this rather
  than {!Package_info} directly, so the multifile-test path convention is handled
  in one place.

  [path] is repo-relative (e.g. the result of [Relative_path.suffix]). *)

(** The package [path] belongs to, ignoring any [__PackageOverride] annotation. *)
val get_package_for_file : Provider_context.t -> path:string -> Package.t option

(** The package [path] belongs to, honoring an [__PackageOverride] annotation in
  [content]. The returned flag says whether an override was applied.

  DO NOT USE from the typechecker: it scans [content] and is very inefficient.
  For services that cannot access decls (the Glean indexer, the
  redundant-[__PackageOverride] linter). *)
val get_package_with_override_for_file_no_env :
  Provider_context.t -> path:string -> content:string -> Package.t option * bool
