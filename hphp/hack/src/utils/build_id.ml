(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

external get_build_revision : unit -> string = "hh_get_build_revision"

let build_revision = get_build_revision ()
let build_id_ohai = build_revision
