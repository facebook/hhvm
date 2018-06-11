(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | OSS_3_26
  | OSS_3_27
  | OSS_3_28
  | HEAD
  | ISODate of int (* yyyymmdd *)
  [@@deriving show]

let from_string (s:string): t =
  match s with
    | "3.26" -> OSS_3_26
    | "3.27" -> OSS_3_27
    | "3.28" -> OSS_3_28
    | "HEAD" -> HEAD
    | _ ->
      if Str.string_match (Str.regexp "^([0-9]+)\\.([0-9]+)$") s 0
      then
        let maj = int_of_string (Str.matched_group 1 s) in
        let min = int_of_string (Str.matched_group 2 s) in
        begin if (maj > Build_id.build_major_version) || (maj = Build_id.build_major_version && min > Build_id.build_minor_version)
        then HEAD
        else failwith (
          "Forward compatibility level is set to '"^s^"', which looks like a "^
          "release version number, but isn't recognized; "^
          "it might be too old ('3.26' is the oldest recognized version)"
        )
        end
      else
        begin if Str.string_match (Str.regexp "^[0-9]+$") s 0
        then ISODate (int_of_string s)
        else failwith ("Failed to parse forward compatibility level: "^s)
        end

let default = HEAD
let current = Printf.sprintf "%d.%d" Build_id.build_major_version Build_id.build_minor_version |> from_string
let minimum = Printf.sprintf "%d.%d" Build_id.build_major_version (Build_id.build_minor_version - 1) |> from_string

let as_int t =
  match t with
    | OSS_3_26 -> 2018_04_23
    | OSS_3_27 -> 2018_06_04
    | OSS_3_28 -> 2018_08_13
    | HEAD     -> 9999_99_99
    | ISODate d -> d

let as_string t =
  match t with
    | OSS_3_26 -> "3.26"
    | OSS_3_27 -> "3.27"
    | OSS_3_28 -> "3.28"
    | HEAD     -> "HEAD"
    | ISODate d -> string_of_int(d)

let greater_than a b = (as_int b) > (as_int a)
