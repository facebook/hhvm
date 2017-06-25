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
  typedef_name       : Hhbc_id.Class.t;
  typedef_type_info  : Hhas_type_info.t;
  typedef_type_structure : Typed_value.t option;
}

let make
  typedef_name
  typedef_type_info
  typedef_type_structure =
  {
    typedef_name;
    typedef_type_info;
    typedef_type_structure;
  }

let name hhas_typedef = hhas_typedef.typedef_name
let type_info  hhas_typedef = hhas_typedef.typedef_type_info
let type_structure hhas_typedef = hhas_typedef.typedef_type_structure
