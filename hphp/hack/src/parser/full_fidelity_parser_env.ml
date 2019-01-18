(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  hhvm_compat_mode: bool;
  php5_compat_mode: bool;
  has_dot_hack_extension: bool;
  codegen: bool;
  force_hh: bool;
  enable_xhp: bool;
  enable_stronger_await_binding: bool;
  lang: FileInfo.file_type option;
  mode: FileInfo.mode option;
  stats: Stats_container.t option;
} [@@deriving show]

let default = {
  hhvm_compat_mode = false;
  php5_compat_mode = false;
  has_dot_hack_extension = false;
  codegen = false;
  force_hh = false;
  enable_xhp = false;
  enable_stronger_await_binding = false;

  lang = None;
  mode = None;
  stats = None;
}

let make
  ?(hhvm_compat_mode = default.hhvm_compat_mode)
  ?(php5_compat_mode = default.php5_compat_mode)
  ?(has_dot_hack_extension = default.has_dot_hack_extension)
  ?(codegen = default.codegen)
  ?(force_hh = default.force_hh)
  ?(enable_xhp = default.enable_xhp)
  ?(enable_stronger_await_binding = default.enable_stronger_await_binding)
  ?lang
  ?mode
  ?stats
  () = {
    hhvm_compat_mode;
    php5_compat_mode;
    has_dot_hack_extension;
    codegen;
    force_hh;
    enable_xhp;
    enable_stronger_await_binding;
    lang;
    mode;
    stats;
  }

let hhvm_compat_mode e = e.hhvm_compat_mode
let php5_compat_mode e = e.php5_compat_mode
let has_dot_hack_extension e = e.has_dot_hack_extension
let codegen e = e.codegen
let force_hh e = e.force_hh
let enable_xhp e = e.enable_xhp
let enable_stronger_await_binding e = e.enable_stronger_await_binding
let lang e = e.lang
let mode e = e.mode
let stats e = e.stats
let is_experimental_mode e = e.mode = Some FileInfo.Mexperimental
