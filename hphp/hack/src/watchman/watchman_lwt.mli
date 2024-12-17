(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include
  Watchman_sig.Process_S
    with type 'a future := 'a Lwt.t
     and type exec_error := Lwt_utils.Process_failure.t
