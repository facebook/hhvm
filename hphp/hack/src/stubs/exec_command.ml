(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Hg
  | For_use_in_testing_only of string
  | Gstack
  | Hackfmt of string
  | Hh_server of string
  | Hh
  | Ls
  | Pgrep
  | Ps
  | Pstack
  | Strobeclient
  | Watchman

let to_string = function
  | Hg -> "hg"
  | For_use_in_testing_only path -> path
  | Gstack -> "gstack"
  | Hackfmt path -> path
  | Hh_server path -> path
  | Hh -> "hh"
  | Ls -> "ls"
  | Pgrep -> "pgrep"
  | Ps -> "ps"
  | Pstack -> "pstack"
  | Strobeclient -> "strobeclient"
  | Watchman -> "watchman"
