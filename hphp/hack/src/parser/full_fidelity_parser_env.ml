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
  codegen: bool;
  force_hh: bool;
  enable_xhp: bool;
  enable_stronger_await_binding: bool;
  disable_lval_as_an_expression: bool;
  disable_nontoplevel_declarations: bool;
  mode: FileInfo.mode option;
  stats: Stats_container.t option;
  disable_unsafe_expr: bool;
  disable_unsafe_block: bool;
} [@@deriving show]

let default = {
  hhvm_compat_mode = false;
  php5_compat_mode = false;
  codegen = false;
  force_hh = false;
  enable_xhp = false;
  enable_stronger_await_binding = false;
  disable_lval_as_an_expression = false;
  disable_nontoplevel_declarations = false;
  disable_unsafe_expr = false;
  disable_unsafe_block = false;

  mode = None;
  stats = None;
}

let make
  ?(hhvm_compat_mode = default.hhvm_compat_mode)
  ?(php5_compat_mode = default.php5_compat_mode)
  ?(codegen = default.codegen)
  ?(force_hh = default.force_hh)
  ?(enable_xhp = default.enable_xhp)
  ?(enable_stronger_await_binding = default.enable_stronger_await_binding)
  ?(disable_lval_as_an_expression = default.disable_lval_as_an_expression)
  ?(disable_nontoplevel_declarations = default.disable_nontoplevel_declarations)
  ?mode
  ?stats
  ?(disable_unsafe_expr = default.disable_unsafe_expr)
  ?(disable_unsafe_block = default.disable_unsafe_block)
  () = {
    hhvm_compat_mode;
    php5_compat_mode;
    codegen;
    force_hh;
    enable_xhp;
    enable_stronger_await_binding;
    disable_lval_as_an_expression;
    disable_nontoplevel_declarations;
    mode;
    stats;
    disable_unsafe_expr;
    disable_unsafe_block;
  }

let hhvm_compat_mode e = e.hhvm_compat_mode
let php5_compat_mode e = e.php5_compat_mode
let codegen e = e.codegen
let force_hh e = e.force_hh
let enable_xhp e = e.enable_xhp
let enable_stronger_await_binding e = e.enable_stronger_await_binding
let disable_lval_as_an_expression e = e.disable_lval_as_an_expression
let disable_nontoplevel_declarations e = e.disable_nontoplevel_declarations
let mode e = e.mode
let stats e = e.stats
let is_hack e = e.mode <> Some FileInfo.Mphp || force_hh e
let is_experimental_mode e = e.mode = Some FileInfo.Mexperimental
let is_strict e = e.mode = Some FileInfo.Mstrict
let is_typechecker e = is_hack e && not (codegen e)
let disable_unsafe_expr e = e.disable_unsafe_expr
let disable_unsafe_block e = e.disable_unsafe_block
