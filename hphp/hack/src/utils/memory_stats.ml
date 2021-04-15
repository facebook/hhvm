(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Base.Option.Monad_infix

let lines path =
  try Some (Str.split (Str.regexp_string "\n") (Path.cat (Path.make path)))
  with _ -> None

let read_status pid_str =
  let colon = Str.regexp_string ":" in
  let to_pairs line =
    match Str.split colon line with
    | [k; v] -> Some (Caml.String.trim k, Caml.String.trim v)
    | _ -> None
  in
  lines (Printf.sprintf "/proc/%s/status" pid_str)
  |> Option.map ~f:(List.filter_map ~f:to_pairs)

(*
  Convert the human-readable format of /proc/[pid]/status back to bytes
  Because the machine-readable /proc/[pid]/stat,statm files do not have the
  VmHWM we need
*)
let to_bytes_opt str =
  match Str.split (Str.regexp_string " ") str with
  | [amount; "kB"] -> Some (int_of_string amount * 1024)
  | _ -> None

let get_status_entry status ~key =
  List.Assoc.find status ~equal:String.( = ) key >>= to_bytes_opt

let get_vm_rss () = read_status "self" >>= get_status_entry ~key:"VmRSS"

let get_vm_hwm () = read_status "self" >>= get_status_entry ~key:"VmHWM"
