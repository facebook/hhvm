(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
module Ast = Tast

type t = {
  xhp_attribute_type: Ast.hint option;
  xhp_attribute_class_var: Ast.class_var;
  xhp_attribute_tag: Aast.xhp_attr_tag option;
  xhp_attribute_maybe_enum: (Pos.t * bool * Ast.expr list) option;
}

let make
    xhp_attribute_type
    xhp_attribute_class_var
    xhp_attribute_tag
    xhp_attribute_maybe_enum =
  {
    xhp_attribute_type;
    xhp_attribute_class_var;
    xhp_attribute_tag;
    xhp_attribute_maybe_enum;
  }

let type_ xa = xa.xhp_attribute_type

let class_var xa = xa.xhp_attribute_class_var

let is_required xa =
  Option.value_map xa.xhp_attribute_tag ~default:false ~f:(fun tag ->
      match tag with
      | Aast.Required
      | Aast.LateInit ->
        true)

let maybe_enum xa = xa.xhp_attribute_maybe_enum
