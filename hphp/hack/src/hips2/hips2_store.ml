(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let marshal_flags = []

let suffix = ".hips2"

let persist ~db_dir ~worker_id t =
  Sys_utils.mkdir_p db_dir ~skip_mocking:true;
  let basename =
    Format.sprintf
      "%d-%d-%d%s"
      worker_id
      (Unix.getpid ())
      (Random.int 999999999)
      suffix
  in
  let path = Filename.concat db_dir @@ basename in
  let out_channel = Out_channel.create path in
  Marshal.to_channel out_channel t marshal_flags;
  Out_channel.close out_channel

let un_persist_one path =
  let in_channel = In_channel.create path in
  Marshal.from_channel in_channel

let un_persist ~db_dir =
  Sys.readdir db_dir
  |> Array.to_sequence
  |> Sequence.filter ~f:(String.is_suffix ~suffix)
  |> Sequence.map ~f:(Filename.concat db_dir)
  |> Sequence.map ~f:un_persist_one
