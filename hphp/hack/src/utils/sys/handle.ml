(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type handle = int

external raw_get_handle : Unix.file_descr -> handle
  = "caml_hh_worker_get_handle"
  [@@noalloc]

external raw_wrap_handle : handle -> Unix.file_descr
  = "caml_hh_worker_create_handle"

external win_setup_handle_serialization : unit -> unit
  = "win_setup_handle_serialization"

let init =
  (* Windows: register the serialize/desarialize functions
     for the custom block of "Unix.file_descr". *)
  lazy (win_setup_handle_serialization ())

let () = Lazy.force init

let () = assert (Obj.is_int (Obj.repr Unix.stdin))

let get_handle = Obj.magic

let wrap_handle = Obj.magic

let to_in_channel h = wrap_handle h |> Unix.in_channel_of_descr

let to_out_channel h = wrap_handle h |> Unix.out_channel_of_descr
