(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t
val save : unit -> t
val restore : t -> unit
val get_hhi_path : t -> Path.t
val get_root_path : t -> Path.t
val fake_state : t
