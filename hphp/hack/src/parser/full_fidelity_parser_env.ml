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
  lang: FileInfo.file_type option;
  mode: FileInfo.mode option;
  stats: Stats_container.t option;
}

let default = {
  hhvm_compat_mode = false;
  php5_compat_mode = false;
  lang = None;
  mode = None;
  stats = None;
}

let make
  ?(hhvm_compat_mode = default.hhvm_compat_mode)
  ?(php5_compat_mode = default.php5_compat_mode)
  ?lang
  ?mode
  ?stats
  () =
  { hhvm_compat_mode; php5_compat_mode; lang; mode; stats }

let hhvm_compat_mode e = e.hhvm_compat_mode
let php5_compat_mode e = e.php5_compat_mode
let lang e = e.lang
let mode e = e.mode
let stats e = e.stats
