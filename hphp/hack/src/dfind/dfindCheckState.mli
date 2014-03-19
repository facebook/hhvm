(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Checks that the current state is clean, that is, that we have more
 * files than there actually are on disk.
 * If this wasn't the case, there is something horribly wrong.
 *)
(*****************************************************************************)

val check: DfindEnv.t -> string
