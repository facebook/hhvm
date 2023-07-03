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

val include_fanout_of_dep : Mode.t -> Dep.t -> DepSet.t -> DepSet.t

val get_maximum_fanout : Mode.t -> Dep.t -> DepSet.t
