(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module for us to interface with Watchman, a file watching service.
    https://facebook.github.io/watchman/
  *)

include Watchman_sig.S

module Process : functor (Exec : Watchman_sig.Exec) ->
  Watchman_sig.Process_S
    with type 'a future := 'a Exec.future
     and type exec_error := Exec.error
