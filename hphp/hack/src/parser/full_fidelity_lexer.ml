(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = struct
  let is_rust = ref false

  let set ~rust = is_rust := rust

  let is_rust () = !is_rust
end
