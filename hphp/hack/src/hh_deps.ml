(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-3"]

(* command line driver *)
let () =
  if !Sys.interactive then
    ()
  else
    print_endline "Dependency Tool has been called. Hello world!"
