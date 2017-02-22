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
  property_name         : Litstr.id;
  (* TODO: final, xhp, visibility, type, initializer *)
}

let make property_name =
  { property_name }

let name hhas_property =
  hhas_property.property_name
