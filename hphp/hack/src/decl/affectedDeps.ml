(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_deps

type t = {
  changed: DepSet.t;
  mro_invalidated: DepSet.t;
  needs_recheck: DepSet.t;
}

let empty =
  {
    changed = DepSet.empty;
    mro_invalidated = DepSet.empty;
    needs_recheck = DepSet.empty;
  }

let mark_changed (deps : t) (changed : DepSet.t) : t =
  { deps with changed = DepSet.union deps.changed changed }

let mark_mro_invalidated (deps : t) (mro_invalidated : DepSet.t) : t =
  {
    deps with
    mro_invalidated = DepSet.union deps.mro_invalidated mro_invalidated;
  }

let mark_as_needing_recheck (deps : t) (needs_recheck : DepSet.t) : t =
  { deps with needs_recheck = DepSet.union deps.needs_recheck needs_recheck }

let mark_all_dependents_as_needing_recheck
    (deps : t) (dep : Dep.dependency Dep.variant) : t =
  mark_as_needing_recheck deps (Typing_deps.get_ideps dep)

let add_maximum_fanout (deps : t) (changed_dep : Dep.t) : t =
  let changed = DepSet.singleton changed_dep in
  let changed_and_descendants = Typing_deps.add_extend_deps changed in
  let needs_recheck = Typing_deps.add_typing_deps changed_and_descendants in
  let deps = mark_changed deps changed in
  let deps = mark_mro_invalidated deps changed_and_descendants in
  let deps = mark_as_needing_recheck deps needs_recheck in
  deps

let union (a : t) (b : t) : t =
  {
    changed = DepSet.union a.changed b.changed;
    mro_invalidated = DepSet.union a.mro_invalidated b.mro_invalidated;
    needs_recheck = DepSet.union a.needs_recheck b.needs_recheck;
  }
