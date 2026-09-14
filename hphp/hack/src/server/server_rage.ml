(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go (_genv : Server_env.genv) (env : Server_env.env) :
    Server_rage_types.result =
  let open Server_rage_types in
  let data =
    Printf.sprintf
      "hh_server pid=%d ppid=%d\ndisk_needs_parsing: %s\n"
      (Unix.getpid ())
      (Unix.getppid ())
      (Relative_path.Set.elements env.Server_env.disk_needs_parsing
      |> List.map ~f:Relative_path.to_absolute
      |> String.concat ~sep:" ")
  in

  [{ title = "status"; data }]
