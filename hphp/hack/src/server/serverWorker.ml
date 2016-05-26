(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* As for [Daemon.register_entry_point], this should stay
   at toplevel, in order to be executed before
   [Daemon.check_entry_point]. *)
let entry = Worker.register_entry_point ~restore:(ServerGlobalState.restore)

let make gc_control heap_handle =
  Worker.make
    ~saved_state:(ServerGlobalState.save ())
    ~entry
    ~nbr_procs: (GlobalConfig.nbr_procs)
    ~gc_control
    ~heap_handle
