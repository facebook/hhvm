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
  | Shell
  | Strobeclient
  | Watchman
  | Watchman_diag

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
  | Shell ->
    if Sys.win32 then
      "cmd.exe"
    else
      "/bin/sh"
  | Strobeclient -> "strobeclient"
  | Watchman -> "watchman"
  | Watchman_diag -> "watchman-diag"
