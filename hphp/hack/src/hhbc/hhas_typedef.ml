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
  typedef_name       : Litstr.id;
  typedef_type_info  : Hhas_type_info.t;
}

let make
  typedef_name
  typedef_type_info =
  {
    typedef_name;
    typedef_type_info;
  }

let name hhas_typedef = hhas_typedef.typedef_name
let type_info  hhas_typedef = hhas_typedef.typedef_type_info
