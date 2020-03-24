(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The remote service mode *)
type remote_mode =
  (* Real remote mode *)
  | Remote
  (* Pseudo remote on the same machine as client *)
  | PseudoRemote
