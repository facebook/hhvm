(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Hack's informant. Currently a dummy that does nothing. *)

type t = unit
type init_env = unit
let init () = ()
let report () = Informant_sig.Move_along
