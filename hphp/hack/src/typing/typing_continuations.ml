(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
include Typing_cont_key

module Map = struct
  let show _ = "<WrappedMap.Make(Continuations)>"

  let pp _ _ = Printf.printf "%s\n" "<WrappedMap.Make(Continuations)>"

  include WrappedMap.Make (Typing_cont_key)
end
