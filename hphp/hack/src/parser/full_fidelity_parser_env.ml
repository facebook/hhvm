(*
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
  disable_lval_as_an_expression: bool;
  disable_nontoplevel_declarations: bool;
  mode: FileInfo.mode option;
  stats: Stats_container.t option;
  rust: bool;
  disable_legacy_soft_typehints: bool;
  allow_new_attribute_syntax: bool;
  disable_legacy_attribute_syntax: bool;
  leak_rust_tree: bool;
}
[@@deriving show]

let default =
  {
    hhvm_compat_mode = false;
    php5_compat_mode = false;
    codegen = false;
    disable_lval_as_an_expression = false;
    disable_nontoplevel_declarations = false;
    rust = true;
    mode = None;
    stats = None;
    disable_legacy_soft_typehints = false;
    allow_new_attribute_syntax = false;
    disable_legacy_attribute_syntax = false;
    leak_rust_tree = false;
  }

let make
    ?(hhvm_compat_mode = default.hhvm_compat_mode)
    ?(php5_compat_mode = default.php5_compat_mode)
    ?(codegen = default.codegen)
    ?(disable_lval_as_an_expression = default.disable_lval_as_an_expression)
    ?(disable_nontoplevel_declarations =
      default.disable_nontoplevel_declarations)
    ?mode
    ?stats
    ?(rust = default.rust)
    ?(disable_legacy_soft_typehints = default.disable_legacy_soft_typehints)
    ?(allow_new_attribute_syntax = default.allow_new_attribute_syntax)
    ?(disable_legacy_attribute_syntax =
      default.disable_legacy_attribute_syntax)
    ?((* DANGER: if you leak the root tree into OCaml, it's on you to ensure that
  * it's eventually disposed to avoid memory leak. *)
    leak_rust_tree = default.leak_rust_tree)
    () =
  {
    hhvm_compat_mode;
    php5_compat_mode;
    codegen;
    disable_lval_as_an_expression;
    disable_nontoplevel_declarations;
    mode;
    stats;
    rust;
    disable_legacy_soft_typehints;
    allow_new_attribute_syntax;
    disable_legacy_attribute_syntax;
    leak_rust_tree;
  }

let hhvm_compat_mode e = e.hhvm_compat_mode

let php5_compat_mode e = e.php5_compat_mode

let codegen e = e.codegen

let disable_lval_as_an_expression e = e.disable_lval_as_an_expression

let disable_nontoplevel_declarations e = e.disable_nontoplevel_declarations

let mode e = e.mode

let stats e = e.stats

let is_experimental_mode e = e.mode = Some FileInfo.Mexperimental

let is_strict e = e.mode = Some FileInfo.Mstrict

let rust e = e.rust

let disable_legacy_soft_typehints e = e.disable_legacy_soft_typehints

let allow_new_attribute_syntax e = e.allow_new_attribute_syntax

let disable_legacy_attribute_syntax e = e.disable_legacy_attribute_syntax

let leak_rust_tree e = e.leak_rust_tree
