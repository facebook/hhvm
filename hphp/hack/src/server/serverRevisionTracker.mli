(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

val initialize : Hg.svn_rev -> unit

val on_state_leave :
  Path.t -> (* project root *)
  string -> (* state name *)
  Hh_json.json option -> (* state metadata *)
  unit

val check_blocking : unit -> unit
