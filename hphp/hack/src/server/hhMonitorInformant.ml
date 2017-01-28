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

include HhMonitorInformant_sig.Types
let init _init_env = ()
let report () = Informant_sig.Move_along
