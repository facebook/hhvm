(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Notes:
* require-implements is not represented in HHAS.
* require-extends is not represented in HHAS.
* TODO: c_uses ?
* TODO: c_xhp_attr_uses ?
* TODO: c_xhp_category ?
*)

module T = Tast

type trait_req_kind =
  | MustExtend
  | MustImplement

type t = {
  class_attributes: Hhas_attribute.t list;
  class_base: Hhbc_id.Class.t option;
  class_implements: Hhbc_id.Class.t list;
  class_name: Hhbc_id.Class.t;
  class_span: Hhas_pos.span;
  class_is_final: bool;
  class_is_sealed: bool;
  class_is_abstract: bool;
  class_is_interface: bool;
  class_is_trait: bool;
  class_is_xhp: bool;
  class_hoisted: Closure_convert.hoist_kind;
  class_is_const: bool;
  class_no_dynamic_props: bool;
  class_needs_no_reifiedinit: bool;
  class_uses: string list;
  (* Deprecated - kill please *)
  class_use_aliases:
    (string option * string * string option * T.use_as_visibility list) list;
  (* Deprecated - kill please *)
  class_use_precedences: (string * string * string list) list;
  class_method_trait_resolutions: (T.method_redeclaration * string) list;
  class_enum_type: Hhas_type_info.t option;
  class_methods: Hhas_method.t list;
  class_properties: Hhas_property.t list;
  class_constants: Hhas_constant.t list;
  class_type_constants: Hhas_type_constant.t list;
  class_requirements: (trait_req_kind * string) list;
  class_upper_bounds: (string * Hhas_type_info.t list) list;
  class_doc_comment: string option;
}

let make
    class_attributes
    class_base
    class_implements
    class_name
    class_span
    class_is_final
    class_is_sealed
    class_is_abstract
    class_is_interface
    class_is_trait
    class_is_xhp
    class_hoisted
    class_is_const
    class_no_dynamic_props
    class_needs_no_reifiedinit
    class_uses
    class_use_aliases
    class_use_precedences
    class_method_trait_resolutions
    class_enum_type
    class_methods
    class_properties
    class_constants
    class_type_constants
    class_requirements
    class_upper_bounds
    class_doc_comment =
  {
    class_attributes;
    class_base;
    class_implements;
    class_name;
    class_span;
    class_is_final;
    class_is_sealed;
    class_is_abstract;
    class_is_interface;
    class_is_trait;
    class_is_xhp;
    class_hoisted;
    class_is_const;
    class_no_dynamic_props;
    class_needs_no_reifiedinit;
    class_uses;
    class_use_aliases;
    class_use_precedences;
    class_method_trait_resolutions;
    class_enum_type;
    class_methods;
    class_properties;
    class_constants;
    class_type_constants;
    class_requirements;
    class_upper_bounds;
    class_doc_comment;
  }

let attributes hhas_class = hhas_class.class_attributes

let base hhas_class = hhas_class.class_base

let implements hhas_class = hhas_class.class_implements

let name hhas_class = hhas_class.class_name

let span hhas_class = hhas_class.class_span

let is_final hhas_class = hhas_class.class_is_final

let is_sealed hhas_class = hhas_class.class_is_sealed

let is_abstract hhas_class = hhas_class.class_is_abstract

let is_interface hhas_class = hhas_class.class_is_interface

let is_trait hhas_class = hhas_class.class_is_trait

let is_xhp hhas_class = hhas_class.class_is_xhp

let is_top hhas_class =
  match hhas_class.class_hoisted with
  | Closure_convert.TopLevel -> true
  | Closure_convert.Hoisted -> false

let is_const hhas_class = hhas_class.class_is_const

let no_dynamic_props hhas_class = hhas_class.class_no_dynamic_props

let needs_no_reifiedinit hhas_class = hhas_class.class_needs_no_reifiedinit

let class_uses hhas_class = hhas_class.class_uses

let class_use_aliases hhas_class = hhas_class.class_use_aliases

let class_use_precedences hhas_class = hhas_class.class_use_precedences

let class_method_trait_resolutions hhas_class =
  hhas_class.class_method_trait_resolutions

let enum_type hhas_class = hhas_class.class_enum_type

let methods hhas_class = hhas_class.class_methods

let with_methods hhas_class class_methods = { hhas_class with class_methods }

let properties hhas_class = hhas_class.class_properties

let constants hhas_class = hhas_class.class_constants

let type_constants hhas_class = hhas_class.class_type_constants

let requirements hhas_class = hhas_class.class_requirements

let upper_bounds hhas_class = hhas_class.class_upper_bounds

let is_closure_class hhas_class =
  List.exists Hhas_method.is_closure_body (methods hhas_class)

let doc_comment hhas_class = hhas_class.class_doc_comment
