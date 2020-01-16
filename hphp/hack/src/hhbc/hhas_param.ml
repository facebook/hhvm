(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  param_name: string;
  param_is_variadic: bool;
  param_is_inout: bool;
  param_user_attributes: Hhas_attribute.t list;
  param_type_info: Hhas_type_info.t option;
  param_default_value: (Label.t * Tast.expr) option;
}

let make
    param_name
    param_is_variadic
    param_is_inout
    param_user_attributes
    param_type_info
    param_default_value =
  {
    param_name;
    param_is_variadic;
    param_is_inout;
    param_user_attributes;
    param_type_info;
    param_default_value;
  }

let name p = p.param_name

let is_variadic p = p.param_is_variadic

let is_inout p = p.param_is_inout

let user_attributes p = p.param_user_attributes

let type_info p = p.param_type_info

let default_value p = p.param_default_value

let without_type p =
  match p.param_type_info with
  | Some ti ->
    let ut = Hhas_type_info.user_type ti in
    let tc = Hhas_type_constraint.make None [] in
    let ti = Hhas_type_info.make ut tc in
    { p with param_type_info = Some ti }
  | None -> p

let with_name name p = { p with param_name = name }
