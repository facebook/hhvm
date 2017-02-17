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
  (* TODO attributes *)
  (* TODO generic type parameters *)
  class_base         : Hhas_type_info.t option;
  class_implements   : Hhas_type_info.t list;
  class_name         : Litstr.id;
  class_is_final     : bool;
  class_is_abstract  : bool;
  class_is_interface : bool;
  class_is_trait     : bool;
  class_is_enum      : bool;
  class_methods      : Hhas_method.t list;
  (* TODO other members *)
  (* TODO XHP stuff *)
}

let make
  class_base
  class_implements
  class_name
  class_is_final
  class_is_abstract
  class_is_interface
  class_is_trait
  class_is_enum
  class_methods =
  {
    class_base;
    class_implements;
    class_name;
    class_is_final;
    class_is_abstract;
    class_is_interface;
    class_is_trait;
    class_is_enum;
    class_methods
  }

let base hhas_class = hhas_class.class_base
let implements hhas_class = hhas_class.class_implements
let name hhas_class = hhas_class.class_name
let is_final hhas_class = hhas_class.class_is_final
let is_abstract hhas_class = hhas_class.class_is_abstract
let is_interface hhas_class = hhas_class.class_is_interface
let is_trait hhas_class = hhas_class.class_is_trait
let is_enum hhas_class = hhas_class.class_is_enum
let methods hhas_class = hhas_class.class_methods
