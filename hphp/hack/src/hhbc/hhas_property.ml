(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  property_attributes: Hhas_attribute.t list;
  property_visibility: Aast.visibility;
  property_is_abstract: bool;
  property_is_static: bool;
  property_is_deep_init: bool;
  property_is_const: bool;
  property_is_lsb: bool;
  property_is_no_bad_redeclare: bool;
  property_has_system_initial: bool;
  property_no_implicit_null: bool;
  property_initial_satisfies_tc: bool;
  property_is_late_init: bool;
  property_name: Hhbc_id.Prop.t;
  property_initial_value: Typed_value.t option;
  property_initializer_instrs: Instruction_sequence.t option;
  property_type_info: Hhas_type_info.t;
  property_doc_comment: string option;
}

let make
    property_attributes
    property_visibility
    property_is_abstract
    property_is_static
    property_is_deep_init
    property_is_const
    property_is_lsb
    property_is_no_bad_redeclare
    property_has_system_initial
    property_no_implicit_null
    property_initial_satisfies_tc
    property_is_late_init
    property_name
    property_initial_value
    property_initializer_instrs
    property_type_info
    property_doc_comment =
  {
    property_attributes;
    property_visibility;
    property_is_abstract;
    property_is_static;
    property_is_deep_init;
    property_is_const;
    property_is_lsb;
    property_is_no_bad_redeclare;
    property_has_system_initial;
    property_no_implicit_null;
    property_initial_satisfies_tc;
    property_is_late_init;
    property_name;
    property_initial_value;
    property_initializer_instrs;
    property_type_info;
    property_doc_comment;
  }

let name hhas_property = hhas_property.property_name

let attributes hhas_property = hhas_property.property_attributes

let is_private hhas_property = hhas_property.property_visibility = Aast.Private

let is_protected hhas_property =
  hhas_property.property_visibility = Aast.Protected

let is_public hhas_property = hhas_property.property_visibility = Aast.Public

let visibility hhas_property = hhas_property.property_visibility

let is_abstract hhas_property = hhas_property.property_is_abstract

let is_static hhas_property = hhas_property.property_is_static

let is_deep_init hhas_property = hhas_property.property_is_deep_init

let initial_value hhas_property = hhas_property.property_initial_value

let initializer_instrs hhas_property = hhas_property.property_initializer_instrs

let is_const hhas_property = hhas_property.property_is_const

let is_lsb hhas_property = hhas_property.property_is_lsb

let is_no_bad_redeclare hhas_property =
  hhas_property.property_is_no_bad_redeclare

let has_system_initial hhas_property = hhas_property.property_has_system_initial

let no_implicit_null hhas_property = hhas_property.property_no_implicit_null

let initial_satisfies_tc hhas_property =
  hhas_property.property_initial_satisfies_tc

let is_late_init hhas_property = hhas_property.property_is_late_init

let type_info hhas_property = hhas_property.property_type_info

let doc_comment hhas_property = hhas_property.property_doc_comment
