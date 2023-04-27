(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Concatenation of hashes, sorted in lexicographic order. *)
type t = string

let mem t h =
  let md5_bin = Md5.to_binary h in
  let rec binary_search i j =
    (* assume inclusive bound *)
    assert (i <= j);
    let m = (i + j) / 2 in
    let m_h = String.sub t ~pos:(m * 16) ~len:16 in
    if i = j then
      String.equal m_h md5_bin
    else if String.(m_h < md5_bin) then
      binary_search (m + 1) j
    else
      binary_search i m
  in
  binary_search 0 ((String.length t / 16) - 1)

let read ~path : t =
  Hh_logger.log "Reading hash table %s" path;
  let channel = Sys_utils.open_in_no_fail path in
  let next_md5 () =
    try
      Some
        (Stdlib.really_input_string channel 24
        |> Base64.decode_exn
        |> String.sub ~pos:0 ~len:16)
    with
    | Caml.End_of_file -> None
  in
  let rec loop set =
    match next_md5 () with
    | None -> set
    | Some md5 -> loop (String.Set.add set md5)
  in
  let set = loop String.Set.empty in
  In_channel.close channel;
  let res = String.concat (String.Set.to_list set) in
  Hh_logger.log "Sym table has %d element" (String.Set.length set / 16);
  if String.length res = 0 then failwith "Hash table is empty";
  if not (String.length res mod 16 = 0) then
    failwith "Hash table has incorrect format";
  res
