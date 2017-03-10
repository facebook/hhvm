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
  class_base         : Litstr.id option;
  class_implements   : Litstr.id list;
  class_name         : Litstr.id;
  class_is_final     : bool;
  class_is_abstract  : bool;
  class_is_interface : bool;
  class_is_trait     : bool;
  class_is_enum      : bool;
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
  class_is_enum
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
    class_is_enum;
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
let is_enum hhas_class = hhas_class.class_is_enum
let methods hhas_class = hhas_class.class_methods
let properties hhas_class = hhas_class.class_properties
let constants hhas_class = hhas_class.class_constants
let type_constants hhas_class = hhas_class.class_type_constants
