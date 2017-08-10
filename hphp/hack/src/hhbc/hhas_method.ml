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
  method_no_injection  : bool;
  method_name          : Hhbc_id.Method.t;
  method_body          : Hhas_body.t;
  method_span          : Hhas_pos.span;
  method_is_async      : bool;
  method_is_generator      : bool;
  method_is_pair_generator : bool;
  method_is_closure_body : bool;
}

let make
  method_attributes
  method_is_protected
  method_is_public
  method_is_private
  method_is_static
  method_is_final
  method_is_abstract
  method_no_injection
  method_name
  method_body
  method_span
  method_is_async
  method_is_generator
  method_is_pair_generator
  method_is_closure_body = {
    method_attributes;
    method_is_protected;
    method_is_public;
    method_is_private;
    method_is_static;
    method_is_final;
    method_is_abstract;
    method_no_injection;
    method_name;
    method_body;
    method_span;
    method_is_async;
    method_is_generator;
    method_is_pair_generator;
    method_is_closure_body;
  }

let attributes method_def = method_def.method_attributes
let is_protected method_def = method_def.method_is_protected
let is_private method_def = method_def.method_is_private
let is_public method_def = method_def.method_is_public
let is_static method_def = method_def.method_is_static
let is_final method_def = method_def.method_is_final
let is_abstract method_def = method_def.method_is_abstract
let no_injection method_def = method_def.method_no_injection
let name method_def = method_def.method_name
let with_name method_def method_name = { method_def with method_name }
let make_private method_def =
  { method_def with
    method_is_protected = false;
    method_is_public = false;
    method_is_private = true }
let body method_def = method_def.method_body
let span method_def = method_def.method_span
let is_async method_def = method_def.method_is_async
let is_generator method_def = method_def.method_is_generator
let is_pair_generator method_def = method_def.method_is_pair_generator
let is_closure_body method_def = method_def.method_is_closure_body
let with_body method_def method_body = { method_def with method_body }
let params m = m.method_body.Hhas_body.body_params
let return_type m = m.method_body.Hhas_body.body_return_type
let with_is_async m method_is_async = { m with method_is_async }
