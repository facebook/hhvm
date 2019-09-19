(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let handler =
  object
    inherit Tast_visitor.handler_base
  end

let whitelist_path _path = ()
