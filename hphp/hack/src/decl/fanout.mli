(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Represents the "fanout" of a change, representing the cached
    information we must invalidate and the files we must re-typecheck if we want
    to produce a correct list of all errors in the repository which reflects
    those changes. *)
type t = {
  changed: Typing_deps.DepSet.t;
      (** The subset of classes in changed files whose decls changed (excluding
          classes whose decls experienced a positions-only change). *)
  to_recheck: Typing_deps.DepSet.t;
      (** This set represents the dependents of the declarations which were
          changed. In order to detect all errors which may have resulted from
          the changes, we must re-typecheck all files containing these
          dependents. Because DepSet.t is lossy--it is only a set of symbol
          hashes--we cannot precisely reproduce the set of symbols which need
          rechecking. Instead, Typing_deps maintains a mapping from symbol hash
          to filename (exposed via Typing_deps.get_files). *)
  to_recheck_if_errors: Typing_deps.DepSet.t;
      (** Additional deps to recheck if their files previously had errors. *)
}

val empty : t

val union : t -> t -> t

val add_fanout_of :
  Typing_deps_mode.t ->
  Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
  t ->
  t

(** Cardinal of deps to recheck. Ignore deps to recheck if they had errors *)
val cardinal : t -> int
