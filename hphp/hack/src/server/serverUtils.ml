(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This is in serverUtils instead of Utils because the latter is a dependency
 * in the JS build and cannot access the Sys module *)
let with_timeout timeout ~on_timeout ~do_ =
  let old_handler = ref Sys.Signal_default in
  let old_timeout = ref 0 in
  Utils.with_context
    ~enter:(fun () ->
      old_handler := Sys.signal Sys.sigalrm (Sys.Signal_handle on_timeout);
      old_timeout := Unix.alarm timeout)
    ~exit:(fun () ->
      ignore (Unix.alarm !old_timeout);
      Sys.set_signal Sys.sigalrm !old_handler)
    ~do_
