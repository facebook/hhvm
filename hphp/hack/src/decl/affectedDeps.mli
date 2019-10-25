(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_deps

(** AffectedDeps.t represents the "fanout" of a change, representing the cached
    information we must invalidate and the files we must re-typecheck if we want
    to produce a correct list of all errors in the repository which reflects
    those changes. *)
type t = {
  changed: DepSet.t;
      (** The subset of classes in changed files whose decls changed (excluding
          classes whose decls experienced a positions-only change). *)
  mro_invalidated: DepSet.t;
      (** These classes either directly experienced some change to their
          inheritance hierarchy (e.g., an interface was added, or the extends
          clause was changed), or one of their ancestors did. Since the
          hierarchy has changed, the member resolution order we have stored for
          them is invalidated, and must be removed from the linearizations heap
          (it will be lazily recomputed on demand). *)
  needs_recheck: DepSet.t;
      (** This set represents the dependents of the declarations which were
          changed. In order to detect all errors which may have resulted from
          the changes, we must re-typecheck all files containing these
          dependents. Because DepSet.t is lossy--it is only a set of symbol
          hashes--we cannot precisely reproduce the set of symbols which need
          rechecking. Instead, Typing_deps maintains a mapping from symbol hash
          to filename (exposed via Typing_deps.get_files). *)
}

val empty : t

val mark_changed : t -> DepSet.t -> t

val mark_mro_invalidated : t -> DepSet.t -> t

val mark_as_needing_recheck : t -> DepSet.t -> t

val mark_all_dependents_as_needing_recheck :
  t -> Dep.dependency Dep.variant -> t

val add_maximum_fanout : t -> Dep.t -> t

val union : t -> t -> t
