(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_deps
module Mode = Typing_deps_mode

(** AffectedDeps.t represents the "fanout" of a change, representing the cached
    information we must invalidate and the files we must re-typecheck if we want
    to produce a correct list of all errors in the repository which reflects
    those changes. *)
type t = {
  changed: DepSet.t;
      (** The subset of classes in changed files whose decls changed (excluding
          classes whose decls experienced a positions-only change). *)
  needs_recheck: DepSet.t;
      (** This set represents the dependents of the declarations which were
          changed. In order to detect all errors which may have resulted from
          the changes, we must re-typecheck all files containing these
          dependents. Because DepSet.t is lossy--it is only a set of symbol
          hashes--we cannot precisely reproduce the set of symbols which need
          rechecking. Instead, Typing_deps maintains a mapping from symbol hash
          to filename (exposed via Typing_deps.get_files). *)
}

val empty : unit -> t

val mark_changed : t -> DepSet.t -> t

val mark_as_needing_recheck : t -> DepSet.t -> t

val mark_all_dependents_as_needing_recheck :
  Mode.t -> t -> Dep.dependency Dep.variant -> t

val mark_all_dependents_as_needing_recheck_from_hash : Mode.t -> t -> Dep.t -> t

val get_maximum_fanout : Mode.t -> Dep.t -> t

val union : t -> t -> t
