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
  method_attributes    : Hhas_attribute.t list;
  method_is_protected  : bool;
  method_is_public     : bool;
  method_is_private    : bool;
  method_is_static     : bool;
  method_is_final      : bool;
  method_is_abstract   : bool;
  method_name          : Litstr.id;
  method_params        : Hhas_param.t list;
  method_return_type   : Hhas_type_info.t option;
  method_body          : Hhbc_ast.instruct list;
}

let make
  method_attributes
  method_is_protected
  method_is_public
  method_is_private
  method_is_static
  method_is_final
  method_is_abstract
  method_name
  method_params
  method_return_type
  method_body = {
    method_attributes;
    method_is_protected;
    method_is_public;
    method_is_private;
    method_is_static;
    method_is_final;
    method_is_abstract;
    method_name;
    method_params;
    method_return_type;
    method_body
  }

let attributes method_def = method_def.method_attributes
let is_protected method_def = method_def.method_is_protected
let is_private method_def = method_def.method_is_private
let is_public method_def = method_def.method_is_public
let is_static method_def = method_def.method_is_static
let is_final method_def = method_def.method_is_final
let is_abstract method_def = method_def.method_is_abstract
let name method_def = method_def.method_name
let with_name method_def method_name = { method_def with method_name }
let make_private method_def =
  { method_def with
    method_is_protected = false;
    method_is_public = false;
    method_is_private = true }
let params method_def = method_def.method_params
let return_type method_def = method_def.method_return_type
let body method_def = method_def.method_body
let with_body method_def method_body = { method_def with method_body }
