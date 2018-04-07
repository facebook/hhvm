(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

let (hint_hooks: (Pos.t * string -> unit) list ref) = ref []

let attach_hint_hook hook =
  hint_hooks := hook :: !hint_hooks

let dispatch_hint_hook id =
  List.iter !hint_hooks begin fun hook -> hook id end

let remove_all_hooks () =
  hint_hooks := [];
