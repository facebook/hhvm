(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  id : int;
  errors : Errors.t;
}

let of_id ~id = {
  id;
  errors = Errors.empty;
}

let get_id ds = ds.id

let clear ds = { ds with errors = Errors.empty }

let get_errors ds = ds.errors

let update ds errors = { ds with errors }
