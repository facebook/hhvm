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

type t = {
  changed: DepSet.t;
  needs_recheck: DepSet.t;
}

let empty () =
  let empty = DepSet.make () in
  { changed = empty; needs_recheck = empty }

let mark_changed (deps : t) (changed : DepSet.t) : t =
  { deps with changed = DepSet.union deps.changed changed }

let mark_as_needing_recheck (deps : t) (needs_recheck : DepSet.t) : t =
  { deps with needs_recheck = DepSet.union deps.needs_recheck needs_recheck }

let mark_all_dependents_as_needing_recheck
    (mode : Mode.t) (deps : t) (dep : Dep.dependency Dep.variant) : t =
  mark_as_needing_recheck deps (Typing_deps.get_ideps mode dep)

let mark_all_dependents_as_needing_recheck_from_hash
    (mode : Mode.t) (deps : t) (hash : Dep.t) : t =
  mark_as_needing_recheck deps (Typing_deps.get_ideps_from_hash mode hash)

let get_maximum_fanout (mode : Mode.t) (changed_dep : Dep.t) : t =
  let changed = DepSet.singleton changed_dep in
  let changed_and_descendants = Typing_deps.add_extend_deps mode changed in
  let needs_recheck =
    Typing_deps.add_typing_deps mode changed_and_descendants
  in
  { changed; needs_recheck }

let union (a : t) (b : t) : t =
  {
    changed = DepSet.union a.changed b.changed;
    needs_recheck = DepSet.union a.needs_recheck b.needs_recheck;
  }
