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
  property_is_private   : bool;
  property_is_protected : bool;
  property_is_public    : bool;
  property_is_static    : bool;
  property_is_deep_init  : bool;
  property_name         : Hhbc_id.Prop.t;
  property_initial_value  : Typed_value.t option;
  property_initializer_instrs : Instruction_sequence.t option;
  (* TODO: xhp *)
}

(* Interestingly, HHAS does not represent the declared types of properties,
unlike formal parameters and return types. We might consider fixing this. *)

let make
  property_is_private
  property_is_protected
  property_is_public
  property_is_static
  property_is_deep_init
  property_name
  property_initial_value
  property_initializer_instrs = {
    property_is_private;
    property_is_protected;
    property_is_public;
    property_is_static;
    property_is_deep_init;
    property_name;
    property_initial_value;
    property_initializer_instrs;
  }

let name hhas_property = hhas_property.property_name
let is_private hhas_property = hhas_property.property_is_private
let is_protected hhas_property = hhas_property.property_is_protected
let is_public hhas_property = hhas_property.property_is_public
let is_static hhas_property = hhas_property.property_is_static
let is_deep_init hhas_property = hhas_property.property_is_deep_init
let initial_value hhas_property = hhas_property.property_initial_value
let initializer_instrs hhas_property = hhas_property.property_initializer_instrs
