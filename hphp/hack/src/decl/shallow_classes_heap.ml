(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

module Capacity = struct
  let capacity = 1000
end

module Class = struct
  type t = shallow_class

  let prefix = Prefix.make ()

  let description = "Decl_ShallowClass"
end

module Classes =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (StringKey) (Class)
    (Capacity)
