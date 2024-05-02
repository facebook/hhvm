(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = GlobalOptions.t [@@deriving show]

let default = GlobalOptions.default

(* Changes here need to be synchronized with rust_parser_errors_ffi.rs *)
type ffi_t =
  bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool

let to_rust_ffi_t po ~hhvm_compat_mode ~hhi_mode ~codegen =
  GlobalOptions.
    ( hhvm_compat_mode,
      hhi_mode,
      codegen,
      po.po_disable_lval_as_an_expression,
      po.po_disable_legacy_soft_typehints,
      po.po_const_static_props,
      po.po_disable_legacy_attribute_syntax,
      po.po_const_default_func_args,
      po.po_abstract_static_props,
      po.po_disallow_func_ptrs_in_constants,
      po.po_enable_xhp_class_modifier,
      po.po_disable_xhp_element_mangling,
      po.po_disable_xhp_children_declarations,
      po.po_const_default_lambda_args,
      po.po_allow_unstable_features,
      po.po_interpret_soft_types_as_like_types,
      po.tco_is_systemlib,
      po.po_disallow_static_constants_in_default_func_args,
      po.po_disallow_direct_superglobals_refs )
