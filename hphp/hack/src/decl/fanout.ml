(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Dep = Typing_deps.Dep
module DepSet = Typing_deps.DepSet

type t = {
  changed: DepSet.t;
  to_recheck: DepSet.t;
}

let empty = { changed = DepSet.make (); to_recheck = DepSet.make () }

let union
    { changed = changed1; to_recheck = to_recheck1 }
    { changed = changed2; to_recheck = to_recheck2 } =
  {
    changed = DepSet.union changed1 changed2;
    to_recheck = DepSet.union to_recheck1 to_recheck2;
  }

let add_fanout_of mode dep { changed; to_recheck } =
  {
    changed = DepSet.add changed (Dep.make dep);
    to_recheck = DepSet.union (Typing_deps.get_ideps mode dep) to_recheck;
  }
