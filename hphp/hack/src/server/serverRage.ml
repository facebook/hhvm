(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go (_genv : ServerEnv.genv) (env : ServerEnv.env) : ServerRageTypes.result =
  let open ServerRageTypes in
  let data =
    Printf.sprintf
      "hh_server pid=%d ppid=%d\ndisk_needs_parsing: %s\n"
      (Unix.getpid ())
      (Unix.getppid ())
      (Relative_path.Set.elements env.ServerEnv.disk_needs_parsing
      |> List.map ~f:Relative_path.to_absolute
      |> String.concat ~sep:" ")
  in

  [{ title = "status"; data }]
