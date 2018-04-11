(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type t = {
  xhp_attribute_type : Ast.hint option;
  xhp_attribute_class_var : Ast.class_var;
  xhp_attribute_is_required : bool;
  xhp_attribute_maybe_enum : ((Pos.t * bool * Ast.expr list) option);
}

let make
  xhp_attribute_type
  xhp_attribute_class_var
  xhp_attribute_is_required
  xhp_attribute_maybe_enum =
  {
    xhp_attribute_type;
    xhp_attribute_class_var;
    xhp_attribute_is_required;
    xhp_attribute_maybe_enum
  }

let type_ xa = xa.xhp_attribute_type
let class_var xa = xa.xhp_attribute_class_var
let is_required xa = xa.xhp_attribute_is_required
let maybe_enum xa = xa.xhp_attribute_maybe_enum
