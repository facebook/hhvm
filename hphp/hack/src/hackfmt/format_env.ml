(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  add_trailing_commas: bool;
}

let default = {
  add_trailing_commas = true;
}

let add_trailing_commas t = t.add_trailing_commas
