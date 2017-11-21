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
  hhvm_compat_mode: bool;
  php5_compat_mode: bool;
}

let default = {
  hhvm_compat_mode = false;
  php5_compat_mode = false;
}

let make
  ?(hhvm_compat_mode = default.hhvm_compat_mode)
  ?(php5_compat_mode = default.php5_compat_mode)
  () =
  { hhvm_compat_mode; php5_compat_mode }

let hhvm_compat_mode e = e.hhvm_compat_mode
let php5_compat_mode e = e.php5_compat_mode
