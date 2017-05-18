(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Notes:
* require-implements is not represented in HHAS.
* require-extends is not represented in HHAS.
* TODO: c_uses ?
* TODO: c_xhp_attr_uses ?
* TODO: c_xhp_category ?
*)

type t = {
  class_attributes   : Hhas_attribute.t list;
  class_base         : Hhbc_id.Class.t option;
  class_implements   : Hhbc_id.Class.t list;
  class_name         : Hhbc_id.Class.t;
  class_is_final     : bool;
  class_is_abstract  : bool;
  class_is_interface : bool;
  class_is_trait     : bool;
  class_is_xhp       : bool;
  class_uses         : Litstr.id list;
  class_enum_type    : Hhas_type_info.t option;
  class_methods      : Hhas_method.t list;
  class_properties   : Hhas_property.t list;
  class_constants    : Hhas_constant.t list;
  class_type_constants : Hhas_type_constant.t list;
}

let make
  class_attributes
  class_base
  class_implements
  class_name
  class_is_final
  class_is_abstract
  class_is_interface
  class_is_trait
  class_is_xhp
  class_uses
  class_enum_type
  class_methods
  class_properties
  class_constants
  class_type_constants =
  {
    class_attributes;
    class_base;
    class_implements;
    class_name;
    class_is_final;
    class_is_abstract;
    class_is_interface;
    class_is_trait;
    class_is_xhp;
    class_uses;
    class_enum_type;
    class_methods;
    class_properties;
    class_constants;
    class_type_constants
  }

let attributes hhas_class = hhas_class.class_attributes
let base hhas_class = hhas_class.class_base
let implements hhas_class = hhas_class.class_implements
let name hhas_class = hhas_class.class_name
let is_final hhas_class = hhas_class.class_is_final
let is_abstract hhas_class = hhas_class.class_is_abstract
let is_interface hhas_class = hhas_class.class_is_interface
let is_trait hhas_class = hhas_class.class_is_trait
let is_xhp hhas_class = hhas_class.class_is_xhp
let class_uses hhas_class = hhas_class.class_uses
let enum_type hhas_class = hhas_class.class_enum_type
let methods hhas_class = hhas_class.class_methods
let with_methods hhas_class class_methods = { hhas_class with class_methods }
let properties hhas_class = hhas_class.class_properties
let with_property hhas_class hhas_property = { hhas_class with
  class_properties = hhas_property :: hhas_class.class_properties }
let constants hhas_class = hhas_class.class_constants
let type_constants hhas_class = hhas_class.class_type_constants
let is_closure_class hhas_class =
  List.exists Hhas_method.is_closure_body (methods hhas_class)
