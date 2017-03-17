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
  property_name         : Litstr.id;
  (* TODO: xhp, initializer *)
}

(* Interestingly, HHAS does not represent the declared types of properties,
unlike formal parameters and return types. We might consider fixing this. *)

let make
  property_is_private
  property_is_protected
  property_is_public
  property_is_static
  property_name = {
    property_is_private;
    property_is_protected;
    property_is_public;
    property_is_static;
    property_name
  }

let name hhas_property = hhas_property.property_name
let is_private hhas_property = hhas_property.property_is_private
let is_protected hhas_property = hhas_property.property_is_protected
let is_public hhas_property = hhas_property.property_is_public
let is_static hhas_property = hhas_property.property_is_static
