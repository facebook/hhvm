(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Default logging level is 2 *)
let verbosity_level = ref 2

let print ~level ?(newline = true) ?(channel = stdout) s =
  if !verbosity_level >= level
  then Printf.fprintf channel "%s%s" s (if newline then "\n" else "")
  else ()

let error ~level ?(newline = true) s =
  print ~level ~newline ~channel:stderr s

let debug ?(newline = true) s =
  print ~level:2 ~newline ~channel:stderr s
