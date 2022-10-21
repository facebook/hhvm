(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = Md5.Set.t

let mem = Md5.Set.mem

let read ~path : t =
  Hh_logger.log "Read hashtable %s" path;
  let channel = Sys_utils.open_in_no_fail path in
  let next_md5 () =
    try
      Some
        (Stdlib.really_input_string channel 24
        |> Base64.decode_exn
        |> String.sub ~pos:0 ~len:16
        |> Md5.of_binary_exn)
    with
    | Caml.End_of_file -> None
  in
  let rec loop set =
    match next_md5 () with
    | None -> set
    | Some md5 -> loop (Md5.Set.add set md5)
  in
  let res = loop Md5.Set.empty in
  In_channel.close channel;
  Hh_logger.log "Sym table has %d element" (Md5.Set.length res);
  res
